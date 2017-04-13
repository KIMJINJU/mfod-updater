/*
	Copyright (C) 2017 EOSYSTEM CO., LTD. (www.eosystem.com)

  	sdcard.h
  		interface declaration and constant definitions are corresponding to SDCARD.
  		This file is part of mfod-updater.

  	Written by
  		Seung-hwan, Park (seunghwan.park@me.com)
 */

#ifndef _SDCARD_H_
#define _SDCARD_H_

#include <stdbool.h>

#define SDCARD_INSERT                   1
#define SDCARD_UNINSERT                 0
#define SDCARD_MOUNT                    1
#define SDCARD_UNMOUNT                  0

#define SDCARD_DEVPATH          "/dev/mmcblk1p1"
#define SDCARD_MOUNT_POINT      "/mnt/sdcard/"

struct sdcard_interface
{
    int (*mount)			(struct sdcard_interface *);
    int (*unmount)			(struct sdcard_interface *);
    int (*getMount)			(struct sdcard_interface *);

}sdcard_t;


/* external functions for create/destroy sdcard interface */
extern struct sdcard_interface *sdcard_create(void);
extern int sdcard_destroy(struct sdcard_interface *sdc);


#endif /* _SDCARD_H_ */
