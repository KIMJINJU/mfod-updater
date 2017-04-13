/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    amod.h
        amod main interface declaration
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/


#ifndef _AMOD_H_
#define _AMOD_H_

#include "core/device.h"
#include "core/magcomp.h"
#include "core/msgproc.h"
#include "core/taqmgr.h"
#include "core/guimgr.h"


#ifndef _USE_K02_37_FOR_POSITION_REPORT_
#ifndef _USE_K05_1_FOR_POSITION_REPORT_
#define _USE_K05_1_FOR_POSITION_REPORT_
#endif
#else
#ifdef _USE_K05_1_FOR_POSITION_REPORT_
#undef _USE_K05_1_FOR_POSITION_REPORT_
#endif
#endif


/* structure declation : main system interface */
typedef struct main_interface
{
    struct device_interface *device;
    struct magcomp_interface *magcomp;
    struct taqmgr_interface *taqmgr;
    struct msgproc_interface *msgproc;
    struct gui_manager_interface *guimgr;
}
main_interface;

#endif /* _AMOD_H_ */
