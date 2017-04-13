/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_window.h
        external function/variables/defines for window
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/
#ifndef _GUI_WINDOW_H_
#define _GUI_WINDOW_H_

#include <directfb.h>

#include "ds/stack.h"
#include "gui/gui_dialog.h"
#include "gui/gui_menu.h"
#include "gui/gui_submenu.h"


/* macro defines : gui focus */
#define GUI_FOCUS_NONE                      0
#define GUI_FOCUS_MENU                      1
#define GUI_FOCUS_SUBMENU                   2
#define GUI_FOCUS_DIALOG                    3

/* macro defines */
#define BASE_FONT_HEIGHT                    10
#define NUM_FONT_HEIGHTS                    8
#define NUM_PRELOAD_IMAGES                  21

/* macro defines : preload image id */
#define IMAGE_ID_RETICLE_DV                 0
#define IMAGE_ID_RETICLE_IR                 1
#define IMAGE_ID_RETICLE_2X                 2
#define IMAGE_ID_AZIMUTH_INDICATOR_DV       3
#define IMAGE_ID_AZIMUTH_INDICATOR_IR       4
#define IMAGE_ID_ELEVATION_INDICATOR_DV     5
#define IMAGE_ID_ELEVATION_INDICATOR_IR     6
#define IMAGE_ID_AZIMUTH_POINTER_DV         7
#define IMAGE_ID_AZIMUTH_POINTER_IR         8
#define IMAGE_ID_ELEVATION_POINTER_DV       9
#define IMAGE_ID_ELEVATION_POINTER_IR       10
#define IMAGE_ID_SVPLOT_BACKGROUND          11
#define IMAGE_ID_RED_INDICATOR_DV           12
#define IMAGE_ID_RED_INDICATOR_IR           13
#define IMAGE_ID_GREEN_INDICATOR_DV         14
#define IMAGE_ID_GREEN_INDICATOR_IR         15
#define IMAGE_ID_POWER_PLUG                 16
#define IMAGE_ID_LRFTRG_INDICATOR_LEFT_DV   17
#define IMAGE_ID_LRFTRG_INDICATOR_RIGHT_DV  18
#define IMAGE_ID_LRFTRG_INDICATOR_LEFT_IR   19
#define IMAGE_ID_LRFTRG_INDICATOR_RIGHT_IR  20

/* macro defines : font heights */
#define FONT_HEIGHT_10                      0
#define FONT_HEIGHT_12                      1
#define FONT_HEIGHT_14                      2
#define FONT_HEIGHT_16                      3
#define FONT_HEIGHT_18                      4
#define FONT_HEIGHT_20                      5
#define FONT_HEIGHT_22                      6
#define FONT_HEIGHT_24                      7

/* macro define : waiting notice code */
#define NOTICE_NONE                         0
#define NOTICE_WAIT                         1
#define NOTICE_CHANGE_OBMODE                2
#define NOTICE_CHANGE_NDFILTER              3
#define NOTICE_INIT_MAGCOMP                 4
#define NOTICE_WAIT_CALC_MAGPARM            5
#define NOTICE_SAVE_MAGPARM                 6
#define NOTICE_EXIT_MAGCOMP                 7
#define NOTICE_WAIT_REPONSE                 8
#define NOTICE_DMIF_TEST                    9
#define NOTICE_ENTER_STANDBY_MODE           10
#define NOTICE_EXIT_STANDBY_MODE            11

/* structure declarations : gui window interface */
typedef struct gui_window_interface
{
    int                 focus;
    int                 notice;
    menu_t              *mainMenu;
    submenu_t           *subMenu;
    stk_t               *dialogStack;
    IDirectFB           *dfb;
    IDirectFBFont       *normalFont[NUM_FONT_HEIGHTS];
    IDirectFBFont       *monoFont[NUM_FONT_HEIGHTS];
    IDirectFBSurface    *imageSurface[NUM_PRELOAD_IMAGES];
    IDirectFBSurface    *guiSurface;
    pthread_mutex_t     mutex;
}
window_t;


/* external functions for gui window */
extern window_t *gui_window_create(IDirectFB *dfb);
extern int gui_window_destroy(window_t *window);

#endif /* _GUI_WINDOW_H_ */
