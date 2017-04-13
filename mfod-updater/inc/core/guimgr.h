/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    guimgr.h
        external function/variables/defines for gui manager
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/


#ifndef _GUIMGR_H_
#define _GUIMGR_H_


#include "gui/guiid.h"
#include "gui/gui_dialog.h"

/* macro defines : gui mode */
#define GUI_MODE_PBIT               0
#define GUI_MODE_IBIT               1
#define GUI_MODE_OBSERVE            2
#define GUI_MODE_PWROFF             3
#define GUI_MODE_MAGCOMP            4
#define GUI_MODE_FACTORY			5
#define GUI_MODE_FWUPDATE			6

/* macro defines : gui notice code */
#define GUI_NOTICE_NONE                 0
#define GUI_NOTICE_WAIT                 1
#define GUI_NOTICE_CHANGE_OBMODE        2
#define GUI_NOTICE_CHANGE_NDFILTER      3
#define GUI_NOTICE_INIT_MAGCOMP         4
#define GUI_NOTICE_WAIT_CALC_MAGPARM    5
#define GUI_NOTICE_SAVE_MAGPARM         6
#define GUI_NOTICE_EXIT_MAGCOMP         7
#define GUI_NOTICE_WAIT_RESPONSE        8
#define GUI_NOTICE_DMIF_TEST            9
#define GUI_NOTICE_ENTER_STANDBY_MODE   10
#define GUI_NOTICE_EXIT_STANDBY_MODE    11

/* structure declarations */
typedef struct gui_manager_interface
{
    int (*startDrawing)      (struct gui_manager_interface *, void *);
    int (*stopDrawing)       (struct gui_manager_interface *);
    int (*setMode)           (struct gui_manager_interface *, int);
    int (*getMode)           (struct gui_manager_interface *);
    int (*handleKeyin)       (struct gui_manager_interface *, unsigned int, void *);
    int (*postNotice)        (struct gui_manager_interface *, int);
    int (*lockKeyin)         (struct gui_manager_interface *);
    int (*unlockKeyin)       (struct gui_manager_interface *);
    int (*showMenu)          (struct gui_manager_interface *, void *, bool);
    int (*showDialog)        (struct gui_manager_interface *, int, void *);
    int (*hideToplevDialog)  (struct gui_manager_interface *);
    void *(*findDialog)      (struct gui_manager_interface *, int);
    void *(*getToplevDialog) (struct gui_manager_interface *);

#ifdef _ENABLE_GUIMGR_INTERNAL_INTERFACE_

#endif  /* _ENABLE_GUIMGR_INTERNAL_INTERFACE_ */
}
guimgr_t;


/* external functions declarations */
extern struct gui_manager_interface *guimgr_create(void);
extern int guimgr_destroy(struct gui_manager_interface *guimgr);

#endif /* _GUIMGR_H_*/
