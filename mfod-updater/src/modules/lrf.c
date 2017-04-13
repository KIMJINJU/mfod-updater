/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    lrf.c
        external/internal function implementations of LRF interface
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/poll.h>

#include "core/logger.h"
#include "etc/util.h"
#include "modules/gpio.h"
#include "modules/lrf.h"
#include "modules/uart.h"

/* macro defines : LRF control GPIO index */
#define NUM_LRFCTL_GPIO				10
#define LRFCTL_GPIO_CPLD_PWR 		0
#define LRFCTL_GPIO_LRF_TXPWR  		1
#define LRFCTL_GPIO_LRF_RXPWR		2
#define LRFCTL_GPIO_CPLD_START		3
#define LRFCTL_GPIO_CPLD_RESET		4
#define LRFCTL_GPIO_CPLD_FCON		5
#define LRFCTL_GPIO_CPLD_READY		6
#define LRFCTL_GPIO_CPLD_DATAREAD	7
#define LRFCTL_GPIO_CPLD_DATARDY	8
#define LRFCTL_GPIO_CPLD_SPARE2		9

/* macro defines : LRF CMD				*/
#define LRF_MSG_SOM					0x81
#define LRF_CMD_GET_STATUS			0x41
#define LRF_CMD_RESET				0x42
#define LRF_CMD_CLR_REG				0x43
#define LRF_CMD_SET_RGMIN			0x44
#define LRF_CMD_SET_RGMAX			0x45
#define LRF_CMD_REQ_MEASURES		0x47

/* macro defines : LRF message offset	*/
#define LRF_MSG_OFFSET_SOM			0
#define LRF_MSG_OFFSET_LENGTH		1
#define LRF_MSG_OFFSET_OPCODE		2
#define LRF_MSG_OFFSET_STATUS		3
#define LRF_MSG_OFFSET_PARAM0		4
#define LRF_MSG_OFFSET_PARAM1		5

/* macro defines : thread index		*/
#define LRF_THREAD_UART				0
#define LRF_THREAD_MEAS				1
#define LRF_THREAD_BITOPT			2

/* macro defines : timeout		*/
#define LRF_COMM_TIMEOUT			3.0

/* macro defines : range compensation interval */
//#define LRF_COMP_INTERVD			6100
//#define LRF_COMP_INTERVC			1070
//#define LRF_COMP_INTERVB			270
//#define LRF_COMP_INTERVA			60

#define LRF_COMP_INTERVB			530
#define LRF_COMP_INTERVA			85

/* macro defines : LRF status bit */
#define LRF_STBIT_EXEERR			0x01
#define LRF_STBIT_BUSY				0x02
#define LRF_STBIT_TEMPERR			0x04
#define LRF_STBIT_RCVERR			0x08
#define LRF_STBIT_XMTRERR			0x10
#define LRF_STBIT_RPROCERR			0x20
#define LRF_STBIT_COMMERR			0x40
#define LRF_STBIT_CMDERR			0x80


/* structure declaration : LRF interface attribute */
struct lrf_attribute
{
	/* external interface */
	struct lrf_interface extif;

	/* internal attribute */
	bool standby;
	bool running;
	bool inProc;
	bool waitAck;
	bool power;
	int error;
	int trigger;
	double rcc[6];					/* rcc   : range compensation coefficients  	*/
	unsigned short rg[2];			/* rg    : range gate							*/
	unsigned short meas[2];			/* meas  : distance measurements				*/
	unsigned short range[2];		/* range : compensated distance measurements 	*/
	pthread_t tid[3];
	pthread_mutex_t mtx[2];
	pthread_cond_t  cond;
	struct timespec timeStamp;
	struct uart_interface *uart;
	struct gpio_interface *gpio[NUM_LRFCTL_GPIO];
};


static int
lrf_enable_power(struct lrf_interface *lrf, bool flag)
{
	int ret = 0;
	struct gpio_interface *xtmr = NULL;
	struct gpio_interface *rcvr = NULL;
	struct gpio_interface *cpld  = NULL;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		xtmr = this->gpio[LRFCTL_GPIO_LRF_TXPWR];
		rcvr = this->gpio[LRFCTL_GPIO_LRF_RXPWR];
		cpld = this->gpio[LRFCTL_GPIO_CPLD_PWR];

		if (flag)
		{
			cpld->setVal(cpld, GPIO_VALUE_HIGH);
			MSLEEP(1);
			xtmr->setVal(xtmr, GPIO_VALUE_HIGH);
			MSLEEP(1);
			rcvr->setVal(rcvr, GPIO_VALUE_HIGH);
			MSLEEP(1);
			this->power = true;
			TLOGMSG(1, ("enable lrf power\n"));
		}
		else
		{
			rcvr->setVal(rcvr, GPIO_VALUE_LOW);
			MSLEEP(1);
			xtmr->setVal(xtmr, GPIO_VALUE_LOW);
			MSLEEP(1);
			cpld->setVal(cpld, GPIO_VALUE_LOW);
			MSLEEP(1);
			this->power = false;
			TLOGMSG(1, ("disable lrf power\n"));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface \n", DBGINFO));
	}

	return ret;
}


static int
lrf_enable_start_signal(struct lrf_interface *lrf, bool flag)
{
	int ret = 0;
	struct gpio_interface *start = NULL;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		start = this->gpio[LRFCTL_GPIO_CPLD_START];

		if (flag)
		{
			start->setVal(start, GPIO_VALUE_HIGH);
			TLOGMSG(1, ("LRF CPLD_START = 0\n"));
		}
		else
		{
			start->setVal(start, GPIO_VALUE_HIGH);
			TLOGMSG(1, ("LRF CPLD_START = 1\n"));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}


static int
lrf_enable_fctl_signal(struct lrf_interface *lrf, bool flag)
{
	int ret = 0;
	struct gpio_interface *fctl = NULL;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		fctl = this->gpio[LRFCTL_GPIO_CPLD_FCON];

		if (flag)
		{
			fctl->setVal(fctl, GPIO_VALUE_HIGH);
			TLOGMSG(1, ("LRF CPLD_FIRECON = 1\n"));
		}
		else
		{
			fctl->setVal(fctl, GPIO_VALUE_LOW);
			TLOGMSG(1, ("LRF CPLD_FIRECON = 0\n"));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface \n", DBGINFO));
	}

	return ret;
}


static int
lrf_wait_frdy_signal(struct lrf_interface *lrf, int timeo)
{
	int ret = -1;
	int count = 0;
	int value = 0;
	int timeout = timeo * 2;
	struct gpio_interface *frdy = NULL;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		frdy = this->gpio[LRFCTL_GPIO_CPLD_READY];
		frdy->open(frdy);

		while (count < timeout)
		{
			frdy->read(frdy, &value);

			if (value == GPIO_VALUE_LOW)
			{
				ret = 0;
				TLOGMSG(1, ("LRF CPLD_READY = %d\n", value));
				break;
			}
			else
			{
				usleep(500);
				count++;
			}
		}

		frdy->close(frdy);
	}
	else
	{
		ret  = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}


static int
lrf_wait_measrdy_signal(struct lrf_interface *lrf, int timeo)
{
	int ret = -1;
	int value = 0;
	int count = 0;
	int timeout = timeo * 2;
	struct gpio_interface *mrdy = NULL;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (lrf)
	{
		mrdy = this->gpio[LRFCTL_GPIO_CPLD_DATARDY];
		mrdy->open(mrdy);

		while (count < timeout)
		{
			mrdy->read(mrdy, &value);

			if (value == GPIO_VALUE_HIGH)
			{
				ret = 0;
				TLOGMSG(1, ("LRF CPLD_DATAREADY = %d\n", value));
				break;
			}
			else
			{
				usleep(500);
				count++;
			}
		}

		mrdy->close(mrdy);
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}


static int
lrf_set_trigger(struct lrf_interface *lrf, int trg)
{
	int ret = 0;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		pthread_mutex_lock(&this->mtx[1]);
		this->trigger = trg;
		pthread_mutex_unlock(&this->mtx[1]);
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}


static int
lrf_poll_trigger(struct lrf_interface *lrf, int timeo)
{
	int ret = -1;
	int count = 0;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		while (count < timeo)
		{
			if (this->trigger == LRF_TRG_FIRE)
			{
				ret = 0;
				break;
			}
			else
			{
				MSLEEP(1);
				count++;
			}
		}
	}
	else
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));

	return ret;
}


static int
lrf_activate_reset_signal(struct lrf_interface *lrf)
{
	int ret = 0;
	struct gpio_interface *reset = NULL;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		reset = this->gpio[LRFCTL_GPIO_CPLD_RESET];
		reset->setVal(reset, GPIO_VALUE_LOW);
		MSLEEP(50);
		reset->setVal(reset, GPIO_VALUE_HIGH);
		TLOGMSG(1, ("LRF CPLD_RESET\n"));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}


static int
lrf_activate_measrd_signal(struct lrf_interface *lrf)
{
	int ret = 0;
	struct gpio_interface *mread = NULL;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (lrf)
	{
		mread = this->gpio[LRFCTL_GPIO_CPLD_DATAREAD];
		mread->setVal(mread, GPIO_VALUE_HIGH);
		MSLEEP(50);
		mread->setVal(mread, GPIO_VALUE_LOW);
		TLOGMSG(1, ("LRF CPLD_DATAREADOK\n"));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "nulll lrf interface\n", DBGINFO));
	}

	return ret;
}

static void
lrf_wait_ack(struct lrf_interface *lrf)
{
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		if (this->waitAck == false)
		{
			pthread_mutex_lock(&this->mtx[0]);
			this->waitAck = true;

			while(this->waitAck)
				pthread_cond_wait(&this->cond, &this->mtx[0]);

			pthread_mutex_unlock(&this->mtx[0]);
		}
		else
			TLOGMSG(1, ("already waiting for ack...\n"));
	}
	else
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
}


static void
lrf_recv_ack(struct lrf_interface *lrf)
{
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		if (this->waitAck == true)
		{
			pthread_mutex_lock(&this->mtx[0]);
			this->waitAck = false;
			pthread_mutex_unlock(&this->mtx[0]);
			pthread_cond_signal(&this->cond);
		}
		else
			TLOGMSG(1, ("already waiting for ack...\n"));
	}
	else
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
}


static char
lrf_get_msglen(char cmd)
{
	char length = 0;

	switch(cmd)
	{
	case LRF_CMD_GET_STATUS: case LRF_CMD_RESET:
	case LRF_CMD_CLR_REG: case LRF_CMD_REQ_MEASURES:
		length = 0x04;
		break;

	case LRF_CMD_SET_RGMIN: case LRF_CMD_SET_RGMAX:
		length = 0x06;
		break;

	default:
		length = -1;
		TLOGMSG(1, (DBGINFOFMT "failed to find message length\n", DBGINFO ));
		break;
	}

	return length;
}



static int
lrf_create_message(char cmd, char *parm, char *msg)
{
	int ret = 0;
	char length = 0;

	length = lrf_get_msglen(cmd);

	if (length != -1)
	{
		*(msg + LRF_MSG_OFFSET_SOM)	   = LRF_MSG_SOM;
		*(msg + LRF_MSG_OFFSET_LENGTH) = length;
		*(msg + LRF_MSG_OFFSET_OPCODE) = cmd;

		if (parm)
			memcpy((msg + LRF_MSG_OFFSET_PARAM0 - 1), parm, sizeof(char) * (length - 4));

		*(msg + length - 1) = calculate_checksum(msg, length);
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "lrf_get_msglen return fail\n", DBGINFO));
	}

	return ret;
}


static int
lrf_txcmd(struct lrf_interface *lrf, char cmd, char *parm)
{
	int nwr = 0;
	int ret = 0;
	char msg[16] = {0};
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		if (lrf_create_message(cmd, parm, msg) == 0)
		{
			nwr = this->uart->write(this->uart, msg, *(msg + LRF_MSG_OFFSET_LENGTH));

			if (nwr < 0)
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "uart write return fail\n", DBGINFO));
			}
			else
			{
				clock_gettime(CLOCK_REALTIME, &this->timeStamp);
				lrf_wait_ack(lrf);
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "lrf_create_message return fail\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}


static int
lrf_set_rgate_max(struct lrf_interface *lrf)
{
	int ret = 0;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		if (lrf_txcmd(lrf, LRF_CMD_SET_RGMAX, (char *) &this->rg[0]) != 0)
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "lrf_txcmd return fail (LRF_CMD_SET_RGMAX)\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}


static int
lrf_set_rgate_min(struct lrf_interface *lrf)
{
	int ret = 0;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		if (lrf_txcmd(lrf, LRF_CMD_SET_RGMIN, (char *) &this->rg[1]) != 0)
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "lrf_txcmd return fail (LRF_CMD_SET_RGMIN)\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}


static int
lrf_request_measures(struct lrf_interface *lrf)
{
	int ret = 0;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		if (lrf_txcmd(lrf, LRF_CMD_REQ_MEASURES, NULL) != 0)
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "lrf_txcmd return null (LRF_CMD_REQ_MEASURES)\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}


static int
lrf_parse_reqmeas_ack(struct lrf_interface *lrf, char *msg)
{
	int ret = 0;
	double temp[2] = {0};
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		memcpy(&this->meas[0], (msg + LRF_MSG_OFFSET_PARAM0), sizeof(unsigned short));
		memcpy(&this->meas[1], (msg + LRF_MSG_OFFSET_PARAM1), sizeof(unsigned short));
		TLOGMSG(1, ("range = %dm, %dm\n", this->meas[0], this->meas[1]));

		for (int i = 0; i < 2; i++)
		{
			/*
			if (attr->range[i] > LRF_COMP_INTERVD)
				temp[i] = (double) attr->meas[i] - 5.0;
			else if (attr->meas[i] >= LRF_COMP_INTERVC)
				temp[i] = (double)attr->meas[i] + (attr->rcc[4] * (double)attr->meas[i] + attr->rcc[5]);
			else if (attr->meas[i] >= LRF_COMP_INTERVB)
				temp[i] = (double)attr->meas[i] + (attr->rcc[2] * (double)attr->range[i] + attr->rcc[3]);
			else if (attr->meas[i] >= LRF_COMP_INTERVA)
				temp[i] = (double)attr->meas[i] + (attr->rcc[0] * (double)attr->meas[i] + attr->rcc[1]);
			else
				temp[i] = 0.0;
			*/

			if (this->meas[i] >= LRF_COMP_INTERVB)
				temp[i] = (double)this->meas[i] + (this->rcc[2] * (double)this->range[i] + this->rcc[3]);
			else if (this->meas[i] >= LRF_COMP_INTERVA)
				temp[i] = (double)this->meas[i] + (this->rcc[0] * (double)this->meas[i] + this->rcc[1]);
			else
				temp[i] = 0.0;
		}

		this->range[0] = (unsigned short) lround(temp[0]);
		this->range[1] = (unsigned short) lround(temp[1]);
		TLOGMSG(1, ("compensated range = %dm, %dm\n", this->range[0], this->range[1]));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}


static int
lrf_parse_rgate_max_ack(struct lrf_interface *lrf, char *msg)
{
	int ret = 0;
	unsigned short temp = 0;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		memcpy(&temp, (msg + LRF_MSG_OFFSET_PARAM0), sizeof(unsigned short));

		if (temp == this->rg[0])
			TLOGMSG(1, ("max range gate = %d\n", temp));
		else
			TLOGMSG(1, (DBGINFOFMT "failed to set range gate max\n", DBGINFO));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}


static int
lrf_parse_rgate_min_ack(struct lrf_interface *lrf, char *msg)
{
	int ret = 0;
	unsigned short temp = 0;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		memcpy(&temp, (msg + LRF_MSG_OFFSET_PARAM0), sizeof(unsigned short));

		if (temp == this->rg[1])
			TLOGMSG(1, ("min range gate = %d\n", temp));
		else
			TLOGMSG(1, (DBGINFOFMT "failed to set range gate min\n", DBGINFO));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
 }


static int
lrf_check_status(struct lrf_interface *lrf, char status)
{
	struct status_desc
	{
		char bit;
		char *string;
	};

	struct status_desc status_desc[] =
	{
		{ .bit = LRF_STBIT_EXEERR	, .string = "command execution error" },
		{ .bit = LRF_STBIT_BUSY		, .string = "in busy status"		  },
		{ .bit = LRF_STBIT_TEMPERR	, .string = "temperature error"		  },
		{ .bit = LRF_STBIT_RCVERR	, .string = "receiver error"		  },
		{ .bit = LRF_STBIT_XMTRERR	, .string = "transmitter error"		  },
		{ .bit = LRF_STBIT_RPROCERR	, .string = "range processor error"	  },
		{ .bit = LRF_STBIT_COMMERR	, .string = "communication error"	  },
		{ .bit = LRF_STBIT_CMDERR	, .string = "command error"			  }
	};

	int ret = 0;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		for (int i = 0; i < DIM(status_desc); i++)
		{
			if (status & status_desc[i].bit)
			{
				ret |= (1 << i);
				TLOGMSG(1, ("lrf status check : %s\n", status_desc[i].string));
			}
			else
				continue;
		}

		if (status & LRF_STBIT_RCVERR)
			this->error |= LRF_ERROR_RCVR;
		else
			this->error &= (~LRF_ERROR_RCVR);

		if ((status & LRF_STBIT_XMTRERR) || (status & LRF_STBIT_RPROCERR) ||(status & LRF_STBIT_TEMPERR))
			this->error |= LRF_ERROR_XMTR;
		else
			this->error &= (~LRF_ERROR_XMTR);
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}


static int
lrf_parse_message(struct lrf_interface *lrf, char *msg)
{
	struct message_parser
	{
		int cmd;
		int (*func) (struct lrf_interface *, char *);
	};

	struct message_parser parser[] =
	{
		{.cmd = LRF_CMD_REQ_MEASURES, .func = lrf_parse_reqmeas_ack  },
		{.cmd = LRF_CMD_SET_RGMAX	, .func = lrf_parse_rgate_max_ack},
		{.cmd = LRF_CMD_SET_RGMIN	, .func = lrf_parse_rgate_min_ack},
	};

	int ret = 0;
	char length = 0;
	char cmd = 0;
	char csum_recv = 0;
	char csum_calc = 0;

	length = *(msg + LRF_MSG_OFFSET_LENGTH);
	cmd = *(msg + LRF_MSG_OFFSET_OPCODE);
	csum_recv = *(msg + length - 1);
	csum_calc = calculate_checksum(msg, length);

	if (csum_recv == csum_calc)
	{
		if (lrf_check_status(lrf, *(msg + LRF_MSG_OFFSET_STATUS)) == 0)
		{
			for (int i = 0; i < DIM(parser); i++)
			{
				if (cmd == parser[i].cmd)
				{
					ret = parser[i].func(lrf, msg);
					break;
				}
				else
					continue;
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "LRF module return error status\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "checksum mismatch (recv = %02x, calc = %02x)\n", csum_recv, csum_calc));
	}

	return ret;
}


static void *
lrf_read_message(void *arg)
{
	int nread = 0;
	int idx = 0;
	int head = 0;
	int ret = 0;
	char buf[256] = {0};
	char *msghdr = NULL;

	struct lrf_interface *lrf = (struct lrf_interface *) arg;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;
	struct uart_interface *uart = this->uart;
	struct timespec tstamp = {.tv_sec = 0, .tv_nsec = 0};
	struct pollfd pfd = {.fd = uart->getFileDesc(uart), .events = POLLIN, .revents = 0};

	memset(buf, 0x00, sizeof(buf));
	this->running = true;

	while(this->running)
	{
		if (poll(&pfd, 1, 1000) > 0)
		{
			nread = uart->read(uart, &buf[idx], uart->getNumRead(uart));

			if (nread > 0)
			{
				for (int i = 0; i < nread; i++)
				{
					if ((buf[idx] == LRF_MSG_SOM) && (!msghdr))
					{
						msghdr = &buf[idx];
						head = idx;
						clock_gettime(CLOCK_REALTIME, &tstamp);
					}

					idx++;
				}

				if (msghdr)
				{
					if ((idx - head) == *(msghdr + LRF_MSG_OFFSET_LENGTH))
					{
						this->error &= ~LRF_ERROR_COMM;
						lrf_parse_message(lrf, msghdr);
						lrf_recv_ack(lrf);
						memset(buf, 0x00, sizeof(buf));
						idx = head = 0;
						msghdr = NULL;
					}
					else
						continue;
				}
				else
					continue;
			}
		}
		else
		{
			if (this->waitAck)
			{
				if (check_timeout(&this->timeStamp, LRF_COMM_TIMEOUT) != 0)
				{
					this->error |= LRF_ERROR_COMM;
					lrf_recv_ack(lrf);
					memset(buf, 0x0, sizeof(buf));
					idx = head = 0;
					msghdr = NULL;
					TLOGMSG(1, (DBGINFOFMT "timeout expires while waiting ack\n", DBGINFO));
				}
				else
					continue;
			}
			else
			{
				if ((!msghdr) && (check_timeout(&tstamp, LRF_COMM_TIMEOUT) != 0))
				{
					memset(buf, 0x00, sizeof(buf));
					idx = head = 0;
					msghdr = NULL;
				}
				else
					continue;
			}
		}
	}

	return NULL;
}


static void *
lrf_thread_measuring(void *arg)
{
	int ret = 0;
	long sleep_time = 0;
	double elapsed_time = 0.0;
	struct timespec t1 = {.tv_sec = 0, .tv_nsec = 0};
	struct timespec t2 = {.tv_sec = 0, .tv_nsec = 0};
	struct lrf_interface *lrf = (struct lrf_interface *) arg;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	this->inProc = true;
	this->meas[0] = 0;
	this->meas[1] = 0;
	lrf_set_trigger(lrf, LRF_TRG_WAIT);
	lrf_enable_power(lrf, true);
	lrf_enable_start_signal(lrf, true);
	lrf_activate_reset_signal(lrf);
	lrf_enable_fctl_signal(lrf, true);
	clock_gettime(CLOCK_REALTIME, &t1);

	if (lrf_wait_frdy_signal(lrf, 3000) == 0)
	{
		clock_gettime(CLOCK_REALTIME, &t2);
		elapsed_time = get_elapsed_time(&t1, &t2);

		if (elapsed_time < 1.000000)
		{
			sleep_time = (long) ((1.000000 - elapsed_time) * 1000000);
			usleep(sleep_time);
		}

		if ((this->rg[0] != 0) && (this->rg[1] != 0))
		{
			lrf_set_rgate_min(lrf);
			lrf_set_rgate_max(lrf);
		}

		if (this->trigger != LRF_TRG_FALSE)
		{
			lrf_set_trigger(lrf, LRF_TRG_READY);

			if (lrf_poll_trigger(lrf, 3000) == 0)
			{
				lrf_enable_fctl_signal(lrf, false);
				TLOGMSG(1, ("fire laser\n"));

				if (lrf_wait_measrdy_signal(lrf, 1000) == 0)
				{
					lrf_request_measures(lrf);
					lrf_activate_measrd_signal(lrf);
				}
				else
				{
					ret = 1;
					TLOGMSG(1, ("CPLD_DATAREADY timeout\n"));
				}
			}
			else
			{
				ret = 1;
				lrf_set_trigger(lrf, LRF_TRG_OVERCHARGE);
				lrf_enable_fctl_signal(lrf, false);
				lrf_wait_measrdy_signal(lrf, 1000);
				TLOGMSG(1, ("lrf overcharged\n"));
			}
		}
		else
		{
			ret = 1;
			lrf_enable_fctl_signal(lrf, false);
			lrf_wait_measrdy_signal(lrf, 1000);
			TLOGMSG(1, ("trigger is released before lrf is charged\n"));
		}
	}
	else
	{
		ret = 1;
		this->error |= LRF_ERROR_XMTR;
		TLOGMSG(1, ("timeout occurred while waiting CPLD_READY signal\n"));
	}

	lrf_enable_start_signal(lrf, false);
	lrf_enable_power(lrf, false);

    return ((void *) ret);
}


static int
lrf_start_measuring(struct lrf_interface *lrf)
{
	int ret = 0;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		if (!this->inProc)
		{
			if (pthread_create(&this->tid[LRF_THREAD_MEAS], NULL, lrf_thread_measuring, (void *)lrf) == 0)
				TLOGMSG(1, ("start range measuring process\n"));
			else
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "pthread_create return fail\n", DBGINFO));
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, ("in measuring process\n"));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}


static int
lrf_finalize_measruing(struct lrf_interface *lrf)
{
	int ret = 0;
    void *retval = NULL;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		if (this->inProc)
		{
			pthread_join(this->tid[LRF_THREAD_MEAS], &retval);
			this->inProc = false;
			lrf_set_trigger(lrf, LRF_TRG_NONE);
            ret = (int) retval;
		}
		else
		{
			ret = -1;
			TLOGMSG(1, ("not in range measuring process\n"));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}


static int
lrf_fire_laser(struct lrf_interface *lrf)
{
	int ret = 0;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		if (this->inProc)
		{
			switch (this->trigger)
			{
			case LRF_TRG_WAIT:
				this->trigger = LRF_TRG_FALSE;
				break;

			case LRF_TRG_READY:
				this->trigger = LRF_TRG_FIRE;
				break;

			default:
				ret = -1;
				TLOGMSG(1, ("invalid lrf trigger status\n"));
				break;
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, ("lrf is not in measuring process\n"));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}


static int
lrf_get_trigger(struct lrf_interface *lrf)
{
	int ret = 0;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
		ret = this->trigger;
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}



static int
lrf_get_range(struct lrf_interface *lrf, int *first, int *last)
{
	int ret = 0;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		if (this->range[0] > 9999)
			*first = 9999;
		else
			*first = (int) this->range[0];

		if (this->range[1] > 9999)
			*last = 9999;
		else
			*last = (int) this->range[1];
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}


static int
lrf_set_measuring_range(struct lrf_interface *lrf, int max, int min)
{
	int ret = 0;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		/* range check */
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}


static int
lrf_get_measuring_range(struct lrf_interface *lrf, int *max, int *min)
{
	int ret = 0;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		*max = (int) this->rg[0];
		*min = (int) this->rg[1];
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface \n", DBGINFO));
	}

	return ret;
}


static int
lrf_test_module(struct lrf_interface *lrf)
{
	int ret = 0;
	long sleep_time = 0;
	double elapsed_time = 0.0;
	struct timespec t1 = { .tv_sec = 0, .tv_nsec = 0};
	struct timespec t2 = { .tv_sec = 0, .tv_nsec = 0};
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;


	if (this)
	{
		lrf_enable_power(lrf, true);
		MSLEEP(800);

		lrf_request_measures(lrf);

		clock_gettime(CLOCK_REALTIME, &t1);
		lrf_enable_start_signal(lrf, true);
		lrf_activate_reset_signal(lrf);
		lrf_enable_fctl_signal(lrf, true);

		if (lrf_wait_frdy_signal(lrf, 5000) == 0)
		{
			clock_gettime(CLOCK_REALTIME, &t2);
			elapsed_time = get_elapsed_time(&t1, &t2);

			if (elapsed_time < 1.000000)
			{
				sleep_time = (long) ((1.000000 - elapsed_time) * 1000000);
				usleep(sleep_time);
			}

			lrf_enable_fctl_signal(lrf, false);
			lrf_wait_measrdy_signal(lrf, 1000);
			this->error &= (~LRF_ERROR_XMTR);
		}
		else
			this->error |= LRF_ERROR_XMTR;

		lrf_enable_power(lrf, false);
		ret = this->error;
		TLOGMSG(1, ("lrf module test result = 0x%02X\n", this->error));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}


static int
lrf_set_rcc(struct lrf_interface *lrf, double *rcc, int numrcc)
{
	int ret = 0;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if(this)
	{
		memcpy(&this->rcc[0], rcc, sizeof(double) * numrcc);

		for(int i = 0; i < numrcc; i++)
			TLOGMSG(1, ("range compensation coeffient %d = %lf\n", i, this->rcc[i]));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}


static int
lrf_get_rcc(struct lrf_interface *lrf, double *a, double *b, double *c, double *d)
{
	int ret = 0;
	struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if(this)
    {
        *a = this->rcc[0];
        *b = this->rcc[1];
        *c = this->rcc[2];
        *d = this->rcc[3];
    }
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}


static int
lrf_get_error(struct lrf_interface *lrf)
{
	int ret = 0;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
		ret = this->error;
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}


static int
lrf_enter_standby_mode(struct lrf_interface *lrf)
{
	int ret = 0;
	struct uart_interface *uart = NULL;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		if (!this->standby)
		{
			uart = this->uart;
			this->running = false;
			pthread_join(this->tid[LRF_THREAD_UART], NULL);
			uart->close(uart);
			this->standby = true;
			TLOGMSG(1, ("lrf module is in standby mode now\n"));
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "lrf module is already in standby mode\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}


static int
lrf_exit_standby_mode(struct lrf_interface *lrf)
{
	int ret = 0;
	int count = 0;
	char *tty = UART_TTYUSB2;
	struct uart_interface *uart = NULL;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		if (this->standby)
		{
			uart = this->uart;

			do
			{
				ret = uart->open(uart, tty, B115200);

				if (ret == 0)
				{
					if (pthread_create(&this->tid[LRF_THREAD_UART], NULL, lrf_read_message, (void *) lrf) == 0)
					{
						this->standby = false;
						TLOGMSG(1, ("lrf is in normal mode\n"));
					}
					else
					{
						ret = -1;
						uart->close(uart);
						TLOGMSG(1, (DBGINFOFMT "pthread_create return fail, %s\n", DBGINFO, strerror(errno)));
						break;
					}
				}
				else
				{
					if (count == 3)
					{
						if (strstr(tty, "USB2"))
							tty = UART_TTYUSB5;
						else
							tty = UART_TTYUSB2;

						count = 0;
						MSLEEP(100);
						TLOGMSG(1, ("not found device, switching devcice and retry open (%s)\n", tty));
					}
					else
					{
						count++;
						MSLEEP(100);
						TLOGMSG(1, ("uart device is not ready, retry open uart device...\n"));
					}
				}
			}
			while (ret != 0);
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "lrf module is already in normal mode\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}


static int
lrf_init_gpios(struct lrf_interface *lrf)
{
    struct gpio_init_info
    {
        int pin;
        int dir;
        int val;
    };

    struct gpio_init_info info[] =
    {
        { .pin = GPIO1_CPLD_PWR		, .dir = GPIO_DIR_OUTPUT, .val = GPIO_VALUE_LOW },
        { .pin = GPIO6_LRF_TXPWR	, .dir = GPIO_DIR_OUTPUT, .val = GPIO_VALUE_LOW },
        { .pin = GPIO1_LRF_RXPWR	, .dir = GPIO_DIR_OUTPUT, .val = GPIO_VALUE_LOW },
        { .pin = GPIO5_CPLD_START	, .dir = GPIO_DIR_OUTPUT, .val = GPIO_VALUE_LOW },
        { .pin = GPIO5_CPLD_RESET	, .dir = GPIO_DIR_OUTPUT, .val = GPIO_VALUE_HIGH},
        { .pin = GPIO5_CPLD_FCON	, .dir = GPIO_DIR_OUTPUT, .val = GPIO_VALUE_LOW },
        { .pin = GPIO1_CPLD_READY	, .dir = GPIO_DIR_INPUT , .val = GPIO_VALUE_LOW },
        { .pin = GPIO5_CPLD_DATAREAD, .dir = GPIO_DIR_OUTPUT, .val = GPIO_VALUE_LOW },
        { .pin = GPIO1_CPLD_DATARDY , .dir = GPIO_DIR_INPUT , .val = GPIO_VALUE_LOW },
        { .pin = GPIO4_CPLD_SPARE2  , .dir = GPIO_DIR_INPUT , .val = GPIO_VALUE_LOW },
    };

    int ret = 0;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

    if (this)
    {
        memset(&this->gpio[0], 0x00, sizeof(struct gpio_interface *) * NUM_LRFCTL_GPIO);

        for (int i = 0; i < NUM_LRFCTL_GPIO; i++)
        {
			this->gpio[i] = gpio_create();

            if (this->gpio[i])
            {
                if (this->gpio[i]->export(this->gpio[i], info[i].pin) == 0)
                {
                    this->gpio[i]->setDir(this->gpio[i], info[i].dir);

                    if (info[i].dir == GPIO_DIR_OUTPUT)
                        this->gpio[i]->setVal(this->gpio[i], info[i].val);
                }
                else
                {
                    for (int j = 0; j < NUM_LRFCTL_GPIO; j++)
                        gpio_destroy(this->gpio[j]);

                    ret = -1;
                    TLOGMSG(1, (DBGINFOFMT "gpio->export return fail (LRFCTL = %d)\n", DBGINFO, i));
                    break;
                }
            }
            else
            {
                for (int j = 0; j < NUM_LRFCTL_GPIO; j++ )
                    gpio_destroy(this->gpio[j]);

                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "gpio_create return null\n", DBGINFO));
                break;
            }
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
    }

    return ret;
}


static int
lrf_deinit_gpios(struct lrf_interface *lrf)
{
    int ret = 0;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

    if (this)
    {
        for (int i = 0; i < NUM_LRFCTL_GPIO; i++)
            gpio_destroy(this->gpio[i]);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
    }

    return ret;
}


static int
lrf_init_uart(struct lrf_interface *lrf)
{
	int ret = 0;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		this->uart = uart_create();

		if (this->uart)
		{
			if (this->uart->open(this->uart, UART_TTYUSB2, B115200) != 0)
			{
				ret = -1;
				uart_destroy(this->uart);
				TLOGMSG(1, (DBGINFOFMT "failed to open uart\n", DBGINFO));
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "failed to create uart interface\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}


static int
lrf_init_attribute(struct lrf_interface *lrf)
{
    int ret = 0;
	struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		if (lrf_init_gpios(lrf) == 0)
		{
			if (lrf_init_uart(lrf) == 0)
			{
				/* set initail attribute value */
				this->error    = LRF_ERROR_NONE;
				this->inProc   = false;
				this->power    = false;
				this->running  = false;
				this->standby  = false;
				this->trigger  = LRF_TRG_NONE;
				this->rg[0]    = 0;
				this->rg[1]    = 0;
				this->meas[0]  = 0;
				this->meas[1]  = 0;
				this->range[0] = 0;
				this->range[1] = 0;
				this->rcc[0]   = 0.0;
				this->rcc[1]   = 0.0;
				this->rcc[2]   = 0.0;
				this->rcc[3]   = 0.0;
				this->rcc[4]   = 0.0;
				this->rcc[5]   = 0.0;
			}
			else
			{
                ret = -1;
				lrf_deinit_gpios(lrf);
				TLOGMSG(1, (DBGINFOFMT "lrf_init_uart return fail\n", DBGINFO));
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "lrf_init_gpios return fail\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}


struct lrf_interface *
lrf_create(void)
{
	struct lrf_interface *lrf = NULL;
	struct lrf_attribute *this = malloc(sizeof(struct lrf_attribute));

	if (this)
	{
        memset(this, 0x00, sizeof(struct lrf_attribute));
        lrf = &(this->extif);
        lrf->fire              = lrf_fire_laser;
        lrf->getTrigger		   = lrf_get_trigger;
        lrf->getRange	       = lrf_get_range;
        lrf->setMeasuringRange = lrf_set_measuring_range;
        lrf->getMeasuringRange = lrf_get_measuring_range;
        lrf->startMeasuring	   = lrf_start_measuring;
        lrf->finalizeMeasuring = lrf_finalize_measruing;
        lrf->testModule		   = lrf_test_module;
        lrf->setRcc			   = lrf_set_rcc;
		lrf->getRcc			   = lrf_get_rcc;
        lrf->getError		   = lrf_get_error;
        lrf->standby		   = lrf_enter_standby_mode;
        lrf->wakeup			   = lrf_exit_standby_mode;

		if ((lrf_init_attribute(lrf)) == 0)
		{
			pthread_mutex_init(&this->mtx[0], NULL);
			pthread_mutex_init(&this->mtx[1], NULL);
			pthread_cond_init(&this->cond, NULL);

			if (pthread_create(&this->tid[LRF_THREAD_UART], NULL, lrf_read_message, (void *)lrf) == 0)
				TLOGMSG(1, ("create lrf interface\n"));
			else
			{
				lrf_deinit_gpios(lrf);
				uart_destroy(this->uart);
				free(this);
				lrf = NULL;
				TLOGMSG(1, (DBGINFOFMT "failed to create lrf interface, failed to create thread\n", DBGINFO));
			}
		}
		else
		{
			free(this);
			lrf = NULL;
			TLOGMSG(1, (DBGINFOFMT "failed to create lrf interface, failed to init attribute\n", DBGINFO));
		}
	}
	else
	{
		lrf = NULL;
		TLOGMSG(1, (DBGINFOFMT "failed to create lrf interface, malloc return null\n", DBGINFO));
	}

	return lrf;
}


int
lrf_destroy(struct lrf_interface *lrf)
{
	int ret = 0;
    struct lrf_attribute *this = (struct lrf_attribute *)lrf;

	if (this)
	{
		this->running = false;
		pthread_join(this->tid[LRF_THREAD_UART], NULL);
		pthread_mutex_destroy(&this->mtx[0]);
		pthread_mutex_destroy(&this->mtx[1]);
		pthread_cond_destroy(&this->cond);
		lrf_deinit_gpios(lrf);
		uart_destroy(this->uart);;
		free(this);
		TLOGMSG(1, ("destroy lrf interface\n"));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
	}

	return ret;
}
