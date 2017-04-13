/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    vgss.c
        external/internal function implementations of vgss interface
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "amod.h"
#include "verinfo.h"
#include "etc/util.h"
#include "core/evque.h"
#include "core/logger.h"
#include "core/sysctrl.h"

/* application crc32 */
unsigned int g_app_crc32 = 0;

void
calculate_appcrc(void)
{
    int fd = 0;
    void *buf = NULL;
    off_t offset = 0;

    fd = open("/root/mfod/mfod-updater", O_RDONLY);

    if (fd != -1)
    {
        offset = lseek(fd, 0, SEEK_END);

        if (offset != -1)
        {
            buf = malloc(offset);

            if (buf)
            {
                lseek(fd, 0, SEEK_SET);
                read(fd, buf, offset);
                g_app_crc32 = crc32(buf, offset);
                free(buf);
            }
        }
        else
            printf("failed to calculate application crc\n");

        close(fd);
    }
}

void
print_appinfo(void)
{
    system("clear");
    LOGMSG(1, ("mfod-updater version %d.%d.%d 0x%08X\n", VERSION_MAJOR, VERSION_MINOR, BUILD_NUMBER, g_app_crc32));
    LOGMSG(1, ("build date : %s %s\n", __DATE__, __TIME__));
    LOGMSG(1, ("(C) EOSYSTEM CO., LTD. 2017\n\n"));
}


struct main_interface *
init_main_interface(void)
{
    int ret = 0;
    struct main_interface *mainif = malloc(sizeof(struct main_interface));

    if (mainif == NULL)
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "failed to malloc for main interface\n", DBGINFO));
        goto quit;
    }

    memset(mainif, 0x00, sizeof(struct main_interface));

    /* initialize gui manager interface */
    mainif->guimgr = guimgr_create();

    if (!mainif->guimgr)
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "failed to create gui manager interface\n", DBGINFO));
        goto quit;
    }

    /* initailize device interface */
    mainif->device = device_create();

    if (!mainif->device)
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "failed to create device manager interface\n", DBGINFO));
        goto quit;
    }

    /* initailize target acquistion manager interface */
    mainif->taqmgr = taqmgr_create(mainif->device->dmc, mainif->device->gnss, mainif->device->lrf);

    if (!mainif->taqmgr)
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "failed to create taq manager interface\n", DBGINFO));
        goto quit;
    }


quit:
    if (ret == 0)
        TLOGMSG(1, ("initiailized main interface\n"));
    else
    {
        if (mainif)
        {
            guimgr_destroy(mainif->guimgr);
            taqmgr_destroy(mainif->taqmgr);
            device_destroy(mainif->device);
            free(mainif);
            mainif = NULL;
        }

        TLOGMSG(1, ("failed initialize main interface\n"));
    }
    return mainif;
}


void
handle_event(struct main_interface *mainif)
{
    struct gui_manager_interface *gui = mainif->guimgr;
    event_t *event = (event_t *) evque_get_event();

    if (event)
    {
        switch(event->code)
        {
        case EVCODE_KEYIN:
            gui->handleKeyin(gui, event->parm, (void *)mainif);
            break;

        case EVCODE_LOW_BATTERY:
            gui->showDialog(gui, GUIID_DIALOG_LOW_VOLTAGE, (void *)mainif);
            break;

        case EVCODE_DEAD_BATTERY:
            sysctrl_power_off((void *)mainif);
            break;

        case EVCODE_PWRSRC_CHANGED:
            break;

        case EVCODE_PWRKEY_RELEASED:
            sysctrl_power_off((void *)mainif);
            break;

        case EVCODE_CREATE_DIALOG:
            mainif->guimgr->showDialog(gui, (int)event->parm, (void *)mainif);
            break;

        default:
            TLOGMSG(1, (DBGINFOFMT "invalid event code\n", DBGINFO));
            break;
        }

        free(event);
    }
    else
        return;
}


int
main(int argc, char **argv)
{
    int ret = 0;
    struct main_interface *mainif = NULL;

    if(logger_init(LOGDEV_CONSOLE) == 0)
    {
        calculate_appcrc();
        print_appinfo();
        mainif = init_main_interface();

        if (mainif)
        {
            mainif->guimgr->startDrawing(mainif->guimgr, (void *) mainif);

            if (evque_create() == 0)
            {
            	mainif->guimgr->showMenu(mainif->guimgr, (void *) mainif, true);
                mainif->guimgr->unlockKeyin(mainif->guimgr);
                while(1)
                    handle_event(mainif);
            }
            else
                ret = -1;
        }
        else
            ret = -1;
    }
    else
    {
        ret = -1;
        printf("failed to init logger\n");
    }

    usleep(1000 * 1000);
    printf("exit mfod-updater\n");

    return ret;
}
