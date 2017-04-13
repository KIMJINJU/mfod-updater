/*
	Copyright (C) 2017 EOSYSTEM CO., LTD. (www.eosystem.com)

  	ispmgr.c
  		implementaions ispmgr interface.
  		This file is part of mfod-updater.

  	Written by
  		Seung-hwan, Park (seunghwan.park@me.com)
 */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>


#include "etc/ispmgr.h"
#include "modules/sdcard.h"
#include "core/logger.h"
#include "etc/util.h"

struct ispmgr_image
{
	bool image_valid;
	char *image_path;
	char *blkdev;
	off_t offset[2];

	struct stat	status;
};

struct ispmgr_attribute
{
	struct ispmgr_interface extif;

    int updateTask;
    int updateError;
    int writeSize;
    int totalSize;
    double writeProgress;

    struct ispmgr_image image[ISPMGR_IMAGES_NUM];
};

static int
ispmgr_write_image(struct ispmgr_interface *ispmgr, char *image, off_t imgoff, char *blkdev, off_t blkoff, off_t size)
{
    int ret = -1;
    int imgfd = -1;
    int blkfd = -1;
    off_t offset = 0;
    off_t nwrite = size;
    char *buffer = NULL;
    struct ispmgr_attribute *this = (struct ispmgr_attribute *)ispmgr;

    this->writeSize = 0;
    this->totalSize = size;
    this->writeProgress = 0.0;

    if(!this)
    {
        TLOGMSG(1, (DBGINFOFMT "null ispmgr interface\n", DBGINFO));
        goto error_exit;
    }

    imgfd = open(image, O_RDONLY);
    if(imgfd < 0)
    {
        TLOGMSG(1, (DBGINFOFMT "open() return fail, %s\n", DBGINFO, strerror(errno)));
        goto error_exit;
    }

    blkfd = open(blkdev, O_WRONLY);
    if(blkfd < 0)
    {
        TLOGMSG(1, (DBGINFOFMT "open() return fail, %s\n", DBGINFO, strerror(errno)));
        goto error_exit;
    }

    /* set image offset */
    if (imgoff != 0)
    {
    	offset = lseek(imgfd, imgoff, SEEK_SET);
        if(offset < 0)
        {
        	TLOGMSG(1, (DBGINFOFMT "lseek() return fail, %s\n", DBGINFO, strerror(errno)));
            goto error_exit;
        }
        else
            TLOGMSG(1, ("reposition image offset to read (0x%X)\n", offset));

    }

    /* set block device offset */
    if (blkoff != 0)
    {
    	offset = lseek(blkfd, blkoff, SEEK_SET);
        if( offset < 0)
        {
        	TLOGMSG(1, (DBGINFOFMT "lseek() return fail, %s\n", DBGINFO, strerror(errno)));
        	goto error_exit;
        }
        else
        	TLOGMSG(1, ("reposition blkdev offset to write (0x%X)\n", offset));
    }

    /* memory allocation for buffer */
    buffer = (char*) malloc(ISPMGR_CYLINDER_SIZE);
    if(buffer == NULL)
    {
        TLOGMSG(1, (DBGINFOFMT "malloc() return fail, %s\n", DBGINFO, strerror(errno)));
        goto error_exit;
    }

    while(nwrite > ISPMGR_CYLINDER_SIZE)
    {
        memset(buffer, 0x00, ISPMGR_CYLINDER_SIZE);

        if (read(imgfd, buffer, ISPMGR_CYLINDER_SIZE) < ISPMGR_CYLINDER_SIZE)
        {
            TLOGMSG(1, (DBGINFOFMT "read() return fail, %s\n", DBGINFO, strerror(errno)));
            goto error_exit;
        }

        if (write(blkfd, buffer, ISPMGR_CYLINDER_SIZE) < ISPMGR_CYLINDER_SIZE)
        {
            TLOGMSG(1, (DBGINFOFMT "write() return fail, %s\n", DBGINFO, strerror(errno)));
            goto error_exit;
        }

        sync();
        nwrite -= ISPMGR_CYLINDER_SIZE;
        this->writeSize = size - nwrite;
        this->writeProgress = 100.0 * (size - nwrite) / size;
    }

    if(nwrite != 0)
    {
    	memset(buffer, 0, ISPMGR_CYLINDER_SIZE);

    	if(read(imgfd, buffer, nwrite) < nwrite)
    	{
    		TLOGMSG(1, (DBGINFOFMT "read() return fail, %s\n", DBGINFO, strerror(errno)));
    		goto error_exit;
    	}

    	if(write(blkfd, buffer, ISPMGR_CYLINDER_SIZE) < nwrite)
    	{
    		TLOGMSG(1, (DBGINFOFMT "write() return fail, %s\n", DBGINFO, strerror(errno)));
    		goto error_exit;
    	}
    	sync();
    	this->writeSize = size;
    	this->writeProgress = 100.0;
    }

    sync();
    ret = 0;
    TLOGMSG(1, ("%d Kbytes write on %s\n", this->writeSize / 1024, blkdev));

error_exit:

    if (buffer) free(buffer);
    if (imgfd != -1) close(imgfd);
    if (blkfd != -1) close(blkfd);

    return ret;
}


static int
ispmgr_write_uboot(struct ispmgr_interface *ispmgr)
{
	int ret = 0;
	struct ispmgr_attribute *this = (struct ispmgr_attribute *)ispmgr;

	TLOGMSG(1, ("ispmgr: updating uboot...\n"));

	this->updateTask = ISPMGR_WRITE_UBOOT;

	ret = ispmgr_write_image(ispmgr, this->image[ISPMGR_UBOOT].image_path,
									this->image[ISPMGR_UBOOT].offset[ISPOFFSET_FILE],
									this->image[ISPMGR_UBOOT].blkdev,
									this->image[ISPMGR_UBOOT].offset[ISPOFFSET_BLKDEV],
									this->image[ISPMGR_UBOOT].status.st_size);

	return ret;
}



static int
ispmgr_write_uimage(struct ispmgr_interface *ispmgr)
{
	int ret = 0;
	struct ispmgr_attribute *this = (struct ispmgr_attribute *)ispmgr;

	TLOGMSG(1, ("ispmgr: updating kernel...\n"));

	this->updateTask = ISPMGR_WRITE_UIMAGE;

	ret = ispmgr_write_image(ispmgr, this->image[ISPMGR_UIMAGE].image_path,
									this->image[ISPMGR_UIMAGE].offset[ISPOFFSET_FILE],
									this->image[ISPMGR_UIMAGE].blkdev,
									this->image[ISPMGR_UIMAGE].offset[ISPOFFSET_BLKDEV],
									this->image[ISPMGR_UIMAGE].status.st_size);

	return ret;
}


static int
ispmgr_write_rootfs(struct ispmgr_interface *ispmgr)
{
	int ret = 0;
	struct ispmgr_attribute *this = (struct ispmgr_attribute *)ispmgr;

	TLOGMSG(1, ("ispmgr: updating rootfs...\n"));

	this->updateTask = ISPMGR_WRITE_ROOTFS;

	ret = ispmgr_write_image(ispmgr, this->image[ISPMGR_ROOTFS].image_path,
									this->image[ISPMGR_ROOTFS].offset[ISPOFFSET_FILE],
									this->image[ISPMGR_ROOTFS].blkdev,
									this->image[ISPMGR_ROOTFS].offset[ISPOFFSET_BLKDEV],
									this->image[ISPMGR_ROOTFS].status.st_size);

	return ret;
}

static int
ispmgr_erase_rootfs(struct ispmgr_interface *ispmgr)
{
	int ret = 0;
	struct ispmgr_attribute *this = (struct ispmgr_attribute *)ispmgr;

	TLOGMSG(1, ("ispmgr: erase rootfs...\n"));

	this->updateTask = ISPMGR_ERASE_ROOTFS;

	ret = ispmgr_write_image(ispmgr, "/dev/zero",
									this->image[ISPMGR_ROOTFS].offset[ISPOFFSET_FILE],
									this->image[ISPMGR_ROOTFS].blkdev,
									this->image[ISPMGR_ROOTFS].offset[ISPOFFSET_BLKDEV],
									this->image[ISPMGR_ROOTFS].status.st_size);

	return ret;
}

static int
ispmgr_start_programming(struct ispmgr_interface *ispmgr)
{
	int ret = -1;
	struct ispmgr_attribute *this = (struct ispmgr_attribute *)ispmgr;

	if(!this)
	{
		TLOGMSG(1, (DBGINFOFMT "null ispmgr interface\n", DBGINFO));
		goto error_exit;
	}

	system("echo 0 > /sys/block/mmcblk0boot0/force_ro");

	if(this->image[ISPMGR_UBOOT].image_valid)
	{
		if(ispmgr_write_uboot(ispmgr) != 0)
		{
			this->updateError = ISPMGR_ERRCODE_WRITE_UBOOT;
			TLOGMSG(1, (DBGINFOFMT "write uboot error!\n", DBGINFO));
			goto error_exit;
		}
		MSLEEP(1000);
	}

	if(this->image[ISPMGR_UIMAGE].image_valid)
	{
		if(ispmgr_write_uimage(ispmgr) != 0)
		{
			this->updateError = ISPMGR_ERRCODE_WRITE_UIMAGE;
			TLOGMSG(1, (DBGINFOFMT "write uimage error!\n", DBGINFO));
			goto error_exit;
		}
		MSLEEP(1000);
	}

	if(this->image[ISPMGR_ROOTFS].image_valid)
	{
		if(ispmgr_erase_rootfs(ispmgr) != 0)
		{
			this->updateError = ISPMGR_ERRCODE_ERASE_ROOTFS;
			TLOGMSG(1, (DBGINFOFMT "erase rootfs error!\n", DBGINFO));
			goto error_exit;
		}
		MSLEEP(1000);
	}

	if(this->image[ISPMGR_ROOTFS].image_valid)
	{
		if(ispmgr_write_rootfs(ispmgr) != 0)
		{
			this->updateError = ISPMGR_ERRCODE_WRITE_ROOTFS;
			TLOGMSG(1, (DBGINFOFMT "write rootfs error!\n", DBGINFO));
			goto error_exit;
		}
		MSLEEP(1000);
	}
	ret = 0;

error_exit:

	return ret;
}

static int
ispmgr_set_blkdev(struct ispmgr_interface *ispmgr, int imgid, char *blkdev, off_t imgoff)
{
	int ret = 0;
	size_t length;
	struct ispmgr_attribute *this = (struct ispmgr_attribute *)ispmgr;

	if(this)
	{
		if((imgid >= ISPMGR_UBOOT) && (imgid <= ISPMGR_ROOTFS))
		{
			if(blkdev)
			{
				if(this->image[imgid].blkdev)
					free(this->image[imgid].blkdev);

				length = strlen(blkdev) + 1;
				this->image[imgid].blkdev = malloc(length);
				memset(this->image[imgid].blkdev, 0x00, length);
				memcpy(this->image[imgid].blkdev, blkdev, length);
				this->image[imgid].offset[ISPOFFSET_BLKDEV] = imgoff;

				TLOGMSG(1, (DBGINFOFMT "ispmgr: set blkdev, %s\n", DBGINFO, this->image[imgid].blkdev));
				TLOGMSG(1, (DBGINFOFMT "ispmgr: set blkdev offset, %d\n", DBGINFO, this->image[imgid].offset[ISPOFFSET_BLKDEV]));
			}
			else
			{
				TLOGMSG(1, (DBGINFOFMT "invalid partition\n", DBGINFO));
				ret = -1;
			}
		}
		else
		{
			TLOGMSG(1, (DBGINFOFMT "invalid image index\n", DBGINFO));
			ret = -1;
		}
	}
	else
	{
		TLOGMSG(1, (DBGINFOFMT "null ispmgr pointer\n", DBGINFO));
		ret = -1;
	}

	return ret;
}

static int
ispmgr_set_image(struct ispmgr_interface *ispmgr, int imgid, char *path, off_t imgoff)
{
	int ret = 0;
	size_t length;
	struct ispmgr_attribute *this = (struct ispmgr_attribute *)ispmgr;

	if(this)
	{
		if((imgid >= ISPMGR_UBOOT) && (imgid <= ISPMGR_ROOTFS))
		{
			if(path)
			{
				if(this->image[imgid].image_path)
					free(this->image[imgid].image_path);

				length = strlen(path) + 1;
				this->image[imgid].image_path = malloc(length);
				memset(this->image[imgid].image_path, 0x00, length);
				memcpy(this->image[imgid].image_path, path, length);
				this->image[imgid].offset[ISPOFFSET_FILE] = imgoff;

				if(stat(this->image[imgid].image_path, &(this->image[imgid].status)) == 0)
				{
					if(this->image[imgid].status.st_size != 0)
						this->image[imgid].image_valid = true;
					else
						this->image[imgid].image_valid = false;
					TLOGMSG(1, (DBGINFOFMT "ispmgr: size %d\n", DBGINFO, this->image[imgid].status.st_size));
				}
				else
				{
					this->image[imgid].image_valid = false;
					TLOGMSG(1, (DBGINFOFMT "image file not found\n", DBGINFO));
				}
				TLOGMSG(1, (DBGINFOFMT "ispmgr: set image path, %s\n", DBGINFO, this->image[imgid].image_path));
				TLOGMSG(1, (DBGINFOFMT "ispmgr: set image offset, %d\n", DBGINFO, this->image[imgid].offset[ISPOFFSET_FILE]));
			}
			else
			{
				TLOGMSG(1, (DBGINFOFMT "invalid image path\n", DBGINFO));
				ret = -1;
			}
		}
		else
		{
			TLOGMSG(1, (DBGINFOFMT "invalid image index\n", DBGINFO));
			ret = -1;
		}
	}
	else
	{
		TLOGMSG(1, (DBGINFOFMT "null ispmgr pointer\n", DBGINFO));
		ret = -1;
	}

	return ret;
}





static int
ispmgr_get_updateprogress(struct ispmgr_interface *ispmgr, int *nwrite, int *ntotal, double *progress)
{
	int ret = 0;
	struct ispmgr_attribute *this = (struct ispmgr_attribute  *)ispmgr;

	if(this)
	{
		*nwrite = this->writeSize;
		*ntotal = this->totalSize;
		*progress = this->writeProgress;
	}
	else
	{
		TLOGMSG(1, (DBGINFOFMT "null ispmgr pointer\n", DBGINFO));
		ret = -1;
	}

	return ret;
}

static  int
ispmgr_get_imagevaliable(struct ispmgr_interface *ispmgr)
{
	int ret = 0;
	bool boot, uimage, rootfs;
	struct ispmgr_attribute *this = (struct ispmgr_attribute *)ispmgr;

	if(this)
	{
		boot 	= this->image[ISPMGR_UBOOT].image_valid;
		uimage 	= this->image[ISPMGR_UIMAGE].image_valid;
		rootfs 	= this->image[ISPMGR_ROOTFS].image_valid;
		if(!boot && !uimage && !rootfs)
			ret = -1;
	}
	else
	{
		TLOGMSG(1, (DBGINFOFMT "null ispmgr pointer\n", DBGINFO));
		ret = -1;
	}
	return ret;
}

static int
ispmgr_get_updatetask(struct ispmgr_interface *ispmgr, int *task)
{
	int ret = 0;
	struct ispmgr_attribute *this = (struct ispmgr_attribute *)ispmgr;

	if(this)
		*task = this->updateTask;
	else
	{
		TLOGMSG(1, (DBGINFOFMT "null ispmgr pointer\n", DBGINFO));
		ret = -1;
	}
	return ret = -1;
}


static int
ispmgr_set_updatetask(struct ispmgr_interface *ispmgr, int task)
{
	int ret = 0;
	struct ispmgr_attribute *this = (struct ispmgr_attribute *)ispmgr;

	if(this)
		this->updateTask = task;
	else
	{
		TLOGMSG(1, (DBGINFOFMT "null ispmgr pointer\n", DBGINFO));
		ret = -1;
	}
	return ret = -1;
}


struct ispmgr_interface *ispmgr_create(void)
{
	struct ispmgr_interface *ispmgr = NULL;
	struct ispmgr_attribute *this = malloc(sizeof(struct ispmgr_attribute));

	if(!this)
	{
		TLOGMSG(1, (DBGINFOFMT "malloc() return null, %s\n", DBGINFO, strerror(errno)));
		goto error_exit;
	}

	memset(this, 0x00, sizeof(struct ispmgr_attribute));
	ispmgr = &(this->extif);
	this->writeSize = 0;
	this->totalSize = 0;
	this->updateError = 0;
	this->writeProgress = 0.0;
	this->updateTask = ISPMGR_STANDBY_PROGRAMMING;

	ispmgr->getUpdateProgress 	= ispmgr_get_updateprogress;
	ispmgr->writeImage 			= ispmgr_write_image;
	ispmgr->writeUboot 			= ispmgr_write_uboot;
	ispmgr->writeUimage 		= ispmgr_write_uimage;
	ispmgr->writeRootfs 		= ispmgr_write_rootfs;
	ispmgr->eraseRootfs 		= ispmgr_erase_rootfs;
	ispmgr->startProgramming 	= ispmgr_start_programming;
	ispmgr->setBlkdev 			= ispmgr_set_blkdev;
	ispmgr->setImages 			= ispmgr_set_image;
	ispmgr->getImagesValiable   = ispmgr_get_imagevaliable;
	ispmgr->getUpdateTask		= ispmgr_get_updatetask;
	ispmgr->setUpdateTask		= ispmgr_set_updatetask;

error_exit:
	return ispmgr;
}


int ispmgr_destroy(struct ispmgr_interface *ispmgr)
{
	int ret = -1;
	struct ispmgr_attribute *this = (struct ispmgr_attribute *)ispmgr;

	if(!this)
	{
		TLOGMSG(1, (DBGINFOFMT "null ispmgr interface\n", DBGINFO));
		goto error_exit;
	}

	for(int i = 0; i < ISPMGR_IMAGES_NUM; i++)
	{
		if(this->image[i].image_path)
			free(this->image[i].image_path);

		if(this->image[i].blkdev)
			free(this->image[i].blkdev);
	}

	ret = 0;
	TLOGMSG(1, ("destroy ispmgr interface\n"));
	free(this);

error_exit:
	return ret;
}



