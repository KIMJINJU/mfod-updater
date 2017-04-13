/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    mcu.c
        external/internal function implementations of MCU interface
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
#include "core/evque.h"
#include "etc/util.h"
#include "modules/mcu.h"
#include "modules/uart.h"

/* macro defines : mcu message offset */
#define MCU_MSG_OFFSET_SOM              0
#define MCU_MSG_OFFSET_LEN              1
#define MCU_MSG_OFFSET_CMD              2
#define MCU_MSG_OFFSET_PARM             3
#define MCU_MSG_SOM                     0x81
#define MCU_MSG_LENGTH                  8

/* macro defines : mcu message command  */
#define MCU_MSG_CMD_REQ_STATUS          0x60
#define MCU_MSG_CMD_REQ_PWRSRC_TYPE     0x61
#define MCU_MSG_CMD_REQ_PWRSRC_VOLT     0x62
#define MCU_MSG_CMD_MONITOR_PWRSRC_VOLT 0x63
#define MCU_MSG_CMD_PWROFF              0x64
#define MCU_MSG_CMD_PWRKEY_STATE        0x65
#define MCU_MSG_CMD_ENABLE_PROXSEN      0x66
#define MCU_MSG_CMD_PROXIMITY_STATUS    0x67
#define MCU_MSG_CMD_INIT_SHUTTER        0x68
#define MCU_MSG_CMD_MOVE_SHUTTER        0x69
#define MCU_MSG_CMD_REQ_SHTRPOS         0x6A
#define MCU_MSG_CMD_SET_CELLTHOLD       0x6B
#define MCU_MSG_CMD_REQ_CELLTHOLD       0x6E
#define MCU_MSG_CMD_SET_PROXTHOLD       0x6D
#define MCU_MSG_CMD_REQ_PROXTHOLD       0x70
#define MCU_MSG_CMD_EXTDC_STATUS		0x73
#define MCU_MSG_CMD_PWRSAVE				0x74
#define MCU_MSG_CMD_REQ_VERSION         0x77

/* macro define : cell status */
#define MCU_CELL_STATUS_NORMAL          0
#define MCU_CELL_STATUS_LOWVOLT         1
#define MCU_CELL_STATUS_DEPLETED        2

#define PRIMARY_CELL_VOLT_MAX           650
#define PRIMARY_CELL_VOLT_MIN           600
#define PRIMARY_CELL_VOLT_INTV          (PRIMARY_CELL_VOLT_MAX - PRIMARY_CELL_VOLT_MIN)

#define SECONDARY_CELL_VOLT_MAX         650
#define SECONDARY_CELL_VOLT_MIN         600
#define SECONDARY_CELL_VOLT_INTV        (SECONDARY_CELL_VOLT_MAX - SECONDARY_CELL_VOLT_MIN)

#define MCU_VOLT_PREVIOUS               0
#define MCU_VOLT_CURRENT                1

/* macro define : status bit */
#define MCU_STATUS_BIT_CELLTYPE			0x02
#define MCU_STATUS_BIT_PWRSRC			0x04
#define MCU_STATUS_BIT_SHUTTER_LXMVT	0x08
#define MCU_STATUS_BIT_SHUTTER_HXMVT	0x10
#define MCU_STATUS_BIT_PROXSEN			0x20

/* macro define : power source bit */
#define MCU_PWRSRC_BIT_SECONDARY_CELL   0x01
#define MCU_PWRSRC_BIT_EXTDC            0x02

/* macro define : power key state */
#define MCU_PWRKEY_PRESS                0x00
#define MCU_PWRKEY_DEPRESS              0x01

/* macro define : cell threshold */
#define MCU_MAX_CELLTHOLD               1400  /* maximum cell threshold  (14.00 V)  */
#define MCU_MIN_CELLTHOLD               1     /* minimum cell thresholde  (0.01 V)  */

/* macro define : proximity threshold */
#define MCU_PATH_PROXTHOLD_CONF     "/mnt/mmc/amod-data/parm/proxthold.conf"
#define MCU_DEFAULT_OPEN_PROXTHOLD  0x08
#define MCU_DEFAULT_CLOSE_PROXTHOLD 0x20


/* structure declaration : power */
struct pwrstatus
{
    bool    monitor;
    int     powerKey;
    int     source;
    int     extVolt;
    int     cellLevel;
    int     cellType;
    int     cellStatus;
    int     cellVolt;
    int     cellThold[2];      /* low voltage threshold of battery     */
    double  cellRemain;        /* battery percentage                   */
};

/* structure declaration : shutter */
struct shutter
{
    bool close;
    bool motion;
    int  filter; /**/
};

/* structure declaration : proximity sensor */
struct proxsensor
{
    bool enable;
    bool close;
    int  threshold[2];
};

/* structure declaration : mcu interface attribute */
struct mcu_attribute
{
    /* external interface */
    struct mcu_interface extif;

    /* internal attribute */
    bool standby;
    bool running;
    bool waitAck;
    int  error;
    int  version;

    pthread_t tid;
    pthread_mutex_t mtx;
    pthread_cond_t cond;

    struct uart_interface  *uart;
    struct timespec        timeStamp;
    struct pwrstatus       powerStatus;
    struct shutter         shutter;
    struct proxsensor      proxSensor;
};


static int
mcu_wait_ack(struct mcu_interface *mcu)
{
    int ret = 0;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
    {
        if (!this->waitAck)
        {
            this->waitAck = true;

            pthread_mutex_lock(&this->mtx);

            while (this->waitAck)
                pthread_cond_wait(&this->cond, &this->mtx);

            pthread_mutex_unlock(&this->mtx);
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "mcu is waiting ack\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_recv_ack(struct mcu_interface *mcu)
{
    int ret = 0;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

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
            TLOGMSG(0, (DBGINFOFMT "mcu is not waiting ack\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_write_message(struct mcu_interface *mcu, char *msg)
{
    int ret = 0;
    int retry = 0;
    struct uart_interface *uart = NULL;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
    {
        if (msg)
        {
            uart = this->uart;

            while (true)
            {
                if (uart->write(uart, msg, *(msg + MCU_MSG_OFFSET_LEN)) > 0)
                {
                    clock_gettime(CLOCK_REALTIME, &this->timeStamp);
                    break;
                }
                else
                {
                    if (retry < 3)
                        retry++;
                    else
                    {
                        ret = -1;
                        this->error |= MCU_ERROR_COMM;
                        TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
                        break;
                    }
                }
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT, "null message\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_parse_mcu_status(struct mcu_interface *mcu, char *parm)
{
	int ret = 0;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

	if (this)
	{
		/* parse cell type bit */
		if (*parm & MCU_STATUS_BIT_CELLTYPE)
		{
			this->powerStatus.cellType = MCU_CELL_SECONDARY;
			TLOGMSG(1, ("cell type = secondary cell (LIION)\n"));
		}
		else
		{
			this->powerStatus.cellType = MCU_CELL_PRIMARY;
			TLOGMSG(1, ("cell type = primary cell (CR123A)\n"));
		}

		/* parse power source bit */
		if (*parm & MCU_STATUS_BIT_PWRSRC)
		{
			this->powerStatus.source = MCU_PWRSRC_EXTDC;
			TLOGMSG(1, ("power source = external dc\n"));
		}
		else
		{
			this->powerStatus.source = MCU_PWRSRC_CELL;
			TLOGMSG(1, ("power source = cell\n"));
		}

		/* parse shutter position bit */
		if (*parm & MCU_STATUS_BIT_SHUTTER_LXMVT)
		{
			this->shutter.close = false;
			this->shutter.filter = MCU_SHUTTER_LXMVT;
			TLOGMSG(1, ("shutter position = low transmissivity filter\n"));
		}
		else
		{
			if (*parm & MCU_STATUS_BIT_SHUTTER_HXMVT)
			{
				this->shutter.close = false;
				this->shutter.filter = MCU_SHUTTER_HXMVT;
				TLOGMSG(1, ("shutter position = high transmissivity filter\n"));
			}
			else
			{
				this->shutter.close = true;
				TLOGMSG(1, ("shutter position = close\n"));
			}
		}

		/* parse proximity sensor bit */
		if (*parm & MCU_STATUS_BIT_PROXSEN)
		{
			this->proxSensor.enable = true;
			TLOGMSG(1, ("proximity sensor = disable\n"));
		}
		else
		{
			this->proxSensor.enable = false;
			TLOGMSG(1, ("proximity sensor = disable\n"));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
	}

	return ret;
}


static int
mcu_parse_init_shutter(struct mcu_interface *mcu)
{
    int ret = 0;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (mcu)
    {
        this->shutter.motion = false;
        this->shutter.filter = MCU_SHUTTER_LXMVT;
        this->shutter.close  = false;
        TLOGMSG(1, ("init shutter position\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_parse_shutter_position(struct mcu_interface *mcu, char *parm)
{
    int ret = 0;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
    {
        switch (*parm)
        {
        case MCU_SHUTTER_CLOSE:
            this->shutter.close = true;
            TLOGMSG(1, ("shutter postion = close\n"));
            break;

        case MCU_SHUTTER_LXMVT:
            this->shutter.close = false;
            this->shutter.filter = MCU_SHUTTER_LXMVT;
            TLOGMSG(1, ("shutter position = low transmisivity filter\n"));
            break;

        case MCU_SHUTTER_HXMVT:
            this->shutter.close = false;
            this->shutter.filter = MCU_SHUTTER_HXMVT;
            TLOGMSG(1, ("shutter position = high transmisivity filter\n"));
            break;

        default:
            ret = -1;
            TLOGMSG(1, ("invalid shutter position parameter\n"));
            break;
        }

        this->shutter.motion = false;
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_parse_pwrsrc_type(struct mcu_interface *mcu, char *parm)
{
    int ret = 0;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (mcu)
    {
        if (*parm & MCU_PWRSRC_BIT_SECONDARY_CELL)
        {
            this->powerStatus.cellType = MCU_CELL_SECONDARY;
            TLOGMSG(1, ("battery type = secondary cell (Li-ion)\n"));
        }
        else
        {
            this->powerStatus.cellType = MCU_CELL_PRIMARY;
            TLOGMSG(1, ("battery type = primary cell (Li-SOCL2)\n"));
        }

        if (*parm & MCU_PWRSRC_BIT_EXTDC)
        {
            this->powerStatus.source = MCU_PWRSRC_EXTDC;
            TLOGMSG(1, ("power source = external dc\n"));
        }
        else
        {
            this->powerStatus.source = MCU_PWRSRC_CELL;
            TLOGMSG(1, ("power source = internal battery\n"));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_parse_pwrkey_state(struct mcu_interface *mcu, char *parm)
{
    int ret = 0;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (mcu)
    {
        switch (*parm)
        {
        case 0x00:
            this->powerStatus.powerKey = MCU_PWRKEY_PRESS;
            TLOGMSG(1, ("power key pushed\n"));
            break;

        case 0x01:
            //set power key released event.
            this->powerStatus.powerKey = MCU_PWRKEY_DEPRESS;
            evque_set_event(EVCODE_PWRKEY_RELEASED, 0);
            TLOGMSG(1, ("power key released\n"));
            break;

        default:
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invaild power key state\n", DBGINFO));
            break;
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_parse_proxmity_status(struct mcu_interface *mcu, char *parm)
{
	int ret = 0;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

	if (mcu)
	{
		switch(*parm)
		{
		case 0x00:
			this->proxSensor.close = false;
			//set event..
			TLOGMSG(1, ("proximity status = open\n"));
			break;

		case 0x01:
			this->proxSensor.close = true;
			// set event;
			TLOGMSG(1, ("proximity status = close\n"));
			break;

		default:
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "invalid proximity status\n", DBGINFO));
			break;
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
	}

	return ret;
}


static int
mcu_parse_proxsen_flag(struct mcu_interface *mcu, char *parm)
{
	int ret = 0;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

	if (this)
	{
		switch (*parm)
		{
		case 0x00:
			this->proxSensor.enable = false;
			TLOGMSG(1, ("proximity sensor = disable\n"));
			break;

		case 0x01:
			this->proxSensor.enable = true;
			TLOGMSG(1, ("proximity sensor = enable\n"));
			break;

		default:
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "invalid proximity sensor flag\n", DBGINFO));
			break;
		}

	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
	}

	return ret;
}


static int
mcu_parse_extdc_status(struct mcu_interface *mcu, char *parm)
{
	int ret = 0;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

	if (this)
	{
		switch (*parm)
		{
		case 0x00:
			this->powerStatus.source = MCU_PWRSRC_CELL;
			// set event : pwrsrc changed to battery
			TLOGMSG(1, ("power source = battery\n"));
			break;

		case 0x01:
			this->powerStatus.source = MCU_PWRSRC_EXTDC;
			// set event : pwrsrc changed to external dc
			TLOGMSG(1, ("power source = external dc\n"));
			break;

		default:
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "invalid external dc status\n", DBGINFO));
			break;
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
	}

	return ret;
}


static int
mcu_parse_version_flag(struct mcu_interface *mcu, char *parm)
{
    int ret = 0;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
    {
        this->version = (int) *parm;
        TLOGMSG(1, ("mcu firmware version = rev.%03d\n", this->version));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_parse_pwrsrc_volt(struct mcu_interface *mcu, char *parm)
{
    int ret = 0;
    short volt = 0;
    static int curr_level = -1;
    static int prev_level = -1;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
    {
        memcpy(&volt, parm, sizeof(short));

        if (this->powerStatus.source == MCU_PWRSRC_CELL)
        {
            if (this->powerStatus.cellType == MCU_CELL_PRIMARY)
                this->powerStatus.cellRemain = 100.0 / PRIMARY_CELL_VOLT_INTV * (volt - PRIMARY_CELL_VOLT_MIN);
            else
                this->powerStatus.cellRemain = 100.0 / SECONDARY_CELL_VOLT_INTV * (volt - SECONDARY_CELL_VOLT_MIN);


            if (this->powerStatus.cellRemain > 40.5)    /* 6200mV */
            {
                if (this->powerStatus.cellStatus != MCU_CELL_STATUS_NORMAL)
                    this->powerStatus.cellStatus = MCU_CELL_STATUS_NORMAL;
            }
            else if (this->powerStatus.cellRemain > 0.5)    /* 6000mV */
            {
                if (this->powerStatus.cellStatus == MCU_CELL_STATUS_NORMAL)
                {
                    this->powerStatus.cellStatus = MCU_CELL_STATUS_LOWVOLT;
                    evque_set_event(EVCODE_LOW_BATTERY, 0);
                }
            }
            else
            {
                if (this->powerStatus.cellStatus == MCU_CELL_STATUS_LOWVOLT)
                {
                    this->powerStatus.cellStatus = MCU_CELL_STATUS_DEPLETED;
                    this->powerStatus.cellRemain = 0.0;
                    evque_set_event(EVCODE_DEAD_BATTERY, 0);
                }
            }

            if (this->powerStatus.cellRemain >= 100.0)
            {
                if (prev_level == -1)
                {
                    prev_level = 10;
                    curr_level = prev_level;
                }
                else
                {
                    curr_level = 10;

                    if (curr_level > prev_level)
                        curr_level = prev_level;
                    else
                        prev_level = curr_level;
                }
            }
            else
            {
                if (prev_level == -1)
                {
                    prev_level = (int) (this->powerStatus.cellRemain / 10) + 1;
                    curr_level = prev_level;
                }
                else
                {
                    curr_level = (int) (this->powerStatus.cellRemain / 10) + 1;

                    if (curr_level > prev_level)
                        curr_level = prev_level;
                    else
                        prev_level = curr_level;
                }
            }

            this->powerStatus.cellLevel = curr_level;
            this->powerStatus.cellVolt = volt * 10;
            TLOGMSG(1, ("internal cell voltage = %dmV\n", this->powerStatus.cellVolt));
            TLOGMSG(0, ("internal cell %%remain = %f%%\n", this->powerStatus.cellRemain));
            TLOGMSG(0, ("internal cell level = %d\n", this->powerStatus.cellLevel));
        }
        else
        {
            curr_level = -1;
            prev_level = -1;
            this->powerStatus.extVolt = (int) volt * 10;
            TLOGMSG(1, ("external dc voltage = %dmV\n", this->powerStatus.extVolt));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_parse_cellthold(struct mcu_interface *mcu, char *parm)
{
    int ret = 0;
    short pcell_thold = 0;
    short scell_thold = 0;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
    {
        memcpy(&pcell_thold, parm, sizeof(short));
        memcpy(&scell_thold, parm + sizeof(short), sizeof(short));
        this->powerStatus.cellThold[0] = (int) pcell_thold;
        this->powerStatus.cellThold[1] = (int) scell_thold;
        TLOGMSG(1, ("low voltage thresholds = (%fmV, %fmV)\n", pcell_thold * 10.0, scell_thold * 10.0));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_parse_message(struct mcu_interface *mcu, char *msg)
{
	int ret = 0;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

	if (this)
	{
		if (msg)
		{
			switch(*(msg + MCU_MSG_OFFSET_CMD))
			{
			case MCU_MSG_CMD_REQ_STATUS:
				ret = mcu_parse_mcu_status(mcu, (msg + MCU_MSG_OFFSET_PARM));
				break;

			case MCU_MSG_CMD_INIT_SHUTTER:
                ret = mcu_parse_init_shutter(mcu);
				break;

            case MCU_MSG_CMD_MOVE_SHUTTER:
            case MCU_MSG_CMD_REQ_SHTRPOS:
                ret = mcu_parse_shutter_position(mcu, (msg + MCU_MSG_OFFSET_PARM));
                break;

			case MCU_MSG_CMD_ENABLE_PROXSEN:
				ret = mcu_parse_proxsen_flag(mcu, (msg + MCU_MSG_OFFSET_PARM));
				break;

			case MCU_MSG_CMD_PROXIMITY_STATUS:
				ret = mcu_parse_proxmity_status(mcu, (msg + MCU_MSG_OFFSET_PARM));
				break;

            case MCU_MSG_CMD_PWRKEY_STATE:
                ret = mcu_parse_pwrkey_state(mcu, (msg + MCU_MSG_OFFSET_PARM));
                break;

            case MCU_MSG_CMD_REQ_PWRSRC_VOLT:
                ret = mcu_parse_pwrsrc_volt(mcu, (msg + MCU_MSG_OFFSET_PARM));
                break;

            case MCU_MSG_CMD_REQ_PWRSRC_TYPE:
                ret = mcu_parse_pwrsrc_type(mcu, (msg + MCU_MSG_OFFSET_PARM));
                break;

			case MCU_MSG_CMD_EXTDC_STATUS:
				ret = mcu_parse_extdc_status(mcu, (msg + MCU_MSG_OFFSET_PARM));
				break;

            case MCU_MSG_CMD_REQ_CELLTHOLD:
                ret = mcu_parse_cellthold(mcu, (msg + MCU_MSG_OFFSET_PARM));
                break;

            case MCU_MSG_CMD_SET_PROXTHOLD:
    			TLOGMSG(1, ("recevice ack - CMD_SET_PROXTHOLD\n"));
    			break;

    		case MCU_MSG_CMD_REQ_PROXTHOLD:
    			TLOGMSG(1, ("proximity threshold = 0x%02X\n", *(msg + MCU_MSG_OFFSET_PARM)));
    			break;

            case MCU_MSG_CMD_MONITOR_PWRSRC_VOLT:
                TLOGMSG(1, ("received ack - CMD_MONITOR_PWRSRC_VOLT\n"));
                break;

            case MCU_MSG_CMD_PWROFF:
                TLOGMSG(1, ("received ack - CMD_PWROFF\n"));
                break;

			case MCU_MSG_CMD_PWRSAVE:
				TLOGMSG(1, ("received ack - CMD_PWRSAVE\n"));
				break;

            case MCU_MSG_CMD_REQ_VERSION:
                ret = mcu_parse_version_flag(mcu, (msg + MCU_MSG_OFFSET_PARM));
                break;

			default:
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "unknown command - stop message parsing\n", DBGINFO));
				break;
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "null message\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
	}

	return ret;
}


static void *
mcu_read_message(void *arg)
{
    int nread = 0;
    int offset = 0;
    int ret = 0;
    double timeo = 0.0;
    char chksum = 0;
    char msg[16] = {0};
    char buf[128] = {0};

    struct mcu_interface *mcu = (struct mcu_interface *) arg;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;
    struct uart_interface *uart = this->uart;
    struct pollfd pfd = {.fd = uart->getFileDesc(uart), .events = POLLIN, .revents = 0};
    struct timespec ts = {.tv_sec = 0, .tv_nsec = 0};

    this->running = true;
    memset(buf, 0x00, sizeof(buf));
    memset(msg, 0x00, sizeof(msg));
    tcflush(uart->getFileDesc(uart), TCIOFLUSH);

    while (this->running)
    {
        ret = poll(&pfd, 1, 1000);

        if (ret > 0)
        {
            nread = uart->read(uart, buf, uart->getNumRead(uart));

            if (nread > 0)
            {
                for (int i = 0 ; i < nread; i++)
                {
                    if (msg[MCU_MSG_OFFSET_SOM] == MCU_MSG_SOM)
                    {
                        if (offset < MCU_MSG_LENGTH)
                        {
                            msg[offset++] = buf[i];

                            if (msg[MCU_MSG_OFFSET_LEN] == offset)
                            {
                                chksum = calculate_checksum(msg, msg[MCU_MSG_OFFSET_LEN]);

                                if (chksum == msg[msg[MCU_MSG_OFFSET_LEN] - 1])
                                    mcu_parse_message(mcu, msg);
                                else
                                    TLOGMSG(1, (DBGINFOFMT "checksum mismatch (0x%02X, 0x%02X)\n", chksum, msg[msg[MCU_MSG_OFFSET_LEN] - 1]));

                                offset = 0;
                                memset(msg, 0x00, sizeof(msg));
                                mcu_recv_ack(mcu);
                                this->error &= ~(MCU_ERROR_COMM);
                            }
                            else
                                continue;
                        }
                        else
                        {
                            offset = 0;
                            memset(msg, 0x00, sizeof(msg));
                        }
                    }
                    else
                    {
                        if (buf[i] == MCU_MSG_SOM)
                        {
                            msg[offset++] = MCU_MSG_SOM;

                            if (this->waitAck == false)
                                clock_gettime(CLOCK_REALTIME, &ts);
                        }
                        else
                            continue;
                    }
                }

                memset(buf, 0x00, sizeof(buf));
            }
            else
                continue;
        }
        else
        {
            if (this->waitAck)
            {
                if (this->shutter.motion)
                    timeo = 9.0;
                else
                    timeo = 3.0;

                 ret = check_timeout(&this->timeStamp, timeo);
            }
            else
                 ret = check_timeout(&ts, 3.0);

            if (ret != 0)
            {
                offset = 0;
                this->error |= MCU_ERROR_COMM;
                memset(buf, 0x00, sizeof(buf));
                memset(msg, 0x00, sizeof(msg));
                mcu_recv_ack(mcu);
            }
            else
                continue;
        }
    }

    return NULL;
}


static char
mcu_get_message_length(char cmd)
{
    char length = 0;

    switch (cmd)
    {
    case MCU_MSG_CMD_REQ_STATUS:
	case MCU_MSG_CMD_INIT_SHUTTER:
	case MCU_MSG_CMD_REQ_PROXTHOLD:
    case MCU_MSG_CMD_PWRKEY_STATE:
    case MCU_MSG_CMD_REQ_PWRSRC_TYPE:
    case MCU_MSG_CMD_REQ_CELLTHOLD:
    case MCU_MSG_CMD_PWROFF:
	case MCU_MSG_CMD_PWRSAVE:
    case MCU_MSG_CMD_REQ_VERSION:
        length = 0x04;
        break;

	case MCU_MSG_CMD_MOVE_SHUTTER:
	case MCU_MSG_CMD_ENABLE_PROXSEN:
	case MCU_MSG_CMD_SET_PROXTHOLD:
    case MCU_MSG_CMD_REQ_PWRSRC_VOLT:
        length = 0x05;
        break;

    case MCU_MSG_CMD_MONITOR_PWRSRC_VOLT:
        length = 0x06;
        break;

    case MCU_MSG_CMD_SET_CELLTHOLD:
        length = 0x08;
        break;

    default:
        length = -1;
        TLOGMSG(1, (DBGINFOFMT "unknown command\n", DBGINFO));
        break;
    }

    return length;
}


static int
mcu_make_message(char cmd, char *parm, char *msg)
{
    int ret = 0;
    char length = 0;

    if (msg)
    {
        length = mcu_get_message_length(cmd);

        if (length != -1)
        {
            *(msg + MCU_MSG_OFFSET_SOM) = MCU_MSG_SOM;
            *(msg + MCU_MSG_OFFSET_LEN) = length;
            *(msg + MCU_MSG_OFFSET_CMD) = cmd;

			if (parm)
            {
				memcpy( msg + MCU_MSG_OFFSET_PARM, parm, sizeof(char) * (length - 4));
                *(msg + length - 1) = calculate_checksum(msg, length);
            }
            else
                *(msg + length - 1) = calculate_checksum(msg, length);

        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to get message length\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_load_proxthold(struct mcu_interface *mcu)
{
    int ret = 0;
    FILE *pfile = NULL;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
    {
        pfile = fopen(MCU_PATH_PROXTHOLD_CONF, "r");

        if (pfile)
        {
            fscanf(pfile, "%d %d", &(this->proxSensor.threshold[0]), &(this->proxSensor.threshold[0]));
            fclose(pfile);
            TLOGMSG(1, ("proximity threshold = (%d, %d)\n", this->proxSensor.threshold[0], this->proxSensor.threshold[1]));
        }
        else
        {
            this->proxSensor.threshold[0] = MCU_DEFAULT_OPEN_PROXTHOLD;
            this->proxSensor.threshold[1] = MCU_DEFAULT_CLOSE_PROXTHOLD;
            TLOGMSG(1, ("load default proximity threshold\n"));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}

static int
mcu_request_status(struct mcu_interface *mcu)
{
    int ret = 0;
    char msg[MCU_MSG_LENGTH] = {0};
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
    {
        memset(msg, 0x00, sizeof(msg));

		if (mcu_make_message(MCU_MSG_CMD_REQ_STATUS, NULL, msg) == 0)
		{
			if (mcu_write_message(mcu, msg) == 0)
				mcu_wait_ack(mcu);
			else
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "failed to make message\n", DBGINFO));
		}
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_init_shutter(struct mcu_interface *mcu)
{
	int ret = 0;
	char msg[MCU_MSG_LENGTH] = {0};
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

	if (this)
	{
		memset(msg, 0x00, sizeof(msg));

		if (mcu_make_message(MCU_MSG_CMD_INIT_SHUTTER, NULL, msg) == 0)
		{
			if (mcu_write_message(mcu, msg) == 0)
            {
                this->shutter.motion = true;
				mcu_wait_ack(mcu);
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
			TLOGMSG(1, (DBGINFOFMT "failed to make message\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
	}

	return ret;
}


static int
mcu_move_shutter(struct mcu_interface *mcu, int pos)
{
    int ret = 0;
    char parm = 0;
    char msg[MCU_MSG_LENGTH] = {0};
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
    {
        switch (pos)
        {
        case MCU_SHUTTER_CLOSE:
            if (this->shutter.close == false)
                parm = (char) pos;
            else
            {
                parm = MCU_SHUTTER_UNKNOWN;
                TLOGMSG(1, ("shutter is already close\n"));
            }

            break;

        case MCU_SHUTTER_LXMVT: case MCU_SHUTTER_HXMVT:
            if (this->shutter.filter != pos)
                parm = (char) pos;
            else
            {
                parm = MCU_SHUTTER_UNKNOWN;
                TLOGMSG(1, ("shutter is already in position\n"));
            }

            break;

        case MCU_SHUTTER_OPEN:
            if (this->shutter.close == true)
                parm = this->shutter.filter;
            else
            {
                parm = MCU_SHUTTER_UNKNOWN;
                TLOGMSG(1, ("shutter is already open\n"));
            }

            break;

        default:
            parm = MCU_SHUTTER_UNKNOWN;
            break;
        }

        if (parm != MCU_SHUTTER_UNKNOWN)
        {
            memset(msg, 0x00, sizeof(msg));

            if (mcu_make_message(MCU_MSG_CMD_MOVE_SHUTTER, &parm, msg) == 0)
            {
                if (mcu_write_message(mcu, msg) == 0)
                {
                    this->shutter.motion = true;
                    mcu_wait_ack(mcu);
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
                TLOGMSG(1, (DBGINFOFMT "failed to make message\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid position parameter or already in postion\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n"));
    }

    return ret;
}


static int
mcu_request_shtrpos(struct mcu_interface *mcu)
{
    int ret = 0;
    char msg[MCU_MSG_LENGTH] = {0};
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
    {
        memset(msg, 0x00, sizeof(msg));

        if (mcu_make_message(MCU_MSG_CMD_REQ_SHTRPOS, NULL, msg) == 0)
        {
            if (mcu_write_message(mcu, msg) == 0)
                mcu_wait_ack(mcu);
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_enable_proxsen(struct mcu_interface *mcu, bool flag)
{
    int ret = 0;
    char parm = (char) flag;
    char msg[MCU_MSG_LENGTH] = {0};
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
    {
        memset(msg, 0x00, sizeof(msg));

        if (mcu_make_message(MCU_MSG_CMD_ENABLE_PROXSEN, &parm, msg) == 0)
        {
            if (mcu_write_message(mcu, msg) == 0)
                mcu_wait_ack(mcu);
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to make message\n", DBGINFO));
        }
    }
    else
    {
        ret = - 1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n"));
    }

    return ret;
}


static int
mcu_set_proximity_threshold(struct mcu_interface *mcu, int mode)
{
	int ret = 0;
	char thold = 0;
	char msg[MCU_MSG_LENGTH] = {0};
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

	if (this)
	{
		switch (mode)
		{
		case MCU_PROXTHOLD_OPEN:
			if (this->proxSensor.threshold[0] > 0xFF)
				thold = 0xFF;
			else if (this->proxSensor.threshold[0] < 0x00)
				thold = 0x00;
			else
				thold = (char) this->proxSensor.threshold[0];
			break;

		case MCU_PROXTHOLD_CLOSE:
			if (this->proxSensor.threshold[1] > 0xFF)
				thold = 0xFF;
			else if (this->proxSensor.threshold[1] < 0x00)
				thold = 0x00;
			else
				thold = (char) this->proxSensor.threshold[1];
			break;

		default:
			thold = -1;
			break;
		}

		if (thold != -1)
		{
			memset(msg, 0x00, sizeof(msg));

			if (mcu_make_message(MCU_MSG_CMD_SET_PROXTHOLD, &thold, msg) == 0)
			{
				if (mcu_write_message(mcu, msg) == 0)
				{
					mcu_wait_ack(mcu);
					TLOGMSG(1, ("proximity threshold = 0x%02X\n", thold));
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
				TLOGMSG(1, (DBGINFOFMT "failed to make message\n", DBGINFO));
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "invalid proximity threshold mode\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
	}

	return ret;
}


static int
mcu_request_proximity_threshold(struct mcu_interface *mcu)
{
	int ret = 0;
	char msg[MCU_MSG_LENGTH] = {0};
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

	if (this)
	{
		memset(msg, 0x00, sizeof(msg));

		if (mcu_make_message(MCU_MSG_CMD_REQ_PROXTHOLD, NULL, msg) == 0)
		{
			if (mcu_write_message(mcu, msg) == 0)
				mcu_wait_ack(mcu);
			else
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "failed to make message\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
	}

	return ret;
}


static int
mcu_request_pwrkey_state(struct mcu_interface *mcu)
{
    int ret = 0;
    char msg[MCU_MSG_LENGTH] = {0};
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
    {
        memset(msg, 0x00, sizeof(msg));

        if (mcu_make_message(MCU_MSG_CMD_PWRKEY_STATE, NULL, msg) == 0)
        {
            if (mcu_write_message(mcu, msg) == 0)
                mcu_wait_ack(mcu);
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to make message\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "nulll mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_request_pwrsrc_type(struct mcu_interface *mcu)
{
    int ret = 0;
    char msg[MCU_MSG_LENGTH] = {0};
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
    {
        memset(msg, 0x00, sizeof(msg));

        if (mcu_make_message(MCU_MSG_CMD_REQ_PWRSRC_TYPE, NULL, msg) == 0)
        {
            if (mcu_write_message(mcu, msg) == 0)
                mcu_wait_ack(mcu);
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to make message\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "nulll mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_request_pwrsrc_voltage(struct mcu_interface *mcu, char mode)
{
    int ret = 0;
    char parm = 0;
    char msg[MCU_MSG_LENGTH] = {0};
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
    {
        switch (mode)
        {
        case 0x00:
        case 0x01:
            parm = mode;
            break;

        default:
            parm = -1;
            break;
        }

        if (parm != -1)
        {
            memset(msg, 0x00, sizeof(msg));

            if (mcu_make_message(MCU_MSG_CMD_REQ_PWRSRC_VOLT, &parm, msg) == 0)
            {
                if (mcu_write_message(mcu, msg) == 0)
                    mcu_wait_ack(mcu);
                else
                {
                    ret = -1;
                    TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
                }
            }
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "failed to make message\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid parameter\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "nulll mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_monitor_pwrsrc_voltage(struct mcu_interface *mcu, char mode, char intv)
{
    int ret = 0;
    char parm[2] = {mode, intv};
    char msg[MCU_MSG_LENGTH] = {0};
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
    {
        memset(msg, 0x00, sizeof(msg));

        if (mcu_make_message(MCU_MSG_CMD_MONITOR_PWRSRC_VOLT, &parm[0], msg) == 0)
        {
            if (mcu_write_message(mcu, msg) == 0)
                mcu_wait_ack(mcu);
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to make message\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "nulll mcu interface\n", DBGINFO));
     }

    return ret;
}


static int
mcu_set_cellthold(struct mcu_interface *mcu, unsigned short primary, unsigned short secondary)
{
    int ret = 0;
    char msg[MCU_MSG_LENGTH] = {0};
    unsigned short parm[2] = {0};
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
    {
        if (primary > MCU_MAX_CELLTHOLD)
            parm[0] = MCU_MAX_CELLTHOLD;
        else if (primary < MCU_MIN_CELLTHOLD)
            parm[0] = MCU_MIN_CELLTHOLD;
        else
            parm[0] = primary;

        if (secondary > MCU_MAX_CELLTHOLD)
            parm[1] = MCU_MAX_CELLTHOLD;
        else if (secondary < MCU_MIN_CELLTHOLD)
            parm[1] = MCU_MIN_CELLTHOLD;
        else
            parm[1] = secondary;

        memset(msg, 0x00, sizeof(msg));

        if (mcu_make_message(MCU_MSG_CMD_SET_CELLTHOLD, (char *) &parm[0], msg) == 0)
        {
            if (mcu_write_message(mcu, msg) == 0)
                mcu_wait_ack(mcu);
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to make message\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "nulll mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_request_cellthold(struct mcu_interface *mcu)
{
    int ret = 0;
    char msg[MCU_MSG_LENGTH] = {0};
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
    {
        memset(msg, 0x00, sizeof(msg));

        if (mcu_make_message(MCU_MSG_CMD_REQ_CELLTHOLD, NULL, msg) == 0)
        {
            if (mcu_write_message(mcu, msg) == 0)
                mcu_wait_ack(mcu);
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to make message\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "nulll mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_disable_pmic(struct mcu_interface *mcu)
{
    int ret = 0;
    char msg[MCU_MSG_LENGTH] = {0};
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
    {
        memset(msg, 0x00, sizeof(msg));

        if (mcu_make_message(MCU_MSG_CMD_PWROFF, NULL, msg) == 0)
        {
            if (mcu_write_message(mcu, msg) == 0)
                mcu_wait_ack(mcu);
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to make message\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "nulll mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_enter_pwrsave_mode(struct mcu_interface *mcu)
{
	int ret = 0;
	char msg[MCU_MSG_LENGTH] = {0};
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

	if (this)
	{
		memset(msg, 0x00, sizeof(msg));

		if (mcu_make_message(MCU_MSG_CMD_PWRSAVE, NULL, msg) == 0)
		{
			if (mcu_write_message(mcu, msg) == 0)
				mcu_wait_ack(mcu);
			else
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
			}
		}
	    else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "failed to make message\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
	}

	return ret;
}


static int
mcu_request_version(struct mcu_interface *mcu)
{
    int ret = 0;
    char msg[MCU_MSG_LENGTH] = {0};
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
    {
        memset(msg, 0x00, sizeof(msg));

        if (mcu_make_message(MCU_MSG_CMD_REQ_VERSION, NULL, msg) == 0)
        {
            if (mcu_write_message(mcu, msg) == 0)
                mcu_wait_ack(mcu);
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "failed to write message\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to make message\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_get_shutter_position(struct mcu_interface *mcu)
{
    int ret = 0;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
    {
        if (this->shutter.close == false)
            ret = this->shutter.filter;
        else
            ret = MCU_SHUTTER_CLOSE;
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_get_version(struct mcu_interface *mcu)
{
    int ret = 0;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
        ret = this->version;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_get_pwrsrc_type(struct mcu_interface *mcu)
{
    int ret = 0;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
        ret = this->powerStatus.source;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_get_pwrsrc_volt(struct mcu_interface *mcu)
{
    int ret = 0;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
    {
        if (this->powerStatus.source == MCU_PWRSRC_CELL)
            ret = this->powerStatus.cellVolt;
        else
            ret = this->powerStatus.extVolt;
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_get_cell_level(struct mcu_interface *mcu)
{
    int ret = 0;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
        ret = this->powerStatus.cellLevel;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_get_cell_type(struct mcu_interface *mcu)
{
    int ret = 0;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
        ret = this->powerStatus.cellType;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_test_module(struct mcu_interface *mcu)
{
    int ret = 0;
    int filter = MCU_SHUTTER_HXMVT;
    bool cls = false;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
    {
        if (this->error & MCU_ERROR_SHUTTER)
        {
            filter = MCU_SHUTTER_HXMVT;
            cls = false;
        }
        else
        {
            filter = this->shutter.filter;
            cls = this->shutter.close;
        }

        mcu_init_shutter(mcu);
        mcu_request_status(mcu);
        MSLEEP(1500);

        if (cls)
        {
            mcu_move_shutter(mcu, filter);
            mcu_move_shutter(mcu, MCU_SHUTTER_CLOSE);
        }
        else
            mcu_move_shutter(mcu, filter);

        mcu_request_status(mcu);

        if ((cls != this->shutter.close) || (filter != this->shutter.filter))
            this->error |= MCU_ERROR_SHUTTER;
        else
            this->error &= ~MCU_ERROR_SHUTTER;

        ret = this->error;
        TLOGMSG(1, ("mcu module test result = 0x%02X\n", this->error));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_get_error(struct mcu_interface *mcu)
{
    int ret = 0;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
        ret = this->error;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}



static int
mcu_enter_standby_mode(struct mcu_interface *mcu)
{
    int ret = 0;
    struct uart_interface *uart = NULL;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
    {
        if (!this->standby)
        {
            uart = this->uart;
            this->running = false;
            pthread_join(this->tid, NULL);
            uart->close(uart);
            this->standby = true;
            TLOGMSG(1, ("mcu module is in standby mode now\n"));
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "mcu module is already in standby mode\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_exit_standby_mode(struct mcu_interface *mcu)
{
    int ret = 0;
    struct uart_interface *uart = NULL;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
    {
        if (this->standby)
        {
            uart = this->uart;
            uart->open(uart, UART_TTYMXC2, B115200);

            if (pthread_create(&this->tid, NULL, mcu_read_message, (void *)mcu) == 0)
            {
                this->standby = false;
                TLOGMSG(1, ("mcu is in normal mode\n"));
            }
            else
            {
                ret = -1;
                uart->close(uart);
                TLOGMSG(1, (DBGINFOFMT "failed to wake up\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "mcu module is already in normal mode\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_init_attribute(struct mcu_interface *mcu)
{
    int ret = 0;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
    {
        this->uart = uart_create();

        if (this->uart)
        {
            this->uart->open(this->uart, UART_TTYMXC2, B115200);

            this->error = MCU_ERROR_NONE;
            this->version = 0;
            this->waitAck = false;
            this->running = false;
            this->standby = false;
            this->timeStamp.tv_sec  = 0;
            this->timeStamp.tv_nsec = 0;

            this->shutter.close  = false;
            this->shutter.filter = MCU_SHUTTER_HXMVT;
            this->shutter.motion = false;

            this->proxSensor.enable = false;
            this->proxSensor.close  = true;
            this->proxSensor.threshold[0] = 0;
            this->proxSensor.threshold[1] = 0;

            this->powerStatus.powerKey      = MCU_PWRKEY_PRESS;
            this->powerStatus.monitor       = false;
            this->powerStatus.source        = MCU_PWRSRC_CELL;
            this->powerStatus.cellLevel     = 0;
            this->powerStatus.cellType      = MCU_CELL_PRIMARY;
            this->powerStatus.cellVolt      = 0;
            this->powerStatus.cellRemain    = 0.0;
            this->powerStatus.cellThold[0]  = 0;
            this->powerStatus.cellThold[1]  = 0;
            this->powerStatus.cellStatus    = 0;;
            this->powerStatus.extVolt       = 0;
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
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}


static int
mcu_deinit_attribute(struct mcu_interface *mcu)
{
    int ret = 0;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
        uart_destroy(this->uart);
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu attribute\n", DBGINFO));
    }

    return ret;
}


struct mcu_interface *
mcu_create(void)
{
    struct mcu_interface *mcu = NULL;
    struct mcu_attribute *this = malloc(sizeof(struct mcu_attribute));

    if (this)
    {
        memset(this, 0x00, sizeof(struct mcu_attribute));

        mcu = &(this->extif);
        mcu->getBatteryLevel  = mcu_get_cell_level;
        mcu->getBatteryType   = mcu_get_cell_type;
        mcu->getPwrSrcType    = mcu_get_pwrsrc_type;
        mcu->getPwrSrcVolt    = mcu_get_pwrsrc_volt;
        mcu->getShutterPos    = mcu_get_shutter_position;
        mcu->getVersion       = mcu_get_version;
        mcu->setShutterPos    = mcu_move_shutter;
        mcu->enterPwrSaveMode = mcu_enter_pwrsave_mode;
        mcu->disablePmic      = mcu_disable_pmic;
        mcu->testModule       = mcu_test_module;
        mcu->getError         = mcu_get_error;
        mcu->standby          = mcu_enter_standby_mode;
        mcu->wakeup           = mcu_exit_standby_mode;

        if (mcu_init_attribute(mcu) == 0)
        {
            pthread_mutex_init(&this->mtx, NULL);
            pthread_cond_init(&this->cond, NULL);

            if (pthread_create(&this->tid, NULL, mcu_read_message, (void *)mcu) == 0)
            {
                MSLEEP(100);
                mcu_request_version(mcu);
                mcu_request_pwrsrc_type(mcu);
                mcu_request_pwrsrc_voltage(mcu, 0x02);
                mcu_set_cellthold(mcu, PRIMARY_CELL_VOLT_MIN, SECONDARY_CELL_VOLT_MIN);
                mcu_monitor_pwrsrc_voltage(mcu, 0x2, 0x0A);
                mcu_load_proxthold(mcu);
                TLOGMSG(1, ("create mcu interface\n"));
            }
            else
            {
                pthread_mutex_destroy(&this->mtx);
                pthread_cond_destroy(&this->cond);
                mcu_deinit_attribute(mcu);
                free(this);
                mcu = NULL;
                TLOGMSG(1, (DBGINFOFMT "failed to create mcu interface, failed to create thread\n", DBGINFO));
            }
        }
        else
        {
            free(this);
            mcu = NULL;
            TLOGMSG(1, (DBGINFOFMT "failed to create mcu interface, failed to init attributes\n", DBGINFO));
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create mcu interface, malloc return null\n", DBGINFO));

    return mcu;
}


int
mcu_destroy(struct mcu_interface *mcu)
{
    int ret = 0;
    struct mcu_attribute *this = (struct mcu_attribute *) mcu;

    if (this)
    {
        this->running = false;
        pthread_join(this->tid, NULL);
        pthread_mutex_destroy(&this->mtx);
        pthread_cond_destroy(&this->cond);
        mcu_deinit_attribute(mcu);
        free(this);
        TLOGMSG(1, ("destroy mcu interface\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null mcu interface\n", DBGINFO));
    }

    return ret;
}
