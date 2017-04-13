/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_proc.h
        external function/variables/defines for gui processing
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#ifndef _GUI_PROC_H_
#define _GUI_PROC_H_

#include "core/guimgr.h"

extern int gui_proc_hotkey(struct gui_manager_interface *, struct device_interface *);
extern int gui_proc_menu();
extern int gui_proc_submenu();
extern int gui_proc_dialog();

#endif /* _GUI_PROC_H_ */
