/*
	Copyright (C) 2017 EOSYSTEM CO., LTD. (www.eosystem.com)

  	ispmgr.h
  		interface declaration and constant definitions are corresponding to ISP (in-system programming)
  		This file is part of mfod-updater.

  	Written by
  		Seung-hwan, Park (seunghwan.park@me.com)
 */

#ifndef _ISPMGR_H_
#define _ISPMGR_H_

#include <sys/stat.h>
#include <stdbool.h>

#define ISPMGR_DEFAULT_UBOOT_PATH           "/mnt/sdcard/u-boot-no-padding.bin"
#define ISPMGR_DEFAULT_UIMAGE_PATH          "/mnt/sdcard/uImage"
#define ISPMGR_DEFAULT_ROOTFS_PATH          "/mnt/sdcard/rootfs.img"

/* */
#define ISPMGR_STANDBY_PROGRAMMING          0
#define ISPMGR_WRITE_UBOOT                  1           /* write uboot on mmcblk0               */
#define ISPMGR_WRITE_UIMAGE                 2           /* write uImage on mmcblk0              */
#define ISPMGR_ERASE_ROOTFS              	3           /* erase root file system on mmcblk0p1  */
#define ISPMGR_WRITE_ROOTFS                 4           /* write root file system on mmcblk0p1  */
#define ISPMGR_END_PROGRAMMING              5

#define ISPMGR_UBOOT                        0
#define ISPMGR_UIMAGE                       1
#define ISPMGR_ROOTFS                       2


#define ISPMGR_RETCODE_UPDATE_OK			0
#define ISPMGR_RETCODE_UPDATE_UBOOT_OK		1
#define ISPMGR_RETCODE_UPDATE_UIMAGE_OK		2
#define ISPMGR_RETCODE_UPDATE_ROOTFS_OK		3
#define ISPMGR_RETCODE_UBOOT_NOT_EXIST		4
#define ISPMGR_RETCODE_UIMAGE_NOT_EXIST		5
#define ISPMGR_RETCODE_ROOTFS_NOT_EXIST		6

#define ISPMGR_ERRCODE_WRITE_UBOOT			0
#define ISPMGR_ERRCODE_WRITE_UIMAGE			1
#define ISPMGR_ERRCODE_WRITE_ROOTFS			2
#define ISPMGR_ERRCODE_ERASE_ROOTFS			3


#define FILE_OFFSET_UBOOT           0
#define FILE_OFFSET_ROOTFS	        0
#define FILE_OFFSET_UIMAGE          0

#define ISPOFFSET_FILE				0
#define ISPOFFSET_BLKDEV			1


#define BLKDEV_OFFSET_UBOOT	        1024
#define BLKDEV_OFFSET_UIMAGE        1048576
#define BLKDEV_OFFSET_ROOTFS	    0

#define BLKDEV_PATH_UBOOT			"/dev/mmcblk0"
#define BLKDEV_PATH_UIMAGE		    "/dev/mmcblk0"
#define BLKDEV_PATH_ROOTFS		    "/dev/mmcblk0p1"

#define ISPMGR_CYLINDER_SIZE        64 * 512
#define ISPMGR_IMAGES_NUM			3
#define ISPMGR_PARTITIONS_NUM		3

/* ispmgr interface */
struct ispmgr_interface
{
    int (*getUpdateProgress)	(struct ispmgr_interface *, int *nwrite, int *ntotal, double *progress);
    int (*writeImage)			(struct ispmgr_interface *, char *image, off_t imgoff, char *blkdev, off_t blkoff, off_t size);
    int (*writeUboot)			(struct ispmgr_interface *);
    int (*writeUimage)			(struct ispmgr_interface *);
    int (*writeRootfs)			(struct ispmgr_interface *);
    int (*eraseRootfs)			(struct ispmgr_interface *);
    int (*startProgramming)		(struct ispmgr_interface *);
    int (*setBlkdev)			(struct ispmgr_interface *, int imgid, char *blkdev, off_t imgoff);
    int (*setImages)			(struct ispmgr_interface *, int imgid, char *blkdev, off_t imgoff);
    int (*getImagesValiable)	(struct ispmgr_interface *);
    int (*getUpdateTask)		(struct ispmgr_interface *, int *task);
    int (*setUpdateTask)		(struct ispmgr_interface *, int task);

}ispmgr_t;


/* external functions for create ispmgr */
extern struct ispmgr_interface *ispmgr_create(void);
extern int ispmgr_destroy(struct ispmgr_interface *ispmgr);


#endif /* _ISPMGR_H_ */
