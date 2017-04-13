/*
	Copyright (C) 2017 EOSYSTEM CO., LTD. (www.eosystem.com)

  	sdcard.c
  		SDCARD functions that performs insertion detection, mount, unmount.
  		This file is part of mfod-updater.

  	Written by
  		Seung-hwan, Park (seunghwan.park@me.com)
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/mount.h>
#include <sys/socket.h>
#include <unistd.h>
#include <linux/fs.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "core/evque.h"
#include "core/logger.h"
#include "modules/sdcard.h"
#include "etc/util.h"

struct sdcard_attribute
{
    struct sdcard_interface extif;

    bool insertion;
    bool mount;
    bool runDectection;
    pthread_t tid;
};


static int
sdcard_check_insertion(struct sdcard_interface *sdc)
{
    int ret = -1;
    char *line = 0;
    size_t length = 0;
    ssize_t nread = 0;
    FILE *stream = NULL;
    struct sdcard_attribute *this = (struct sdcard_attribute *) sdc;

    /* null interface check */
    if (!this)
    {
        TLOGMSG(1, (DBGINFOFMT "null sdcard interface\n", DBGINFO));
        goto error_exit;
    }

    /* open partition infomation */
    if ((stream = fopen("/proc/partitions", "rt")) == NULL)
    {
        TLOGMSG(1, (DBGINFOFMT "failed to open partition infomation\n", DBGINFO));
        goto error_exit;
    }

    /* find sdcard infomation */
    while ((nread = getline(&line, &length, stream)) != -1)
    {
        if (strstr(line, "mmcblk1p1") != NULL)
        {
            ret = 0;
            this->insertion = true;
            break;
        }
        else
            continue;
    }

    free(line);
    fclose(stream);

error_exit:
    return ret;
}




static int
sdcard_get_mount(struct sdcard_interface *sdc)
{
    int ret = 0;
    bool ismount, isinsertion;
    struct sdcard_attribute *this = (struct sdcard_attribute *) sdc;

    if(this)
    {
    	ismount = this->mount;
    	isinsertion  = this->insertion;

    	if((ismount == false) || (isinsertion == false))
    		ret = -1;
    }
    else
    {
    	TLOGMSG(1, (DBGINFOFMT "null sdcard interface\n", DBGINFO));
    	ret = -1;
    }
    return ret;
}




static int
sdcard_mount(struct sdcard_interface *sdc)
{
    int ret = -1;
    struct sdcard_attribute *this = (struct sdcard_attribute *) sdc;

    if (!this)
    {
        TLOGMSG(1, (DBGINFOFMT "null sdcard interface\n", DBGINFO));
        goto error_exit;
    }

    if (!this->insertion)
    {
        TLOGMSG(1, (DBGINFOFMT "sdcard is not inserted!\n", DBGINFO));
        goto error_exit;
    }

    if (this->mount)
    {
        TLOGMSG(1, (DBGINFOFMT "sdcard is already mounted!\n", DBGINFO));
        goto error_exit;
    }

    mkdir(SDCARD_MOUNT_POINT, S_IRWXU | S_IRWXG | S_IRWXO);

    if (mount(SDCARD_DEVPATH, SDCARD_MOUNT_POINT, "vfat", 0, NULL) == 0)
    {
        ret = 0;
        this->mount = true;
        TLOGMSG(1, ("%s mount on %s\n", SDCARD_DEVPATH, SDCARD_MOUNT_POINT));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "mount() return fail, %s\n", DBGINFO, strerror(errno)));

error_exit:
    return ret;
}



static int
sdcard_unmount(struct sdcard_interface *sdc)
{
    int ret = -1;
    struct sdcard_attribute *this = (struct sdcard_attribute *) sdc;

    if (!this)
    {
        TLOGMSG(1, (DBGINFOFMT "null sdcard interface\n", DBGINFO));
        goto error_exit;
    }

    if (!this->mount)
    {
        TLOGMSG(1, (DBGINFOFMT "sdcard is already unmounted\n", DBGINFO));
        goto error_exit;
    }

    if(umount2(SDCARD_MOUNT_POINT, MNT_DETACH) == 0)
	{
		ret = 0;
		this->mount = false;
		TLOGMSG(1, ("unmount sdcard \n", SDCARD_MOUNT_POINT));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "umount() return fail, %s\n", DBGINFO, strerror(errno)));
	}

error_exit:
    return ret;
}



static void *
sdcard_dectect_insersion(void *arg)
{
    int ns = -1;
    int length = 0;
    char buf[1024] = {0};
    struct iovec iov = {0};
    struct msghdr hdr = {0};
    struct sockaddr_nl sa = {0};
    struct timeval timeo = {.tv_sec = 3, .tv_usec = 0};
    struct sdcard_interface *sdc = (struct sdcard_interface *) arg;
    struct sdcard_attribute *this= (struct sdcard_attribute *) sdc;

    this->runDectection = true;
    memset(&sa, 0x00, sizeof(struct sockaddr_nl));
    sa.nl_family = AF_NETLINK;
    sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;

    memset(&iov, 0x00, sizeof(struct iovec));
    iov.iov_base = &buf[0];
    iov.iov_len = sizeof(buf);

    memset(buf, 0x00, sizeof(buf));
    memset(&hdr, 0x00, sizeof(struct msghdr));
    hdr.msg_name 	   = &sa;
	hdr.msg_namelen    = sizeof(struct sockaddr_nl);
	hdr.msg_iov 	   = &iov;
	hdr.msg_iovlen 	   = 1;
	hdr.msg_control    = NULL;
	hdr.msg_controllen = 0;
	hdr.msg_flags 	   = 0;

    if ((ns = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT)) == -1)
    {
        TLOGMSG(1, (DBGINFOFMT "socket() return fail, %s\n", DBGINFO, strerror(errno)));
        //evque_set_evnet(EVCODE_FAIL_INIT_INSERTION_DETECTION);
        goto error_exit;
    }

    if (setsockopt(ns, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(struct timeval)) == -1)
    {
        TLOGMSG(1, (DBGINFOFMT "setsockopt() return fail, %s\n", DBGINFO, strerror(errno)));
        //evque_set_evnet(EVCODE_FAIL_INIT_INSERTION_DETECTION);
        goto error_exit;
    }

    if (bind(ns, (struct sockaddr *)&sa, sizeof(struct sockaddr_nl)) != 0)
    {
        TLOGMSG(1, (DBGINFOFMT "bind() return fail, %s\n", DBGINFO, strerror(errno)));
        //evque_set_evnet(EVCODE_FAIL_INIT_INSERTION_DETECTION);
        goto error_exit;
    }

    while (this->runDectection)
    {
        if ((length = recvmsg(ns, &hdr, 0)) >= 0)
        {
            if ((strstr(buf, "add@")) && (strstr(buf, "mmcblk1p1")))
            {
                MSLEEP(200);

                if (sdcard_check_insertion(sdc) == 0)
                	sdcard_mount(sdc);
                else
                    continue;

            }
            else if ((strstr(buf, "remove@")) && (strstr(buf, "mmcblk1p1")))
			{
                MSLEEP(200);

                if (this->mount)
                	sdcard_unmount(sdc);
                else
                    continue;
            }
            else
                continue;
        }
        else
            continue;
    }

error_exit:
    if (ns != -1)
        close(ns);

    return NULL;
}


struct sdcard_interface *sdcard_create(void)
{
    struct sdcard_interface *sdc = NULL;
    struct sdcard_attribute *this = malloc(sizeof(struct sdcard_attribute));

    if (!this)
    {
        TLOGMSG(1, (DBGINFOFMT "malloc() return null, %s\n", DBGINFO, strerror(errno)));
        goto error_exit;
    }

    memset(this, 0x00, sizeof(struct sdcard_attribute));
    sdc = &(this->extif);

    this->insertion = false;
    this->mount = false;
    this->runDectection = false;

    /* set external interface */
    sdc->mount         = sdcard_mount;
    sdc->unmount       = sdcard_unmount;
    sdc->getMount       = sdcard_get_mount;

    if(sdcard_check_insertion(sdc) == 0)
    	sdcard_mount(sdc);

    /* create thread for insertion detection */
    if (pthread_create(&this->tid, NULL, sdcard_dectect_insersion, (void *)sdc) == 0)
    	TLOGMSG(1, ("create sdcard interface\n"));
    else
    {
        free(this);
        sdc = NULL;
        TLOGMSG(1, (DBGINFOFMT "pthread_create() return fail, %s", DBGINFO, strerror(errno)));
    }

error_exit:
    return sdc;
}


int sdcard_destroy(struct sdcard_interface *sdc)
{
    int ret = -1;
    struct sdcard_attribute *this = (struct sdcard_attribute *) sdc;

    /* null interface check */
    if (!this)
    {
        TLOGMSG(1, (DBGINFOFMT "null sdcard interface\n", DBGINFO));
        goto error_exit;
    }

    this->runDectection = false;
    pthread_join(this->tid, NULL);

    if (this->mount)
       sdcard_unmount(sdc);

    ret = 0;
    free(this);
    TLOGMSG(1, ("destroy sdcard interface\n"));

error_exit:
    return ret;
}

