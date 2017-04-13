/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    lrf.c
        external/internal function implementations of LRF interface
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <sys/poll.h>

#include "core/logger.h"
#include "etc/util.h"
#include "modules/gpio.h"
#include "modules/ircam.h"
#include "modules/irshtr.h"
#include "modules/uart.h"
#include "etc/devconf.h"

/* macro defines : ircam message offset */
#define IRCAM_MSG_OFFSET_SOM    0
#define IRCAM_MSG_OFFSET_MID    1
#define IRCAM_MSG_OFFSET_CMD    2
#define IRCAM_MSG_OFFSET_PARAM0 3
#define IRCAM_MSG_OFFSET_PARAM1 4
#define IRCAM_MSG_OFFSET_CHKSUM 5
#define IRCAM_MSG_OFFSET_EOM    6

/* macro defines : ircam message */
#define IRCAM_MSG_MID           0xE1
#define IRCAM_MSG_LEN           7
#define IRCAM_MSG_SOM           0x7E
#define IRCAM_MSG_EOM           0xC1

/* macro defines : ircam commands */
#define IRCAM_MSG_CMD_NUC       0x43
#define IRCAM_MSG_CMD_IRPOL     0x57
#define IRCAM_MSG_CMD_CONTRAST  0x56
#define IRCAM_MSG_CMD_BRIGHT    0x42
#define IRCAM_MSG_CMD_EDGE      0x6A
#define IRCAM_MSG_CMD_GAIN      0x4A
#define IRCAM_MSG_CMD_LEVEL     0x67
#define IRCAM_MSG_CMD_GAINMODE  0x4B
#define IRCAM_MSG_CMD_HISTEQ    0x74
#define IRCAM_MSG_CMD_COLOR     0x6B
#define IRCAM_MSG_CMD_READY     0x73
#define IRCAM_MSG_CMD_LDTEC     0x54//0x45
#define IRCAM_MSG_CMD_ARRAY		0x79
#define IRCAM_MSG_CMD_MOVE		0x69

/* macro defines : brighteness max, min, default */
#define IRCAM_BRIGHT_MAX        0xFF
#define IRCAM_BRIGHT_MIN        0x00
#define IRCAM_BRIGHT_DEFAULT    0x80

/* macro defines : contrast max, min, default */
#define IRCAM_CONTRAST_MAX      0xFF
#define IRCAM_CONTRAST_MIN      0x00
#define IRCAM_CONTRAST_DEFAULT  0x80

/* macro defines : edge enhancement max, min, default */
#define IRCAM_EDGE_MAX          0x0A
#define IRCAM_EDGE_MIN          0x00
#define IRCAM_EDGE_DEFAULT      0x02

/* macro defines : gain max, min, default */
#define IRCAM_GAIN_MAX          0xCA
#define IRCAM_GAIN_MIN          0x02
#define IRCAM_GAIN_DEFAULT      0x80

/* macro defines : level max, min, default */
#define IRCAM_LEVEL_MAX         0xE6
#define IRCAM_LEVEL_MIN         0x1E
#define IRCAM_LEVEL_DEFAULT     0x80

/* macro defines : communication timeout */
#define IRCAM_COMM_TIMEOUT      3.0
#define IRCAM_BOOT_TIMEOUT      15000

/* macro defines : write message retry limit */
#define IRCAM_WRMSG_RETRY    3


/* structure declaration : IRCAM interface attribute */
struct ircam_attribute
{
    /* external interface */
    struct ircam_interface extif;

    /* interface attribute */
    bool running;
    bool waitAck;
    int  bright;
    int  color;
    int  contrast;
    int  edge;
    int  error;
    int  gain;
    int  gainMode;
    int  histeq;
    int  irpol;
    int  level;
    int  nuc;
    int  power;
    int  shtrMode;
    int  teclessData;
    int  irhshift;
    int  irvshift;

    pthread_t tid;
    pthread_mutex_t mtx;
    pthread_cond_t cond;

    struct timespec txts;
    struct gpio_interface *gpio;
    struct uart_interface *uart;
    struct irshtr_interface *shtr;
};


static int
ircam_wait_ack(struct ircam_interface *ircam)
{
    int ret = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
    {
        if (!this->waitAck)
        {
            pthread_mutex_lock(&this->mtx);
            this->waitAck = true;

            while(this->waitAck)
                pthread_cond_wait(&this->cond, &this->mtx);

            pthread_mutex_unlock(&this->mtx);
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid waitack flag\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_recv_ack(struct ircam_interface *ircam)
{
    int ret = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
    {
        if (this->waitAck)
        {
            pthread_mutex_lock(&this->mtx);
            this->waitAck = false;
            pthread_mutex_unlock(&this->mtx);
            pthread_cond_signal(&this->cond);
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid waitack flag\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_parse_message(struct ircam_interface *ircam, char *msg)
{
    int ret = 0;
    int param = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (ircam)
    {
        param = *(msg + IRCAM_MSG_OFFSET_PARAM1);

        switch(*(msg + IRCAM_MSG_OFFSET_CMD))
        {
        case IRCAM_MSG_CMD_NUC:
            break;

        case IRCAM_MSG_CMD_IRPOL:
            if ((param == IRCAM_IRPOL_BLACK) || (param == IRCAM_IRPOL_WHITE))
                this->irpol = param;
            else
                TLOGMSG(1, (DBGINFOFMT "invalid param for irpol\n", DBGINFO));
            break;

        case IRCAM_MSG_CMD_CONTRAST:
            this->contrast = param;
            break;

        case IRCAM_MSG_CMD_BRIGHT:
            this->bright = param;
            break;

        case IRCAM_MSG_CMD_EDGE:
            if ((param >= IRCAM_EDGE_MIN) && (param <= IRCAM_EDGE_MAX))
                this->edge = param;
            else
                TLOGMSG(1, (DBGINFOFMT "invalid param for edge enhancement\n", DBGINFO));
            break;

        case IRCAM_MSG_CMD_GAIN:
            if ((param >= IRCAM_GAIN_MIN) && (param <= IRCAM_GAIN_MAX))
                this->gain = param;
            else
                TLOGMSG(1, (DBGINFOFMT "invalid param for gain\n", DBGINFO));
            break;

        case IRCAM_MSG_CMD_LEVEL:
            if ((param >= IRCAM_LEVEL_MIN) && (param <= IRCAM_LEVEL_MAX))
                this->level = param;
            else
                TLOGMSG(1, (DBGINFOFMT "invalid param for level\n", DBGINFO));
            break;

        case IRCAM_MSG_CMD_GAINMODE:
            if ((param == IRCAM_GAINMODE_DEFAULT) || (param == IRCAM_GAINMODE_HIGH))
                this->gainMode = param;
            else
                TLOGMSG(1, (DBGINFOFMT "invalid param for gain mode\n", DBGINFO));
            break;

        case IRCAM_MSG_CMD_HISTEQ:
            if ((param == IRCAM_HISTEQ_LOCAL) || (param == IRCAM_HISTEQ_GLOBAL))
                this->histeq = param;
            else
                TLOGMSG(1, (DBGINFOFMT "invalid param for histeq\n", DBGINFO));
            break;

        case IRCAM_MSG_CMD_COLOR:
            if ((param == IRCAM_COLOR_MONO) || (param == IRCAM_COLOR_SEPIA) || (param == IRCAM_COLOR_SPECTRUM) || (param == IRCAM_COLOR_ISOTHREM))
                this->color = param;
            else
                TLOGMSG(1, (DBGINFOFMT "invalid param for ir color\n", DBGINFO));
            break;

        case IRCAM_MSG_CMD_LDTEC:
            if ((param == IRCAM_TECDATA_LOW) || (param == IRCAM_TECDATA_NORMAL) || (param == IRCAM_TECDATA_HIGH))
                this->teclessData = param;
            else
                TLOGMSG(1, (DBGINFOFMT "invalid param for tecless data\n", DBGINFO));
            break;

        case IRCAM_MSG_CMD_READY:
            TLOGMSG(1, ("IRCAM_MSG_CMD_READY\n"));

            if (this->power == IRCAM_POWER_BOOTING)
                this->power = IRCAM_POWER_ENABLE;

            if (param & 0x01)
                this->error &= ~(IRCAM_ERROR_BOOT);
            else
                this->error |= (IRCAM_ERROR_BOOT);

            if (param & 0x02)
                this->error &= (~IRCAM_ERROR_DETECTOR);
            else
                this->error |= (IRCAM_ERROR_DETECTOR);

            break;

        case IRCAM_MSG_CMD_ARRAY:
        	break;
        case IRCAM_MSG_CMD_MOVE:
        	this->irhshift = *(msg + IRCAM_MSG_OFFSET_PARAM0);
        	this->irvshift = param;
        	 break;

        default:
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "unknown ircam message: %d\n", DBGINFO, *(msg + IRCAM_MSG_OFFSET_CMD) ));
            break;
        }
        TLOGMSG(1, (DBGINFOFMT "recv ircam message: %x %x %x\n", DBGINFO,
                			 *(msg + IRCAM_MSG_OFFSET_CMD), *(msg + IRCAM_MSG_OFFSET_PARAM0), *(msg + IRCAM_MSG_OFFSET_PARAM1)));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n"));
    }

    return ret;
}


static int
ircam_write_message(struct ircam_interface *ircam, char cmd, char parm_0, char parm_1)
{
    int ret = 0;
    int nwr = 0;
    int count = 0;
    char msg[8] = {0};
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
    {
        memset(msg, 0x00, sizeof(msg));
        msg[0] = IRCAM_MSG_SOM;
        msg[1] = IRCAM_MSG_MID;
        msg[2] = cmd;
        msg[3] = parm_0;
        msg[4] = parm_1;
        msg[5] = (char) (0x11 + parm_0 + parm_1);
        msg[6] = IRCAM_MSG_EOM;

        TLOGMSG(1, (DBGINFOFMT "send ircam message: %x %x %x\n", DBGINFO,
                			cmd, parm_0, parm_1));
        while(true)
        {
            nwr = this->uart->write(this->uart, msg, IRCAM_MSG_LEN);

            if (nwr == -1)
            {
                TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));

                if (count < IRCAM_WRMSG_RETRY)
                {
                    count = count + 1;
                    TLOGMSG(1, (DBGINFOFMT "retry write message\n", DBGINFO));
                    continue;
                }
                else
                {
                    ret = -1;
                    this->error |= IRCAM_ERROR_COMM;
                    TLOGMSG(1, (DBGINFOFMT "write message retry limit exceeded\n", DBGINFO));
                    break;
                }
            }
            else
            {
                this->error &= (~IRCAM_ERROR_COMM);
                clock_gettime(CLOCK_REALTIME, &this->txts);
                break;
            }
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static void *
ircam_read_message(void *arg)
{
    int nread = 0;
    int offset = 0;

    char msg[8] = {0};
    char buf[64] = {0};
    struct ircam_interface *ircam = (struct ircam_interface *) arg;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;
    struct uart_interface *uart = this->uart;
    struct pollfd pfd = {.fd = uart->getFileDesc(uart), .events = POLLIN, .revents = 0};
    struct timespec tstamp = {.tv_sec = 0, .tv_nsec = 0};

    memset(msg, 0x00, sizeof(msg));
    memset(buf, 0x00, sizeof(buf));
    this->running = true;

    while(this->running)
    {
        if (poll(&pfd, 1, 1000) > 0)
        {
            nread = uart->read(uart, buf, uart->getNumRead(uart));

            for (int i = 0; i < nread; i++)
            {
                switch (buf[i])
                {
                case IRCAM_MSG_SOM:
                    if (msg[0] != IRCAM_MSG_SOM)
                        clock_gettime(CLOCK_REALTIME, &tstamp);

                    msg[offset++] = buf[i];
                    break;

                case IRCAM_MSG_EOM:
                    if (msg[0] == IRCAM_MSG_SOM)
                    {
                        if (offset == IRCAM_MSG_OFFSET_EOM)
                        {
                            msg[offset++] = buf[i];
                            ircam_parse_message(ircam, msg);
                            ircam_recv_ack(ircam);
                            memset(msg, 0x00, sizeof(msg));
                            offset = 0;
                        }
                        else
                            msg[offset++] = buf[i];
                    }

                    break;

                default:
                    if (msg[0] == IRCAM_MSG_SOM)
                        msg[offset++] = buf[i];
                    break;
                }
            }

            memset(buf, 0x00, sizeof(buf));
        }
        else
        {
            if (this->waitAck)
            {
                if (check_timeout(&this->txts, IRCAM_COMM_TIMEOUT) != 0)
                {
                    ircam_recv_ack(ircam);
                    memset(buf, 0x00, sizeof(buf));
                    memset(msg, 0x00, sizeof(msg));
                    offset = 0;
                }
                else
                    continue;
            }
            else
            {
                if ((msg[0] == IRCAM_MSG_SOM) && (check_timeout(&tstamp, IRCAM_COMM_TIMEOUT) != 0))
                {
                    ircam_recv_ack(ircam);
                    memset(buf, 0x00, sizeof(buf));
                    memset(msg, 0x00, sizeof(msg));
                    offset = 0;
                }
                else
                    continue;
            }
        }
    }

    return NULL;
}


static int
ircam_update_nuc(struct ircam_interface *ircam, int flag)
{
    int ret = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
    {
        this->nuc = IRCAM_NUC_INPROC;

        if (flag == IRCAM_INTERNAL_SHUTTER)
            this->shtr->close(this->shtr);

        if (ircam_write_message(ircam, IRCAM_MSG_CMD_NUC, 0, 0) == 0)
        {
            ircam_wait_ack(ircam);

            if (this->error == IRCAM_ERROR_NONE)
                TLOGMSG(1, ("nuc update\n"));
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "error (0x%02X)\n", DBGINFO, this->error));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to write message\n"));
        }

        if (flag == IRCAM_INTERNAL_SHUTTER)
            this->shtr->open(this->shtr);

        this->nuc = IRCAM_NUC_IDLE;
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n"));
    }

    return ret;
}


static int
ircam_set_irpol(struct ircam_interface *ircam, int irpol)
{
    int ret = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
    {
        if ((irpol == IRCAM_IRPOL_WHITE) || (irpol == IRCAM_IRPOL_BLACK))
        {
            if (ircam_write_message(ircam, IRCAM_MSG_CMD_IRPOL, 0x00, (char) irpol) == 0)
            {
                ircam_wait_ack(ircam);

                if (this->error == IRCAM_ERROR_NONE)
                    TLOGMSG(1, ("ir polarity = %s hot\n", this->irpol == IRCAM_IRPOL_WHITE ? "white" : "black"));
                else
                {
                    ret = -1;
                    TLOGMSG(1, (DBGINFOFMT "error (0x%02X)\n", DBGINFO, this->error));
                }
            }
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid irpol value\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_set_contrast(struct ircam_interface *ircam, int val)
{
    int ret = 0;
    char param = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
    {
        if (val > IRCAM_CONTRAST_MAX)
            param = IRCAM_CONTRAST_MAX;
        else if (val < IRCAM_CONTRAST_MIN)
            param = IRCAM_CONTRAST_MIN;
        else
            param = (char) val;

        if (ircam_write_message(ircam, IRCAM_MSG_CMD_CONTRAST, 0x00, param) == 0)
        {
            ircam_wait_ack(ircam);

            if (this->error == IRCAM_ERROR_NONE)
                TLOGMSG(1, ("ir contrast = %d\n", this->contrast));
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "error (0x%02X)\n", DBGINFO, this->error));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_set_brightness(struct ircam_interface *ircam, int val)
{
    int ret = 0;
    char param = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
    {
        if (val > IRCAM_BRIGHT_MAX)
            param = IRCAM_BRIGHT_MAX;
        else if (val < IRCAM_BRIGHT_MIN)
            param = IRCAM_BRIGHT_MIN;
        else
            param = (char) val;

        if (ircam_write_message(ircam, IRCAM_MSG_CMD_BRIGHT, 0x00, param) == 0)
        {
            ircam_wait_ack(ircam);

            if (this->error == IRCAM_ERROR_NONE)
                TLOGMSG(1, ("ir brightness = %d\n", this->bright));
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "error (0x%02X)\n", DBGINFO, this->error));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_set_edge_enhancement(struct ircam_interface *ircam, int val)
{
    int ret = 0;
    char param = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
    {
        if (val > IRCAM_EDGE_MAX)
            param = IRCAM_EDGE_MAX;
        else if (val < IRCAM_EDGE_MIN)
            param = IRCAM_EDGE_MIN;
        else
            param = (char) val;

        if (ircam_write_message(ircam, IRCAM_MSG_CMD_EDGE, 0x00, param) == 0)
        {
            ircam_wait_ack(ircam);

            if (this->error == IRCAM_ERROR_NONE)
                TLOGMSG(1, ("ir edge enhancement = %d\n", this->edge));
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "error (0x%02X)\n", DBGINFO, this->error));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_set_gain(struct ircam_interface *ircam, int val)
{
    int ret = 0;
    char param = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
    {
        if (val > IRCAM_GAIN_MAX)
            param = IRCAM_GAIN_MAX;
        else if (val < IRCAM_GAIN_MIN)
            param = IRCAM_GAIN_MIN;
        else
            param = (char) val;

        if (ircam_write_message(ircam, IRCAM_MSG_CMD_GAIN, 0x00, param) == 0)
        {
            ircam_wait_ack(ircam);

            if (this->error == IRCAM_ERROR_NONE)
                TLOGMSG(1, ("ircam gain = %d\n", this->gain));
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "error (0x%02X)\n", DBGINFO, this->error));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_set_level(struct ircam_interface *ircam, int val)
{
    int ret = 0;
    char param = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
    {
        if (val > IRCAM_LEVEL_MAX)
            param = IRCAM_LEVEL_MAX;
        else if (val < IRCAM_LEVEL_MIN)
            param = IRCAM_LEVEL_MIN;
        else
            param = (char) val;

        if (ircam_write_message(ircam, IRCAM_MSG_CMD_LEVEL, 0x00, param) == 0)
        {
            ircam_wait_ack(ircam);

            if (this->error == IRCAM_ERROR_NONE)
                TLOGMSG(1, ("ircam level = %d\n", this->level));
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "error (0x%02X)\n", DBGINFO, this->error));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_set_color(struct ircam_interface *ircam, int val)
{
    int ret = 0;
    char param = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
    {
        if ((val == IRCAM_COLOR_MONO) || (val == IRCAM_COLOR_SEPIA) || (val == IRCAM_COLOR_SPECTRUM) || (val == IRCAM_COLOR_ISOTHREM))
            param = (char) val;
        else
            param = (char) this->color;

        if (ircam_write_message(ircam, IRCAM_MSG_CMD_COLOR, 0x00, param) == 0)
        {
            ircam_wait_ack(ircam);

            if (this->error == IRCAM_ERROR_NONE)
                TLOGMSG(1, ("ir color = %d\n", this->color));
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "error (0x%02X)\n", DBGINFO, this->error));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_set_gainmode(struct ircam_interface *ircam, int mode)
{
    int ret = 0;
    char param = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
    {
        if ((mode == IRCAM_GAINMODE_HIGH) || (mode == IRCAM_GAINMODE_DEFAULT))
            param = (char) mode;
        else
            param = (char) this->gainMode;

        if (ircam_write_message(ircam, IRCAM_MSG_CMD_GAINMODE, 0x00, param) == 0)
        {
            ircam_wait_ack(ircam);

            if (this->error == IRCAM_ERROR_NONE)
                TLOGMSG(1, ("iracam gain mode = %s\n", this->gainMode == IRCAM_GAINMODE_DEFAULT ? "default" : "high"));
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "error (0x%02X)\n", DBGINFO, this->error));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_set_histeq(struct ircam_interface *ircam, int mode)
{
    int ret = 0;
    char param = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
    {
        if ((mode == IRCAM_HISTEQ_LOCAL) || (mode == IRCAM_HISTEQ_GLOBAL))
            param = (char) mode;
        else
            param = (char) this->histeq;

        if (ircam_write_message(ircam, IRCAM_MSG_CMD_HISTEQ, 0x00, param) == 0)
        {
            ircam_wait_ack(ircam);

            if (this->error == IRCAM_ERROR_NONE)
                TLOGMSG(1, ("ircam heq mode = %s\n", this->histeq == IRCAM_HISTEQ_LOCAL ? "local" : "global"));
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "error (0x%02X)\n", DBGINFO, this->error));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_set_shutter_mode(struct ircam_interface *ircam, int flag)
{
    int ret = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
    {
        this->shtrMode = flag;
        TLOGMSG(1, ("use %s shutter for 1p-nuc\n",
                (flag == IRCAM_INTERNAL_SHUTTER) ? "internal": "external"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_set_nuc_ready(struct ircam_interface *ircam)
{
    int ret = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
    {
        this->nuc = IRCAM_NUC_READY;
        TLOGMSG(1, ("ready for manual nuc\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_load_tecdata(struct ircam_interface *ircam, int idx)
{
    int ret = 0;
    char param = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
    {
        if ((idx == IRCAM_TECDATA_LOW) || (idx == IRCAM_TECDATA_HIGH) || (idx == IRCAM_TECDATA_NORMAL))
            param = (char) idx;
        else
            param = (char) this->teclessData;

        if (ircam_write_message(ircam, IRCAM_MSG_CMD_LDTEC, 0x00, param) == 0)
        {
            ircam_wait_ack(ircam);

            if (this->error == IRCAM_ERROR_NONE)
                TLOGMSG(1, ("load ircam tecless data #%d\n", this->teclessData));
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "error (0x%02X)\n", DBGINFO, this->error));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_get_nuc_status(struct ircam_interface *ircam)
{
    int ret = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
        ret = this->nuc;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_get_irpol(struct ircam_interface *ircam)
{
    int ret = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
        ret = this->irpol;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_get_contrast(struct ircam_interface *ircam)
{
    int ret = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
        ret = this->contrast;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_get_brightness(struct ircam_interface *ircam)
{
    int ret = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
        ret = this->bright;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_get_edge_enhancement(struct ircam_interface *ircam)
{
    int ret = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
        ret = this->edge;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_get_gain(struct ircam_interface *ircam)
{
    int ret = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
        ret = this->gain;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_get_level(struct ircam_interface *ircam)
{
    int ret = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
        ret = this->level;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_get_color(struct ircam_interface *ircam)
{
    int ret = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
        ret = this->color;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_get_gainmode(struct ircam_interface *ircam)
{
    int ret = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
        ret = this->gainMode;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_get_histeq(struct ircam_interface *ircam)
{
    int ret = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
        ret = this->histeq;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_get_tecdata(struct ircam_interface *ircam)
{
    int ret = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
        ret = this->teclessData;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_get_shutter_mode(struct ircam_interface *ircam)
{
    int ret = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
        ret = this->shtrMode;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_enable_power(struct ircam_interface *ircam, bool flag)
{
    int ret = 0;
    int count = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
    {
        if (flag)
        {
            if (this->power != IRCAM_POWER_ENABLE)
            {
                this->gpio->setVal(this->gpio, GPIO_VALUE_HIGH);
                this->power = IRCAM_POWER_BOOTING;

                while (true)
                {
                    if (this->power == IRCAM_POWER_ENABLE)
                    {
                        TLOGMSG(1, ("enable ircam power\n"));
                        break;
                    }
                    else
                    {
                        if (count < IRCAM_BOOT_TIMEOUT)
                        {
                            count++;
                            MSLEEP(1);
                        }
                        else
                        {
                            this->gpio->setVal(this->gpio, GPIO_VALUE_LOW);
                            this->power = IRCAM_POWER_DISABLE;
                            this->error = IRCAM_ERROR_BOOT;
                            TLOGMSG(1, (DBGINFOFMT "failed to enable power\n", DBGINFO));
                            break;
                        }
                    }
                }
            }
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "already in power on state\n", DBGINFO));
            }
        }
        else
        {
            this->gpio->setVal(this->gpio, GPIO_VALUE_LOW);
            this->power = IRCAM_POWER_DISABLE;
            TLOGMSG(1, ("disable ircam power\n"));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_test_module(struct ircam_interface *ircam)
{
    int ret = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
    {
        if (this->power == IRCAM_POWER_DISABLE)
            ircam_enable_power(ircam, true);

        if (this->shtr->testModule(this->shtr) != 0)
            this->error |= IRCAM_ERROR_IRSHTR;

        ret = this->error;
        TLOGMSG(1, ("ircam test result = 0x%02X\n", ret));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_get_error(struct ircam_interface *ircam)
{
    int ret = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
        ret = this->error;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_init_attribute(struct ircam_interface *ircam)
{
    int ret = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
    {
        this->gpio = gpio_create();

        if (this->gpio)
        {
            this->gpio->export(this->gpio, GPIO4_IRCAM_PWR);
            this->gpio->setDir(this->gpio, GPIO_DIR_OUTPUT);
            this->gpio->setVal(this->gpio, GPIO_VALUE_LOW);

            this->uart = uart_create();

            if (this->uart)
            {
                this->uart->open(this->uart, UART_TTYMXC1, B115200);
                this->shtr = irshtr_create();

                if (this->shtr)
                {
                    this->bright    = IRCAM_BRIGHT_DEFAULT;
                    this->color     = IRCAM_COLOR_MONO;
                    this->contrast  = IRCAM_CONTRAST_DEFAULT;
                    this->edge      = IRCAM_EDGE_DEFAULT;
                    this->error     = IRCAM_ERROR_NONE;
                    this->histeq    = IRCAM_HISTEQ_LOCAL;
                    this->irpol     = IRCAM_IRPOL_WHITE;
                    this->gain      = IRCAM_GAIN_DEFAULT;
                    this->gainMode  = IRCAM_GAINMODE_DEFAULT;
                    this->level     = IRCAM_LEVEL_DEFAULT;
                    this->power     = IRCAM_POWER_DISABLE;
                    this->shtrMode  = IRCAM_INTERNAL_SHUTTER;
                    this->teclessData = IRCAM_TECDATA_NORMAL;
                    this->waitAck   = false;
                }
                else
                {
                    ret = -1;
                    uart_destroy(this->uart);
                    gpio_destroy(this->gpio);
                    TLOGMSG(1, (DBGINFOFMT "failed to create irshtr interface\n", DBGINFO));
                }
            }
            else
            {
                ret = -1;
                gpio_destroy(this->gpio);
                TLOGMSG(1, (DBGINFOFMT "failed to create uart interface\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to create gpio interface\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}


static int
ircam_deinit_attribute(struct ircam_interface *ircam)
{
    int ret = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
    {
        irshtr_destroy(this->shtr);
        uart_destroy(this->uart);
        gpio_destroy(this->gpio);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam attribute\n", DBGINFO));
    }

    return ret;
}


static int
ircam_move(struct ircam_interface *ircam, int direction)
{
	int ret = 0;
	char param = 0;
	struct ircam_attribute *this = (struct ircam_attribute *) ircam;

	if(this)
	{
		param = (char)direction;
		if(ircam_write_message(ircam, IRCAM_MSG_CMD_MOVE, 0x00, param) == 0)
		{
			ircam_wait_ack(ircam);
			if (this->error != IRCAM_ERROR_NONE)
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "error (0x%02X)\n", DBGINFO, this->error));
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
	}

	return ret;
}



static int
ircam_array_mode(struct ircam_interface *ircam, int mode)
{
	int ret = 0;
	char param = 0;
	char irh[16] = {0};
	char irv[16] = {0};
	struct ircam_attribute *this = (struct ircam_attribute *) ircam;

	memset(irh, 0x00, sizeof(irh));
	memset(irv, 0x00, sizeof(irv));

	if(this)
	{
		param = (char)mode;
		if(ircam_write_message(ircam, IRCAM_MSG_CMD_ARRAY, 0x00, mode) == 0)
		{
			ircam_wait_ack(ircam);
			if (this->error != IRCAM_ERROR_NONE)
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "error (0x%02X)\n", DBGINFO, this->error));
			}
			else
			{
				if(mode == IRCAM_ARRAY_SAVEEND)
				{
					snprintf(irh, sizeof(irh), "%d", this->irhshift);
					snprintf(irv, sizeof(irv), "%d", this->irvshift);
					devconf_set_value(DEVCONF_KEY_IRHSHIFT, irh);
					devconf_set_value(DEVCONF_KEY_IRVSHIFT, irv);
					devconf_save_parameters(DEVCONF_FILE_PATH);
				}
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
	}

	return ret;
}


/* external functions create/destroy IRCAM interface */
struct ircam_interface *
ircam_create(void)
{
    struct ircam_interface *ircam = NULL;
    struct ircam_attribute *this = malloc(sizeof(struct ircam_attribute));

    if (this)
    {
        memset(this, 0x00, sizeof(struct ircam_attribute));
        ircam = &(this->extif);

        this->irhshift = 30;
        this->irvshift = 30;
        ircam->enablePower   = ircam_enable_power;
        ircam->testModule    = ircam_test_module;
        ircam->getError      = ircam_get_error;
        ircam->updateNuc     = ircam_update_nuc;
        ircam->setEdge       = ircam_set_edge_enhancement;
        ircam->setTecdata    = ircam_load_tecdata;
        ircam->setGain       = ircam_set_gain;
        ircam->setColor      = ircam_set_color;
        ircam->setPolarity   = ircam_set_irpol;
        ircam->setLevel      = ircam_set_level;
        ircam->setGainMode   = ircam_set_gainmode;
        ircam->setHistEq     = ircam_set_histeq;
        ircam->setBright     = ircam_set_brightness;
        ircam->setContrast   = ircam_set_contrast;
        ircam->setShtrMode   = ircam_set_shutter_mode;
        ircam->setNucReady   = ircam_set_nuc_ready;
        ircam->getNucStatus  = ircam_get_nuc_status;
        ircam->getEdge       = ircam_get_edge_enhancement;
        ircam->getTecdata    = ircam_get_tecdata;
        ircam->getGain       = ircam_get_gain;
        ircam->getColor      = ircam_get_color;
        ircam->getPolarity   = ircam_get_irpol;
        ircam->getLevel      = ircam_get_level;
        ircam->getGainMode   = ircam_get_gainmode;
        ircam->getHistEq     = ircam_get_histeq;
        ircam->getBright     = ircam_get_brightness;
        ircam->getContrast   = ircam_get_contrast;
        ircam->getShtrMode   = ircam_get_shutter_mode;
        ircam->moveIr		 = ircam_move;
        ircam->arrayIr		 = ircam_array_mode;

        if (ircam_init_attribute(ircam) == 0)
        {
            pthread_mutex_init(&this->mtx, NULL);
            pthread_cond_init(&this->cond, NULL);

            if (pthread_create(&this->tid, NULL, ircam_read_message, (void *)ircam) == 0)
                TLOGMSG(1, ("init ircam interface\n"));
            else
            {
                pthread_cond_destroy(&this->cond);
                pthread_mutex_destroy(&this->mtx);
                ircam_deinit_attribute(ircam);
                free(this);
                ircam = NULL;
                TLOGMSG(1, (DBGINFOFMT "failed to create ircam interface, failed to create thread\n", DBGINFO));
            }
        }
        else
        {
            free(this);
            ircam = NULL;
            TLOGMSG(1, (DBGINFOFMT "failed to create ircam interface, failed to init ircam attribute\n", DBGINFO));
        }
    }
    else
    {
        ircam = NULL;
        TLOGMSG(1, (DBGINFOFMT "failed to create ircam interface, malloc return null\n", DBGINFO));
    }

    return ircam;
}


int
ircam_destroy(struct ircam_interface *ircam)
{
    int ret = 0;
    struct ircam_attribute *this = (struct ircam_attribute *) ircam;

    if (this)
    {
        this->running = false;
        pthread_join(this->tid, NULL);
        pthread_mutex_destroy(&this->mtx);
        pthread_cond_destroy(&this->cond);
        ircam_deinit_attribute(ircam);
        free(this);
        TLOGMSG(1, ("destroy ircam interface\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ircam interface\n", DBGINFO));
    }

    return ret;
}
