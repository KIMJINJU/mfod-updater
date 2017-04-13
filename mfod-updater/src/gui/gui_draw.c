/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_draw.c
        external/internal function implementations of gui draw handler interface
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#include <directfb.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <amod.h>

#include "types.h"
#include "core/logger.h"
#include "etc/util.h"
#include "gui/gui_draw.h"
#include "gui/gui_str.h"

/* global variables */
static int font_height = 0;


static void
set_font(IDirectFBSurface *surf, IDirectFBFont *font)
{
    font->GetHeight(font, &font_height);
    surf->SetFont(surf, font);
}


static void
set_draw_color(IDirectFBSurface *surf, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    surf->SetColor(surf, r, g, b, a);
}


static void
draw_rectangle(IDirectFBSurface *surf, rect_t *rt, bool bold)
{
    if (bold)
    {
        for (int i = 0; i < 2; i++)
            surf->DrawRectangle(surf, rt->x + i, rt->y + i, rt->w - (2 * i), rt->h - (2 * i));
    }
    else
        surf->DrawRectangle(surf, rt->x, rt->y, rt->w, rt->h);
}


static void
draw_fill_rectangle(IDirectFBSurface *surf, rect_t *rt)
{
    surf->FillRectangle(surf, rt->x, rt->y, rt->w, rt->h);
}


static void
draw_text(IDirectFBSurface *surf, char *text, int count, rect_t *rt, DFBSurfaceTextFlags flag)
{
    int x = 0;
    int y = 0;

    switch (flag)
    {
    case DSTF_CENTER:
        x = rt->x + (rt->w / 2);
        break;

    case DSTF_LEFT:
        x = rt->x;
        break;

    case DSTF_RIGHT:
        x = rt->x + rt->w;
        break;

    default:
        x = rt->x;
        break;
    }

    y = rt->y + rt->h / 2 + font_height / 3;
    surf->DrawString(surf, text, count, x, y, flag);
}


static void
draw_indicator_background(window_t *window, struct vgss_interface *vgss, rect_t *rt, int color)
{
    IDirectFBSurface *surface = window->guiSurface;

    if (vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE) == PREVIEW_STATE_RUN)
    {
        if (color)
            surface->Blit(surface, window->imageSurface[IMAGE_ID_RED_INDICATOR_IR], NULL, rt->x, rt->y + 1);
        else
            surface->Blit(surface, window->imageSurface[IMAGE_ID_GREEN_INDICATOR_IR], NULL, rt->x, rt->y + 1);
    }
    else
    {
        if (color)
            surface->Blit(surface, window->imageSurface[IMAGE_ID_RED_INDICATOR_DV], NULL, rt->x, rt->y + 1);
        else
            surface->Blit(surface, window->imageSurface[IMAGE_ID_GREEN_INDICATOR_DV], NULL, rt->x, rt->y + 1);
    }
}


static void
draw_button_widget(window_t *window, dialog_t *dialog, dialog_widget_t *widget)
{
    rect_t pos = {0, 0, 0, 0};
    button_t *button = (button_t *) widget->data;

    button->getPosition(button, &pos);
    pos.y  = pos.y - 1;

    if (dialog->getFocus(dialog) == (void *) widget)
        set_draw_color(window->guiSurface, 0, 255, 0, 0);
    else
        set_draw_color(window->guiSurface, 128, 128, 128, 0);

    set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_16]);
    draw_rectangle(window->guiSurface, &pos, true);
    draw_text(window->guiSurface, button->getTitle(button), -1, &pos, DSTF_CENTER);
}


static void
draw_caption_widget(window_t *window, dialog_widget_t *widget)
{
    int height = FONT_HEIGHT_16;
    rect_t drawpos = {0, 0, 0, 0};
    IDirectFBFont *font = NULL;
    DFBSurfaceTextFlags align = DSTF_CENTER;
    caption_t *caption = (caption_t *)widget->data;

    switch (caption->getAlign(caption))
    {
    case CAPTION_ALIGN_LEFT:
        align = DSTF_LEFT;
        break;

    case CAPTION_ALIGN_RIGHT:
        align = DSTF_RIGHT;
        break;

    default:
        align = DSTF_CENTER;
        break;
    }

    caption->getPosition(caption, &drawpos);
    height = caption->getFontHeight(caption);

    if ((height == -1) || (height > FONT_HEIGHT_24))
        height = FONT_HEIGHT_16;

    if (caption->getFont(caption) == CAPTION_FONT_MONOSPACED)
        font = window->monoFont[height];
    else
        font = window->normalFont[height];

    set_draw_color(window->guiSurface, 0, 255, 0, 0);
    set_font(window->guiSurface, font);
    draw_text(window->guiSurface, caption->getString(caption), -1, &drawpos, align);
}


static void
draw_slider_widget(window_t *window, dialog_t *dialog, dialog_widget_t *widget)
{
    int maxval = 0;
    int minval = 0;
    int currval = 0;
    char cval[8] = {0};
    slider_t *slider = (slider_t *) widget->data;
    rect_t pos_slider = {0, 0,   0,  0};
    rect_t pos_frame  = {0, 0, 200, 15};
    rect_t pos_value  = {0, 0, 100, 15};
    rect_t pos_gauge  = {0, 0, 200, 15};
    rect_t pos_title  = {0, 0, 100, 15};

    slider->getMaxValue(slider, &maxval);
    slider->getMinValue(slider, &minval);
    slider->getCurrentValue(slider, &currval);
    slider->getPosition(slider, &pos_slider);
    pos_frame.x = pos_slider.x;
    pos_frame.y = pos_slider.y + 15;
    pos_title.x = pos_slider.x;
    pos_title.y = pos_slider.y;
    pos_value.x = pos_slider.x + 100;
    pos_value.y = pos_slider.y;
    pos_gauge.x = pos_slider.x;
    pos_gauge.y = pos_slider.y + 15;
    pos_gauge.w = (int)floor((double) pos_slider.w / (double) (maxval - minval) * (double) currval);

    memset(cval, 0x00, sizeof(cval));
    snprintf(cval, sizeof(cval), "%d", currval);


    if (dialog->getFocus(dialog) == (void *) widget)
        set_draw_color(window->guiSurface, 0, 255, 0, 0);
    else
        set_draw_color(window->guiSurface, 128, 128, 128, 0);

    set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_14]);
    draw_text(window->guiSurface, slider->getTitle(slider), -1, &pos_title, DSTF_LEFT);
    set_font(window->guiSurface, window->monoFont[FONT_HEIGHT_16]);
    draw_text(window->guiSurface, cval, -1, &pos_value, DSTF_RIGHT);
    draw_rectangle(window->guiSurface, &pos_frame, true);
    draw_fill_rectangle(window->guiSurface, &pos_gauge);
}


static void
draw_spinbox_widget(window_t *window, dialog_t *dialog, dialog_widget_t *widget)
{
    int currval = 0;
    char buf[32] = {0};
    spinbox_t *spinbox = (spinbox_t *) widget->data;
    rect_t drawpos[3] = {0};

    memset(buf, 0x00, sizeof(buf));
    spinbox->getCurrVal(spinbox, &currval);

    if (spinbox->getDatatype(spinbox) == SPINBOX_DATATYPE_NUMERIC)
    {
        if (spinbox->getSign(spinbox) == SPINBOX_UNSIGNED)
            snprintf(buf, sizeof(buf), "%0*d", spinbox->getDigit(spinbox), currval);
        else
            snprintf(buf, sizeof(buf), "%+0*d", spinbox->getDigit(spinbox) + 1, currval);
    }
    else
        snprintf(buf, sizeof(buf), "%c", currval);

    set_draw_color(window->guiSurface, 0, 255, 0, 0);
    spinbox->getPosition(spinbox, &drawpos[0]);

    if (dialog->getFocus(dialog) == widget)
    {
        drawpos[1].x = drawpos[0].x;
        drawpos[1].y = drawpos[0].y - 18;
        drawpos[1].w = drawpos[0].w;
        drawpos[1].h = drawpos[0].h;
        drawpos[2].x = drawpos[0].x;
        drawpos[2].y = drawpos[0].y + 16;
        drawpos[2].w = drawpos[0].w;
        drawpos[2].h = drawpos[0].h;

        set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_12]);
        draw_text(window->guiSurface, STR_CURSOR_UP, -1, &drawpos[1], DSTF_CENTER);
        draw_text(window->guiSurface, STR_CURSOR_DOWN, -1, &drawpos[2], DSTF_CENTER);
    }

    if (spinbox->getFontSize(spinbox) == SPINBOX_FONT_SIZE_NORMAL)
        set_font(window->guiSurface, window->monoFont[FONT_HEIGHT_18]);
    else
        set_font(window->guiSurface, window->monoFont[FONT_HEIGHT_24]);

    draw_text(window->guiSurface, buf, -1, &drawpos[0], DSTF_CENTER);
}


static void
draw_qsel_widget(window_t *window, dialog_t *dialog, dialog_widget_t *widget)
{
    rect_t drawpos = {0};
    qsel_t *qsel = (qsel_t *) widget->data;

    set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_14]);

    for (int i = 0; i < 5; i++)
    {
        set_draw_color(window->guiSurface, 0, 255, 0, 0);

        if (qsel->getFocus(qsel) == i)
        {
            qsel->getPosition(qsel, i, &drawpos);
            draw_fill_rectangle(window->guiSurface, &drawpos);
            set_draw_color(window->guiSurface, 0, 0, 0, 0);
        }
        else
        {
            qsel->getPosition(qsel, i, &drawpos);
            draw_rectangle(window->guiSurface, &drawpos, false);
        }

        draw_text(window->guiSurface, qsel->getString(qsel, i), -1, &drawpos, DSTF_CENTER);
    }
}


static void
draw_svplot_widget(window_t *window, dialog_t *dialog, dialog_widget_t *widget)
{
    static int i = 0;
    int nsv_gps = 0;
    int nsv_glns = 0;
    char *tag = NULL;
    char *val = NULL;
    char buf[8] = {0};
    rect_t pos = {0};
    rect_t rtsv = {0, 0, 10, 10};
    rect_t rtid = {0, 0, 10, 10};
    struct svcoord *gps = NULL;
    struct svcoord *glns = NULL;
    svplot_t *svplot = (svplot_t *) widget->data;
    IDirectFBSurface *surf = window->guiSurface;

    if (i % 5 == 0)
    {
        svplot->update(svplot);
        svplot->getValue(svplot, SVPLOT_INDEX_BGIMG, &pos);
        surf->Blit(surf, window->imageSurface[IMAGE_ID_SVPLOT_BACKGROUND], NULL, pos.x, pos.y);
        set_font(surf, window->normalFont[FONT_HEIGHT_16]);
        set_draw_color(surf, 0, 255, 0, 0);

        for (int j = 1; j < 0x0D; j++)
        {
            tag = svplot->getTag(svplot, j, &pos);
            draw_text(surf, tag, -1, &pos, DSTF_LEFT);

            if ((j == SVPLOT_INDEX_NSV_GPS) || (j == SVPLOT_INDEX_NSV_GLNS))
                set_font(surf, window->normalFont[FONT_HEIGHT_16]);
            else
                set_font(surf, window->monoFont[FONT_HEIGHT_18]);

            val = svplot->getValue(svplot, j, &pos);
            draw_text(surf, val, -1, &pos, DSTF_LEFT);
            set_font(surf, window->normalFont[FONT_HEIGHT_16]);
        }

        svplot->getSatInfo(svplot, &nsv_gps, &nsv_glns, &gps, &glns);

        for (int j = 0; j < nsv_gps; j++)
        {
            switch((gps + j)->snr)
            {
                case SVPLOT_SNR_EXCELENT:
                    set_draw_color(surf, 0, 255, 0, 0);
                    break;

                case SVPLOT_SNR_GOOD:
                    set_draw_color(surf, 173, 255, 47, 0);
                    break;

                case SVPLOT_SNR_MODERATE:
                    set_draw_color(surf, 255, 255, 0, 0);
                    break;

                case SVPLOT_SNR_POOR:
                    set_draw_color(surf, 255, 140, 0, 0);
                    break;

                default:
                    continue;
            }

            rtsv.x = (gps + j)->x - 5;
            rtsv.y = (gps + j)->y - 5;
            rtid.x = (gps + j)->x - 5;
            rtid.y = (gps + j)->y - 15;
            snprintf(buf, sizeof(buf), "%d", (gps + j)->id);
            set_font(surf, window->normalFont[FONT_HEIGHT_10]);
            draw_text(surf, "\u25CF", -1, &rtsv, DSTF_CENTER);
            set_font(surf, window->monoFont[FONT_HEIGHT_12]);
            draw_text(surf, buf, -1, &rtid, DSTF_CENTER);
        }

        for (int j = 0; j < nsv_glns; j++)
        {
            switch((glns + j)->snr)
            {
            case SVPLOT_SNR_EXCELENT:
                set_draw_color(surf, 0, 255, 0, 0);
                break;

            case SVPLOT_SNR_GOOD:
                set_draw_color(surf, 173, 255, 47, 0);
                break;

            case SVPLOT_SNR_MODERATE:
                set_draw_color(surf, 255, 255, 0, 0);
                break;

            case SVPLOT_SNR_POOR:
                set_draw_color(surf, 255, 140, 0, 0);
                break;

            default:
                continue;
            }

            rtsv.x = (glns + j)->x - 5;
            rtsv.y = (glns + j)->y - 5;
            rtid.x = (glns + j)->x - 5;
            rtid.y = (glns + j)->y - 15;
            snprintf(buf, sizeof(buf), "%d", (glns + j)->id);
            set_font(surf, window->normalFont[FONT_HEIGHT_10]);
            draw_text(surf, "\u25A0", -1, &rtsv, DSTF_CENTER);
            set_font(surf, window->monoFont[FONT_HEIGHT_12]);
            draw_text(surf, buf, -1, &rtid, DSTF_CENTER);
        }
    }
    else
        i++;
}

#define NUM_ROWS_IN_PAGE        8

static void
draw_grid_widget(window_t *window, dialog_t *dialog, dialog_widget_t *widget)
{
    int page = 0;
    int nrows = 0;
    rect_t pos[2] = {0};
    grid_row_t *row = NULL;
    grid_row_t *sel = NULL;
    grid_column_t *col = NULL;
    grid_t *grid = (grid_t *) widget->data;
    IDirectFBSurface *surf = window->guiSurface;

    row = grid->getColumnHeader(grid);
    col = row->setHeadColumn(row);
    grid->getPosition(grid, &pos[0]);
    pos[1].x = pos[0].x;
    pos[1].y = pos[0].y;
    set_draw_color(surf, 0, 255, 0, 0);
    set_font(surf, window->normalFont[FONT_HEIGHT_14]);
    draw_rectangle(surf, &pos[0], false);

    /* draw header row */
    for (int i = 0; i < row->getNumColumns(row); i++)
    {
        col->getSize(col, &pos[1].w, &pos[1].h);
        draw_text(surf, col->getString(col), -1, &pos[1], DSTF_CENTER);
        surf->DrawLine(surf, pos[1].x, pos[0].y, pos[1].x, pos[0].y + pos[0].h - 1);
        pos[1].x = pos[1].x + pos[1].w;
        col = col->getNextColumn(col);
    }

    surf->DrawLine(surf, pos[0].x, pos[0].y + pos[1].h -1, pos[0].x + pos[0].w - 1, pos[0].y + pos[1].h - 1);

    /* draw grid row */
    if (grid->getNumRows(grid) != 0)
    {
        set_font(surf, window->monoFont[FONT_HEIGHT_16]);
        sel = grid->getCurrRow(grid);
        page = sel->getRowIndex(sel) / NUM_ROWS_IN_PAGE;

        if ((grid->getNumRows(grid) - page * NUM_ROWS_IN_PAGE) < NUM_ROWS_IN_PAGE)
            nrows = grid->getNumRows(grid) % NUM_ROWS_IN_PAGE;
        else
            nrows = NUM_ROWS_IN_PAGE;

        row = grid->getHeadRow(grid);

        while(page * NUM_ROWS_IN_PAGE != row->getRowIndex(row))
            row = row->getNextRow(row);

        for (int i = 0; i < nrows; i++)
        {
            pos[1].x = pos[0].x;
            pos[1].y = pos[1].y + pos[1].h;

            col = row->setHeadColumn(row);

            for (int j = 0; j < row->getNumColumns(row); j++)
            {
                col->getSize(col, &pos[1].w, &pos[1].h);

                if (sel == row)
                {
                    set_draw_color(surf, 0, 255, 0, 0);
                    draw_fill_rectangle(surf, &pos[1]);
                    set_draw_color(surf, 0, 0, 0, 0);
                    draw_text(surf, col->getString(col), -1, &pos[1], DSTF_CENTER);
                }
                else
                {
                    set_draw_color(surf, 0, 255, 0, 0);
                    draw_text(surf, col->getString(col), -1, &pos[1], DSTF_CENTER);
                    surf->DrawLine(surf, pos[1].x, pos[0].y, pos[1].x, pos[0].y + pos[0].h - 1);
                }

                pos[1].x = pos[1].x + pos[1].w;
                col = col->getNextColumn(col);
            }

            //surf->DrawLine(surf, pos[0].x, pos[0].y + pos[1].h -1, pos[0].x + pos[0].w - 1, pos[0].y + pos[1].h - 1);
            row = row->getNextRow(row);
        }
    }
}


void
draw_pwroff_splash(window_t *window)
{
    rect_t drawpos[] =
    {
        {  0,   0, 800, 600},
        {260, 370, 280, 100}
    };

    set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_16]);
    set_draw_color(window->guiSurface, 0, 0, 0, 0);
    draw_fill_rectangle(window->guiSurface, &drawpos[0]);

    set_draw_color(window->guiSurface, 0, 255, 0, 0);
    draw_rectangle(window->guiSurface, &drawpos[1], true);
    draw_text(window->guiSurface, STR_NOTICE_POWEROFF, -1, &drawpos[1], DSTF_CENTER);
}


void
draw_background(window_t *window, struct vgss_interface *vgss)
{
    window->guiSurface->SetColor(window->guiSurface, 0, 0, 0, 0);
    window->guiSurface->FillRectangle(window->guiSurface, 0, 0, 800, 600);

    if (vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE) == PREVIEW_STATE_RUN)
    {
        window->guiSurface->SetColor(window->guiSurface, 8, 8, 8, 0);
        window->guiSurface->FillRectangle(window->guiSurface, 80, 65, 640, 480);
    }
}


void
draw_reticle(window_t *window, struct vgss_interface *vgss)
{
    DFBRectangle drawpos = {0, 0, 640, 480};
    IDirectFBSurface *surface = window->guiSurface;

    if (vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE) == PREVIEW_STATE_RUN)
    {
        if (vgss->ctrlZoom(vgss, EZOOM_CTRL_STATE) == EZOOM_STATE_ENABLE)
            surface->Blit(surface, window->imageSurface[IMAGE_ID_RETICLE_2X], &drawpos, 80, 65);
        else
            surface->Blit(surface, window->imageSurface[IMAGE_ID_RETICLE_IR], &drawpos, 80, 65);
    }
}


void
draw_timeinfo(window_t *window, struct device_interface *device)
{
    char str_date[32];
    char str_time[32];

    static rect_t drawpos[] =
    {
        {80, 14, 100, 20},  /* date position */
        {180, 14,  100, 20}   /* time position */
    };

    memset(str_date, 0x00, sizeof(str_date));
    memset(str_time, 0x00, sizeof(str_time));
    device->getSystemTime(device, str_date, str_time, sizeof(str_date), sizeof(str_time));
    set_font(window->guiSurface, window->monoFont[FONT_HEIGHT_18]);
    set_draw_color(window->guiSurface, 0, 255, 0, 0);
    draw_text(window->guiSurface, str_date, -1, &drawpos[0], DSTF_LEFT);
    draw_text(window->guiSurface, str_time, -1, &drawpos[1], DSTF_LEFT);
}


void
draw_observer_location(window_t *window, struct taqmgr_interface *taqmgr)
{
    char *string = NULL;
    unsigned char r = 0;
    unsigned char g = 0;
    rect_t rt = {180, 14, 330, 20};

    if (taqmgr->getUserData(taqmgr) & USE_USER_INPUT_OBLOC)
    {
        r = 0; g = 255;
        string = taqmgr->getCoordString(taqmgr);
        set_font(window->guiSurface, window->monoFont[FONT_HEIGHT_18]);
    }
    else
    {
        if (taqmgr->getError(taqmgr) == GNSS_ERROR_NONE)
        {
            if (taqmgr->isOnline(taqmgr))
            {
                string = taqmgr->getCoordString(taqmgr);

                switch (taqmgr->getDopLevel(taqmgr))
                {
                case DOPLV_GOOD:
                    r = 128;
                    g = 255;
                    break;

                case DOPLV_MODERATE:
                    r = 255;
                    g = 255;
                    break;

                case DOPLV_FAIR:
                    r = 255;
                    g = 128;
                    break;

                case DOPLV_POOR:
                    r = 255;
                    break;

                default:
                    g = 255;
                    break;
                }

                set_font(window->guiSurface, window->monoFont[FONT_HEIGHT_18]);
            }
            else
            {
                r = 0; g = 255;
                string = STR_NO_GNSS_SIGNAL;
                set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_16]);
            }
        }
        else
        {
            r = 255; g = 0;
            string = STR_NO_GNSS_SIGNAL;
            set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_16]);
        }
    }

    set_draw_color(window->guiSurface, r, g, 0, 0);
    draw_text(window->guiSurface, string, -1, &rt, DSTF_LEFT);
}


void
draw_angular_indicator(window_t *window, struct main_interface *mainif)
{
    int mil[3] = {0, 0, 0};
    char az[32] = {0};
    char el[32] = {0};
    char bk[32] = {0};
    double deg[3] = {0.0, 0.0, 0.0};
    IDirectFBFont *font = window->monoFont[FONT_HEIGHT_18];
    struct dmc_interface *dmc = mainif->device->dmc;
    struct taqmgr_interface *taqmgr = mainif->taqmgr;


    static rect_t drawpos[] =
    {
        {180,  34,  48,  20},   /* azimuth string position             0 */
        {230,  34, 100,  20},   /* azimuth value string position       1 */
        {329,  34,  16,  20},   /* azimuth unit string position        2 */
        {357 , 34,  48,  20},   /* elevation string position           3 */
        {405,  34, 108,  20},   /* elevation value string position     4 */
        {513,  34,  16,  20},   /* elevation unit string position      5 */
        {357 , 34,  48,  20},   /* bank string position                6 */
        {405,  34, 108,  20},   /* bank value string position          7 */
        {513,  34,  16,  20}    /* bank unit string position           8 */
    };


    if (dmc->getError(dmc) == DMC_ERROR_NONE)
    {
        memset(az, 0x00, sizeof(az));
        memset(el, 0x00, sizeof(el));
        memset(bk, 0x00, sizeof(bk));
        taqmgr->getAzimuth(taqmgr, &mil[0], &deg[0]);
        taqmgr->getElevation(taqmgr, &mil[1], &deg[1]);
        taqmgr->getBank(taqmgr, &mil[2], &deg[2]);
        snprintf(az, sizeof(az), "%04d", mil[0]);
        snprintf(el, sizeof(el), "%+05d", mil[1]);
        snprintf(bk, sizeof(bk), "%+05d", mil[2]);
        font->GetStringWidth(font, az, -1, &drawpos[1].w);
        font->GetStringWidth(font, el, -1, &drawpos[4].w);
        font->GetStringWidth(font, bk, -1, &drawpos[7].w);

        drawpos[2].x = drawpos[1].x + drawpos[1].w;
        drawpos[3].x = drawpos[2].x + drawpos[2].w + 12;
        drawpos[4].x = drawpos[3].x + drawpos[3].w + 2;
        drawpos[5].x = drawpos[4].x + drawpos[4].w;
        drawpos[6].x = drawpos[5].x + drawpos[5].w + 12;
        drawpos[7].x = drawpos[6].x + drawpos[6].w + 2;
        drawpos[8].x = drawpos[7].x + drawpos[7].w;

        if ((800 == mil[1]) || (-800 == mil[1]) || (800 == mil[2]) || (-800 == mil[2]))
            set_draw_color(window->guiSurface, 255, 0, 0, 0);
        else
            set_draw_color(window->guiSurface, 0, 255, 0, 0);

        set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_16]);
        draw_text(window->guiSurface, STR_AZIMUTH, -1, &drawpos[0], DSTF_LEFT);
        set_font(window->guiSurface, window->monoFont[FONT_HEIGHT_18]);
        draw_text(window->guiSurface, az, -1, &drawpos[1], DSTF_LEFT);
        set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_16]);
        draw_text(window->guiSurface, STR_MIL, -1, &drawpos[2], DSTF_LEFT);

        draw_text(window->guiSurface, STR_ELEVATION, -1, &drawpos[3], DSTF_LEFT);
        set_font(window->guiSurface, window->monoFont[FONT_HEIGHT_18]);
        draw_text(window->guiSurface, el, -1, &drawpos[4], DSTF_LEFT);
        set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_16]);
        draw_text(window->guiSurface, STR_MIL, -1, &drawpos[5], DSTF_LEFT);

        draw_text(window->guiSurface, STR_BANK, -1, &drawpos[6], DSTF_LEFT);
        set_font(window->guiSurface, window->monoFont[FONT_HEIGHT_18]);
        draw_text(window->guiSurface, bk, -1, &drawpos[7], DSTF_LEFT);
        set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_16]);
        draw_text(window->guiSurface, STR_MIL, -1, &drawpos[8], DSTF_LEFT);
    }
    else
    {
        set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_16]);
        set_draw_color(window->guiSurface, 255, 0, 0, 0);
        draw_text(window->guiSurface, STR_DMC_ERROR, -1, &drawpos[0], DSTF_LEFT);
    }
}


void
draw_range_indicator(window_t *window, struct taqmgr_interface *taqmgr)
{
    int range[2] = {0, 0};
    char first[16] = {0};
    char last[16]  = {0};

    static rect_t drawpos[] =
    {
        {550, 14, 80, 20},      /* first range position */
        {550, 34, 80, 20},      /*  last range position */
        {550, 14, 80, 40},
    };

    taqmgr->getRange(taqmgr, &range[0], &range[1]);
    snprintf(first, sizeof(first), "%04dm", range[0]);
    //snprintf(last, sizeof(last), "%05dm", range[1]);
    set_draw_color(window->guiSurface, 0, 255, 0, 0);
    set_font(window->guiSurface, window->monoFont[FONT_HEIGHT_24]);
    draw_text(window->guiSurface, first, -1, &drawpos[2], DSTF_CENTER);
    //set_font(window->guiSurface, window->monoFont[FONT_HEIGHT_18]);
    //draw_text(window->guiSurface, last, -1, &drawpos[1], DSTF_CENTER);
}


void
draw_battery_indicator(window_t *window, struct mcu_interface *mcu)
{
    int level = 0;
    int ngauge = 0;

    rect_t drawpos[] =
    {
        {650,  32,   6,   8},       /* drawing position : battery '+' terminal  */
        {654,  21,  66,  30},       /* drawing position : battery body          */
        {706,  25,  10,  22}        /* drawing position : battery level gauge   */
    };

    if (mcu->getPwrSrcType(mcu) == MCU_PWRSRC_CELL)
    {
        level = mcu->getBatteryLevel(mcu);
        ngauge = level / 2 + level % 2;

        if (level <= 2)
            set_draw_color(window->guiSurface, 255, 0, 0, 0);
        else if (level <= 4)
            set_draw_color(window->guiSurface, 255, 165, 0, 0);
        else if (level <= 6)
            set_draw_color(window->guiSurface, 255, 255, 0, 0);
        else
            set_draw_color(window->guiSurface, 0, 255, 0, 0);

        draw_rectangle(window->guiSurface, &drawpos[0], true);
        draw_rectangle(window->guiSurface, &drawpos[1], true);

        for (int i = 0; i < ngauge; i++)
        {
            if ((level - ((2 * i) + 1)) == 0)
                draw_rectangle(window->guiSurface, &drawpos[2], false);
            else
                draw_fill_rectangle(window->guiSurface, &drawpos[2]);

            drawpos[2].x = drawpos[2].x - 12;
        }
    }
    else
    {
        set_draw_color(window->guiSurface, 0, 255, 0, 0);
        window->guiSurface->Blit(window->guiSurface, window->imageSurface[IMAGE_ID_POWER_PLUG], NULL, 664, 20);
        draw_rectangle(window->guiSurface, &drawpos[0], true);
        draw_rectangle(window->guiSurface, &drawpos[1], true);
    }
}

void
draw_ftmode_indicator(window_t *window, struct vgss_interface *vgss)
{
	static rect_t drawpos = {80, 34, 80, 30};

	set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_24]);
	set_draw_color(window->guiSurface, 0, 255, 0, 0);
	draw_text(window->guiSurface, STR_FT, -1, &drawpos, DSTF_LEFT);
}



void
draw_obmode_indicator(window_t *window, struct vgss_interface *vgss)
{
    static rect_t drawpos =  {85, 60, 40, 22};

    draw_indicator_background(window, vgss, &drawpos, 0);
    set_draw_color(window->guiSurface, 0, 255, 0, 0);
    set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_16]);

    if (vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE) == PREVIEW_STATE_STOP)
        draw_text(window->guiSurface, STR_DV, -1, &drawpos, DSTF_CENTER);
    else
        draw_text(window->guiSurface, STR_IR, -1, &drawpos, DSTF_CENTER);
}


void
draw_irpol_indicator(window_t *window, struct vgss_interface *vgss, struct ircam_interface *ircam)
{
    static rect_t drawpos = {130, 60, 40, 22};

    if (vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE) == PREVIEW_STATE_RUN)
    {
        draw_indicator_background(window, vgss, &drawpos, 0);
        set_draw_color(window->guiSurface, 0, 255, 0, 0);
        set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_16]);

        if (ircam->getColor(ircam) == IRCAM_COLOR_MONO)
        {
            if (ircam->getPolarity(ircam) == IRCAM_IRPOL_BLACK)
                draw_text(window->guiSurface, STR_BHOT, -1, &drawpos, DSTF_CENTER);
            else
                draw_text(window->guiSurface, STR_WHOT, -1, &drawpos, DSTF_CENTER);
        }
        else
            draw_text(window->guiSurface, STR_PCLR, -1, &drawpos, DSTF_CENTER);
    }
}


void
draw_irnuc_indicator(window_t *window, struct vgss_interface *vgss, struct ircam_interface *ircam)
{
    static rect_t drawpos = {175, 60, 40, 22};

    if (vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE) == PREVIEW_STATE_RUN)
    {
        if (ircam->getNucStatus(ircam) == IRCAM_NUC_INPROC)
        {
            draw_indicator_background(window, vgss, &drawpos, 0);
            set_draw_color(window->guiSurface, 0, 255, 0, 0);
            set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_16]);
            draw_text(window->guiSurface, STR_NUC, -1, &drawpos, DSTF_CENTER);
        }
    }
}


void
draw_ezoom_indicator(window_t *window, struct vgss_interface *vgss)
{
    static rect_t drawpos = {85, 86, 40, 22};

    if (vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE) == PREVIEW_STATE_RUN)
    {
        if (vgss->ctrlZoom(vgss, EZOOM_CTRL_STATE) == EZOOM_STATE_ENABLE)
        {
            draw_indicator_background(window, vgss, &drawpos, 0);
            set_draw_color(window->guiSurface, 0, 255, 0, 0);
            set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_16]);
            draw_text(window->guiSurface, STR_EZOOM, -1, &drawpos, DSTF_CENTER);
        }
    }
}


void
draw_ireis_indicator(window_t *window, struct vgss_interface *vgss)
{
    static rect_t drawpos = {130, 86, 40, 22};

    if (vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE) == PREVIEW_STATE_RUN)
    {
        if (vgss->ctrlEIS(vgss, EIS_CTRL_STATE) == EIS_STATE_ENABLE)
        {
            draw_indicator_background(window, vgss, &drawpos, 0);
            set_draw_color(window->guiSurface, 0, 255, 0, 0);
            set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_16]);
            draw_text(window->guiSurface, STR_EIS, -1, &drawpos, DSTF_CENTER);
        }
    }
}


void
draw_lrfst_indicator(window_t *window, struct taqmgr_interface *taqmgr, struct vgss_interface *vgss, struct lrf_interface *lrf)
{
    char *string = NULL;
    IDirectFBFont *font = window->normalFont[FONT_HEIGHT_18];
    IDirectFBSurface *surface = window->guiSurface;
    rect_t pos = {.x = 400, .y = 140, .w = 100, .h = 26};

    if (taqmgr->getUserData(taqmgr) & USE_USER_INPUT_RANGE)
    {
        if (taqmgr->getMrst(taqmgr) == MANUAL_RANGING_INPROC)
            string = STR_MANUAL_RANGING;
        else
            return;
    }
    else
    {
        switch(lrf->getTrigger(lrf))
        {
        case LRF_TRG_READY:
            string = STR_LRF_READY;
            break;

        case LRF_TRG_WAIT:
            string = STR_LRF_CHARGE;
            break;

        default:
            return;
        }
    }
    //TLOGMSG(1, ("lrf trigger = %d\n", lrf->get_trigger(lrf)));

    font->GetStringWidth(font, string, -1, &pos.w);
    pos.w = pos.w + 20;
    pos.x = (int) (pos.x - pos.w / 2);
    set_font(surface, font);
    set_draw_color(surface, 0, 0, 0, 0);
    draw_fill_rectangle(surface, &pos);
    set_draw_color(surface, 255, 0, 0, 0);
    draw_rectangle(surface, &pos, false);

    if (vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE) == PREVIEW_STATE_RUN)
    {
        surface->Blit(surface, window->imageSurface[IMAGE_ID_LRFTRG_INDICATOR_LEFT_IR], NULL, pos.x, pos.y);
        surface->Blit(surface, window->imageSurface[IMAGE_ID_LRFTRG_INDICATOR_RIGHT_IR], NULL, pos.x + pos.w - 10, pos.y);
    }
    else
    {
        surface->Blit(surface, window->imageSurface[IMAGE_ID_LRFTRG_INDICATOR_LEFT_DV], NULL, pos.x, pos.y);
        surface->Blit(surface, window->imageSurface[IMAGE_ID_LRFTRG_INDICATOR_RIGHT_DV], NULL, pos.x + pos.w - 10, pos.y);
    }

    pos.y = pos.y - 1;
    draw_text(surface, string, -1, &pos, DSTF_CENTER);
}


void
draw_azmode_indicator(window_t *window, struct vgss_interface *vgss, struct taqmgr_interface *taqmgr)
{
    char *string = NULL;
    static rect_t drawpos = {630, 60, 40, 22};

    switch (taqmgr->getAzMode(taqmgr))
    {
    case AZMODE_MAGNETIC_NORTH:
        string = STR_MNORTH;
        draw_indicator_background(window, vgss, &drawpos, 1);
        set_draw_color(window->guiSurface, 255, 0, 0, 0);
        break;

    case AZMODE_GRID_NORTH:
        string = STR_GNORTH;
        draw_indicator_background(window, vgss, &drawpos, 0);
        set_draw_color(window->guiSurface, 0, 255, 0, 0);
        break;

    case AZMODE_TRUE_NORTH:
        string = STR_TNORTH;
        draw_indicator_background(window, vgss, &drawpos, 0);
        set_draw_color(window->guiSurface, 0, 255, 0, 0);
        break;

    default:
        return;
    }

    set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_16]);
    draw_text(window->guiSurface, string, -1, &drawpos, DSTF_CENTER);
}


void
draw_celltype_indicator(window_t *window, struct vgss_interface *vgss, struct mcu_interface *mcu)
{
    char *string = NULL;
    static rect_t drawpos = {675, 60, 40, 22};

    if (mcu->getPwrSrcType(mcu) == MCU_PWRSRC_CELL)
    {
        if (mcu->getBatteryType(mcu) == MCU_CELL_SECONDARY)
            string = STR_SECONDARY;
        else
            string = STR_PRIMARY;
    }
    else
        string = STR_EXTERNAL;

    draw_indicator_background(window, vgss, &drawpos, 0);
    set_draw_color(window->guiSurface, 0, 255, 0, 0);
    set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_16]);
    draw_text(window->guiSurface, string, -1, &drawpos, DSTF_CENTER);
}


void
draw_daqmode_indicator(window_t *window, struct vgss_interface *vgss, struct taqmgr_interface *taqmgr)
{
    char *string = NULL;
    static rect_t drawpos = {675, 60, 40, 22};

    if ((taqmgr->getUserData(taqmgr) & USE_USER_INPUT_OBLOC) || (taqmgr->getUserData(taqmgr) & USE_USER_INPUT_RANGE))
    {
        string = STR_MANUAL_DAQ;
        draw_indicator_background(window, vgss, &drawpos, 1);
        set_draw_color(window->guiSurface, 255, 0, 0, 0);
    }
    else
    {
        string = STR_AUTO_DAQ;
        draw_indicator_background(window, vgss, &drawpos, 0);
        set_draw_color(window->guiSurface, 0, 255, 0, 0);
    }

    set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_16]);
    draw_text(window->guiSurface, string, -1, &drawpos, DSTF_CENTER);
}


void
draw_rgate_indicator(window_t *window, struct vgss_interface *vgss, struct lrf_interface *lrf)
{
    int max = 0;
    int min = 0;
    static rect_t drawpos = {675, 86, 40, 22};

    lrf->getMeasuringRange(lrf, &max, &min);

    if ((max != 0) && (min != 0))
    {
        draw_indicator_background(window, vgss, &drawpos, 0);
        set_draw_color(window->guiSurface, 0, 255, 0, 0);
        set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_16]);
        draw_text(window->guiSurface, "RG", -1, &drawpos, DSTF_CENTER);
    }
}


void
draw_taqmode_indicator(window_t *window, struct vgss_interface *vgss, struct taqmgr_interface *taqmgr)
{
    char *str[2] = {NULL, NULL};
    IDirectFBSurface *surf = window->guiSurface;
    IDirectFBFont *font = window->normalFont[FONT_HEIGHT_18];

    rect_t pos[] =
    {
        {400, 110, 100, 26},
        {400, 464, 100, 26},
        {350, 430, 100, 26}
    };

    if (taqmgr->getStatus(taqmgr) == TAQMGR_STATUS_INPROC)
    {
        switch (taqmgr->getTaqMode(taqmgr))
        {
            case TAQMODE_CIRCULAR_TARGET:
                str[0] = STR_TAQMODE_CIRCULAR_TARGET;
                str[1] = STR_GUIDE_CIRCULAR_TARGET;
                break;

            case TAQMODE_SQUARE_TARGET_LENGTH:
                set_font(surf, font);
                set_draw_color(surf, 255, 0, 0, 0);
                draw_text(surf, "1 / 2", -1, &pos[2], DSTF_CENTER);
                str[0] = STR_TAQMODE_SQUARE_TARGET;
                str[1] = STR_GUIDE_SQUARE_TARGET_LENGTH;
                break;

            case TAQMODE_SQUARE_TARGET_WIDTH:
                set_font(surf, font);
                set_draw_color(surf, 255, 0, 0, 0);
                draw_text(surf, "2 / 2", -1, &pos[2], DSTF_CENTER);
                str[0] = STR_TAQMODE_SQUARE_TARGET;
                str[1] = STR_GUIDE_SQUARE_TARGET_WIDTH;
                break;

            case TAQMODE_FOS_CORRECTION:
                str[0] = STR_TAQMODE_FOS_CORRECTION;
                str[1] = STR_GUIDE_FOS_CORRECTION;
                break;

            default:
                str[1] = STR_GUIDE_POINT_TARGET;
                break;
        }

        for (int i = 0; i < 2; i++)
        {
            if (str[i])
            {
                font->GetStringWidth(font, str[i], -1, &pos[i].w);
                pos[i].w = pos[i].w + 20;
                pos[i].x = pos[i].x - pos[i].w / 2;
                set_font(surf, font);
                set_draw_color(surf, 0, 0, 0, 0);
                draw_fill_rectangle(surf, &pos[i]);
                set_draw_color(surf, 255, 0, 0, 0);
                draw_rectangle(surf, &pos[i], false);

                if (vgss->ctrlPreview(vgss, PREVIEW_CTRL_STATE) == PREVIEW_STATE_RUN)
                {
                    surf->Blit(surf, window->imageSurface[IMAGE_ID_LRFTRG_INDICATOR_LEFT_IR], NULL, pos[i].x, pos[i].y);
                    surf->Blit(surf, window->imageSurface[IMAGE_ID_LRFTRG_INDICATOR_RIGHT_IR], NULL, pos[i].x + pos[i].w - 10, pos[i].y);
                }
                else
                {
                    surf->Blit(surf, window->imageSurface[IMAGE_ID_LRFTRG_INDICATOR_LEFT_DV], NULL, pos[i].x, pos[i].y);
                    surf->Blit(surf, window->imageSurface[IMAGE_ID_LRFTRG_INDICATOR_RIGHT_DV], NULL, pos[i].x + pos[i].w - 10, pos[i].y);
                }

                pos[i].y = pos[i].y - 1;
                draw_text(surf, str[i], -1, &pos[i], DSTF_CENTER);
            }
        }
    }
}


void
draw_target_information(window_t *window, struct taqmgr_interface *taqmgr)
{
    double fwdaz = 0.0;
    char *coordstr = NULL;
    char string[11][32] = {0};
    struct tm *tm = NULL;
    taqdata_t data = {0};
    IDirectFBFont *font12n = window->normalFont[FONT_HEIGHT_12];
    IDirectFBFont *font16n = window->normalFont[FONT_HEIGHT_16];
    IDirectFBFont *font18m = window->monoFont[FONT_HEIGHT_18];
    IDirectFBSurface *surf = window->guiSurface;

    rect_t tag = {0, 0, 0, 0};
    rect_t unit = {0, 0, 100, 26};
    rect_t drawpos[] =
    {
        {216, 100, 368, 110},
        {226, 116, 250,  24},   // 1 - coordinate
        {480, 116,  70,  24},   // 2 - aquistion time
        {226, 150,  50,  24},   // 3 - altitude
        {300, 150,  60,  24},   // 4 - horizontal distance
        {375, 150,  60,  24},   // 5 - foward azimuth
        {450, 150,  60,  24},   // 6 - foward elevation
        {525, 150,  60,  24},   // 7 - target attitude
        {226, 184,  60,  24},   // 8 - target length
        {226, 184,  60,  24},   // 9 - target width
        {226, 184,  60,  24},   // 10 - target radius
        {226, 184,  60,  24},   // 11 - lateral shift
        {226, 184,  60,  24},   // 12 - range shift
    };


    if ((taqmgr->getResult(taqmgr, &data)) == 0)
    {
        /* get data */
        coordstr = &(data.target.coordstr[taqmgr->getCoordSystem(taqmgr)][0]);
        fwdaz = compensate_fwdaz(data.observer.fwdaz, data.observer.magdecl, data.observer.gridvar);

        /* make strings */
        tm = localtime(&data.taqtime.tv_sec);

        if (tm)
        {
            strftime(&(string[0][0]), 32, "%H:%M:%S", tm);
            snprintf(&(string[1][0]), 32, "%+05dm", lround(data.target.altitude));
            snprintf(&(string[2][0]), 32, "%05dm", data.observer.hdist);
            snprintf(&(string[3][0]), 32, "%04d", lround(DEG2MIL(fwdaz)));
            snprintf(&(string[4][0]), 32, "%+03d", lround(DEG2MIL(data.observer.fwdel)));

            if (data.target.shape == TARGET_SHAPE_SQUARE)
            {
                snprintf(&(string[5][0]), 32, "%04d", data.target.attitude);
                snprintf(&(string[6][0]), 32, "%05dm", data.target.length);
                snprintf(&(string[7][0]), 32, "%05dm", data.target.width);
            }
            else
            {
                snprintf(&(string[5][0]), 32, "----");
                snprintf(&(string[6][0]), 32, "-----m");
                snprintf(&(string[7][0]), 32, "-----m");
            }

            if (data.target.shape == TARGET_SHAPE_CIRCLE)
                snprintf(&(string[8][0]), 32, "%05dm", data.target.radius);
            else
                snprintf(&(string[8][0]), 32, "-----m");

            snprintf(&(string[9][0]), 32, "%+05dm", data.shift.lateral);
            snprintf(&(string[10][0]), 32, "%+05dm", data.shift.range);

            /* draw background */
            set_draw_color(surf, 0, 0, 0, 0);
            draw_fill_rectangle(surf, &drawpos[0]);
            set_draw_color(surf, 0, 255, 0, 0);
            draw_rectangle(surf, &drawpos[0], false);

            // draw coordstr
            memcpy(&tag, &drawpos[1], sizeof(rect_t));
            tag.y = tag.y - 16;
            set_font(surf, font12n);
            draw_text(surf, STR_TARGET_COORDINATE, -1, &tag, DSTF_LEFT);
            set_font(surf, font18m);
            draw_text(surf, coordstr, -1, &drawpos[1], DSTF_LEFT);

            // draw altitude
            font18m->GetStringWidth(font18m, &(string[2][0]), -1, &drawpos[3].w);
            memcpy(&tag, &drawpos[3], sizeof(rect_t));
            tag.y = tag.y - 16;
            set_font(surf, font12n);
            draw_text(surf, STR_TARGET_ALTITUDE, -1, &tag, DSTF_LEFT);
            set_font(surf, font18m);
            draw_text(surf, &(string[1][0]), -1, &drawpos[3], DSTF_LEFT);

            // draw horizontal distance
            drawpos[4].x = drawpos[3].x + drawpos[3].w + 15;
            font18m->GetStringWidth(font18m, &(string[2][0]), -1, &drawpos[4].w);
            memcpy(&tag, &drawpos[4], sizeof(rect_t));
            tag.y = tag.y - 16;
            set_font(surf, font12n);
            draw_text(surf, STR_HDIST, -1, &tag, DSTF_LEFT);
            set_font(surf, font18m);
            draw_text(surf, &(string[2][0]), -1, &drawpos[4], DSTF_LEFT);

            // draw foward azimuth
            drawpos[5].x = drawpos[4].x + drawpos[4].w + 15;
            font18m->GetStringWidth(font18m, &(string[3][0]), -1, &drawpos[5].w);
            font16n->GetStringWidth(font16n, STR_MIL, -1, &unit.w);
            memcpy(&tag, &drawpos[5], sizeof(rect_t));
            tag.y = tag.y - 16;
            unit.x = drawpos[5].x + drawpos[5].w;
            unit.y = drawpos[5].y - 1;
            set_font(surf, font12n);
            draw_text(surf, STR_AZIMUTH, -1, &tag, DSTF_LEFT);
            set_font(surf, font18m);
            draw_text(surf, &(string[3][0]), -1, &drawpos[5], DSTF_LEFT);
            set_font(surf, font16n);
            draw_text(surf, STR_MIL, -1, &unit, DSTF_CENTER);

            // draw target length
            font18m->GetStringWidth(font18m, &(string[6][0]), -1, &drawpos[8].w);
            memcpy(&tag, &drawpos[8], sizeof(rect_t));
            tag.y = tag.y - 16;
            set_font(surf, font12n);
            draw_text(surf, STR_TARGET_LENGTH, -1, &tag, DSTF_LEFT);
            set_font(surf, font18m);
            draw_text(surf, &(string[6][0]), -1, &drawpos[8], DSTF_LEFT);

            // draw target width
            drawpos[9].x = drawpos[8].x + drawpos[8].w + 15;
            font18m->GetStringWidth(font18m, &(string[7][0]), -1, &drawpos[9].w);
            memcpy(&tag, &drawpos[9], sizeof(rect_t));
            tag.y = tag.y - 16;
            set_font(surf, font12n);
            draw_text(surf, STR_TARGET_WIDTH, -1, &tag, DSTF_LEFT);
            set_font(surf, font18m);
            draw_text(surf, &(string[7][0]), -1, &drawpos[9], DSTF_LEFT);

            // draw target radius
            drawpos[10].x = drawpos[9].x + drawpos[9].w + 15;
            font18m->GetStringWidth(font18m, &(string[8][0]), -1, &drawpos[10].w);
            memcpy(&tag, &drawpos[10], sizeof(rect_t));
            tag.y = tag.y - 16;
            set_font(surf, font12n);
            draw_text(surf, STR_TARGET_RADIUS, -1, &tag, DSTF_LEFT);
            set_font(surf, font18m);
            draw_text(surf, &(string[8][0]), -1, &drawpos[10], DSTF_LEFT);

            // draw lateral shift
            drawpos[11].x = drawpos[10].x + drawpos[10].w + 15;
            font18m->GetStringWidth(font18m, &(string[9][0]), -1, &drawpos[11].w);
            memcpy(&tag, &drawpos[11], sizeof(rect_t));
            tag.y = tag.y - 16;
            set_font(surf, font12n);
            draw_text(surf, STR_TARGET_LSHIFT, -1, &tag, DSTF_LEFT);
            set_font(surf, font18m);
            draw_text(surf, &(string[9][0]), -1, &drawpos[11], DSTF_LEFT);

            // draw range shift
            drawpos[12].x = drawpos[11].x + drawpos[11].w + 15;
            font18m->GetStringWidth(font18m, &(string[10][0]), -1, &drawpos[12].w);
            memcpy(&tag, &drawpos[12], sizeof(rect_t));
            tag.y = tag.y - 16;
            set_font(surf, font12n);
            draw_text(surf, STR_TARGET_RSHIFT, -1, &tag, DSTF_LEFT);
            set_font(surf, font18m);
            draw_text(surf, &(string[10][0]), -1, &drawpos[12], DSTF_LEFT);

            // draw foward elevation
            //drawpos[6].x = drawpos[5].x + drawpos[5].w + unit.w + 15;
            drawpos[6].x = drawpos[11].x;
            font18m->GetStringWidth(font18m, &(string[4][0]), -1, &drawpos[6].w);
            font16n->GetStringWidth(font16n, STR_MIL, -1, &unit.w);
            memcpy(&tag, &drawpos[6], sizeof(rect_t));
            tag.y = tag.y - 16;
            unit.x = drawpos[6].x + drawpos[6].w;
            unit.y = drawpos[6].y - 1;
            set_font(surf, font12n);
            draw_text(surf, STR_ELEVATION, -1, &tag, DSTF_LEFT);
            set_font(surf, font18m);
            draw_text(surf, &(string[4][0]), -1, &drawpos[6], DSTF_LEFT);
            set_font(surf, font16n);
            draw_text(surf, STR_MIL, -1, &unit, DSTF_CENTER);

            // draw taraget attitude
            //drawpos[7].x = drawpos[6].x + drawpos[6].w + unit.w + 15;
            drawpos[7].x = drawpos[12].x;
            font18m->GetStringWidth(font18m, &(string[5][0]), -1, &drawpos[7].w);
            font16n->GetStringWidth(font16n, STR_MIL, -1, &unit.w);
            memcpy(&tag, &drawpos[7], sizeof(rect_t));
            tag.y = tag.y - 16;
            unit.x = drawpos[7].x + drawpos[7].w;
            unit.y = drawpos[7].y - 1;
            set_font(surf, font12n);
            draw_text(surf, STR_TARGET_ATTITUDE, -1, &tag, DSTF_LEFT);
            set_font(surf, font18m);
            draw_text(surf, &(string[5][0]), -1, &drawpos[7], DSTF_LEFT);
            set_font(surf, font16n);
            draw_text(surf, STR_MIL, -1, &unit, DSTF_CENTER);

            // draw acqustion time
            drawpos[2].x = drawpos[12].x;
            font18m->GetStringWidth(font18m, &(string[0][0]), -1, &drawpos[2].w);
            memcpy(&tag, &drawpos[2], sizeof(rect_t));
            tag.y = tag.y - 16;
            set_font(surf, font12n);
            draw_text(surf, STR_TAQTIME, -1, &tag, DSTF_LEFT);
            set_font(surf, font18m);
            draw_text(surf, &(string[0][0]), -1, &drawpos[2], DSTF_LEFT);
        }
        else
            return;
    }
    else
        return;
}


void
draw_magnetic_compensation_status(window_t *window, struct magcomp_interface *magcomp, struct dmc_interface *dmc)
{
    int dir = 0;
    int mov[2] = {0};
    int mil[3] = {0};
    int daqpos[3] = {0};
    char progress[16] = {0};
    char direction[16] = {0};
    double deg[3] = {0};
    IDirectFBSurface *surf = window->guiSurface;

    rect_t drawpos[] =
    {
        {360, 170,  80,  30},           /* 0 : title                        */
        {370, 285,  60,  30},           /* 1 : move direction string        */
        {360, 400,  80,  30},           /* 2 : compensation progress string */
        {385, 255,  30,  30},	        /* 3 : proximity indicator - top    */
        {385, 315,  30,  30},	        /* 4 : proximity indicator - bottom */
        {350, 285,  30,  30},  	        /* 5 : proximity indicator - left   */
        {420, 285,  30,  30},	        /* 6 : proximity indicator - right  */
    };


    memset(progress, 0x00, sizeof(progress));
    memset(direction, 0x00, sizeof(direction));
    dir = magcomp->getDirection(magcomp);
    snprintf(progress, sizeof(progress), "%02d / 12", magcomp->getCurrentQcp(magcomp));

    switch (dir)
    {
    case MAGCOMP_MOVE_DIR_STOP:
        snprintf(direction, sizeof(direction), STR_STOP_MOVE);
        break;

    case MAGCOMP_MOVE_DIR_CW:
        snprintf(direction, sizeof(direction), STR_TURN_CW);
        break;

    case MAGCOMP_MOVE_DIR_CCW:
        snprintf(direction, sizeof(direction), STR_TURN_CCW);
        break;

    case MAGCOMP_MOVE_DIR_UP:
        snprintf(direction, sizeof(direction), STR_TURN_UP);
        break;

    case MAGCOMP_MOVE_DIR_DOWN:
        snprintf(direction, sizeof(direction), STR_TURN_DOWN);
        break;

    case MAGCOMP_MOVE_DIR_TCW:
        snprintf(direction, sizeof(direction), STR_TILT_CW);
        break;

    case MAGCOMP_MOVE_DIR_TCCW:
        snprintf(direction, sizeof(direction), STR_TILT_CCW);
        break;

    default:
        snprintf(direction, sizeof(direction), "\uC815\uC9C0");
        break;
    }

    dmc->getAzimuth(dmc, &mil[0], &deg[0]);
    dmc->getElevation(dmc, &mil[1], &deg[1]);
    dmc->getBank(dmc, &mil[2], &deg[2]);
    magcomp->getPosition(magcomp, &daqpos[0], &daqpos[1], &daqpos[2]);

    if ((daqpos[0] == 0) && (deg[0] > 180.0))
        mov[0]  = abs((int)lround(360.0 - deg[0]));
    else
        mov[0] = abs((int)lround(daqpos[0] - deg[0]));

    mov[1] = abs((int)lround(daqpos[1] - deg[1]));

    for (int i = 0; i < DIM(mov); i++)
    {
        if (mov[i] <= 10)
            mov[i] = 0;
        else if (mov[i] > 60)
            mov[i] = 50;
        else
            mov[i] = mov[i] - 10;
    }

    drawpos[3].y = drawpos[3].y - mov[1];
    drawpos[4].y = drawpos[4].y + mov[1];
    drawpos[5].x = drawpos[5].x - mov[0];
    drawpos[6].x = drawpos[6].x + mov[0];

    if (dir == MAGCOMP_MOVE_DIR_STOP)
        set_draw_color(surf, 255, 0, 0, 0);
    else
        set_draw_color(surf, 0, 255, 0, 0);

    set_font(surf, window->normalFont[FONT_HEIGHT_22]);
    draw_text(surf, STR_FILL_CURSOR_DOWN , -1, &drawpos[3], DSTF_CENTER);
    draw_text(surf, STR_FILL_CURSOR_UP   , -1, &drawpos[4], DSTF_CENTER);
    draw_text(surf, STR_FILL_CURSOR_RIGHT, -1, &drawpos[5], DSTF_CENTER);
    draw_text(surf, STR_FILL_CURSOR_LEFT , -1, &drawpos[6], DSTF_CENTER);
    draw_text(surf, direction, -1, &drawpos[1], DSTF_CENTER);
    set_draw_color(surf, 0, 255, 0, 0);
    set_font(surf, window->normalFont[FONT_HEIGHT_16]);
    draw_text(surf, STR_TARGET_COMP_DMC, -1, &drawpos[0], DSTF_CENTER);
    set_font(surf, window->monoFont[FONT_HEIGHT_18]);
    draw_text(surf, progress, -1, &drawpos[2], DSTF_CENTER);
}


void
draw_pbit_progress(window_t *window, struct device_interface *device)
{
    IDirectFBSurface *surf = window->guiSurface;

    char *module[] =
    {
        STR_BIT_MAINBOARD,
        STR_BIT_LRF,
        STR_BIT_DMC,
        STR_BIT_GNSS,
        STR_BIT_DVO,
        STR_BIT_IRCAM
    };

    rect_t drawpos[] =
    {
        {  0,   0, 800, 600},
        {270, 180, 260, 180},
        {270, 180, 260,  30},
        {280, 210, 120,  24},
        {400, 210, 120,  24},
    };

    set_font(surf, window->normalFont[FONT_HEIGHT_20]);
    set_draw_color(surf, 0, 0, 0, 0);
    draw_fill_rectangle(surf, &drawpos[0]);
    set_draw_color(surf, 0, 255, 0, 0);
    draw_rectangle(surf, &drawpos[1], true);
    draw_text(surf, STR_BIT_BIT, -1, &drawpos[2], DSTF_CENTER);

    for (int i = 0; i < DEVICE_NUM_BIT_ITEMS; i++)
    {
        set_font(surf, window->normalFont[FONT_HEIGHT_16]);
        draw_text(surf, module[i], -1, &drawpos[3], DSTF_LEFT);

        switch (device->getBitProgress(device, i))
        {
        case DEVICE_BIT_PROGRESS_STANDBY:
            draw_text(surf, STR_BIT_STANDBY, -1, &drawpos[4], DSTF_RIGHT);
            break;

        case DEVICE_BIT_PROGRESS_INPROC:
            draw_text(surf, STR_BIT_INPROC, -1, &drawpos[4], DSTF_RIGHT);
            break;

        case DEVICE_BIT_PROGRESS_DONE:
            if (device->getBitResult(device, i) == DEVICE_BIT_RESULT_OK)
                draw_text(surf, STR_BIT_OK, -1, &drawpos[4], DSTF_RIGHT);
            else
            {
                set_draw_color(surf, 255, 0, 0, 0);
                draw_text(surf, STR_BIT_ERROR, -1, &drawpos[4], DSTF_RIGHT);
                set_draw_color(surf, 0, 255, 0, 0);
            }

            break;
        }

        drawpos[3].y += 24;
        drawpos[4].y += 24;
    }
}


void
draw_ibit_progress(window_t *window, struct main_interface *mainif)
{
    char buf[32] = {0};
    IDirectFBSurface *surf = window->guiSurface;
    struct mcu_interface *mcu = mainif->device->mcu;
    struct msgproc_interface *msgproc = mainif->msgproc;
    struct device_interface *device = mainif->device;

    char *module[] =
    {
        STR_BIT_MAINBOARD,
        STR_BIT_LRF,
        STR_BIT_DMC,
        STR_BIT_GNSS,
        STR_BIT_DVO,
        STR_BIT_IRCAM
    };

    rect_t drawpos[] =
    {
        {  0,   0, 800, 600},
        {270, 132, 260, 228},
        {270, 132, 260,  30},
        {280, 162, 120,  24},
        {400, 162, 120,  24},
    };

    set_font(surf, window->normalFont[FONT_HEIGHT_20]);
    set_draw_color(surf, 0, 0, 0, 0);
    draw_fill_rectangle(surf, &drawpos[0]);
    set_draw_color(surf, 0, 255, 0, 0);
    draw_rectangle(surf, &drawpos[1], true);
    draw_text(surf, STR_BIT_BIT, -1, &drawpos[2], DSTF_CENTER);

    for (int i = 0; i < DEVICE_NUM_BIT_ITEMS; i++)
    {
        set_font(surf, window->normalFont[FONT_HEIGHT_16]);
        draw_text(surf, module[i], -1, &drawpos[3], DSTF_LEFT);

        switch (device->getBitProgress(device, i))
        {
        case DEVICE_BIT_PROGRESS_STANDBY:
            draw_text(surf, STR_BIT_STANDBY, -1, &drawpos[4], DSTF_RIGHT);
            break;

        case DEVICE_BIT_PROGRESS_INPROC:
            draw_text(surf, STR_BIT_INPROC, -1, &drawpos[4], DSTF_RIGHT);
            break;

        case DEVICE_BIT_PROGRESS_DONE:
            if (device->getBitResult(device, i) == DEVICE_BIT_RESULT_OK)
                draw_text(surf, STR_BIT_OK, -1, &drawpos[4], DSTF_RIGHT);
            else
            {
                set_draw_color(surf, 255, 0, 0, 0);
                draw_text(surf, STR_BIT_ERROR, -1, &drawpos[4], DSTF_RIGHT);
                set_draw_color(surf, 0, 255, 0, 0);
            }

            break;
        }

        drawpos[3].y += 24;
        drawpos[4].y += 24;
    }

    draw_text(surf, STR_BIT_DMIF, -1, &drawpos[3], DSTF_LEFT);

    switch (msgproc->getTestProgress(msgproc))
    {
    case MSGPROC_TEST_PROGRESS_STANDBY:
        draw_text(surf, STR_BIT_STANDBY, -1, &drawpos[4], DSTF_RIGHT);
        break;

    case MSGPROC_TEST_PROGRESS_INPROC:
        draw_text(surf, STR_BIT_INPROC, -1, &drawpos[4], DSTF_RIGHT);
        break;

    default:
        if (msgproc->getTestResult(msgproc) == MSGPROC_TEST_RESULT_OK)
            draw_text(surf, STR_BIT_OK, -1, &drawpos[4], DSTF_RIGHT);
        else
        {
            set_draw_color(surf, 255, 0, 0, 0);
            draw_text(surf, STR_BIT_DMIF_ERROR, -1, &drawpos[4], DSTF_RIGHT);
            set_draw_color(surf, 0, 255, 0, 0);
        }
        break;
    }

    drawpos[3].y += 24;
    drawpos[4].y += 24;

    if (mcu->getPwrSrcType(mcu) == MCU_PWRSRC_CELL)
    {
        if (mcu->getBatteryType(mcu) == MCU_CELL_PRIMARY)
            draw_text(surf, STR_BIT_PRIMARY_CELL_VOLT, -1, &drawpos[3], DSTF_LEFT);
        else
            draw_text(surf, STR_BIT_SECONDARY_CELL_VOLT, -1, &drawpos[3], DSTF_LEFT);
    }
    else
        draw_text(surf, STR_BIT_EXTDC_VOLT, -1, &drawpos[3], DSTF_LEFT);

    snprintf(buf, sizeof(buf), "%.2fV", mcu->getPwrSrcVolt(mcu) / 1000.0);
    draw_text(surf, buf, -1, &drawpos[4], DSTF_RIGHT);
}




void
draw_update_progress(window_t *window, struct main_interface *mainif)
{
	char buf[32] = {0};
	int updatetask = 0;
	int writesize = 0;
	int totalsize = 0;
	double writeprogress = 0.0;
	struct ispmgr_interface *ispmgr = mainif->device->ispmgr;
	IDirectFBSurface *surf = window->guiSurface;

	rect_t position[] =
	{
			{260,	250,	280,	100},
			{260, 	250,	280,	30},
			{260,	285,	280,	30},
			{260,	310,	280,	20},
			{200,	180,	400,	40}
	};


	ispmgr->getUpdateTask(ispmgr, &updatetask);
	set_font(surf, window->normalFont[FONT_HEIGHT_20]);
	set_draw_color(surf, 255, 0, 0, 0);
	draw_text(surf, STR_UPDATE_WARNING, -1, &position[4], DSTF_CENTER);

	set_font(surf, window->normalFont[FONT_HEIGHT_18]);
	set_draw_color(surf, 0, 255, 0, 0);
	draw_rectangle(surf, &position[0], true);
	draw_text(surf, STR_FWUPDATE_UPDATE, -1, &position[1], DSTF_CENTER);

	switch(updatetask)
	{
	case ISPMGR_WRITE_UBOOT:
		draw_text(surf, STR_UPDATE_BOOTLOADER, -1, &position[2], DSTF_CENTER);
		break;

	case ISPMGR_WRITE_UIMAGE:
		draw_text(surf, STR_UPDATE_UIMAGE, -1, &position[2], DSTF_CENTER);
		break;

	case ISPMGR_ERASE_ROOTFS:
		draw_text(surf, STR_ERASE_ROOTFS, -1, &position[2], DSTF_CENTER);
		break;

	case ISPMGR_WRITE_ROOTFS:
		draw_text(surf, STR_UPDATE_ROOTFS, -1, &position[2], DSTF_CENTER);
		break;
	}

	ispmgr->getUpdateProgress(ispmgr, &writesize, &totalsize, &writeprogress);
	memset(buf, 0x00, sizeof(buf));

	snprintf(buf, sizeof(buf), "%d KB / %d KB  (%.1f%%)",lround(writesize / 1024),
			lround(totalsize / 1024), writeprogress);

	set_font(surf, window->normalFont[FONT_HEIGHT_12]);
	draw_text(surf, buf, -1, &position[3], DSTF_CENTER);
	set_font(surf, window->normalFont[FONT_HEIGHT_16]);
}





void
draw_dialog(window_t *window)
{
    char *title = NULL;
    list_t *widget_list = NULL;
    dialog_t *dialog = NULL;
    stk_node_t *node = NULL;
    dialog_widget_t *widget = NULL;
    rect_t pos_dialog = {0, 0, 0, 0};
    rect_t pos_title  = {0, 0, 0, 0};

    pthread_mutex_lock(&window->mutex);

    if (window->focus == GUI_FOCUS_DIALOG)
    {
        node = window->dialogStack->bottom;

        while (node)
        {
            dialog = (dialog_t *) node->data;

            if (dialog)
            {
                dialog->getPosition(dialog, &pos_dialog);
                set_draw_color(window->guiSurface, 0, 0, 0, 0);
                draw_fill_rectangle(window->guiSurface, &pos_dialog);
                set_draw_color(window->guiSurface, 0, 255, 0, 0);
                draw_rectangle(window->guiSurface, &pos_dialog, true);
                title = dialog->getTitle(dialog);

                if (title)
                {
                    pos_title.x = pos_dialog.x;
                    pos_title.y = pos_dialog.y;
                    pos_title.w = pos_dialog.w;
                    pos_title.h = 30;
                    set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_18]);
                    draw_text(window->guiSurface, title, -1, &pos_title, DSTF_CENTER);
                }

                widget_list = dialog->getWidgetList(dialog);

                for(int i = 0;  i < widget_list->count; i++)
                {
                    widget = (dialog_widget_t *) list_get_node(widget_list, i)->data;

                    switch(widget->type)
                    {
                    case WIDGET_BUTTON:
                        draw_button_widget(window, dialog, widget);
                        break;

                    case WIDGET_CAPTION:
                        draw_caption_widget(window, widget);
                        break;

                    case WIDGET_GRID:
                        draw_grid_widget(window, dialog, widget);
                        break;

                    case WIDGET_QSEL:
                        draw_qsel_widget(window, dialog, widget);
                        break;

                    case WIDGET_SLIDER:
                        draw_slider_widget(window, dialog, widget);
                        break;

                    case WIDGET_SPINBOX:
                        draw_spinbox_widget(window, dialog, widget);
                        break;

                    case WIDGET_SVPLOT:
                        draw_svplot_widget(window, dialog, widget);
                        break;

                    default:
                        TLOGMSG(1, (DBGINFOFMT "unknown widget type\n", DBGINFO));
                        break;
                    }
                }
            }

            node = node->prev;
        }
    }

    pthread_mutex_unlock(&window->mutex);
}


void
draw_menu(window_t *window)
{
    int num_item = 0;
    menu_item_t *focus = NULL;
    menu_item_t *item = NULL;

    if (window->mainMenu)
    {
        pthread_mutex_lock(&window->mutex);
        num_item = window->mainMenu->getNumItems(window->mainMenu);
        focus = window->mainMenu->getFocus(window->mainMenu);
        set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_16]);

        for (int i = 0; i < num_item; i++)
        {
            item = window->mainMenu->getItem(window->mainMenu, i);

            if (focus == item)
                set_draw_color(window->guiSurface, 0, 255, 0, 0);
            else
                set_draw_color(window->guiSurface, 127, 127, 127, 0);

            draw_rectangle(window->guiSurface, &(item->position), true);
            draw_text(window->guiSurface, item->string, -1, &(item->position), DSTF_CENTER);
        }

        pthread_mutex_unlock(&window->mutex);
    }
}


void
draw_submenu(window_t *window)
{
    int nitem = 0;
    submenu_t *submenu = NULL;
    submenu_item_t *focus = NULL;
    submenu_item_t *item = NULL;

    rect_t drawpos[] =
    {
        {0, 0, 0,  0},   /* submenu position             */
        {0, 0, 0, 20},   /* submenu item position        */
        {0, 0, 8,  8},   /* submenu cursor position      */
    };


    if (window->mainMenu)
    {
        pthread_mutex_lock(&window->mutex);
        submenu = window->mainMenu->getFocus(window->mainMenu)->submenu;
        submenu->getPosition(submenu, &drawpos[0]);
        drawpos[1].x = drawpos[0].x + 20;
        drawpos[1].y = drawpos[0].y + 5;
        drawpos[1].w = drawpos[0].w - 10;
        drawpos[2].x = drawpos[0].x + 8;
        drawpos[2].y = drawpos[0].y + 11;
        nitem = submenu->getNumItems(submenu);
        focus = submenu->getFocus(submenu);

        set_draw_color(window->guiSurface, 0, 0, 0, 0);
        draw_fill_rectangle(window->guiSurface, &drawpos[0]);
        set_draw_color(window->guiSurface, 0, 255, 0, 0);
        draw_rectangle(window->guiSurface, &drawpos[0], true);
        set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_16]);

        for (int i = 0; i < nitem; i++)
        {
            item = submenu->getItem(submenu, i);

            if (item == focus)
            {
                set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_14]);
                draw_text(window->guiSurface, STR_CURSOR, -1, &drawpos[2], DSTF_CENTER);
                set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_16]);
            }

            draw_text(window->guiSurface, item->string, -1, &drawpos[1], DSTF_LEFT);
            drawpos[1].y = drawpos[1].y + 20;
            drawpos[2].y = drawpos[2].y + 20;
        }

        pthread_mutex_unlock(&window->mutex);
    }
}


void
draw_notice(window_t *window)
{
    char *string = NULL;
    rect_t position = {260, 370, 280, 100};

    switch (window->notice)
    {
    case NOTICE_INIT_MAGCOMP:
        string = STR_NOTICE_INIT_MAGCOMP;
        break;

    case NOTICE_CHANGE_OBMODE:
        string = STR_NOTICE_CHANGE_OBMODE;
        break;

    case NOTICE_CHANGE_NDFILTER:
        string = STR_NOTICE_CHANGE_FILTER;
        break;

    case NOTICE_EXIT_MAGCOMP:
        string = STR_NOTICE_EXIT_MAGCOMP;
        break;

    case NOTICE_WAIT_CALC_MAGPARM:
        string = STR_NOTICE_CALC_MAGCOMP;
        break;

    case NOTICE_WAIT_REPONSE:
        string = STR_NOTICE_WAIT_RESPONSE;
        break;

    case NOTICE_DMIF_TEST:
        string = STR_NOTICE_DMIF_TEST;
        break;

    case NOTICE_ENTER_STANDBY_MODE:
        string = STR_NOTICE_ENTER_STANDBY_MODE;
        break;

    case NOTICE_EXIT_STANDBY_MODE:
        string = STR_NOTICE_EXIT_STANDBY_MODE;
        break;

    default:
        return;
    }

    set_draw_color(window->guiSurface, 0, 0, 0, 0);
    draw_fill_rectangle(window->guiSurface, &position);
    set_draw_color(window->guiSurface, 0, 255, 0, 0);
    draw_rectangle(window->guiSurface, &position, true);
    set_font(window->guiSurface, window->normalFont[FONT_HEIGHT_16]);
    draw_text(window->guiSurface, string, -1, &position, DSTF_CENTER);
}
