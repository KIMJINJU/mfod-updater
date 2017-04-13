/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    camif.h
        external function/variables/defines for camera interface
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#include <stdbool.h>

#define CAMIF_CAMID_PRIMARY   0
#define CAMIF_CAMID_SECONDARY 1

#define CAMIF_SRC_WIDTH       640
#define CAMIF_SRC_HEIGHT      480
#define CAMIF_SRC_FPS         30

/* structure declaration : camera interface */
struct cam_interface
{
    int (*getFileDesc) (struct cam_interface *this);
    int (*queBuf)      (struct cam_interface *this, void *buf);
    int (*deqBuf)      (struct cam_interface *this, void *buf);
    int (*onStream)    (struct cam_interface *this);
    int (*offStream)   (struct cam_interface *this);
    void *(*getBuf)    (struct cam_interface *this, int bufidx);

#ifdef CAMIF_INTERNAL_INTERFACE
#endif
};


/* external functions for create/destroy cam interfac */
extern struct cam_interface *camif_create(int width, int height, int fps);
extern int camif_destroy(struct cam_interface *camif);
