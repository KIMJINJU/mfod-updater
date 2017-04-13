/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    dmc.c
        external/internal function implementations of GNSS interface
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/


#include <errno.h>
#include <math.h>
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
#include "modules/dmc.h"
#include "modules/uart.h"


/* constant macro defines : DATA INTEGRATION TIME DIVISOR */
#define ITIMEDIV_MIN                0x02
#define ITIMEDIV_MAX                0x0E

/* constant macro defines : MEASURING RANGE */
#define MEASRANGE_MIN               0x0002
#define MEASRANGE_MAX               0x7FFF
#define MEASRANGE_DEFAULT           0x1900

#define DMCMSG_LENGTH               256
#define DMCMSG_MEAS_LENGTH          16
#define DMCMSG_STAT_LENGTH          27
#define DMCMSG_EOF                 '\r'

#define DMCACK_NONE                 0
#define DMCACK_MODE_13              1
#define DMCACK_MODE_AC              2
#define DMCACK_REQSTAT              3
#define DMCACK_STORE_MAGPARM        4
#define DMCACK_RECALL_MAGPARM       5
#define DMCACK_WAIT_CALC_MAGPARM    6
#define DMCACK_QCP                  7
#define DMCACK_REQMEAS              8
#define DMCACK_UNKNOWN              -1

#define DMCACK_MODE_13_LENGTH           16
#define DMCACK_REQSTAT_LENGTH           27
#define DMCACK_STORE_MAGPARM_LENGTH     5
#define DMCACK_RECALL_MAGPARM_LENGTH    5
#define DMAACK_WAIT_CALC_MAGPARM_LENGTH 5
#define DMCACK_QCP_LENGTH               6

#define ACCBUFLEN                   256

/* structure declaration : dmc attribute */
struct dmc_attribute
{
    /* external interface */
    struct dmc_interface extif;

    /* internal attribute*/
    bool standby;
    bool running;
    bool measure;
    bool accumulation;
    bool compensation;

    int waitAck;
    int error;
    int ncp;
    int fom;
    int azimuth;
    int elevation;
    int bank;
    int magparm;
    int accIndex;
    int accBuffer[3][ACCBUFLEN];

    pthread_t       tid;
    pthread_mutex_t mtx;
    pthread_cond_t  cond;

    struct timespec timeStamp;
    struct uart_interface *uart;
};


static int
dmc_write_message(struct dmc_interface *dmc, char *cmd, int cmdlen)
{
    int cnt = 0;
    int nwr = 0;
    int ret = 0;
    struct uart_interface *uart = NULL;
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
retry:
        uart = this->uart;
        nwr = uart->write(uart, cmd, cmdlen);

        if (nwr < 0)
        {
            if (cnt < 3)
            {
                cnt++;
                goto retry;
            }
            else
            {
                ret = -1;
                this->error |= DMC_ERROR_COMM;
                TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
            }
        }
        else
        {
            clock_gettime(CLOCK_REALTIME, &this->timeStamp);
            MSLEEP(10);
            TLOGMSG(0, ("transmit message"));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_set_coding(struct dmc_interface *dmc)
{
    int ret = 0;
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        if (dmc_write_message(dmc, "S82\r", 4) == 0)
            TLOGMSG(1, ("set hexadecimal coding\n"));
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "dmc_write_message return fail\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_set_itime(struct dmc_interface *dmc, unsigned char itime)
{
    int nwr = 0;
    int ret = 0;
    char cmd[8] = {0};
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        if ((itime >= DMC_ITIME_MIN) && (itime <= DMC_ITIME_MAX))
        {
            memset(cmd, 0x00, sizeof(cmd));
            nwr = snprintf(cmd, sizeof(cmd), "S4%02X\r", itime);

            if (dmc_write_message(dmc, cmd, nwr) == 0)
                TLOGMSG(1, ("dmc data integration time = %02X\n", itime));
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "dmc_write_message return fail\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid value for integration time\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_set_itimediv(struct dmc_interface *dmc, unsigned char itimediv)
{
    int ret = 0;
    int nwr = 0;
    char cmd[8] = {0};
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        if ((itimediv >= ITIMEDIV_MIN) && (itimediv <= ITIMEDIV_MAX))
        {
            memset(cmd, 0x00, sizeof(cmd));
            nwr = snprintf(cmd, sizeof(cmd), "S*4%02X\r", itimediv);

            if (dmc_write_message(dmc, cmd, nwr) == 0)
                TLOGMSG(1, ("dmc data integration time divisor = %02X\n", itimediv));
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "dmc_write_message return fail\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid value for integraion time divisor\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_set_measrange(struct dmc_interface *dmc, unsigned short range)
{
    int nwr = 0;
    int ret = 0;
    char cmd[16] = {0};
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        if ((range >= MEASRANGE_MIN) && (range <= MEASRANGE_MAX))
        {
            memset(cmd, 0x00, sizeof(cmd));
            nwr = snprintf(cmd, sizeof(cmd), "S3%04X\r", range);

            if (dmc_write_message(dmc, cmd, nwr) == 0)
                TLOGMSG(1, ("angular measuring range = %04X\n", range));
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "dmc_write_message return fail\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid value for measuring range\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_set_los(struct dmc_interface *dmc, int horz, int vert)
{
    int nwr = 0;
    int ret = 0;
    char cmd[8] = {0};
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        if ((horz >= 1) && (horz <= 6))
        {
            if ((vert != horz) && (vert >= 1 ) && (vert <= 6))
            {
                memset(cmd, 0x00, sizeof(cmd));
                nwr = snprintf(cmd, sizeof(cmd), "SLD%d%d\r", horz, vert);

                if (dmc_write_message(dmc, cmd, nwr) == 0)
                    TLOGMSG(1, ("dmc line of sight = (%d, %d)\n", horz, vert));
                else
                {
                    ret = -1;
                    TLOGMSG(1, (DBGINFOFMT "dmc_write_message return fail\n", DBGINFO));
                }
            }
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "invalid argument for los orientation\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid argument for los orientation\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_enable_gimbal(struct dmc_interface *dmc, bool flag)
{
    int nwr = 0;
    int ret = 0;
    char cmd[8] = {0};
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        memset(cmd, 0x00, sizeof(cmd));
        nwr = snprintf(cmd, sizeof(cmd), "S6%d\r", flag == true ? 1 : 2);

        if (dmc_write_message(dmc, cmd, nwr) == 0)
            TLOGMSG(1, ("dmc gimbal = %s\n", flag == true ? "enable" : "disable"));
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "dmc_write_message return fail\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_set_damping(struct dmc_interface *dmc, unsigned char damping)
{
    int nwr = 0;
    int ret = 0;
    char cmd[8] = {0};
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        memset(cmd, 0x00, sizeof(cmd));
        nwr = snprintf(cmd, sizeof(cmd), "S5%02X\r", damping);

        if (dmc_write_message(dmc, cmd, nwr) == 0)
            TLOGMSG(1, ("dmc damping factor = %02X\n", damping));
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "dmc_write_message return fail\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_start_continuous_measure(struct dmc_interface *dmc)
{
    int ret = 0;
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        if (dmc_write_message(dmc, "S91\r", 4) == 0)
        {
            this->measure = true;
            TLOGMSG(1, ("start continuous angular measuring\n"));
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "dmc_write_message return fail\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static void
dmc_wait_ack(struct dmc_interface *dmc, int ack)
{
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this->waitAck == false)
    {
        pthread_mutex_lock(&this->mtx);

        this->waitAck = ack;

        while(this->waitAck)
            pthread_cond_wait(&this->cond, &this->mtx);

        pthread_mutex_unlock(&this->mtx);
    }
    else
        TLOGMSG(1, (DBGINFOFMT "dmc interface is waiting for ack from dmc\n", DBGINFO));
}


static void
dmc_recv_ack(struct dmc_interface *dmc)
{
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this->waitAck != 0)
    {
        pthread_mutex_lock(&this->mtx);
        this->waitAck = DMCACK_NONE;
        pthread_mutex_unlock(&this->mtx);
        pthread_cond_signal(&this->cond);
    }
    else
        TLOGMSG(1, (DBGINFOFMT "dmc interface is not waiting for ack\n", DBGINFO));
}


static void
dmc_parse_mode13_data(struct dmc_interface *dmc, char *msg)
{
    char az[8];
    char el[8];
    char bk[8];
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        memset(az, 0x00, sizeof(az));
        memset(el, 0x00, sizeof(el));
        memset(bk, 0x00, sizeof(bk));
        memcpy(az, msg + 1, 4);
        memcpy(el, msg + 5, 4);
        memcpy(bk, msg + 9, 4);
        this->azimuth  = strtol(az, NULL, 16);
        this->elevation = (short) strtol(el, NULL, 16);
        this->bank     = (short) strtol(bk, NULL, 16);

        if (this->accumulation)
        {
            this->accBuffer[0][this->accIndex] = this->azimuth;
            this->accBuffer[1][this->accIndex] = this->elevation;
            this->accBuffer[2][this->accIndex] = this->bank;

            if (this->accIndex < ACCBUFLEN - 1)
                this->accIndex++;
            else
                this->accIndex = 0;

            TLOGMSG(0, ("azimuth = %d, elevation = %d, bank = %d\n", this->azimuth, this->elevation, this->bank));
        }

        if (this->waitAck)
            dmc_recv_ack(dmc);
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
}


static void
dmc_parse_status_data(struct dmc_interface *dmc, char *msg)
{
    char flag = 0;
    char buf[4] = {0};
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        memset(buf, 0x00, sizeof(buf));
        memcpy(buf, msg + 20, 2);
        flag = (char) strtol(buf, NULL, 16);

        if (flag & 0x80)
            this->error &= (~DMC_ERROR_MCPU);
        else
            this->error |= DMC_ERROR_MCPU;

        dmc_recv_ack(dmc);
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null dmc interface pointer\n", DBGINFO));
}


static void
dmc_parse_recall_magparm_ack(struct dmc_interface *dmc, char *msg)
{
    char buf[8] = {0};
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        memset(buf, 0x00, sizeof(buf));
        memcpy(buf, msg + 1, 4);

        if (strstr(buf, "S--"))
        {
            this->error &= (~DMC_ERROR_MAGPARM);
            TLOGMSG(1, ("recalled magnetic compensation parameter\n"));
        }
        else
        {
            this->error |= DMC_ERROR_MAGPARM;
            TLOGMSG(1, ("failed to recall magnetic compensation parameter\n"));
        }

        dmc_recv_ack(dmc);
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
}


static void
dmc_parse_store_magparm_ack(struct dmc_interface *dmc, char *msg)
{
    char buf[8] = {0};
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        memset(buf, 0x00, sizeof(buf));
        memcpy(buf, msg + 1, 4);

        if (strstr(buf, "S--"))
            this->error &= (~DMC_ERROR_MAGPARM);
        else
            this->error |= DMC_ERROR_MAGPARM;

        dmc_recv_ack(dmc);
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
}


static void
dmc_parse_qcp_ack(struct dmc_interface *dmc, char *msg)
{
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        if (*(msg + 4) == 'E')
            this->error |= DMC_ERROR_QCP;
        else
        {
            this->error &= (~DMC_ERROR_QCP);
            this->ncp = (*(msg + 1 ) - 0x30) * 10 + (*(msg + 2) - 0x30);
        }

        dmc_recv_ack(dmc);
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
}


static void
dmc_parse_calc_magparm(struct dmc_interface *dmc, char *msg)
{
    char buf[8] = {0};
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        if (*(msg + 1) == 'S')
        {
            memset(buf, 0x00, sizeof(buf));
            memcpy(buf, msg + 1, 4);

            if (strstr(buf, "xx"))
                this->fom = -1;
            else
                this->fom = (*(msg + 2) - 0x30) * 10 + (*(msg + 3) - 0x30);

        }

        dmc_recv_ack(dmc);
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
}


static void
dmc_parse_message(struct dmc_interface *dmc, int msgid, char *msg)
{
    switch (msgid)
    {
    case DMCACK_MODE_13:
    case DMCACK_REQMEAS:
        dmc_parse_mode13_data(dmc, msg);
        break;

    case DMCACK_REQSTAT:
        dmc_parse_status_data(dmc, msg);
        break;

    case DMCACK_STORE_MAGPARM:
        dmc_parse_store_magparm_ack(dmc, msg);
        break;

    case DMCACK_RECALL_MAGPARM:
        dmc_parse_recall_magparm_ack(dmc, msg);
        break;

    case DMCACK_WAIT_CALC_MAGPARM:
        dmc_parse_calc_magparm(dmc, msg);
        break;

    case DMCACK_QCP:
        dmc_parse_qcp_ack(dmc, msg);
        break;

    default:
        break;
    }
}


static int
dmc_classify_message(struct dmc_interface *dmc, char hdr, int len)
{
    int ret = DMCACK_UNKNOWN;
    struct dmc_attribute *this = (struct dmc_attribute *) dmc;

    struct ackprop
    {
        int hdr;
        int id;
        int length;
    }
    ack[] =
    {
        {'S', DMCACK_MODE_13          , DMCACK_MODE_13_LENGTH           },
        {'S', DMCACK_REQSTAT          , DMCACK_REQSTAT_LENGTH           },
        {'C', DMCACK_STORE_MAGPARM    , DMCACK_STORE_MAGPARM_LENGTH     },
        {'C', DMCACK_RECALL_MAGPARM   , DMCACK_RECALL_MAGPARM_LENGTH    },
        {'C', DMCACK_WAIT_CALC_MAGPARM, DMAACK_WAIT_CALC_MAGPARM_LENGTH },
        {'S', DMCACK_QCP              , DMCACK_QCP_LENGTH               },
        {'S', DMCACK_REQMEAS          , DMCACK_MODE_13_LENGTH           }
    };


    for (int i = 0; i < DIM(ack); i++)
    {
        if ((ack[i].hdr == hdr) && (ack[i].length == len))
        {
            if (this->waitAck == 0)
            {
                ret = ack[i].id;
                break;
            }
            else
            {
                if (this->waitAck == ack[i].id)
                {
                    ret = ack[i].id;
                    break;
                }
                else
                    continue;
            }
        }
        else
            continue;
    }

    return ret;
}


static void *
dmc_read_message(void *arg)
{
    int idx = 0;
    int count = 0;
    int nread = 0;
    bool parse = false;
    char buf[256] = {0};
    char msg[256] = {0};
    struct timespec ts = {.tv_sec = 0, .tv_nsec = 0};
    struct dmc_interface *dmc = (struct dmc_interface *) arg;
    struct dmc_attribute *this = (struct dmc_attribute *) dmc;
    struct uart_interface *uart = this->uart;
    struct pollfd pev = {.fd = uart->getFileDesc(uart), .events = POLLIN, .revents = 0};

    this->running = true;
    tcflush(uart->getFileDesc(uart), TCIOFLUSH);
    memset(buf, 0x00, sizeof(buf));
    memset(msg, 0x00, sizeof(msg));

    while (this->running)
    {
        if (parse == true)
        {
            dmc_parse_message(dmc, dmc_classify_message(dmc, msg[0], idx), msg);
            memset(msg, 0x00, sizeof(msg));
            parse = false;
            idx = 0;
        }
        else
        {
            if (poll(&pev, 1, 1000) > 0)
            {
                nread = uart->getNumRead(uart);

                if (uart->read(uart, buf, nread) != -1)
                {
                    for (int i = 0; i < nread; i++)
                    {
                        if (buf[i] == DMCMSG_EOF)
                            parse = true;

                        msg[idx++] = buf[i];
                    }

                    count = 0;
                    this->error &= (~DMC_ERROR_COMM);
                }
            }
            else
            {
                if (this->waitAck != DMCACK_NONE)
                {
                    clock_gettime(CLOCK_REALTIME, &ts);

                    if (get_elapsed_time(&this->timeStamp, &ts) > 10.0)
                    {
                        this->error |= DMC_ERROR_COMM;
                        dmc_recv_ack(dmc);
                    }
                    else
                        continue;
                }
                else
                {
                    if (this->measure)
                    {
                        if (count < 10)
                            count++;
                        else
                            this->error |= DMC_ERROR_COMM;
                    }
                    else
                        continue;
                }

                idx = 0;
                memset(msg, 0x00, sizeof(msg));
            }
        }
    }

    return NULL;
}


static int
dmc_get_azimuth(struct dmc_interface *dmc, int *mil, double *deg)
{
    int ret = 0;
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        *mil = (int) this->azimuth;
        *deg = MIL2DEG(this->azimuth);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_get_elevation(struct dmc_interface *dmc, int *mil, double *deg)
{
    int ret = 0;
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        *mil = (int) this->elevation;
        *deg = MIL2DEG(*mil);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_get_bank(struct dmc_interface *dmc, int *mil, double *deg)
{
    int ret = 0;
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        *mil = (int) this->bank;
        *deg = MIL2DEG(*mil);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_store_magparm(struct dmc_interface *dmc, int bin)
{
    int ret = 0;
    int nwr = 0;
    char cmd[8] = {0};
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        if ((bin >= MAGPARM_FACTORY) && (bin <= MAGPARM_USER2))
        {
            memset(cmd, 0x00, sizeof(cmd));
            nwr = snprintf(cmd, sizeof(cmd), "S96\rs%d\r", bin);

            if (dmc_write_message(dmc, cmd, nwr) == 0)
            {
                dmc_wait_ack(dmc, DMCACK_STORE_MAGPARM);

                if (this->error == DMC_ERROR_NONE)
                {
                    this->magparm = bin;
                    TLOGMSG(1, ("store magparm at storage #%d\n", bin));
                }
                else
                {
                    ret = -1;

                    if (this->error & DMC_ERROR_MAGPARM)
                        TLOGMSG(1, (DBGINFOFMT "failed to store magparm\n", DBGINFO));

                    if (this->error & DMC_ERROR_COMM)
                        TLOGMSG(1, (DBGINFOFMT "dmc module not response\n", DBGINFO));
                }
            }
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "dmc_write_message return fail\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid magparm index\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_recall_magparm(struct dmc_interface *dmc, int bin)
{
    int nwr = 0;
    int ret = 0;
    char cmd[8];
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        if ((bin >= MAGPARM_FACTORY) && (bin <= MAGPARM_USER2))
        {
            memset(cmd, 0x00, sizeof(cmd));
            nwr = snprintf(cmd, sizeof(cmd), "S96\rr%d\r", bin);

            if (dmc_write_message(dmc, cmd, nwr) == 0)
            {
                dmc_wait_ack(dmc, DMCACK_RECALL_MAGPARM);

                if (this->error == DMC_ERROR_NONE)
                {
                    this->magparm = bin;
                    TLOGMSG(1, ("magnetic compensation parameter = %d\n", this->magparm));
                }
                else
                {
                    ret = -1;

                    if (this->error & DMC_ERROR_MAGPARM)
                        TLOGMSG(1, (DBGINFOFMT "failed to recall magparm\n", DBGINFO));

                    if (this->error & DMC_ERROR_COMM)
                        TLOGMSG(1, (DBGINFOFMT "dmc module not response\n", DBGINFO));
                }
            }
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "dmc_write_message return fail\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid magparm index\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_get_magparm(struct dmc_interface *dmc)
{
    int ret = 0;
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
        ret = this->magparm;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface pointer\n", DBGINFO));
    }

    return ret;
}


static int
dmc_request_status(struct dmc_interface *dmc)
{
    int ret = 0;
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        if (dmc_write_message(dmc, "I", 1) == 0)
        {
            clock_gettime(CLOCK_REALTIME, &this->timeStamp);
            dmc_wait_ack(dmc, DMCACK_REQSTAT);
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "dmc_write_message return fail\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}



static int
dmc_reset(struct dmc_interface *dmc)
{
    int ret = 0;
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        if (dmc_write_message(dmc, "R", 1) == 0)
        {
            this->measure = false;
            TLOGMSG(1, ("reset dmc\n"));
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "dmc_write_message return fail\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_enter_compmode(struct dmc_interface *dmc)
{
    int ret = 0;
    int nwr = 0;
    char cmd[8] = {0};
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        memset(cmd, 0x00, sizeof(cmd));
        nwr = snprintf(cmd, sizeof(cmd), "S96\rX");

        if (dmc_write_message(dmc, cmd, nwr) == 0)
        {
            this->compensation = true;
            this->fom = 0;
            this->ncp = 0;
            TLOGMSG(1, ("enter dmc compensation mode\n"));
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to transmit command\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_exit_compmode(struct dmc_interface *dmc)
{
    int ret = 0;
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        if (this->compensation)
        {
            dmc_reset(dmc);
            this->compensation = false;
            TLOGMSG(1, ("exit dmc compensation mode\n"));
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "dmc is not in compensation mode\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_query_compdata(struct dmc_interface *dmc)
{
    int ret = 0;
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        if (this->compensation)
        {
            if (dmc_write_message(dmc, "Q", 1) == 0)
            {
                dmc_wait_ack(dmc, DMCACK_QCP);

                if (this->error == DMC_ERROR_NONE)
                    TLOGMSG(1, ("success query compensation data, ncp = %d\n", this->ncp));
                else
                {
                    ret = -1;

                    if (this->error & DMC_ERROR_QCP)
                        TLOGMSG(1, (DBGINFOFMT "failed to get compensation data\n", DBGINFO));

                    if (this->error & DMC_ERROR_COMM)
                        TLOGMSG(1, (DBGINFOFMT "dmc module not response\n", DBGINFO));
                }
            }
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "dmc_write_message return fail\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "dmc is not in compensation mode\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_get_ncp(struct dmc_interface *dmc)
{
    int ret = 0;
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        if (this->compensation)
            ret = this->ncp;
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "dmc is not in compensation mode\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_wait_calc_magparm(struct dmc_interface *dmc)
{
    int ret = 0;
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        if ((this->compensation) && (this->ncp == 12))
        {
            dmc_wait_ack(dmc, DMCACK_WAIT_CALC_MAGPARM);

            if (this->error == DMC_ERROR_NONE)
            {
                if (this->fom == -1)
                {
                    ret = -1;
                    TLOGMSG(1, (DBGINFOFMT "failed to calculate magnetic compensation parameter\n", DBGINFO));
                }
                else
                    TLOGMSG(1, ("caculated magnetic compensation parameter, fom = %f\n", this->fom / 10.0));
            }
            else
            {
                ret = -1;

                if (this->error & DMC_ERROR_COMM)
                    TLOGMSG(1, (DBGINFOFMT "dmc module not response\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "not in compensation mode\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_get_fom(struct dmc_interface *dmc)
{
    int ret = 0;
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        if (this->compensation)
            ret = this->fom;
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "not int compensation mode\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_configure(struct dmc_interface *dmc, int mode)
{
    int ret = 0;
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        if (mode == DMC_CONFIG_NORMAL)
        {
            dmc_recall_magparm(dmc, this->magparm);
            dmc_set_coding(dmc);
            dmc_set_itime(dmc, DMC_ITIME_DEFAULT);
            dmc_enable_gimbal(dmc, true);
            dmc_set_los(dmc, 2, 3);
            //dmc_set_los(dmc, 2, 1);
            dmc_set_damping(dmc, DMC_DAMPING_DEFAULT);
            dmc_set_measrange(dmc, MEASRANGE_DEFAULT);
            TLOGMSG(1, ("configured dmc for normal mode\n"));
        }
        else
        {
            dmc_recall_magparm(dmc, MAGPARM_FACTORY);
            dmc_set_los(dmc, 2, 3);
            //dmc_set_los(dmc, 2, 1);
            dmc_set_coding(dmc);
            dmc_set_itime(dmc, DMC_ITIME_DEFAULT);
            dmc_enable_gimbal(dmc, true);
            dmc_set_damping(dmc, 0x00);
            dmc_set_measrange(dmc, MEASRANGE_DEFAULT);
            TLOGMSG(1, ("configured dmc for compensation mode\n"));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_measure(struct dmc_interface *dmc, int mode)
{
    int ret = 0;
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        switch (mode)
        {
        case DMC_CONTINUOUS_MEASURE:
            dmc_reset(dmc);
            dmc_configure(dmc, DMC_CONFIG_NORMAL);
            dmc_start_continuous_measure(dmc);
            break;

        case DMC_SINGLE_MEASURE:
            if (dmc_write_message(dmc, "M", 1) == 0)
            {
                dmc_wait_ack(dmc, DMCACK_REQMEAS);

                if (this->error & DMC_ERROR_COMM)
                {
                    ret = -1;
                    TLOGMSG(1, (DBGINFOFMT "dmc module not response\n", DBGINFO));
                }
            }
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "dmc_write_message return fail\n", DBGINFO));
            }

            break;

        default:
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid argument for measure mode\n", DBGINFO));
            break;
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_test_module(struct dmc_interface *dmc)
{
    int ret = 0;
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        dmc->reset(dmc);
        dmc_request_status(dmc);
        ret = this->error;
        TLOGMSG(1, ("dmc module test result = 0x%02X\n", this->error));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_get_error(struct dmc_interface *dmc)
{
    int ret = 0;
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
        ret = this->error;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface pointer\n", DBGINFO));
    }

    return ret;
}


static int
dmc_accumulate_measures(struct dmc_interface *dmc, bool flag)
{
    int ret = 0;
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        if (flag)
        {
            memset(this->accBuffer, 0x00, sizeof(int) * 3 * ACCBUFLEN);
            this->accIndex = 0;
        }

        this->accumulation = flag;
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_get_accidx(struct dmc_interface *dmc)
{
    int ret = 0;
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
        ret = this->accIndex;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_get_accbuf(struct dmc_interface *dmc, int **azbuf, int **elbuf, int **bkbuf)
{
    int ret = 0;
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        *azbuf = &(this->accBuffer[0][0]);
        *elbuf = &(this->accBuffer[1][0]);
        *bkbuf = &(this->accBuffer[2][0]);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_enter_standby_mode(struct dmc_interface *dmc)
{
    int ret = 0;
    struct uart_interface *uart = NULL;
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        if (!this->standby)
        {
            uart = this->uart;
            dmc->reset(dmc);
            this->running = false;
            pthread_join(this->tid, NULL);
            uart->close(uart);
            this->standby = true;
            TLOGMSG(1, ("dmc module is in standby mode now\n"));
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "dmc module is already in standby mode\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_exit_standby_mode(struct dmc_interface *dmc)
{
    int ret = 0;
    int count = 0;
    char *tty = UART_TTYUSB1;
    struct uart_interface *uart = NULL;
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        if (this->standby)
        {
            uart = this->uart;

            do
            {
                ret = uart->open(uart, tty, B9600);

                if (ret == 0)
                {
                    if (pthread_create(&this->tid, NULL, dmc_read_message, (void *)dmc) == 0)
                    {
                        this->standby = false;
                        dmc->measure(dmc, DMC_CONTINUOUS_MEASURE);
                        TLOGMSG(1, ("dmc is in normal mode\n"));
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
                    if (count == 12)
                    {
                        ret = -1;
                        TLOGMSG(1, ("failed to found uart device\n"));
                        break;
                    }
                    else
                    {
                        if (count % 3 == 0)
                        {
                            if (strstr(tty, "USB1"))
                                tty = UART_TTYUSB4;
                            else
                                tty = UART_TTYUSB1;

                            TLOGMSG(1, ("not found device, switching devcice and retry open (%s)\n", tty));
                        }
                        else
                            TLOGMSG(1, ("uart device is not ready, retry open uart device...\n"));

                        count++;
                        MSLEEP(100);
                    }
                }
            }
            while (ret != 0);
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "dmc module is already in normal mode\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


static int
dmc_init_attribute(struct dmc_interface *dmc)
{
    int ret = 0;
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (this)
    {
        this->uart = uart_create();

        if (this->uart)
        {
            this->uart->open(this->uart, UART_TTYUSB1, B9600);

            if (pthread_create(&this->tid, NULL, dmc_read_message, (void *) dmc) == 0)
            {
                pthread_mutex_init(&this->mtx, NULL);
                pthread_cond_init(&this->cond, NULL);
                this->error        = DMC_ERROR_NONE;
                this->azimuth      = 0;
                this->elevation    = 0;
                this->bank         = 0;
                this->magparm      = MAGPARM_FACTORY;
                this->accumulation = false;
                this->standby      = false;
                this->accIndex       = 0;
            }
            else
            {
                ret = -1;
                uart_destroy(this->uart);
                TLOGMSG(1, (DBGINFOFMT "%s\n", DBGINFO, strerror(errno)));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed create uart interface\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}


/* external function : create dmc interface */
struct dmc_interface *
dmc_create(void)
{
    struct dmc_interface *dmc = NULL;
    struct dmc_attribute *this = malloc(sizeof(struct dmc_attribute));

    if (this)
    {
        memset(this, 0x00, sizeof(struct dmc_attribute));
        dmc = &(this->extif);

        if (dmc_init_attribute(dmc) == 0)
        {
            /* external interface */
            dmc->standby       = dmc_enter_standby_mode;
            dmc->wakeup        = dmc_exit_standby_mode;
            dmc->testModule    = dmc_test_module;
            dmc->getError      = dmc_get_error;
            dmc->reset         = dmc_reset;
            dmc->getAccIndex   = dmc_get_accidx;
            dmc->getAccBuf     = dmc_get_accbuf;
            dmc->accumulate    = dmc_accumulate_measures;
            dmc->configure     = dmc_configure;
            dmc->measure       = dmc_measure;
            dmc->setItime      = dmc_set_itime;
            dmc->setDamping    = dmc_set_damping;
            dmc->getAzimuth    = dmc_get_azimuth;
            dmc->getElevation  = dmc_get_elevation;
            dmc->getBank       = dmc_get_bank;
            dmc->getNcp        = dmc_get_ncp;
            dmc->getFom        = dmc_get_fom;
            dmc->getMagParm    = dmc_get_magparm;
            dmc->storeMagParm  = dmc_store_magparm;
            dmc->recallMagParm = dmc_recall_magparm;
            dmc->enterCompMode = dmc_enter_compmode;
            dmc->exitCompMode  = dmc_exit_compmode;
            dmc->queryCompData = dmc_query_compdata;
            dmc->waitCompParm  = dmc_wait_calc_magparm;

            //dmc->recallMagParm(dmc, MAGPARM_FACTORY);
            //dmc->measure(dmc, DMC_CONTINUOUS_MEASURE);
        }
        else
        {
            free(this);
            dmc = NULL;
            TLOGMSG(1, (DBGINFOFMT "failed to init dmc attribute\n", DBGINFO));
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "%s\n", DBGINFO, strerror(errno)));

    return dmc;
}

/* external function : destroy dmc interface */
int
dmc_destroy(struct dmc_interface *dmc)
{
    int ret = 0;
    struct dmc_attribute *this = (struct dmc_attribute *)dmc;

    if (dmc)
    {
        dmc->reset(dmc);
        this->running = false;
        pthread_join(this->tid, NULL);
        pthread_mutex_destroy(&this->mtx);
        pthread_cond_destroy(&this->cond);
        this->uart->close(this->uart);
        free(this);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));
    }

    return ret;
}
