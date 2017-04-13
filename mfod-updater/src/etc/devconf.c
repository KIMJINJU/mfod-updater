#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "etc/devconf.h"
#include "etc/util.h"
#include "core/logger.h"

#define VALUE_TYPE_FLOAT        0
#define VALUE_TYPE_INTEGER      1

struct parameter_table
{
    char *key;
    char value[DEVCONF_VALUE_LENGTH];
}
parmtlb[] =
{
    { .key = DEVCONF_KEY_SRCCA    , .value = {0} 	},
    { .key = DEVCONF_KEY_SRCCB    , .value = {0} 	},
    { .key = DEVCONF_KEY_MRCCA    , .value = {0}	},
    { .key = DEVCONF_KEY_MRCCB    , .value = {0}    },
    { .key = DEVCONF_KEY_LRCCA    , .value = {0}    },
    { .key = DEVCONF_KEY_LRCCB    , .value = {0}    },
    { .key = DEVCONF_KEY_AZOFF    , .value = {0}    },
    { .key = DEVCONF_KEY_ELOFF    , .value = {0}	},
    { .key = DEVCONF_KEY_IRHSHIFT , .value = {0} 	},
    { .key = DEVCONF_KEY_IRVSHIFT , .value = {0} 	},
    { .key = DEVCONF_KEY_IRUPSCALE, .value = {0} 	},
    { .key = DEVCONF_KEY_PROXOPEN , .value = {0} 	},
    { .key = DEVCONF_KEY_PROXCLOSE, .value = {0} 	},
    { .key = DEVCONF_KEY_COORDSYS , .value = {0} 	}
};


void devconf_enumerate_parameters(void)
{
    TLOGMSG(1, ( "==================== device parameters ====================\n"));

    for (int i = 0; i < DIM(parmtlb); i++)
        TLOGMSG(1, ("%s = %s \n", parmtlb[i].key, parmtlb[i].value));

    TLOGMSG(1, ( "===========================================================\n\n"));
}


void devconf_reset_parameters(void)
{
    for (int i = 0; i < DIM(parmtlb); i++)
    {
        memset(parmtlb[i].value, 0x00, DEVCONF_VALUE_LENGTH);
        snprintf(parmtlb[i].value, DEVCONF_VALUE_LENGTH, "0");
    }
}


int devconf_load_parameters(char *path)
{
    int ret = 0;
    int nwr = 0;
    FILE *file = NULL;
    char *line = NULL;
    char *value = NULL;
    char buf[DEVCONF_VALUE_LENGTH] = {0};
    size_t length = 0;
    ssize_t nread = 0;

    if ((file = fopen(path, "r")) != NULL)
    {
        while ((nread = getline(&line, &length, file)) != -1)
        {
            for (int i = 0; i < DIM(parmtlb); i++)
            {
                if (strstr(line, parmtlb[i].key) && ((value = strchr(line, '=')) != NULL))
                {
                    while (isdigit(*value) == 0)
                    {
                        if ((*value == '+') || (*value == '-'))
                            break;
                        else
                            value++;
                    }

                    memset(buf, 0x00, sizeof(buf));
                    nwr = snprintf(buf, sizeof(buf), "%s", value);
                    memset(parmtlb[i].value, 0x00, DEVCONF_VALUE_LENGTH);
                    memcpy(parmtlb[i].value, buf, nwr - 1);
                }
                else
                    continue;
            }
        }

        free(line);
        fclose(file);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "fopen return null, %s\n", DBGINFO, strerror(errno)));
    }

    return ret;
}


int devconf_save_parameters(char *path)
{
    int fd = 0;
    int ret = 0;

    if ((fd = open(path, O_RDWR | O_CREAT)) != -1)
    {
        for(int i = 0; i < DIM(parmtlb); i++)
            dprintf(fd, "%s = %s\n", parmtlb[i].key, parmtlb[i].value);

        close(fd);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "open return failed, %s\n", DBGINFO, strerror(errno)));
    }

    return ret;
}


int devconf_set_value(char *key, char *value)
{
    int ret = -1;

    for (int i = 0; i < DIM(parmtlb); i++)
    {
        if (strcmp(key, parmtlb[i].key) == 0)
        {
            ret = 0;
            memset(parmtlb[i].value, 0x00, DEVCONF_VALUE_LENGTH);
            snprintf(parmtlb[i].value, DEVCONF_VALUE_LENGTH, "%s", value);
            break;
        }
    }

    return ret;
}


char* devconf_get_value(char *key)
{
    char *value = NULL;

    for (int i = 0; i < DIM(parmtlb); i++)
    {
        if (strcmp(key, parmtlb[i].key) == 0)
        {
            value = &(parmtlb[i].value[0]);
            break;
        }
    }

    return value;
}
