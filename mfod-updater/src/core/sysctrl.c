/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    sysctrl.c
        external/internal function implementations of top-level system control module
        this file is  part of mainif-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/


#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

#include "amod.h"
#include "core/evque.h"
#include "core/logger.h"
#include "core/sysctrl.h"
#include "etc/util.h"
#include "gui/gui_dialog.h"
#include "etc/devconf.h"

int
sysctrl_power_off(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;

	mainif->guimgr->setMode(mainif->guimgr, GUI_MODE_PWROFF);
	mainif->device->vgss->ctrlPreview(mainif->device->vgss, PREVIEW_CTRL_DEINIT);
	MSLEEP(500);
	mainif->device->updateSystemTime(mainif->device);
	mainif->device->mcu->disablePmic(mainif->device->mcu);
	system("poweroff");

	while(1);

	return 0;
}


int
sysctrl_push_trigger(void *arg)
{
	int ret = 0;
	int dlgid = 0;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;
	struct gui_manager_interface *guimgr = mainif->guimgr;

	ret = taqmgr->start(taqmgr);

	if (ret == 0)
	{
		taqmgr->setStatus(taqmgr, TAQMGR_STATUS_INPROC);
		TLOGMSG(1, ("start target acquistion...\n"));
	}
	else
	{
		switch (ret)
		{
		case TAQERROR_ANGLE_LIMIT:
			dlgid = GUIID_DIALOG_ERROR_ANGLE_LIMIT;
			break;

		case TAQERROR_GRIDVAR:
			dlgid = GUIID_DIALOG_ERROR_GRIDVAR;
			break;

		default:
			dlgid = GUIID_DIALOG_ERROR_TAQ;
			break;
		}

		ret = -1;
		guimgr->showDialog(guimgr, dlgid, arg);
	}

	return ret;
}


int
sysctrl_release_trigger(void *arg)
{
	int ret = 0;
	int dlgid = 0;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;
	struct msgproc_interface *msgproc = mainif->msgproc;
	struct gui_manager_interface *guimgr = mainif->guimgr;

	ret = taqmgr->finalize(taqmgr);

	if (ret == 0)
	{
		switch (taqmgr->getTaqMode(taqmgr))
		{
		case TAQMODE_CIRCULAR_TARGET:
		case TAQMODE_SQUARE_TARGET_WIDTH:
			taqmgr->setTaqMode(taqmgr, TAQMODE_POINT_TARGET);
			taqmgr->setStatus(taqmgr, TAQMGR_STATUS_TARGET_ACQUIRED);

			if (msgproc->getAutoXmit(msgproc) == MSGPROC_AUTO_XMIT_ON)
				sysctrl_xmit_target_location(arg);

			guimgr->showDialog(guimgr, GUIID_DIALOG_TAQSEL, arg);
			break;

		case TAQMODE_FOS_CORRECTION:
			taqmgr->setTaqMode(taqmgr, TAQMODE_POINT_TARGET);
			taqmgr->setStatus(taqmgr, TAQMGR_STATUS_TARGET_ACQUIRED);

			if (msgproc->getAutoXmit(msgproc) == MSGPROC_AUTO_XMIT_ON)
				sysctrl_xmit_target_shift(arg);

			guimgr->showDialog(guimgr, GUIID_DIALOG_TAQSEL, arg);
			break;

		case TAQMODE_SQUARE_TARGET_LENGTH:
			taqmgr->setTaqMode(taqmgr, TAQMODE_SQUARE_TARGET_WIDTH);
			break;

		default:
			taqmgr->setStatus(taqmgr, TAQMGR_STATUS_TARGET_ACQUIRED);

			if (msgproc->getAutoXmit(msgproc) == MSGPROC_AUTO_XMIT_ON)
				sysctrl_xmit_target_location(arg);

			guimgr->showDialog(guimgr, GUIID_DIALOG_TAQSEL, arg);
			break;
		}
	}
	else
	{
		if (taqmgr->getTaqMode(taqmgr) == TAQMODE_POINT_TARGET)
			taqmgr->setStatus(taqmgr, TAQMGR_STATUS_IDLE);
		else
		{
			switch (ret)
			{
			case TAQ_RETCODE_ERROR:
				taqmgr->setStatus(taqmgr, TAQMGR_STATUS_IDLE);
				taqmgr->reset(taqmgr);
				taqmgr->setTaqMode(taqmgr, TAQMODE_POINT_TARGET);
				guimgr->showDialog(guimgr, GUIID_DIALOG_ERROR_DMC_OFFLINE, arg);
				break;

			case TAQ_RETCODE_GNSS_ERROR:
				taqmgr->setStatus(taqmgr, TAQMGR_STATUS_IDLE);
				taqmgr->reset(taqmgr);
				taqmgr->setTaqMode(taqmgr, TAQMODE_POINT_TARGET);
				guimgr->showDialog(guimgr, GUIID_DIALOG_ERROR_GNSS_OFFLINE, arg);
				break;

			case TAQ_RETCODE_DMC_ERROR:
				taqmgr->setStatus(taqmgr, TAQMGR_STATUS_IDLE);
				taqmgr->reset(taqmgr);
				taqmgr->setTaqMode(taqmgr, TAQMODE_POINT_TARGET);
				guimgr->showDialog(guimgr, GUIID_DIALOG_ERROR_TAQ, arg);
				break;

			case TAQ_RETCODE_FAIL_GEODCALC:
				taqmgr->setStatus(taqmgr, TAQMGR_STATUS_TARGET_ACQUIRED);
				taqmgr->setTaqMode(taqmgr, TAQMODE_POINT_TARGET);
				guimgr->showDialog(guimgr, GUIID_DIALOG_TAQSEL, arg);
				guimgr->showDialog(guimgr, GUIID_DIALOG_ERROR_GEODCALC, arg);
				break;

			case TAQ_RETCODE_TARGET_SIZE_OVER:
				taqmgr->setStatus(taqmgr, TAQMGR_STATUS_TARGET_ACQUIRED);
				taqmgr->setTaqMode(taqmgr, TAQMODE_POINT_TARGET);
				guimgr->showDialog(guimgr, GUIID_DIALOG_TAQSEL, arg);
				guimgr->showDialog(guimgr, GUIID_DIALOG_ERROR_TARGET_SIZE, arg);
				break;

			case TAQ_RETCODE_SHIFT_RANGE_OVER:
				taqmgr->setStatus(taqmgr, TAQMGR_STATUS_TARGET_ACQUIRED);
				taqmgr->setTaqMode(taqmgr, TAQMODE_POINT_TARGET);
				guimgr->showDialog(guimgr, GUIID_DIALOG_TAQSEL, arg);
				guimgr->showDialog(guimgr, GUIID_DIALOG_ERROR_TARGET_SHIFT, arg);
				break;

			default:
				taqmgr->setStatus(taqmgr, TAQMGR_STATUS_TARGET_ACQUIRED);
				taqmgr->setTaqMode(taqmgr, TAQMODE_POINT_TARGET);
				guimgr->showDialog(guimgr, GUIID_DIALOG_TAQSEL, arg);
				guimgr->showDialog(guimgr, GUIID_DIALOG_ERROR_ZERO_RANGE, arg);
				break;
			}
		}
	}

	return ret;
}


int
sysctrl_handle_left_keyin(void *arg)
{
	int ret = 0;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct vgss_interface *vgss = mainif->device->vgss;
	struct mcu_interface  *mcu  = mainif->device->mcu;
	struct gui_manager_interface *guimgr = mainif->guimgr;

	if (vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE) == PREVIEW_STATE_RUN)
	{
		if (vgss->ctrlZoom(vgss, EZOOM_CTRL_STATE) == EZOOM_STATE_DISABLE)
			vgss->ctrlZoom(vgss, EZOOM_CTRL_ENABLE);
		else
			TLOGMSG(1, (DBGINFOFMT "already enabled electronic zoom\n", DBGINFO));
	}
	else
	{
		if (mcu->getShutterPos(mcu) == MCU_SHUTTER_HXMVT)
		{
			guimgr->postNotice(guimgr, GUI_NOTICE_CHANGE_NDFILTER);

			if (mcu->setShutterPos(mcu, MCU_SHUTTER_LXMVT) != 0)
				ret = SYSCTRL_RETCODE_FAIL_SHTR_CTRL;

			guimgr->postNotice(guimgr, GUI_NOTICE_NONE);
		}
		else
			TLOGMSG(1, (DBGINFOFMT "already set lxmvt filter\n", DBGINFO));
	}

	return ret;
}


int
sysctrl_handle_right_keyin(void *arg)
{
	int ret = 0;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct vgss_interface *vgss = mainif->device->vgss;
	struct mcu_interface  *mcu  = mainif->device->mcu;
	struct gui_manager_interface *guimgr = mainif->guimgr;

	if (vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE) == PREVIEW_STATE_RUN)
	{
		if (vgss->ctrlZoom(vgss, EZOOM_CTRL_STATE) == EZOOM_STATE_ENABLE)
			vgss->ctrlZoom(vgss, EZOOM_CTRL_DISABLE);
		else
			TLOGMSG(1, (DBGINFOFMT "already disabled electronic zoom\n", DBGINFO));

	}
	else
	{
		if (mcu->getShutterPos(mcu) == MCU_SHUTTER_LXMVT)
		{
			guimgr->postNotice(guimgr, GUI_NOTICE_CHANGE_NDFILTER);

			if (mcu->setShutterPos(mcu, MCU_SHUTTER_HXMVT) != 0)
			{
				ret = -1;

				/* show failed to shutter contol */
			}

			guimgr->postNotice(guimgr, GUI_NOTICE_NONE);
		}
		else
			TLOGMSG(1, (DBGINFOFMT "already set hxmvt filter\n", DBGINFO));
	}

	return ret;
}


int
sysctrl_toggle_irpol(void *arg)
{
	int ret = 0;
	int irpol = IRCAM_IRPOL_WHITE;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct vgss_interface *vgss = mainif->device->vgss;
	struct ircam_interface *ircam = mainif->device->ircam;

	if (vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE) == PREVIEW_STATE_RUN)
	{
		if (ircam->getColor(ircam) == IRCAM_COLOR_MONO)
		{
			if (ircam->getPolarity(ircam) == IRCAM_IRPOL_WHITE)
				irpol = IRCAM_IRPOL_BLACK;
			else
				irpol = IRCAM_IRPOL_WHITE;

			if (ircam->setPolarity(ircam, irpol) != 0)
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "failed to set ir polarity\n", DBGINFO));
			}
		}
		else
			TLOGMSG(1, ("pseudo color is activated\n"));
	}

	return ret;
}


int
sysctrl_update_nuc(void *arg)
{
	int ret = 0;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct vgss_interface *vgss = mainif->device->vgss;
	struct ircam_interface *ircam = mainif->device->ircam;

	if (vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE) == PREVIEW_STATE_RUN)
	{
		if (ircam->getShtrMode(ircam) == IRCAM_INTERNAL_SHUTTER)
		{
			if (ircam->updateNuc(ircam, IRCAM_INTERNAL_SHUTTER) != 0)
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "failed to update nuc\n", DBGINFO));

				/* show error dialog */
			}
		}
		else
		{
			if (ircam->getNucStatus(ircam) == IRCAM_NUC_READY)
			{
				if (ircam->updateNuc(ircam, IRCAM_EXTERNAL_SHUTTER) != 0)
				{
					ret = -1;
					TLOGMSG(1, (DBGINFOFMT "failed to update nuc\n", DBGINFO));

					/* show error dialog */
				}
			}
			else
			{
				ircam->setNucReady(ircam);
				evque_set_event(EVCODE_CREATE_DIALOG, GUIID_DIALOG_COVER_IRLENS);
			}
		}
	}

	return ret;
}

int
sysctrl_toggle_obmode(void *arg)
{
	int ret = SYSCTRL_RETCODE_CHANGE_OBMODE_SUCCESS;
	int bright = 0;
	struct main_interface    *mainif  = (struct main_interface *) arg;
	struct mcu_interface     *mcu   = mainif->device->mcu;
	struct vgss_interface    *vgss  = mainif->device->vgss;
	struct ircam_interface   *ircam = mainif->device->ircam;
	struct display_interface *disp  = mainif->device->disp;
	struct gui_manager_interface *guimgr = mainif->guimgr;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;
	dialog_t *dialog = NULL;
	dialog_widget_t *widget = NULL;

	switch (vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE))
	{
	case PREVIEW_STATE_RUN:
		guimgr->postNotice(guimgr, GUI_NOTICE_CHANGE_OBMODE);
		vgss->ctrlPreview(vgss, PREVIEW_CTRL_DEINIT);

		dialog = guimgr->getToplevDialog(guimgr);
		widget = dialog->getFocus(dialog);
		if(widget->id == GUIID_BUTTON_SAVE_END)
			ircam->arrayIr(ircam, IRCAM_ARRAY_SAVEEND);

		ircam->enablePower(ircam, false);

		if (mcu->setShutterPos(mcu, MCU_SHUTTER_OPEN) == 0)
		{
			disp->getBright(disp, DISP_BRIGHT_DV, &bright);
			disp->setBright(disp, DISP_BRIGHT_DV, bright);
		}
		else
		{
			ret = SYSCTRL_RETCODE_FAIL_SHTR_CTRL;
			TLOGMSG(1, (DBGINFOFMT "failed to set shutter position\n", DBGINFO));
		}

		guimgr->postNotice(guimgr, GUI_NOTICE_NONE);
		break;

	case PREVIEW_STATE_STOP:
		guimgr->postNotice(guimgr, GUI_NOTICE_CHANGE_OBMODE);
		if (ircam->enablePower(ircam, true) == 0)
		{
			if (mcu->setShutterPos(mcu, MCU_SHUTTER_CLOSE) == 0)
			{
				disp->getBright(disp, DISP_BRIGHT_IR, &bright);
				disp->setBright(disp, DISP_BRIGHT_IR, bright);
				vgss->ctrlPreview(vgss, PREVIEW_CTRL_INIT);
				ircam->updateNuc(ircam, IRCAM_INTERNAL_SHUTTER);
				vgss->ctrlPreview(vgss, PREVIEW_CTRL_RUN);
				ircam->setTecdata(ircam, IRCAM_TECDATA_NORMAL);
				MSLEEP(1500);
				ircam->updateNuc(ircam, IRCAM_INTERNAL_SHUTTER);
				//ircam->set_gainmode(ircam, ircam->get_gainmode(ircam));
				ircam->setHistEq(ircam, ircam->getHistEq(ircam));
				ircam->setPolarity(ircam, ircam->getPolarity(ircam));
				ircam->setColor(ircam, ircam->getColor(ircam));
				ircam->setContrast(ircam, ircam->getContrast(ircam));
				ircam->setBright(ircam, ircam->getBright(ircam));
				ircam->setEdge(ircam, ircam->getEdge(ircam));
				ircam->setGain(ircam, ircam->getGain(ircam));
				ircam->setLevel(ircam, ircam->getLevel(ircam));
				ircam->arrayIr(ircam, IRCAM_ARRAY_START);
			}
			else
			{
				ret = SYSCTRL_RETCODE_FAIL_SHTR_CTRL;
				TLOGMSG(1, (DBGINFOFMT "failed to set shutter position\n", DBGINFO));
			}
		}
		else
		{
			ret = SYSCTRL_RETCODE_FAIL_IRCAM_POWER;
			TLOGMSG(1, (DBGINFOFMT "failed to turn on ircam\n", DBGINFO));
		}
		guimgr->postNotice(guimgr, GUI_NOTICE_NONE);
		break;

	default:
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "invalid preview state\n", DBGINFO));
		break;
	}

	return ret;
}


int
sysctrl_increase_dispbr(void *arg)
{
	int ret = 0;
	int bright = 0;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct vgss_interface *vgss = mainif->device->vgss;
	struct display_interface *disp = mainif->device->disp;

	switch(vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE))
	{
	case PREVIEW_STATE_RUN:
		disp->getBright(disp, DISP_BRIGHT_IR, &bright);
		disp->setBright(disp, DISP_BRIGHT_IR, bright - 1);
		break;

	case PREVIEW_STATE_STOP:
		disp->getBright(disp, DISP_BRIGHT_DV, &bright);
		disp->setBright(disp, DISP_BRIGHT_DV, bright - 1);
		break;

	default:
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "invalid preview state\n", DBGINFO));
		break;
	}

	TLOGMSG(1, ("dispbr = %d\n", bright));

	return ret;
}


int
sysctrl_decrease_dispbr(void *arg)
{
	int ret = 0;
	int bright = 0;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct vgss_interface *vgss = mainif->device->vgss;
	struct display_interface *disp = mainif->device->disp;

	switch(vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE))
	{
	case PREVIEW_STATE_RUN:
		disp->getBright(disp, DISP_BRIGHT_IR, &bright);

		if (disp->setBright(disp, DISP_BRIGHT_IR, bright + 1) != 0)
			ret = SYSCTRL_RETCODE_FAIL_DISP_CTRL;

		break;

	case PREVIEW_STATE_STOP:
		disp->getBright(disp, DISP_BRIGHT_DV, &bright);

		if (disp->setBright(disp, DISP_BRIGHT_DV, bright + 1) != 0)
			ret = SYSCTRL_RETCODE_FAIL_DISP_CTRL;

		break;

	default:
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "invalid preview state\n", DBGINFO));
		break;
	}

	TLOGMSG(1, ("dispbr = %d\n", bright));

	return ret;
}


int
sysctrl_increase_irbr(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;
	struct ircam_interface *ircam = mainif->device->ircam;

 	if (ircam->setBright(ircam, ircam->getBright(ircam) + 1) == 0)
		return SYSCTRL_RETCODE_OK;
	else
		return SYSCTRL_RETCODE_FAIL_IRCAM_CTRL;
}


int
sysctrl_decrease_irbr(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;
	struct ircam_interface *ircam = mainif->device->ircam;

	if (ircam->setBright(ircam, ircam->getBright(ircam) - 1) == 0)
		return SYSCTRL_RETCODE_OK;
	else
		return SYSCTRL_RETCODE_FAIL_IRCAM_CTRL;
}


int
sysctrl_increase_ircont(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;
	struct ircam_interface *ircam = mainif->device->ircam;

	if (ircam->setContrast(ircam, ircam->getContrast(ircam) + 1) == 0)
		return SYSCTRL_RETCODE_OK;
	else
		return SYSCTRL_RETCODE_FAIL_IRCAM_CTRL;
}


int
sysctrl_decrease_ircont(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;
	struct ircam_interface *ircam = mainif->device->ircam;

	if (ircam->setContrast(ircam, ircam->getContrast(ircam) - 1) == 0)
		return SYSCTRL_RETCODE_OK;
	else
		return SYSCTRL_RETCODE_FAIL_IRCAM_CTRL;
}


int
sysctrl_increase_iredge(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;
	struct ircam_interface *ircam = mainif->device->ircam;

	if (ircam->setEdge(ircam, ircam->getEdge(ircam) + 1) == 0)
		return SYSCTRL_RETCODE_OK;
	else
		return SYSCTRL_RETCODE_FAIL_IRCAM_CTRL;
}


int
sysctrl_decrease_iredge(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;
	struct ircam_interface *ircam = mainif->device->ircam;

	if (ircam->setEdge(ircam, ircam->getEdge(ircam) - 1) == 0)
		return SYSCTRL_RETCODE_OK;
	else
		return SYSCTRL_RETCODE_FAIL_IRCAM_CTRL;
}


int
sysctrl_enable_ireis(void *arg)
{
	int ret = SYSCTRL_RETCODE_OK;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct vgss_interface *vgss = mainif->device->vgss;

	if (vgss->ctrlEIS(vgss, EIS_CTRL_STATE) == EIS_STATE_DISABLE)
	{
		vgss->ctrlEIS(vgss, EIS_CTRL_ENABLE);
		TLOGMSG(1, ("enable electronic image stabilizer\n"));
	}
	else
		TLOGMSG(1, ("eis is already enabled\n"));

	return ret;
}


int
sysctrl_disable_ireis(void *arg)
{
	int ret = SYSCTRL_RETCODE_OK;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct vgss_interface *vgss = mainif->device->vgss;

	if (vgss->ctrlEIS(vgss, EIS_CTRL_STATE) == EIS_STATE_ENABLE)
	{
		vgss->ctrlEIS(vgss, EIS_CTRL_EXIT);

		while (vgss->ctrlEIS(vgss, EIS_CTRL_STATE) != EIS_STATE_DISABLE)
			MSLEEP(1);

		TLOGMSG(1, ("enable electronic image stabilizer\n"));
	}
	else
		TLOGMSG(1, ("eis is already enabled\n"));

	return ret;
}


int
sysctrl_set_irclr_mono(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;

	if (mainif->device->ircam->setColor(mainif->device->ircam, IRCAM_COLOR_MONO) == 0)
		return SYSCTRL_RETCODE_OK;
	else
		return SYSCTRL_RETCODE_FAIL_IRCAM_CTRL;
}


int
sysctrl_set_irclr_sepia(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;

	if (mainif->device->ircam->setColor(mainif->device->ircam, IRCAM_COLOR_SEPIA) == 0)
		return SYSCTRL_RETCODE_OK;
	else
		return SYSCTRL_RETCODE_FAIL_IRCAM_CTRL;
}


int
sysctrl_set_irclr_spectrum(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;

	if (mainif->device->ircam->setColor(mainif->device->ircam, IRCAM_COLOR_SPECTRUM) == 0)
		return SYSCTRL_RETCODE_OK;
	else
		return SYSCTRL_RETCODE_FAIL_IRCAM_CTRL;
}


int
sysctrl_set_irclr_isotherm(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;

	if (mainif->device->ircam->setColor(mainif->device->ircam, IRCAM_COLOR_ISOTHREM) == 0)
		return SYSCTRL_RETCODE_OK;
	else
		return SYSCTRL_RETCODE_FAIL_IRCAM_CTRL;
}


int
sysctrl_enable_gheq(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;

	if (mainif->device->ircam->setHistEq(mainif->device->ircam, IRCAM_HISTEQ_GLOBAL) == 0)
		return SYSCTRL_RETCODE_OK;
	else
		return SYSCTRL_RETCODE_FAIL_IRCAM_CTRL;
}


int
sysctrl_enable_lheq(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;

	if (mainif->device->ircam->setHistEq(mainif->device->ircam, IRCAM_HISTEQ_LOCAL) == 0)
		return SYSCTRL_RETCODE_OK;
	else
		return SYSCTRL_RETCODE_FAIL_IRCAM_CTRL;
}


int
sysctrl_increase_irgain(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;
	struct ircam_interface *ircam = mainif->device->ircam;

	if (ircam->setGain(ircam, ircam->getGain(ircam) + 2) == 0)
		return SYSCTRL_RETCODE_OK;
	else
		return SYSCTRL_RETCODE_FAIL_IRCAM_CTRL;
}


int
sysctrl_decrease_irgain(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;
	struct ircam_interface *ircam = mainif->device->ircam;

	if (ircam->setGain(ircam, ircam->getGain(ircam) - 2) == 0)
		return SYSCTRL_RETCODE_OK;
	else
		return SYSCTRL_RETCODE_FAIL_IRCAM_CTRL;
}


int
sysctrl_increase_irlevel(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;
	struct ircam_interface *ircam = mainif->device->ircam;

	if (ircam->setLevel(ircam, ircam->getLevel(ircam) + 2) == 0)
		return SYSCTRL_RETCODE_OK;
	else
		return SYSCTRL_RETCODE_FAIL_IRCAM_CTRL;
}


int
sysctrl_decrease_irlevel(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;
	struct ircam_interface *ircam = mainif->device->ircam;

	if (ircam->setLevel(ircam, ircam->getLevel(ircam) - 2) == 0)
		return SYSCTRL_RETCODE_OK;
	else
		return SYSCTRL_RETCODE_FAIL_IRCAM_CTRL;
}


int
sysctrl_set_coordsys_geodetic(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;
	return mainif->taqmgr->setCoordSystem(mainif->taqmgr, COORDSYS_GEODETIC);
}


int
sysctrl_set_coordsys_utm(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;
	return mainif->taqmgr->setCoordSystem(mainif->taqmgr, COORDSYS_UTM);
}


int
sysctrl_set_coordsys_mgrs(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;
	return mainif->taqmgr->setCoordSystem(mainif->taqmgr, COORDSYS_MGRS);
}


int
sysctrl_set_systime(void *arg)
{
	int ret = SYSCTRL_RETCODE_OK;
	int year = 0;
	int month = 0;
    int utc_offset = 0;
    time_t time_offset = 0;
	dialog_t *dialog = NULL;
	spinbox_t *spinbox = NULL;
	struct tm *tm = NULL;
	struct timespec ts = {.tv_sec = 0, .tv_nsec = 0};
	struct main_interface *mainif = (struct main_interface *)arg;
    struct device_interface *device = mainif->device;
    struct gui_manager_interface  *guimgr = mainif->guimgr;

	dialog = guimgr->getToplevDialog(guimgr);

	if (dialog)
    {
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += get_time_offset();
		tm = localtime(&ts.tv_sec);

        if (tm)
		{
			spinbox = (spinbox_t *) dialog->findWidget(dialog, GUIID_SPINBOX_UTC_OFFSET)->data;
			spinbox->getCurrVal(spinbox, &utc_offset);

			spinbox = (spinbox_t *) dialog->findWidget(dialog, GUIID_SPINBOX_YEAR)->data;
			spinbox->getCurrVal(spinbox, &year);

			spinbox = (spinbox_t *) dialog->findWidget(dialog, GUIID_SPINBOX_MONTH)->data;
			spinbox->getCurrVal(spinbox, &month);

			spinbox = (spinbox_t *) dialog->findWidget(dialog, GUIID_SPINBOX_DAY)->data;
			spinbox->getCurrVal(spinbox, &tm->tm_mday);

			spinbox = (spinbox_t *) dialog->findWidget(dialog, GUIID_SPINBOX_HOUR)->data;
			spinbox->getCurrVal(spinbox, &tm->tm_hour);

			spinbox = (spinbox_t *) dialog->findWidget(dialog, GUIID_SPINBOX_MINUTE)->data;
			spinbox->getCurrVal(spinbox, &tm->tm_min);

			tm->tm_year = year - 1900;
			tm->tm_mon = month - 1;
			time_offset = mktime(tm) - time(NULL);
			set_utc_offset(utc_offset);
			set_time_offset(time_offset);
			device->saveUtcOffset(device, utc_offset);
			TLOGMSG(1, ("system time = %d, compensated time = %d, time offset = %d\n", time(NULL), mktime(tm), time_offset));
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "failed to set system time\n", DBGINFOFMT));
		}
    }
    else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "failed to set system time\n", DBGINFOFMT));
	}

	return ret;
}


int
sysctrl_change_lastday(void *arg)
{
	int ret = 0;
	int year = 0;
	int month = 0;
	spinbox_t *spinbox = NULL;
	dialog_t  *dialog = NULL;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct gui_manager_interface *guimgr = mainif->guimgr;

	dialog = guimgr->getToplevDialog(guimgr);

	if (dialog)
	{
		spinbox = (spinbox_t *) dialog->findWidget(dialog, GUIID_SPINBOX_YEAR)->data;
		spinbox->getCurrVal(spinbox, &year);

		spinbox = (spinbox_t *) dialog->findWidget(dialog, GUIID_SPINBOX_MONTH)->data;
		spinbox->getCurrVal(spinbox, &month);

		spinbox = (spinbox_t *) dialog->findWidget(dialog, GUIID_SPINBOX_DAY)->data;
		spinbox->setMaxValue(spinbox, get_lastday(year, month));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "failed to get toplev dialog\n", DBGINFO));
	}

	return ret;
}


int
sysctrl_enable_evout(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;

	if (mainif->device->vgss->ctrlExtVideo(mainif->device->vgss, EVOUT_CTRL_ENABLE) == 0)
		return SYSCTRL_RETCODE_OK;
	else
		return SYSCTRL_RETCODE_FAIL_EVOUT_CTRL;
}


int
sysctrl_disable_evout(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;

	if (mainif->device->vgss->ctrlExtVideo(mainif->device->vgss, EVOUT_CTRL_DISABLE) == 0)
		return SYSCTRL_RETCODE_OK;
	else
		return SYSCTRL_RETCODE_FAIL_EVOUT_CTRL;
}


int
sysctrl_use_user_obloc(void *arg)
{
	int ret = 0;
	int alt = 0;
	int cval = 0;
	char mgrs[16] = {0};
	dialog_t *dialog = NULL;
	spinbox_t *spinbox = NULL;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;
	struct gui_manager_interface *guimgr = mainif->guimgr;

	int guiid_mgrs[] =
	{
		GUIID_SPINBOX_MGRS_ZONE_DIGIT4,
		GUIID_SPINBOX_MGRS_ZONE_DIGIT3,
		GUIID_SPINBOX_MGRS_ZONE_DIGIT2,
		GUIID_SPINBOX_MGRS_ZONE_DIGIT1,
		GUIID_SPINBOX_MGRS_ZONE_DIGIT0,
		GUIID_SPINBOX_MGRS_EASTING_DIGIT4,
		GUIID_SPINBOX_MGRS_EASTING_DIGIT3,
		GUIID_SPINBOX_MGRS_EASTING_DIGIT2,
		GUIID_SPINBOX_MGRS_EASTING_DIGIT1,
		GUIID_SPINBOX_MGRS_EASTING_DIGIT0,
		GUIID_SPINBOX_MGRS_NORTHING_DIGIT4,
		GUIID_SPINBOX_MGRS_NORTHING_DIGIT3,
		GUIID_SPINBOX_MGRS_NORTHING_DIGIT2,
		GUIID_SPINBOX_MGRS_NORTHING_DIGIT1,
		GUIID_SPINBOX_MGRS_NORTHING_DIGIT0
	};

	int guiid_alt[] =
	{
		GUIID_SPINBOX_ALTITUDE_SIGN,
		GUIID_SPINBOX_ALTITUDE_DIGIT3,
		GUIID_SPINBOX_ALTITUDE_DIGIT2,
		GUIID_SPINBOX_ALTITUDE_DIGIT1,
		GUIID_SPINBOX_ALTITUDE_DIGIT0
	};

	dialog = guimgr->findDialog(guimgr, GUIID_DIALOG_INPUT_OBLOC);

	if (dialog)
	{
		memset(mgrs, 0x00, sizeof(mgrs));

		for (int i = 0; i < DIM(guiid_mgrs); i++)
		{
			spinbox = (spinbox_t *) dialog->findWidget(dialog, guiid_mgrs[i])->data;
			spinbox->getCurrVal(spinbox, &cval);
			mgrs[i] = (char) cval;
		}

		for (int i = 1; i < DIM(guiid_alt); i++)
		{
			spinbox = (spinbox_t *) dialog->findWidget(dialog, guiid_alt[i])->data;
			spinbox->getCurrVal(spinbox, &cval);
			alt = alt + (cval - 0x30) * (int) pow(10.0, 3 - (i - 1));
		}

		spinbox = (spinbox_t *) dialog->findWidget(dialog, guiid_alt[0])->data;
		spinbox->getCurrVal(spinbox, &cval);

		if (cval == 0x2D) alt = -alt;

		TLOGMSG(1, ("user observation location = %s %+05dm\n", mgrs, alt));

		if (taqmgr->setUserOrigin(taqmgr, mgrs, alt) == 0)
			ret = SYSCTRL_RETCODE_USE_USER_OBLOC;
		else
			ret = SYSCTRL_RETCODE_INVALID_OBLOC;
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "failed to find GUIID_DIALOG_INPUT_OBLOC\n", DBGINFO));
	}

	return ret;
}


int
sysctrl_use_auto_obloc(void *arg)
{
	struct main_interface *mainif = (struct main_interface *)arg;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;
	struct gui_manager_interface *guimgr = mainif->guimgr;

	if (taqmgr->setUserOrigin(taqmgr, NULL, 0) == 0)
		return SYSCTRL_RETCODE_USE_AUTO_OBLOC;
	else
		return SYSCTRL_RETCODE_ERROR;
}


int
sysctrl_use_user_range(void *arg)
{
	int ret = 0;
	int cval = 0;
	int range = 0;
	dialog_t *dialog = NULL;
	spinbox_t *spinbox = NULL;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;
	struct gui_manager_interface *guimgr = mainif->guimgr;

	int guiid[] =
	{
		GUIID_SPINBOX_RANGE_DIGIT3,
		GUIID_SPINBOX_RANGE_DIGIT2,
		GUIID_SPINBOX_RANGE_DIGIT1,
		GUIID_SPINBOX_RANGE_DIGIT0
	};


	dialog = guimgr->findDialog(guimgr, GUIID_DIALOG_INPUT_RANGE);

	if (dialog)
	{
		for (int i = 0; i < DIM(guiid); i++)
		{
			spinbox = (spinbox_t *) dialog->findWidget(dialog, guiid[i])->data;
			spinbox->getCurrVal(spinbox, &cval);
			range = range + (cval * pow(10, 3 - i));
		}

		TLOGMSG(1, ("user range : %dm\n", range));

		if (taqmgr->setUserRange(taqmgr, range) == 0)
			ret = SYSCTRL_RETCODE_USE_USER_RANGE;
		else
			ret = SYSCTRL_RETCODE_INVALID_RANGE;
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "failed to find GUIID_DIALOG_INPUT_RANGE\n", DBGINFO));
	}

	return ret;
}


int
sysctrl_use_auto_range(void *arg)
{
	struct main_interface *mainif = (struct main_interface *)arg;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;

	if (taqmgr->setUserRange(taqmgr, -1) == 0)
		return SYSCTRL_RETCODE_USE_AUTO_RANGE;
	else
		return SYSCTRL_RETCODE_ERROR;
}


int
sysctrl_use_user_gridvar(void *arg)
{
	int ret = 0;
	int cval = 0;
	int gv = 0;
	dialog_t *dialog = NULL;
	spinbox_t *spinbox = NULL;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;
	struct gui_manager_interface *guimgr = mainif->guimgr;

	int guiid[] =
	{
		GUIID_SPINBOX_GRIDVAR_SIGN,
		GUIID_SPINBOX_GRIDVAR_DIGIT2,
		GUIID_SPINBOX_GRIDVAR_DIGIT1,
		GUIID_SPINBOX_GRIDVAR_DIGIT0
	};

	dialog = guimgr->findDialog(guimgr, GUIID_DIALOG_INPUT_GRIDVAR);

	if (dialog)
	{
		for (int i = 1; i < DIM(guiid); i++)
		{
			spinbox = (spinbox_t *) dialog->findWidget(dialog, guiid[i])->data;
			spinbox->getCurrVal(spinbox, &cval);
			gv = gv + (cval - 0x30) * pow(10, 2 - (i - 1));
		}

		spinbox = (spinbox_t *) dialog->findWidget(dialog, guiid[0])->data;
		spinbox->getCurrVal(spinbox, &cval);

		if (cval == 0x2D) gv = -gv;

		if ((-800 <= gv) && (800 >= gv))
		{
			taqmgr->setGridVar(taqmgr, MIL2DEG(gv));
			ret = SYSCTRL_RETCODE_USE_USER_GRIDVAR;
		}
		else
			ret = SYSCTRL_RETCODE_INVALID_GRIDVAR;
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "failed to find GUIID_DIALOG_INPUT_RANGE\n", DBGINFO));
	}

	return ret;
}


int
sysctrl_use_auto_gridvar(void *arg)
{
	struct main_interface *mainif = (struct main_interface *)arg;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;

	if (taqmgr->setGridVar(taqmgr, 180.0) == 0)
		return SYSCTRL_RETCODE_USE_AUTO_GRIDVAR;
	else
		return SYSCTRL_RETCODE_ERROR;
}


int
sysctrl_stop_target_acquisition(void *arg)
{
	int ret = 0;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;

	taqmgr->saveResult(taqmgr);

	if (taqmgr->reset(taqmgr) == 0)
	{
		taqmgr->setStatus(taqmgr, TAQMGR_STATUS_IDLE);
		ret = SYSCTRL_RETCODE_STOP_TARGET_ACQUISITION;
	}
	else
		ret = SYSCTRL_RETCODE_ERROR;

	return ret;
}


int
sysctrl_terminate_target_acquisition(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;

	taqmgr->terminate(taqmgr);

	return SYSCTRL_RETCODE_TERMINATE_TARGET_ACQUISITION;
}


int
sysctrl_enter_magcompmode(void *arg)
{
	int ret = SYSCTRL_RETCODE_INIT_MAGCOMP;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct gui_manager_interface *guimgr = mainif->guimgr;
	struct magcomp_interface *magcomp = NULL;

	mainif->magcomp = magcomp_create(mainif->device->dmc);

	if (mainif->magcomp)
	{
		magcomp = mainif->magcomp;
		guimgr->postNotice(guimgr, GUI_NOTICE_INIT_MAGCOMP);
		magcomp->setGeometry(magcomp, MAGCOMP_GEOMETRY_TRIPOD);
		magcomp->startCompensation(magcomp);
		MSLEEP(500);
		guimgr->setMode(guimgr, GUI_MODE_MAGCOMP);
		guimgr->postNotice(guimgr, GUI_NOTICE_NONE);
	}
	else
	{
		ret = SYSCTRL_RETCODE_FAIL_INIT_MAGCOMP;
		TLOGMSG(1, (DBGINFOFMT "failed to init magnetic compensator interface\n"));
	}

	return ret;
}


int
sysctrl_exit_magcompmode(void *arg)
{
	int ret = SYSCTRL_RETCODE_OK;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct gui_manager_interface *guimgr = mainif->guimgr;
	struct magcomp_interface *magcomp = mainif->magcomp;
	struct dmc_interface *dmc = mainif->device->dmc;

	guimgr->postNotice(guimgr, GUI_NOTICE_EXIT_MAGCOMP);
	magcomp->terminateCompensation(magcomp);
	guimgr->setMode(guimgr, GUI_MODE_OBSERVE);
	magcomp_destroy(magcomp);
	dmc->measure(dmc, DMC_CONTINUOUS_MEASURE);
	MSLEEP(500);
	guimgr->postNotice(guimgr, GUI_NOTICE_NONE);

	return ret;
}


int
sysctrl_retry_magcomp(void *arg)
{
	int ret = SYSCTRL_RETCODE_OK;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct gui_manager_interface *guimgr = mainif->guimgr;
	struct magcomp_interface *magcomp = mainif->magcomp;
	//struct dmc_interface *dmc = mainif->device->dmc;

	guimgr->postNotice(guimgr, GUI_NOTICE_INIT_MAGCOMP);
	magcomp->terminateCompensation(magcomp);
	magcomp->setGeometry(magcomp, MAGCOMP_GEOMETRY_TRIPOD);
	magcomp->startCompensation(magcomp);
	MSLEEP(500);
	guimgr->postNotice(guimgr, GUI_NOTICE_NONE);

	return ret;
}


int
sysctrl_apply_magparm(void *arg)
{
	int ret = SYSCTRL_RETCODE_OK;
	//int cell = MCU_CELL_PRIMARY;
	//int pwrsrc = MCU_PWRSRC_CELL;
	int idx = MAGPARM_USER2;

	struct main_interface *mainif = (struct main_interface *) arg;
	struct gui_manager_interface *guimgr = mainif->guimgr;
	struct magcomp_interface *magcomp = mainif->magcomp;
	struct dmc_interface *dmc = mainif->device->dmc;
	//struct mcu_interface *mcu = mainif->device->mcu;
	//struct vgss_interface *vgss = mainif->device->vgss;

	guimgr->postNotice(guimgr, GUI_NOTICE_SAVE_MAGPARM);
	//pwrsrc = mcu->getPwrSrcType(mcu);
	//cell = mcu->getBatteryType(mcu);

	/*
	if (vgss->ctrl_preview(vgss, PREVIEW_CTRL_STATE) == PREVIEW_STATE_STOP)
	{
		if (pwrsrc == MCU_PWRSRC_CELL)
		{
			if (cell == MCU_CELL_PRIMARY)
				index = MAGPARM_DV_PCELL;
			else
				index = MAGPARM_DV_SCELL;
		}
		else
			index = MAGPARM_DV_EXTDC;
	}
	else
	{
		if (pwrsrc == MCU_PWRSRC_CELL)
		{
			if (cell == MCU_CELL_PRIMARY)
				index = MAGPARM_IR_PCELL;
			else
				index = MAGPARM_IR_SCELL;
		}
		else
			index = MAGPARM_IR_EXTDC;
	}
	*/

	dmc->storeMagParm(dmc, idx);
	MSLEEP(500);
	guimgr->postNotice(guimgr, GUI_NOTICE_EXIT_MAGCOMP);
	magcomp->terminateCompensation(magcomp);
	guimgr->setMode(guimgr, GUI_MODE_OBSERVE);
	magcomp_destroy(magcomp);
	dmc->measure(dmc, DMC_CONTINUOUS_MEASURE);
	MSLEEP(500);
	guimgr->postNotice(guimgr, GUI_NOTICE_NONE);

	return ret;
}


int
sysctrl_acquire_magcompdata(void *arg)
{
	int ret = SYSCTRL_RETCODE_OK;
	double fom = 0.0;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct dmc_interface *dmc = mainif->device->dmc;
	struct magcomp_interface *magcomp = mainif->magcomp;
	struct gui_manager_interface *guimgr = mainif->guimgr;

	if (magcomp->acquireCompdata(magcomp) == 0)
	{
		if (magcomp->getCurrentTask(magcomp) == MAGCOMP_TASK_ENDQCP)
		{
			guimgr->postNotice(guimgr, GUI_NOTICE_WAIT_CALC_MAGPARM);

			if (dmc->waitCompParm(dmc) == 0)
			{
				fom = dmc->getFom(dmc) / 10.0;

				if ((0.0 < fom) && (0.6 > fom))
					evque_set_event(EVCODE_CREATE_DIALOG, GUIID_DIALOG_MAGCOMP_OK);
				else
					evque_set_event(EVCODE_CREATE_DIALOG, GUIID_DIALOG_MAGCOMP_ERRFOM);
			}
			else
				evque_set_event(EVCODE_CREATE_DIALOG, GUIID_DIALOG_MAGCOMP_ERRCALC);

			guimgr->postNotice(guimgr, GUI_NOTICE_NONE);
		}
	}
	else
	{
		ret = SYSCTRL_RETCODE_ERROR;
		evque_set_event(EVCODE_CREATE_DIALOG, GUIID_DIALOG_MAGCOMP_ERRDAQ);
		TLOGMSG(1, (DBGINFOFMT "failed to acquire magnetic compensation data\n", DBGINFO));
	}

	return ret;
}


int
sysctrl_xmit_target_information(void *arg)
{
	int ret = 0;
	taqdata_t data = {0};
	struct main_interface *mainif = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;

	taqmgr->getResult(taqmgr, &data);

	if ((data.shift.lateral == 0) && (data.shift.range == 0))
		ret = SYSCTRL_RETCODE_XMIT_TARGET_LOCATION;
	else
		ret = SYSCTRL_RETCODE_SELECT_TARGET_INFO_TO_XMIT;

	return ret;
}


int
sysctrl_xmit_observer_location(void *arg)
{
	int ret = 0;
	taqdata_t data = {0};
	struct main_interface *mainif = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;
	struct gui_manager_interface *guimgr = mainif->guimgr;
	struct msgproc_interface *msgproc = mainif->msgproc;

	guimgr->postNotice(guimgr, GUI_NOTICE_WAIT_RESPONSE);
	memset(&data, 0x00, sizeof(taqdata_t));
	taqmgr->getOrigin(taqmgr, &data.observer.latitude, &data.observer.longitude, &data.observer.altitude);

#ifdef _USE_K05_1_FOR_POSITION_REPORT_
	ret = msgproc->xmitMessage(msgproc, MSGGRP_K05, MSGNUM_POSITION_REPORT, &data);
#else
	ret = msgproc->xmitMessage(msgproc, MSGGRP_K02, MSGNUM_OBREADY_REPORT, &data);
#endif

	switch (ret)
	{
	case MSGPROC_RECV_ACK:
		ret = SYSCTRL_RETCODE_RECV_ACK;
		break;

	case MSGPROC_RECV_NACK_CANTPRO2:
		ret = SYSCTRL_RETCODE_RECV_NACK_CANTPRO2;
		break;

	case MSGPROC_RECV_NACK_CANTPRO15:
		ret = SYSCTRL_RETCODE_RECV_NACK_CANTPRO15;
		break;

	case MSGPROC_RECV_NACK_CANTPRO19:
		ret = SYSCTRL_RETCODE_RECV_NACK_CANTPRO19;
		break;

	case MSGPROC_RECV_NACK_CANTPRO25:
		ret = SYSCTRL_RETCODE_RECV_NACK_CANTPRO25;
		break;

	default:
		ret = SYSCTRL_RETCODE_RESPONSE_TIMEOUT;
		break;
	}

	guimgr->postNotice(guimgr, GUI_NOTICE_NONE);

	return ret;
}


int
sysctrl_xmit_target_location(void *arg)
{
	int ret = 0;
	taqdata_t data = {0};
	struct main_interface *mainif = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;
	struct msgproc_interface *msgproc = mainif->msgproc;
	struct gui_manager_interface *guimgr = mainif->guimgr;

	guimgr->postNotice(guimgr, GUI_NOTICE_WAIT_RESPONSE);
	memset(&data, 0x00, sizeof(taqdata_t));
	taqmgr->getResult(taqmgr, &data);
	ret = msgproc->xmitMessage(msgproc, MSGGRP_K02, MSGNUM_TARGET_DATA, &data);

	switch (ret)
	{
	case MSGPROC_RECV_ACK:
		ret = SYSCTRL_RETCODE_RECV_ACK;
		break;

	case MSGPROC_RECV_NACK_CANTPRO2:
		ret = SYSCTRL_RETCODE_RECV_NACK_CANTPRO2;
		break;

	case MSGPROC_RECV_NACK_CANTPRO15:
		ret = SYSCTRL_RETCODE_RECV_NACK_CANTPRO15;
		break;

	case MSGPROC_RECV_NACK_CANTPRO19:
		ret = SYSCTRL_RETCODE_RECV_NACK_CANTPRO19;
		break;

	case MSGPROC_RECV_NACK_CANTPRO25:
		ret = SYSCTRL_RETCODE_RECV_NACK_CANTPRO25;
		break;

	default:
		ret = SYSCTRL_RETCODE_RESPONSE_TIMEOUT;
		break;
	}

	guimgr->postNotice(guimgr, GUI_NOTICE_NONE);

	return ret;
}


int
sysctrl_xmit_target_shift(void *arg)
{
	int ret = 0;
	taqdata_t data = {0};
	struct main_interface *mainif = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;
	struct msgproc_interface *msgproc = mainif->msgproc;
	struct gui_manager_interface *guimgr = mainif->guimgr;

	guimgr->postNotice(guimgr, GUI_NOTICE_WAIT_RESPONSE);
	memset(&data, 0x00, sizeof(taqdata_t));
	taqmgr->getResult(taqmgr, &data);
	ret = msgproc->xmitMessage(msgproc, MSGGRP_K02, MSGNUM_SUBSEQUENT_ADJUST, &data);

	switch (ret)
	{
		case MSGPROC_RECV_ACK:
			ret = SYSCTRL_RETCODE_RECV_ACK;
			break;

		case MSGPROC_RECV_NACK_CANTPRO2:
			ret = SYSCTRL_RETCODE_RECV_NACK_CANTPRO2;
			break;

		case MSGPROC_RECV_NACK_CANTPRO15:
			ret = SYSCTRL_RETCODE_RECV_NACK_CANTPRO15;
			break;

		case MSGPROC_RECV_NACK_CANTPRO19:
			ret = SYSCTRL_RETCODE_RECV_NACK_CANTPRO19;
			break;

		case MSGPROC_RECV_NACK_CANTPRO25:
			ret = SYSCTRL_RETCODE_RECV_NACK_CANTPRO25;
			break;

		default:
			ret = SYSCTRL_RETCODE_RESPONSE_TIMEOUT;
			break;
	}

	guimgr->postNotice(guimgr, GUI_NOTICE_NONE);

	return ret;
}


int
sysctrl_xmit_target_infomation_at_list(void *arg)
{
    int ret = 0;
    taqdata_t *data = NULL;
    taqdata_manager_t *tlist = NULL;
    struct main_interface *mainif = (struct main_interface *) arg;
    struct taqmgr_interface *taqmgr = mainif->taqmgr;

    tlist = mainif->taqmgr->getTargetList(mainif->taqmgr);
    data = tlist->getFocus(tlist);

    if (data)
    {
        if ((data->shift.lateral == 0) && (data->shift.range == 0))
            ret = 0;
        else
            ret = 1;
    }
    else
    {
        ret = -1;
        TLOGMSG(1, ("null taqdata\n"));
    }

    return ret;
}


int
sysctrl_xmit_target_location_at_list(void *arg)
{
    int ret = 0;
    taqdata_t *data = NULL;
    taqdata_manager_t *tlist = NULL;
    struct main_interface *mainif = (struct main_interface *) arg;
    struct msgproc_interface *msgproc = mainif->msgproc;
    struct gui_manager_interface *guimgr = mainif->guimgr;

    guimgr->postNotice(guimgr, GUI_NOTICE_WAIT_RESPONSE);
    tlist = mainif->taqmgr->getTargetList(mainif->taqmgr);
    data = tlist->getFocus(tlist);
    ret = msgproc->xmitMessage(msgproc, MSGGRP_K02, MSGNUM_TARGET_DATA, data);

    switch (ret)
    {
        case MSGPROC_RECV_ACK:
            ret = SYSCTRL_RETCODE_RECV_ACK;
            break;

        case MSGPROC_RECV_NACK_CANTPRO2:
            ret = SYSCTRL_RETCODE_RECV_NACK_CANTPRO2;
            break;

        case MSGPROC_RECV_NACK_CANTPRO15:
            ret = SYSCTRL_RETCODE_RECV_NACK_CANTPRO15;
            break;

        case MSGPROC_RECV_NACK_CANTPRO19:
            ret = SYSCTRL_RETCODE_RECV_NACK_CANTPRO19;
            break;

        case MSGPROC_RECV_NACK_CANTPRO25:
            ret = SYSCTRL_RETCODE_RECV_NACK_CANTPRO25;
            break;

        default:
            ret = SYSCTRL_RETCODE_RESPONSE_TIMEOUT;
            break;
    }

    guimgr->postNotice(guimgr, GUI_NOTICE_NONE);

    return ret;
}


int
sysctrl_xmit_target_shift_at_list(void *arg)
{
    int ret = 0;
    taqdata_t *data = NULL;
    taqdata_manager_t *tlist = NULL;
    struct main_interface *mainif = (struct main_interface *) arg;
    struct msgproc_interface *msgproc = mainif->msgproc;
    struct gui_manager_interface *guimgr = mainif->guimgr;

    guimgr->postNotice(guimgr, GUI_NOTICE_WAIT_RESPONSE);
    tlist = mainif->taqmgr->getTargetList(mainif->taqmgr);
    data = tlist->getFocus(tlist);
    ret = msgproc->xmitMessage(msgproc, MSGGRP_K02, MSGNUM_SUBSEQUENT_ADJUST, data);

    switch (ret)
    {
        case MSGPROC_RECV_ACK:
            ret = SYSCTRL_RETCODE_RECV_ACK;
            break;

        case MSGPROC_RECV_NACK_CANTPRO2:
            ret = SYSCTRL_RETCODE_RECV_NACK_CANTPRO2;
            break;

        case MSGPROC_RECV_NACK_CANTPRO15:
            ret = SYSCTRL_RETCODE_RECV_NACK_CANTPRO15;
            break;

        case MSGPROC_RECV_NACK_CANTPRO19:
            ret = SYSCTRL_RETCODE_RECV_NACK_CANTPRO19;
            break;

        case MSGPROC_RECV_NACK_CANTPRO25:
            ret = SYSCTRL_RETCODE_RECV_NACK_CANTPRO25;
            break;

        default:
            ret = SYSCTRL_RETCODE_RESPONSE_TIMEOUT;
            break;
    }

    guimgr->postNotice(guimgr, GUI_NOTICE_NONE);

    return ret;
}


int
sysctrl_test_dmif(void *arg)
{
	int ret = 0;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct msgproc_interface *msgproc = mainif->msgproc;
	struct gui_manager_interface *guimgr = mainif->guimgr;

	guimgr->postNotice(guimgr, GUI_NOTICE_DMIF_TEST);
	ret = msgproc->xmitMessage(msgproc, MSGGRP_K00, MSGNUM_NETWORK_MONITOR, NULL);

	switch (ret)
	{
	case MSGPROC_RECV_ACK:
		ret = SYSCTRL_RETCODE_RECV_ACK;
		break;

	case MSGPROC_RECV_NACK_CANTPRO2:
		ret = SYSCTRL_RETCODE_RECV_NACK_CANTPRO2;
		break;

	case MSGPROC_RECV_NACK_CANTPRO15:
		ret = SYSCTRL_RETCODE_RECV_NACK_CANTPRO15;
		break;

	case MSGPROC_RECV_NACK_CANTPRO19:
		ret = SYSCTRL_RETCODE_RECV_NACK_CANTPRO19;
		break;

	case MSGPROC_RECV_NACK_CANTPRO25:
		ret = SYSCTRL_RETCODE_RECV_NACK_CANTPRO25;
		break;

	default:
		ret = SYSCTRL_RETCODE_RESPONSE_TIMEOUT;
		break;
	}

	guimgr->postNotice(guimgr, GUI_NOTICE_NONE);

	return ret;
}


int
sysctrl_set_taqmode_circular_target(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;

	taqmgr->setStatus(taqmgr, TAQMGR_STATUS_INPROC);
	taqmgr->setTaqMode(taqmgr, TAQMODE_CIRCULAR_TARGET);

	return SYSCTRL_RETCODE_SET_TAQMODE_CIRCULAR_TARGET;
}


int
sysctrl_set_taqmode_square_target(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;

	taqmgr->setStatus(taqmgr, TAQMGR_STATUS_INPROC);
	taqmgr->setTaqMode(taqmgr, TAQMODE_SQUARE_TARGET_LENGTH);

	return SYSCTRL_RETCODE_SET_TAQMODE_SQAURE_TARGET;
}


int
sysctrl_set_taqmode_fos_correction(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;

	taqmgr->setStatus(taqmgr, TAQMGR_STATUS_INPROC);
	taqmgr->setTaqMode(taqmgr, TAQMODE_FOS_CORRECTION);

	return SYSCTRL_RETCODE_SET_TAQMODE_FOS_CORRECTION;
}


int
sysctrl_enter_standby_mode(void *arg)
{
    int bright = 0;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct device_interface *device = mainif->device;
	struct gui_manager_interface *guimgr = mainif->guimgr;
    struct mcu_interface *mcu = device->mcu;
    struct vgss_interface *vgss = device->vgss;
    struct ircam_interface *ircam = device->ircam;
    struct display_interface *disp = device->disp;

    guimgr->postNotice(guimgr, GUI_NOTICE_ENTER_STANDBY_MODE);

    if (vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE) == PREVIEW_STATE_RUN)
    {
        vgss->ctrlPreview(vgss, PREVIEW_CTRL_DEINIT);
        ircam->enablePower(ircam, false);
        mcu->setShutterPos(mcu, MCU_SHUTTER_OPEN);
        disp->getBright(disp, DISP_BRIGHT_DV, &bright);
        disp->setBright(disp, DISP_BRIGHT_DV, bright);
    }

	device->standby(device);
	guimgr->postNotice(guimgr, GUI_NOTICE_NONE);
	device->suspend(device);
	MSLEEP(1000);

	return 0;
}


int
sysctrl_exit_standby_mode(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;
	struct device_interface *device = mainif->device;
	struct gui_manager_interface *guimgr = mainif->guimgr;

	guimgr->postNotice(guimgr, GUI_NOTICE_EXIT_STANDBY_MODE);
	device->wakeup(device);
	guimgr->postNotice(guimgr, GUI_NOTICE_NONE);
	guimgr->unlockKeyin(guimgr);

	return 0;

}


int
sysctrl_execute_ibit(void *arg)
{
	int ret = 0;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct device_interface *device = mainif->device;
	struct msgproc_interface *msgproc = mainif->msgproc;

	msgproc->resetTest(msgproc);

	if (device->executeBit(device) == 0)
		ret = SYSCTRL_RETCODE_IBIT_PASS;
	else
		ret = SYSCTRL_RETCODE_IBIT_FAIL;

	msgproc->testInterface(msgproc);

	return ret;
}


int
sysctrl_enable_auto_xmit(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;
	struct msgproc_interface *msgproc = mainif->msgproc;

	return msgproc->enableAutoXmit(msgproc, MSGPROC_AUTO_XMIT_ON);
}


int
sysctrl_disable_auto_xmit(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;
	struct msgproc_interface *msgproc = mainif->msgproc;

	return msgproc->enableAutoXmit(msgproc, MSGPROC_AUTO_XMIT_OFF);
}


int
sysctrl_select_internal_shutter(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;
	struct ircam_interface *ircam = mainif->device->ircam;

	return ircam->setShtrMode(ircam, IRCAM_INTERNAL_SHUTTER);
}


int
sysctrl_select_external_shutter(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;
	struct ircam_interface *ircam = mainif->device->ircam;

	return ircam->setShtrMode(ircam, IRCAM_EXTERNAL_SHUTTER);
}


int
sysctrl_check_target_list(void *arg)
{
	int ret = 0;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct taqdata_manager_interface *tlist = mainif->taqmgr->getTargetList(mainif->taqmgr);

	if (tlist->getNumData(tlist) != 0)
		ret = SYSCTRL_RETCODE_EXEC_TARGET_LIST_FUNC;
	else
		ret = SYSCTRL_RETCODE_EMPTY_TARGET_LIST;

	return ret;
}


int
sysctrl_delete_target(void *arg)
{
	struct main_interface *mainif = (struct main_interface *) arg;
	struct taqdata_manager_interface *tlist = mainif->taqmgr->getTargetList(mainif->taqmgr);

	tlist->removeData(tlist);

	return SYSCTRL_RETCODE_TARGET_DELETED;
}

int
sysctrl_save_short_dist_correct(void *arg)
{
	int ret = 0;
	char dist_a[16] = {0};
	char dist_b[16] = {0};
	int cval_a = 0;
	int cval_b = 0;
	dialog_t *dialog = NULL;
	spinbox_t *spinbox = NULL;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;
	struct gui_manager_interface *guimgr = mainif->guimgr;

	int A_guiid[] =
	{
			GUIID_SPINBOX_DIST_SIGN_A,
			GUIID_SPINBOX_DIST_CORRECT_DIGITA0,
			GUIID_SPINBOX_DIST_CORRECT_DIGITA1,
			GUIID_SPINBOX_DIST_CORRECT_DIGITA2,
			GUIID_SPINBOX_DIST_CORRECT_DIGITA3,
			GUIID_SPINBOX_DIST_CORRECT_DIGITA4,
			GUIID_SPINBOX_DIST_CORRECT_DIGITA5,
			GUIID_SPINBOX_DIST_CORRECT_DIGITA6
	};

	int B_guiid[] =
	{
			GUIID_SPINBOX_DIST_SIGN_B,
			GUIID_SPINBOX_DIST_CORRECT_DIGITB0,
			GUIID_SPINBOX_DIST_CORRECT_DIGITB1,
			GUIID_SPINBOX_DIST_CORRECT_DIGITB2,
			GUIID_SPINBOX_DIST_CORRECT_DIGITB3,
			GUIID_SPINBOX_DIST_CORRECT_DIGITB4,
			GUIID_SPINBOX_DIST_CORRECT_DIGITB5,
			GUIID_SPINBOX_DIST_CORRECT_DIGITB6
	};

	dialog = guimgr->findDialog(guimgr, GUIID_DIALOG_SHORTDIST);

	if (dialog)
	{
		memset(dist_a, 0x00, sizeof(dist_a));
		memset(dist_b, 0x00, sizeof(dist_b));
		for (int i = 0; i < DIM(A_guiid); i++)
		{
			spinbox = (spinbox_t *) dialog->findWidget(dialog, A_guiid[i])->data;
			spinbox->getCurrVal(spinbox, &cval_a);
			if(i < 3)dist_a[i] = (char)cval_a;
			else dist_a[i+1] = (char)cval_a;
		}
		dist_a[3] = 0x2E;
		for (int i = 0; i < DIM(B_guiid); i++)
		{
			spinbox = (spinbox_t *) dialog->findWidget(dialog, B_guiid[i])->data;
			spinbox->getCurrVal(spinbox, &cval_b);
			if(i < 3)dist_b[i] = (char)cval_b;
			else dist_b[i+1] = (char)cval_b;
		}
		dist_b[3] = 0x2E;

		TLOGMSG(1, ("short distance correction value A = %s, B = %s\n", dist_a, dist_b));
		taqmgr->setDistOffsetSH(taqmgr, dist_a, dist_b);
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "failed to find GUIID_DIALOG_SHORTDIST\n", DBGINFO));
	}

	return ret;
}

int
sysctrl_save_middle_dist_correct(void *arg)
{
	int ret = 0;
	char dist_a[16] = {0};
	char dist_b[16] = {0};
	int cval_a = 0;
	int cval_b = 0;
	dialog_t *dialog = NULL;
	spinbox_t *spinbox = NULL;
	FILE *pfile = NULL;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;
	struct gui_manager_interface *guimgr = mainif->guimgr;

	int A_guiid[] =
	{
			GUIID_SPINBOX_DIST_SIGN_A,
			GUIID_SPINBOX_DIST_CORRECT_DIGITA0,
			GUIID_SPINBOX_DIST_CORRECT_DIGITA1,
			GUIID_SPINBOX_DIST_CORRECT_DIGITA2,
			GUIID_SPINBOX_DIST_CORRECT_DIGITA3,
			GUIID_SPINBOX_DIST_CORRECT_DIGITA4,
			GUIID_SPINBOX_DIST_CORRECT_DIGITA5,
			GUIID_SPINBOX_DIST_CORRECT_DIGITA6
	};

	int B_guiid[] =
	{
			GUIID_SPINBOX_DIST_SIGN_B,
			GUIID_SPINBOX_DIST_CORRECT_DIGITB0,
			GUIID_SPINBOX_DIST_CORRECT_DIGITB1,
			GUIID_SPINBOX_DIST_CORRECT_DIGITB2,
			GUIID_SPINBOX_DIST_CORRECT_DIGITB3,
			GUIID_SPINBOX_DIST_CORRECT_DIGITB4,
			GUIID_SPINBOX_DIST_CORRECT_DIGITB5,
			GUIID_SPINBOX_DIST_CORRECT_DIGITB6
	};

	dialog = guimgr->findDialog(guimgr, GUIID_DIALOG_MIDDIST);

	if (dialog)
	{
		memset(dist_a, 0x00, sizeof(dist_a));
		memset(dist_b, 0x00, sizeof(dist_b));
		for (int i = 0; i < DIM(A_guiid); i++)
		{
			spinbox = (spinbox_t *) dialog->findWidget(dialog, A_guiid[i])->data;
			spinbox->getCurrVal(spinbox, &cval_a);
			if(i < 3)dist_a[i] = (char)cval_a;
			else dist_a[i+1] = (char)cval_a;
		}
		dist_a[3] = 0x2E;
		for (int i = 0; i < DIM(B_guiid); i++)
		{
			spinbox = (spinbox_t *) dialog->findWidget(dialog, B_guiid[i])->data;
			spinbox->getCurrVal(spinbox, &cval_b);
			if(i < 3)dist_b[i] = (char)cval_b;
			else dist_b[i+1] = (char)cval_b;
		}
		dist_b[3] = 0x2E;

		TLOGMSG(1, ("middle distance correction value A = %s, B = %s\n", dist_a, dist_b));
		taqmgr->setDistOffsetMD(taqmgr, dist_a, dist_b);
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "failed to find GUIID_DIALOG_MIDDIST\n", DBGINFO));
	}

	return ret;
}


int
sysctrl_save_long_dist_correct(void *arg)
{
	int ret = 0;
	char dist_a[16] = {0};
	char dist_b[16] = {0};
	int cval_a = 0;
	int cval_b = 0;
	dialog_t *dialog = NULL;
	spinbox_t *spinbox = NULL;
	FILE *pfile = NULL;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;
	struct gui_manager_interface *guimgr = mainif->guimgr;

	int A_guiid[] =
	{
			GUIID_SPINBOX_DIST_SIGN_A,
			GUIID_SPINBOX_DIST_CORRECT_DIGITA0,
			GUIID_SPINBOX_DIST_CORRECT_DIGITA1,
			GUIID_SPINBOX_DIST_CORRECT_DIGITA2,
			GUIID_SPINBOX_DIST_CORRECT_DIGITA3,
			GUIID_SPINBOX_DIST_CORRECT_DIGITA4,
			GUIID_SPINBOX_DIST_CORRECT_DIGITA5,
			GUIID_SPINBOX_DIST_CORRECT_DIGITA6
	};

	int B_guiid[] =
	{
			GUIID_SPINBOX_DIST_SIGN_B,
			GUIID_SPINBOX_DIST_CORRECT_DIGITB0,
			GUIID_SPINBOX_DIST_CORRECT_DIGITB1,
			GUIID_SPINBOX_DIST_CORRECT_DIGITB2,
			GUIID_SPINBOX_DIST_CORRECT_DIGITB3,
			GUIID_SPINBOX_DIST_CORRECT_DIGITB4,
			GUIID_SPINBOX_DIST_CORRECT_DIGITB5,
			GUIID_SPINBOX_DIST_CORRECT_DIGITB6
	};

	dialog = guimgr->findDialog(guimgr, GUIID_DIALOG_LONGDIST);

	if (dialog)
	{
		memset(dist_a, 0x00, sizeof(dist_a));
		memset(dist_b, 0x00, sizeof(dist_b));
		for (int i = 0; i < DIM(A_guiid); i++)
		{
			spinbox = (spinbox_t *) dialog->findWidget(dialog, A_guiid[i])->data;
			spinbox->getCurrVal(spinbox, &cval_a);
			if(i < 3)dist_a[i] = (char)cval_a;
			else dist_a[i+1] = (char)cval_a;
		}
		dist_a[3] = 0x2E;
		for (int i = 0; i < DIM(B_guiid); i++)
		{
			spinbox = (spinbox_t *) dialog->findWidget(dialog, B_guiid[i])->data;
			spinbox->getCurrVal(spinbox, &cval_b);
			if(i < 3)dist_b[i] = (char)cval_b;
			else dist_b[i+1] = (char)cval_b;
		}
		dist_b[3] = 0x2E;

		TLOGMSG(1, ("long distance correction value A = %s, B = %s\n", dist_a, dist_b));
		taqmgr->setDistOffsetLG(taqmgr, dist_a, dist_b);
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "failed to find GUIID_DIALOG_LONGDIST\n", DBGINFO));
	}

	return ret;
}


int
sysctrl_save_azimuth_correct(void *arg)
{
	int ret = 0;
	char azm[16] = {0};
	int cval = 0;
	dialog_t *dialog = NULL;
	spinbox_t *spinbox = NULL;
	FILE *pfile = NULL;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;
	struct gui_manager_interface *guimgr = mainif->guimgr;

	int guiid[] =
	{
			GUIID_SPINBOX_AZIMUTH_SIGN,
			GUIID_SPINBOX_AZIMUTH_DIGIT0,
			GUIID_SPINBOX_AZIMUTH_DIGIT1,
			GUIID_SPINBOX_AZIMUTH_DIGIT2,
			GUIID_SPINBOX_AZIMUTH_DIGIT3,
			GUIID_SPINBOX_AZIMUTH_DIGIT4,
	};

	dialog = guimgr->findDialog(guimgr, GUIID_DIALOG_AZM);

	if (dialog)
	{
		memset(azm, 0x00, sizeof(azm));
		for (int i = 0; i < DIM(guiid); i++)
		{
			spinbox = (spinbox_t *) dialog->findWidget(dialog, guiid[i])->data;
			spinbox->getCurrVal(spinbox, &cval);
			if(i < 3) azm[i] = (char)cval;
			else azm[i+1] = (char)cval;
		}
		azm[3] = 0x2E;

		TLOGMSG(1, ("azimuth correction value A = %s\n", azm));
		taqmgr->setAngleOffsetAZ(taqmgr, azm);
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "failed to find GUIID_DIALOG_AZM\n", DBGINFO));
	}

	return ret;
}

int
sysctrl_save_altitude_correct(void *arg)
{
	int ret = 0;
	char alt[16] = {0};
	int cval = 0;
	dialog_t *dialog = NULL;
	spinbox_t *spinbox = NULL;
	FILE *pfile = NULL;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;
	struct gui_manager_interface *guimgr = mainif->guimgr;

	int guiid[] =
	{
			GUIID_SPINBOX_ALTITUDE_SIGN,
			GUIID_SPINBOX_ALTITUDE_DIGIT0,
			GUIID_SPINBOX_ALTITUDE_DIGIT1,
			GUIID_SPINBOX_ALTITUDE_DIGIT2,
			GUIID_SPINBOX_ALTITUDE_DIGIT3,
			GUIID_SPINBOX_ALTITUDE_DIGIT4,
	};

	dialog = guimgr->findDialog(guimgr, GUIID_DIALOG_ALTT);

	if (dialog)
	{
		memset(alt, 0x00, sizeof(alt));
		for (int i = 0; i < DIM(guiid); i++)
		{
			spinbox = (spinbox_t *) dialog->findWidget(dialog, guiid[i])->data;
			spinbox->getCurrVal(spinbox, &cval);
			if(i < 3) alt[i] = (char)cval;
			else alt[i+1] = (char)cval;
		}
		alt[3] = 0x2E;

		TLOGMSG(1, ("altitude correction value A = %s\n", alt));
		taqmgr->setAngleOffsetAT(taqmgr, alt);
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "failed to find GUIID_DIALOG_ALTT\n", DBGINFO));
	}

	return ret;
}


int
sysctrl_save_sensor_correct(void *arg)
{
	int ret = 0;
	char sensor_a[16] = {0};
	char sensor_b[16] = {0};
	int cval_a = 0;
	int cval_b = 0;
	dialog_t *dialog = NULL;
	spinbox_t *spinbox = NULL;
	struct main_interface *mainif = (struct  main_interface *)arg;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;
	struct gui_manager_interface *guimgr = mainif->guimgr;

	int A_guiid[] =
	{
			GUIID_SPINBOX_SENSOR_DIGITA0,
			GUIID_SPINBOX_SENSOR_DIGITA1,
			GUIID_SPINBOX_SENSOR_DIGITA2
	};

	int B_guiid[] =
	{
			GUIID_SPINBOX_SENSOR_DIGITB0,
			GUIID_SPINBOX_SENSOR_DIGITB1,
			GUIID_SPINBOX_SENSOR_DIGITB2
	};

	dialog = guimgr->findDialog(guimgr, GUIID_DIALOG_SENSOR);

	if(dialog)
	{
		memset(sensor_a, 0x00, sizeof(sensor_a));
		memset(sensor_b, 0x00, sizeof(sensor_b));
		for(int i = 0; i < DIM(A_guiid); i++)
		{
			spinbox = (spinbox_t *)dialog->findWidget(dialog, A_guiid[i])->data;
			spinbox->getCurrVal(spinbox, &cval_a);
			sensor_a[i] = (char)cval_a;
		}

		for(int i = 0; i < DIM(B_guiid); i++)
		{
			spinbox = (spinbox_t *)dialog->findWidget(dialog, B_guiid[i])->data;
			spinbox->getCurrVal(spinbox, &cval_b);
			sensor_b[i] = (char)cval_b;
		}
		TLOGMSG(1, ("sensor correction value A = %s, B = %s\n", sensor_a, sensor_b));
		taqmgr->setSensorOffset(taqmgr, sensor_a, sensor_b);

	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "failed to find GUIID_DIALOG_SHORTDIST\n", DBGINFO));
	}

	return ret;
}


int
sysctrl_init_correct(void *arg)
{
	int ret = 0;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct taqmgr_interface *taqmgr = mainif->taqmgr;

	devconf_reset_parameters();
	ret = devconf_save_parameters(DEVCONF_FILE_PATH);
	if(ret == 0)
		devconf_enumerate_parameters();
	else
		ret = -1;

	return ret;
}

int
sysctrl_firmware_update(void *arg)
{
	int ret = SYSCTRL_RETCODE_OK;
	struct main_interface *mainif = (struct main_interface *)arg;
	struct sdcard_interface *sdc = mainif->device->sdc;
	struct ispmgr_interface *ispmgr = mainif->device->ispmgr;

	ispmgr->setUpdateTask(ispmgr, ISPMGR_STANDBY_PROGRAMMING);

	if(sdc->getMount(sdc) == -1)
	{
		ret 	= SYSCTRL_RETCODE_FWUPDATE_NOSDCARD;
		goto error_exit;
	}

	ispmgr->setImages(ispmgr, ISPMGR_UBOOT, ISPMGR_DEFAULT_UBOOT_PATH, FILE_OFFSET_UBOOT);
	ispmgr->setImages(ispmgr, ISPMGR_UIMAGE, ISPMGR_DEFAULT_UIMAGE_PATH, FILE_OFFSET_UIMAGE);
	ispmgr->setImages(ispmgr, ISPMGR_ROOTFS, ISPMGR_DEFAULT_ROOTFS_PATH, FILE_OFFSET_ROOTFS);
	ispmgr->setBlkdev(ispmgr, ISPMGR_UBOOT, BLKDEV_PATH_UBOOT, BLKDEV_OFFSET_UBOOT);
	ispmgr->setBlkdev(ispmgr, ISPMGR_UIMAGE, BLKDEV_PATH_UIMAGE, BLKDEV_OFFSET_UIMAGE);
	ispmgr->setBlkdev(ispmgr, ISPMGR_ROOTFS, BLKDEV_PATH_ROOTFS, BLKDEV_OFFSET_ROOTFS);

	if(ispmgr->getImagesValiable(ispmgr) == -1)
	{
		ret = SYSCTRL_RETCODE_FWUPDATE_NOFILE;
		goto error_exit;
	}

	if(ispmgr->startProgramming(ispmgr) != 0)
	{
		ret = SYSCTRL_RETCODE_FWUPDATE_FAIL;
		goto error_exit;
	}

	ispmgr->setUpdateTask(ispmgr, ISPMGR_STANDBY_PROGRAMMING);

error_exit:

	return ret;
}


int
sysctrl_move_ir_left(void *arg)
{
	int ret = 0;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct ircam_interface *ircam = mainif->device->ircam;

	if(ircam->moveIr(ircam, IRCAM_MOVE_LEFT) == 0)
	{
		ret = SYSCTRL_RETCODE_SUCCESS_IRMOVE;
		TLOGMSG(1, (DBGINFOFMT "success: ir move left\n", DBGINFO));
	}
	else
	{
		ret = SYSCTRL_RETCODE_FAIL_IRMOVE;
		TLOGMSG(1, (DBGINFOFMT "fail: ir move left\n", DBGINFO));
	}

	return ret;
}

int
sysctrl_move_ir_right(void *arg)
{
	int ret = 0;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct ircam_interface *ircam = mainif->device->ircam;

	if(ircam->moveIr(ircam, IRCAM_MOVE_RIGHT) == 0)
	{
		ret = SYSCTRL_RETCODE_SUCCESS_IRMOVE;
		TLOGMSG(1, (DBGINFOFMT "success: ir move right\n", DBGINFO));
	}
	else
	{
		ret = SYSCTRL_RETCODE_FAIL_IRMOVE;
		TLOGMSG(1, (DBGINFOFMT "fail: ir move right\n", DBGINFO));
	}

	return ret;
}

int
sysctrl_move_ir_up(void *arg)
{
	int ret = 0;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct ircam_interface *ircam = mainif->device->ircam;

	if(ircam->moveIr(ircam, IRCAM_MOVE_UP) == 0)
	{
		ret = SYSCTRL_RETCODE_SUCCESS_IRMOVE;
		TLOGMSG(1, (DBGINFOFMT "success: ir move up\n", DBGINFO));
	}
	else
	{
		ret = SYSCTRL_RETCODE_FAIL_IRMOVE;
		TLOGMSG(1, (DBGINFOFMT "fail: ir move up\n", DBGINFO));
	}

	return ret;
}

int
sysctrl_move_ir_down(void *arg)
{
	int ret = 0;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct ircam_interface *ircam = mainif->device->ircam;

	if(ircam->moveIr(ircam, IRCAM_MOVE_DOWN) == 0)
	{
		ret = SYSCTRL_RETCODE_SUCCESS_IRMOVE;
		TLOGMSG(1, (DBGINFOFMT "success: ir move down\n", DBGINFO));
	}
	else
	{
		ret = SYSCTRL_RETCODE_FAIL_IRMOVE;
		TLOGMSG(1, (DBGINFOFMT "fail: ir move down\n", DBGINFO));
	}

	return ret;
}


int
sysctrl_irarray_end(void *arg)
{
	int ret = 0;
	struct main_interface *mainif = (struct main_interface *) arg;
	struct gui_manager_interface *guimgr = mainif->guimgr;

	guimgr->showDialog(guimgr, GUIID_DIALOG_IRARRAY, arg);

	return ret;
}
