/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    device.c
        external/internal function implementations of device interface
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>

#include "core/device.h"
#include "core/logger.h"
#include "modules/gpio.h"
#include "etc/util.h"


/* structure declaration : device attributes */
struct device_attribute
{
    /* external interface */
    struct device_interface extif;

    /* internal attribute */
    int powerMode;
    int moduleStatus;
    int bitProgress[DEVICE_NUM_BIT_ITEMS];
    int bitResult[DEVICE_NUM_BIT_ITEMS];

    struct gpio_interface *main5v;
};


static void
device_set_timezone(int offset)
{
    int ret = 0;
    char val[32] = {0};

    memset(val, 0x00, sizeof(val));
    snprintf(val, sizeof(val), "local%+03d:00:00", -offset);
    setenv("TZ", val, 1);
}


static int
device_memtest(void)
{
    int ret = 0;
    char *ptr = NULL;
    char pattern[4] = {0xAA, 0xFF, 0x55, 0x00};

/*
    for (int i = 0; i < 10; i++)
    {
        ptr = malloc(sizeof(char) * 1024 * 1024);

        if (ptr)
        {
            for (int j = 0; j < 4; j++)
            {
                TLOGMSG(1, ("memory test, address = %p, size = %d, pattern = 0x%02X\n", ptr, 1024 * 1024, pattern[j]));

                memset(ptr, pattern[j], sizeof(char) * 1024 * 1024);

                for (int k = 0; k < sizeof(char) * 1024 * 1024; k++)
                {
                    if (pattern[j] != *(ptr + k))
                    {
                        ret = -1;
                        TLOGMSG(1, (DBGINFOFMT "pattern mismatching\n", DBGINFO));
                        break;
                    }
                }

                if (ret != 0)
                    break;
            }

            free(ptr);

            if (ret != 0)
                break;
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));
            break;
        }
    }
*/
    return ret;
}


static int
device_suspend_cpu(struct device_interface *device)
{
    int fd = 0;
    int ret = 0;
    int nwr = 0;
    char *str = "mem";
    char buf[16] = {0};
    struct device_attribute *this = (struct device_attribute *) device;

    if (this)
    {
        if (this->powerMode == POWER_MODE_STANDBY)
        {
            fd = open("/sys/power/state", O_RDWR);

            if (fd < 0)
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "open return fail\n", DBGINFO));
            }
            else
            {
                nwr = snprintf(buf, sizeof(buf),  "%s", str);

                if (nwr > 0)
                    write(fd, buf, nwr);
                else
                {
                    ret = -1;
                    TLOGMSG(1, (DBGINFOFMT "snprintf return fail, %s\n", DBGINFO, strerror(errno)));
                }

                close(fd);
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "device is not in standby mode\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null device interface\n", DBGINFOFMT));
    }

    return ret;
}



static int
device_compensate_system_time(struct device_interface *device)
{
    int ret = 0;
    int offset = 0;
    FILE *file = NULL;
    char *parm = "/mnt/mmc/mfod-data/param/offset_utc.parm";
    struct device_attribute *this = (struct device_attribute *) device;


    if (this)
    {
        file = fopen(parm, "r");

        if (file)
        {
            fscanf(file, "%d", &offset);
            set_utc_offset(offset);
            device_set_timezone(offset);
            fclose(file);
            TLOGMSG(1, ("load utc offset = %d\n", offset));
        }
        else
        {
            file = fopen(parm, "w");

            if (file)
            {
                fprintf(file, "%d", offset);
                set_utc_offset(offset);
                device_set_timezone(offset);
                fclose(file);
                TLOGMSG(1, ("load default utc offset\n"));
            }
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "failed to open file, %s\n", DBGINFO, strerror(errno)));
            }
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null device interface\n", DBGINFO));
    }

    return ret;
}


static int
device_save_utc_offset(struct device_interface *device, int offset)
{
    int ret = 0;
    FILE *pfile = NULL;
    struct device_attribute *this = (struct device_attribute *) device;

    if (this)
    {
        pfile = fopen("/mnt/mmc/amod-data/param/offset_utc.parm", "w");

        if (pfile)
        {
            fprintf(pfile, "%d", offset);
            fclose(pfile);
            set_utc_offset(offset);
            device_set_timezone(offset);
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "fopen return null, %s\n", DBGINFO, strerror(errno)));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null device interface\n", DBGINFO));
    }

    return ret;
}


static int
device_get_systime(struct device_interface *device, char *buf_date, char *buf_time, int len_date, int len_time)
{
    int ret = 0;
    struct tm *tm = NULL;
    struct timespec ts = {0, 0};
    struct device_attribute *this = (struct device_attribute *) device;

    if (this)
    {
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += get_time_offset();
        tm = localtime(&ts.tv_sec);

        if (tm)
        {
            strftime(buf_time, len_time, "%T", tm);
            strftime(buf_date, len_date, "%Y.%m.%d", tm);
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "gmtime return null, %s\n", DBGINFO, strerror(errno)));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null device interface\n", DBGINFO));
    }

    return ret;
}


static int
device_update_systime(struct device_interface *device)
{
    int fd = 0;
    int ret = 0;
    struct tm *tm = NULL;
    struct timespec ts = {0, 0};
    struct rtc_time rt = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    struct device_attribute *this = (struct device_attribute *) device;

    if (this)
    {
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += get_time_offset();
        tm = gmtime(&ts.tv_sec);

        if (tm)
        {
            fd = open("/dev/rtc0", O_RDWR, 0);

            if (fd != -1)
            {
                rt.tm_year = tm->tm_year;
                rt.tm_mon = tm->tm_mon;
                rt.tm_mday = tm->tm_mday;
                rt.tm_hour = tm->tm_hour;
                rt.tm_min = tm->tm_min;
                rt.tm_sec = tm->tm_sec;
                rt.tm_wday = tm->tm_wday;
                rt.tm_yday = tm->tm_yday;
                rt.tm_isdst = tm->tm_isdst;

                if (ioctl(fd, RTC_SET_TIME, &rt) == 0)
                {
                    TLOGMSG(1, ("date set to %d.%d.%d\n", rt.tm_year + 1900, rt.tm_mon + 1, rt.tm_mday));
                    TLOGMSG(1, ("time set to %02d:%02d%02d\n", rt.tm_hour, rt.tm_min, rt.tm_sec));
                }
                else
                    TLOGMSG(1, (DBGINFOFMT, "failed to set rtc\n", DBGINFO));

                close(fd);
            }
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "failed to open rtc\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "gmtime return null, %s\n", DBGINFO, strerror(errno)));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null device interface\n", DBGINFO));
    }

    return ret;
}


static int
device_enter_standby_mode(struct device_interface *device)
{
    int fd = 0;
    int ret = 0;
    struct device_attribute *this = (struct device_attribute *) device;

    if (this)
    {
        if (this->powerMode == POWER_MODE_NORMAL)
        {
            device->gnss->standby(device->gnss);
            device->dmc->standby(device->dmc);
            device->lrf->standby(device->lrf);
            device->mcu->standby(device->mcu);
            device->disp->powerOff(device->disp);
            //this->attr->main5v->setval(this->attr->main5v, GPIO_VALUE_LOW);
            this->powerMode = POWER_MODE_STANDBY;
            MSLEEP(1000);
            TLOGMSG(1, ("enter standby mode\n"));
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "already in standby state\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null device interface\n", DBGINFO));
    }

    return ret;
}


static int
device_exit_standby_mode(struct device_interface *device)
{
    int ret = 0;
    struct device_attribute *this = (struct device_attribute *) device;

    if (this)
    {
        if (this->powerMode == POWER_MODE_STANDBY)
        {
            //this->attr->main5v->setval(this->attr->main5v, GPIO_VALUE_HIGH);
            device->disp->powerOn(device->disp);
            device->disp->setBright(device->disp, DISP_BRIGHT_DV, DISP_BRIGHT_DV_DEFAULT);
            device->mcu->wakeup(device->mcu);
            device->gnss->wakeup(device->gnss);
            device->dmc->wakeup(device->dmc);
            device->lrf->wakeup(device->lrf);
            this->powerMode = POWER_MODE_WAKEUP_INPROC;
            TLOGMSG(1, ("exit standby mode\n"));
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "already in wake up state\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null device interface\n", DBGINFO));
    }

    return ret;
}


static int
device_get_power_mode(struct device_interface *device)
{
    int ret = 0;
    struct device_attribute *this = (struct device_attribute *) device;

    if (this)
        ret = (int) this->powerMode;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null device interface\n", DBGINFO));
    }

    return ret;
}


static int
device_set_power_mode(struct device_interface *device, int mode)
{
    int ret = 0;
    struct device_attribute *this = (struct device_attribute *) device;

    if (this)
    {
        if ((mode >= POWER_MODE_NORMAL) && (mode <= POWER_MODE_WAKEUP_INPROC))
            this->powerMode = mode;
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid power mode flag\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null device interface\n", DBGINFO));
    }

    return ret;
}


static int
device_execute_bit(struct device_interface *device)
{
    int ret = 0;
    int errcode = 0;
    struct lrf_interface *lrf = NULL;
    struct dmc_interface *dmc = NULL;
    struct mcu_interface *mcu = NULL;
    struct gnss_interface *gnss = NULL;
    struct vgss_interface *vgss = NULL;
    struct ircam_interface *ircam = NULL;
    struct device_attribute *this = (struct device_attribute *) device;

    if (this)
    {
        lrf = device->lrf;
        dmc = device->dmc;
        mcu = device->mcu;
        gnss = device->gnss;
        vgss = device->vgss;
        ircam = device->ircam;
        memset(&(this->bitResult[0]), 0x00, sizeof(int) * DEVICE_NUM_BIT_ITEMS);
        memset(&(this->bitProgress[0]), DEVICE_BIT_PROGRESS_STANDBY, sizeof(int) * DEVICE_NUM_BIT_ITEMS);
        TLOGMSG(1, ("start bit...\n"));

        for (int i = 0; i < DEVICE_NUM_BIT_ITEMS; i++)
        {
            this->bitProgress[i] = DEVICE_BIT_PROGRESS_INPROC;

            switch (i)
            {
            case DEVICE_MODULE_MEMORY:
                errcode = device_memtest();

                if (errcode == 0)
                    TLOGMSG(1, ("pbit - pass memory test\n"));
                else
                {
                    TLOGMSG(1, ("pbit - failed memory test\n"));
                    this->moduleStatus |= DEVICE_ERROR_MEMTEST;
                }

                break;

            case DEVICE_MODULE_LRF:
                errcode = lrf->testModule(lrf);

                if (errcode != LRF_ERROR_NONE)
                {
                    if (errcode & LRF_ERROR_XMTR)
                    {
                        this->moduleStatus |= DEVICE_ERROR_LRF_XMTR;
                        TLOGMSG(1, ("pbit - laser transmitter error\n"));
                    }

                    if (errcode & LRF_ERROR_COMM)
                    {
                        this->moduleStatus |= DEVICE_ERROR_LRF_COMM;
                        TLOGMSG(1, ("pbit - communication, lrf module\n"));
                    }
                }
                else
                    TLOGMSG(1, ("pbit - pass lrf module test\n"));

                break;

            case DEVICE_MODULE_DMC:
                errcode = dmc->testModule(dmc);

                if (errcode != DMC_ERROR_NONE)
                {
                    if (errcode & DMC_ERROR_COMM)
                    {
                        this->moduleStatus |= DEVICE_ERROR_DMC_COMM;
                        TLOGMSG(1, ("pbit - communication error, dmc module\n"));
                    }

                    if (errcode & DMC_ERROR_MCPU)
                    {
                        this->moduleStatus |= DEVICE_ERROR_DMC_MCPU;
                        TLOGMSG(1, ("pbit - dmc module processor error?\n"));
                    }
                }
                else
                {
                    TLOGMSG(1, ("pbit - pass dmc module test\n"));
                    dmc->recallMagParm(dmc, MAGPARM_FACTORY);
                    dmc->measure(dmc, DMC_CONTINUOUS_MEASURE);
                }

                break;

            case DEVICE_MODULE_GNSS:
                errcode = gnss->testModule(gnss);

                if (errcode != GNSS_ERROR_NONE)
                {
                    if (errcode & GNSS_ERROR_COMM)
                    {
                        this->moduleStatus |= DEVICE_ERROR_GNSS_COMM;
                        TLOGMSG(1, ("bit - communication error, gnss module\n"));
                    }
                }
                else
                    TLOGMSG(1, ("bit - pass gnss module test\n"));

                break;

            case DEVICE_MODULE_DVO:
                errcode = mcu->testModule(mcu);

                if (errcode != MCU_ERROR_NONE)
                {
                    if (errcode & MCU_ERROR_COMM)
                    {
                        this->moduleStatus |= DEVICE_ERROR_MCU_COMM;
                        TLOGMSG(1, ("bit - communication error, mcu module\n"));
                    }

                    if (errcode & MCU_ERROR_SHUTTER)
                    {
                        this->moduleStatus |= DEVICE_ERROR_MCU_NDFILTER;
                        TLOGMSG(1, ("bit - filter control error\n"));
                    }
                }
                else
                    TLOGMSG(1, ("bit - pass mcu module test\n"));

                break;

            case DEVICE_MODULE_IRCAM:
                errcode = ircam->testModule(ircam);

                if (errcode != IRCAM_ERROR_NONE)
                {
                    if ((errcode & IRCAM_ERROR_BOOT) || (errcode & IRCAM_ERROR_DETECTOR) || (errcode & IRCAM_ERROR_COMM))
                    {
                        this->moduleStatus |= DEVICE_ERROR_IRCAM_COMM;
                        TLOGMSG(1, ("bit - communication or engine error, ircam\n"));
                    }

                    if (errcode & IRCAM_ERROR_IRSHTR)
                    {
                        this->moduleStatus |= DEVICE_ERROR_IRCAM_IRSHTR;
                        TLOGMSG(1, ("bit - ir shutter error\n"));
                    }
                }
                else
                {
                    if (vgss->testCamIF(vgss) != VGSS_ERROR_NONE)
                    {
                        this->moduleStatus |= DEVICE_ERROR_IRCAM_CAMIF;
                        TLOGMSG(1, ("bit - camera interface data error\n"));
                    }
                    else
                        TLOGMSG(1, ("bit - pass camre interface test\n"));
                }

                if (vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE) == PREVIEW_STATE_STOP)
                    ircam->enablePower(ircam, false);

                break;
            }

            this->bitProgress[i] = DEVICE_BIT_PROGRESS_DONE;

            if (errcode)
                this->bitResult[i] = DEVICE_BIT_RESULT_ERROR;
            else
                this->bitResult[i] = DEVICE_BIT_RESULT_OK;
        }

        if (this->moduleStatus == DEVICE_ERROR_NONE)
        {
            ret = 0;
            TLOGMSG(1, ("bit - all modules are passed test\n"));
        }
        else
        {
            ret = -1;
            TLOGMSG(1, ("bit - test failed\n"));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null device interface\n", DBGINFO));
    }

    return ret;
}


static int
device_get_bit_progress(struct device_interface *device, int module)
{
    int ret = 0;
    struct device_attribute *this = (struct device_attribute *) device;

    if (this)
    {
        if ((module >= DEVICE_MODULE_MEMORY) && (module <= DEVICE_MODULE_BATTERY))
            ret = this->bitProgress[module];
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "null device interface\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null device interface\n", DBGINFO));
    }

    return ret;
}


static int
device_get_bit_result(struct device_interface *device, int module)
{
    int ret = 0;
    struct device_attribute *this = (struct device_attribute *) device;

    if (this)
    {
        if ((module >= DEVICE_MODULE_MEMORY) && (module <= DEVICE_MODULE_BATTERY))
            ret = this->bitResult[module];
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "null device interface\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null device interface\n", DBGINFO));
    }

    return ret;
}


struct device_interface*
device_create(void)
{
    int ret = -1;
    struct device_interface *device = NULL;
    struct device_attribute *this = malloc(sizeof(struct device_attribute));

    if (!this)
    {
        TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));
        goto quit;
    }

    memset(this, 0x00, sizeof(struct device_attribute));
    this->main5v = gpio_create();

    if (!this->main5v)
    {
        TLOGMSG(1, (DBGINFOFMT "main5v gpio interface\n", DBGINFO));
        goto quit;
    }

    this->main5v->export(this->main5v, GPIO6_MAIN_5V);
    this->main5v->setDir(this->main5v, GPIO_DIR_OUTPUT);
    this->main5v->setVal(this->main5v, GPIO_VALUE_HIGH);
    device = &(this->extif);

    device->disp = disp_create();

    if (!device->disp)
    {
        TLOGMSG(1, (DBGINFOFMT "failed to create display interface\n", DBGINFO));
        goto quit;
    }

    device->dmc = dmc_create();

    if (!device->dmc)
    {
        TLOGMSG(1, (DBGINFOFMT "failed to create dmc interface\n", DBGINFO));
        goto quit;
    }

    device->gnss = gnss_create();

    if (!device->gnss)
    {
        TLOGMSG(1, (DBGINFOFMT "failed to create gnss interface\n", DBGINFO));
        goto quit;
    }

    device->ircam = ircam_create();

    if (!device->ircam)
    {
        TLOGMSG(1, (DBGINFOFMT "failed to create ircam interface\n", DBGINFO));
        goto quit;
    }

    device->lrf = lrf_create();

    if (!device->lrf)
    {
        TLOGMSG(1, (DBGINFOFMT "failed to create lrf interface\n", DBGINFO));
        goto quit;
    }

    device->mcu = mcu_create();

    if (!device->mcu)
    {
        TLOGMSG(1, (DBGINFOFMT "failed to create mcu interface\n", DBGINFO));
        goto quit;
    }

    device->vgss = vgss_create();

    if (!device->vgss)
    {
        TLOGMSG(1, (DBGINFOFMT "failed to create vgss interface\n", DBGINFO));
        goto quit;
    }

    device->sdc = sdcard_create();
    if(!device->sdc)
    {
    	TLOGMSG(1, (DBGINFOFMT "failed to create sdc interface\n", DBGINFO));
    	goto quit;
    }

    device->ispmgr = ispmgr_create();
    if(!device->ispmgr)
    {
    	TLOGMSG(1, (DBGINFOFMT "failed to create ispmgr interface\n", DBGINFO));
    	goto quit;
    }

    ret = 0;

quit:

    if (ret == -1)
    {
        if (this)
        {
            device_destroy(device);
            device = NULL;
            TLOGMSG(1, (DBGINFOFMT "failed to init device interface\n", DBGINFO));
        }
    }
    else
    {
        device->suspend = device_suspend_cpu;
        device->standby = device_enter_standby_mode;
        device->wakeup  = device_exit_standby_mode;
        device->getPowerMode     = device_get_power_mode;
        device->setPowerMode     = device_set_power_mode;
        device->getSystemTime    = device_get_systime;
        device->updateSystemTime = device_update_systime;
        device->saveUtcOffset    = device_save_utc_offset;
        device->executeBit       = device_execute_bit;
        device->getBitProgress   = device_get_bit_progress;
        device->getBitResult     = device_get_bit_result;
        device_compensate_system_time(device);
        TLOGMSG(1, ("create device interface\n"));
    }

    return device;
}


int
device_destroy(struct device_interface *device)
{
    int ret = 0;
    struct device_attribute *this = (struct device_attribute *) device;

    if (this)
    {
        if (device->vgss)
            vgss_destroy(device->vgss);

        if (device->mcu)
            mcu_destroy(device->mcu);

        if (device->lrf)
            lrf_destroy(device->lrf);

        if (device->ircam)
            ircam_destroy(device->ircam);

        if (device->gnss)
            gnss_destroy(device->gnss);

        if (device->dmc)
            dmc_destroy(device->dmc);

        if (device->disp)
            disp_destroy(device->disp);

        if(device->sdc)
        	sdcard_destroy(device->sdc);

        if(device->ispmgr)
        	ispmgr_destroy(device->ispmgr);

        free(this);
        TLOGMSG(1, ("destroy device interface\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null device interface\n", DBGINFO));
    }

    return ret;
}
