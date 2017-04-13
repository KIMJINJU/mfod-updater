/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    logger.c
        external/internal function implementations of IRSHTR interface
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "core/logger.h"
#include "ds/queue.h"
#include "etc/util.h"


#define MAX_LOGFILES			10
#define DIR_LOGFILE				"/mnt/mmc/mfod-data/log"
#define LOGFILE_NAME_LENGTH		256
#define LOGSTR_LENGTH			512


/* structure declaration */
struct logfile
{
	struct dirent dirent;
	struct stat   stat;
};

struct logmsg
{
	int print_tstamp;
	char string[LOGSTR_LENGTH];
	struct timespec tstamp;
};

struct logger
{
	int				running;
	int				device;
	char			logfile[LOGFILE_NAME_LENGTH];
	FILE 			*fplog;
	queue_t			*msgque;
	pthread_t 		tid;
	pthread_mutex_t mtx;
};


/* GLOBAL VARIABLES */
struct logger *g_logger = NULL;


/* INTERNAL FUNCTIONS */
static int
logger_logfile_filter(const struct dirent *ent)
{
	if (strstr(ent->d_name, ".log") != NULL)
		return 1;
	else
		return 0;
}


static int
logger_sort_logfile(const struct dirent **ent1, const struct dirent **ent2)
{
	char filepath1[64];
	char filepath2[64];
	struct stat fileinfo1 = {0};
	struct stat fileinfo2 = {0};
	double diff = 0.0;

	memset(filepath1, 0x00, sizeof(filepath1));
	memset(filepath2, 0x00, sizeof(filepath2));
	snprintf(filepath1, sizeof(filepath1), "%s/%s", DIR_LOGFILE, (*ent1)->d_name);
	snprintf(filepath2, sizeof(filepath2), "%s/%s", DIR_LOGFILE, (*ent2)->d_name);

	stat(filepath1, &fileinfo1);
	stat(filepath2, &fileinfo2);

	diff = difftime(fileinfo1.st_ctim.tv_sec, fileinfo2.st_ctim.tv_sec);

	if (diff < 0.0)
		return 1;
	else if (diff > 0.0)
		return -1;
	else
		return 0;
}


static int
logger_cleanup_logfile(void)
{
	int ret = 0;
	int numlogs = 0;
	char path[256] = {0};
	struct dirent **loglist = NULL;

	/* scan logfile directory and sort logfiles */
	numlogs = scandir(DIR_LOGFILE, &loglist, logger_logfile_filter, logger_sort_logfile);

	if (numlogs != -1)
	{
		if (numlogs >= MAX_LOGFILES)
		{
			for (int i = (MAX_LOGFILES - 1); i < numlogs; i++)
			{
				memset(path, 0x00, sizeof(path));
				snprintf(path, sizeof(path), "%s/%s", DIR_LOGFILE, loglist[i]->d_name);

				if (remove(path) == 0)
					printf("delete log file : %s\n", loglist[i]->d_name);
				else
					printf(DBGINFOFMT, "failed to delete log file : %s\n", DBGINFO, loglist[i]->d_name);
			}
		}

		for (int i = 0; i < numlogs; i++)
			free(loglist[i]);

        free(loglist);
	}
	else
	{
		ret = -1;
		printf(DBGINFOFMT, "failed to scan log directory\n", DBGINFO);
	}

	return ret;
}


static int
logger_init_logfile(void)
{
	int ret = 0;
	char buf[64];
	struct tm *tm;
	struct timespec ts;

	if (g_logger)
	{
		g_logger->fplog = NULL;
		memset(buf, 0x00, sizeof(buf));
		clock_gettime(CLOCK_REALTIME, &ts);
		tm = localtime(&ts.tv_sec);

		if (tm)
			strftime(buf, sizeof(buf), "%y%m%d_%H%M%S", tm);
		else
			snprintf(buf, sizeof(buf), "%ld", ts.tv_sec);

		snprintf(&g_logger->logfile[0], LOGFILE_NAME_LENGTH, "%s/%s.log", DIR_LOGFILE, &buf[0]);
	}
	else
	{
		ret = -1;
		printf("%s, %d : null logger pointer\n", __FILE__, __LINE__);
	}

	return ret;
}


static int
logger_open_logfile(void)
{
	int ret = 0;

	if (g_logger)
	{
		if (!g_logger->fplog)
		{
			g_logger->fplog = fopen(g_logger->logfile, "a");

			if (!g_logger->fplog)
			{
				ret = -1;
				printf("%s, %d : failed to open log file\n", __FILE__, __LINE__);
			}
		}
		else
		{
			ret = -1;
			printf("%s, %d : file pointer is not null\n", __FILE__, __LINE__);
		}
	}
	else
	{
		ret = -1;
		printf("%s, %d : null logger pointer\n", __FILE__, __LINE__);
	}

	return ret;
}


static int
logger_close_logfile(void)
{
	int ret = 0;

	if (g_logger)
	{
		if (g_logger->fplog)
		{
			fclose(g_logger->fplog);
			g_logger->fplog = NULL;
		}
		else
		{
			ret = -1;
			printf("%s, %d : null file pointer\n", __FILE__, __LINE__);
		}
	}
	else
	{
		ret = -1;
		printf("%s, %d : null logger pointer\n", __FILE__, __LINE__);
	}

	return ret;
}


static void
logger_make_timestamp_string(struct logmsg *logmsg, char *str, int length)
{
	char temp[32];
	struct tm *tm = NULL;

	memset(temp, 0x00, sizeof(temp));
	memset(str, 0x00, length);
	tm = localtime(&logmsg->tstamp.tv_sec);

	if (tm)
	{
		strftime(temp, sizeof(temp), "[%T", tm);
		snprintf(str, length, "%s.%03ld]", temp, logmsg->tstamp.tv_nsec / 1000000);
	}
	else
		snprintf(str, length, "[%ld.%03ld]", logmsg->tstamp.tv_sec, logmsg->tstamp.tv_nsec / 1000000);
}


static void *
logger_print_logmsg(void *arg)
{
	char str[64];
	struct logmsg *logmsg = NULL;

	while(g_logger->running)
	{
		pthread_mutex_lock(&g_logger->mtx);

		while(g_logger->msgque->count > 0)
		{
			if (queue_deque(g_logger->msgque, (void *) &logmsg) != 0)
				continue;

			if (g_logger->device & LOGDEV_CONSOLE)
			{
				if (logmsg->print_tstamp)
				{
					logger_make_timestamp_string(logmsg, str, sizeof(str));
					printf ("%s %s", str, logmsg->string);
				}
				else
					printf ("%s", logmsg->string);
			}

			if (g_logger->device & LOGDEV_FILE)
			{
				if (logger_open_logfile() == 0)
				{
					if (logmsg->print_tstamp)
					{
						logger_make_timestamp_string(logmsg, str, sizeof(str));
						fprintf(g_logger->fplog, "%s %s", str, logmsg->string);
					}
					else
						fprintf (g_logger->fplog, "%s", logmsg->string);

					logger_close_logfile();
				}
				else
					printf("%s, %d : failed to open logfile\n", __FILE__, __LINE__);
			}

			free(logmsg);
		}

		pthread_mutex_unlock(&g_logger->mtx);

		usleep(500 * 1000);
	}

	return NULL;
}


/* EXTERNAL FUNCTIONS */
int
logger_init(int logdev)
{
	int ret = 0;

	g_logger = malloc(sizeof(struct logger));

	if (g_logger)
	{
		g_logger->msgque = queue_create();

		if (g_logger->msgque)
		{
			g_logger->device = logdev;

			if (g_logger->device & LOGDEV_FILE)
			{
				memset(&g_logger->logfile[0], 0x00, sizeof(char) * LOGFILE_NAME_LENGTH);
				logger_init_logfile();
				logger_cleanup_logfile();
			}

			g_logger->running = 1;
			pthread_mutex_init(&(g_logger->mtx), NULL);

			if (pthread_create(&g_logger->tid, NULL, logger_print_logmsg, NULL) != 0)
			{
				pthread_mutex_destroy(&g_logger->mtx);
				queue_destroy(g_logger->msgque);
				free(g_logger);
				g_logger = NULL;
				ret = -1;

				printf("%s, %d : pthread_create return fail, %s\n", __FILE__, __LINE__, strerror(errno));
			}
		}
		else
		{
			free(g_logger);
			g_logger = NULL;
			ret = -1;
			printf("%s, %d : queue_create return null\n", __FILE__, __LINE__);
		}
	}
	else
	{
		ret = -1;
		printf("%s, %d : malloc return null, %s\n", __FILE__, __LINE__, strerror(errno));
	}


	return ret;
}


int
logger_deinit(void)
{
	int ret = 0;

	if (g_logger)
	{
		g_logger->running = 0;
		pthread_join(g_logger->tid, NULL);
		pthread_mutex_destroy(&g_logger->mtx);
		queue_destroy(g_logger->msgque);
		free(g_logger);
		g_logger = NULL;
	}
	else
	{
		ret = -1;
		printf("%s, %d : null logger pointer\n", __FILE__, __LINE__);
	}

	return ret;
}


int
logger_write_tlog(const char *fmt, ...)
{
	int ret = 0;
	struct logmsg *logmsg = NULL;
	va_list arg;

	if (g_logger)
	{
		logmsg = (struct logmsg *) malloc(sizeof(struct logmsg));

		if (logmsg)
		{
			pthread_mutex_lock(&g_logger->mtx);
			va_start(arg, fmt);
			vsnprintf(logmsg->string, LOGSTR_LENGTH, fmt, arg);
			logmsg->print_tstamp = 1;
			clock_gettime(CLOCK_REALTIME, &logmsg->tstamp);
			logmsg->tstamp.tv_sec = logmsg->tstamp.tv_sec + get_time_offset();
			queue_enque(g_logger->msgque, (void *)logmsg);
			va_end(arg);
			pthread_mutex_unlock(&g_logger->mtx);
		}
		else
		{
			ret = -1;
			printf("%s, %d : malloc return null\n", __FILE__, __LINE__);
		}
	}
	else
	{
		ret = -1;
		printf("%s, %d : null logger pointer\n", __FILE__, __LINE__);
	}

	return ret;
}


int
logger_write_log(const char *fmt, ...)
{
	int ret = 0;
	struct logmsg *logmsg = NULL;
	va_list arg;

	if (g_logger)
	{
		logmsg = (struct logmsg *) malloc(sizeof(struct logmsg));

		if (logmsg)
		{
			pthread_mutex_lock(&g_logger->mtx);
			va_start(arg, fmt);
			vsnprintf(logmsg->string, LOGSTR_LENGTH, fmt, arg);
			logmsg->print_tstamp = 0;
			queue_enque(g_logger->msgque, (void *)logmsg);
			va_end(arg);
			pthread_mutex_unlock(&g_logger->mtx);
		}
		else
		{
			ret = -1;
			printf("%s, %d : malloc return null\n", __FILE__, __LINE__);
		}
	}
	else
	{
		ret = -1;
		printf("%s, %d : null logger interface\n", __FILE__, __LINE__);
	}

	return ret;
}
