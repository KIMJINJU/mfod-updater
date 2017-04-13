/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    eis.c
        external/internal function implementations of electronic image stabilizer (eis) interface
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/


#include <stdio.h>
#include <stdlib.h>

#include "core/logger.h"
#include "modules/eis.h"
#include "morpho/morpho_error.h"
#include "morpho/morpho_image_data.h"
#include "morpho/morpho_video_refiner.h"


/* macro defines */
#define EIS_NUM_SRCBUFS          3

#define EIS_SRC_WIDTH               608
#define EIS_SRC_HEIGHT              456
#define EIS_OUT_WIDTH               552
#define EIS_OUT_HEIGHT              416

#define EIS_IMGFMTSTR_YUV420P       "YUV420_PLANAR"
#define EIS_IMGFMTSTR_YUV420SP      "YUV420_SEMIPLANAR"
#define EIS_IMGFMTSTR_YVU420SP      "YVU420_SEMIPLANAR"
#define EIS_IMGFMT_YUV420P          0
#define EIS_IMGFMT_YUV420SP         1


/* strcuture declaration : eis attributes */
struct eis_attribute
{
    /* external interface */
    struct eis_interface extif;

    /* internal attribute for eis interface */
    int running;
    int srcWidth;
    int srcHeight;
    int outWidth;
    int outHeight;
    int imgFormat;
    int srcIndex;

    void *eisBuffer;

    morpho_ImageData *srcBuffer[EIS_NUM_SRCBUFS];
    morpho_ImageData *outBuffer;
    morpho_VideoRefiner videoRefiner;
};


static morpho_ImageData *
eis_alloc_imgbuf(int imgwid, int imghgt, int imgfmt)
{
    void *addr = NULL;
    morpho_ImageData *imgbuf = malloc(sizeof(morpho_ImageData));

    if (imgbuf)
    {
        addr = malloc(imgwid * imghgt * 1.5);

        if (addr)
        {
            imgbuf->width = imgwid;
            imgbuf->height = imghgt;

            switch(imgfmt)
            {
            case EIS_IMGFMT_YUV420P:
                imgbuf->dat.planar.y = addr;
                imgbuf->dat.planar.u = (char *) addr + imgbuf->width + imgbuf->height;
                imgbuf->dat.planar.v = (char *) addr + imgbuf->width + imgbuf->height + (imgbuf->width + imgbuf->height / 4);
                break;

            case EIS_IMGFMT_YUV420SP:
                imgbuf->dat.semi_planar.y = addr;
                imgbuf->dat.semi_planar.uv = (char *) addr + imgbuf->width * imgbuf->height;
                break;

            default:
                free(addr);
                free(imgbuf);
                imgbuf = NULL;
                TLOGMSG(1, (DBGINFOFMT "unknown image format\n", DBGINFO));
                break;
            }
        }
        else
        {
            free(imgbuf);
            imgbuf = NULL;
            TLOGMSG(1, (DBGINFOFMT "failed to alloc memory for buf\n", DBGINFO));
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to alloc memory for imgbuf\n", DBGINFO));

    return imgbuf;
}


static void
eis_free_imgbuf(morpho_ImageData *imgbuf)
{
    if (imgbuf)
    {
        free(imgbuf->dat.p);
        free(imgbuf);
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null imgbuf\n", DBGINFO));
}


static int
eis_start(struct eis_interface *eis)
{
    int ret = 0;
    struct eis_attribute *this = (struct eis_attribute *) eis;

    if (this)
    {
        if (morpho_VideoRefiner_start(&this->videoRefiner) == MORPHO_OK)
        {
            this->srcIndex = 0;
            this->running = 1;
            TLOGMSG(1, ("start image stablilizer\n"));
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to start image stabilizer\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null image stablilizer interface\n", DBGINFO));
    }

    return ret;
}


static int
eis_stop(struct eis_interface *eis)
{
    int ret = 0;
    struct eis_attribute *this = (struct eis_attribute *) eis;

    if (this)
    {
        if (morpho_VideoRefiner_finalize(&this->videoRefiner) == MORPHO_OK)
        {
            this->running = 0;
            TLOGMSG(1, ("stop image stabilizer\n"));
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to stop image stabilizer\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null image stablilizer interface\n", DBGINFO));
    }

    return ret;
}


static int
eis_process(struct eis_interface *eis)
{
    int ret = 0;
    int curr = 0;
    int prv1 = 0;
    int prv2 = 0;
    struct eis_attribute *this = (struct eis_attribute *) eis;

    if (this)
    {
        curr = this->srcIndex;
        prv1 = (this->srcIndex + 2) % 3;
        prv2 = (this->srcIndex + 1) % 3;

        morpho_VideoRefiner_process(&this->videoRefiner, this->outBuffer, this->srcBuffer[curr],
                                    this->srcBuffer[prv1], this->srcBuffer[prv2]);

        this->srcIndex = (this->srcIndex + 1) % 3;
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null image stablilizer interface\n", DBGINFO));
    }

    return ret;
}


static void *
eis_get_srcbuf(struct eis_interface *eis)
{
    void *buf = NULL;
    struct eis_attribute *this = (struct eis_attribute *) eis;

    if (this)
        buf = this->srcBuffer[this->srcIndex]->dat.p;
    else
        TLOGMSG(1, (DBGINFOFMT "null image stablilizer interface\n", DBGINFO));

    return buf;
}


static void *
eis_get_outbuf(struct eis_interface *eis)
{
    void *buf = NULL;
    struct eis_attribute *this = (struct eis_attribute *) eis;

    if (this)
        buf = this->outBuffer->dat.p;
    else
        TLOGMSG(1, (DBGINFOFMT "null image stablilizer interface\n", DBGINFO));

    return buf;
}


static int
eis_init_attribute(struct eis_interface *eis)
{
    int ret = 0;
    int szbuf = 0;
    struct eis_attribute *this = (struct eis_attribute *) eis;

    if (this)
    {
        this->imgFormat = EIS_IMGFMT_YUV420SP;
        this->srcWidth = EIS_SRC_WIDTH;
        this->srcHeight = EIS_SRC_HEIGHT;
        this->outWidth = EIS_OUT_WIDTH;
        this->outHeight = EIS_OUT_HEIGHT;

        szbuf = morpho_VideoRefiner_getBufferSize(this->srcWidth, this->srcHeight, EIS_IMGFMTSTR_YUV420SP,
                                                  MORPHO_VIDEO_REFINER_MOTION_4DOF_SOFT,
                                                  MORPHO_VIDEO_REFINER_ACCURACY_HIGH,
                                                  MORPHO_VIDEO_REFINER_MODE_MOVIE_SOLID, 1);

        if (szbuf == 0)
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to get buffer size", DBGINFO));
            goto fail_init_attr;
        }

        this->eisBuffer = malloc(szbuf);

        if (!this->eisBuffer)
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to alloc eisbuf memory\n", DBGINFO));
            goto fail_init_attr;
        }

        ret = morpho_VideoRefiner_initialize(&this->videoRefiner, this->eisBuffer, szbuf, this->srcWidth, this->srcHeight,
                                             this->outWidth, this->outHeight, EIS_IMGFMTSTR_YUV420SP,
                                             MORPHO_VIDEO_REFINER_MOTION_4DOF_SOFT, MORPHO_VIDEO_REFINER_ACCURACY_HIGH);

        if (ret != MORPHO_OK)
        {
            ret = -1;
            free(this->eisBuffer);
            TLOGMSG(1, (DBGINFOFMT "failed to initialize image stablilizer\n", DBGINFO));
            goto fail_init_attr;
        }

        morpho_VideoRefiner_setMode(&this->videoRefiner, MORPHO_VIDEO_REFINER_MODE_MOVIE_SOLID);
        morpho_VideoRefiner_setOutputImageSize(&this->videoRefiner, EIS_OUT_WIDTH, EIS_OUT_HEIGHT);
        morpho_VideoRefiner_setFrameRate(&this->videoRefiner, 30);
        morpho_VideoRefiner_setViewAngle(&this->videoRefiner, MORPHO_VIDEO_REFINER_ANGLE_TYPE_HORIZONTAL, 5.6);
        morpho_VideoRefiner_setRollingShutterCoeff(&this->videoRefiner, 80);
        morpho_VideoRefiner_setImageGyroTimeLag(&this->videoRefiner, 0);
        morpho_VideoRefiner_setDelayFrameNum(&this->videoRefiner, 1);
        morpho_VideoRefiner_setFixLevel(&this->videoRefiner, 7);
        morpho_VideoRefiner_setMovingSubjectLevel(&this->videoRefiner, 0);
        morpho_VideoRefiner_setNoMovementLevel(&this->videoRefiner, 0);
        morpho_VideoRefiner_setFeaturelessLevel(&this->videoRefiner, 0);
        morpho_VideoRefiner_setSpatialNRLevel(&this->videoRefiner, 2);
        morpho_VideoRefiner_setTemporalLumaNRLevel(&this->videoRefiner, 2);
        morpho_VideoRefiner_setTemporalChromaNRLevel(&this->videoRefiner, 2);
        morpho_VideoRefiner_setLumaEnhanceLevel(&this->videoRefiner, 5);
        morpho_VideoRefiner_setChromaEnhanceLevel(&this->videoRefiner, 5);

        for (int i = 0; i < EIS_NUM_SRCBUFS; i++)
        {
            this->srcBuffer[i] = eis_alloc_imgbuf(this->srcWidth, this->srcHeight, this->imgFormat);

            if (!this->srcBuffer[i])
            {
                for (int j = 0; j < i; j++)
                    eis_free_imgbuf(this->srcBuffer[j]);

                ret = -1;
                free(this->eisBuffer);
                TLOGMSG(1, (DBGINFOFMT "failed to alloc source image buffer\n", DBGINFO));
                goto fail_init_attr;
            }
            else
                continue;
        }

        this->outBuffer = eis_alloc_imgbuf(this->outWidth, this->outHeight, this->imgFormat);

        if (!this->outBuffer)
        {
            ret = -1;
            free(this->eisBuffer);
            TLOGMSG(1, (DBGINFOFMT "failed to alloc output image buffer\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null eis interface\n", DBGINFO));
    }

fail_init_attr:
    return ret;
}


struct eis_interface *
eis_create(void)
{
    struct eis_interface *eis = NULL;
    struct eis_attribute *this = malloc(sizeof(struct eis_attribute));

    if (this)
    {
        eis = &(this->extif);
        eis->start     = eis_start;
        eis->stop      = eis_stop;
        eis->process   = eis_process;
        eis->getOutbuf = eis_get_outbuf;
        eis->getSrcbuf = eis_get_srcbuf;

        if (eis_init_attribute(eis) == 0)
        {
            TLOGMSG(1, ("image stablilizer input image size = %d, %d\n", this->srcWidth, this->srcHeight));
            TLOGMSG(1, ("image stablilizer output image size = %d, %d\n", this->outWidth, this->outHeight));
            TLOGMSG(1, ("image stablilizer image format = %s\n", EIS_IMGFMTSTR_YUV420SP));
            TLOGMSG(1, ("create image stablilizer interface\n"));
        }
        else
        {
            free(this);
            eis = NULL;
            TLOGMSG(1, (DBGINFOFMT "failed to create image stablizer interface\n", DBGINFO));
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));

    return eis;
}


int
eis_destroy(struct eis_interface *eis)
{
    int ret = 0;
    struct eis_attribute *this = (struct eis_attribute *) eis;

    if (this)
    {
        if (this->running == 0)
        {
            for (int i = 0; i < EIS_NUM_SRCBUFS; i++)
                eis_free_imgbuf(this->srcBuffer[i]);

            eis_free_imgbuf(this->outBuffer);
            free(this->eisBuffer);
            free(this);
            TLOGMSG(1, ("destroy image stablilizer interface\n"));
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "image stabilizer is running\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null image stablilizer interface\n", DBGINFO));
    }

    return ret;
}
