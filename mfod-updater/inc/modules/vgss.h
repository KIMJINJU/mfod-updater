/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    vdev.h
        external function/variables/defines for video graphics sub system interface
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#ifndef _VGSS_H_
#define _VGSS_H_

#include <stdbool.h>

#define VGSS_SUCCESS                0
#define VGSS_FAIL                   -1

/* macro defines : vdev error codes */
#define VGSS_ERROR_NONE             0x0000
#define VGSS_ERROR_VGAENC           0x0001
#define VGSS_ERROR_TVENC            0x0002
#define VGSS_ERROR_CAMIF_QBUF       0x0004
#define VGSS_ERROR_CAMIF_DQBUF      0x0008
#define VGSS_ERROR_CAMIF_STREAM     0x0010
#define VGSS_ERROR_IPU              0x0020
#define VGSS_ERROR_EIS              0x0040


/* marco defines : external video out state */
#define EVOUT_CTRL_DISABLE      0
#define EVOUT_CTRL_ENABLE       1
#define EVOUT_CTRL_STATE        2

/* marco defines : external video out state */
#define EVOUT_STATE_DISABLE     0
#define EVOUT_STATE_ENABLE      1


/* macro defines : preview control code */
#define PREVIEW_CTRL_INIT		0x00
#define PREVIEW_CTRL_DEINIT	    0x01
#define PREVIEW_CTRL_RUN		0x02
#define PREVIEW_CTRL_PAUSE		0x03
#define PREVIEW_CTRL_STATE		0x04

/* marco defines : preview state */
#define PREVIEW_STATE_STOP		0x10
#define PREVIEW_STATE_RUN		0x11
#define PREVIEW_STATE_PAUSE		0x12

/* macro defines : eis control code */
#define EIS_CTRL_DISABLE		0x00
#define EIS_CTRL_ENABLE			0x01
#define EIS_CTRL_EXIT           0x02
#define EIS_CTRL_STATE			0x03

/* marco defines : eis state */
#define EIS_STATE_DISABLE		0x10
#define EIS_STATE_ENABLE		0x11
#define EIS_STATE_EXIT          0x12

/* macro defines : eis control code */
#define EZOOM_CTRL_DISABLE		0x00
#define EZOOM_CTRL_ENABLE		0x01
#define EZOOM_CTRL_STATE		0x02

/* marco defines : eis state */
#define EZOOM_STATE_DISABLE		0x10
#define EZOOM_STATE_ENABLE		0x11


/* structure declaration : vgss interface */
struct vgss_interface
{
    int (*ctrlPreview) (struct vgss_interface *, int);
    int (*ctrlZoom)   (struct vgss_interface *, int);
    int (*ctrlEIS)     (struct vgss_interface *, int);
    int (*ctrlExtVideo)(struct vgss_interface *, int);
    int (*testCamIF)   (struct vgss_interface *);

#ifdef _VGSS_INTERNAL_INTERFACE_

#endif
};


/* exteranl functions : create/destroy vdev interface */
extern struct vgss_interface *vgss_create(void);
extern int vgss_destroy(struct vgss_interface *vgss);

#endif /* _VGSS_H_ */
