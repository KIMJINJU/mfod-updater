/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_dialog_creator.c
        external/internal function implementations of creating dialog
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <verinfo.h>

#include "amod.h"
#include "core/logger.h"
#include "core/sysctrl.h"
#include "etc/util.h"
#include "etc/devconf.h"
#include "gui/gui_dialog_creator.h"
#include "gui/gui_str.h"



dialog_t *
create_dialog_display_adjust(void *arg)
{
    int dispbr = 0;
    dialog_t *dialog = NULL;
    slider_t *slider = NULL;
    struct main_interface *mainif = (struct main_interface *) arg;
    struct vgss_interface *vgss = mainif->device->vgss;
    struct ircam_interface *ircam = mainif->device->ircam;
    struct display_interface *disp = mainif->device->disp;

    rect_t position[] =
    {
        {.x = 280, .y = 410, .w = 240, .h = 50 },  // 0 - adjust dialog position, dv;
        {.x = 280, .y = 376, .w = 240, .h = 152},  // 1 - adjust dialog position, ir;
        {.x = 300, .y = 420, .w = 200, .h = 30 },  // 2 - dispbr slider position, dv;
        {.x = 300, .y = 386, .w = 200, .h = 30 },  // 3 - dispbr slider position, ir;
        {.x = 300, .y = 420, .w = 200, .h = 30 },  // 4 - irbr   slider position, ir;
        {.x = 300, .y = 454, .w = 200, .h = 30 },  // 5 - ircont slider position, ir;
        {.x = 300, .y = 488, .w = 200, .h = 30 },  // 6 - iredge slider position, ir;
    };

    dialog = gui_dialog_create(GUIID_DIALOG_ADJUST);

    if (dialog)
    {
        if (vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE) == PREVIEW_STATE_STOP)
        {
            /* slider widget - display brightness */
            slider = gui_slider_create();

            if (slider)
            {
                disp->getBright(disp, DISP_BRIGHT_DV, &dispbr);
                slider->setTitle(slider, STR_DISPBR);
                slider->setMaxValue(slider, DISP_BRIGHT_DV_MIN);
                slider->setMinValue(slider, DISP_BRIGHT_DV_MAX);
                slider->setCurrentValue(slider, abs(dispbr - DISP_BRIGHT_DV_MIN));
                slider->setPosition(slider, &position[2]);
                slider->setCallback(slider, SLIDER_CALLBACK_INCREASE_VALUE, sysctrl_increase_dispbr);
                slider->setCallback(slider, SLIDER_CALLBACK_DECREASE_VALUE, sysctrl_decrease_dispbr);
                dialog->addWidget(dialog, GUIID_SLIDER_DISPBR, WIDGET_SLIDER, slider);
                dialog->setPosition(dialog, &position[0]);
                dialog->setFocus(dialog, GUIID_SLIDER_DISPBR);
            }
            else
            {
                gui_dialog_destroy(dialog);
                return NULL;
            }
        }
        else
        {
            /* slider widget - display brightness */
            slider = gui_slider_create();

            if (slider)
            {
                disp->getBright(disp, DISP_BRIGHT_IR, &dispbr);
                slider->setTitle(slider, STR_DISPBR);
                slider->setMaxValue(slider, DISP_BRIGHT_IR_MIN - DISP_BRIGHT_IR_MAX);
                slider->setMinValue(slider, 0);
                slider->setCurrentValue(slider, abs(dispbr - DISP_BRIGHT_IR_MIN));
                slider->setPosition(slider, &position[3]);
                slider->setCallback(slider, SLIDER_CALLBACK_INCREASE_VALUE, sysctrl_increase_dispbr);
                slider->setCallback(slider, SLIDER_CALLBACK_DECREASE_VALUE, sysctrl_decrease_dispbr);
                dialog->addWidget(dialog, GUIID_SLIDER_DISPBR, WIDGET_SLIDER, slider);
            }
            else
            {
                gui_dialog_destroy(dialog);
                return NULL;
            }

            /* slider widget - ir brightness */
            slider = gui_slider_create();

            if (slider)
            {
                slider->setTitle(slider, STR_IRBR);
                slider->setMaxValue(slider, 100);
                slider->setMinValue(slider, 0);
                slider->setCurrentValue(slider, ircam->getBright(ircam) - 78);
                slider->setPosition(slider, &position[4]);
                slider->setCallback(slider, SLIDER_CALLBACK_INCREASE_VALUE, sysctrl_increase_irbr);
                slider->setCallback(slider, SLIDER_CALLBACK_DECREASE_VALUE, sysctrl_decrease_irbr);
                dialog->addWidget(dialog, GUIID_SLIDER_IRBR, WIDGET_SLIDER, slider);
            }
            else
            {
                gui_dialog_destroy(dialog);
                return NULL;
            }

            /* slider widget - ir contrast */
            slider = gui_slider_create();

            if (slider)
            {
                slider->setTitle(slider, STR_IRCONT);
                slider->setMaxValue(slider, 100);
                slider->setMinValue(slider, 0);
                slider->setCurrentValue(slider, ircam->getContrast(ircam) - 78);
                slider->setPosition(slider, &position[5]);
                slider->setCallback(slider, SLIDER_CALLBACK_INCREASE_VALUE, sysctrl_increase_ircont);
                slider->setCallback(slider, SLIDER_CALLBACK_DECREASE_VALUE, sysctrl_decrease_ircont);
                dialog->addWidget(dialog, GUIID_SLIDER_IRCONT, WIDGET_SLIDER, slider);
            }
            else
            {
                gui_dialog_destroy(dialog);
                return NULL;
            }

            /* slider widget - ir edge enhancement */
            slider = gui_slider_create();

            if (slider)
            {
                slider->setTitle(slider, STR_IREDGE);
                slider->setMaxValue(slider, 5);
                slider->setMinValue(slider, 0);
                slider->setCurrentValue(slider, ircam->getEdge(ircam));
                slider->setPosition(slider, &position[6]);
                slider->setCallback(slider, SLIDER_CALLBACK_INCREASE_VALUE, sysctrl_increase_iredge);
                slider->setCallback(slider, SLIDER_CALLBACK_DECREASE_VALUE, sysctrl_decrease_iredge);
                dialog->addWidget(dialog, GUIID_SLIDER_IREDGE, WIDGET_SLIDER, slider);
                dialog->setPosition(dialog, &position[1]);
                dialog->setFocus(dialog, GUIID_SLIDER_DISPBR);
            }
            else
            {
                gui_dialog_destroy(dialog);
                return NULL;
            }
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_obmode(void *arg)
{
    caption_t *caption = NULL;
    dialog_t  *dialog  = NULL;
    button_t  *button  = NULL;
    struct main_interface *mainif = (struct main_interface *) arg;
    struct vgss_interface *vgss = mainif->device->vgss;

    rect_t position[] =
    {
        {.x = 260, .y = 370, .w = 280, .h = 100},   // 0 - obmode dialog position
        {.x = 270, .y = 400, .w = 260, .h = 20 },   // 1 - caption widget position
        {.x = 310, .y = 430, .w = 80 , .h = 30 },   // 2 - button widget postion (yes)
        {.x = 410, .y = 430, .w = 80 , .h = 30 },   // 3 - button widget postion, (no)
    };

    dialog = gui_dialog_create(GUIID_DIALOG_OBMODE);

    if (dialog)
    {
        /* yes button widget */
		button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_YES);
            button->setPosition(button, &position[2]);
            button->setCallback(button, sysctrl_toggle_obmode);
            dialog->addWidget(dialog, GUIID_BUTTON_YES, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* no button widget */
		button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_NO);
            button->setPosition(button, &position[3]);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_NO, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* change obmode caption widget */
		caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[1]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);

            if (vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE) == PREVIEW_STATE_STOP)
                caption->setString(caption, STR_CHANGE_OBMODE_IR);
            else
                caption->setString(caption, STR_CHANGE_OBMODE_DV);

            dialog->addWidget(dialog, GUIID_CAPTION_OBMODE, WIDGET_CAPTION, (void *) caption);
            dialog->setTitle(dialog, STR_OBMODE);
            dialog->setPosition(dialog, &position[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_NO);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_ireis(void *arg)
{
    int focus = 0;
    char string[64] = {0};
    caption_t *caption = NULL;
    dialog_t  *dialog = NULL;
    button_t  *button = NULL;
    struct main_interface *mainif = (struct main_interface *) arg;
    struct vgss_interface *vgss = mainif->device->vgss;

    rect_t position[] =
    {
        {.x = 260, .y = 370, .w = 280, .h = 100},   /* 0 : ireis dialog position        */
        {.x = 270, .y = 400, .w = 260, .h = 20 },   /* 1 : caption widget postion       */
        {.x = 310, .y = 430, .w = 80 , .h = 30 },   /* 2 : button widget (on)  position */
        {.x = 410, .y = 430, .w = 80 , .h = 30 },   /* 3 : button widget (off) position */
    };

	dialog = gui_dialog_create(GUIID_DIALOG_IREIS);

    if (dialog)
    {
        /* on button widget */
		button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_ON);
            button->setPosition(button, &position[2]);
            button->setCallback(button, sysctrl_enable_ireis);
            dialog->addWidget(dialog, GUIID_BUTTON_ON, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* off button widget */
		button = gui_button_create();

		if (button)
        {
            button->setTitle(button, STR_OFF);
            button->setPosition(button, &position[3]);
            button->setCallback(button, sysctrl_disable_ireis);
            dialog->addWidget(dialog, GUIID_BUTTON_OFF, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* change ireis caption widget */
		caption = gui_caption_create();

		if (caption)
        {
            memset(string, 0x00, sizeof(string));

            if (vgss->ctrlEIS(vgss, EIS_CTRL_STATE) == EIS_STATE_ENABLE)
            {
                snprintf(string, sizeof(string), "[ %s : %s ]", STR_CURRCONF, STR_ON);
                focus = GUIID_BUTTON_ON;
            }
            else
            {
                snprintf(string, sizeof(string), "[ %s : %s ]", STR_CURRCONF, STR_OFF);
                focus = GUIID_BUTTON_OFF;
            }

            caption->setPosition(caption, &position[1]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setString(caption, string);
            dialog->addWidget(dialog, GUIID_CAPTION_IREIS, WIDGET_CAPTION, (void *) caption);
            dialog->setTitle(dialog, STR_IREIS);
            dialog->setPosition(dialog, &position[0]);
            dialog->setFocus(dialog, focus);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_irclr(void *arg)
{
    int focus = 0;
    char string[64] = {0};
    caption_t *caption = NULL;
    dialog_t  *dialog = NULL;
    button_t  *button = NULL;
    struct main_interface *mainif = (struct main_interface *) arg;
    struct ircam_interface *ircam = mainif->device->ircam;

    rect_t position[] =
    {
        {.x = 235, .y = 370, .w = 330, .h = 100},   /* 0 : ireis dialog position             */
        {.x = 305, .y = 400, .w = 190, .h =  20},   /* 1 : caption widget postion            */
        {.x = 245, .y = 430, .w =  70, .h =  30},   /* 2 : button widget position (mono)     */
        {.x = 325, .y = 430, .w =  70, .h =  30},   /* 3 : button widget position (sepia)    */
        {.x = 405, .y = 430, .w =  70, .h =  30},   /* 4 : button widget position (spectrum) */
        {.x = 485, .y = 430, .w =  70, .h =  30}    /* 5 : button widget position (isothem)  */
    };

	dialog = gui_dialog_create(GUIID_DIALOG_IRCLR);

	if (dialog)
    {
        /* mono button widget */
		button = gui_button_create();

		if (button)
        {
            button->setTitle(button, STR_IRCLR_MONO);
            button->setPosition(button, &position[2]);
            button->setCallback(button, sysctrl_set_irclr_mono);
            dialog->addWidget(dialog, GUIID_BUTTON_MONO, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* sepia button widget */
		button = gui_button_create();

		if (button)
        {
            button->setTitle(button, STR_IRCLR_SEPIA);
            button->setPosition(button, &position[3]);
            button->setCallback(button, sysctrl_set_irclr_sepia);
            dialog->addWidget(dialog, GUIID_BUTTON_SEPIA, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* spectrum button widget */
		button = gui_button_create();

        if (button)
		{
            button->setTitle(button, STR_IRCLR_SPECTRUM);
            button->setPosition(button, &position[4]);
            button->setCallback(button, sysctrl_set_irclr_spectrum);
            dialog->addWidget(dialog, GUIID_BUTTON_SPECTRUM, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* isotherm button widget */
		button = gui_button_create();

		if (button)
        {
            button->setTitle(button, STR_IRCLR_ISOTHERM);
            button->setPosition(button, &position[5]);
            button->setCallback(button, sysctrl_set_irclr_isotherm);
            dialog->addWidget(dialog, GUIID_BUTTON_ISOTHERM, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* caption widget */
		caption = gui_caption_create();

		if (caption)
        {
            memset(string, 0x00, sizeof(string));

            switch(ircam->getColor(ircam))
            {
            case IRCAM_COLOR_SEPIA:
                snprintf(string, sizeof(string), "[ %s : %s ]", STR_CURRCONF, STR_IRCLR_SEPIA);
                focus = GUIID_BUTTON_SEPIA;
                break;

            case IRCAM_COLOR_SPECTRUM:
                snprintf(string, sizeof(string), "[ %s : %s ]", STR_CURRCONF, STR_IRCLR_SPECTRUM);
                focus = GUIID_BUTTON_SPECTRUM;
                break;

            case IRCAM_COLOR_ISOTHREM:
                snprintf(string, sizeof(string), "[ %s : %s ]", STR_CURRCONF, STR_IRCLR_ISOTHERM);
                focus = GUIID_BUTTON_ISOTHERM;
                break;

            default:
                snprintf(string, sizeof(string), "[ %s : %s ]", STR_CURRCONF, STR_IRCLR_MONO);
                focus = GUIID_BUTTON_MONO;
                break;
            }

            caption->setPosition(caption, &position[1]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setString(caption, string);
            dialog->addWidget(dialog, GUIID_CAPTION_IRCLR, WIDGET_CAPTION, (void *) caption);
            dialog->setTitle(dialog, STR_IRCLR);
            dialog->setPosition(dialog, &position[0]);
            dialog->setFocus(dialog, focus);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_irglv(void *arg)
{
    int gain = 0;
    int level = 0;
    dialog_t *dialog = NULL;
    slider_t *slider = NULL;
    struct main_interface *amod = (struct main_interface *) arg;
    struct ircam_interface *ircam = amod->device->ircam;

    rect_t position[] =
    {
        {.x = 280, .y = 406, .w = 240, .h = 92},  /* 0 - irglv dialog position, ir      */
        {.x = 300, .y = 416, .w = 200, .h = 30},  /* 1 - irgain slider position, ir     */
        {.x = 300, .y = 450, .w = 200, .h = 30},  /* 2 - irlevel slider position, ir    */
    };

	dialog = gui_dialog_create(GUIID_DIALOG_IRGLV);

    if (dialog)
	{
        /* slider widget - irgain */
        slider = gui_slider_create();

        if (slider)
        {
            gain = ircam->getGain(ircam);
            slider->setTitle(slider, STR_IRGLV_GAIN);
            slider->setMaxValue(slider, 100);
            slider->setMinValue(slider, 0);
            slider->setCurrentValue(slider, gain / 2);
            slider->setPosition(slider, &position[1]);
            slider->setCallback(slider, SLIDER_CALLBACK_INCREASE_VALUE, sysctrl_increase_irgain);
            slider->setCallback(slider, SLIDER_CALLBACK_DECREASE_VALUE, sysctrl_decrease_irgain);
            dialog->addWidget(dialog, GUIID_SLIDER_IRGAIN, WIDGET_SLIDER, slider);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* slider widget - irlevel */
		slider = gui_slider_create();

        if (slider)
		{
            level = ircam->getLevel(ircam);
            slider->setTitle(slider, STR_IRGLV_LEVEL);
            slider->setMaxValue(slider, 100);
            slider->setMinValue(slider, 0);
            slider->setCurrentValue(slider, level / 2);
            slider->setPosition(slider, &position[2]);
            slider->setCallback(slider, SLIDER_CALLBACK_INCREASE_VALUE, sysctrl_increase_irlevel);
            slider->setCallback(slider, SLIDER_CALLBACK_DECREASE_VALUE, sysctrl_decrease_irlevel);
            dialog->addWidget(dialog, GUIID_SLIDER_IRLEVEL, WIDGET_SLIDER, slider);
            dialog->setPosition(dialog, &position[0]);
            dialog->setFocus(dialog, GUIID_SLIDER_IRGAIN);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_enter_magcompmode(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;
    struct main_interface *mainif = (struct main_interface *)arg;
    struct dmc_interface *dmc = mainif->device->dmc;

    rect_t position[] =
    {
        {.x = 260, .y = 370, .w = 280, .h = 100},   /* 0 : magcomp dialog position                  */
        {.x = 270, .y = 400, .w = 260, .h =  20},   /* 1 : caption widget postion                   */
        {.x = 310, .y = 430, .w =  80, .h =  30},   /* 2 : button widget - enter magcomp position   */
        {.x = 410, .y = 430, .w =  80, .h =  30},   /* 3 : button widget - cancel position          */
        {.x = 360, .y = 430, .w =  80, .h =  30},   /* 4 : button widget - confirm button  */
    };

    if (dmc->getError(dmc) == DMC_ERROR_NONE)
    {
		dialog = gui_dialog_create(GUIID_DIALOG_ENTER_MAGCOMPMODE);

		if (dialog)
        {
            /* confirm button widget */
			button = gui_button_create();
	
			if (button)
            {
                button->setTitle(button, STR_CONFIRM);
                button->setPosition(button, &position[2]);
                button->setCallback(button, sysctrl_enter_magcompmode);
                dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            }
            else
            {
                gui_dialog_destroy(dialog);
                return NULL;
            }

            /* cancel  button widget */
			button = gui_button_create();

			if (button)
            {
                button->setTitle(button, STR_CANCEL);
                button->setPosition(button, &position[3]);
                button->setCallback(button, NULL);
                dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *) button);
            }
            else
            {
                gui_dialog_destroy(dialog);
                return NULL;
            }

            /* caption widget */
			caption = gui_caption_create();

			if (caption)
            {
                caption->setPosition(caption, &position[1]);
                caption->setFont(caption, CAPTION_FONT_NORMAL);
                caption->setFontHeight(caption, FONT_HEIGHT_16);
                caption->setAlign(caption, CAPTION_ALIGN_CENTER);
                caption->setString(caption, STR_ENTER_MAGCOMPMODE);
                dialog->addWidget(dialog, GUIID_CAPTION_ENTER_MAGCOMPMODE, WIDGET_CAPTION, (void *) caption);
                dialog->setTitle(dialog, STR_TARGET_COMP_DMC);
                dialog->setPosition(dialog, &position[0]);
                dialog->setFocus(dialog, GUIID_BUTTON_CANCEL);
            }
            else
            {
                gui_dialog_destroy(dialog);
                return NULL;
            }
        }
        else
            TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));
    }
    else
    {
		dialog = gui_dialog_create(GUIID_DIALOG_ERROR_DMC_OFFLINE);

        if (dialog)
		{
			/* error string */
			caption = gui_caption_create();

            if (caption)
			{
                caption->setPosition(caption, &position[1]);
                caption->setFontHeight(caption, FONT_HEIGHT_16);
                caption->setAlign(caption, CAPTION_ALIGN_CENTER);
                caption->setFont(caption, CAPTION_FONT_NORMAL);
                caption->setString(caption, STR_DMC_ERROR);
                dialog->addWidget(dialog, GUIID_CAPTION_ERROR_DMC_OFFLINE, WIDGET_CAPTION, (void *) caption);
            }
            else
            {
                gui_dialog_destroy(dialog);
                return NULL;
            }

			/* confirm button */
			button = gui_button_create();

            if (button)
			{
                button->setPosition(button, &position[4]);
                button->setTitle(button, STR_CONFIRM);
                button->setCallback(button, NULL);
                dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
                dialog->setTitle(dialog, STR_ERROR);
                dialog->setPosition(dialog, &position[0]);
                dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
            }
            else
            {
                gui_dialog_destroy(dialog);
                return NULL;
            }
        }
        else
            TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));
    }

    return dialog;
}


dialog_t *
create_dialog_exit_magcompmode(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t position[] =
    {
        {.x = 260, .y = 370, .w = 280, .h = 100},   /* 0 : magcomp dialog position                  */
        {.x = 270, .y = 400, .w = 260, .h = 20 },   /* 1 : caption widget postion                   */
        {.x = 310, .y = 430, .w = 80 , .h = 30 },   /* 2 : button widget - enter magcomp position   */
        {.x = 410, .y = 430, .w = 80 , .h = 30 },   /* 3 : button widget - cancel position          */
    };


	dialog = gui_dialog_create(GUIID_DIALOG_EXIT_MAGCOMPMODE);

    if (dialog)
	{
        /* confirm button widget */
		button = gui_button_create();

		if (button)
        {
            button->setTitle(button, STR_CONFIRM);
            button->setPosition(button, &position[2]);
            button->setCallback(button, sysctrl_exit_magcompmode);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* cancel  button widget */
		button = gui_button_create();

		if (button)
        {
            button->setTitle(button, STR_CANCEL);
            button->setPosition(button, &position[3]);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* caption widget */
		caption = gui_caption_create();

		if (caption) 
		{
            caption->setPosition(caption, &position[1]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setString(caption, STR_EXIT_MAGCOMPMODE);
            dialog->addWidget(dialog, GUIID_CAPTION_EXIT_MAGCOMPMODE, WIDGET_CAPTION, (void *) caption);
            dialog->setTitle(dialog, STR_TARGET_COMP_DMC);
            dialog->setPosition(dialog, &position[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CANCEL);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_apply_magparm(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t position[] =
    {
        {.x = 260, .y = 370, .w = 280, .h = 100},   /* 0 : dialog position                  */
        {.x = 270, .y = 400, .w = 260, .h = 20 },   /* 1 : caption widget postion           */
        {.x = 310, .y = 430, .w = 80 , .h = 30 },   /* 2 : button widget - confirm position */
        {.x = 410, .y = 430, .w = 80 , .h = 30 },   /* 3 : button widget - cancel position  */
    };

	dialog = gui_dialog_create(GUIID_DIALOG_APPLY_MAGPARM);

    if (dialog)
	{
        /* confirm button widget */
		button = gui_button_create();

        if (button)
		{
            button->setTitle(button, STR_CONFIRM);
            button->setPosition(button, &position[2]);
            button->setCallback(button, sysctrl_apply_magparm);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* cancel  button widget */
		button = gui_button_create();

        if (button)
		{
            button->setTitle(button, STR_CANCEL);
            button->setPosition(button, &position[3]);
            button->setCallback(button, sysctrl_exit_magcompmode);
            dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* caption widget */
		caption = gui_caption_create();

        if (caption)
		{
            caption->setPosition(caption, &position[1]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setString(caption, STR_APPLY_MAGPARM);
            dialog->addWidget(dialog, GUIID_CAPTION_APPLY_MAGPARM, WIDGET_CAPTION, (void *) caption);
            dialog->setTitle(dialog, STR_TARGET_COMP_DMC);
            dialog->setPosition(dialog, &position[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CANCEL);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));


    return dialog;
}


dialog_t *
create_dialog_retry_magcomp(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t position[] =
    {
        {.x = 260, .y = 370, .w = 280, .h = 100},   /* 0 : dialog position                  */
        {.x = 270, .y = 400, .w = 260, .h = 20 },   /* 1 : caption widget postion           */
        {.x = 310, .y = 430, .w = 80 , .h = 30 },   /* 2 : button widget - confirm position */
        {.x = 410, .y = 430, .w = 80 , .h = 30 },   /* 3 : button widget - cancel position  */
    };

	dialog = gui_dialog_create(GUIID_DIALOG_RETRY_MAGCOMP);

    if (dialog)
	{
        /* confirm button widget */
		button = gui_button_create();

        if (button)
		{
            button->setTitle(button, STR_CONFIRM);
            button->setPosition(button, &position[2]);
            button->setCallback(button, sysctrl_retry_magcomp);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* cancel  button widget */
		button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_CANCEL);
            button->setPosition(button, &position[3]);
            button->setCallback(button, sysctrl_exit_magcompmode);
            dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* caption widget */
		caption = gui_caption_create();

        if (caption)
		{
            caption->setPosition(caption, &position[1]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setString(caption, STR_RETRY_MAGCOMP);
            dialog->addWidget(dialog, GUIID_CAPTION_RETRY_MAGCOMP, WIDGET_CAPTION, (void *) caption);
            dialog->setTitle(dialog, STR_TARGET_COMP_DMC);
            dialog->setPosition(dialog, &position[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CANCEL);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_success_magcomp(void *arg)
{
    char string[64] = {0};
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;
    struct main_interface *mainif = (struct main_interface *) arg;
    struct dmc_interface *dmc = mainif->device->dmc;

    rect_t pos[] = 
	{
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


	dialog = gui_dialog_create(GUIID_DIALOG_MAGCOMP_OK);

    if (dialog) 
	{
		/* caption widget */
		caption = gui_caption_create();

        if (caption)
		{
            memset(string, 0x00, sizeof(string));
            snprintf(string, sizeof(string), "%s (FOM = %.1f%s)", STR_SUCCESS_MAGCOMP, dmc->getFom(dmc) / 10.0, STR_DEGREE);
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, string);
            dialog->addWidget(dialog, GUIID_CAPTION_MAGCOMP_OK, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

		/* confirm button widget */
		button = gui_button_create();

		if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_TARGET_COMP_DMC);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_magcomp_errfom(void *arg)
{
    char string[64] = {0};
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;
    struct main_interface *mainif = (struct main_interface *) arg;
    struct dmc_interface *dmc = mainif->device->dmc;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


	dialog = gui_dialog_create(GUIID_DIALOG_MAGCOMP_ERRFOM);

    if (dialog)
	{
		/* caption widget */
		caption = gui_caption_create();

        if (caption)
		{
            memset(string, 0x00, sizeof(string));
            snprintf(string, sizeof(string), "%s (FOM = %.1f%s)", STR_FAIL_MAGCOMP, dmc->getFom(dmc) / 10.0, STR_DEGREE);
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, string);
            dialog->addWidget(dialog, GUIID_CAPTION_MAGCOMP_ERRFOM, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

		/* confirm button widget */
		button = gui_button_create();

        if (button) 
		{
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_magcomp_errcalc(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


	dialog = gui_dialog_create(GUIID_DIALOG_MAGCOMP_ERRCALC);

    if (dialog)
	{
		/* caption widget */
		caption  = gui_caption_create();

        if (caption) 
		{
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_MAGCOMP_ERRCALC);
            dialog->addWidget(dialog, GUIID_CAPTION_MAGCOMP_ERRCALC, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

		/* confirm button widget */
		button = gui_button_create();

        if (button)
		{
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_magcomp_errdaq(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


	dialog = gui_dialog_create(GUIID_DIALOG_MAGCOMP_ERRDAQ);

	if (dialog)
    {
		/* caption widget */
		caption = gui_caption_create();

        if (caption)
		{
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_MAGCOMP_ERRDAQ);
            dialog->addWidget(dialog, GUIID_CAPTION_MAGCOMP_ERRDAQ, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

		/* confirm button widget */
		button = gui_button_create();

		if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_irheq(void *arg)
{
    int focus = 0;
    char string[64] = {0};
    caption_t *caption = NULL;
    dialog_t  *dialog = NULL;
    button_t  *button = NULL;
    struct main_interface *amod = (struct main_interface *) arg;
    struct ircam_interface *ircam = amod->device->ircam;

    rect_t position[] =
    {
        {.x = 260, .y = 370, .w = 280, .h = 100},   /* 0 : ireis dialog position        */
        {.x = 270, .y = 400, .w = 260, .h = 20 },   /* 1 : caption widget postion       */
        {.x = 310, .y = 430, .w = 80 , .h = 30 },   /* 2 : button widget (GHEQ)  position */
        {.x = 410, .y = 430, .w = 80 , .h = 30 },   /* 3 : button widget (LHEQ) position */
    };


	dialog = gui_dialog_create(GUIID_DIALOG_IRHEQ);

    if (dialog)
	{
        /* on button widget */
		button = gui_button_create();
		
		if (button)
        {
            button->setTitle(button, STR_IRHEQ_GHEQ);
            button->setPosition(button, &position[2]);
            button->setCallback(button, sysctrl_enable_gheq);
            dialog->addWidget(dialog, GUIID_BUTTON_GHEQ, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* off button widget */
		button = gui_button_create();

        if (button)
		{
            button->setTitle(button, STR_IRHEQ_LHEQ);
            button->setPosition(button, &position[3]);
            button->setCallback(button, sysctrl_enable_lheq);
            dialog->addWidget(dialog, GUIID_BUTTON_LHEQ, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* change ireis caption widget */
		caption = gui_caption_create();

        if (caption)
		{
            memset(string, 0x00, sizeof(string));

            if (ircam->getHistEq(ircam) == IRCAM_HISTEQ_GLOBAL)
            {
                snprintf(string, sizeof(string), "[ %s : %s ]", STR_CURRCONF, STR_IRHEQ_GHEQ);
                focus = GUIID_BUTTON_GHEQ;
            }
            else
            {
                snprintf(string, sizeof(string), "[ %s : %s ]", STR_CURRCONF, STR_IRHEQ_LHEQ);
                focus = GUIID_BUTTON_LHEQ;
            }

            caption->setPosition(caption, &position[1]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setString(caption, string);
            dialog->addWidget(dialog, GUIID_CAPTION_IRHEQ, WIDGET_CAPTION, (void *) caption);
            dialog->setTitle(dialog, STR_IRHEQ);
            dialog->setPosition(dialog, &position[0]);
            dialog->setFocus(dialog, focus);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_xmit_obloc(void *arg)
{
    caption_t *caption = NULL;
    dialog_t  *dialog = NULL;
    button_t  *button = NULL;
    struct main_interface *mainif = (struct main_interface *) arg;
    struct taqmgr_interface *taqmgr = mainif->taqmgr;

    rect_t position[] =
    {
        {.x = 260, .y = 370, .w = 280, .h = 100},   /* 0 : dialog position              */
        {.x = 270, .y = 400, .w = 260, .h =  20},   /* 1 : caption widget postion       */
        {.x = 310, .y = 430, .w =  80, .h =  30},   /* 2 : button widget position       */
        {.x = 410, .y = 430, .w =  80, .h =  30},   /* 3 : button widget position       */
        {.x = 360, .y = 430, .w =  80, .h =  30},   /* 4 : button                       */
    };


	dialog = gui_dialog_create(GUIID_DIALOG_XMIT_OBLOC);

    if (dialog)
	{
        if (taqmgr->getUserData(taqmgr) & USE_USER_INPUT_OBLOC)
        {
            /* confirm button widget */
			button = gui_button_create();

            if (button)
			{
                button->setTitle(button, STR_CONFIRM);
                button->setPosition(button, &position[2]);
                button->setCallback(button, sysctrl_xmit_observer_location);
                dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            }
            else
            {
                gui_dialog_destroy(dialog);
                return NULL;
            }

            /* cancle button widget */
			button = gui_button_create();

            if (button)
			{
                button->setTitle(button, STR_CANCEL);
                button->setPosition(button, &position[3]);
                button->setCallback(button, NULL);
                dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *) button);
            }
            else
            {
                gui_dialog_destroy(dialog);
                return NULL;
            }

            /* xmit obloc caption widget */
			caption = gui_caption_create();

            if (caption)
			{
                caption->setPosition(caption, &position[1]);
                caption->setFont(caption, CAPTION_FONT_NORMAL);
                caption->setFontHeight(caption, FONT_HEIGHT_16);
                caption->setAlign(caption, CAPTION_ALIGN_CENTER);
                caption->setString(caption, STR_XMIT_OBLOC);
                dialog->addWidget(dialog, GUIID_CAPTION_XMIT_OBLOC, WIDGET_CAPTION, (void *) caption);
                dialog->setTitle(dialog, STR_TARGET_XMIT_OBLOC);
                dialog->setPosition(dialog, &position[0]);
                dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
            }
            
			else
            {
                gui_dialog_destroy(dialog);
                return NULL;
            }
        }
        else
        {
            if (taqmgr->isOnline(taqmgr))
            {
                /* confirm button widget */
				button = gui_button_create();

                if (button) 
				{
                    button->setTitle(button, STR_CONFIRM);
                    button->setPosition(button, &position[2]);
                    button->setCallback(button, sysctrl_xmit_observer_location);
                    dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
                }
                else
                {
                    gui_dialog_destroy(dialog);
                    return NULL;
                }

                /* cancle button widget */
				button = gui_button_create();
				
				if (button)
                {
                    button->setTitle(button, STR_CANCEL);
                    button->setPosition(button, &position[3]);
                    button->setCallback(button, NULL);
                    dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *) button);
                }
                else
                {
                    gui_dialog_destroy(dialog);
                    return NULL;
                }

                /* xmit obloc caption widget */
                caption = gui_caption_create();

                if (caption)
                {
                    caption->setPosition(caption, &position[1]);
                    caption->setFont(caption, CAPTION_FONT_NORMAL);
                    caption->setFontHeight(caption, FONT_HEIGHT_16);
                    caption->setAlign(caption, CAPTION_ALIGN_CENTER);
                    caption->setString(caption, STR_XMIT_OBLOC);
                    dialog->addWidget(dialog, GUIID_CAPTION_XMIT_OBLOC, WIDGET_CAPTION, (void *) caption);
                    dialog->setTitle(dialog, STR_TARGET_XMIT_OBLOC);
					dialog->setPosition(dialog, &position[0]);
                    dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
                }
                else
                {
                    gui_dialog_destroy(dialog);
                    return NULL;
                }
            }
            else
            {
                /* confirm button widget */
                button = gui_button_create();

                if (button)
                {
                    button->setTitle(button, STR_CONFIRM);
                    button->setPosition(button, &position[4]);
                    button->setCallback(button, NULL);
                    dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
                }
                else
                {
                    gui_dialog_destroy(dialog);
                    return NULL;
                }

                /* xmit obloc caption widget */
                caption = gui_caption_create();

                if (caption)
                {
                    caption->setPosition(caption, &position[1]);
                    caption->setFont(caption, CAPTION_FONT_NORMAL);
                    caption->setFontHeight(caption, FONT_HEIGHT_16);
                    caption->setAlign(caption, CAPTION_ALIGN_CENTER);
                    caption->setString(caption, STR_CANT_XMIT_OBLOC);
                    dialog->addWidget(dialog, GUIID_CAPTION_XMIT_OBLOC, WIDGET_CAPTION, (void *) caption);
                    dialog->setTitle(dialog, STR_ERROR);
                    dialog->setPosition(dialog, &position[0]);
                    dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
                }
                else
                {
                    gui_dialog_destroy(dialog);
                    return NULL;
                }
            }
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_xmit_taloc(void *arg)
{
    caption_t *caption = NULL;
    dialog_t  *dialog = NULL;
    button_t  *button = NULL;

    rect_t position[] =
    {
        {.x = 260, .y = 370, .w = 280, .h = 100},   /* 0 : dialog position              */
        {.x = 270, .y = 400, .w = 260, .h =  20},   /* 1 : caption widget postion       */
        {.x = 310, .y = 430, .w =  80, .h =  30},   /* 2 : button widget position       */
        {.x = 410, .y = 430, .w =  80, .h =  30},   /* 3 : button widget position       */
        {.x = 360, .y = 430, .w =  80, .h =  30},   /* 4 : button                       */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_XMIT_TALOC);

    if (dialog)
    {
        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_CONFIRM);
            button->setPosition(button, &position[2]);
            button->setCallback(button, sysctrl_xmit_target_location);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* cancle button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_CANCEL);
            button->setPosition(button, &position[3]);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* xmit obloc caption widget */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[1]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setString(caption, STR_XMIT_TALOC);
            dialog->addWidget(dialog, GUIID_CAPTION_XMIT_TALOC, WIDGET_CAPTION, (void *) caption);
            dialog->setTitle(dialog, STR_XMIT_INFO);
            dialog->setPosition(dialog, &position[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_xmit_taloc_at_list(void *arg)
{
    caption_t *caption = NULL;
    dialog_t  *dialog = NULL;
    button_t  *button = NULL;

    rect_t position[] =
    {
        {.x = 260, .y = 370, .w = 280, .h = 100},   /* 0 : dialog position              */
        {.x = 270, .y = 400, .w = 260, .h =  20},   /* 1 : caption widget postion       */
        {.x = 310, .y = 430, .w =  80, .h =  30},   /* 2 : button widget position       */
        {.x = 410, .y = 430, .w =  80, .h =  30},   /* 3 : button widget position       */
        {.x = 360, .y = 430, .w =  80, .h =  30},   /* 4 : button                       */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_XMIT_TALOC_TLIST);

    if (dialog)
    {
        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_CONFIRM);
            button->setPosition(button, &position[2]);
            button->setCallback(button, sysctrl_xmit_target_location_at_list);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* cancle button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_CANCEL);
            button->setPosition(button, &position[3]);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* xmit obloc caption widget */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[1]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setString(caption, STR_XMIT_TALOC);
            dialog->addWidget(dialog, GUIID_CAPTION_XMIT_TALOC, WIDGET_CAPTION, (void *) caption);
            dialog->setTitle(dialog, STR_XMIT_INFO);
            dialog->setPosition(dialog, &position[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_select_xmitifno(void *arg)
{
    int focus = 0;
    caption_t *caption = NULL;
    dialog_t  *dialog = NULL;
    button_t  *button = NULL;
    struct main_interface *amod = (struct main_interface *) arg;

    rect_t position[] =
    {
        {.x = 270, .y = 370, .w = 260, .h = 100},   /* 0 : ireis dialog position   */
        {.x = 305, .y = 400, .w = 190, .h =  20},   /* 1 : caption widget postion  */
        {.x = 300, .y = 430, .w =  60, .h =  30},   /* 2 : button widget position  */
        {.x = 370, .y = 430, .w =  60, .h =  30},   /* 3 : button widget position  */
        {.x = 440, .y = 430, .w =  60, .h =  30},   /* 4 : button widget position  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_SELECT_XMITINFO);

    if (dialog)
    {
        /* transmit target location button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_COORDINATE);
            button->setPosition(button, &position[2]);
            button->setCallback(button, sysctrl_xmit_target_location);
            dialog->addWidget(dialog, GUIID_BUTTON_TARGET_LOCATION, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* transmit target shift button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_TARGET_SHIFT);
            button->setPosition(button, &position[3]);
            button->setCallback(button, sysctrl_xmit_target_shift);
            dialog->addWidget(dialog, GUIID_BUTTON_TARGET_SHIFT, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* cancel button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_CANCEL);
            button->setPosition(button, &position[4]);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* caption widget */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[1]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setString(caption, STR_SELECT_TARGET_INFO_TO_XMIT);
            dialog->addWidget(dialog, GUIID_CAPTION_SELECT_XMITINFO, WIDGET_CAPTION, (void *) caption);
            dialog->setTitle(dialog, STR_XMIT_INFO);
            dialog->setPosition(dialog, &position[0]);
            dialog->setFocus(dialog, focus);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_select_xmitifno_at_list(void *arg)
{
    int focus = 0;
    caption_t *caption = NULL;
    dialog_t  *dialog = NULL;
    button_t  *button = NULL;
    struct main_interface *amod = (struct main_interface *) arg;

    rect_t position[] =
    {
        {.x = 270, .y = 370, .w = 260, .h = 100},   /* 0 : ireis dialog position   */
        {.x = 305, .y = 400, .w = 190, .h =  20},   /* 1 : caption widget postion  */
        {.x = 300, .y = 430, .w =  60, .h =  30},   /* 2 : button widget position  */
        {.x = 370, .y = 430, .w =  60, .h =  30},   /* 3 : button widget position  */
        {.x = 440, .y = 430, .w =  60, .h =  30},   /* 4 : button widget position  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_SELECT_XMITINFO_TLIST);

    if (dialog)
    {
        /* transmit target location button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_COORDINATE);
            button->setPosition(button, &position[2]);
            button->setCallback(button, sysctrl_xmit_target_location_at_list);
            dialog->addWidget(dialog, GUIID_BUTTON_TARGET_LOCATION, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* transmit target shift button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_TARGET_SHIFT);
            button->setPosition(button, &position[3]);
            button->setCallback(button, sysctrl_xmit_target_shift_at_list);
            dialog->addWidget(dialog, GUIID_BUTTON_TARGET_SHIFT, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* cancel button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_CANCEL);
            button->setPosition(button, &position[4]);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* caption widget */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[1]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setString(caption, STR_SELECT_TARGET_INFO_TO_XMIT);
            dialog->addWidget(dialog, GUIID_CAPTION_SELECT_XMITINFO, WIDGET_CAPTION, (void *) caption);
            dialog->setTitle(dialog, STR_XMIT_INFO);
            dialog->setPosition(dialog, &position[0]);
            dialog->setFocus(dialog, focus);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_input_gridvar(void *arg)
{
    double val = 0.0;
    char gv[16] = {0};
    dialog_t  *dialog = NULL;
    button_t  *button = NULL;
    spinbox_t *spinbox = NULL;
    caption_t *caption = NULL;
    struct main_interface *amod = (struct main_interface *) arg;
    struct taqmgr_interface *taqmgr = amod->taqmgr;

    rect_t pos[] =
    {
        {270, 394, 260, 120},		// dialog
        {310, 474,  80,  30},		// apply button
        {410, 474,  80,  30},		// cancel button
        {350, 432,  20,  24},
        {370, 432,  20,  24},
        {390, 432,  20,  24},
        {410, 432,  20,  24},
        {430, 432,  20,  24},
    };

    struct spinbox_property
    {
        int id;
        int minval;
        int maxval;
        int cval;
    }
    property [] =
    {
        {GUIID_SPINBOX_GRIDVAR_SIGN   , 0x2B, 0x2D, 0x2B},
        {GUIID_SPINBOX_GRIDVAR_DIGIT2 , 0x30, 0x39, 0x00},
        {GUIID_SPINBOX_GRIDVAR_DIGIT1 , 0x30, 0x39, 0x00},
        {GUIID_SPINBOX_GRIDVAR_DIGIT0 , 0x30, 0x39, 0x00}
    };


    dialog = gui_dialog_create(GUIID_DIALOG_INPUT_GRIDVAR);

    if (dialog)
    {
        taqmgr->getGridVar(taqmgr, &val);
        memset(gv, 0x00, sizeof(gv));
        snprintf(gv, sizeof(gv), "%+04d", lround(DEG2MIL(val)));

        for (int i = 0; i < DIM(property); i++)
            property[i].cval = gv[i];

        /* apply button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_APPLY);
            button->setPosition(button, &pos[1]);
            button->setCallback(button, sysctrl_use_user_gridvar);
            dialog->addWidget(dialog, GUIID_BUTTON_APPLY, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* cancel button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_CANCEL);
            button->setPosition(button, &pos[2]);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        for (int i = 0; i < DIM(property); i++)
        {
            spinbox = gui_spinbox_create();

            if (spinbox)
            {
                spinbox->setDigit(spinbox, 1);
                spinbox->setMaxValue(spinbox, property[i].maxval);
                spinbox->setMinValue(spinbox, property[i].minval);
                spinbox->setCurrVal(spinbox, property[i].cval);
                spinbox->setPosition(spinbox, &pos[i + 3]);
                spinbox->setDataType(spinbox, SPINBOX_DATATYPE_CHAR);
                spinbox->setFontSize(spinbox, SPINBOX_FONT_SIZE_LARGE);

                if (property[i].id == GUIID_SPINBOX_GRIDVAR_SIGN)
                    spinbox->setDeltaValue(spinbox, 2);

                dialog->addWidget(dialog, property[i].id, WIDGET_SPINBOX, (void *) spinbox);
            }
            else
            {
                gui_dialog_destroy(dialog);
                return NULL;
            }
        }

        /* caption widget */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setPosition(caption, &pos[7]);
            caption->setString(caption, STR_MIL);
            dialog->addWidget(dialog, GUIID_CAPTION_GRIDVAR_UNIT, WIDGET_CAPTION, (void *) caption);
            dialog->setTitle(dialog, STR_TARGET_INPUT_GRIDVAR);
            dialog->setFocus(dialog, GUIID_BUTTON_CANCEL);
            dialog->setPosition(dialog, &pos[0]);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_input_obloc(void *arg)
{
    int alt = 0;
    char str[16]  = {0};
    char mgrs[32] = {0};
    dialog_t  *dialog = NULL;
    button_t  *button = NULL;
    spinbox_t *spinbox = NULL;
    caption_t *caption = NULL;
    struct main_interface *amod = (struct main_interface *) arg;
    struct taqmgr_interface *taqmgr = amod->taqmgr;

    rect_t pos[] =
    {
        {155, 394, 490, 120},		// dialog
        {310, 474,  80,  30},		// apply button
        {410, 474,  80,  30},		// cancel button
        {175, 432,  20,  24},		// spinbox - zone 4
        {195, 432,  20,  24},		// spinbox - zone 3
        {215, 432,  20,  24},		// spinbox - zone 2
        {235, 432,  20,  24},		// spinbox - zone 1
        {255, 432,  20,  24},		// spinbox - zone 0
        {285, 432,  20,  24},		// spinbox - easting 4
        {305, 432,  20,  24},		// spinbox - easting 3
        {325, 432,  20,  24},		// spinbox - easting 2
        {345, 432,  20,  24},		// spinbox - easting 1
        {365, 432,  20,  24},		// spinbox - easting 0
        {395, 432,  20,  24},		// spinbox - northing 4
        {415, 432,  20,  24},		// spinbox - northing 3
        {435, 432,  20,  24},		// spinbox - northing 2
        {455, 432,  20,  24},		// spinbox - northing 1
        {475, 432,  20,  24},		// spinbox - northing 0
        {505, 432,  20,  24},		// spinbox - altitude sign
        {525, 432,  20,  24},		// spinbox - altitude 3
        {545, 432,  20,  24},		// spinbox - altitude 2
        {565, 432,  20,  24},		// spinbox - altitude 1
        {585, 432,  20,  24},		// spinbox - altitude 0
        {605, 432,  20,  24},		// caption - altitude unit
    };

    struct spinbox_property
    {
        int id;
        int minval;
        int maxval;
        int cval;
    };

    struct spinbox_property mgrs_digit[] =
    {
        {GUIID_SPINBOX_MGRS_ZONE_DIGIT4	    , 0x30, 0x39, 0x30},
        {GUIID_SPINBOX_MGRS_ZONE_DIGIT3     , 0x30, 0x39, 0x30},
        {GUIID_SPINBOX_MGRS_ZONE_DIGIT2     , 0x41, 0x5A, 0x41},
        {GUIID_SPINBOX_MGRS_ZONE_DIGIT1     , 0x41, 0x5A, 0x42},
        {GUIID_SPINBOX_MGRS_ZONE_DIGIT0     , 0x41, 0x5A, 0x43},
        {GUIID_SPINBOX_MGRS_EASTING_DIGIT4  , 0x30, 0x39, 0x31},
        {GUIID_SPINBOX_MGRS_EASTING_DIGIT3  , 0x30, 0x39, 0x32},
        {GUIID_SPINBOX_MGRS_EASTING_DIGIT2  , 0x30, 0x39, 0x33},
        {GUIID_SPINBOX_MGRS_EASTING_DIGIT1  , 0x30, 0x39, 0x34},
        {GUIID_SPINBOX_MGRS_EASTING_DIGIT0  , 0x30, 0x39, 0x35},
        {GUIID_SPINBOX_MGRS_NORTHING_DIGIT4 , 0x30, 0x39, 0x31},
        {GUIID_SPINBOX_MGRS_NORTHING_DIGIT3 , 0x30, 0x39, 0x32},
        {GUIID_SPINBOX_MGRS_NORTHING_DIGIT2 , 0x30, 0x39, 0x33},
        {GUIID_SPINBOX_MGRS_NORTHING_DIGIT1 , 0x30, 0x39, 0x34},
        {GUIID_SPINBOX_MGRS_NORTHING_DIGIT0 , 0x30, 0x39, 0x35}
    };

    struct spinbox_property altitude_digit[] =
    {
        {GUIID_SPINBOX_ALTITUDE_SIGN   , 0x2B, 0x2D, 0x00},
        {GUIID_SPINBOX_ALTITUDE_DIGIT3 , 0x30, 0x39, 0x35},
        {GUIID_SPINBOX_ALTITUDE_DIGIT2 , 0x30, 0x39, 0x35},
        {GUIID_SPINBOX_ALTITUDE_DIGIT1 , 0x30, 0x39, 0x35},
        {GUIID_SPINBOX_ALTITUDE_DIGIT0 , 0x30, 0x39, 0x35}
    };


    dialog = gui_dialog_create(GUIID_DIALOG_INPUT_OBLOC);

    if (dialog)
    {
        memset(mgrs, 0x00, sizeof(mgrs));
        memset(str, 0x00, sizeof(str));
        taqmgr->getUserOrigin(taqmgr, mgrs, &alt);
        snprintf(str, sizeof(str), "%+05d", alt);
        TLOGMSG(1, ("user obloc = %s %+05dm\n", mgrs, alt));

        for (int i = 0; i < DIM(mgrs_digit); i++)
            mgrs_digit[i].cval = mgrs[i];

        for (int i = 0; i < DIM(altitude_digit); i++)
            altitude_digit[i].cval = str[i];

        /* apply button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_APPLY);
            button->setPosition(button, &pos[1]);
            button->setCallback(button, sysctrl_use_user_obloc);
            dialog->addWidget(dialog, GUIID_BUTTON_APPLY, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* cancel button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_CANCEL);
            button->setPosition(button, &pos[2]);
            button->setCallback(button, sysctrl_use_auto_obloc);
            dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        for (int i = 0; i < DIM(mgrs_digit); i++)
        {
            spinbox = gui_spinbox_create();

            if (spinbox)
            {
                spinbox->setMaxValue(spinbox, mgrs_digit[i].maxval);
                spinbox->setMinValue(spinbox, mgrs_digit[i].minval);
                spinbox->setCurrVal(spinbox, mgrs_digit[i].cval);
                spinbox->setPosition(spinbox, &pos[i + 3]);
                spinbox->setFontSize(spinbox, SPINBOX_FONT_SIZE_LARGE);
                spinbox->setDataType(spinbox, SPINBOX_DATATYPE_CHAR);
                dialog->addWidget(dialog, mgrs_digit[i].id, WIDGET_SPINBOX, (void *) spinbox);
            }
            else
            {
                gui_dialog_destroy(dialog);
                return NULL;
            }
        }

        for (int i = 0; i < DIM(altitude_digit); i++)
        {
            spinbox = gui_spinbox_create();

            if (spinbox)
            {
                spinbox->setMaxValue(spinbox, altitude_digit[i].maxval);
                spinbox->setMinValue(spinbox, altitude_digit[i].minval);
                spinbox->setCurrVal(spinbox, altitude_digit[i].cval);
                spinbox->setPosition(spinbox, &pos[i + 18]);
                spinbox->setFontSize(spinbox, SPINBOX_FONT_SIZE_LARGE);
                spinbox->setDataType(spinbox, SPINBOX_DATATYPE_CHAR);

                if (altitude_digit[i].id == GUIID_SPINBOX_ALTITUDE_SIGN)
                    spinbox->setDeltaValue(spinbox, 2);

                dialog->addWidget(dialog, altitude_digit[i].id, WIDGET_SPINBOX, (void *) spinbox);
            }
            else
            {
                gui_dialog_destroy(dialog);
                return NULL;
            }
        }

        /* caption widget */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setString(caption, "m");
            caption->setPosition(caption, &pos[23]);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setFontHeight(caption, FONT_HEIGHT_24);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            dialog->addWidget(dialog, GUIID_CAPTION_ALTITUDE_UNIT, WIDGET_CAPTION, (void *) caption);
            dialog->setTitle(dialog, STR_TARGET_INPUT_OBLOC);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CANCEL);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_input_range(void *arg)
{
    int range = 0;
    char str[8] = {0};
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;
    spinbox_t *spinbox = NULL;
    struct main_interface *amod = (struct main_interface *) arg;
    struct taqmgr_interface *taqmgr = amod->taqmgr;

    rect_t pos[] =
    {
        {270, 394, 260, 120},		// dialog
        {310, 474,  80,  30},		// apply button
        {410, 474,  80,  30},		// cancel button
        {350, 432,  20,  24},
        {370, 432,  20,  24},
        {390, 432,  20,  24},
        {410, 432,  20,  24},
        {434, 432,  20,  24},
    };

    struct spinbox_property
    {
        int id;
        int minval;
        int maxval;
        int cval;
    }
    property [] =
    {
        {GUIID_SPINBOX_RANGE_DIGIT3 , 0, 9, 0},
        {GUIID_SPINBOX_RANGE_DIGIT2 , 0, 9, 0},
        {GUIID_SPINBOX_RANGE_DIGIT1 , 0, 9, 0},
        {GUIID_SPINBOX_RANGE_DIGIT0 , 0, 9, 0}
    };


    dialog = gui_dialog_create(GUIID_DIALOG_INPUT_RANGE);

    if (dialog)
    {
        /* apply button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_APPLY);
            button->setPosition(button, &pos[1]);
            button->setCallback(button, sysctrl_use_user_range);
            dialog->addWidget(dialog, GUIID_BUTTON_APPLY, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* cancel button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_CANCEL);
            button->setPosition(button, &pos[2]);
            button->setCallback(button, sysctrl_use_auto_range);
            dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        taqmgr->getUserRange(taqmgr, &range);
        memset(str, 0x00, sizeof(str));
        snprintf(str, sizeof(str), "%04d", range);

        for (int i = 0; i < DIM(property); i++)
        {
            spinbox = gui_spinbox_create();

            if (spinbox)
            {
                spinbox->setDigit(spinbox, 1);
                spinbox->setMaxValue(spinbox, property[i].maxval);
                spinbox->setMinValue(spinbox, property[i].minval);
                spinbox->setCurrVal(spinbox, str[i] - 0x30);
                spinbox->setPosition(spinbox, &pos[i + 3]);
                spinbox->setFontSize(spinbox, SPINBOX_FONT_SIZE_LARGE);
                dialog->addWidget(dialog, property[i].id, WIDGET_SPINBOX, (void *) spinbox);
            }
            else
            {
                gui_dialog_destroy(dialog);
                return NULL;
            }
        }

        /* caption widget */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setString(caption, "m");
            caption->setPosition(caption, &pos[7]);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setFontHeight(caption, FONT_HEIGHT_24);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            dialog->addWidget(dialog, GUIID_CAPTION_RANGE_UNIT, WIDGET_CAPTION, (void *) caption);
            dialog->setTitle(dialog, STR_TARGET_INPUT_RANGE);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CANCEL);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_input_rgate(void *arg)
{
    dialog_t *dialog = NULL;

    return dialog;
}


dialog_t *
create_dialog_gnss_status(void *arg)
{
    dialog_t *dialog = NULL;
    svplot_t *svplot = NULL;
    button_t *button = NULL;
    struct main_interface *mainif = (struct main_interface *) arg;
    struct gnss_interface *gnss = mainif->device->gnss;
    struct taqmgr_interface *taqmgr = mainif->taqmgr;

    rect_t pos[] =
    {
        {190, 250, 420, 285},		// dialog
        {360, 500,  80,  30},		// apply button
    };


    dialog = gui_dialog_create(GUIID_DIALOG_GNSS_STATUS);

    if (dialog)
    {
        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_CONFIRM);
            button->setPosition(button, &pos[1]);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* svplot widget */
        svplot = gui_svplot_create(gnss, taqmgr);

        if (svplot)
        {
            dialog->addWidget(dialog, GUIID_SVPLOT_GNSS, WIDGET_SVPLOT, (void *) svplot);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setTitle(dialog, STR_TARGET_GNSS_STATUS);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_coordsys_selection(void *arg)
{
    int focus = 0;
    char string[64] = {0};
    caption_t *caption = NULL;
    dialog_t  *dialog = NULL;
    button_t  *button = NULL;
    struct main_interface *amod = (struct main_interface *) arg;
    struct taqmgr_interface *taqmgr = amod->taqmgr;

    rect_t position[] =
    {
        {.x = 270, .y = 370, .w = 260, .h = 100},   /* 0 : ireis dialog position             */
        {.x = 305, .y = 400, .w = 190, .h =  20},   /* 1 : caption widget postion            */
        {.x = 300, .y = 430, .w =  60, .h =  30},   /* 2 : button widget position (geodetic) */
        {.x = 370, .y = 430, .w =  60, .h =  30},   /* 3 : button widget position (utm)      */
        {.x = 440, .y = 430, .w =  60, .h =  30},   /* 4 : button widget position (mgrs)     */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_SET_COORDSYS);

    if (dialog)
    {
        /* geodetic button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_COORDSYS_GEODETIC);
            button->setPosition(button, &position[2]);
            button->setCallback(button, sysctrl_set_coordsys_geodetic);
            dialog->addWidget(dialog, GUIID_BUTTON_GEODETIC, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* utm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_COORDSYS_UTM);
            button->setPosition(button, &position[3]);
            button->setCallback(button, sysctrl_set_coordsys_utm);
            dialog->addWidget(dialog, GUIID_BUTTON_UTM, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* mgrs button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_COORDSYS_MGRS);
            button->setPosition(button, &position[4]);
            button->setCallback(button, sysctrl_set_coordsys_mgrs);
            dialog->addWidget(dialog, GUIID_BUTTON_MGRS, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* caption widget */
        caption = gui_caption_create();

        if (caption)
        {
            memset(string, 0x00, sizeof(string));

            switch(taqmgr->getCoordSystem(taqmgr))
            {
            case COORDSYS_GEODETIC:
                snprintf(string, sizeof(string), "[ %s : %s ]", STR_CURRCONF, STR_COORDSYS_GEODETIC);
                focus = GUIID_BUTTON_GEODETIC;
                break;

            case COORDSYS_UTM:
                snprintf(string, sizeof(string), "[ %s : %s ]", STR_CURRCONF, STR_COORDSYS_UTM);
                focus = GUIID_BUTTON_UTM;
                break;

            default:
                snprintf(string, sizeof(string), "[ %s : %s ]", STR_CURRCONF, STR_COORDSYS_MGRS);
                focus = GUIID_BUTTON_MGRS;
                break;
            }

            caption->setPosition(caption, &position[1]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setString(caption, string);
            dialog->addWidget(dialog, GUIID_CAPTION_COORDSYS, WIDGET_CAPTION, (void *) caption);
            dialog->setTitle(dialog, STR_COORDSYS);
            dialog->setPosition(dialog, &position[0]);
            dialog->setFocus(dialog, focus);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_timeset(void *arg)
{
    int focus = 0;
    char buf[256];
    dialog_t  *dialog = NULL;
    button_t  *button = NULL;
    caption_t *caption = NULL;
    spinbox_t *spinbox = NULL;
    struct tm *tm = NULL;
    struct timespec ts = {.tv_sec = 0, .tv_nsec = 0};
    struct main_interface *amod = (struct main_interface *) arg;
    struct device_interface *device = amod->device;

    rect_t position[] =
    {
        {.x = 245, .y = 380, .w = 310, .h = 140},   /* 0 : timest dialog position                   */
        {.x = 310, .y = 480, .w =  80, .h =  30},   /* 1 : button widget postion  (confirm)         */
        {.x = 410, .y = 480, .w =  80, .h =  30},   /* 2 : button widget position (cancel)          */
        {.x = 255, .y = 430, .w =  50, .h =  24},   /* 3 : spinbox widget position(year)            */
        {.x = 305, .y = 430, .w =  10, .h =  24},   /* 4 : caption widget position(delimiter ".")   */
        {.x = 315, .y = 430, .w =  30, .h =  24},   /* 5 : spinbox widget position(month)           */
        {.x = 345, .y = 430, .w =  10, .h =  24},   /* 6 : caption widget position(delimiter ".")   */
        {.x = 355, .y = 430, .w =  30, .h =  24},   /* 7 : spinbox widget position(day)             */
        {.x = 400, .y = 430, .w =  30, .h =  24},   /* 8 : spinbox widget position(hour)            */
        {.x = 430, .y = 430, .w =  10, .h =  24},   /* 9 : caption widget position(delimiter ":")   */
        {.x = 440, .y = 430, .w =  30, .h =  24},   /* 10 : spinbox widget position(minute)         */
        {.x = 475, .y = 430, .w =  30, .h =  24},   /* 11 : caption widget position("UTC")          */
        {.x = 505, .y = 430, .w =  40, .h =  24},   /* 12 : spinbox widget position(utc offset)     */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_TIME_CONFIG);

    if (dialog)
    {
        /* OK button */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_CONFIRM);
            button->setPosition(button, &position[1]);
            button->setCallback(button, sysctrl_set_systime);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* cancel button */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_CANCEL);
            button->setPosition(button, &position[2]);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += get_time_offset();
        tm = localtime(&ts.tv_sec);

        if (tm)
        {
            /* spinbox - year */
            spinbox = gui_spinbox_create();

            if (spinbox)
            {
                spinbox->setPosition(spinbox, &position[3]);
                spinbox->setMaxValue(spinbox, 2035);
                spinbox->setMinValue(spinbox, 2010);
                spinbox->setCurrVal(spinbox, tm->tm_year + 1900);
                spinbox->setDigit(spinbox, 4);
                spinbox->setCallback(spinbox, sysctrl_change_lastday);
                dialog->addWidget(dialog, GUIID_SPINBOX_YEAR, WIDGET_SPINBOX, (void *) spinbox);
            }
            else
            {
                gui_dialog_destroy(dialog);
                return NULL;
            }
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* caption - delimiter */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setString(caption, ".");
            caption->setPosition(caption, &position[4]);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setFontHeight(caption, FONT_HEIGHT_18);
            dialog->addWidget(dialog, GUIID_CAPTION_DELIMIT_YEAR, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* spinbox - month */
        spinbox = gui_spinbox_create();

        if (spinbox)
        {
            spinbox->setPosition(spinbox, &position[5]);
            spinbox->setMaxValue(spinbox, 12);
            spinbox->setMinValue(spinbox, 1);
            spinbox->setCurrVal(spinbox, tm->tm_mon + 1);
            spinbox->setDigit(spinbox, 2);
            spinbox->setCallback(spinbox, sysctrl_change_lastday);
            dialog->addWidget(dialog, GUIID_SPINBOX_MONTH, WIDGET_SPINBOX, (void *) spinbox);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* caption - delimiter */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setString(caption, ".");
            caption->setPosition(caption, &position[6]);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setFontHeight(caption, FONT_HEIGHT_18);
            dialog->addWidget(dialog, GUIID_CAPTION_DELIMIT_MONTH, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* spinbox - day */
        spinbox = gui_spinbox_create();

        if (spinbox)
        {
            spinbox->setPosition(spinbox, &position[7]);
            spinbox->setMaxValue(spinbox, get_lastday(tm->tm_year + 1900, tm->tm_mon + 1));
            spinbox->setMinValue(spinbox, 1);
            spinbox->setCurrVal(spinbox, tm->tm_mday);
            spinbox->setDigit(spinbox, 2);
            dialog->addWidget(dialog, GUIID_SPINBOX_DAY, WIDGET_SPINBOX, (void *) spinbox);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* spinbox - hour */
        spinbox = gui_spinbox_create();

        if (spinbox)
        {
            spinbox->setPosition(spinbox, &position[8]);
            spinbox->setMaxValue(spinbox, 23);
            spinbox->setMinValue(spinbox, 0);
            spinbox->setDigit(spinbox, 2);
            spinbox->setCurrVal(spinbox, tm->tm_hour);
            dialog->addWidget(dialog, GUIID_SPINBOX_HOUR, WIDGET_SPINBOX, (void *) spinbox);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* caption - delimiter ":" */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setString(caption, ":");
            caption->setPosition(caption, &position[9]);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setFontHeight(caption, FONT_HEIGHT_18);
            dialog->addWidget(dialog, GUIID_CAPTION_DELIMIT_TIME, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* spinbox - minute */
        spinbox = gui_spinbox_create();

        if (spinbox)
        {
            spinbox->setPosition(spinbox, &position[10]);
            spinbox->setMaxValue(spinbox, 59);
            spinbox->setMinValue(spinbox, 0);
            spinbox->setCurrVal(spinbox, tm->tm_min);
            spinbox->setDigit(spinbox, 2);
            dialog->addWidget(dialog, GUIID_SPINBOX_MINUTE, WIDGET_SPINBOX, (void *) spinbox);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* caption - "UTC" */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setString(caption, "UTC");
            caption->setPosition(caption, &position[11]);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setFontHeight(caption, FONT_HEIGHT_18);
            dialog->addWidget(dialog, GUIID_CAPTION_UTC, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* spinbox - utc offset */
        spinbox = gui_spinbox_create();

        if (spinbox)
        {
            spinbox->setPosition(spinbox, &position[12]);
            spinbox->setMaxValue(spinbox, 14);
            spinbox->setMinValue(spinbox, -12);
            spinbox->setCurrVal(spinbox, get_utc_offset());
            spinbox->setDigit(spinbox, 2);
            spinbox->setSign(spinbox, SPINBOX_SIGEND);
            dialog->addWidget(dialog, GUIID_SPINBOX_UTC_OFFSET, WIDGET_SPINBOX, (void *) spinbox);
            dialog->setTitle(dialog, STR_DEVCONF_TIME_CONFIG);
            dialog->setPosition(dialog, &position[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CANCEL);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_taqsel(void *arg)
{
    qsel_t *qsel = NULL;
    dialog_t *dialog = NULL;

    rect_t pos[] =
    {
        {290, 355, 220, 130},   /* 0 - dialog position              */
        {370, 405,  60,  30},   /* 1 - qsel item position : center  */
        {300, 405,  60,  30},   /* 2 - qsel item position : left    */
        {440, 405,  60,  30},   /* 3 - qsel item position : right   */
        {370, 365,  60,  30},   /* 4 - qsel item position : up      */
        {370, 445,  60,  30},   /* 5 - qsel item position : down    */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_TAQSEL);

    if (dialog)
    {
        qsel = gui_qsel_create();

        if (qsel)
        {
            qsel->setPosition(qsel, QSEL_CENTER, &pos[1]);
            qsel->setPosition(qsel, QSEL_LEFT, &pos[2]);
            qsel->setPosition(qsel, QSEL_RIGHT, &pos[3]);
            qsel->setPosition(qsel, QSEL_UP, &pos[4]);
            qsel->setPosition(qsel, QSEL_DOWN, &pos[5]);
            qsel->setString(qsel, QSEL_CENTER, STR_CLOSE);
            qsel->setString(qsel, QSEL_LEFT, STR_CIRCULAR_TARGET);
            qsel->setString(qsel, QSEL_RIGHT, STR_SQUARE_TARGET);
            qsel->setString(qsel, QSEL_UP, STR_FOS_CORRECTTION);
            qsel->setString(qsel, QSEL_DOWN, STR_XMIT_INFO_TAQSEL);
            qsel->setCallback(qsel, QSEL_CENTER, sysctrl_stop_target_acquisition);
            qsel->setCallback(qsel, QSEL_LEFT, sysctrl_set_taqmode_circular_target);
            qsel->setCallback(qsel, QSEL_RIGHT, sysctrl_set_taqmode_square_target);
            qsel->setCallback(qsel, QSEL_UP, sysctrl_set_taqmode_fos_correction);
            qsel->setCallback(qsel, QSEL_DOWN, sysctrl_xmit_target_information);
            qsel->setFocus(qsel, QSEL_NONE);
            dialog->addWidget(dialog, GUIID_QSEL_TAQSEL, WIDGET_QSEL, (void *) qsel);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_QSEL_TAQSEL);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_evout(void *arg)
{
    char string[64] = {0};
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;
    struct main_interface *amod = (struct main_interface *) arg;
    struct vgss_interface *vgss = amod->device->vgss;

    rect_t drawpos[] =
    {
        {260, 370, 280, 100},   /* 0 - dialog position */
        {310, 430,  80,  30},   /* 1 - button (on)  position */
        {410, 430,  80,  30},   /* 2 - button (off) position */
        {270, 400, 260,  20},   /* 3 - caption  position */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_EXTERN_VOUT);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            memset(string, 0x00, sizeof(string));

            if (vgss->ctrlExtVideo(vgss, EVOUT_CTRL_STATE) == EVOUT_STATE_ENABLE)
                snprintf(string, sizeof(string), "[ %s : %s ]", STR_CURRCONF, STR_ON);
            else
                snprintf(string, sizeof(string), "[ %s : %s ]", STR_CURRCONF, STR_OFF);

            caption->setPosition(caption, &drawpos[3]);
            caption->setString(caption, string);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            dialog->addWidget(dialog, GUIID_CAPTION_EVOUT, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* on button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &drawpos[1]);
            button->setTitle(button, STR_ON);
            button->setCallback(button, sysctrl_enable_evout);
            dialog->addWidget(dialog, GUIID_BUTTON_ON, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* off button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &drawpos[2]);
            button->setTitle(button, STR_OFF);
            button->setCallback(button, sysctrl_disable_evout);
            dialog->addWidget(dialog, GUIID_BUTTON_OFF, WIDGET_BUTTON, (void *) button);
            dialog->setPosition(dialog, &drawpos[0]);
            dialog->setTitle(dialog, STR_DEVCONF_EXTERN_VOUT);
            dialog->setFocus(dialog, GUIID_BUTTON_OFF);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_dmif_test(void *arg)
{
    caption_t *caption = NULL;
    dialog_t  *dialog = NULL;
    button_t  *button = NULL;
    struct main_interface *mainif = (struct main_interface *) arg;

    rect_t position[] =
    {
        {.x = 260, .y = 370, .w = 280, .h = 100},   /* 0 : dialog position              */
        {.x = 270, .y = 400, .w = 260, .h =  20},   /* 1 : caption widget postion       */
        {.x = 310, .y = 430, .w =  80, .h =  30},   /* 2 : button widget position       */
        {.x = 410, .y = 430, .w =  80, .h =  30},   /* 3 : button widget position       */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_DMIF_TEST);

    if (dialog)
    {
        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_CONFIRM);
            button->setPosition(button, &position[2]);
            button->setCallback(button, sysctrl_test_dmif);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* cancle button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_CANCEL);
            button->setPosition(button, &position[3]);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* xmit obloc caption widget */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[1]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setString(caption, STR_DMIF_TEST);
            dialog->addWidget(dialog, GUIID_CAPTION_DMIF_TEST, WIDGET_CAPTION, (void *) caption);
            dialog->setTitle(dialog, STR_DLGTITLE_DMIF_TEST);
            dialog->setPosition(dialog, &position[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_standby_mode(void *arg)
{
    caption_t *caption = NULL;
    dialog_t  *dialog = NULL;
    button_t  *button = NULL;
    struct main_interface *mainif = (struct main_interface *) arg;

    rect_t position[] =
    {
        {.x = 260, .y = 370, .w = 280, .h = 100},   /* 0 : dialog position              */
        {.x = 270, .y = 400, .w = 260, .h =  20},   /* 1 : caption widget postion       */
        {.x = 310, .y = 430, .w =  80, .h =  30},   /* 2 : button widget position       */
        {.x = 410, .y = 430, .w =  80, .h =  30},   /* 3 : button widget position       */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_SLEEP);

    if (dialog)
    {
        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_CONFIRM);
            button->setPosition(button, &position[2]);
            button->setCallback(button, sysctrl_enter_standby_mode);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* cancle button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_CANCEL);
            button->setPosition(button, &position[3]);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* xmit obloc caption widget */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[1]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setString(caption, STR_ENTER_STANDBY_MODE);
            dialog->addWidget(dialog, GUIID_CAPTION_ENTER_STANDBY_MODE, WIDGET_CAPTION, (void *) caption);
            dialog->setTitle(dialog, STR_DEVCONF_SLEEP);
            dialog->setPosition(dialog, &position[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CANCEL);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_termtaq(void *arg)
{
    char *str = NULL;
    caption_t *caption = NULL;
    dialog_t  *dialog = NULL;
    button_t  *button = NULL;
    struct main_interface *mainif = (struct main_interface *) arg;
    struct taqmgr_interface *taqmgr = mainif->taqmgr;

    rect_t position[] =
    {
        {.x = 260, .y = 370, .w = 280, .h = 100},   /* 0 : dialog position              */
        {.x = 270, .y = 400, .w = 260, .h =  20},   /* 1 : caption widget postion       */
        {.x = 310, .y = 430, .w =  80, .h =  30},   /* 2 : button widget position       */
        {.x = 410, .y = 430, .w =  80, .h =  30},   /* 3 : button widget position       */
    };

    switch (taqmgr->getTaqMode(taqmgr))
    {
    case TAQMODE_SQUARE_TARGET_LENGTH:
    case TAQMODE_SQUARE_TARGET_WIDTH:
        str = STR_TERM_STAQ;
        break;

    case TAQMODE_CIRCULAR_TARGET:
        str = STR_TERM_CTAQ;
        break;

    case TAQMODE_FOS_CORRECTION:
        str = STR_TERM_FOS_CORRECTION;
        break;

    default:
        return dialog;
    }


    dialog = gui_dialog_create(GUIID_DIALOG_TERMTAQ);

    if (dialog)
    {
        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_CONFIRM);
            button->setPosition(button, &position[2]);
            button->setCallback(button, sysctrl_terminate_target_acquisition);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* cancle button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_CANCEL);
            button->setPosition(button, &position[3]);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* xmit obloc caption widget */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[1]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setString(caption, str);
            dialog->addWidget(dialog, GUIID_CAPTION_TERMTAQ, WIDGET_CAPTION, (void *) caption);
            dialog->setTitle(dialog, STR_DLGTITLE_TERMTAQ);
            dialog->setPosition(dialog, &position[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CANCEL);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_user_obloc(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_USER_OBLOC);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_ENABLE_USER_OBLOC);
            dialog->addWidget(dialog, GUIID_CAPTION_USER_OBLOC, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_TARGET_INPUT_OBLOC);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_auto_obloc(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_AUTO_OBLOC);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_ENABLE_AUTO_OBLOC);
            dialog->addWidget(dialog, GUIID_CAPTION_AUTO_OBLOC, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_TARGET_INPUT_OBLOC);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_invalid_obloc(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_INVALID_OBLOC);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_INVALID_OBLOC);
            dialog->addWidget(dialog, GUIID_CAPTION_INVALID_OBLOC, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_user_range(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_USER_RANGE);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_ENABLE_USER_RANGE);
            dialog->addWidget(dialog, GUIID_CAPTION_USER_RANGE, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_TARGET_INPUT_RANGE);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_auto_range(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_AUTO_RANGE);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_ENABLE_AUTO_RANGE);
            dialog->addWidget(dialog, GUIID_CAPTION_AUTO_RANGE, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_TARGET_INPUT_RANGE);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_invalid_range(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_INVALID_RANGE);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_INVALID_RANGE);
            dialog->addWidget(dialog, GUIID_CAPTION_INVALID_RANGE, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_user_gridvar(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_USER_GRIDVAR);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_ENABLE_USER_GRIDVAR);
            dialog->addWidget(dialog, GUIID_CAPTION_USER_GRIDVAR, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_TARGET_INPUT_GRIDVAR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_auto_gridvar(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_AUTO_GRIDVAR);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_ENABLE_AUTO_GRIDVAR);
            dialog->addWidget(dialog, GUIID_CAPTION_AUTO_GRIDVAR, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_TARGET_INPUT_RANGE);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_low_voltage(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_LOW_VOLTAGE);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_CHANGE_BATTERY);
            dialog->addWidget(dialog, GUIID_CAPTION_LOW_VOLTAGE, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_LOW_VOLTAGE);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_invalid_gridvar(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_INVALID_GRIDVAR);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_INVALID_GRIDVAR);
            dialog->addWidget(dialog, GUIID_CAPTION_INVALID_RANGE, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_ircamctrl_error(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_ERROR_IRCAM_CTRL);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_ERROR_IRCAM_CTRL);
            dialog->addWidget(dialog, GUIID_CAPTION_ERROR_IRCAM_CTRL, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_ircampwr_error(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_ERROR_IRCAM_POWER);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_ERROR_IRCAM_POWER);
            dialog->addWidget(dialog, GUIID_CAPTION_ERROR_IRCAM_POWER, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_dispctrl_error(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_ERROR_DISP_CTRL);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_ERROR_DISP_CTRL);
            dialog->addWidget(dialog, GUIID_CAPTION_ERROR_DISP_CTRL, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_evoutctrl_error(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_ERROR_EVOUT_CTRL);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_ERROR_EVOUT_CTRL);
            dialog->addWidget(dialog, GUIID_CAPTION_ERROR_EVOUT_CTRL, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_shtrctrl_error(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_ERROR_SHTR_CTRL);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_ERROR_SHTR_CTRL);
            dialog->addWidget(dialog, GUIID_CAPTION_ERROR_SHTR_CTRL, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_lrfctrl_error(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_ERROR_LRF_CTRL);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_ERROR_LRF_CTRL);
            dialog->addWidget(dialog, GUIID_CAPTION_ERROR_LRF_CTRL, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_angle_error(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_ERROR_ANGLE_LIMIT);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_ERROR_ANGLE_LIMIT);
            dialog->addWidget(dialog, GUIID_CAPTION_ERROR_ANGLE_LIMIT, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_unknown_error(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_UNKNOWN_ERROR);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_UNKNOWN_ERROR);
            dialog->addWidget(dialog, GUIID_CAPTION_UNKNOWN_ERROR, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_magcomp_initerr(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_MAGCOMP_ERRINIT);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_FAIL_INIT_MAGCOMP);
            dialog->addWidget(dialog, GUIID_CAPTION_FAIL_INIT_MAGCOMP, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_error_zero_range(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_ERROR_ZERO_RANGE);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_ERROR_ZERO_RANGE);
            dialog->addWidget(dialog, GUIID_CAPTION_ERROR_ZERO_RANGE, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_error_target_shift(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_ERROR_TARGET_SHIFT);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_ERROR_TARGET_SHIFT);
            dialog->addWidget(dialog, GUIID_CAPTION_ERROR_TARGET_SHIFT, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_error_target_size(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_ERROR_TARGET_SIZE);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_ERROR_TARGET_SIZE);
            dialog->addWidget(dialog, GUIID_CAPTION_ERROR_TARGET_SIZE, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_error_geodcalc(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };

    dialog = gui_dialog_create(GUIID_DIALOG_ERROR_GEODCALC);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_ERROR_GEODCALC);
            dialog->addWidget(dialog, GUIID_CAPTION_ERROR_GEODCALC, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_error_dmc_offline(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_ERROR_DMC_OFFLINE);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_ERROR_DMC_OFFLINE);
            dialog->addWidget(dialog, GUIID_CAPTION_ERROR_DMC_OFFLINE, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    
        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_error_gnss_offline(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_ERROR_GNSS_OFFLINE);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_ERROR_GNSS_OFFLINE);
            dialog->addWidget(dialog, GUIID_CAPTION_ERROR_GNSS_OFFLINE, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_error_taq(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_ERROR_TAQ);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_ERROR_TAQ);
            dialog->addWidget(dialog, GUIID_CAPTION_ERROR_TAQ, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_success_xmit(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_SUCCESS_XMIT);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_XMIT_TARGET_INFO);
            dialog->addWidget(dialog, GUIID_CAPTION_SUCCESS_XMIT, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_XMIT_SUCCESS);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_success_dmif_test(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_SUCCESS_DMIF_TEST);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_SUCCESS_DMIF_TEST);
            dialog->addWidget(dialog, GUIID_CAPTION_SUCCESS_DMIF_TEST, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_DLGTITLE_DMIF_TEST);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_timeout(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_ERROR_TIMEOUT);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_RESPONSE_TIMEOUT);
            dialog->addWidget(dialog, GUIID_CAPTION_ERROR_TIMEOUT, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();
        
        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_nack_cantpro2(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_ERROR_NACK_CANPRO2);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_NACK_CANPRO2);
            dialog->addWidget(dialog, GUIID_CAPTION_ERROR_NACK_CANPRO2, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
        
        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_nack_cantpro15(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_ERROR_NACK_CANPRO15);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_NACK_CANPRO15);
            dialog->addWidget(dialog, GUIID_CAPTION_ERROR_NACK_CANPRO15, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
        
        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_nack_cantpro19(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_ERROR_NACK_CANPRO19);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_NACK_CANPRO19);
            dialog->addWidget(dialog, GUIID_CAPTION_ERROR_NACK_CANPRO19, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
        
        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_nack_cantpro25(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_ERROR_NACK_CANPRO25);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_NACK_CANPRO25);
            dialog->addWidget(dialog, GUIID_CAPTION_ERROR_NACK_CANPRO25, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_error_gridvar(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_ERROR_GRIDVAR);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_ERROR_GRIDVAR);
            dialog->addWidget(dialog, GUIID_CAPTION_ERROR_GRIDVAR, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
        
        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;

}


dialog_t *
create_dialog_exec_bit(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t position[] =
    {
        {.x = 260, .y = 370, .w = 280, .h = 100},   /* 0 : magcomp dialog position                  */
        {.x = 270, .y = 400, .w = 260, .h = 20 },   /* 1 : caption widget postion                   */
        {.x = 310, .y = 430, .w = 80 , .h = 30 },   /* 2 : button widget - enter magcomp position   */
        {.x = 410, .y = 430, .w = 80 , .h = 30 },   /* 3 : button widget - cancel position          */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_EXEC_BIT);

    if (dialog)
    {
        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_CONFIRM);
            button->setPosition(button, &position[2]);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* cancel  button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_CANCEL);
            button->setPosition(button, &position[3]);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* caption widget */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[1]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setString(caption, STR_EXEC_BIT);
            dialog->addWidget(dialog, GUIID_CAPTION_EXEC_BIT, WIDGET_CAPTION, (void *) caption);
            dialog->setTitle(dialog, STR_BIT_BIT);
            dialog->setPosition(dialog, &position[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CANCEL);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_bit_pass(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_BIT_PASS);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_BIT_PASS);
            dialog->addWidget(dialog, GUIID_CAPTION_BIT_PASS, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_BIT_BIT);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_bit_fail(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
    {
        {260, 370, 280, 100},   /* 0 : dialog   */
        {360, 430,  80,  30},   /* 1 : button   */
        {270, 400, 260,  20}    /* 2 : caption  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_BIT_FAIL);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_BIT_LIMIT_FUNCTION);
            dialog->addWidget(dialog, GUIID_CAPTION_BIT_FAIL, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_ERROR);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_auto_xmit(void *arg)
{
    int focus = 0;
    char string[64] = {0};
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;
    struct main_interface *mainif = (struct main_interface *) arg;
    struct msgproc_interface *msgproc = mainif->msgproc;

    rect_t position[] =
    {
        {.x = 260, .y = 370, .w = 280, .h = 100},   /* 0 : magcomp dialog position                  */
        {.x = 270, .y = 400, .w = 260, .h = 20 },   /* 1 : caption widget postion                   */
        {.x = 310, .y = 430, .w = 80 , .h = 30 },   /* 2 : button widget - enter magcomp position   */
        {.x = 410, .y = 430, .w = 80 , .h = 30 },   /* 3 : button widget - cancel position          */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_XMIT_DATA);

    if (dialog)
    {
        /* on button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_AUTO);
            button->setPosition(button, &position[2]);
            button->setCallback(button, sysctrl_enable_auto_xmit);
            dialog->addWidget(dialog, GUIID_BUTTON_ON, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* off button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_MANUAL);
            button->setPosition(button, &position[3]);
            button->setCallback(button, sysctrl_disable_auto_xmit);
            dialog->addWidget(dialog, GUIID_BUTTON_OFF, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* caption widget */
        caption = gui_caption_create();

        if (caption)
        {
            memset(string, 0x00, sizeof(string));

            if (msgproc->getAutoXmit(msgproc) == MSGPROC_AUTO_XMIT_ON)
            {
                snprintf(string, sizeof(string), "[ %s : %s ]", STR_CURRCONF, STR_AUTO);
                focus = GUIID_BUTTON_ON;
            }
            else
            {
                snprintf(string, sizeof(string), "[ %s : %s ]", STR_CURRCONF, STR_MANUAL);
                focus = GUIID_BUTTON_OFF;
            }

            caption->setPosition(caption, &position[1]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setString(caption, string);
            dialog->addWidget(dialog, GUIID_CAPTION_AUTO_XMIT_DATA, WIDGET_CAPTION, (void *) caption);
            dialog->setTitle(dialog, STR_CONF_XMIT_DATA);
            dialog->setPosition(dialog, &position[0]);
            dialog->setFocus(dialog, focus);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_target_list(void *arg)
{
    char string[64] = {0};
    double fwdaz = 0.0;
    struct tm *tm = NULL;
    grid_t *grid = NULL;
    grid_row_t *row = NULL;
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;
    taqdata_t *data = NULL;
    taqdata_manager_t *tlist = NULL;
    struct main_interface *mainif = (struct main_interface *) arg;
    struct taqmgr_interface *taqmgr = mainif->taqmgr;

    rect_t pos[] =
    {
        {.x =  80, .y = 274, .w = 640, .h = 260},   /* 0 : target list dialog position      */
        {.x = 225, .y = 494, .w =  80, .h =  30},   /* 1 : target detail button position    */
        {.x = 315, .y = 494, .w =  80, .h =  30},   /* 2 : xmit target button position      */
        {.x = 405, .y = 494, .w =  80, .h =  30},   /* 3 : delete target button position    */
        {.x = 495, .y = 494, .w =  80, .h =  30},   /* 4 : close dialog button  position    */
        {.x =  90, .y = 305, .w = 620, .h = 180},   /* 5 : grid position                    */
        {.x = 260, .y = 370, .w = 280, .h = 100},   /* 6 : target list dialog position      */
        {.x = 270, .y = 400, .w = 260, .h = 20 },   /* 7 : caption widget postion           */
        {.x = 360, .y = 430, .w = 80 , .h = 30 },   /* 8 : button widget - confirm button   */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_TARGET_LIST);

    if (dialog)
    {
        tlist = taqmgr->getTargetList(taqmgr);

        //if (tlist->getNumData(tlist) != 0)
        {
            /* create detail target info button */
            button = gui_button_create();

            if (button)
            {
                button->setTitle(button, STR_DETAIL_INFO);
                button->setPosition(button, &pos[1]);
                button->setCallback(button, sysctrl_check_target_list);
                dialog->addWidget(dialog, GUIID_BUTTON_DETAIL_INFO, WIDGET_BUTTON, (void *) button);
            }
            else
            {
                gui_dialog_destroy(dialog);
                return NULL;
            }

            /* create transmit target info button */
            button = gui_button_create();

            if (button)
            {
                button->setTitle(button, STR_XMIT_TARGET);
                button->setPosition(button, &pos[2]);
                button->setCallback(button, sysctrl_check_target_list);
                dialog->addWidget(dialog, GUIID_BUTTON_XMIT_TARGET, WIDGET_BUTTON, (void *) button);
            }
            else
            {
                gui_dialog_destroy(dialog);
                return NULL;
            }

            /* create delete target button */
            button = gui_button_create();

            if (button)
            {
                button->setTitle(button, STR_DELETE_TARGET);
                button->setPosition(button, &pos[3]);
                button->setCallback(button, sysctrl_check_target_list);
                dialog->addWidget(dialog, GUIID_BUTTON_DELETE_TARGET, WIDGET_BUTTON, (void *) button);
            }
            else
            {
                gui_dialog_destroy(dialog);
                return NULL;
            }

            /* create close dialog button */
            button = gui_button_create();
            
            if (button)
            {
                button->setTitle(button, STR_CLOSE);
                button->setPosition(button, &pos[4]);
                button->setCallback(button, NULL);
                dialog->addWidget(dialog, GUIID_BUTTON_CLOSE, WIDGET_BUTTON, (void *) button);
            }
            else
            {
                gui_dialog_destroy(dialog);
                return NULL;
            }

            /* create target list */
            grid = gui_grid_create();

            if (grid)
            {
                grid->setPosition(grid, &pos[5]);
                grid->addHeaderColumn(grid, " ", 20, 20);
                grid->addHeaderColumn(grid, "#", 38, 20);
                grid->addHeaderColumn(grid, STR_TAQTIME, 128, 20);
                grid->addHeaderColumn(grid, STR_OBSERVER_COORDINATE, 150, 20);
                grid->addHeaderColumn(grid, STR_TARGET_COORDINATE, 150, 20);
                grid->addHeaderColumn(grid, STR_DISTANCE, 67, 20);
                grid->addHeaderColumn(grid, STR_AZIMUTH, 67, 20);
                tlist->setFocus(tlist, 0);

                for (int i = 0; i < tlist->getNumData(tlist); i++)
                {
                    data = tlist->getData(tlist, i);
                    row = grid->addRow(grid);

                    if (row)
                    {
                        row->addColumn(row, " ", 20, 20);

                        memset(string, 0x00, sizeof(string));
                        snprintf(string, sizeof(string), "%02d", i + 1);
                        row->addColumn(row, string, 38, 20);

                        memset(string,  0x00, sizeof(string));
                        tm = localtime(&data->taqtime.tv_sec);

                        if (tm)
                            strftime(string, sizeof(string), "%y.%m.%d %H:%M", tm);
                        else
                            snprintf(string, sizeof(string), "2000.01.01 00:00");

                        row->addColumn(row, string, 128, 20);

                        row->addColumn(row, &(data->observer.coordstr[COORDSYS_MGRS][0]), 150, 20);
                        row->addColumn(row, &(data->target.coordstr[COORDSYS_MGRS][0]), 150, 20);

                        memset(string, 0x00, sizeof(string));
                        snprintf(string, sizeof(string), "%05dm", data->observer.hdist);
                        row->addColumn(row, string, 67, 20);

                        memset(string, 0x00,  sizeof(string));
                        fwdaz = compensate_fwdaz(data->observer.fwdaz, data->observer.magdecl, data->observer.gridvar);
                        snprintf(string, sizeof(string), "%04d", (int)lround(DEG2MIL(data->observer.fwdaz)));
                        row->addColumn(row, string, 67, 20);
                    }
                    else
                    {
                        gui_dialog_destroy(dialog);
                        return NULL;
                    }
                }

                dialog->addWidget(dialog, GUIID_GRID_TARGET_LIST, WIDGET_GRID, (void *) grid);
                dialog->setFocus(dialog, GUIID_BUTTON_CLOSE);
                dialog->setTitle(dialog, STR_TARGET_LIST);
                dialog->setPosition(dialog, &pos[0]);
            }
            else
            {
                gui_dialog_destroy(dialog);
                return NULL;
            }
        }
        /* else
        {
            caption = gui_caption_create();

            if (caption)
            {
                caption->setPosition(caption, &pos[7]);
                caption->setFontHeight(caption, FONT_HEIGHT_16);
                caption->setAlign(caption, CAPTION_ALIGN_CENTER);
                caption->setFont(caption, CAPTION_FONT_NORMAL);
                caption->setString(caption, STR_NO_TARGET);
                dialog->addWidget(dialog, GUIID_CAPTION_NO_TARGET, WIDGET_CAPTION, (void *) caption);
            }
            else
            {
                gui_dialog_destroy(dialog);
                return NULL;
            }

       
            button = gui_button_create();
            
            if (button)
            {
                button->setTitle(button, STR_CONFIRM);
                button->setPosition(button, &pos[8]);
                button->setCallback(button, NULL);
                dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
                dialog->setTitle(dialog, STR_ERROR);
                dialog->setPosition(dialog, &pos[6]);
                dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
            }
            else
            {
                gui_dialog_destroy(dialog);
                return NULL;
            }
        }
         */
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_target_detail(void *arg)
{
    double fwdaz = 0.0;
    char string[64] = {0};
    caption_t *caption = NULL;
    dialog_t  *dialog = NULL;
    button_t  *button = NULL;
    taqdata_t *data = NULL;
    struct tm *tm = NULL;
    struct main_interface *mainif = (struct main_interface *) arg;
    struct taqdata_manager_interface *tlist = mainif->taqmgr->getTargetList(mainif->taqmgr);

    rect_t position[] =
    {
        {.x =  80, .y = 274, .w = 640, .h = 260},   /*  0 : dialog position                                 */
        {.x = 360, .y = 494, .w =  80, .h =  30},   /*  1 : button widget position                          */
        {.x =  90, .y = 460, .w =  50, .h =  20},   /*  2 : caption widget postion  - taqtime tag           */
        {.x = 160, .y = 460, .w = 160, .h =  20},   /*  3 : caption widget position - taqtime value         */
        {.x =  90, .y = 300, .w =  50, .h =  20},   /*  4 : caption widget position - obloc tag             */
        {.x = 160, .y = 300, .w = 160, .h =  20},   /*  5 : caption widget position - obloc mgrs            */
        {.x = 160, .y = 320, .w = 160, .h =  20},   /*  6 : caption widget position - obloc utm             */
        {.x = 160, .y = 340, .w = 160, .h =  20},   /*  7 : caption widget position - obloc geodetic        */
        {.x =  90, .y = 360, .w =  50, .h =  20},   /*  8 : caption widget position - obalt tag             */
        {.x = 160, .y = 360, .w = 160, .h =  20},   /*  9 : caption widget position - obalt tag             */
        {.x =  90, .y = 380, .w =  50, .h =  20},   /* 10 : caption widget position - forward azimuth tag   */
        {.x = 160, .y = 380, .w = 160, .h =  20},   /* 11 : caption widget position - forward azimuth       */
        {.x =  90, .y = 400, .w =  50, .h =  20},   /* 12 : caption widget position - forward elevation tag */
        {.x = 160, .y = 400, .w = 160, .h =  20},   /* 13 : caption widget position - forward elevation     */
        {.x =  90, .y = 420, .w =  50, .h =  20},   /* 14 : caption widget position - hdist tag             */
        {.x = 160, .y = 420, .w = 160, .h =  20},   /* 15 : caption widget position - hdist value           */
        {.x =  90, .y = 440, .w =  50, .h =  20},   /* 16 : caption widget position - sdist tag             */
        {.x = 160, .y = 440, .w = 160, .h =  20},   /* 17 : caption widget position - sdist value           */
        {.x = 410, .y = 300, .w =  50, .h =  20},   /* 18 : caption widget position - tgtloc tag            */
        {.x = 480, .y = 300, .w = 160, .h =  20},   /* 19 : caption widget position - tgtloc mgrs           */
        {.x = 480, .y = 320, .w = 160, .h =  20},   /* 20 : caption widget position - tgtloc utm            */
        {.x = 480, .y = 340, .w = 160, .h =  20},   /* 21 : caption widget position - tgtloc geodetic       */
        {.x = 410, .y = 360, .w =  50, .h =  20},   /* 22 : caption widget position - tgtalt tag            */
        {.x = 480, .y = 360, .w = 160, .h =  20},   /* 23 : caption widget position - tgtalt value          */
        {.x = 410, .y = 380, .w =  50, .h =  20},   /* 24 : caption widget position - circular target tag   */
        {.x = 480, .y = 380, .w = 160, .h =  20},   /* 25 : caption widget position - circular target value */
        {.x = 410, .y = 400, .w =  50, .h =  20},   /* 26 : caption widget position - sqaure target tag     */
        {.x = 480, .y = 400, .w = 160, .h =  20},   /* 27 : caption widget position - square target value   */
        {.x = 410, .y = 420, .w =  50, .h =  20},   /* 28 : caption widget position - attitude tag          */
        {.x = 480, .y = 420, .w = 160, .h =  20},   /* 29 : caption widget position - attitude value        */
        {.x = 410, .y = 440, .w =  50, .h =  20},   /* 30 : caption widget position - adjustment tag        */
        {.x = 480, .y = 440, .w = 160, .h =  20},   /* 31 : caption widget position - adjustment value      */
        {.x = 410, .y = 460, .w =  50, .h =  20},   /* 32 : caption widget position - adjust azimuth tag    */
        {.x = 480, .y = 460, .w = 160, .h =  20},   /* 23 : caption widget position - adjust azimuth value  */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_TARGET_DETAIL);

    if (dialog)
    {
        /* confirm button */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_CONFIRM);
            button->setPosition(button, &position[1]);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        data = tlist->getFocus(tlist);

        /* observation time - tag
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[2]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_14);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, STR_TAQTIME);
            dialog->addWidget(dialog, GUIID_CAPTION_TAQTIME_TAG, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
        */

        /* observation time - value
        caption = gui_caption_create();

        if (caption)
        {
            memset(string,  0x00, sizeof(string));
            tm = localtime(&data->taqtime.tv_sec);

            if (tm)
                strftime(string, sizeof(string), "%y.%m.%d %H:%M", tm);
            else
                snprintf(string, sizeof(string), "2000.01.01 00:00");

            caption->setPosition(caption, &position[3]);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, string);
            dialog->addWidget(dialog, GUIID_CAPTION_TAQTIME_VAL, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
        */

        /* observation location - tag */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[4]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_14);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, STR_OBSERVER_COORDINATE);
            dialog->addWidget(dialog, GUIID_CAPTION_OBLOC_TAG, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* observation location - value */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[5]);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, &data->observer.coordstr[COORDSYS_MGRS][0]);
            dialog->addWidget(dialog, GUIID_CAPTION_OBLOC_MGRS, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[6]);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, &data->observer.coordstr[COORDSYS_UTM][0]);
            dialog->addWidget(dialog, GUIID_CAPTION_OBLOC_UTM, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[7]);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, &data->observer.coordstr[COORDSYS_GEODETIC][0]);
            dialog->addWidget(dialog, GUIID_CAPTION_OBLOC_GEODETIC, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* observer altitude */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[8]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_14);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, STR_OBSERVER_ALTITUDE);
            dialog->addWidget(dialog, GUIID_CAPTION_OBALT_TAG, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        caption = gui_caption_create();

        if (caption)
        {
            memset(string, 0x00, sizeof(string));
            snprintf(string, sizeof(string), "%+05dm", (int)data->observer.altitude);
            caption->setPosition(caption, &position[9]);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, string);
            dialog->addWidget(dialog, GUIID_CAPTION_OBALT_VAL, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* forward azimuth */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[10]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_14);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, STR_FWDAZ);
            dialog->addWidget(dialog, GUIID_CAPTION_FWDAZ_TAG, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        caption = gui_caption_create();

        if (caption)
        {
            fwdaz = compensate_fwdaz(data->observer.fwdaz, data->observer.magdecl, data->observer.gridvar);
            memset(string, 0x00, sizeof(string));
            snprintf(string, sizeof(string), "%04dmil / %05.1f%s", (int)lround(DEG2MIL(fwdaz)), fwdaz, STR_DEGREE);
            caption->setPosition(caption, &position[11]);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, string);
            dialog->addWidget(dialog, GUIID_CAPTION_FWDAZ_VAL, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* forward elevation */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[12]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_14);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, STR_FWDEL);
            dialog->addWidget(dialog, GUIID_CAPTION_FWDEL_TAG, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        caption = gui_caption_create();

        if (caption)
        {
            memset(string, 0x00, sizeof(string));
            snprintf(string, sizeof(string), "%+05dmil / %+05.1f%s", (int)lround(DEG2MIL(data->observer.fwdel)), data->observer.fwdel, STR_DEGREE);
            caption->setPosition(caption, &position[13]);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, string);
            dialog->addWidget(dialog, GUIID_CAPTION_FWDEL_VAL, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* horizontal distance */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[14]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_14);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, STR_HDIST);
            dialog->addWidget(dialog, GUIID_CAPTION_HDIST_TAG, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        caption = gui_caption_create();

        if (caption)
        {
            memset(string, 0x00, sizeof(string));
            snprintf(string, sizeof(string), "%04dm", data->observer.hdist);
            caption->setPosition(caption, &position[15]);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, string);
            dialog->addWidget(dialog, GUIID_CAPTION_HDIST_VAL, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* slope distance */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[16]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_14);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, STR_SDIST);
            dialog->addWidget(dialog, GUIID_CAPTION_SDIST_TAG, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        caption = gui_caption_create();

        if (caption)
        {
            memset(string, 0x00, sizeof(string));
            snprintf(string, sizeof(string), "%04dm", data->observer.sdist);
            caption->setPosition(caption, &position[17]);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, string);
            dialog->addWidget(dialog, GUIID_CAPTION_SDIST_VAL, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* target location */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[18]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_14);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, STR_TARGET_COORDINATE);
            dialog->addWidget(dialog, GUIID_CAPTION_TGTLOC_TAG, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[19]);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, &data->target.coordstr[COORDSYS_MGRS][0]);
            dialog->addWidget(dialog, GUIID_CAPTION_TGTLOC_MGRS, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[20]);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, &data->target.coordstr[COORDSYS_UTM][0]);
            dialog->addWidget(dialog, GUIID_CAPTION_TGTLOC_UTM, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[21]);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, &data->target.coordstr[COORDSYS_GEODETIC][0]);
            dialog->addWidget(dialog, GUIID_CAPTION_TGTLOC_GEODETIC, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* target altitude  */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[22]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_14);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, STR_TARGET_ALTITUDE);
            dialog->addWidget(dialog, GUIID_CAPTION_TGTALT_TAG, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        caption = gui_caption_create();

        if (caption)
        {
            memset(string, 0x00, sizeof(string));
            snprintf(string, sizeof(string), "%+05dm", (int)data->target.altitude);
            caption->setPosition(caption, &position[23]);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, string);
            dialog->addWidget(dialog, GUIID_CAPTION_TGTALT_VAL, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* target radius */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[24]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_14);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, STR_CIRCULAR_TARGET_INFO);
            dialog->addWidget(dialog, GUIID_CAPTION_CIRCULAR_TARGET_TAG, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        caption = gui_caption_create();

        if (caption)
        {
            memset(string, 0x00, sizeof(string));

            if (data->target.shape == TARGET_SHAPE_CIRCLE)
                snprintf(string, sizeof(string), "%04dm", data->target.radius);
            else
                snprintf(string, sizeof(string), "-");

            caption->setPosition(caption, &position[25]);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, string);
            dialog->addWidget(dialog, GUIID_CAPTION_CIRCULAR_TARGET_VAL, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* target length / width */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[26]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_14);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, STR_SQUARE_TARGET_INFO);
            dialog->addWidget(dialog, GUIID_CAPTION_SQUARE_TARGET_TAG, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        caption = gui_caption_create();

        if (caption)
        {
            memset(string, 0x00, sizeof(string));

            if (data->target.shape == TARGET_SHAPE_SQUARE)
                snprintf(string, sizeof(string), "%04dm / %04dm", data->target.length, data->target.width);
            else
                snprintf(string, sizeof(string), "-");

            caption->setPosition(caption, &position[27]);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, string);
            dialog->addWidget(dialog, GUIID_CAPTION_SQUARE_TARGET_VAL, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* target attitude */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[28]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_14);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, STR_ATTITUDE);
            dialog->addWidget(dialog, GUIID_CAPTION_SQUARE_TARGET_TAG, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        caption = gui_caption_create();

        if (caption)
        {
            memset(string, 0x00, sizeof(string));

            if (data->target.shape == TARGET_SHAPE_SQUARE)
                snprintf(string, sizeof(string), "%04dmil / %5.1f%s", data->target.attitude, MIL2DEG(data->target.attitude), STR_DEGREE);
            else
                snprintf(string, sizeof(string), "-");

            caption->setPosition(caption, &position[29]);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, string);
            dialog->addWidget(dialog, GUIID_CAPTION_ATTITUDE_VAL, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* target adjustment  */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[30]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_14);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, STR_ADJUSTMENT);
            dialog->addWidget(dialog, GUIID_CAPTION_ADJUSTMENT_TAG, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        caption = gui_caption_create();

        if (caption)
        {
            memset(string, 0x00, sizeof(string));
            snprintf(string, sizeof(string), "%+05dm / %+05dm", data->shift.lateral, data->shift.range);
            caption->setPosition(caption, &position[31]);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, string);
            dialog->addWidget(dialog, GUIID_CAPTION_ADJUSTMENT_VAL, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* adjusted azimuth */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[32]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_14);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, STR_ADJUSTED_AZIMUTH);
            dialog->addWidget(dialog, GUIID_CAPTION_ADJUSTED_AZIMUTH_TAG, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        caption = gui_caption_create();

        if (caption)
        {
            memset(string, 0x00, sizeof(string));
            snprintf(string, sizeof(string), "%04dmil / %05.1f%s", (int)lround(DEG2MIL(data->shift.fwdaz)), data->shift.fwdaz, STR_DEGREE);
            caption->setPosition(caption, &position[33]);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setString(caption, string);
            dialog->addWidget(dialog, GUIID_CAPTION_ADJUSTED_AZIMUTH_VAL, WIDGET_CAPTION, (void *) caption);
            dialog->setTitle(dialog, STR_TARGET_DETAIL);
            dialog->setPosition(dialog, &position[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_delete_target(void *arg)
{
    caption_t *caption = NULL;
    dialog_t  *dialog = NULL;
    button_t  *button = NULL;
    struct main_interface *mainif = (struct main_interface *) arg;

    rect_t position[] =
    {
        {.x = 260, .y = 370, .w = 280, .h = 100},   /* 0 : dialog position              */
        {.x = 270, .y = 400, .w = 260, .h =  20},   /* 1 : caption widget postion       */
        {.x = 310, .y = 430, .w =  80, .h =  30},   /* 2 : button widget position       */
        {.x = 410, .y = 430, .w =  80, .h =  30},   /* 3 : button widget position       */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_DELETE_TARGET);

    if (dialog)
    {
        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_CONFIRM);
            button->setPosition(button, &position[2]);
            button->setCallback(button, sysctrl_delete_target);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* cancle button widget */
        button = gui_button_create();
        
        if (button)
        {
            button->setTitle(button, STR_CANCEL);
            button->setPosition(button, &position[3]);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* delete target caption widget */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &position[1]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setString(caption, STR_CONFIRM_DELETE_TARGET);
            dialog->addWidget(dialog, GUIID_CAPTION_DELETE_TARGET, WIDGET_CAPTION, (void *) caption);
            dialog->setTitle(dialog, STR_DELETE_TARGET);
            dialog->setPosition(dialog, &position[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CANCEL);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_select_shutter(void *arg)
{
    int focus = 0;
    char string[64] = {0};
    caption_t *caption = NULL;
    dialog_t  *dialog = NULL;
    button_t  *button = NULL;
    struct main_interface *amod = (struct main_interface *) arg;
    struct ircam_interface *ircam = amod->device->ircam;

    rect_t position[] =
    {
        {.x = 260, .y = 370, .w = 280, .h = 100},   /* 0 : ireis dialog position                */
        {.x = 270, .y = 400, .w = 260, .h = 20 },   /* 1 : caption widget postion               */
        {.x = 310, .y = 430, .w = 80 , .h = 30 },   /* 2 : button widget (INTSHTR)  position    */
        {.x = 410, .y = 430, .w = 80 , .h = 30 },   /* 3 : button widget (EXTSHTR) position     */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_SELECT_SHUTTER);

    if (dialog)
    {
        /* internal shutter button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_INTERNAL_SHUTTER);
            button->setPosition(button, &position[2]);
            button->setCallback(button, sysctrl_select_internal_shutter);
            dialog->addWidget(dialog, GUIID_BUTTON_INTSHTR, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* external shutter button widget */
        button = gui_button_create();

        if (button)
        {
            button->setTitle(button, STR_EXTERNAL_SHUTTER);
            button->setPosition(button, &position[3]);
            button->setCallback(button, sysctrl_select_external_shutter);
            dialog->addWidget(dialog, GUIID_BUTTON_EXTSHTR, WIDGET_BUTTON, (void *) button);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* caption widget */
        caption = gui_caption_create();

        if (caption)
        {
            memset(string, 0x00, sizeof(string));

            if (ircam->getShtrMode(ircam) == IRCAM_INTERNAL_SHUTTER)
            {
                snprintf(string, sizeof(string), "[ %s : %s ]", STR_CURRCONF, STR_INTERNAL_SHUTTER);
                focus = GUIID_BUTTON_GHEQ;
            }
            else
            {
                snprintf(string, sizeof(string), "[ %s : %s ]", STR_CURRCONF, STR_EXTERNAL_SHUTTER);
                focus = GUIID_BUTTON_LHEQ;
            }

            caption->setPosition(caption, &position[1]);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setString(caption, string);
            dialog->addWidget(dialog, GUIID_CAPTION_SELECT_SHUTTER, WIDGET_CAPTION, (void *) caption);
            dialog->setTitle(dialog, STR_IRCONF_SHTRSEL);
            dialog->setPosition(dialog, &position[0]);
            dialog->setFocus(dialog, focus);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}


dialog_t *
create_dialog_cover_irlens(void *arg)
{
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t pos[] =
            {
                    {260, 370, 280, 100},   /* 0 : dialog   */
                    {360, 430,  80,  30},   /* 1 : button   */
                    {270, 400, 260,  20}    /* 2 : caption  */
            };


    dialog = gui_dialog_create(GUIID_DIALOG_COVER_IRLENS);

    if (dialog)
    {
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_COVER_IRLENS);
            dialog->addWidget(dialog, GUIID_CAPTION_COVER_IRLENS, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button widget */
        button = gui_button_create();

        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, sysctrl_update_nuc);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_IRNUC);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}



dialog_t *
create_dialog_short_distance_correct(void *arg)
{
	dialog_t *dialog = NULL;
	button_t *button = NULL;
	caption_t *caption = NULL;
	spinbox_t *spinbox = NULL;
	struct main_interface *mfod = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mfod->taqmgr;
	char dist_A[16] = {0};
	char dist_b[16] = {0};
	double A = 0.0;
	double b = 0.0;


	rect_t position[] =
	{
			{.x = 200,	.y = 150, 	.w = 400,	.h = 290}, 	/* 0: short distance correct dialog position */
			{.x = 270, 	.y = 180, 	.w = 260, 	.h = 30 },  /* 1: caption widget position */
			{.x = 300, 	.y = 380, 	.w = 80,  	.h = 30 },  /* 2: button widget(save) position */
			{.x = 450,	.y = 380,	.w = 80, 	.h = 30	},	/* 3: button widget (cancel) position */
			{.x = 300, 	.y = 240,	.w = 20,	.h = 30	},	/* 4: caption widget (A) postion */
			{.x = 300, 	.y = 310, 	.w = 20,	.h = 30	}, 	/* 5: caption widget (B) position */
			{.x = 390, 	.y = 240, 	.w = 20, 	.h = 30	},	/* 6: caption widget (.) position */
			{.x = 330, 	.y = 240,	.w = 20, 	.h = 30	},	/* 7: spinbox widget Asign position */
			{.x = 350, 	.y = 240,	.w = 20, 	.h = 30	},	/* 8: spinbox widget A0 position */
			{.x = 370, 	.y = 240,	.w = 20, 	.h = 30	},	/* 9: spinbox widget A1 position */
			{.x = 410, 	.y = 240,	.w = 20, 	.h = 30	},	/* 10: spinbox widget A2 position */
			{.x = 430, 	.y = 240,	.w = 20, 	.h = 30	},	/* 11: spinbox widget A3 position */
			{.x = 450, 	.y = 240,	.w = 20, 	.h = 30	},	/* 12: spinbox widget A4 position */
			{.x = 470, 	.y = 240,	.w = 20, 	.h = 30	},	/* 13: spinbox widget A5 position */
			{.x = 490, 	.y = 240,	.w = 20, 	.h = 30	},	/* 14: spinbox widget A6 position */
			{.x = 330, 	.y = 310,	.w = 20, 	.h = 30	},	/* 15: spinbox widget Bsign position */
			{.x = 350, 	.y = 310,	.w = 20, 	.h = 30	},	/* 16: spinbox widget B0 position */
			{.x = 370, 	.y = 310,	.w = 20, 	.h = 30	},	/* 17: spinbox widget B1 position */
			{.x = 410, 	.y = 310,	.w = 20, 	.h = 30	},	/* 18: spinbox widget B2 position */
			{.x = 430, 	.y = 310,	.w = 20, 	.h = 30	},	/* 19: spinbox widget B3 position */
			{.x = 450, 	.y = 310,	.w = 20, 	.h = 30	},	/* 20: spinbox widget B4 position */
			{.x = 470, 	.y = 310,	.w = 20, 	.h = 30	},	/* 21: spinbox widget B5 position */
			{.x = 490, 	.y = 310,	.w = 20, 	.h = 30	},	/* 22: spinbox widget B6 position */
			{.x = 390, 	.y = 310, 	.w = 20, 	.h = 30	},	/* 23: caption widget (.) position */
	};

	struct spinbox_property
	{
		int id;
		int minval;
		int maxval;
		int cval;
	};

	struct spinbox_property A_digit[] =
	{
			{GUIID_SPINBOX_DIST_SIGN_A				, 0x2B,	0x2D, 0x2B},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITA0		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITA1		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITA2		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITA3		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITA4		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITA5		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITA6		, 0x30,	0x39, 0x30}
	};

	struct spinbox_property B_digit[] =
	{
			{GUIID_SPINBOX_DIST_SIGN_B				, 0x2B,	0x2D, 0x2B},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITB0		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITB1		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITB2		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITB3		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITB4		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITB5		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITB6		, 0x30,	0x39, 0x30}
	};


	dialog = gui_dialog_create(GUIID_DIALOG_SHORTDIST);
	if(dialog)
	{
		caption = gui_caption_create();
		if(caption)
		{
			caption->setString(caption, "A :");
			caption->setPosition(caption, &position[4]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			dialog->addWidget(dialog, GUIID_CAPTION_A, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setString(caption, "b :");
			caption->setPosition(caption, &position[5]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			dialog->addWidget(dialog, GUIID_CAPTION_B, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setString(caption, ".");
			caption->setPosition(caption, &position[6]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			dialog->addWidget(dialog, GUIID_CAPTION_DECIMAL_POINT_A, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setString(caption, ".");
			caption->setPosition(caption, &position[23]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			dialog->addWidget(dialog, GUIID_CAPTION_DECIMAL_POINT_B, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		button = gui_button_create();
		if(button)
		{
			button->setTitle(button, STR_SAVE);
			button->setPosition(button, &position[2]);
			button->setCallback(button, sysctrl_save_short_dist_correct);
			dialog->addWidget(dialog, GUIID_BUTTON_SAVE, WIDGET_BUTTON, (void *)button);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		button = gui_button_create();
		if(button)
		{
			button->setTitle(button, STR_CANCEL);
			button->setPosition(button, &position[3]);
			button->setCallback(button, NULL);
			dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *)button);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		memset(dist_A, 0x00, sizeof(dist_A));
		memset(dist_b, 0x00, sizeof(dist_b));
		taqmgr->getDistOffsetSH(taqmgr, &A, &b);
		snprintf(dist_A, sizeof(dist_A), "%+08d", ROUND(A*100.0));
		snprintf(dist_b, sizeof(dist_b), "%+08d", ROUND(b*100.0));
		TLOGMSG(1, ("short  distance offset  A = %s, B = %s\n", dist_A, dist_b));

		for(int i = 0; i < DIM(A_digit); i++) A_digit[i].cval = dist_A[i];
		for(int i = 0; i < DIM(B_digit); i++) B_digit[i].cval = dist_b[i];

		for(int i = 0; i < DIM(A_digit); i++)
		{
			spinbox = gui_spinbox_create();
			if(spinbox)
			{
				spinbox->setMaxValue(spinbox, A_digit[i].maxval);
				spinbox->setMinValue(spinbox, A_digit[i].minval);
				spinbox->setCurrVal(spinbox, A_digit[i].cval);
				spinbox->setPosition(spinbox, &position[i + 7]);
				spinbox->setFontSize(spinbox, SPINBOX_FONT_SIZE_LARGE);
				spinbox->setDataType(spinbox, SPINBOX_DATATYPE_CHAR);

				if(A_digit[i].id == GUIID_SPINBOX_DIST_SIGN_A)
					spinbox->setDeltaValue(spinbox, 2);

				dialog->addWidget(dialog, A_digit[i].id, WIDGET_SPINBOX, (void *)spinbox);
			}
			else
			{
				gui_dialog_destroy(dialog);
				return NULL;
			}
		}

		for(int i = 0; i < DIM(B_digit); i++)
		{
			spinbox = gui_spinbox_create();
			if(spinbox)
			{
				spinbox->setMaxValue(spinbox, B_digit[i].maxval);
				spinbox->setMinValue(spinbox, B_digit[i].minval);
				spinbox->setCurrVal(spinbox, B_digit[i].cval);
				spinbox->setPosition(spinbox, &position[i + 15]);
				spinbox->setFontSize(spinbox, SPINBOX_FONT_SIZE_LARGE);
				spinbox->setDataType(spinbox, SPINBOX_DATATYPE_CHAR);

				if(B_digit[i].id == GUIID_SPINBOX_DIST_SIGN_B)
					spinbox->setDeltaValue(spinbox, 2);

				dialog->addWidget(dialog, B_digit[i].id, WIDGET_SPINBOX, (void *)spinbox);
			}
			else
			{
				gui_dialog_destroy(dialog);
				return NULL;
			}

		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[1]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			caption->setString(caption, STR_DISTCRR_SHORT);
			dialog->addWidget(dialog, GUIID_CAPTION_SHORTDIST, WIDGET_CAPTION, (void *)caption);
			dialog->setPosition(dialog, &position[0]);
			dialog->setFocus(dialog, GUIID_BUTTON_CANCEL);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}
	}
	else
		TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

	return dialog;
}


dialog_t *
create_dialog_middle_distance_correct(void *arg)
{
	dialog_t *dialog = NULL;
	button_t *button = NULL;
	caption_t *caption = NULL;
	spinbox_t *spinbox = NULL;
	struct main_interface *mfod = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mfod->taqmgr;
	double A = 0.0;
	double b = 0.0;
	char dist_A[16] = {0};
	char dist_b[16] = {0};

	rect_t position[] =
	{
			{.x = 200,	.y = 150, 	.w = 400,	.h = 290}, 	/* 0: short distance correct dialog position */
			{.x = 270, 	.y = 180, 	.w = 260, 	.h = 30 },  /* 1: caption widget position */
			{.x = 300, 	.y = 380, 	.w = 80,  	.h = 30 },  /* 2: button widget(save) position */
			{.x = 450,	.y = 380,	.w = 80, 	.h = 30	},	/* 3: button widget (cancel) position */
			{.x = 300, 	.y = 240,	.w = 20,	.h = 30	},	/* 4: caption widget (A) postion */
			{.x = 300, 	.y = 310, 	.w = 20,	.h = 30	}, 	/* 5: caption widget (B) position */
			{.x = 390, 	.y = 240, 	.w = 20, 	.h = 30	},	/* 6: caption widget (.) position */
			{.x = 330, 	.y = 240,	.w = 20, 	.h = 30	},	/* 7: spinbox widget Asign position */
			{.x = 350, 	.y = 240,	.w = 20, 	.h = 30	},	/* 8: spinbox widget A0 position */
			{.x = 370, 	.y = 240,	.w = 20, 	.h = 30	},	/* 9: spinbox widget A1 position */
			{.x = 410, 	.y = 240,	.w = 20, 	.h = 30	},	/* 10: spinbox widget A2 position */
			{.x = 430, 	.y = 240,	.w = 20, 	.h = 30	},	/* 11: spinbox widget A3 position */
			{.x = 450, 	.y = 240,	.w = 20, 	.h = 30	},	/* 12: spinbox widget A4 position */
			{.x = 470, 	.y = 240,	.w = 20, 	.h = 30	},	/* 13: spinbox widget A5 position */
			{.x = 490, 	.y = 240,	.w = 20, 	.h = 30	},	/* 14: spinbox widget A6 position */
			{.x = 330, 	.y = 310,	.w = 20, 	.h = 30	},	/* 15: spinbox widget Bsign position */
			{.x = 350, 	.y = 310,	.w = 20, 	.h = 30	},	/* 16: spinbox widget B0 position */
			{.x = 370, 	.y = 310,	.w = 20, 	.h = 30	},	/* 17: spinbox widget B1 position */
			{.x = 410, 	.y = 310,	.w = 20, 	.h = 30	},	/* 18: spinbox widget B2 position */
			{.x = 430, 	.y = 310,	.w = 20, 	.h = 30	},	/* 19: spinbox widget B3 position */
			{.x = 450, 	.y = 310,	.w = 20, 	.h = 30	},	/* 20: spinbox widget B4 position */
			{.x = 470, 	.y = 310,	.w = 20, 	.h = 30	},	/* 21: spinbox widget B5 position */
			{.x = 490, 	.y = 310,	.w = 20, 	.h = 30	},	/* 22: spinbox widget B6 position */
			{.x = 390, 	.y = 310, 	.w = 20, 	.h = 30	},	/* 23: caption widget (.) position */
	};

	struct spinbox_property
	{
		int id;
		int minval;
		int maxval;
		int cval;
	};

	struct spinbox_property A_digit[] =
	{
			{GUIID_SPINBOX_DIST_SIGN_A				, 0x2B,	0x2D, 0x2B},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITA0		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITA1		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITA2		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITA3		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITA4		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITA5		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITA6		, 0x30,	0x39, 0x30}
	};

	struct spinbox_property B_digit[] =
	{
			{GUIID_SPINBOX_DIST_SIGN_B				, 0x2B,	0x2D, 0x2B},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITB0		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITB1		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITB2		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITB3		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITB4		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITB5		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITB6		, 0x30,	0x39, 0x30}
	};

	dialog = gui_dialog_create(GUIID_DIALOG_MIDDIST);

	if(dialog)
	{
		caption = gui_caption_create();
		if(caption)
		{
			caption->setString(caption, "A :");
			caption->setPosition(caption, &position[4]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			dialog->addWidget(dialog, GUIID_CAPTION_A, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setString(caption, "b :");
			caption->setPosition(caption, &position[5]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			dialog->addWidget(dialog, GUIID_CAPTION_B, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setString(caption, ".");
			caption->setPosition(caption, &position[6]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			dialog->addWidget(dialog, GUIID_CAPTION_DECIMAL_POINT_A, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setString(caption, ".");
			caption->setPosition(caption, &position[23]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			dialog->addWidget(dialog, GUIID_CAPTION_DECIMAL_POINT_B, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		button = gui_button_create();
		if(button)
		{
			button->setTitle(button, STR_SAVE);
			button->setPosition(button, &position[2]);
			button->setCallback(button, sysctrl_save_middle_dist_correct);
			dialog->addWidget(dialog, GUIID_BUTTON_SAVE, WIDGET_BUTTON, (void *)button);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		button = gui_button_create();
		if(button)
		{
			button->setTitle(button, STR_CANCEL);
			button->setPosition(button, &position[3]);
			button->setCallback(button, NULL);
			dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *)button);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		memset(dist_A, 0x00, sizeof(dist_A));
		memset(dist_b, 0x00, sizeof(dist_b));
		taqmgr->getDistOffsetMD(taqmgr, &A, &b);
		snprintf(dist_A, sizeof(dist_A), "%+08d", ROUND(A*100.0));
		snprintf(dist_b, sizeof(dist_b), "%+08d", ROUND(b*100.0));
		TLOGMSG(1, ("middle distance offset A = %s, b = %s\n",dist_A, dist_b));

		for(int i = 0; i < DIM(A_digit); i++) A_digit[i].cval = dist_A[i];
		for(int i = 0; i < DIM(B_digit); i++) B_digit[i].cval = dist_b[i];

		for(int i = 0; i < DIM(A_digit); i++)
		{
			spinbox = gui_spinbox_create();
			if(spinbox)
			{
				spinbox->setMaxValue(spinbox, A_digit[i].maxval);
				spinbox->setMinValue(spinbox, A_digit[i].minval);
				spinbox->setCurrVal(spinbox, A_digit[i].cval);
				spinbox->setPosition(spinbox, &position[i + 7]);
				spinbox->setFontSize(spinbox, SPINBOX_FONT_SIZE_LARGE);
				spinbox->setDataType(spinbox, SPINBOX_DATATYPE_CHAR);

				if(A_digit[i].id == GUIID_SPINBOX_DIST_SIGN_A)
					spinbox->setDeltaValue(spinbox, 2);

				dialog->addWidget(dialog, A_digit[i].id, WIDGET_SPINBOX, (void *)spinbox);
			}
			else
			{
				gui_dialog_destroy(dialog);
				return NULL;
			}
		}

		for(int i = 0; i < DIM(B_digit); i++)
		{
			spinbox = gui_spinbox_create();
			if(spinbox)
			{
				spinbox->setMaxValue(spinbox, B_digit[i].maxval);
				spinbox->setMinValue(spinbox, B_digit[i].minval);
				spinbox->setCurrVal(spinbox, B_digit[i].cval);
				spinbox->setPosition(spinbox, &position[i + 15]);
				spinbox->setFontSize(spinbox, SPINBOX_FONT_SIZE_LARGE);
				spinbox->setDataType(spinbox, SPINBOX_DATATYPE_CHAR);

				if(B_digit[i].id == GUIID_SPINBOX_DIST_SIGN_B)
					spinbox->setDeltaValue(spinbox, 2);

				dialog->addWidget(dialog, B_digit[i].id, WIDGET_SPINBOX, (void *)spinbox);
			}
			else
			{
				gui_dialog_destroy(dialog);
				return NULL;
			}

		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[1]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			caption->setString(caption, STR_DISTCRR_MIDDLE);
			dialog->addWidget(dialog, GUIID_CAPTION_MIDDIST, WIDGET_CAPTION, (void *)caption);

			dialog->setPosition(dialog, &position[0]);
			dialog->setFocus(dialog, GUIID_BUTTON_CANCEL);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}
	}
	else
		TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

	return dialog;
}

dialog_t *
create_dialog_long_distance_correct(void *arg)
{
	dialog_t *dialog = NULL;
	button_t *button = NULL;
	caption_t *caption = NULL;
	spinbox_t *spinbox = NULL;
	struct main_interface *mfod = (struct main_interface *)arg;
	struct taqmgr_interface *taqmgr = mfod->taqmgr;
	double A = 0.0;
	double b = 0.0;
	char dist_A[16] = {0};
	char dist_b[16] = {0};

	rect_t position[] =
	{
			{.x = 200,	.y = 150, 	.w = 400,	.h = 290}, 	/* 0: short distance correct dialog position */
			{.x = 270, 	.y = 180, 	.w = 260, 	.h = 30 },  /* 1: caption widget position */
			{.x = 300, 	.y = 380, 	.w = 80,  	.h = 30 },  /* 2: button widget(save) position */
			{.x = 450,	.y = 380,	.w = 80, 	.h = 30	},	/* 3: button widget (cancel) position */
			{.x = 300, 	.y = 240,	.w = 20,	.h = 30	},	/* 4: caption widget (A) postion */
			{.x = 300, 	.y = 310, 	.w = 20,	.h = 30	}, 	/* 5: caption widget (B) position */
			{.x = 390, 	.y = 240, 	.w = 20, 	.h = 30	},	/* 6: caption widget (.) position */
			{.x = 330, 	.y = 240,	.w = 20, 	.h = 30	},	/* 7: spinbox widget Asign position */
			{.x = 350, 	.y = 240,	.w = 20, 	.h = 30	},	/* 8: spinbox widget A0 position */
			{.x = 370, 	.y = 240,	.w = 20, 	.h = 30	},	/* 9: spinbox widget A1 position */
			{.x = 410, 	.y = 240,	.w = 20, 	.h = 30	},	/* 10: spinbox widget A2 position */
			{.x = 430, 	.y = 240,	.w = 20, 	.h = 30	},	/* 11: spinbox widget A3 position */
			{.x = 450, 	.y = 240,	.w = 20, 	.h = 30	},	/* 12: spinbox widget A4 position */
			{.x = 470, 	.y = 240,	.w = 20, 	.h = 30	},	/* 13: spinbox widget A5 position */
			{.x = 490, 	.y = 240,	.w = 20, 	.h = 30	},	/* 14: spinbox widget A6 position */
			{.x = 330, 	.y = 310,	.w = 20, 	.h = 30	},	/* 15: spinbox widget Bsign position */
			{.x = 350, 	.y = 310,	.w = 20, 	.h = 30	},	/* 16: spinbox widget B0 position */
			{.x = 370, 	.y = 310,	.w = 20, 	.h = 30	},	/* 17: spinbox widget B1 position */
			{.x = 410, 	.y = 310,	.w = 20, 	.h = 30	},	/* 18: spinbox widget B2 position */
			{.x = 430, 	.y = 310,	.w = 20, 	.h = 30	},	/* 19: spinbox widget B3 position */
			{.x = 450, 	.y = 310,	.w = 20, 	.h = 30	},	/* 20: spinbox widget B4 position */
			{.x = 470, 	.y = 310,	.w = 20, 	.h = 30	},	/* 21: spinbox widget B5 position */
			{.x = 490, 	.y = 310,	.w = 20, 	.h = 30	},	/* 22: spinbox widget B6 position */
			{.x = 390, 	.y = 310, 	.w = 20, 	.h = 30	},	/* 23: caption widget (.) position */
	};

	struct spinbox_property
	{
		int id;
		int minval;
		int maxval;
		int cval;
	};

	dialog = gui_dialog_create(GUIID_DIALOG_LONGDIST);

	struct spinbox_property A_digit[] =
	{
			{GUIID_SPINBOX_DIST_SIGN_A				, 0x2B,	0x2D, 0x2B},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITA0		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITA1		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITA2		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITA3		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITA4		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITA5		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITA6		, 0x30,	0x39, 0x30}
	};

	struct spinbox_property B_digit[] =
	{
			{GUIID_SPINBOX_DIST_SIGN_B				, 0x2B,	0x2D, 0x2B},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITB0		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITB1		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITB2		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITB3		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITB4		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITB5		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_DIST_CORRECT_DIGITB6		, 0x30,	0x39, 0x30}
	};

	if(dialog)
	{
		caption = gui_caption_create();
		if(caption)
		{
			caption->setString(caption, "A :");
			caption->setPosition(caption, &position[4]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			dialog->addWidget(dialog, GUIID_CAPTION_A, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setString(caption, "b :");
			caption->setPosition(caption, &position[5]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			dialog->addWidget(dialog, GUIID_CAPTION_B, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setString(caption, ".");
			caption->setPosition(caption, &position[6]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			dialog->addWidget(dialog, GUIID_CAPTION_DECIMAL_POINT_A, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setString(caption, ".");
			caption->setPosition(caption, &position[23]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			dialog->addWidget(dialog, GUIID_CAPTION_DECIMAL_POINT_B, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		button = gui_button_create();
		if(button)
		{
			button->setTitle(button, STR_SAVE);
			button->setPosition(button, &position[2]);
			button->setCallback(button, sysctrl_save_long_dist_correct);
			dialog->addWidget(dialog, GUIID_BUTTON_SAVE, WIDGET_BUTTON, (void *)button);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		button = gui_button_create();
		if(button)
		{
			button->setTitle(button, STR_CANCEL);
			button->setPosition(button, &position[3]);
			button->setCallback(button, NULL);
			dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *)button);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		memset(dist_A, 0x00, sizeof(dist_A));
		memset(dist_b, 0x00, sizeof(dist_b));
		taqmgr->getDistOffsetLG(taqmgr, &A, &b);
		snprintf(dist_A, sizeof(dist_A), "%+08d", ROUND(A*100.0));
		snprintf(dist_b, sizeof(dist_b), "%+08d", ROUND(b*100.0));
		TLOGMSG(1, ("long distance offset A = %s, b = %s\n",dist_A, dist_b));

		for(int i = 0; i < DIM(A_digit); i++) A_digit[i].cval = dist_A[i];
		for(int i = 0; i < DIM(B_digit); i++) B_digit[i].cval = dist_b[i];

		for(int i = 0; i < DIM(A_digit); i++)
		{
			spinbox = gui_spinbox_create();
			if(spinbox)
			{
				spinbox->setMaxValue(spinbox, A_digit[i].maxval);
				spinbox->setMinValue(spinbox, A_digit[i].minval);
				spinbox->setCurrVal(spinbox, A_digit[i].cval);
				spinbox->setPosition(spinbox, &position[i + 7]);
				spinbox->setFontSize(spinbox, SPINBOX_FONT_SIZE_LARGE);
				spinbox->setDataType(spinbox, SPINBOX_DATATYPE_CHAR);

				if(A_digit[i].id == GUIID_SPINBOX_DIST_SIGN_A)
					spinbox->setDeltaValue(spinbox, 2);

				dialog->addWidget(dialog, A_digit[i].id, WIDGET_SPINBOX, (void *)spinbox);
			}
			else
			{
				gui_dialog_destroy(dialog);
				return NULL;
			}

		}

		for(int i = 0; i < DIM(B_digit); i++)
		{
			spinbox = gui_spinbox_create();
			if(spinbox)
			{
				spinbox->setMaxValue(spinbox, B_digit[i].maxval);
				spinbox->setMinValue(spinbox, B_digit[i].minval);
				spinbox->setCurrVal(spinbox, B_digit[i].cval);
				spinbox->setPosition(spinbox, &position[i + 15]);
				spinbox->setFontSize(spinbox, SPINBOX_FONT_SIZE_LARGE);
				spinbox->setDataType(spinbox, SPINBOX_DATATYPE_CHAR);

				if(B_digit[i].id == GUIID_SPINBOX_DIST_SIGN_B)
					spinbox->setDeltaValue(spinbox, 2);

				dialog->addWidget(dialog, B_digit[i].id, WIDGET_SPINBOX, (void *)spinbox);
			}
			else
			{
				gui_dialog_destroy(dialog);
				return NULL;
			}

		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[1]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			caption->setString(caption, STR_DISTCRR_LONG);
			dialog->addWidget(dialog, GUIID_CAPTION_LONGDIST, WIDGET_CAPTION, (void *)caption);

			dialog->setPosition(dialog, &position[0]);
			dialog->setFocus(dialog, GUIID_BUTTON_CANCEL);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}
	}
	else
		TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

	return dialog;
}

dialog_t *
create_dialog_distance_correct_check(void *arg)
{
	dialog_t *dialog = NULL;
	button_t *button = NULL;
	caption_t *caption = NULL;
	struct main_interface *mfod = (struct main_interface *) arg;
	struct taqmgr_interface  *taqmgr = mfod->taqmgr;
	char str_short[50] = {0};
	char str_middle[50] = {0};
	char str_long[50] = {0};
	double a = 0.0;
	double b = 0.0;

	rect_t position[] =
	{
			{.x = 60,	.y = 150,	.w = 680,	.h = 384},	/* 0: short distance correct dialog position */
			{.x = 270,	.y = 180,	.w = 260,	.h = 30},  	/* 1: caption widget position */
			{.x = 375, 	.y = 484, 	.w = 80, 	.h = 30}, 	/* 2: button widget (confirm) position */
			{.x = 100,	.y = 250,	.w = 220,	.h = 30},	/* 3: caption widget (short distance) posion*/
			{.x = 100,	.y = 330, 	.w = 220, 	.h = 30},	/* 4: caption widget (middele distance) position */
			{.x = 100, 	.y = 410,	.w = 220,	.h = 30},	/* 5: caption widget (long distance) posiotn */
			{.x = 320,	.y = 250,	.w = 100, 	.h = 30},	/* 6: caption widget (short a) position */
			{.x = 320,	.y = 330, 	.w = 400, 	.h = 30},	/* 7: caption widget (middle a) position */
			{.x = 320,	.y = 410,	.w = 400, 	.h = 30},	/* 8: caption widget (long a) position */
			{.x = 500, 	.y = 250,	.w = 100, 	.h = 30},	/* 9: caption widget (short b) position */
			{.x = 500, 	.y = 330,	.w = 100,	.h = 30},   /*10: caption widget (middle b) position */
			{.x = 500, 	.y = 410,	.w = 100, 	.h = 30 }	/*11: caption widget (long b) position */
	};

	dialog = gui_dialog_create(GUIID_DIALOG_DISTCHECK);

	if(dialog)
	{
		button = gui_button_create();
		if(button)
		{
			button->setTitle(button, STR_CONFIRM);
			button->setPosition(button, &position[2]);
			button->setCallback(button, NULL);
			dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *)button);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[3]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			caption->setString(caption, STR_DISTCRR_SHORT);
			dialog->addWidget(dialog, GUIID_CAPTION_SHORTDIST, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		memset(str_short, 0x00, sizeof(str_short));
		taqmgr->getDistOffsetSH(taqmgr, &a, &b);
		snprintf(str_short, sizeof(str_short), "A : %+.5lf", a);
		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[6]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_LEFT);
			caption->setString(caption, str_short);
			dialog->addWidget(dialog, GUIID_CAPTION_SHORTDIST_VAL_A, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		snprintf(str_short, sizeof(str_short), "b : %+.5lf", b);
		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[9]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_LEFT);
			caption->setString(caption, str_short);
			dialog->addWidget(dialog, GUIID_CAPTION_SHORTDIST_VAL_B, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}


		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[4]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			caption->setString(caption, STR_DISTCRR_MIDDLE);
			dialog->addWidget(dialog, GUIID_CAPTION_MIDDIST, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		memset(str_middle, 0x00, sizeof(str_middle));
		taqmgr->getDistOffsetMD(taqmgr, &a, &b);
		snprintf(str_middle, sizeof(str_middle), "A : %+.5lf ", a);
		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[7]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_LEFT);
			caption->setString(caption, str_middle);
			dialog->addWidget(dialog, GUIID_CAPTION_MIDDIST_VAL_A, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		snprintf(str_middle, sizeof(str_middle), "b : %+.5lf ", b);
		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[10]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_LEFT);
			caption->setString(caption, str_middle);
			dialog->addWidget(dialog, GUIID_CAPTION_MIDDIST_VAL_B, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}


		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[5]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			caption->setString(caption, STR_DISTCRR_LONG);
			dialog->addWidget(dialog, GUIID_CAPTION_LONGDIST, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		memset(str_long, 0x00, sizeof(str_long));
		taqmgr->getDistOffsetLG(taqmgr, &a, &b);
		snprintf(str_long, sizeof(str_long), "A : %+.5lf", a);
		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[8]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_LEFT);
			caption->setString(caption, str_long);
			dialog->addWidget(dialog, GUIID_CAPTION_LONGDIST_VAL_A, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		snprintf(str_long, sizeof(str_long), "b : %+.5lf", b);
		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[11]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_LEFT);
			caption->setString(caption, str_long);
			dialog->addWidget(dialog, GUIID_CAPTION_LONGDIST_VAL_B, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}


		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[1]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			caption->setString(caption, STR_DISTCRR_CHECK);
			dialog->addWidget(dialog, GUIID_CAPTION_DISTCHECK, WIDGET_CAPTION, (void *)caption);

			dialog->setPosition(dialog, &position[0]);
			dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}
		devconf_enumerate_parameters();
	}
	else
		TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

	return dialog;
}

dialog_t *
create_dialog_azimuth_correct(void *arg)
{
	dialog_t *dialog = NULL;
	button_t *button = NULL;
	caption_t *caption = NULL;
	spinbox_t *spinbox = NULL;
	struct main_interface *mfod = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mfod->taqmgr;
	double angle = 0.0;
	char angle_azimuth[16] = {0};

	rect_t position[] =
	{
			{.x = 200,  .y = 150, 	.w = 400, 	.h = 290}, 	/* 0: short distance correct dialog position */
			{.x = 270, 	.y = 180, 	.w = 260, 	.h = 20	},  /* 1: caption widget position */
			{.x = 300, 	.y = 380, 	.w = 80,	.h = 30	},	/* 2: button widget(save) positon */
			{.x = 450,	.y = 380,	.w = 80, 	.h = 30	},	/* 3: button widget (cancel) position */
			{.x = 460, 	.y = 275,	.w = 20,	.h = 30	},	/* 4: caption widget (mil) position */
			{.x = 370, 	.y = 275, 	.w = 20, 	.h = 30	},	/* 5: caption widget (.) position */
			{.x = 310, 	.y = 275,	.w = 20, 	.h = 30	},	/* 6: spinbox widget Asign position */
			{.x = 330, 	.y = 275,	.w = 20, 	.h = 30	},	/* 7: spinbox widget A0 position */
			{.x = 350, 	.y = 275,	.w = 20, 	.h = 30	},	/* 8: spinbox widget A1 position */
			{.x = 390, 	.y = 275,	.w = 20, 	.h = 30	},	/* 9: spinbox widget A2 position */
			{.x = 410, 	.y = 275,	.w = 20, 	.h = 30	},	/* 10: spinbox widget A3 position */
			{.x = 430, 	.y = 275,	.w = 20, 	.h = 30	},	/* 11: spinbox widget A4 position */
	};

	struct spinbox_property
	{
		int id;
		int minval;
		int maxval;
		int cval;
	};

	struct spinbox_property azimuth[] =
	{
			{GUIID_SPINBOX_AZIMUTH_SIGN			, 0x2B,	0x2D, 0x2B},
			{GUIID_SPINBOX_AZIMUTH_DIGIT0		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_AZIMUTH_DIGIT1		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_AZIMUTH_DIGIT2		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_AZIMUTH_DIGIT3		, 0x30,	0x39, 0x30},
			{GUIID_SPINBOX_AZIMUTH_DIGIT4		, 0x30,	0x39, 0x30},
	};

	dialog = gui_dialog_create(GUIID_DIALOG_AZM);

	if(dialog)
	{
		caption = gui_caption_create();
		if(caption)
		{
			caption->setString(caption, "mil");
			caption->setPosition(caption, &position[4]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			dialog->addWidget(dialog, GUIID_CAPTION_ANGLE_UNIT, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setString(caption, ".");
			caption->setPosition(caption, &position[5]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			dialog->addWidget(dialog, GUIID_CAPTION_DECIMAL_POINT, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		button = gui_button_create();
		if(button)
		{
			button->setTitle(button, STR_SAVE);
			button->setPosition(button, &position[2]);
			button->setCallback(button, sysctrl_save_azimuth_correct);
			dialog->addWidget(dialog, GUIID_BUTTON_SAVE, WIDGET_BUTTON, (void *)button);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		button = gui_button_create();
		if(button)
		{
			button->setTitle(button, STR_CANCEL);
			button->setPosition(button, &position[3]);
			button->setCallback(button, NULL);
			dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *)button);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		memset(angle_azimuth, 0x00, sizeof(angle_azimuth));
		taqmgr->getAngleOffsetAZ(taqmgr, &angle);
		snprintf(angle_azimuth, sizeof(angle_azimuth), "%+06d", ROUND(angle));
		TLOGMSG(1, ("angle azimuth offset = %s\n", angle_azimuth));

		for(int i = 0; i <DIM(azimuth); i++) azimuth[i].cval = angle_azimuth[i];

		for(int i = 0; i < DIM(azimuth); i++)
		{
			spinbox = gui_spinbox_create();
			if(spinbox)
			{
				spinbox->setMaxValue(spinbox, azimuth[i].maxval);
				spinbox->setMinValue(spinbox, azimuth[i].minval);
				spinbox->setCurrVal(spinbox, azimuth[i].cval);
				spinbox->setPosition(spinbox, &position[i + 6]);
				spinbox->setFontSize(spinbox, SPINBOX_FONT_SIZE_LARGE);
				spinbox->setDataType(spinbox, SPINBOX_DATATYPE_CHAR);

				if(azimuth[i].id == GUIID_SPINBOX_AZIMUTH_SIGN)
					spinbox->setDeltaValue(spinbox, 2);

				dialog->addWidget(dialog, azimuth[i].id, WIDGET_SPINBOX, (void *)spinbox);
			}
			else
			{
				gui_dialog_destroy(dialog);
				return NULL;
			}
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[1]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			caption->setString(caption, STR_ANGCRR_AZM);
			dialog->addWidget(dialog, GUIID_CAPTION_AZM, WIDGET_CAPTION, (void *)caption);

			dialog->setPosition(dialog, &position[0]);
			dialog->setFocus(dialog, GUIID_BUTTON_CANCEL);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

	}
	else
		TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

	return dialog;
}

dialog_t *
create_dialog_altitude_correct(void *arg)
{
	dialog_t *dialog = NULL;
	button_t *button = NULL;
	caption_t *caption = NULL;
	spinbox_t *spinbox = NULL;
	struct main_interface *mfod = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mfod->taqmgr;
	double angle = 0.0;
	char angle_altitude[16] = {0};

	rect_t position[] =
	{
			{.x = 200,  .y = 150, 	.w = 400, 	.h = 290}, 	/* 0: short distance correct dialog position */
			{.x = 270, 	.y = 180, 	.w = 260, 	.h = 20	},  /* 1: caption widget position */
			{.x = 300, 	.y = 380, 	.w = 80,	.h = 30	},	/* 2: button widget(save) positon */
			{.x = 450,	.y = 380,	.w = 80, 	.h = 30	},	/* 3: button widget (cancel) position */
			{.x = 460, 	.y = 275,	.w = 20,	.h = 30	},	/* 4: caption widget (mil) position */
			{.x = 370, 	.y = 275, 	.w = 20, 	.h = 30	},	/* 5: caption widget (.) position */
			{.x = 310, 	.y = 275,	.w = 20, 	.h = 30	},	/* 6: spinbox widget Asign position */
			{.x = 330, 	.y = 275,	.w = 20, 	.h = 30	},	/* 7: spinbox widget A0 position */
			{.x = 350, 	.y = 275,	.w = 20, 	.h = 30	},	/* 8: spinbox widget A1 position */
			{.x = 390, 	.y = 275,	.w = 20, 	.h = 30	},	/* 9: spinbox widget A2 position */
			{.x = 410, 	.y = 275,	.w = 20, 	.h = 30	},	/* 10: spinbox widget A3 position */
			{.x = 430, 	.y = 275,	.w = 20, 	.h = 30	},	/* 11: spinbox widget A4 position */
	};

	struct spinbox_property
	{
		int id;
		int minval;
		int maxval;
		int cval;
	};

	struct spinbox_property altitude[] =
	{
			{GUIID_SPINBOX_ALTITUDE_SIGN,		0x2B,	0x2D,	0x2B},
			{GUIID_SPINBOX_ALTITUDE_DIGIT0, 	0x30,	0x39,	0x30},
			{GUIID_SPINBOX_ALTITUDE_DIGIT1, 	0x30, 	0x39,	0x30},
			{GUIID_SPINBOX_ALTITUDE_DIGIT2,		0x30,	0x39,	0x30},
			{GUIID_SPINBOX_ALTITUDE_DIGIT3,		0x30,	0x39,	0x30},
			{GUIID_SPINBOX_ALTITUDE_DIGIT4,		0x30,	0x39,	0x30}
	};

	dialog = gui_dialog_create(GUIID_DIALOG_ALTT);

	if(dialog)
	{
		caption = gui_caption_create();
		if(caption)
		{
			caption->setString(caption, "mil");
			caption->setPosition(caption, &position[4]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			dialog->addWidget(dialog, GUIID_CAPTION_ANGLE_UNIT, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setString(caption, ".");
			caption->setPosition(caption, &position[5]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			dialog->addWidget(dialog, GUIID_CAPTION_DECIMAL_POINT, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		button = gui_button_create();
		if(button)
		{
			button->setTitle(button, STR_SAVE);
			button->setPosition(button, &position[2]);
			button->setCallback(button, sysctrl_save_altitude_correct);
			dialog->addWidget(dialog, GUIID_BUTTON_SAVE, WIDGET_BUTTON, (void *)button);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		button = gui_button_create();
		if(button)
		{
			button->setTitle(button, STR_CANCEL);
			button->setPosition(button, &position[3]);
			button->setCallback(button, NULL);
			dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *)button);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		memset(angle_altitude, 0x00, sizeof(angle_altitude));
		taqmgr->getAngleOffsetAT(taqmgr, &angle);
		snprintf(angle_altitude, sizeof(angle_altitude), "%+06d", ROUND(angle));
		TLOGMSG(1, ("angle altitude offset = %s\n", angle_altitude));

		for(int i = 0; i  <DIM(altitude); i++) altitude[i].cval = angle_altitude[i];

		for(int i = 0; i < DIM(altitude); i++)
		{
			spinbox = gui_spinbox_create();
			if(spinbox)
			{
				spinbox->setMaxValue(spinbox, altitude[i].maxval);
				spinbox->setMinValue(spinbox, altitude[i].minval);
				spinbox->setCurrVal(spinbox, altitude[i].cval);
				spinbox->setPosition(spinbox, &position[i + 6]);
				spinbox->setFontSize(spinbox, SPINBOX_FONT_SIZE_LARGE);
				spinbox->setDataType(spinbox, SPINBOX_DATATYPE_CHAR);

				if(altitude[i].id == GUIID_SPINBOX_ALTITUDE_SIGN)
					spinbox->setDeltaValue(spinbox, 2);

				dialog->addWidget(dialog, altitude[i].id, WIDGET_SPINBOX, (void *)spinbox);
			}
			else
			{
				gui_dialog_destroy(dialog);
				return NULL;
			}
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[1]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			caption->setString(caption, STR_ANGCRR_ALTT);
			dialog->addWidget(dialog, GUIID_CAPTION_ALTT, WIDGET_CAPTION, (void *)caption);

			dialog->setPosition(dialog, &position[0]);
			dialog->setFocus(dialog, GUIID_BUTTON_CANCEL);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}
	}
	else
		TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

	return dialog;
}

dialog_t *
create_dialog_angle_correct_check(void *arg)
{
	dialog_t *dialog = NULL;
	button_t *button = NULL;
	caption_t *caption = NULL;
	struct main_interface *mfod = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mfod->taqmgr;
	double azimuth = 0.0;
	double altitude = 0.0;
	char str_azimuth[50] = {0};
	char str_altitude[50] = {0};

	rect_t position[] =
	{
			{.x = 200,	.y = 150,	.w = 400,	.h = 384},	/* 0: short distance correct dialog position */
			{.x = 270,	.y = 180,	.w = 260,	.h = 30	},	/* 1: caption widget position */
			{.x = 375,	.y = 484, 	.w = 80,	.h = 30	},	/* 2: button widget (confirm) position */
			{.x = 250,	.y = 280,	.w = 100,	.h = 30	},	/* 3: caption widget (azimuth) position */
			{.x = 250,	.y = 380, 	.w = 100,	.h = 30	},	/* 4: caption widget (altitude) position	*/
			{.x = 420,	.y = 280,	.w = 100, 	.h = 30	}, 	/* 5: caption widget (azimuth val) position */
			{.x = 420,	.y = 380,	.w = 100,	.h = 30	},	/* 6: caption widget (altitude val) position */
	};

	dialog = gui_dialog_create(GUIID_DIALOG_ANGLECHECK);

	if(dialog)
	{
		button = gui_button_create();
		if(button)
		{
			button->setTitle(button, STR_CONFIRM);
			button->setPosition(button, &position[2]);
			button->setCallback(button, NULL);
			dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *)button);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[3]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_LEFT);
			caption->setString(caption, STR_ANGCRR_AZM);
			dialog->addWidget(dialog, GUIID_CAPTION_AZM, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		memset(str_azimuth, 0x00, sizeof(str_azimuth));
		taqmgr->getAngleOffsetAZ(taqmgr, &azimuth);
		snprintf(str_azimuth, sizeof(str_azimuth), "%+.3lf mil", azimuth);
		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[5]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_LEFT);
			caption->setString(caption, str_azimuth);
			dialog->addWidget(dialog, GUIID_CAPTION_AZIMUTH_VAL, WIDGET_CAPTION, (void*)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[4]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_LEFT);
			caption->setString(caption, STR_ANGCRR_ALTT);
			dialog->addWidget(dialog, GUIID_CAPTION_ALTT, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		memset(str_altitude, 0x00, sizeof(str_altitude));
		taqmgr->getAngleOffsetAT(taqmgr, &altitude);
		snprintf(str_altitude, sizeof(str_altitude), "%+.3lf mil", altitude);
		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[6]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_LEFT);
			caption->setString(caption, str_altitude);
			dialog->addWidget(dialog, GUIID_CAPTION_ALTITUDE_VAL, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[1]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			caption->setString(caption, STR_ANGCRR_CHECK);
			dialog->addWidget(dialog, GUIID_CAPTION_ANGLECHECK, WIDGET_CAPTION, (void *)caption);

			dialog->setPosition(dialog, &position[0]);
			dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}
		devconf_enumerate_parameters();
	}
	else
		TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

	return dialog;
}

dialog_t *
create_dialog_sensor_correct(void *arg)
{
	dialog_t *dialog = NULL;
	button_t *button = NULL;
	caption_t *caption = NULL;
	spinbox_t *spinbox = NULL;
	struct main_interface *mfod = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mfod->taqmgr;
	char sensor_A[16] = {0};
	char sensor_B[16] = {0};
	unsigned char A = 0;
	unsigned char B = 0;

	rect_t position[] =
	{
			{.x = 200,	.y = 150, 	.w = 400, 	.h = 290}, 	/* 0: short distance correct dialog position */
			{.x = 270, 	.y = 180, 	.w = 260, 	.h = 20	},  /* 1: caption widget position */
			{.x = 300, 	.y = 380, 	.w = 80,  	.h = 30 },  /* 2: button widget(save) position */
			{.x = 450,	.y = 380,	.w = 80, 	.h = 30	},	/* 3: button widget (cancel) position */
			{.x = 340, 	.y = 240,	.w = 20,	.h = 30	},	/* 4: caption widget (A-V) position */
			{.x = 340, 	.y = 310, 	.w = 20,	.h = 30 },  /* 5: caption widget (B-H) position */
			{.x = 390,	.y = 240,	.w = 20,	.h = 30	},	/* 6: spinbox widget A0 position */
			{.x = 410, 	.y = 240,	.w = 20,	.h = 30	},	/* 7: spinbox widget A1 position */
			{.x = 430,	.y = 240,	.w = 20,	.h = 30	},	/* 8: spinbox widget A2 position */
			{.x = 390, 	.y = 310,	.w = 20, 	.h = 30	},	/* 9: spinbox widget B0 position */
			{.x = 410,	.y = 310,	.w = 20,	.h = 30	},	/* 10: spinbox widget B1 position */
			{.x = 430,	.y = 310, 	.w = 20,	.h = 30	},	/* 11: spinbox widget B2 position */
	};

	struct spinbox_property
	{
		int id;
		int minval;
		int maxval;
		int cval;
	};

	struct spinbox_property A_digit[] =
	{
			{GUIID_SPINBOX_SENSOR_DIGITA0,	0x30,	0x32,	0x30},
			{GUIID_SPINBOX_SENSOR_DIGITA1, 	0x30,	0x39,	0x30},
			{GUIID_SPINBOX_SENSOR_DIGITA2, 	0x30,	0x39,	0x30}
	};

	struct spinbox_property B_digit[] =
	{
			{GUIID_SPINBOX_SENSOR_DIGITB0,	0x30,	0x32,	0x30},
			{GUIID_SPINBOX_SENSOR_DIGITB1, 	0x30,	0x39,	0x30},
			{GUIID_SPINBOX_SENSOR_DIGITB2, 	0x30,	0x39,	0x30}
	};

	dialog = gui_dialog_create(GUIID_DIALOG_SENSOR);

	if(dialog)
	{
		caption = gui_caption_create();
		if(caption)
		{
			caption->setString(caption, "A :");
			caption->setPosition(caption, &position[4]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			dialog->addWidget(dialog, GUIID_CAPTION_A, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setString(caption, "B :");
			caption->setPosition(caption, &position[5]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			dialog->addWidget(dialog, GUIID_CAPTION_B, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		button = gui_button_create();
		if(button)
		{
			button->setTitle(button, STR_SAVE);
			button->setPosition(button, &position[2]);
			button->setCallback(button, sysctrl_save_sensor_correct);
			dialog->addWidget(dialog, GUIID_BUTTON_SAVE, WIDGET_BUTTON, (void *)button);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		button = gui_button_create();
		if(button)
		{
			button->setTitle(button, STR_CANCEL);
			button->setPosition(button, &position[3]);
			button->setCallback(button, NULL);
			dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *)button);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		memset(sensor_A, 0x00, sizeof(sensor_A));
		memset(sensor_B, 0x00, sizeof(sensor_B));
		taqmgr->getSensorOffset(taqmgr, &A, &B);
		snprintf(sensor_A, sizeof(sensor_A), "%03d", A);
		snprintf(sensor_B, sizeof(sensor_B), "%03d", B);
		TLOGMSG(1, ("sensor offset  A = %s, B = %s\n", sensor_A, sensor_B));

		for(int i = 0; i < DIM(A_digit); i++) A_digit[i].cval = sensor_A[i];
		for(int i = 0; i < DIM(B_digit); i++) B_digit[i].cval = sensor_B[i];

		for(int i = 0; i < DIM(A_digit); i++)
		{
			spinbox = gui_spinbox_create();
			if(spinbox)
			{
				spinbox->setMaxValue(spinbox, A_digit[i].maxval);
				spinbox->setMinValue(spinbox, A_digit[i].minval);
				spinbox->setCurrVal(spinbox, A_digit[i].cval);
				spinbox->setPosition(spinbox, &position[i+6]);
				spinbox->setFontSize(spinbox, SPINBOX_FONT_SIZE_LARGE);
				spinbox->setDataType(spinbox, SPINBOX_DATATYPE_CHAR);

				dialog->addWidget(dialog, A_digit[i].id, WIDGET_SPINBOX, (void *)spinbox);
			}
			else
			{
				gui_dialog_destroy(dialog);
				return NULL;
			}
		}

		for(int i = 0; i < DIM(B_digit); i++)
		{
			spinbox = gui_spinbox_create();
			if(spinbox)
			{
				spinbox->setMaxValue(spinbox, B_digit[i].maxval);
				spinbox->setMinValue(spinbox, B_digit[i].minval);
				spinbox->setCurrVal(spinbox, B_digit[i].cval);
				spinbox->setPosition(spinbox, &position[i+9]);
				spinbox->setFontSize(spinbox, SPINBOX_FONT_SIZE_LARGE);
				spinbox->setDataType(spinbox, SPINBOX_DATATYPE_CHAR);

				dialog->addWidget(dialog, B_digit[i].id, WIDGET_SPINBOX, (void *)spinbox);
			}
			else
			{
				gui_dialog_destroy(dialog);
				return NULL;
			}
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[1]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			caption->setString(caption, STR_SENCRR_SENSOR);
			dialog->addWidget(dialog, GUIID_CAPTION_SENSOR, WIDGET_CAPTION, (void *)caption);

			dialog->setPosition(dialog, &position[0]);
			dialog->setFocus(dialog, GUIID_BUTTON_CANCEL);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}
	}
	else
		TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

	return dialog;
}

dialog_t *
create_dialog_sensor_correct_check(void *arg)
{
	dialog_t *dialog = NULL;
	button_t *button = NULL;
	caption_t *caption = NULL;
	struct main_interface *mfod = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mfod->taqmgr;
	unsigned char A = 0;
	unsigned char B = 0;
	char str_A[25] = {0};
	char str_B[25] = {0};

	rect_t position[] =
	{
			{.x = 200,	.y = 150, 	.w = 400, 	.h = 384}, 	/* 0: short distance correct dialog position */
			{.x = 270, 	.y = 180, 	.w = 260, 	.h = 20	},  /* 1: caption widget position */
			{.x = 375, 	.y = 484, 	.w = 80, 	.h = 30}, 	/* 2: button widget (confirm) position */
			{.x = 270, 	.y = 280,	.w = 250,	.h = 30}, 	/* 3: caption widget (A val) position */
			{.x = 270, 	.y = 380, 	.w = 250,	.h = 30},	/* 4: caption widget (B val) position */
	};

	dialog = gui_dialog_create(GUIID_DIALOG_SENSORCHECK);

	if(dialog)
	{
		button = gui_button_create();
		if(button)
		{
			button->setPosition(button, &position[2]);
			button->setTitle(button, STR_CONFIRM);
			button->setCallback(button, NULL);
			dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *)button);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		memset(str_A, 0x00, sizeof(str_A));
		memset(str_B, 0x00, sizeof(str_A));
		taqmgr->getSensorOffset(taqmgr, &A, &B);
		snprintf(str_A, sizeof(str_A), "A              %03d", A);
		snprintf(str_B, sizeof(str_B), "B              %03d", B);

		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[3]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			caption->setString(caption, str_A);
			dialog->addWidget(dialog, GUIID_CAPTION_A, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[4]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			caption->setString(caption, str_B);
			dialog->addWidget(dialog, GUIID_CAPTION_B, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[1]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			caption->setString(caption, STR_SENCRR_CHECK);
			dialog->addWidget(dialog, GUIID_CAPTION_SENSORCHECK, WIDGET_CAPTION, (void *)caption);

			dialog->setPosition(dialog, &position[0]);
			dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}
		devconf_enumerate_parameters();

	}
	else
		TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

	return dialog;
}

dialog_t *
create_dialog_ir_array(void *arg)
{
	dialog_t *dialog = NULL;
	button_t *button = NULL;
	caption_t *caption = NULL;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct vgss_interface *vgss = mainif->device->vgss;

	rect_t position[] =
	{
			{.x = 260,  .y = 370, 	.w = 280, 	.h = 100}, 	/* 0: short distance correct dialog position */
			{.x = 270, 	.y = 400, 	.w = 260, 	.h = 20	},  /* 1: caption widget position */
			{.x = 300, 	.y = 430, 	.w = 80,  	.h = 30 },  /* 2: button widget(start) position */
			{.x = 420,	.y = 430,	.w = 80, 	.h = 30	},	/* 3: button widget(cancel) position */
	};

	dialog = gui_dialog_create(GUIID_DIALOG_IRARRAY);

	if(dialog)
	{

		if(vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE) == PREVIEW_STATE_STOP)
		{
			button = gui_button_create();
			if(button)
			{
				button->setTitle(button, STR_YES);
				button->setPosition(button, &position[2]);
				button->setCallback(button, sysctrl_toggle_obmode);
				dialog->addWidget(dialog, GUIID_BUTTON_YES, WIDGET_BUTTON, (void *)button);
			}
			else
			{
				gui_dialog_destroy(dialog);
				return NULL;
			}
		}
		else
		{
			button = gui_button_create();
			if(button)
			{
				button->setTitle(button, STR_SAVE_END);
				button->setPosition(button, &position[2]);
				button->setCallback(button, sysctrl_toggle_obmode);
				dialog->addWidget(dialog, GUIID_BUTTON_SAVE_END, WIDGET_BUTTON, (void *)button);
			}
			else
			{
				gui_dialog_destroy(dialog);
				return NULL;
			}
		}

		button = gui_button_create();
		if(button)
		{
			if(vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE) == PREVIEW_STATE_STOP)
				button->setTitle(button, STR_NO);
			else
				button->setTitle(button, STR_CANCEL);
			button->setPosition(button, &position[3]);
			button->setCallback(button, NULL);
			dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *)button);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[1]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_16);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);

			 if(vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE) == PREVIEW_STATE_STOP)
			     caption->setString(caption, STR_IRARRAY_START);
			 else
			     caption->setString(caption, STR_IRARRAY_END);
			dialog->addWidget(dialog, GUIID_CAPTION_IRARRAY, WIDGET_CAPTION, (void *)caption);
			dialog->setTitle(dialog, STR_IR_ARRAY);
			dialog->setPosition(dialog, &position[0]);
			dialog->setFocus(dialog, GUIID_BUTTON_CANCEL);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}
	}
	else
		TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

	return dialog;
}


dialog_t*
create_dialog_ir_scale_input(void *arg)
{
	dialog_t *dialog = NULL;
	button_t *button = NULL;
	caption_t *caption = NULL;

	rect_t position[] =
	{
			{.x = 260,	.y = 370,	.w = 280,	.h = 100}, 	/* 0: short distance correct dialog position */
			{.x = 270,  .y = 400, 	.w = 260,	.h = 20	}, 	/* 1: caption widget position */
			{.x = 300, 	.y = 430, 	.w = 80,  	.h = 30 },  /* 2: button widget(save) position */
			{.x = 420,	.y = 430,	.w = 80, 	.h = 30	},	/* 3: button widget (cancel) position */
	};

	dialog = gui_dialog_create(GUIID_DIALOG_IRSCALEINPUT);

	if(dialog)
	{
		button = gui_button_create();
		if(button)
		{
			button->setPosition(button, &position[2]);
			button->setTitle(button, STR_ON);
			button->setCallback(button, NULL);
			dialog->addWidget(dialog, GUIID_BUTTON_ON, WIDGET_BUTTON, (void *)button);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		button = gui_button_create();
		if(button)
		{
			button->setPosition(button, &position[3]);
			button->setTitle(button, STR_OFF);
			button->setCallback(button, NULL);
			dialog->addWidget(dialog, GUIID_BUTTON_OFF, WIDGET_BUTTON, (void *)button);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}


		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[1]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_16);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			caption->setString(caption, STR_IRRARR_IRSCALEINPUT);
			dialog->addWidget(dialog, GUIID_CAPTION_IRSCALEINPUT, WIDGET_CAPTION,(void*)caption);
			dialog->setTitle(dialog, STR_IRRARR_IRSCALEINPUT);
			dialog->setPosition(dialog, &position[0]);
			dialog->setFocus(dialog, GUIID_BUTTON_OFF);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}
	}

	return dialog;
}


dialog_t *
create_dialog_ir_array_check(void *arg)
{
	dialog_t *dialog = NULL;
	button_t *button = NULL;
	caption_t *caption = NULL;
	struct main_interface *mainif = (struct main_interface *)arg;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;
	unsigned char vertical = 0;
	unsigned char horizontal = 0;
	int scale = 0;
	char str_v[16] = {0};
	char str_h[16] = {0};

	rect_t position[] =
	{
			{.x = 200,  .y = 150, 	.w = 400, 	.h = 384}, 	/* 0: short distance correct dialog position */
			{.x = 270, 	.y = 180, 	.w = 260, 	.h = 20	},  /* 1: caption widget position */
			{.x = 375, 	.y = 484, 	.w = 80, 	.h = 30 }, 	/* 2: button widget (confirm) position */
			{.x = 270,	.y = 230,	.w = 100,	.h = 30	},	/* 3: caption widget (v) position */
			{.x = 270,	.y = 330, 	.w = 100,	.h = 30	},	/* 4: caption widget (h) position	*/
			{.x = 270, 	.y = 430, 	.w = 150, 	.h = 30 }, 	/* 5: caption widget (scale up) position */
			{.x = 480,	.y = 230,	.w = 100, 	.h = 30	}, 	/* 6: caption widget (v val) position */
			{.x = 480,	.y = 330,	.w = 100,	.h = 30	},	/* 7: caption widget (h val) position */
			{.x = 480,	.y = 430, 	.w = 100, 	.h = 30	}, 	/* 8: caption widget (scale val) position */
	};

	dialog = gui_dialog_create(GUIID_DIALOG_IRCHECK);

	if(dialog)
	{
		button =  gui_button_create();
		if(button)
		{
			button->setPosition(button, &position[2]);
			button->setTitle(button, STR_CONFIRM);
			button->setCallback(button, NULL);
			dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *)button);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		caption =  gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[3]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_LEFT);
			caption->setString(caption, STR_VERTICAL);
			dialog->addWidget(dialog, GUIID_CAPTION_VERTICAL, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		memset(str_v, 0x00, sizeof(str_v));
		memset(str_h, 0x00, sizeof(str_h));
		taqmgr->getIrArrayOffset(taqmgr, &vertical, &horizontal);
		snprintf(str_v, sizeof(str_v), "%03d", vertical);
		snprintf(str_h, sizeof(str_h), "%03d", horizontal);

		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[6]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_LEFT);
			caption->setString(caption, str_v);
			dialog->addWidget(dialog, GUIID_CAPTION_VERTICAL_VAL, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[4]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_LEFT);
			caption->setString(caption, STR_HORIZONTAL);
			dialog->addWidget(dialog, GUIID_CAPTION_HORIZONTAL, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[7]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_LEFT);
			caption->setString(caption, str_h);
			dialog->addWidget(dialog, GUIID_CAPTION_HORIZONTAL_VAL, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[5]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_LEFT);
			caption->setString(caption, STR_IRRARR_IRSCALEINPUT);
			dialog->addWidget(dialog, GUIID_CAPTION_IRSCALEINPUT, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		taqmgr->getIrScaleUp(taqmgr, &scale);
		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[8]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_LEFT);
			if(scale == 0) caption->setString(caption, STR_OFF);
			else caption->setString(caption, STR_ON);
			dialog->addWidget(dialog, GUIID_CAPTION_IRSCALEINPUT_VAL, WIDGET_CAPTION, (void *)caption);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[1]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_24);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			caption->setString(caption, STR_IRARR_CHECK);
			dialog->addWidget(dialog, GUIID_CAPTION_IRCHECK, WIDGET_CAPTION, (void *)caption);

			dialog->setPosition(dialog, &position[0]);
			dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}
		devconf_enumerate_parameters();
	}
	else
		TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

	return dialog;
}

dialog_t *
create_dialog_firmware_update(void *arg)
{
	dialog_t *dialog = NULL;
	button_t *button = NULL;
	caption_t *caption = NULL;


	rect_t position[] =
	{
			{.x = 260,	.y = 370, 	.w = 280, 	.h = 100}, 	/* 0: short distance correct dialog position */
			{.x = 270, 	.y = 400, 	.w = 260, 	.h = 20	},	/* 1: caption widget position */
			{.x = 300, 	.y = 430, 	.w = 80, 	.h = 30	},	/* 2: button widget(update) position */
			{.x = 420, 	.y = 430, 	.w = 80, 	.h = 30	}	/* 3: button widget(cancel) position */
	};

	dialog = gui_dialog_create(GUIID_DIALOG_FWUPDATE);

	if(dialog)
	{
		button =  gui_button_create();
		if(button)
		{
			button->setPosition(button, &position[2]);
			button->setTitle(button, STR_YES);
			button->setCallback(button, NULL);
			dialog->addWidget(dialog, GUIID_BUTTON_YES, WIDGET_BUTTON, (void *)button);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		button = gui_button_create();
		if(button)
		{
			button->setPosition(button, &position[3]);
			button->setTitle(button, STR_CANCEL);
			button->setCallback(button, NULL);
			dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *)button);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[1]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_16);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			caption->setString(caption, STR_FWUPDATE_CONFIRM);

			dialog->addWidget(dialog, GUIID_CAPTION_FWUPDATE, WIDGET_CAPTION, (void *)caption);
			dialog->setTitle(dialog, STR_FWUPDATE_UPDATE);
			dialog->setPosition(dialog, &position[0]);
			dialog->setFocus(dialog, GUIID_BUTTON_CANCEL);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}
	}
	else
		TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

	return dialog;
}

dialog_t *
create_dialog_correct_init(void *arg)
{
	dialog_t *dialog = NULL;
	button_t *button = NULL;
	caption_t *caption = NULL;

	rect_t position[] =
	{
			{.x = 260,  .y = 370, 	.w = 280, 	.h = 100}, 	/* 0: short distance correct dialog position */
			{.x = 270, 	.y = 400, 	.w = 260, 	.h = 20	}, 	/* 1: caption widget position */
			{.x = 300, 	.y = 430, 	.w = 80, 	.h = 30	},	/* 2: button widget(init) position */
			{.x = 420, 	.y = 430, 	.w = 80, 	.h = 30	}	/* 3: button widget(cancel) posirion */
	};

	dialog = gui_dialog_create(GUIID_DIALOG_INIT);

	if(dialog)
	{
		button = gui_button_create();
		if(button)
		{
			button->setPosition(button, &position[2]);
			button->setTitle(button, STR_YES);
			button->setCallback(button, sysctrl_init_correct);
			dialog->addWidget(dialog, GUIID_BUTTON_YES, WIDGET_BUTTON, (void *)button);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		button = gui_button_create();
		if(button)
		{
			button->setPosition(button, &position[3]);
			button->setTitle(button, STR_CANCEL);
			button->setCallback(button, NULL);
			dialog->addWidget(dialog, GUIID_BUTTON_CANCEL, WIDGET_BUTTON, (void *)button);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}

		caption = gui_caption_create();
		if(caption)
		{
			caption->setPosition(caption, &position[1]);
			caption->setFont(caption, CAPTION_FONT_NORMAL);
			caption->setFontHeight(caption, FONT_HEIGHT_16);
			caption->setAlign(caption, CAPTION_ALIGN_CENTER);
			caption->setString(caption, STR_CORRECT_INIT);
			dialog->addWidget(dialog, GUIID_CAPTION_INIT, WIDGET_CAPTION, (void *)caption);
			dialog->setTitle(dialog, STR_FWUPDATE_INIT);
			dialog->setPosition(dialog, &position[0]);
			dialog->setFocus(dialog, GUIID_BUTTON_CANCEL);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}
	}
	else
		TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

	return dialog;
}


dialog_t *
create_dialog_ir_move(void *arg)
{
	dialog_t *dialog = NULL;
	qsel_t *qsel = NULL;

	rect_t position[] =
	{
			{.x = 290,	.y = 450,	.w = 220,	.h = 130},   /* 0 - dialog position              */
			{.x = 370, 	.y = 500,  	.w = 60,	.h = 30	},   /* 1 - qsel item position : center  */
			{.x = 300, 	.y = 500,  	.w = 60,  	.h = 30	},   /* 2 - qsel item position : left    */
			{.x = 440, 	.y = 500,  	.w = 60,  	.h = 30 },   /* 3 - qsel item position : right   */
			{.x = 370, 	.y = 460,  	.w = 60,  	.h = 30	},   /* 4 - qsel item position : up      */
			{.x = 370, 	.y = 540,	.w = 60,  	.h = 30	},   /* 5 - qsel item position : down    */
	};

	dialog = gui_dialog_create(GUIID_DIALOG_MOVE_IR);
	if(dialog)
	{
		qsel = gui_qsel_create();
		if(qsel)
		{
			qsel->setPosition(qsel, QSEL_CENTER, &position[1]);
			qsel->setPosition(qsel, QSEL_LEFT, &position[2]);
			qsel->setPosition(qsel, QSEL_RIGHT, &position[3]);
			qsel->setPosition(qsel, QSEL_UP, &position[4]);
			qsel->setPosition(qsel, QSEL_DOWN, &position[5]);
			qsel->setString(qsel, QSEL_CENTER, STR_CLOSE);
			qsel->setString(qsel, QSEL_LEFT, STR_ARROW_LEFT);
			qsel->setString(qsel, QSEL_RIGHT, STR_ARROW_RIGHT);
			qsel->setString(qsel, QSEL_UP, STR_ARROW_UP);
			qsel->setString(qsel, QSEL_DOWN, STR_ARROW_DOWN);
			qsel->setCallback(qsel, QSEL_CENTER, sysctrl_irarray_end);
			qsel->setCallback(qsel, QSEL_LEFT, sysctrl_move_ir_right);
			qsel->setCallback(qsel, QSEL_RIGHT, sysctrl_move_ir_left);
			qsel->setCallback(qsel, QSEL_UP, sysctrl_move_ir_up);
			qsel->setCallback(qsel, QSEL_DOWN, sysctrl_move_ir_down);
			qsel->setFocus(qsel, QSEL_NONE);
			dialog->addWidget(dialog, GUIID_QSEL_TAQSEL, WIDGET_QSEL, (void *) qsel);
			dialog->setPosition(dialog, &position[0]);
			dialog->setFocus(dialog, GUIID_QSEL_TAQSEL);
		}
		else
		{
			gui_dialog_destroy(dialog);
			return NULL;
		}
	}
	else
		TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

	return dialog;
}



dialog_t *
create_dialog_fwupdate_nofile(void *arg)
{
	dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t position[] =
    {
        {260, 	250, 	280, 	100},   /* 0 : dialog   */
        {360, 	310,  	80,  	30},   /* 1 : button   */
        {270, 	280, 	260,  	20}    /* 2 : caption  */
    };

    dialog = gui_dialog_create(GUIID_DIALOG_NOFILE);

    if (dialog)
    {
    	caption = gui_caption_create();
    	if (caption)
    	{
    		caption->setPosition(caption, &position[2]);
    		caption->setFontHeight(caption, FONT_HEIGHT_16);
    		caption->setAlign(caption, CAPTION_ALIGN_CENTER);
    		caption->setFont(caption, CAPTION_FONT_NORMAL);
    		caption->setString(caption, STR_NOFILE);
    		dialog->addWidget(dialog, GUIID_CAPTION_FWUPDATE_ERROR, WIDGET_CAPTION, (void *) caption);
    	}
    	else
    	{
    		gui_dialog_destroy(dialog);
    		return NULL;
    	}

    	/* confirm button widget */
    	button = gui_button_create();
    	if (button)
    	{
    		button->setPosition(button, &position[1]);
    		button->setTitle(button, STR_CONFIRM);
    		button->setCallback(button, NULL);
    		dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
    		dialog->setTitle(dialog, STR_ERROR);
    		dialog->setPosition(dialog, &position[0]);
    		dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
    	}
    	else
    	{
    		gui_dialog_destroy(dialog);
    		return NULL;
    	}
    }
    else
    	TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

	return dialog;
}


dialog_t *
create_dialog_fwupdate_success(void *arg)
{
	dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t position[] =
    {
        {260, 	250, 	280, 	100},   /* 0 : dialog   */
        {360, 	310,  	80, 	30},   /* 1 : button   */
        {270, 	280, 	260,  	20}    /* 2 : caption  */
    };

    dialog = gui_dialog_create(GUIID_DIALOG_SUCCESS_FWUPDATE);

    if (dialog)
    {
    	caption = gui_caption_create();
    	if (caption)
    	{
    		caption->setPosition(caption, &position[2]);
    		caption->setFontHeight(caption, FONT_HEIGHT_16);
    		caption->setAlign(caption, CAPTION_ALIGN_CENTER);
    		caption->setFont(caption, CAPTION_FONT_NORMAL);
    		caption->setString(caption, STR_SUCCESS_FWUPDATE);
    		dialog->addWidget(dialog, GUIID_CAPTION_FWUPDATE_ERROR, WIDGET_CAPTION, (void *) caption);
    	}
    	else
    	{
    		gui_dialog_destroy(dialog);
    		return NULL;
    	}

    	/* confirm button widget */
    	button = gui_button_create();
    	if (button)
    	{
    		button->setPosition(button, &position[1]);
    		button->setTitle(button, STR_CONFIRM);
    		button->setCallback(button, NULL);
    		dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);

    		dialog->setPosition(dialog, &position[0]);
    		dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
    	}
    	else
    	{
    		gui_dialog_destroy(dialog);
    		return NULL;
    	}
    }
    else
    	TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

	return dialog;
}



dialog_t *
create_dialog_fwupdate_fail(void *arg)
{
	dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t position[] =
    {
        {260, 	250, 	280, 	100},   /* 0 : dialog   */
        {360, 	310,  	80,  	30},   /* 1 : button   */
        {270, 	280, 	260,  	20}    /* 2 : caption  */
    };

    dialog = gui_dialog_create(GUIID_DIALOG_FAIL_FWUPDATE);

    if (dialog)
    {
    	caption = gui_caption_create();
    	if (caption)
    	{
    		caption->setPosition(caption, &position[2]);
    		caption->setFontHeight(caption, FONT_HEIGHT_16);
    		caption->setAlign(caption, CAPTION_ALIGN_CENTER);
    		caption->setFont(caption, CAPTION_FONT_NORMAL);
    		caption->setString(caption, STR_FAIL_FWUPDATE);
    		dialog->addWidget(dialog, GUIID_CAPTION_FWUPDATE_ERROR, WIDGET_CAPTION, (void *) caption);
    	}
    	else
    	{
    		gui_dialog_destroy(dialog);
    		return NULL;
    	}

    	/* confirm button widget */
    	button = gui_button_create();
    	if (button)
    	{
    		button->setPosition(button, &position[1]);
    		button->setTitle(button, STR_CONFIRM);
    		button->setCallback(button, NULL);
    		dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
    		dialog->setTitle(dialog, STR_ERROR);
    		dialog->setPosition(dialog, &position[0]);
    		dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
    	}
    	else
    	{
    		gui_dialog_destroy(dialog);
    		return NULL;
    	}
    }
    else
    	TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

	return dialog;
}



dialog_t *
create_dialog_fwupdate_nosdcard(void *arg)
{
	dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;

    rect_t position[] =
    {
        {260, 	250, 	280, 	100},   /* 0 : dialog   */
        {360, 	310,  	80,  	30},   /* 1 : button   */
        {270, 	280, 	260,  	20}    /* 2 : caption  */
    };

    dialog = gui_dialog_create(GUIID_DIALOG_NOSDCARD);

    if (dialog)
    {
    	caption = gui_caption_create();
    	if (caption)
    	{
    		caption->setPosition(caption, &position[2]);
    		caption->setFontHeight(caption, FONT_HEIGHT_16);
    		caption->setAlign(caption, CAPTION_ALIGN_CENTER);
    		caption->setFont(caption, CAPTION_FONT_NORMAL);
    		caption->setString(caption, STR_INSERT_SDCARD);
    		dialog->addWidget(dialog, GUIID_CAPTION_FWUPDATE_ERROR, WIDGET_CAPTION, (void *) caption);
    	}
    	else
    	{
    		gui_dialog_destroy(dialog);
    		return NULL;
    	}

    	/* confirm button widget */
    	button = gui_button_create();
    	if (button)
    	{
    		button->setPosition(button, &position[1]);
    		button->setTitle(button, STR_CONFIRM);
    		button->setCallback(button, NULL);
    		dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);

    		dialog->setTitle(dialog, STR_ERROR);
    		dialog->setPosition(dialog, &position[0]);
    		dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
    	}
    	else
    	{
    		gui_dialog_destroy(dialog);
    		return NULL;
    	}
    }
    else
    	TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

	return dialog;
}



/* application crc */
extern unsigned int g_app_crc32;

dialog_t *
create_dialog_system_info(void *arg)
{
    double azoff = 0.0;
    double eloff = 0.0;
    double rcc[4] = {0};
    char string[64]  = {0};
    dialog_t *dialog = NULL;
    button_t *button = NULL;
    caption_t *caption = NULL;
    struct main_interface *mainif = (struct main_interface *)arg;

    rect_t pos[] =
    {
        {210, 304, 380, 230},   /* 0 : dialog   */
        {360, 494,  80,  30},   /* 1 : button   */
        {220, 334, 360,  20},   /* 2 : caption  - S/W VERSION                   */
        {220, 354, 360,  20},   /* 3 : caption  - S/W CRC32                     */
        {220, 374, 360,  20},   /* 4 : caption  - S/W BULID DATE                */
        {220, 394, 360,  20},   /* 5 : caption  - range compensation parameter  */
        {220, 414, 360,  20},   /* 5 : caption  - range compensation parameter  */
        {220, 434, 360,  20},   /* 6 : caption  - angular offset                */
        {220, 464, 360,  20},   /* 7 : caption  - eosystem                      */
    };


    dialog = gui_dialog_create(GUIID_DIALOG_SYSTEM_INFO);

    if (dialog)
    {
        /* application version string */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_14);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_APP_VERSION);
            dialog->addWidget(dialog, GUIID_CAPTION_APP_VERSION, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* application version string */
        caption = gui_caption_create();

        if (caption)
        {
            memset(string, 0x00, sizeof(string));
            snprintf(string, sizeof(string), "ver.%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, BUILD_NUMBER);
            caption->setPosition(caption, &pos[2]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_RIGHT);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setString(caption, string);
            dialog->addWidget(dialog, GUIID_CAPTION_APP_VERSION_VALUE, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* application crc string */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[3]);
            caption->setFontHeight(caption, FONT_HEIGHT_14);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_APP_CRC);
            dialog->addWidget(dialog, GUIID_CAPTION_APP_CRC32, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* application crc32 value string */
        caption = gui_caption_create();

        if (caption)
        {
            memset(string, 0x00, sizeof(string));
            snprintf(string, sizeof(string), "0x%08X", g_app_crc32);
            caption->setPosition(caption, &pos[3]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_RIGHT);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setString(caption, string);
            dialog->addWidget(dialog, GUIID_CAPTION_APP_CRC32_VALUE, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* application build date string  */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[4]);
            caption->setFontHeight(caption, FONT_HEIGHT_14);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_APP_BUILD_DATE);
            dialog->addWidget(dialog, GUIID_CAPTION_APP_BUILD_DATE, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* application build date value string */
        caption = gui_caption_create();

        if (caption)
        {
            memset(string, 0x00, sizeof(string));
            snprintf(string, sizeof(string), "%s %s", __DATE__, __TIME__);
            caption->setPosition(caption, &pos[4]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_RIGHT);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setString(caption, string);
            dialog->addWidget(dialog, GUIID_CAPTION_APP_BUILD_DATE_VALUE, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* range compensation parameter string  */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[5]);
            caption->setFontHeight(caption, FONT_HEIGHT_14);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_RCC_PARAM);
            dialog->addWidget(dialog, GUIID_CAPTION_RCC_PARAM, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* short range rcc value string */
        caption = gui_caption_create();

        if (caption)
        {
            memset(string, 0x00, sizeof(string));
            mainif->device->lrf->getRcc(mainif->device->lrf, &rcc[0], &rcc[1], &rcc[2], &rcc[3]);
            snprintf(string, sizeof(string), "A = %.5f, b = %.5f", rcc[0], rcc[1]);
            caption->setPosition(caption, &pos[5]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_RIGHT);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setString(caption, string);
            dialog->addWidget(dialog, GUIID_CAPTION_RCC_PARAM_VALUE0, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* long range rcc value string */
        caption = gui_caption_create();

        if (caption)
        {
            memset(string, 0x00, sizeof(string));
            snprintf(string, sizeof(string), "A'= %.5f, b'= %.5f", rcc[2], rcc[3]);
            caption->setPosition(caption, &pos[6]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_RIGHT);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setString(caption, string);
            dialog->addWidget(dialog, GUIID_CAPTION_RCC_PARAM_VALUE1, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* angular offset string  */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[7]);
            caption->setFontHeight(caption, FONT_HEIGHT_14);
            caption->setAlign(caption, CAPTION_ALIGN_LEFT);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_ANGULAR_OFFSET);
            dialog->addWidget(dialog, GUIID_CAPTION_ANGULAR_OFFSET, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* angular offset value string */
        caption = gui_caption_create();

        if (caption)
        {
            memset(string, 0x00, sizeof(string));
            mainif->taqmgr->getAngleOffset(mainif->taqmgr, &azoff, &eloff);
            snprintf(string, sizeof(string), "a = %.5f, e = %.5f", azoff, eloff);
            caption->setPosition(caption, &pos[7]);
            caption->setFontHeight(caption, FONT_HEIGHT_16);
            caption->setAlign(caption, CAPTION_ALIGN_RIGHT);
            caption->setFont(caption, CAPTION_FONT_MONOSPACED);
            caption->setString(caption, string);
            dialog->addWidget(dialog, GUIID_CAPTION_ANGULAR_OFSSET_VALUE, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* eosystem string  */
        caption = gui_caption_create();

        if (caption)
        {
            caption->setPosition(caption, &pos[8]);
            caption->setFontHeight(caption, FONT_HEIGHT_14);
            caption->setAlign(caption, CAPTION_ALIGN_CENTER);
            caption->setFont(caption, CAPTION_FONT_NORMAL);
            caption->setString(caption, STR_EOSYSTEM);
            dialog->addWidget(dialog, GUIID_CAPTION_EOSYSTEM, WIDGET_CAPTION, (void *) caption);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }

        /* confirm button */
        button = gui_button_create();
        
        if (button)
        {
            button->setPosition(button, &pos[1]);
            button->setTitle(button, STR_CONFIRM);
            button->setCallback(button, NULL);
            dialog->addWidget(dialog, GUIID_BUTTON_CONFIRM, WIDGET_BUTTON, (void *) button);
            dialog->setTitle(dialog, STR_DEVCONF_SYSTEM_INFO);
            dialog->setPosition(dialog, &pos[0]);
            dialog->setFocus(dialog, GUIID_BUTTON_CONFIRM);
        }
        else
        {
            gui_dialog_destroy(dialog);
            return NULL;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog\n", DBGINFO));

    return dialog;
}



