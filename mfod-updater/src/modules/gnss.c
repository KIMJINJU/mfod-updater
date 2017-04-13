/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gnss.c
        external/internal function implementations of GNSS interface
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/poll.h>

#include "core/logger.h"
#include "etc/util.h"
#include "modules/gnss.h"
#include "modules/uart.h"

/* macro defines : satellite channels */
#define NUM_CHANNELS            64
#define GPS_CHANNEL_INDEX       0
#define GLNS_CHANNEL_INDEX      32

/* macro defines : NEMA0183 messages format */
#define NMEA_MSG_LENGTH         256
#define NMEA_MSG_START          0x24        /* ASCII chracter '$'               */
#define NMEA_MSG_END            0x2A        /* ASCII chracter '*'               */
#define NMEA_MSG_CR             0x0D        /* ASCII chracter carrige return    */
#define NMEA_MSG_LF             0x0A        /* ASCII chracter line feed         */
#define NMEA_MSG_DELIMIT        ','

/* macro defines : NEMA0183 messages id for GPS */
#define NMEA_MSG_GPGLL          "GPGLL"
#define NMEA_MSG_GPGGA          "GPGGA"
#define NMEA_MSG_GPGSA          "GPGSA"
#define NMEA_MSG_GPGSV          "GPGSV"
#define NMEA_MSG_GPRMC          "GPRMC"
#define NMEA_MSG_GPVTG          "GPVTG"

/* macro defines : NEMA0183 messages id for GLONASS */
#define NMEA_MSG_GLGLL          "GLGLL"
#define NMEA_MSG_GLGGA          "GLGGA"
#define NMEA_MSG_GLGSA          "GLGSA"
#define NMEA_MSG_GLGSV          "GLGSV"
#define NMEA_MSG_GLRMC          "GLRMC"
#define NMEA_MSG_GLVTG          "GLVTG"

/* macro defines : NEMA0183 messages id for unified GNSS */
#define NMEA_MSG_GNGLL          "GNGLL"
#define NMEA_MSG_GNGGA          "GNGGA"
#define NMEA_MSG_GNGSA          "GNGSA"
#define NMEA_MSG_GNRMC          "GNRMC"
#define NMEA_MSG_GNVTG          "GNVTG"
#define NMEA_MSG_PUBX           "PUBX"

/* macro defines : number of data fields in NMEA messages */
#define NMEA_NUM_DATA_XXGGA     13
#define NMEA_NUM_DATA_XXGSV     19
#define NMEA_NUM_DATA_XXGSA     17
#define NMEA_NUM_DATA_XXRMC     12


/* structure declaration : dms */
struct dms
{
    int deg;
    int min;
    double sec;
};

/* structure declaration : GNSS channel */
struct gnss_channel
{
    int id;
    int az;
    int el;
    int snr;
};

/* structure declaration : GNSS attributes */
struct gnss_attribute
{
    /* external interface */
    struct gnss_interface extif;

    /* internal attribute */
    bool    standby;
    bool    running;
    bool    valid;
    char    hsphere[2];         /* hsphere[0] : NS, hsphere[1] : EW         */
    int     error;              /* gnss error flag                          */
    int     fix;                /* fix indicator : NONE, 2D or 3D           */
    int     nsu;                /* nsu : number of sattlites used           */
    int     nsv[2];             /* nsv[0] : GPS NSV, nsv[1] : GLONASS NSV   */
    double  pdop;
    double  hdop;
    double  vdop;
    double  altitude;

    struct tm  utc;
    struct dms latitude;
    struct dms longitude;
    struct gnss_channel channel[NUM_CHANNELS];    /* channel[0:31] : GPS, channel[32:61] : GLONASS */
    struct uart_interface *uart;

    pthread_t       tid;
    pthread_mutex_t mtx;
    pthread_cond_t  cond;
};


static int
gnss_write_message(struct gnss_interface *gnss, char *cmd, int cmdlen)
{
    int ret = 0;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

    if (gnss)
    {
        if (this->uart->write(this->uart, cmd, cmdlen) == -1)
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "uart->write return fail\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));
    }

    return ret;
}


static int
gnss_calculate_checksum(char *msg, char *csum)
{
    int ret = 0;
    int length = 0;
    char *tail = NULL;

    if (*msg == NMEA_MSG_START)
    {
        tail = strchr(msg, NMEA_MSG_END);
        length = tail - msg;
        *csum = 0;

        for (int i = 1; i < length; i++)
            *csum ^= *(msg + i);

        TLOGMSG(0, ("checksum = 0x%02X\n", csum));
    }
    else
    {
        ret  = -1;
        TLOGMSG(1, (DBGINFOFMT, "can't find nmea message header\n", DBGINFO));
    }

    return ret;
}


static int
gnss_compare_checksum(char *msg)
{
    int ret = 0;
    char csum_recv = 0;
    char csum_calc = 0;
    char buf[8] = {0};
    char *ptr = NULL;

    if (*msg == NMEA_MSG_START)
    {
        ptr = strchr(msg, NMEA_MSG_END);

        if (ptr)
        {
            memset(buf, 0x00, sizeof(buf));
            snprintf(&buf[0], sizeof(buf), "0x");
            memcpy(&buf[2], (ptr + 1), 2);
            csum_recv = (char) strtol(&buf[0], NULL, 16);
            gnss_calculate_checksum(msg, &csum_calc);

            if (csum_recv != csum_calc)
                ret = -1;
            else
                ret = 0;

            TLOGMSG(0, ("nmea csum_recv = 0x%02X, csum_calc = 0x%02X\n", csum_recv, csum_calc));
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "can't find nmea message tail\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "can't find nmea message header\n", DBGINFO));
    }

    return ret;
}


static void
gnss_copy_field(const char *field, char *buf, int field_len, int buf_len)
{
    memset(buf, 0x00, buf_len);
    memcpy(buf, field, field_len);
}


static int
gnss_set_msgout(struct gnss_interface *gnss)
{
    struct msgoutcfg
    {
        char *msgid;
        int  i2c;
        int  uart1;
        int  uart2;
        int  usb;
        int  spi;
    };

    static struct msgoutcfg cfg[] =
    {
        {"DTM", 0, 0, 0, 0, 0},
        {"GBS", 0, 0, 0, 0, 0},
        {"GGA", 0, 1, 0, 0, 0},
        {"GLL", 0, 0, 0, 0, 0},
        {"GNS", 0, 0, 0, 0, 0},
        {"GRS", 0, 0, 0, 0, 0},
        {"GSA", 0, 1, 0, 0, 0},
        {"GST", 0, 0, 0, 0, 0},
        {"GSV", 0, 1, 0, 0, 0},
        {"RMC", 0, 1, 0, 0, 0},
        {"VTG", 0, 0, 0, 0, 0},
        {"THS", 0, 0, 0, 0, 0},
        {"ZDA", 0, 0, 0, 0, 0},
        {"VLW", 0, 0, 0, 0, 0}
    };

    int ret = 0;
    int nwr = 0;
    char csum = 0;
    char msg[NMEA_MSG_LENGTH] = {0};
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

    if (this)
    {
        for (int i = 0; i < DIM(cfg); i++)
        {
            memset(msg, 0x00, sizeof(msg));
            nwr = snprintf(msg, sizeof(msg), "$PUBX,40,%s,%d,%d,%d,%d,%d,0*",
                  cfg[i].msgid, cfg[i].i2c, cfg[i].uart1, cfg[i].uart2, cfg[i].usb, cfg[i].spi);

            if (gnss_calculate_checksum(msg, &csum) == 0)
            {
                snprintf(&msg[nwr], NMEA_MSG_LENGTH - nwr, "%02X\r\n", csum);
                gnss_write_message(gnss, msg, strlen(msg));
                MSLEEP(20);
            }
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "gnss_calculate_checksum return fail\n", DBGINFO));
                break;
            }
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));
    }

    return ret;
}


static int
gnss_parse_xxgga(struct gnss_interface *gnss, char *msg)
{
    int ret = 0;
    char temp[32] = {0};
    char *sof = NULL;
    char *eof = NULL;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

    if (this && msg)
    {
        eof = strchr(msg, NMEA_MSG_DELIMIT);

        for (int i = 0; i < NMEA_NUM_DATA_XXGGA; i++)
        {
            sof = eof;

            if (i == NMEA_NUM_DATA_XXGGA - 1)
            {
                if (sof)
                {
                    eof = strchr(sof + 1, NMEA_MSG_END);

                    if (!eof)
                    {
                        ret = -1;
                        break;
                    }
                }
                else
                {
                    ret = -1;
                    break;
                }
            }
            else
            {
                if (sof)
                {
                    eof = strchr(sof + 1, NMEA_MSG_DELIMIT);

                    if (!eof)
                    {
                        ret = -1;
                        break;
                    }
                }
                else
                {
                    ret = -1;
                    break;
                }
            }

            if ((sof + 1) == eof)
            {
                if (i == 8)
                    this->altitude = 0.0;
            }
            else
            {
                switch(i)
                {
                case 6:
                    gnss_copy_field(sof + 1, temp, eof - sof - 1, sizeof(temp));
                    this->nsu = strtol(temp, NULL, 10);
                    TLOGMSG(0, ("number of satellites used (nsu) = %d\n", this->nsu));
                    break;

                case 8:
                    gnss_copy_field(sof + 1, temp, eof - sof - 1, sizeof(temp));
                    this->altitude = strtod(temp, NULL);
                    TLOGMSG(0, ("alititude = %f m\n", this->altitude));
                    break;

                default:
                    break;
                }
            }
        }
    }
    else
    {
        ret = -1;

        if (!this)
            TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));

        if (!msg)
            TLOGMSG(1, (DBGINFOFMT "null nmea message\n", DBGINFO));
    }

    return ret;
}


static int
gnss_parse_xxgsa(struct gnss_interface *gnss, char *msg)
{
    int ret = 0;
    char temp[32] = {0};
    char *sof = NULL;
    char *eof = NULL;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

    if (this && msg)
    {
        eof = strchr(msg, NMEA_MSG_DELIMIT);

        for (int i = 0; i < NMEA_NUM_DATA_XXGSA; i++)
        {
            sof = eof;

            if (i == NMEA_NUM_DATA_XXGSA - 1)
            {
                if (sof)
                {
                    eof = strchr(sof + 1, NMEA_MSG_END);

                    if (!eof)
                    {
                        ret = -1;
                        break;
                    }
                }
                else
                {
                    ret = -1;
                    break;
                }
            }
            else
            {
                if (sof)
                {
                    eof = strchr(sof + 1, NMEA_MSG_DELIMIT);

                    if (!eof)
                    {
                        ret = -1;
                        break;
                    }
                }
                else
                {
                    ret = -1;
                    break;
                }
            }

            if ((sof + 1) == eof)
            {
                TLOGMSG(0, ("xxgsa data field %d is null\n", i));

                switch (i)
                {
                case 14:
                    this->pdop = 0.0;
                    break;

                case 15:
                    this->hdop = 0.0;
                    break;

                case 16:
                    this->vdop = 0.0;
                    break;

                default:
                    break;
                }
            }
            else
            {
                switch(i)
                {
                case 1:
                    gnss_copy_field(sof + 1, temp, eof - sof - 1, sizeof(temp));
                    this->fix = strtol(temp, NULL, 10);
                    TLOGMSG(0, ("fix = %d\n", this->fix));
                    break;

                case 14:
                    gnss_copy_field(sof + 1, temp, eof - sof - 1, sizeof(temp));
                    this->pdop = strtod(temp, NULL);
                    TLOGMSG(0, ("pdop = %f\n", this->pdop));
                    break;

                case 15:
                    gnss_copy_field(sof + 1, temp, eof - sof - 1, sizeof(temp));
                    this->hdop = strtod(temp, NULL);
                    TLOGMSG(0, ("hdop = %f\n", this->hdop));
                    break;

                case 16:
                    gnss_copy_field(sof + 1, temp, eof - sof - 1, sizeof(temp));
                    this->vdop = strtod(temp, NULL);
                    TLOGMSG(0, ("vdop = %f\n", this->vdop));
                    break;

                default:
                    break;
                }
            }
        }
    }
    else
    {
        ret = -1;

        if (!this)
            TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));

        if (!msg)
            TLOGMSG(1, (DBGINFOFMT "null nmea message\n", DBGINFO));
    }

    return ret;
}


static int
gnss_parse_xxgsv(struct gnss_interface *gnss, char *msg)
{
    int msg_num = 0;
    int num_msgs = 0;
    int idx = 0;
    int count = 0;
    int end = 0;
    int gnssid = 0;
    char temp[32] = {0};
    char *sof = NULL;
    char *eof = NULL;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

    if (this && msg)
    {
        /* identify gnss type */
        if (strstr(msg, NMEA_MSG_GPGSV))
            gnssid = GNSS_GPS;
        else if (strstr(msg, NMEA_MSG_GLGSV))
            gnssid = GNSS_GLONASS;
        else
        {
            TLOGMSG(1, ("can't identify gnss\n"));
            return -1;
        }

        end = NMEA_NUM_DATA_XXGSA - 1;
        eof = strchr(msg, NMEA_MSG_DELIMIT);

        for (int i = 0; i < NMEA_NUM_DATA_XXGSA; i++)
        {
            sof = eof;

            if (i == end)
            {
                if (sof)
                {
                    eof = strchr(sof + 1, NMEA_MSG_END);

                    if (!eof)
                        return -1;
                }
                else
                    return -1;
            }
            else
            {
                if (sof)
                {
                    eof = strchr(sof + 1, NMEA_MSG_DELIMIT);

                    if (!eof)
                        return -1;
                }
                else
                    return -1;
            }

            if ((sof + 1) == eof)
            {
                TLOGMSG(0, ("xxgsv data field %d is null\n", i));

                switch(i)
                {
                case 3: case 7: case 11: case 15:
                    if (++count == this->nsv[gnssid])
                        end = i + 3;

                    idx = 32 * gnssid + ((4 * (msg_num - 1)) + ((i - 3) / 4 + 1)) - 1;
                    this->channel[idx].id = 0;
                    break;

                case 4: case 8: case 12: case 16:
                    this->channel[idx].el = 0;
                    break;

                case 5: case 9: case 13: case 17:
                    this->channel[idx].az = 0;
                    break;

                case 6: case 10: case 14: case 18:
                    this->channel[idx].snr = 0;
                    break;

                default:
                    break;
                }

                if (end == i)
                    break;
            }
            else
            {
                switch(i)
                {
                case 0:
                    gnss_copy_field(sof + 1, temp, eof - sof - 1, sizeof(temp));
                    num_msgs = strtol(temp, NULL, 10);
                    break;

                case 1:
                    gnss_copy_field(sof + 1, temp, eof - sof - 1, sizeof(temp));
                    msg_num = strtol(temp, NULL, 10);
                    count = 4 * (msg_num - 1);
                    break;

                case 2:
                    gnss_copy_field(sof + 1, temp, eof - sof - 1, sizeof(temp));
                    this->nsv[gnssid] = strtol(temp, NULL, 10);

                    if (this->nsv[gnssid] == 0)
                    {
                        TLOGMSG(0, ("nsv %s = %d\n", gnssid == 0 ? "gps" : "glonass", this->nsv[gnssid]));
                        return 0;
                    }

                    break;

                case 3: case 7: case 11: case 15:
                    if (++count == this->nsv[gnssid])
                        end = i + 3;

                    gnss_copy_field(sof + 1, temp, eof - sof - 1, sizeof(temp));
                    idx = 32 * gnssid + ((4 * (msg_num - 1)) + ((i - 3) / 4 + 1)) - 1;
                    this->channel[idx].id = strtol(temp, NULL, 10);
                    TLOGMSG(0, ("channel[%d] = %s#%s\n", idx, gnssid == 0 ? "GPS":"GLNS", temp));
                    break;

                case 4: case 8: case 12: case 16:
                    gnss_copy_field(sof + 1, temp, eof - sof - 1, sizeof(temp));
                    idx = 32 * gnssid + ((4 * (msg_num - 1)) + ((i - 3) / 4 + 1)) - 1;
                    this->channel[idx].el = strtol(temp, NULL, 10);
                    TLOGMSG(0, ("%s#%d elevation = %d\n", gnssid == 0 ? "GPS":"GLNS", this->channel[idx].id, this->channel[idx].el));
                    break;

                case 5: case 9: case 13: case 17:
                    gnss_copy_field(sof + 1, temp, eof - sof - 1, sizeof(temp));
                    idx = 32 * gnssid + ((4 * (msg_num - 1)) + ((i - 3) / 4 + 1)) - 1;
                    this->channel[idx].az = strtol(temp, NULL, 10);
                    TLOGMSG(0, ("%s#%d azimuth = %d\n", gnssid == 0 ? "GPS":"GLNS", this->channel[idx].id, this->channel[idx].az));
                    break;

                case 6: case 10: case 14: case 18:
                    gnss_copy_field(sof + 1, temp, eof - sof - 1, sizeof(temp));
                    idx = 32 * gnssid + ((4 * (msg_num - 1)) + ((i - 3) / 4 + 1)) - 1;
                    this->channel[idx].snr = strtol(temp, NULL, 10);
                    TLOGMSG(0, ("%s#%d azimuth = %d\n", gnssid == 0 ? "GPS":"GLNS", this->channel[idx].id, this->channel[idx].snr));
                    break;

                default:
                    break;
                }
            }
        }
    }
    else
    {
        if (!this)
            TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));

        if (!msg)
            TLOGMSG(1, (DBGINFOFMT "null nmea message\n", DBGINFO));

        return -1;
    }

    return 0;
}


static int
gnss_parse_xxrmc(struct gnss_interface *gnss, char *msg)
{
    int ret = 0;
    char temp[32] = {0};
    char deg[8] = {0};
    char min[8] = {0};
    char sec[8] = {'0', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    char *sof = NULL;
    char *eof = NULL;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

    if (this && msg)
    {
        eof = strchr(msg, NMEA_MSG_DELIMIT);

        for (int i = 0; i < NMEA_NUM_DATA_XXRMC; i++)
        {
            sof = eof;

            if (i == NMEA_NUM_DATA_XXRMC - 1)
            {
                if (sof)
                {
                    eof = strchr(sof + 1, NMEA_MSG_END);

                    if (!eof)
                    {
                        ret = -1;
                        break;
                    }
                }
                else
                {
                    ret = -1;
                    break;
                }
            }
            else
            {
                if (sof)
                {
                    eof = strchr(sof + 1, NMEA_MSG_DELIMIT);

                    if (!eof)
                    {
                        ret = -1;
                        break;
                    }
                }
                else
                {
                    ret = -1;
                    break;
                }
            }

            if ((sof + 1) == eof)
                TLOGMSG(0, ("xxrmc data field %d is null \n", i));
            else
            {
                switch(i)
                {
                case 0:
                    gnss_copy_field(sof + 1, temp, eof - sof - 1, sizeof(temp));
                    strptime(temp, "%H%M%S", &this->utc);
                    break;

                case 1:
                    gnss_copy_field(sof + 1, temp, eof - sof - 1, sizeof(temp));

                    if (temp[0] == 'A')
                        this->valid = true;
                    else
                        this->valid = false;

                    TLOGMSG(0, ("pos mode = %c\n", temp[0]));
                    break;

                case 2: /* latitude field */
                    gnss_copy_field(sof + 1, temp, eof - sof - 1, sizeof(temp));
                    memset(deg, 0x00, sizeof(deg));
                    memset(min, 0x00, sizeof(min));
                    memset(&sec[1], 0x00, sizeof(sec) - 1);
                    memcpy(deg, temp, 2);
                    memcpy(min, &temp[2], 2);
                    memcpy(&sec[1], &temp[4], 5);
                    this->latitude.deg = strtol(deg, NULL, 10);
                    this->latitude.min = strtol(min, NULL, 10);
                    this->latitude.sec = strtod(sec, NULL) * 60.0;
                    break;

                case 3: /* NS indicator field*/
                    gnss_copy_field(sof + 1, temp, eof - sof - 1, sizeof(temp));
                    this->hsphere[0] = temp[0];
                    break;

                case 4: /* longitude field */
                    gnss_copy_field(sof + 1, temp, eof - sof - 1, sizeof(temp));
                    memset(deg, 0x00, sizeof(deg));
                    memset(min, 0x00, sizeof(min));
                    memset(&sec[1], 0x00, sizeof(sec) - 1);
                    memcpy(deg, temp, 3);
                    memcpy(min, &temp[3], 2);
                    memcpy(&sec[1], &temp[5], 5);
                    this->longitude.deg = strtol(deg, NULL, 10);
                    this->longitude.min = strtol(min, NULL, 10);
                    this->longitude.sec = strtod(sec, NULL) * 60.0;
                    break;

                case 5: /* EW indicator field */
                    gnss_copy_field(sof + 1, temp, eof - sof - 1, sizeof(temp));
                    this->hsphere[1] = temp[0];
                    break;

                case 8:
                    gnss_copy_field(sof + 1, temp, eof - sof - 1, sizeof(temp));
                    strptime(temp, "%d%m%y", &this->utc);
                    break;

                default:
                    break;
                }
            }
        }
    }
    else
    {
        ret = -1;

        if (!this)
            TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));

        if (!msg)
            TLOGMSG(1, (DBGINFOFMT "null nmea message\n", DBGINFO));
    }

    return ret;
}


static int
gnss_classify_message(struct gnss_interface *gnss, char *msg)
{
    struct parser_table
    {
        char *msgid;
        int (*parser) (struct gnss_interface *, char *);
    };

    static struct parser_table pt[] =
    {
        {NMEA_MSG_GPGGA, gnss_parse_xxgga},
        {NMEA_MSG_GPGSA, gnss_parse_xxgsa},
        {NMEA_MSG_GPGSV, gnss_parse_xxgsv},
        {NMEA_MSG_GPRMC, gnss_parse_xxrmc},
        {NMEA_MSG_GLGGA, gnss_parse_xxgga},
        {NMEA_MSG_GLGSA, gnss_parse_xxgsa},
        {NMEA_MSG_GLGSV, gnss_parse_xxgsv},
        {NMEA_MSG_GLRMC, gnss_parse_xxrmc},
        {NMEA_MSG_GNGGA, gnss_parse_xxgga},
        {NMEA_MSG_GNGSA, gnss_parse_xxgsa},
        {NMEA_MSG_GNRMC, gnss_parse_xxrmc}
    };

    int ret = 0;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

    if (this)
    {
        if (gnss_compare_checksum(msg) == 0)
        {
            for (int i = 0; i < DIM(pt); i++)
            {
                if (strstr(msg, pt[i].msgid) != NULL)
                {
                    ret = pt[i].parser(gnss, msg);
                    break;
                }
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "checksum mismatch\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));
    }

    return ret;
}


static void *
gnss_read_message(void *arg)
{
    int nread = 0;
    int retry = 0;
    int idx = 0;
    char buf[2048] = {0};
    char pkt[NMEA_MSG_LENGTH] = {0};

    struct gnss_interface *gnss = (struct gnss_interface *) arg;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;
    struct uart_interface *uart = this->uart;
    struct pollfd state = {.fd = uart->getFileDesc(uart), .events = POLLIN, .revents = 0};

    this->running = true;
    memset(&pkt[0], 0x00, sizeof(pkt));
    memset(&buf[0], 0x00, sizeof(buf));

    while(this->running)
    {
        if (poll(&state, 1, 1000) > 0)
        {
            nread = uart->read(uart, &buf[0], uart->getNumRead(uart));

            for (int i = 0; i < nread; i++)
            {
                switch(buf[i])
                {
                case NMEA_MSG_START:
                    if (pkt[0] != NMEA_MSG_START)
                        pkt[idx++] = buf[i];
                    break;

                case NMEA_MSG_LF:
                    if ((pkt[0] == NMEA_MSG_START) && (pkt[idx - 1] == NMEA_MSG_CR))
                    {
                        pkt[idx++] = buf[i];
                        gnss_classify_message(gnss, &pkt[0]);
                        memset(&pkt[0], 0x00, sizeof(pkt));
                        idx = retry = 0;
                        this->error &= ~(GNSS_ERROR_COMM);
                    }

                    break;

                default:
                    if (pkt[0] == NMEA_MSG_START)
                        pkt[idx++] = buf[i];
                    break;
                }
            }

            memset(&buf[0], 0x00, sizeof(buf));
        }
        else
        {
            if (retry == 2)
            {
                idx = 0;
                memset(&pkt[0], 0x00, sizeof(pkt));
                this->error |= GNSS_ERROR_COMM;
            }
            else
                retry++;
        }
    }

    return NULL;
}


static int
gnss_is_valid(struct gnss_interface *gnss)
{
    int ret = 0;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

    if (this)
        ret = this->valid;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));
    }

    return ret;
}


static int
gnss_is_fix(struct gnss_interface *gnss)
{
    int ret = 0;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

    if (this)
        ret = this->fix;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));
    }

    return ret;
}


static int
gnss_get_latitude(struct gnss_interface *gnss, int *deg, int *min, double *sec, char *ns)
{
    int ret = 0;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

    if (this)
    {
        if ((this->fix != GNSS_FIX_NONE) && (this->valid))
        {
            *deg = this->latitude.deg;
            *min = this->latitude.min;
            *sec = this->latitude.sec;
            *ns = this->hsphere[0];
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "gnss data is not valid or fix\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));
    }

    return ret;
}


static int
gnss_get_longitude(struct gnss_interface *gnss, int *deg, int *min, double *sec, char *ew)
{
    int ret = 0;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

    if (this)
    {
        if ((this->fix != GNSS_FIX_NONE) && (this->valid))
        {
            *deg = this->longitude.deg;
            *min = this->longitude.min;
            *sec = this->longitude.sec;
            *ew = this->hsphere[1];
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "gnss data is not valid or fix\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gnss interface pointer\n", DBGINFO));
    }

    return ret;
}


static int
gnss_get_altitude(struct gnss_interface *gnss, double *alt)
{
    int ret = 0;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

    if (this)
    {
        if ((this->fix != GNSS_FIX_NONE) && (this->valid))
            *alt = this->altitude;
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "gnss data is not valid or fix\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));
    }

    return ret;
}


static int
gnss_get_utc(struct gnss_interface *gnss, time_t *utc)
{
    int ret = 0;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

    if (this)
    {
        if ((this->fix != GNSS_FIX_NONE) && (this->valid))
            *utc = mktime(&this->utc);
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "gnss data is not valid or fix\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));
    }

    return ret;
}


static int
gnss_get_dop(struct gnss_interface *gnss, double *pdop, double *hdop, double *vdop)
{
    int ret = 0;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

    if (this)
    {
        if ((this->fix != GNSS_FIX_NONE) && (this->valid))
        {
            *pdop = this->pdop;
            *hdop = this->hdop;
            *vdop = this->vdop;
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "gnss data is not valid or fix\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));
    }

    return ret;
}


static int
gnss_get_nsu(struct gnss_interface *gnss, int *nsu)
{
    int ret = 0;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

    if (this)
        *nsu = this->nsu;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));
    }

    return ret;
}


static int
gnss_get_nsv(struct gnss_interface *gnss, int *nsv_gps, int *nsv_glns)
{
    int ret = 0;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

    if (this)
    {
        *nsv_gps = this->nsv[GNSS_GPS];
        *nsv_glns = this->nsv[GNSS_GLONASS];
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));
    }

    return ret;
}


static int
gnss_get_satid(struct gnss_interface *gnss, int gnssid, int idx, int *id)
{
    int ret = 0;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

    if (this)
    {
        if (gnssid == GNSS_GPS || gnssid == GNSS_GLONASS)
        {
            if (0 <= idx && idx < this->nsv[gnssid])
                *id = this->channel[32 * gnssid + idx].id;
            else
            {
                ret = 1;
                TLOGMSG(1, (DBGINFOFMT "invalid channel index\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid gnss id\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));
    }

    return ret;
}


static int
gnss_get_sataz(struct gnss_interface *gnss, int gnssid, int idx, int *az)
{
    int ret = 0;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

    if (this)
    {
        if (gnssid == GNSS_GPS || gnssid == GNSS_GLONASS)
        {
            if (0 <= idx && idx < this->nsv[gnssid])
                *az = this->channel[32 * gnssid + idx].az;
            else
            {
                ret = 1;
                TLOGMSG(1, (DBGINFOFMT "invalid channel index\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid gnss id\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));
    }

    return ret;
}


static int
gnss_get_satel(struct gnss_interface *gnss, int gnssid, int idx, int *el)
{
    int ret = 0;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

    if (this)
    {
        if (gnssid == GNSS_GPS || gnssid == GNSS_GLONASS)
        {
            if (0 <= idx && idx < this->nsv[gnssid])
                *el = this->channel[32 * gnssid + idx].el;
            else
            {
                ret = 1;
                TLOGMSG(1, (DBGINFOFMT "invalid channel index\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid gnss id\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));
    }

    return ret;
}


static int
gnss_get_satsnr(struct gnss_interface *gnss, int gnssid, int idx, int *snr)
{
    int ret = 0;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

    if (this)
    {
        if (gnssid == GNSS_GPS || gnssid == GNSS_GLONASS)
        {
            if (0 <= idx && idx < this->nsv[gnssid])
                *snr = this->channel[32 * gnssid + idx].snr;
            else
            {
                ret = 1;
                TLOGMSG(1, (DBGINFOFMT "invalid channel index\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid gnss id\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));
    }

    return ret;
}


static int
gnss_enter_standby_mode(struct gnss_interface *gnss)
{
    int ret = 0;
    struct uart_interface *uart = NULL;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

    if (this)
    {
        if (!this->standby)
        {
            uart = this->uart;
            this->running = false;
            pthread_join(this->tid, NULL);
            uart->close(uart);
            this->standby = true;
            TLOGMSG(1, ("gnss module is in standby mode\n"));
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "gnss module is already in standby mode\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));
    }

    return ret;
}


static int
gnss_exit_standby_mode(struct gnss_interface *gnss)
{
    int ret = 0;
    int count = 0;
    char *tty = UART_TTYUSB0;
    struct uart_interface *uart = NULL;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

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
                    if (pthread_create(&this->tid, NULL, gnss_read_message, (void *)this) == 0)
                    {
                        this->standby = false;
                        TLOGMSG(1, ("gnss module is in normal mode\n"));
                    }
                    else
                    {
                        ret = -1;
                        uart->close(uart);
                        TLOGMSG(1, (DBGINFOFMT "pthread_return fail, %s\n", DBGINFO, strerror(errno)));
                        break;
                    }
                }
                else
                {
                    if (count == 3)
                    {
                        if (strstr(tty, "USB0"))
                            tty = UART_TTYUSB3;
                        else
                            tty = UART_TTYUSB0;

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
            TLOGMSG(1, (DBGINFOFMT "gnss module is already in normal mode\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));
    }

    return ret;
}


static int
gnss_test_module(struct gnss_interface *gnss)
{
    int ret = 0;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

    if (this)
    {
        MSLEEP(3000);
        ret = this->error;
        TLOGMSG(1, ("gnss module test result = 0x%02x\n", this->error));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));
    }

    return ret;
}


static int
gnss_get_error(struct gnss_interface *gnss)
{
    int ret = 0;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

    if (this)
        ret = this->error;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));
    }

    return ret;
}


static int
gnss_init_attribute(struct gnss_interface *gnss)
{
    int ret = 0;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

    if (this)
    {
        this->uart = uart_create();

        if (this->uart)
        {
            if(this->uart->open(this->uart, UART_TTYUSB0, B9600) == 0)
            {
                this->valid         = false;
                this->running       = false;
                this->standby       = false;
                this->error         = 0;
                this->nsv[0]        = 0;
                this->nsv[1]        = 0;
                this->nsu           = 0;
                this->altitude      = 0.0;
                this->pdop          = 0.0;
                this->hdop          = 0.0;
                this->vdop          = 0.0;
                this->latitude.deg  = 0;
                this->latitude.min  = 0;
                this->latitude.sec  = 0.0;
                this->longitude.deg = 0;
                this->longitude.min = 0;
                this->longitude.sec = 0.0;
                pthread_cond_init(&this->cond, NULL);
                pthread_mutex_init(&this->mtx, NULL);
                memset(&this->channel[0], 0x00, sizeof(struct gnss_channel) * NUM_CHANNELS);
            }
            else
            {
                ret = -1;
                uart_destroy(this->uart);
                TLOGMSG(1, (DBGINFOFMT "open uart return fail\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "create uart interface return null\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));
    }

    return ret;
}


static int
gnss_deinit_attribute(struct gnss_interface *gnss)
{
    int ret = 0;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

    if (this)
    {
        pthread_mutex_destroy(&this->mtx);
        pthread_cond_destroy(&this->cond);
        uart_destroy(this->uart);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));
    }

    return ret;
}


struct gnss_interface *
gnss_create(void)
{
    struct gnss_interface *gnss = NULL;
    struct gnss_attribute *this = malloc(sizeof(struct gnss_attribute));

    if (this)
    {
        memset(this, 0x00, sizeof(struct gnss_attribute));
        gnss = &(this->extif);
        gnss->getLatitude  = gnss_get_latitude;
        gnss->getLongitude = gnss_get_longitude;
        gnss->getAltitude  = gnss_get_altitude;
        gnss->getDOP     = gnss_get_dop;
        gnss->getUTC     = gnss_get_utc;
        gnss->getNSU     = gnss_get_nsu;
        gnss->getNSV     = gnss_get_nsv;
        gnss->getSatID   = gnss_get_satid;
        gnss->getSatAZ   = gnss_get_sataz;
        gnss->getSatEL   = gnss_get_satel;
        gnss->getSatSNR  = gnss_get_satsnr;
        gnss->fix        = gnss_is_fix;
        gnss->valid      = gnss_is_valid;
        gnss->standby    = gnss_enter_standby_mode;
        gnss->wakeup     = gnss_exit_standby_mode;
        gnss->getError   = gnss_get_error;
        gnss->testModule = gnss_test_module;

        if (gnss_init_attribute(gnss) == 0)
        {
            if (pthread_create(&this->tid, NULL, gnss_read_message, (void *)gnss) == 0)
            {
                gnss_set_msgout(gnss);
                TLOGMSG(1, ("create gnss interface\n"));
            }
            else
            {
                gnss_deinit_attribute(gnss);
                free(this);
                gnss = NULL;
                TLOGMSG(1, (DBGINFOFMT "failed to create gnss interface, failed to create thread\n", DBGINFO));
            }
        }
        else
        {
            free(this);
            gnss = NULL;
            TLOGMSG(1, (DBGINFOFMT "failed to create gnss interface, failed to init gnss attribute\n", DBGINFO));
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed create gnss interface, malloc return null\n", DBGINFO));

    return gnss;
}


int
gnss_destroy(struct gnss_interface *gnss)
{
    int ret = 0;
    struct gnss_attribute *this = (struct gnss_attribute *) gnss;

    if (this)
    {
        this->running = false;
        pthread_join(this->tid, NULL);
        gnss_deinit_attribute(gnss);
        free(this);
        TLOGMSG(1, ("destroy gnss interface\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));
    }

    return ret;
}
