/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_manager.c
        external/internal function implementations of gui manager
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/


#include <directfb.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/logger.h"
#include "gui/gui_window.h"


/* macro defines : font path */
#define NORMAL_FONT_PATH            "/root/mfod/resource/font/KoPubDotumBold.ttf"
#define MSPACE_FONT_PATH            "/root/mfod/resource/font/Inconsolata-Bold.ttf"


/* global variable : preload image path */
static char *preload_image_path[NUM_PRELOAD_IMAGES] =
{
    "/root/mfod/resource/image/reticle_dv.bmp",
    "/root/mfod/resource/image/reticle_ir.bmp",
    "/root/mfod/resource/image/reticle_2x.bmp",
    "/root/mfod/resource/image/azimuth_indicator_dv.bmp",
    "/root/mfod/resource/image/azimuth_indicator_ir.bmp",
    "/root/mfod/resource/image/elevation_indicator_dv.bmp",
    "/root/mfod/resource/image/elevation_indicator_ir.bmp",
    "/root/mfod/resource/image/azimuth_pointer_dv.bmp",
    "/root/mfod/resource/image/azimuth_pointer_ir.bmp",
    "/root/mfod/resource/image/elevation_pointer_dv.bmp",
    "/root/mfod/resource/image/elevation_pointer_ir.bmp",
    "/root/mfod/resource/image/svplot_background.bmp",
    "/root/mfod/resource/image/red_indicator_dv.bmp",
    "/root/mfod/resource/image/red_indicator_ir.bmp",
    "/root/mfod/resource/image/green_indicator_dv.bmp",
    "/root/mfod/resource/image/green_indicator_ir.bmp",
    "/root/mfod/resource/image/power_plug.bmp",
	"/root/mfod/resource/image/lrftrg_indicator_left_dv.bmp",
    "/root/mfod/resource/image/lrftrg_indicator_right_dv.bmp",
    "/root/mfod/resource/image/lrftrg_indicator_left_ir.bmp",
	"/root/mfod/resource/image/lrftrg_indicator_right_ir.bmp",
};


window_t *
gui_window_create(IDirectFB *dfb)
{
    int ret = -1;
    window_t *window = NULL;
    IDirectFBImageProvider *provider = NULL;
    DFBFontDescription font_desc;
    DFBSurfaceDescription surf_desc;

    if (!dfb)
    {
        TLOGMSG(1, (DBGINFOFMT "null directfb interface\n", DBGINFO));
        goto quit;
    }

    window = malloc(sizeof(window_t));

    if (!window)
    {
        TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));
        goto quit;
    }
    else
    {
        memset(window, 0x00, sizeof(window_t));
        window->dfb   = dfb;
    }

    /* create directfb primary surface */
    memset(&surf_desc, 0x00, sizeof(DFBSurfaceDescription));
    surf_desc.flags = DSDESC_CAPS;
    surf_desc.caps = DSCAPS_PRIMARY | DSCAPS_DOUBLE;

    if (dfb->CreateSurface(dfb, &surf_desc, &window->guiSurface) != DFB_OK)
    {
        TLOGMSG(1, (DBGINFOFMT "failed to create directfb surface\n", DBGINFO));
        goto quit;
    }

    /* create directfb fonts */
    for (int i = 0; i < NUM_FONT_HEIGHTS; i++)
    {
        memset(&font_desc, 0x00, sizeof(DFBFontDescription));
        font_desc.flags = DFDESC_HEIGHT;
        font_desc.height = BASE_FONT_HEIGHT + (2 * i);

        if (dfb->CreateFont(dfb, NORMAL_FONT_PATH, &font_desc, &(window->normalFont[i])) != DFB_OK)
        {
            TLOGMSG(1, (DBGINFOFMT "failed to create normal font\n", DBGINFO));
            goto quit;
        }

        if (dfb->CreateFont(dfb, MSPACE_FONT_PATH, &font_desc, &(window->monoFont[i])) != DFB_OK)
        {
            TLOGMSG(1, (DBGINFOFMT "failed to create monospace font\n", DBGINFO));
            goto quit;
        }
    }

    /* create directfb image surfaces*/
    for (int i = 0; i < NUM_PRELOAD_IMAGES; i++)
    {
        if (dfb->CreateImageProvider(dfb, preload_image_path[i], &provider) != DFB_OK)
        {
            TLOGMSG(1, (DBGINFOFMT "failed to create image provier\n", DBGINFO));
            TLOGMSG(1, (DBGINFOFMT "%s\n", DBGINFO, preload_image_path[i]));
            goto quit;
        }

        memset(&surf_desc, 0x00, sizeof(DFBSurfaceDescription));
        provider->GetSurfaceDescription(provider, &surf_desc);

        if (dfb->CreateSurface(dfb, &surf_desc, &(window->imageSurface[i])) != DFB_OK)
        {
            provider->Release(provider);
            TLOGMSG(1, (DBGINFOFMT "failed to create image surface", DBGINFO));
            goto quit;
        }

        provider->RenderTo(provider, window->imageSurface[i], NULL);
        provider->Release(provider);
    }

    /* create dialog stack */
    window->dialogStack = stack_create();

    if (!window->dialogStack)
    {
        TLOGMSG(1, (DBGINFOFMT "failed to create dialog stack\n", DBGINFO));
        goto quit;
    }

    if (pthread_mutex_init(&window->mutex, NULL) != 0)
    {
        TLOGMSG(1, (DBGINFOFMT "failed to init mutex\n", DBGINFO));
        goto quit;
    }
    else
        ret = 0;

quit:
    if (ret != 0)
    {
        /* clean-up */
        gui_window_destroy(window);
        window = NULL;
    }
    else
    {
        window->notice  = NOTICE_NONE;
        window->focus   = GUI_FOCUS_MENU;
        window->mainMenu = NULL;
        window->subMenu = NULL;
        TLOGMSG(1, ("create window interface\n"));
    }

    return window;
}


int
gui_window_destroy(window_t *window)
{
    int ret = 0;

    if (window)
    {
        pthread_mutex_destroy(&window->mutex);

        if (window->dialogStack)
            stack_destroy(window->dialogStack);

        for (int i = 0; i < NUM_PRELOAD_IMAGES; i++)
        {
            if (window->imageSurface[i])
                window->imageSurface[i]->Release(window->imageSurface[i]);
        }

        for (int i = 0; i < NUM_FONT_HEIGHTS; i++)
        {
            if (window->normalFont[i])
                window->normalFont[i]->Release(window->normalFont[i]);

            if (window->monoFont[i])
                window->monoFont[i]->Release(window->monoFont[i]);
        }

        if (window->guiSurface)
            window->guiSurface->Release(window->guiSurface);

        free(window);

        TLOGMSG(1, ("destroy window interface\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null window interface\n", DBGINFO));
    }

    return ret;
}
