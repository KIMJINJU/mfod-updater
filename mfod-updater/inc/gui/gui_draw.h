/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_draw.h
        external function/variables/defines for gui drawing functions
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#ifndef _GUI_DRAW_H_
#define _GUI_DRAW_H_

#include "amod.h"
#include "gui/gui_window.h"

/* external functions declaration */
//extern void draw_splash(window_t *);
extern void draw_pwroff_splash(window_t *window);
extern void draw_background(window_t *, struct vgss_interface *);
extern void draw_reticle(window_t *, struct vgss_interface *);
extern void draw_timeinfo(window_t *, struct device_interface *);
extern void draw_observer_location(window_t *, struct taqmgr_interface *);
extern void draw_angular_indicator(window_t *, struct main_interface *);
extern void draw_range_indicator(window_t *, struct taqmgr_interface *);
extern void draw_battery_indicator(window_t *, struct mcu_interface *);
extern void draw_obmode_indicator(window_t *, struct vgss_interface *);
extern void draw_ftmode_indicator(window_t *window, struct vgss_interface *vgss);
extern void draw_irpol_indicator(window_t *, struct vgss_interface *, struct ircam_interface *);
extern void draw_irnuc_indicator(window_t *, struct vgss_interface *, struct ircam_interface *);
extern void draw_ezoom_indicator(window_t *, struct vgss_interface *);
extern void draw_ireis_indicator(window_t *, struct vgss_interface *);
extern void draw_lrfst_indicator(window_t *, struct taqmgr_interface *, struct vgss_interface *, struct lrf_interface *);
extern void draw_azmode_indicator(window_t *, struct vgss_interface *, struct taqmgr_interface *);
extern void draw_celltype_indicator(window_t *, struct vgss_interface *, struct mcu_interface *);
extern void draw_daqmode_indicator(window_t *, struct vgss_interface *, struct taqmgr_interface *);
extern void draw_rgate_indicator(window_t *, struct vgss_interface *, struct lrf_interface *);
extern void draw_taqmode_indicator(window_t *, struct vgss_interface *, struct taqmgr_interface *);
extern void draw_target_information(window_t *, struct taqmgr_interface *);
extern void draw_magnetic_compensation_status(window_t *, struct magcomp_interface *, struct dmc_interface *);
extern void draw_pbit_progress(window_t *, struct device_interface *);
extern void draw_ibit_progress(window_t *, struct main_interface *);
extern void draw_update_progress(window_t *, struct main_interface*);

extern void draw_dialog(window_t *);
extern void draw_menu(window_t *);
extern void draw_submenu(window_t *);
extern void draw_notice(window_t *);

#endif /* _GUI_DRAW_ */
