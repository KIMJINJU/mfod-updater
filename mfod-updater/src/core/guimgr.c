/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    guimgr.c
        external/internal function implementations of gui manager
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/


#include <directfb.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <core/evque.h>


#include "amod.h"
#include "core/logger.h"
#include "core/guimgr.h"
#include "core/sysctrl.h"
//#include "ds/stack.h"
#include "etc/util.h"
#include "gui/gui_str.h"
#include "gui/gui_dialog_creator.h"
#include "gui/gui_draw.h"
#include "gui/gui_keyin_handler.h"
#include "gui/gui_window.h"


/* gui update rate (hz)         */
#define GUI_UPDATE_RATE                     10

/* submenu position flag        */
#define SUBMENU_POSITION_DVMODE             0
#define SUBMENU_POSITION_IRMODE             1
#define SUBMENU_POSITION_FTMODE				2

/* gui focus move flag          */
#define GUI_SET_FOCUS_PREV_ITEM             0
#define GUI_SET_FOCUS_NEXT_ITEM             1
#define GUI_SET_FOCUS_PREV_MENU_ITEM        2
#define GUI_SET_FOCUS_NEXT_MENU_ITEM        3
#define GUI_SET_FOCUS_PREV_SUBMENU_ITEM     4
#define GUI_SET_FOCUS_NEXT_SUBMENU_ITEM     5
#define GUI_SET_FOCUS_PREV_GRID_ROW         6
#define GUI_SET_FOCUS_NEXT_GRID_ROW         7

/* gui post processing flag     */
#define NO_HANDLE_CALLBACK_RETCODE          99


/* structure declaration : gui manager attributes */
struct gui_manager_attribute
{
    /* external interface */
    struct gui_manager_interface extif;

    /* internal interface */
    bool             drawing;
    int              mode;
    pthread_t        tidDrawing;
    window_t         *window;
    keyin_handler_t  *keyinHandler;
};


static void *
guimgr_drawproc(void *arg)
{
    long idle_time = 0;
    double drawing_time = 0.0;
    double refresh_time = 0.0;
    struct timespec t1 = {.tv_sec = 0, .tv_nsec = 0};
    struct timespec t2 = {.tv_sec = 0, .tv_nsec = 0};
    struct main_interface *mainif = (struct main_interface *) arg;
    struct device_interface *device = mainif->device;
    struct taqmgr_interface *taqmgr = mainif->taqmgr;
    struct gui_manager_attribute *this = (struct gui_manager_attribute *) mainif->guimgr;
    IDirectFBSurface *surface = this->window->guiSurface;

    this->drawing = true;
    refresh_time = 1.000000 / GUI_UPDATE_RATE;

    while (this->drawing)
    {
        clock_gettime(CLOCK_REALTIME, &t1);

        switch(this->mode)
        {
        case GUI_MODE_PBIT:
            draw_pbit_progress(this->window, device);
            draw_dialog(this->window);
            break;

        case GUI_MODE_IBIT:
            draw_ibit_progress(this->window, mainif);
            draw_dialog(this->window);
            break;

        case GUI_MODE_OBSERVE:
            draw_background(this->window, device->vgss);
            draw_reticle(this->window, device->vgss);
            draw_timeinfo(this->window, device);
            draw_observer_location(this->window, taqmgr);
            draw_angular_indicator(this->window, mainif);
            draw_range_indicator(this->window, taqmgr);
            draw_battery_indicator(this->window, device->mcu);
            draw_obmode_indicator(this->window, device->vgss);
            draw_irpol_indicator(this->window, device->vgss, device->ircam);
            draw_irnuc_indicator(this->window, device->vgss, device->ircam);
            draw_ezoom_indicator(this->window, device->vgss);
            draw_ireis_indicator(this->window, device->vgss);
            draw_lrfst_indicator(this->window, taqmgr, device->vgss, device->lrf);
            draw_azmode_indicator(this->window, device->vgss, taqmgr);
            draw_daqmode_indicator(this->window, device->vgss, taqmgr);
            draw_rgate_indicator(this->window, device->vgss, device->lrf);
            draw_target_information(this->window, taqmgr);
            draw_taqmode_indicator(this->window, device->vgss, taqmgr);
            draw_menu(this->window);
            draw_submenu(this->window);
            draw_dialog(this->window);
            draw_notice(this->window);
            break;
        case GUI_MODE_FACTORY:
        	draw_background(this->window, device->vgss);
        	draw_reticle(this->window, device->vgss);
        	draw_timeinfo(this->window, device);
        	draw_ftmode_indicator(this->window, device->vgss);
        	draw_battery_indicator(this->window, device->mcu);
        	draw_menu(this->window);
        	draw_submenu(this->window);
        	draw_dialog(this->window);
        	draw_notice(this->window);
        	break;

        case GUI_MODE_FWUPDATE:
        	draw_background(this->window, device->vgss);
        	draw_timeinfo(this->window, device);
        	draw_ftmode_indicator(this->window, device->vgss);
        	draw_battery_indicator(this->window, device->mcu);
        	draw_update_progress(this->window, mainif);
        	draw_dialog(this->window);
        	break;

        case GUI_MODE_PWROFF:
            draw_pwroff_splash(this->window);
            break;

        case GUI_MODE_MAGCOMP:
            draw_background(this->window, device->vgss);
            draw_timeinfo(this->window, device);
            draw_battery_indicator(this->window, device->mcu);
            draw_magnetic_compensation_status(this->window, mainif->magcomp, device->dmc);
            draw_dialog(this->window);
            draw_notice(this->window);
            break;
        }

        surface->Flip(surface, NULL, DSFLIP_WAITFORSYNC);
        clock_gettime(CLOCK_REALTIME, &t2);
        drawing_time = get_elapsed_time(&t1, &t2);

        if (drawing_time < refresh_time)
        {
            idle_time = (long) ((refresh_time - drawing_time) * 1000000);
            usleep(idle_time);
            TLOGMSG(0, ("gui drwing time = %.6f sec\n", drawing_time));
        }
    }

    return NULL;
}


static IDirectFB *
guimgr_init_directfb(void)
{
    IDirectFB *dfb = NULL;

    if (DirectFBInit(NULL, NULL) == DFB_OK)
    {
        if (DirectFBCreate(&dfb) == DFB_OK)
        {
            if ((dfb->SetCooperativeLevel(dfb, DFSCL_FULLSCREEN)) == DFB_OK)
                TLOGMSG(1, ("init directfb\n"));
            else
            {
                dfb->Release(dfb);
                dfb = NULL;
                TLOGMSG(1, (DBGINFOFMT "failed to set cooperative level\n", DBGINFO));
            }
        }
        else
        {
            dfb = NULL;
            TLOGMSG(1, (DBGINFOFMT "failed to create directfb main interface\n", DBGINFO));
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to init directfb\n", DBGINFO));

    return dfb;
}


static void *
guimgr_find_dialog(struct gui_manager_interface *guimgr, int dlgid)
{
	dialog_t *dialog = NULL;
	stk_node_t *node = NULL;
	struct gui_manager_attribute *this = (struct gui_manager_attribute *) guimgr;

	if (this)
	{
		node = this->window->dialogStack->top;

		while (node)
		{
			dialog = (dialog_t *) node->data;

			if (dlgid == dialog->getID(dialog))
				break;
			else
				node = node->next;
		}

		if (!node)
			dialog = NULL;
	}
	else
		TLOGMSG(1, (DBGINFOFMT "null guimgr interface\n", DBGINFO));

	return dialog;
}


static void *
guimgr_get_toplev_dialog(struct gui_manager_interface *guimgr)
{
    void *dialog = NULL;
    struct gui_manager_attribute *this = (struct gui_manager_attribute *) guimgr;

    if (this)
    {
        dialog = stack_get_top(this->window->dialogStack);

        if (dialog == NULL)
            TLOGMSG(1, (DBGINFOFMT "failed to get top level dialog\n", DBGINFO));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null gui manager interface interface\n", DBGINFO));

    return dialog;
}


static int
guimgr_show_dialog(struct gui_manager_interface *guimgr, int dialog_id, void *arg)
{
    int ret = 0;
    dialog_t *dialog = NULL;
    struct gui_manager_attribute *this = (struct gui_manager_attribute *) guimgr;

    /* structure declartion : dialog proprety */
    static struct dialog_table
    {
        int id;
        dialog_t * (*creator)(void *);
    }
    dialog_table[] =
    {
        { .id = GUIID_DIALOG_OBMODE                 , .creator = create_dialog_obmode                   },
        { .id = GUIID_DIALOG_ADJUST                 , .creator = create_dialog_display_adjust           },
        { .id = GUIID_DIALOG_IREIS                  , .creator = create_dialog_ireis                    },
        { .id = GUIID_DIALOG_IRCLR                  , .creator = create_dialog_irclr                    },
        { .id = GUIID_DIALOG_IRGLV                  , .creator = create_dialog_irglv                    },
        { .id = GUIID_DIALOG_IRHEQ                  , .creator = create_dialog_irheq                    },
        { .id = GUIID_DIALOG_TARGET_LIST            , .creator = create_dialog_target_list              },
        { .id = GUIID_DIALOG_XMIT_OBLOC             , .creator = create_dialog_xmit_obloc               },
        { .id = GUIID_DIALOG_INPUT_GRIDVAR          , .creator = create_dialog_input_gridvar            },
        { .id = GUIID_DIALOG_INPUT_OBLOC            , .creator = create_dialog_input_obloc              },
        { .id = GUIID_DIALOG_INPUT_RANGE            , .creator = create_dialog_input_range              },
        { .id = GUIID_DIALOG_GNSS_STATUS            , .creator = create_dialog_gnss_status              },
        { .id = GUIID_DIALOG_SET_COORDSYS           , .creator = create_dialog_coordsys_selection       },
        { .id = GUIID_DIALOG_TIME_CONFIG            , .creator = create_dialog_timeset                  },
        { .id = GUIID_DIALOG_EXTERN_VOUT            , .creator = create_dialog_evout                    },
        { .id = GUIID_DIALOG_DMIF_TEST              , .creator = create_dialog_dmif_test                },
        { .id = GUIID_DIALOG_SLEEP                  , .creator = create_dialog_standby_mode             },
        { .id = GUIID_DIALOG_SYSTEM_INFO            , .creator = create_dialog_system_info              },
        { .id = GUIID_DIALOG_TAQSEL                 , .creator = create_dialog_taqsel                   },
        { .id = GUIID_DIALOG_INVALID_OBLOC          , .creator = create_dialog_invalid_obloc            },
        { .id = GUIID_DIALOG_USER_OBLOC             , .creator = create_dialog_user_obloc               },
        { .id = GUIID_DIALOG_AUTO_OBLOC             , .creator = create_dialog_auto_obloc               },
        { .id = GUIID_DIALOG_INVALID_RANGE          , .creator = create_dialog_invalid_range            },
        { .id = GUIID_DIALOG_USER_RANGE             , .creator = create_dialog_user_range               },
        { .id = GUIID_DIALOG_AUTO_RANGE             , .creator = create_dialog_auto_range               },
        { .id = GUIID_DIALOG_INVALID_GRIDVAR        , .creator = create_dialog_invalid_gridvar          },
        { .id = GUIID_DIALOG_USER_GRIDVAR           , .creator = create_dialog_user_gridvar             },
        { .id = GUIID_DIALOG_AUTO_GRIDVAR           , .creator = create_dialog_auto_gridvar             },
        { .id = GUIID_DIALOG_ERROR_ANGLE_LIMIT      , .creator = create_dialog_angle_error              },
        { .id = GUIID_DIALOG_ERROR_LRF_CTRL         , .creator = create_dialog_lrfctrl_error            },
        { .id = GUIID_DIALOG_ERROR_SHTR_CTRL        , .creator = create_dialog_shtrctrl_error           },
        { .id = GUIID_DIALOG_ERROR_DISP_CTRL        , .creator = create_dialog_dispctrl_error           },
        { .id = GUIID_DIALOG_ERROR_IRCAM_CTRL       , .creator = create_dialog_ircamctrl_error          },
        { .id = GUIID_DIALOG_ERROR_IRCAM_POWER      , .creator = create_dialog_ircampwr_error           },
        { .id = GUIID_DIALOG_ERROR_EVOUT_CTRL       , .creator = create_dialog_evoutctrl_error          },
        { .id = GUIID_DIALOG_UNKNOWN_ERROR          , .creator = create_dialog_unknown_error            },
        { .id = GUIID_DIALOG_ENTER_MAGCOMPMODE      , .creator = create_dialog_enter_magcompmode        },
        { .id = GUIID_DIALOG_EXIT_MAGCOMPMODE       , .creator = create_dialog_exit_magcompmode         },
        { .id = GUIID_DIALOG_RETRY_MAGCOMP          , .creator = create_dialog_retry_magcomp            },
        { .id = GUIID_DIALOG_APPLY_MAGPARM          , .creator = create_dialog_apply_magparm            },
        { .id = GUIID_DIALOG_MAGCOMP_OK             , .creator = create_dialog_success_magcomp          },
        { .id = GUIID_DIALOG_MAGCOMP_ERRFOM         , .creator = create_dialog_magcomp_errfom           },
        { .id = GUIID_DIALOG_MAGCOMP_ERRCALC        , .creator = create_dialog_magcomp_errcalc          },
        { .id = GUIID_DIALOG_MAGCOMP_ERRDAQ         , .creator = create_dialog_magcomp_errdaq           },
        { .id = GUIID_DIALOG_MAGCOMP_ERRINIT        , .creator = create_dialog_magcomp_initerr          },
        { .id = GUIID_DIALOG_XMIT_TALOC             , .creator = create_dialog_xmit_taloc               },
        { .id = GUIID_DIALOG_SELECT_XMITINFO        , .creator = create_dialog_select_xmitifno          },
        { .id = GUIID_DIALOG_ERROR_ZERO_RANGE       , .creator = create_dialog_error_zero_range         },
        { .id = GUIID_DIALOG_ERROR_TARGET_SHIFT     , .creator = create_dialog_error_target_shift       },
        { .id = GUIID_DIALOG_ERROR_TARGET_SIZE      , .creator = create_dialog_error_target_size        },
        { .id = GUIID_DIALOG_ERROR_GEODCALC         , .creator = create_dialog_error_geodcalc           },
        { .id = GUIID_DIALOG_ERROR_DMC_OFFLINE      , .creator = create_dialog_error_dmc_offline        },
        { .id = GUIID_DIALOG_ERROR_GNSS_OFFLINE     , .creator = create_dialog_error_gnss_offline       },
        { .id = GUIID_DIALOG_ERROR_TAQ              , .creator = create_dialog_error_taq                },
        { .id = GUIID_DIALOG_SUCCESS_XMIT           , .creator = create_dialog_success_xmit             },
        { .id = GUIID_DIALOG_SUCCESS_DMIF_TEST      , .creator = create_dialog_success_dmif_test        },
        { .id = GUIID_DIALOG_ERROR_TIMEOUT          , .creator = create_dialog_timeout                  },
        { .id = GUIID_DIALOG_ERROR_NACK_CANPRO2     , .creator = create_dialog_nack_cantpro2            },
        { .id = GUIID_DIALOG_ERROR_NACK_CANPRO15    , .creator = create_dialog_nack_cantpro15           },
        { .id = GUIID_DIALOG_ERROR_NACK_CANPRO19    , .creator = create_dialog_nack_cantpro19           },
        { .id = GUIID_DIALOG_ERROR_NACK_CANPRO25    , .creator = create_dialog_nack_cantpro25           },
        { .id = GUIID_DIALOG_TERMTAQ                , .creator = create_dialog_termtaq                  },
        { .id = GUIID_DIALOG_ERROR_GRIDVAR          , .creator = create_dialog_error_gridvar            },
        { .id = GUIID_DIALOG_LOW_VOLTAGE            , .creator = create_dialog_low_voltage              },
        { .id = GUIID_DIALOG_XMIT_DATA              , .creator = create_dialog_auto_xmit                },
        { .id = GUIID_DIALOG_BIT_PASS               , .creator = create_dialog_bit_pass                 },
        { .id = GUIID_DIALOG_BIT_FAIL               , .creator = create_dialog_bit_fail                 },
        { .id = GUIID_DIALOG_EXEC_BIT               , .creator = create_dialog_exec_bit                 },
        { .id = GUIID_DIALOG_SELECT_SHUTTER         , .creator = create_dialog_select_shutter           },
        { .id = GUIID_DIALOG_COVER_IRLENS           , .creator = create_dialog_cover_irlens             },
        { .id = GUIID_DIALOG_DELETE_TARGET          , .creator = create_dialog_delete_target            },
        { .id = GUIID_DIALOG_TARGET_DETAIL          , .creator = create_dialog_target_detail            },
        { .id = GUIID_DIALOG_SELECT_XMITINFO_TLIST  , .creator = create_dialog_select_xmitifno_at_list  },
        { .id = GUIID_DIALOG_XMIT_TALOC_TLIST       , .creator = create_dialog_xmit_taloc_at_list       },
		{ .id = GUIID_DIALOG_SHORTDIST				, .creator = create_dialog_short_distance_correct	},
		{ .id = GUIID_DIALOG_MIDDIST				, .creator = create_dialog_middle_distance_correct  },
		{ .id = GUIID_DIALOG_LONGDIST				, .creator = create_dialog_long_distance_correct    },
		{ .id = GUIID_DIALOG_DISTCHECK				, .creator = create_dialog_distance_correct_check	},
		{ .id = GUIID_DIALOG_AZM					, .creator = create_dialog_azimuth_correct			},
		{ .id = GUIID_DIALOG_ALTT					, .creator = create_dialog_altitude_correct			},
		{ .id = GUIID_DIALOG_ANGLECHECK				, .creator = create_dialog_angle_correct_check		},
		{ .id = GUIID_DIALOG_SENSOR					, .creator = create_dialog_sensor_correct			},
		{ .id = GUIID_DIALOG_SENSORCHECK			, .creator = create_dialog_sensor_correct_check		},
		{ .id = GUIID_DIALOG_IRARRAY				, .creator = create_dialog_ir_array					},
		{ .id = GUIID_DIALOG_IRCHECK				, .creator = create_dialog_ir_array_check			},
		{ .id = GUIID_DIALOG_FWUPDATE				, .creator = create_dialog_firmware_update			},
		{ .id = GUIID_DIALOG_INIT					, .creator = create_dialog_correct_init				},
		{ .id = GUIID_DIALOG_IRSCALEINPUT			, .creator = create_dialog_ir_scale_input			},
		{ .id = GUIID_DIALOG_MOVE_IR				, .creator = create_dialog_ir_move					},
		{ .id = GUIID_DIALOG_NOFILE					, .creator = create_dialog_fwupdate_nofile			},
		{ .id = GUIID_DIALOG_SUCCESS_FWUPDATE		, .creator = create_dialog_fwupdate_success			},
		{ .id = GUIID_DIALOG_FAIL_FWUPDATE			, .creator = create_dialog_fwupdate_fail			},
		{ .id = GUIID_DIALOG_NOSDCARD				, .creator = create_dialog_fwupdate_nosdcard		},
    };

    if (this)
    {
        for (int i = 0; i < DIM(dialog_table); i++)
        {
            if (dialog_table[i].id == dialog_id)
            {
                if (dialog_table[i].creator)
                    dialog = dialog_table[i].creator(arg);
                else
                    TLOGMSG(1, (DBGINFOFMT "null dialog creator\n", DBGINFO));

                break;
            }
            else
                continue;
        }

        if (dialog)
        {
            stack_push(this->window->dialogStack, (void *)dialog);
            this->window->focus = GUI_FOCUS_DIALOG;
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gui manager interface\n", DBGINFO));
    }

    return ret;
}


static int
guimgr_call_submenu_dialog(struct gui_manager_interface *guimgr, void *arg)
{
    int id = 0;
    int ret = -1;
    struct gui_manager_attribute *this = (struct gui_manager_attribute *) guimgr;
    submenu_t *submenu = this->window->mainMenu->getFocus(this->window->mainMenu)->submenu;

    static struct guiid_table
    {
        int submenu;
        int dialog;
    }
    guiid_table[] =
    {
        {.submenu = GUIID_SUBMENU_IREIS         , .dialog = GUIID_DIALOG_IREIS              },
        {.submenu = GUIID_SUBMENU_IRCLR         , .dialog = GUIID_DIALOG_IRCLR              },
        {.submenu = GUIID_SUBMENU_IRGLV         , .dialog = GUIID_DIALOG_IRGLV              },
        {.submenu = GUIID_SUBMENU_IRHEQ         , .dialog = GUIID_DIALOG_IRHEQ              },
        {.submenu = GUIID_SUBMENU_IRSHTR        , .dialog = GUIID_DIALOG_SELECT_SHUTTER     },
        {.submenu = GUIID_SUBMENU_TARGET_LIST   , .dialog = GUIID_DIALOG_TARGET_LIST        },
        {.submenu = GUIID_SUBMENU_COMP_DMC      , .dialog = GUIID_DIALOG_ENTER_MAGCOMPMODE  },
        {.submenu = GUIID_SUBMENU_XMIT_OBLOC    , .dialog = GUIID_DIALOG_XMIT_OBLOC         },
        {.submenu = GUIID_SUBMENU_INPUT_GRIDVAR , .dialog = GUIID_DIALOG_INPUT_GRIDVAR      },
        {.submenu = GUIID_SUBMENU_INPUT_OBLOC   , .dialog = GUIID_DIALOG_INPUT_OBLOC        },
        {.submenu = GUIID_SUBMENU_INPUT_RANGE   , .dialog = GUIID_DIALOG_INPUT_RANGE        },
        {.submenu = GUIID_SUBMENU_GNSS_STATUS   , .dialog = GUIID_DIALOG_GNSS_STATUS        },
        {.submenu = GUIID_SUBMENU_SET_COORDSYS  , .dialog = GUIID_DIALOG_SET_COORDSYS       },
        {.submenu = GUIID_SUBMENU_TIME_CONFIG   , .dialog = GUIID_DIALOG_TIME_CONFIG        },
        {.submenu = GUIID_SUBMENU_EXTERN_VOUT   , .dialog = GUIID_DIALOG_EXTERN_VOUT        },
        {.submenu = GUIID_SUBMENU_DMIF_TEST     , .dialog = GUIID_DIALOG_DMIF_TEST          },
        {.submenu = GUIID_SUBMENU_IBIT          , .dialog = GUIID_DIALOG_EXEC_BIT           },
        {.submenu = GUIID_SUBMENU_SLEEP         , .dialog = GUIID_DIALOG_SLEEP              },
        {.submenu = GUIID_SUBMENU_SYSTEM_INFO   , .dialog = GUIID_DIALOG_SYSTEM_INFO        },
        {.submenu = GUIID_SUBMENU_XMIT_DATA     , .dialog = GUIID_DIALOG_XMIT_DATA          },
		{.submenu = GUIID_SUBMENU_SHORTDIST		, .dialog = GUIID_DIALOG_SHORTDIST			},
		{.submenu = GUIID_SUBMENU_MIDDIST		, .dialog = GUIID_DIALOG_MIDDIST			},
		{.submenu = GUIID_SUBMENU_LONGDIST		, .dialog = GUIID_DIALOG_LONGDIST			},
		{.submenu = GUIID_SUBMENU_DISTCHECK		, .dialog = GUIID_DIALOG_DISTCHECK			},
		{.submenu = GUIID_SUBMENU_AZM			, .dialog = GUIID_DIALOG_AZM				},
		{.submenu = GUIID_SUBMENU_ALTT			, .dialog = GUIID_DIALOG_ALTT				},
		{.submenu = GUIID_SUBMENU_ANGLECHECK	, .dialog = GUIID_DIALOG_ANGLECHECK			},
		{.submenu = GUIID_SUBMENU_SENSOR		, .dialog = GUIID_DIALOG_SENSOR				},
		{.submenu = GUIID_SUBMENU_SENSORCHECK	, .dialog = GUIID_DIALOG_SENSORCHECK		},
		{.submenu = GUIID_SUBMENU_IRARRAY		, .dialog = GUIID_DIALOG_IRARRAY			},
		{.submenu = GUIID_SUBMENU_IRSCALEINPUT,	  .dialog = GUIID_DIALOG_IRSCALEINPUT		},
		{.submenu = GUIID_SUBMENU_IRCHECK		, .dialog = GUIID_DIALOG_IRCHECK			},
		{.submenu = GUIID_SUBMENU_FWUPDATE		, .dialog = GUIID_DIALOG_FWUPDATE			},
		{.submenu = GUIID_SUBMENU_INIT			, .dialog = GUIID_DIALOG_INIT				}
    };

    id = submenu->getFocus(submenu)->id;

    for (int i = 0; i < DIM(guiid_table); i++)
    {
        if (id == guiid_table[i].submenu)
        {
            ret = guimgr_show_dialog(guimgr, guiid_table[i].dialog, arg);
            break;
        }
        else
            continue;
    }

    return ret;
}


static int
guimgr_hide_toplev_dialog(struct gui_manager_interface *guimgr)
{
    int ret = 0;
    dialog_t *dialog = NULL;
    struct gui_manager_attribute *this = (struct gui_manager_attribute *) guimgr;

    if (this)
    {
        pthread_mutex_lock(&this->window->mutex);
        dialog = stack_pop(this->window->dialogStack);

        if (dialog)
        {
            gui_dialog_destroy(dialog);

            if (stack_is_empty(this->window->dialogStack) == 0)
            {
                if (this->window->mainMenu)
                    this->window->focus = GUI_FOCUS_MENU;
                else
                    this->window->focus = GUI_FOCUS_NONE;
            }
            else
                this->window->focus = GUI_FOCUS_DIALOG;
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to get dialog from stack\n", DBGINFO));
        }

        pthread_mutex_unlock(&this->window->mutex);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gui manager interface\n", DBGINFO));
    }

    return ret;
}


static submenu_t *
guimgr_create_distcorrect_submenu(int pos)
{
	submenu_t *submenu = NULL;
	static rect_t pos_ftmode = {.x = 60, .y = 444, .w =160, .h = 90};

	static submenu_item_t items[] =
	{
		{.id = GUIID_SUBMENU_SHORTDIST,		.string = STR_DISTCRR_SHORT 	},
		{.id = GUIID_SUBMENU_MIDDIST,		.string = STR_DISTCRR_MIDDLE	},
		{.id = GUIID_SUBMENU_LONGDIST,		.string = STR_DISTCRR_LONG		},
		{.id = GUIID_SUBMENU_DISTCHECK,		.string = STR_DISTCRR_CHECK		}
	};

	submenu = gui_submenu_create();

	if(submenu)
	{
		for(int i = 0; i < DIM(items); i++)
			submenu->addItem(submenu, items[i].id, items[i].string);

		submenu->setPosition(submenu, &pos_ftmode);
	}
	else
	    TLOGMSG(1, (DBGINFOFMT "failed to create submenu\n", DBGINFO));

	return submenu;
}



static submenu_t *
guimgr_create_anglecorrect_submenu(int pos)
{
	submenu_t *submenu = NULL;
	static rect_t pos_ftmode = {.x = 190, .y = 464, .w =160, .h = 70};

	static submenu_item_t items[] =
	{
		{.id = GUIID_SUBMENU_AZM,			.string = STR_ANGCRR_AZM 	},
		{.id = GUIID_SUBMENU_ALTT,			.string = STR_ANGCRR_ALTT	},
		{.id = GUIID_SUBMENU_ANGLECHECK,	.string = STR_ANGCRR_CHECK	}
	};

	submenu = gui_submenu_create();

	if(submenu)
	{
		for(int i = 0; i < DIM(items); i++)
			submenu->addItem(submenu, items[i].id, items[i].string);

		submenu->setPosition(submenu, &pos_ftmode);
	}
	else
	    TLOGMSG(1, (DBGINFOFMT "failed to create submenu\n", DBGINFO));

	return submenu;
}


static submenu_t *
guimgr_create_sensorcorrect_submenu(int pos)
{
	submenu_t *submenu = NULL;
	static rect_t pos_ftmode = {.x = 320, .y = 484, .w =160, .h = 50};

	static submenu_item_t items[] =
	{
		{.id = GUIID_SUBMENU_SENSOR,		.string = STR_SENCRR_SENSOR },
		{.id = GUIID_SUBMENU_SENSORCHECK,	.string = STR_SENCRR_CHECK	}
	};

	submenu = gui_submenu_create();

	if(submenu)
	{
		for(int i = 0; i < DIM(items); i++)
			submenu->addItem(submenu, items[i].id, items[i].string);

		submenu->setPosition(submenu, &pos_ftmode);
	}
	else
	    TLOGMSG(1, (DBGINFOFMT "failed to create submenu\n", DBGINFO));

	return submenu;
}

static submenu_t *
guimgr_create_irarray_submenu(int pos)
{
	submenu_t *submenu = NULL;
	static rect_t pos_ftmode = {.x = 450, .y = 464, .w =160, .h = 70};

	static submenu_item_t items[] =
	{
		{.id = GUIID_SUBMENU_IRARRAY,		.string = STR_IRARR_ARRAY 			},
		{.id = GUIID_SUBMENU_IRSCALEINPUT,	.string = STR_IRRARR_IRSCALEINPUT	},
		{.id = GUIID_SUBMENU_IRCHECK,		.string = STR_IRARR_CHECK 			}
	};

	submenu = gui_submenu_create();

	if(submenu)
	{
		for(int i = 0; i < DIM(items); i++)
			submenu->addItem(submenu, items[i].id, items[i].string);

		submenu->setPosition(submenu, &pos_ftmode);
	}
	else
	    TLOGMSG(1, (DBGINFOFMT "failed to create submenu\n", DBGINFO));

	return submenu;
}


static submenu_t *
guimgr_create_fwupdate_submenu(int pos)
{
	submenu_t *submenu = NULL;
	static rect_t pos_ftmode = {.x = 570, .y = 484, .w =160, .h = 50};

	static submenu_item_t items[] =
	{
		{.id = GUIID_SUBMENU_FWUPDATE,	.string = STR_FWUPDATE_UPDATE },
		{.id = GUIID_SUBMENU_INIT,		.string = STR_FWUPDATE_INIT	  }
	};

	submenu = gui_submenu_create();

	if(submenu)
	{
		for(int i = 0; i < DIM(items); i++)
			submenu->addItem(submenu, items[i].id, items[i].string);

		submenu->setPosition(submenu, &pos_ftmode);
	}
	else
	    TLOGMSG(1, (DBGINFOFMT "failed to create submenu\n", DBGINFO));

	return submenu;
}


static submenu_t *
guimgr_create_irconf_submenu(int pos)
{
    submenu_t *submenu = NULL;
    static rect_t pos_dvmode = {.x = 260, .y = 364, .w = 140, .h = 170};
    static rect_t pos_irmode = {.x = 190, .y = 424, .w = 140, .h = 110};

    static submenu_item_t items[] =
    {
        {.id = GUIID_SUBMENU_IREIS , .string = STR_IRCONF_IREIS  },
        {.id = GUIID_SUBMENU_IRCLR , .string = STR_IRCONF_IRCLR  },
        {.id = GUIID_SUBMENU_IRGLV , .string = STR_IRCONF_IRGLV  },
        {.id = GUIID_SUBMENU_IRHEQ , .string = STR_IRCONF_IRHEQ  },
        {.id = GUIID_SUBMENU_IRSHTR, .string = STR_IRCONF_SHTRSEL},
    };

    submenu = gui_submenu_create();

    if (submenu)
    {
        for (int i = 0; i < DIM(items); i++)
            submenu->addItem(submenu, items[i].id, items[i].string);

        if (pos == SUBMENU_POSITION_DVMODE)
            submenu->setPosition(submenu, &pos_dvmode);
        else
            submenu->setPosition(submenu, &pos_irmode);
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create submenu\n", DBGINFO));

    return submenu;
}


static submenu_t *
guimgr_create_target_submenu(int pos)
{
    submenu_t *submenu = NULL;
    static rect_t pos_dvmode = {.x = 260, .y = 384, .w = 140, .h = 150};
    static rect_t pos_irmode = {.x = 330, .y = 384, .w = 140, .h = 150};

    static submenu_item_t items[] =
    {
        {.id = GUIID_SUBMENU_TARGET_LIST  , .string = STR_TARGET_SHOW_LIST      },
        {.id = GUIID_SUBMENU_XMIT_OBLOC   , .string = STR_TARGET_XMIT_OBLOC     },
        {.id = GUIID_SUBMENU_COMP_DMC     , .string = STR_TARGET_COMP_DMC       },
        {.id = GUIID_SUBMENU_INPUT_GRIDVAR, .string = STR_TARGET_INPUT_GRIDVAR  },
        {.id = GUIID_SUBMENU_INPUT_OBLOC  , .string = STR_TARGET_INPUT_OBLOC    },
        {.id = GUIID_SUBMENU_INPUT_RANGE  , .string = STR_TARGET_INPUT_RANGE    },
        {.id = GUIID_SUBMENU_GNSS_STATUS  , .string = STR_TARGET_GNSS_STATUS    },
    };

    submenu = gui_submenu_create();

    if (submenu)
    {
        for (int i = 0; i < DIM(items); i++)
            submenu->addItem(submenu, items[i].id, items[i].string);

        if (pos == SUBMENU_POSITION_DVMODE)
            submenu->setPosition(submenu, &pos_dvmode);
        else
            submenu->setPosition(submenu, &pos_irmode);
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create submenu\n", DBGINFO));

    return submenu;
}


static submenu_t *
guimgr_create_devconf_submenu(int pos)
{
    submenu_t *submenu = NULL;
    static rect_t pos_dvmode = {.x = 400, .y = 364, .w = 140, .h = 170};
    static rect_t pos_irmode = {.x = 470, .y = 364, .w = 140, .h = 170};

    static submenu_item_t items[] =
    {
        {.id = GUIID_SUBMENU_SET_COORDSYS   , .string = STR_DEVCONF_SET_COORDSYS},
        {.id = GUIID_SUBMENU_TIME_CONFIG    , .string = STR_DEVCONF_TIME_CONFIG },
        {.id = GUIID_SUBMENU_EXTERN_VOUT    , .string = STR_DEVCONF_EXTERN_VOUT },
        {.id = GUIID_SUBMENU_XMIT_DATA      , .string = STR_DEVCONF_XMIT_DATA   },
        {.id = GUIID_SUBMENU_DMIF_TEST      , .string = STR_DEVCONF_DMIF_TEST   },
        {.id = GUIID_SUBMENU_IBIT           , .string = STR_DEVCONF_IBIT        },
        {.id = GUIID_SUBMENU_SLEEP          , .string = STR_DEVCONF_SLEEP       },
        {.id = GUIID_SUBMENU_SYSTEM_INFO     , .string = STR_DEVCONF_SYSTEM_INFO     },
    };

    submenu = gui_submenu_create();

    if (submenu)
    {
        for (int i = 0; i < DIM(items); i++)
            submenu->addItem(submenu, items[i].id, items[i].string);

        if (pos == SUBMENU_POSITION_DVMODE)
            submenu->setPosition(submenu, &pos_dvmode);
        else
            submenu->setPosition(submenu, &pos_irmode);
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create submenu\n", DBGINFO));

    return submenu;
}


static menu_t *
guimgr_create_ftmode_menu(void)
{
	menu_t *menu = NULL;

	struct item_property
	{
		menu_item_t item;
		submenu_t *(*submenu_creater)(int);
	}

	prop[] =
	{
		{{GUIID_FTMENU_DIST_CORRECT, 	STR_DIST_CORRECT, 	{90, 540, 100, 40},	NULL}, guimgr_create_distcorrect_submenu	},
		{{GUIID_FTMENU_ANGLE_CORRECT, 	STR_ANGLE_CORRECT, 	{220, 540, 100, 40}, NULL}, guimgr_create_anglecorrect_submenu	},
		{{GUIID_FTMENU_SENSOR_CORRECT, 	STR_SENSOR_CORRECT, {350, 540, 100, 40}, NULL}, guimgr_create_sensorcorrect_submenu	},
		{{GUIID_FTMENU_IR_ARRAY, 		STR_IR_ARRAY, 		{480, 540, 100, 40}, NULL}, guimgr_create_irarray_submenu		},
		{{GUIID_FTMENU_FW_UPDATE, 		STR_FW_UPDATE, 		{610, 540, 100, 40}, NULL}, guimgr_create_fwupdate_submenu		}
	};

	menu = gui_menu_create();

	if(menu)
	{
		for(int i = 0; i < DIM(prop); i++)
		{
			prop[i].item.submenu = prop[i].submenu_creater(SUBMENU_POSITION_FTMODE);

			if(prop[i].item.submenu)
				menu->addItem(menu, prop[i].item.id, prop[i].item.string, &(prop[i].item.position), prop[i].item.submenu);
			else
			{
				gui_menu_destroy(menu);
				menu = NULL;
				TLOGMSG(1, (DBGINFOFMT "failed to create ftmode menu\n", DBGINFO));
				break;
			}
		}
	}
	else
		TLOGMSG(1, (DBGINFOFMT "failed to create menu\n", DBGINFO));

	return menu;
}


static menu_t *
guimgr_create_dvmode_menu(void)
{
    menu_t *menu = NULL;

    struct item_property
    {
        menu_item_t item;
        submenu_t *(*submenu_creator)(int);
    }
    prop[] =
    {
        {{GUIID_MENU_TARGET , STR_TARGET , {280, 540, 100, 40}, NULL}, guimgr_create_target_submenu },
        {{GUIID_MENU_DEVCONF, STR_DEVCONF, {420, 540, 100, 40}, NULL}, guimgr_create_devconf_submenu}
    };


    menu = gui_menu_create();

    if (menu)
    {
        for (int i = 0; i < DIM(prop); i++)
        {
            prop[i].item.submenu = prop[i].submenu_creator(SUBMENU_POSITION_DVMODE);

            if (prop[i].item.submenu)
                menu->addItem(menu, prop[i].item.id, prop[i].item.string, &(prop[i].item.position), prop[i].item.submenu);
            else
            {
                gui_menu_destroy(menu);
                menu = NULL;
                TLOGMSG(1, (DBGINFOFMT "failed to create dvmode menu\n", DBGINFO));
                break;
            }
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create menu\n", DBGINFO));;

    return menu;
}


static menu_t *
guimgr_create_irmode_menu(void)
{
    menu_t *menu = NULL;

    struct item_property
    {
        menu_item_t item;
        submenu_t *(*submenu_creator)(int);
    }
    prop[] =
    {
        {{GUIID_MENU_IRCONF ,  STR_IRCONF , {210, 540, 100, 40}, NULL}, guimgr_create_irconf_submenu  },
        {{GUIID_MENU_TARGET ,  STR_TARGET , {350, 540, 100, 40}, NULL}, guimgr_create_target_submenu  },
        {{GUIID_MENU_DEVCONF,  STR_DEVCONF, {490, 540, 100, 40}, NULL}, guimgr_create_devconf_submenu }
    };

    menu = gui_menu_create();

    if (menu)
    {
        for (int i = 0; i < DIM(prop); i++)
        {
            prop[i].item.submenu = prop[i].submenu_creator(SUBMENU_POSITION_IRMODE);

            if (prop[i].item.submenu)
                menu->addItem(menu, prop[i].item.id, prop[i].item.string, &(prop[i].item.position), prop[i].item.submenu);
            else
            {
                gui_menu_destroy(menu);
                menu = NULL;
                TLOGMSG(1, (DBGINFOFMT "failed to create irmode menu\n", DBGINFO));
                break;
            }
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create menu\n", DBGINFO));;

    return menu;
}


static int
guimgr_show_menu(struct gui_manager_interface *guimgr, void *arg, bool flag)
{
    int ret = 0;
    struct main_interface *amod = (struct main_interface *) arg;
    struct vgss_interface *vgss = amod->device->vgss;
    struct gui_manager_attribute *this = (struct gui_manager_attribute *) guimgr;

    if (this)
    {
        pthread_mutex_lock(&this->window->mutex);
        if(flag)
        {
			if (!this->window->mainMenu)
			{
				this->window->mainMenu = guimgr_create_ftmode_menu();

				if (this->window->mainMenu)
					this->window->focus = GUI_FOCUS_MENU;
				else
				{
					ret = 1;
					TLOGMSG(1, (DBGINFOFMT "failed to create menu\n", DBGINFO));
				}
			}
        }
        else
        {
        	if(this->window->mainMenu)
        	{
        		gui_menu_destroy(this->window->mainMenu);
        		this->window->mainMenu = NULL;
        		this->window->focus = GUI_FOCUS_NONE;
        	}
        }

        pthread_mutex_unlock(&this->window->mutex);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gui manager interface\n", DBGINFO));
    }

    return ret;
}


static int
guimgr_show_submenu(struct gui_manager_interface *guimgr, void *arg, bool flag)
{
    int ret = 0;
    menu_t *menu = NULL;
    submenu_t *submenu = NULL;
    struct main_interface *amod = (struct main_interface *) arg;
    struct vgss_interface *vgss = amod->device->vgss;
    struct gui_manager_attribute *this = (struct gui_manager_attribute *) guimgr;

    if (this)
    {
        pthread_mutex_lock(&this->window->mutex);

        if (flag)
        {
            menu = this->window->mainMenu;

            switch(menu->getFocus(menu)->id)
            {
            case GUIID_FTMENU_DIST_CORRECT:
            	submenu = guimgr_create_distcorrect_submenu(SUBMENU_POSITION_FTMODE);
            	break;
            case GUIID_FTMENU_ANGLE_CORRECT:
            	submenu = guimgr_create_anglecorrect_submenu(SUBMENU_POSITION_FTMODE);
            	break;
            case GUIID_FTMENU_SENSOR_CORRECT:
            	submenu = guimgr_create_sensorcorrect_submenu(SUBMENU_POSITION_FTMODE);
            	break;
            case GUIID_FTMENU_IR_ARRAY:
            	submenu = guimgr_create_irarray_submenu(SUBMENU_POSITION_FTMODE);
            	break;
            case GUIID_FTMENU_FW_UPDATE:
            	submenu = guimgr_create_fwupdate_submenu(SUBMENU_POSITION_FTMODE);
            	break;
            default:
                TLOGMSG(1, (DBGINFOFMT "invalid menu guiid \n", DBGINFO));
                break;
            }

            if (submenu)
            {
                this->window->focus = GUI_FOCUS_SUBMENU;
                this->window->subMenu = submenu;
            }
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "failed to create submenu\n", DBGINFO));
            }
        }
        else
        {
            if (this->window->subMenu)
            {
                gui_submenu_destroy(this->window->subMenu);
                this->window->subMenu = NULL;
                this->window->focus = GUI_FOCUS_MENU;
            }
        }

        pthread_mutex_unlock(&this->window->mutex);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gui manager interface\n", DBGINFO));
    }

    return ret;
}


static int
guimgr_move_focus(struct gui_manager_interface *guimgr, int flag)
{
    int ret = NO_HANDLE_CALLBACK_RETCODE;
    menu_t *menu = NULL;
    grid_t *grid = NULL;
    submenu_t *submenu = NULL;
    dialog_t  *dialog = NULL;
    dialog_widget_t *widget = NULL;

    struct gui_manager_attribute *this = (struct gui_manager_attribute *) guimgr;

    switch (this->window->focus)
    {
    case GUI_FOCUS_MENU:
        menu = this->window->mainMenu;
        submenu = menu->getFocus(menu)->submenu;

        switch (flag)
        {
        case GUI_SET_FOCUS_PREV_MENU_ITEM:
            menu->focusPrev(menu);
            break;

        case GUI_SET_FOCUS_NEXT_MENU_ITEM:
            menu->focusNext(menu);
            break;

        case GUI_SET_FOCUS_PREV_SUBMENU_ITEM:
            submenu->focusPrev(submenu);
            break;

        case GUI_SET_FOCUS_NEXT_SUBMENU_ITEM:
            submenu->focusNext(submenu);
            break;

        default:
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid focus move flag\n", DBGINFO));
            break;
        }

        break;

    case GUI_FOCUS_DIALOG:
        dialog = (dialog_t *) stack_get_top(this->window->dialogStack);

        if (dialog)
        {
            switch (flag)
            {
            case GUI_SET_FOCUS_PREV_ITEM:
                dialog->focusPrev(dialog);
                break;

            case GUI_SET_FOCUS_NEXT_ITEM:
                dialog->focusNext(dialog);
                break;

            default:
                if (dialog->getID(dialog) == GUIID_DIALOG_TARGET_LIST)
                {
                    widget = dialog->findWidget(dialog, GUIID_GRID_TARGET_LIST);

                    if (widget)
                    {
                        grid = (grid_t *) widget->data;

                        if (flag == GUI_SET_FOCUS_NEXT_GRID_ROW)
                            grid->setNextRow(grid);
                        else
                            grid->setPrevRow(grid);

                        ret = flag;
                    }
                }

                break;
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to get dialog from dialog stack\n", DBGINFO));
        }

        break;

    default:
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "invalid focus \n", DBGINFO));
        break;
    }

    return ret;
}


static int
guimgr_process_hotkey(struct gui_manager_interface *guimgr, unsigned int keyin, void *arg)
{
    int ret = 0;
    struct main_interface *mainif = (struct main_interface *) arg;
    struct taqmgr_interface *taqmgr = mainif->taqmgr;
    struct vgss_interface *vgss = mainif->device->vgss;

    if (guimgr->getMode(guimgr) == GUI_MODE_MAGCOMP)
    {
        if (keyin == KEYIN_ENTER_HOLD)
            guimgr->showDialog(guimgr, GUIID_DIALOG_EXIT_MAGCOMPMODE, arg);
    }
    else
    {
        switch (keyin)
        {
        case KEYIN_UP_RELEASE:
        case KEYIN_UP_HOLD_RELEASE:
            sysctrl_toggle_irpol(arg);
            break;

        case KEYIN_DOWN_RELEASE:
        case KEYIN_DOWN_HOLD_RELEASE:
            sysctrl_update_nuc(arg);
            break;

        case KEYIN_LEFT_RELEASE:
        case KEYIN_LEFT_HOLD_RELEASE:
            sysctrl_handle_left_keyin(arg);
            break;

        case KEYIN_RIGHT_RELEASE:
        case KEYIN_RIGHT_HOLD_RELEASE:
            sysctrl_handle_right_keyin(arg);
            break;

        case KEYIN_ENTER_HOLD:
        	if(vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE) == PREVIEW_STATE_STOP)
        		guimgr_show_menu(guimgr, arg, true);

            break;


        default:
            ret = -1;
            TLOGMSG(0, (DBGINFOFMT "undefined key input for hotkey\n", DBGINFO));
            break;
        }
    }

    TLOGMSG(0, ("key input = 0x%04X\n", keyin));

    return ret;
}


static int
guimgr_process_menu(struct gui_manager_interface *guimgr, unsigned int keyin, void *arg)
{
    int ret = 0;
    struct main_interface *mainif = (struct main_interface *) arg;
    struct vgss_interface *vgss = mainif->device->vgss;

    switch (keyin)
    {
    case KEYIN_LEFT_RELEASE:
    case KEYIN_LEFT_REPEAT:
        guimgr_move_focus(guimgr, GUI_SET_FOCUS_PREV_MENU_ITEM);
        break;

    case KEYIN_RIGHT_RELEASE:
    case KEYIN_RIGHT_REPEAT:
        guimgr_move_focus(guimgr, GUI_SET_FOCUS_NEXT_MENU_ITEM);
        break;

    case KEYIN_UP_RELEASE:
    case KEYIN_UP_REPEAT:
        guimgr_move_focus(guimgr, GUI_SET_FOCUS_PREV_SUBMENU_ITEM);
        break;

    case KEYIN_DOWN_RELEASE:
    case KEYIN_DOWN_REPEAT:
        guimgr_move_focus(guimgr, GUI_SET_FOCUS_NEXT_SUBMENU_ITEM);
        break;

    case KEYIN_ENTER_RELEASE:
        guimgr_call_submenu_dialog(guimgr, arg);
        break;

    case KEYIN_ENTER_HOLD:
        if(vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE) == PREVIEW_STATE_STOP)
        	guimgr_show_menu(guimgr, arg, false);
        break;

    default:
        ret = -1;
        TLOGMSG(0, (DBGINFOFMT "undefined key input for menu interface\n", DBGINFO));
        break;
    }

    TLOGMSG(0, ("key input = 0x%04X\n", keyin));

    return ret;
}


static int
guimgr_process_submenu(struct gui_manager_interface *guimgr, unsigned int keyin, void *arg)
{
    int ret = 0;
    struct main_interface *mainif = (struct main_interface *) arg;
    struct vgss_interface *vgss = mainif->device->vgss;

    switch (keyin)
    {
    case KEYIN_UP_RELEASE:
    case KEYIN_UP_REPEAT:
        guimgr_move_focus(guimgr, GUI_SET_FOCUS_PREV_ITEM);
        break;

    case KEYIN_DOWN_RELEASE:
    case KEYIN_DOWN_REPEAT:
        guimgr_move_focus(guimgr, GUI_SET_FOCUS_NEXT_ITEM);
        break;

    case KEYIN_ENTER_RELEASE:
        guimgr_call_submenu_dialog(guimgr, arg);
        break;

    case KEYIN_ENTER_HOLD:
    	if(vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE) == PREVIEW_STATE_STOP)
    		 guimgr_show_submenu(guimgr, arg, false);
        break;

    default:
        ret = -1;
        TLOGMSG(0, (DBGINFOFMT "undefined key input for submenu interface\n", DBGINFO));
        break;
    }

    TLOGMSG(0, ("key input = 0x%04X\n", keyin));

    return ret;
}


static int
guimgr_process_button(struct gui_manager_interface *guimgr, button_t *button, unsigned int keyin, void *arg)
{
    int ret = NO_HANDLE_CALLBACK_RETCODE;

    switch (keyin)
    {
    case KEYIN_UP_RELEASE:
    case KEYIN_UP_REPEAT:
        ret = guimgr_move_focus(guimgr, GUI_SET_FOCUS_PREV_GRID_ROW);
        break;

    case KEYIN_DOWN_RELEASE:
    case KEYIN_DOWN_REPEAT:
        ret = guimgr_move_focus(guimgr, GUI_SET_FOCUS_NEXT_GRID_ROW);
        break;

    case KEYIN_LEFT_RELEASE:
    case KEYIN_LEFT_REPEAT:
        guimgr_move_focus(guimgr, GUI_SET_FOCUS_PREV_ITEM);
        break;

    case KEYIN_RIGHT_RELEASE:
    case KEYIN_RIGHT_REPEAT:
        guimgr_move_focus(guimgr, GUI_SET_FOCUS_NEXT_ITEM);
        break;

    case KEYIN_ENTER_RELEASE:
        ret = button->execCallback(button, arg);
        break;

    case KEYIN_ENTER_HOLD:
        //if ((guimgr->getMode(guimgr) != GUI_MODE_MAGCOMP)
        if (guimgr->getMode(guimgr) == GUI_MODE_OBSERVE)
            guimgr_hide_toplev_dialog(guimgr);

        break;

    default:
        TLOGMSG(0, (DBGINFOFMT "undefined key input for button widget\n", DBGINFO));
        break;
    }

    TLOGMSG(0, ("key input = 0x%04X\n", keyin));

    return ret;
}


static void
guimgr_post_process_button(struct gui_manager_interface *guimgr, int dlgid, int retcode, void *arg)
{
    int ret = 0;
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    dialog_widget_t *widget = NULL;
    struct main_interface *mainif = (struct main_interface *) arg;
    struct gui_manager_attribute *this = (struct gui_manager_attribute *) guimgr;
	struct vgss_interface    *vgss  = mainif->device->vgss;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;

    switch(dlgid)
    {
    case GUIID_DIALOG_OBMODE:
        if (retcode == SYSCTRL_RETCODE_FAIL_IRCAM_POWER)
        {
            guimgr->hideToplevDialog(guimgr);
            guimgr->showDialog(guimgr, GUIID_DIALOG_ERROR_IRCAM_POWER, arg);
        }
        else if (retcode == SYSCTRL_RETCODE_FAIL_SHTR_CTRL)
        {
            guimgr->hideToplevDialog(guimgr);
            guimgr->showDialog(guimgr, GUIID_DIALOG_ERROR_SHTR_CTRL, arg);
        }
        else
            guimgr->hideToplevDialog(guimgr);

        break;

    case GUIID_DIALOG_IRARRAY:
    	if(vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE) == PREVIEW_STATE_RUN)
    	{
    		guimgr->hideToplevDialog(guimgr);
    		if(retcode == SYSCTRL_RETCODE_CHANGE_OBMODE_SUCCESS)
    		{
    			guimgr->showMenu(guimgr, arg, false);
    			guimgr->showDialog(guimgr, GUIID_DIALOG_MOVE_IR, arg);
    		}
    	}
    	else
    	{
    		guimgr->hideToplevDialog(guimgr);
    		if(retcode == SYSCTRL_RETCODE_CHANGE_OBMODE_SUCCESS)
    		{
    			guimgr->hideToplevDialog(guimgr);
    			guimgr->showMenu(guimgr, arg, true);
    		}
    	}
    	break;
    case GUIID_DIALOG_IRSCALEINPUT:
    	dialog = guimgr->getToplevDialog(guimgr);
    	widget = dialog->getFocus(dialog);

    	if(widget->id == GUIID_BUTTON_ON)
    		taqmgr->setIrScaleUp(taqmgr, "1");
    	else
    		taqmgr->setIrScaleUp(taqmgr, "0");
    	guimgr->hideToplevDialog(guimgr);
    	break;

    case GUIID_DIALOG_FWUPDATE:
    	dialog = guimgr->getToplevDialog(guimgr);
    	widget = dialog->getFocus(dialog);

    	if(widget->id == GUIID_BUTTON_YES)
    	{
    		guimgr->lockKeyin(guimgr);
    		guimgr->hideToplevDialog(guimgr);
    		guimgr->showMenu(guimgr, arg, false);
    		guimgr->setMode(guimgr, GUI_MODE_FWUPDATE);

    		ret = sysctrl_firmware_update(arg);

    		guimgr->setMode(guimgr, GUI_MODE_FACTORY);
    		if(ret == SYSCTRL_RETCODE_OK)
    			guimgr->showDialog(guimgr,  GUIID_DIALOG_SUCCESS_FWUPDATE, arg);
    		else if(ret == SYSCTRL_RETCODE_FWUPDATE_NOSDCARD)
    			guimgr->showDialog(guimgr, GUIID_DIALOG_NOSDCARD, arg);
    		else if(ret == SYSCTRL_RETCODE_FWUPDATE_FAIL)
    			guimgr->showDialog(guimgr, GUIID_DIALOG_FAIL_FWUPDATE, arg);
    		else
    			guimgr->showDialog(guimgr, GUIID_DIALOG_NOFILE, arg);

    		guimgr->unlockKeyin(guimgr);
    	}
    	else
    		guimgr->hideToplevDialog(guimgr);
    	break;

    case GUIID_DIALOG_NOFILE:
    case GUIID_DIALOG_SUCCESS_FWUPDATE:
    case GUIID_DIALOG_NOSDCARD:
    case GUIID_DIALOG_FAIL_FWUPDATE:
    	guimgr->hideToplevDialog(guimgr);
    	guimgr->showMenu(guimgr, arg, true);

    	break;

    default:
        guimgr->hideToplevDialog(guimgr);
        break;
    }
}


static int
guimgr_process_slider(struct gui_manager_interface *guimgr, slider_t *slider, unsigned int keyin, void *arg)
{
    int ret = NO_HANDLE_CALLBACK_RETCODE;

    switch (keyin)
    {
    case KEYIN_UP_RELEASE:
    case KEYIN_UP_REPEAT:
        guimgr_move_focus(guimgr, GUI_SET_FOCUS_PREV_ITEM);
        break;

    case KEYIN_DOWN_RELEASE:
    case KEYIN_DOWN_REPEAT:
        guimgr_move_focus(guimgr, GUI_SET_FOCUS_NEXT_ITEM);
        break;

    case KEYIN_LEFT_RELEASE:
    case KEYIN_LEFT_REPEAT:
        ret = slider->execCallback(slider, SLIDER_CALLBACK_DECREASE_VALUE, arg);
        break;

    case KEYIN_RIGHT_RELEASE:
    case KEYIN_RIGHT_REPEAT:
        ret = slider->execCallback(slider, SLIDER_CALLBACK_INCREASE_VALUE, arg);
        break;

    case KEYIN_ENTER_RELEASE:
    case KEYIN_ENTER_HOLD:
        guimgr_hide_toplev_dialog(guimgr);
        break;

    default:
        TLOGMSG(0, (DBGINFOFMT "undefined key input for slider widget\n", DBGINFO));
        break;
    }

    TLOGMSG(0, ("key input = 0x%04X\n", keyin));

    return ret;
}


static void
guimgr_post_process_slider(struct gui_manager_interface *guimgr, int dlgid, int retcode, void *arg)
{
    switch (dlgid)
    {
    case GUIID_DIALOG_ADJUST:
    case GUIID_DIALOG_IRGLV:
        if (retcode == SYSCTRL_RETCODE_FAIL_IRCAM_CTRL)
        {
            guimgr->hideToplevDialog(guimgr);
            guimgr->showDialog(guimgr, GUIID_DIALOG_ERROR_IRCAM_CTRL, arg);
        }
        else if (retcode == SYSCTRL_RETCODE_FAIL_DISP_CTRL)
        {
            guimgr->hideToplevDialog(guimgr);
            guimgr->showDialog(guimgr, GUIID_DIALOG_ERROR_DISP_CTRL, arg);
        }

        break;

    default:
        break;
    }
}


static int
guimgr_process_spinbox(struct gui_manager_interface *guimgr, spinbox_t *spinbox,  unsigned int keyin, void *arg)
{
    int ret = 0;
    int val = 0;
    int dval = 0;

    switch (keyin)
    {
    case KEYIN_UP_RELEASE:
    case KEYIN_UP_REPEAT:
        spinbox->getCurrVal(spinbox, &val);
        spinbox->getDeltaValue(spinbox, &dval);
        spinbox->setCurrVal(spinbox, val + dval);
        spinbox->execCallback(spinbox, arg);
        break;

    case KEYIN_DOWN_RELEASE:
    case KEYIN_DOWN_REPEAT:
        spinbox->getCurrVal(spinbox, &val);
        spinbox->getDeltaValue(spinbox, &dval);
        spinbox->setCurrVal(spinbox, val - dval);
        spinbox->execCallback(spinbox, arg);
        break;

    case KEYIN_LEFT_RELEASE:
    case KEYIN_LEFT_REPEAT:
        guimgr_move_focus(guimgr, GUI_SET_FOCUS_PREV_ITEM);
        break;

    case KEYIN_RIGHT_RELEASE:
    case KEYIN_RIGHT_REPEAT:
        guimgr_move_focus(guimgr, GUI_SET_FOCUS_NEXT_ITEM);
        break;

    case KEYIN_ENTER_HOLD:
        guimgr_hide_toplev_dialog(guimgr);
        break;

    default:
        TLOGMSG(0, (DBGINFOFMT "undefined key input for spinbox widget\n", DBGINFO));
        break;
    }

    return ret;
}


static void
guimgr_post_process_qsel(struct gui_manager_interface *guimgr, int dlgid, int retcode, void *arg)
{

    switch (dlgid)
    {
    case GUIID_DIALOG_TAQSEL:
        if (retcode == SYSCTRL_RETCODE_XMIT_TARGET_LOCATION)
            guimgr->showDialog(guimgr, GUIID_DIALOG_XMIT_TALOC, arg);
        else if (retcode == SYSCTRL_RETCODE_SELECT_TARGET_INFO_TO_XMIT)
            guimgr->showDialog(guimgr, GUIID_DIALOG_SELECT_XMITINFO, arg);
        else
            guimgr_hide_toplev_dialog(guimgr);

        break;
    default:
        break;
    }
}


static int
guimgr_process_qsel(struct gui_manager_interface *guimgr, qsel_t *qsel, unsigned int keyin, void *arg)
{
    int ret = NO_HANDLE_CALLBACK_RETCODE;

    switch (keyin)
    {
    case KEYIN_UP_RELEASE:
    case KEYIN_UP_HOLD:
        qsel->setFocus(qsel, QSEL_UP);
        ret = qsel->execCallback(qsel, QSEL_UP, arg);
        break;

    case KEYIN_DOWN_RELEASE:
    case KEYIN_DOWN_HOLD:
        qsel->setFocus(qsel, QSEL_DOWN);
        ret = qsel->execCallback(qsel, QSEL_DOWN, arg);
        break;

    case KEYIN_LEFT_RELEASE:
    case KEYIN_LEFT_HOLD:
        qsel->setFocus(qsel, QSEL_LEFT);
        ret = qsel->execCallback(qsel, QSEL_LEFT, arg);
        break;

    case KEYIN_RIGHT_RELEASE:
    case KEYIN_RIGHT_HOLD:
        qsel->setFocus(qsel, QSEL_RIGHT);
        ret = qsel->execCallback(qsel, QSEL_RIGHT, arg);
        break;

    case KEYIN_ENTER_HOLD:
    case KEYIN_ENTER_RELEASE:
        qsel->setFocus(qsel, QSEL_CENTER);
        ret = qsel->execCallback(qsel, QSEL_CENTER, arg);
        break;

    default:
        TLOGMSG(0, (DBGINFOFMT "undefined key input for qsel  widget\n", DBGINFO));
        break;
    }

    return ret;
}


static int
guimgr_process_dialog(struct gui_manager_interface *guimgr, unsigned int keyin, void *arg)
{
    int ret = 0;
    dialog_t *dialog = NULL;
    dialog_widget_t *widget = NULL;
    struct gui_manager_attribute *this = (struct gui_manager_attribute *) guimgr;

    dialog = (dialog_t *) stack_get_top(this->window->dialogStack);

    if (dialog)
    {
        widget = dialog->getFocus(dialog);

        if (widget)
        {
            switch(widget->type)
            {
            case WIDGET_BUTTON:
                ret = guimgr_process_button(guimgr, (button_t *)widget->data, keyin, arg);

                if (ret != NO_HANDLE_CALLBACK_RETCODE)
                    guimgr_post_process_button(guimgr, dialog->getID(dialog), ret, arg);

                break;

            case WIDGET_SLIDER:
                ret = guimgr_process_slider(guimgr, (slider_t *)widget->data, keyin, arg);

                if (ret != NO_HANDLE_CALLBACK_RETCODE)
                    guimgr_post_process_slider(guimgr, dialog->getID(dialog), ret, arg);

                break;

            case WIDGET_SPINBOX:
                ret = guimgr_process_spinbox(guimgr, (spinbox_t *)widget->data, keyin, arg);
                break;

            case WIDGET_QSEL:
                ret = guimgr_process_qsel(guimgr, (qsel_t *)widget->data, keyin, arg);

                if (ret != NO_HANDLE_CALLBACK_RETCODE)
                    guimgr_post_process_qsel(guimgr, dialog->getID(dialog), ret, arg);

                break;

            default:
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "unknown widget type\n", DBGINFO));
                break;
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to get focused widget\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "failed to get top-level dialog\n", DBGINFO));
    }

    return ret;
}


static int
guimgr_hook_keyin(struct gui_manager_interface *guimgr, unsigned int keyin, void *arg)
{
    int ret = 0;
    static int wakeup = 0;
    struct main_interface *mainif = (struct main_interface *) arg;
    struct device_interface *device = mainif->device;
    struct gui_manager_attribute *this = (struct gui_manager_attribute *) guimgr;

    if (device->getPowerMode(device) != POWER_MODE_NORMAL)
    {
        switch (keyin)
        {
        case KEYIN_UP_RELEASE:
        case KEYIN_UP_HOLD_RELEASE:
        case KEYIN_DOWN_RELEASE:
        case KEYIN_DOWN_HOLD_RELEASE:
        case KEYIN_LEFT_RELEASE:
        case KEYIN_LEFT_HOLD_RELEASE:
        case KEYIN_RIGHT_RELEASE:
        case KEYIN_RIGHT_HOLD_RELEASE:
        case KEYIN_ENTER_RELEASE:
        case KEYIN_ENTER_HOLD_RELEASE:
        case KEYIN_TRIGGER_RELEASE:
        case KEYIN_TRIGGER_HOLD_RELEASE:
        case KEYIN_CHANNEL_RELEASE:
        case KEYIN_CHANNEL_HOLD_RELEASE:
            if (device->getPowerMode(device) == POWER_MODE_WAKEUP_INPROC)
            {
                device->setPowerMode(device, POWER_MODE_NORMAL);
                guimgr->unlockKeyin(guimgr);
            }
            else
                sysctrl_exit_standby_mode((void *) mainif);

            break;

        case KEYIN_UP_PUSH:
        case KEYIN_DOWN_PUSH:
        case KEYIN_LEFT_PUSH:
        case KEYIN_RIGHT_PUSH:
        case KEYIN_ENTER_PUSH:
        case KEYIN_TRIGGER_PUSH:
        case KEYIN_CHANNEL_PUSH:
            if (device->getPowerMode(device) == POWER_MODE_STANDBY)
            {
                guimgr->lockKeyin(guimgr);
                guimgr->showMenu(guimgr, (void *) mainif, false);
            }

            break;

        default:
            ret = 0;
            break;
        }
    }
    else if (this->window->mainMenu)
    {
    	if(keyin == KEYIN_ENTER_HOLD)
    		guimgr_show_menu(guimgr, arg, false);
        else
            ret = -1;
    }
    else
        ret = -1;

    return ret;
}


static int
guimgr_handle_keyin(struct gui_manager_interface *guimgr, unsigned int keyin, void *arg)
{
    int ret = 0;
    struct gui_manager_attribute *this = (struct gui_manager_attribute *) guimgr;

    if (this)
    {
        if (guimgr_hook_keyin(guimgr, keyin, arg) != 0)
        {
            switch (this->window->focus)
            {
            case GUI_FOCUS_NONE:
                guimgr_process_hotkey(guimgr, keyin, arg);
                break;

            case GUI_FOCUS_MENU:
                guimgr_process_menu(guimgr, keyin, arg);
                break;

            case GUI_FOCUS_SUBMENU:
                guimgr_process_submenu(guimgr, keyin, arg);
                break;

            case GUI_FOCUS_DIALOG:
                guimgr_process_dialog(guimgr, keyin, arg);
                break;

            default:
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "invalid gui focus\n", DBGINFO));
                break;
            }
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gui manager interface\n", DBGINFO));
    }

    return ret;
}


static int
guimgr_post_notice(struct gui_manager_interface *guimgr, int notice)
{
    int ret = 0;
    struct gui_manager_attribute *this = (struct gui_manager_attribute *) guimgr;

    if (this)
        this->window->notice = notice;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gui manager interface\n", DBGINFO));
    }

    return ret;
}


static int
guimgr_start_drawing(struct gui_manager_interface *guimgr, void *amod)
{
    int ret = 0;
    struct gui_manager_attribute *this = (struct gui_manager_attribute *) guimgr;

    if (this)
    {
        if (!this->drawing)
        {
            if (pthread_create(&(this->tidDrawing), NULL, guimgr_drawproc, amod) == 0)
                TLOGMSG(1, ("start gui drawing\n"));
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "failed to create gui drawing thread\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "already create gui drawing thread\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gui manager interface\n", DBGINFO));
    }

    return ret;
}


static int
guimgr_stop_drawing(struct gui_manager_interface *guimgr)
{
    int ret = 0;
    struct gui_manager_attribute *this = (struct gui_manager_attribute *) guimgr;

    if (this)
    {
        if (this->drawing)
        {
            this->drawing = false;
            pthread_join(this->tidDrawing, NULL);
            TLOGMSG(1, ("stop gui drawing process\n"));
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "already stop gui drawing process\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gui manager interface\n", DBGINFO));
    }

    return ret;
}


static int
guimgr_lock_keyin(struct gui_manager_interface *guimgr)
{
    int ret = 0;
    struct gui_manager_attribute *this = (struct gui_manager_attribute *) guimgr;

    if (this)
        this->keyinHandler->lock(this->keyinHandler);
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gui manager interface\n", DBGINFO));
    }

    return ret;
}


static int
guimgr_unlock_keyin(struct gui_manager_interface *guimgr)
{
    int ret = 0;
    struct gui_manager_attribute *this = (struct gui_manager_attribute *) guimgr;

    if (this)
        this->keyinHandler->unlock(this->keyinHandler);
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gui manager interface\n", DBGINFO));
    }

    return ret;
}


static int
guimgr_set_mode(struct gui_manager_interface *guimgr, int mode)
{
    int ret = 0;
    struct gui_manager_attribute *this = (struct gui_manager_attribute *) guimgr;

    if (this)
        this->mode = mode;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gui manager interface\n", DBGINFO));
    }

    return ret;
}


static int
guimgr_get_mode(struct gui_manager_interface *guimgr)
{
    int ret = 0;
    struct gui_manager_attribute *this = (struct gui_manager_attribute *) guimgr;

    if (this)
        ret = this->mode;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gui manager interface\n", DBGINFO));
    }

    return ret;
}


struct gui_manager_interface *
guimgr_create(void)
{
    int ret = 0;
    IDirectFB *dfb = NULL;
    struct gui_manager_interface *guimgr = NULL;
    struct gui_manager_attribute *this = malloc(sizeof(struct gui_manager_attribute));

    if (this == NULL)
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));
        goto quit;
    }

    memset(this, 0x00, sizeof(struct gui_manager_attribute));
    dfb = guimgr_init_directfb();

    if (dfb == NULL)
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "failed to init directfb\n", DBGINFO));
        goto quit;
    }

    this->window = gui_window_create(dfb);

    if (this->window == NULL)
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "failed to create window interface\n", DBGINFO));
        goto quit;
    }

    this->keyinHandler = gui_keyin_handler_create(dfb);

    if (this->keyinHandler == NULL)
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "failed to create keyin handler\n", DBGINFO));
        goto quit;
    }

    /* init draw handler */
    this->mode    = GUI_MODE_FACTORY;
    this->drawing = false;
    guimgr = &(this->extif);
    guimgr->setMode       = guimgr_set_mode;
    guimgr->getMode       = guimgr_get_mode;
    guimgr->startDrawing  = guimgr_start_drawing;
    guimgr->stopDrawing   = guimgr_stop_drawing;
    guimgr->handleKeyin   = guimgr_handle_keyin;
    guimgr->lockKeyin     = guimgr_lock_keyin;
    guimgr->unlockKeyin   = guimgr_unlock_keyin;
    guimgr->showDialog    = guimgr_show_dialog;
    guimgr->showMenu      = guimgr_show_menu;
    guimgr->postNotice    = guimgr_post_notice;
    guimgr->findDialog    = guimgr_find_dialog;
    guimgr->hideToplevDialog = guimgr_hide_toplev_dialog;
    guimgr->getToplevDialog  = guimgr_get_toplev_dialog;
    TLOGMSG(1, ("create gui manager interface\n"));

quit:
    if (ret != 0)
    {
        if (this)
        {
            guimgr = &(this->extif);
            guimgr_destroy(guimgr);
            guimgr = NULL;
        }

        TLOGMSG(1, (DBGINFOFMT "failed to create guimgr interface\n", DBGINFO));
    }
    else
        TLOGMSG(1, ("create gui manager interface\n"));

    return guimgr;
}


int
guimgr_destroy(struct gui_manager_interface *guimgr)
{
    int ret = 0;
    struct gui_manager_attribute *this = (struct gui_manager_attribute *) guimgr;

    if (this)
    {
        guimgr_stop_drawing(guimgr);
        gui_keyin_handler_destroy(this->keyinHandler);
        gui_window_destroy(this->window);
        free(this);
        TLOGMSG(1, ("destroy gui manager interface\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null gui manager interface\n", DBGINFO));
    }

    return ret;
}
