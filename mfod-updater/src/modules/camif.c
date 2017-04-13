/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    camif.c
        external/internal function implementations of camera interface
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "core/logger.h"
#include "modules/camif.h"


#define CAMIF_NUM_BUFFERS     4


/* structure declaration : mxc buffer offset */
struct v4l2_mxc_offset
{
    unsigned int u_offset;
    unsigned int v_offset;
};

/* structure declaration : camera buffer */
struct cam_buffer
{
    size_t offset;
    char *start;
    unsigned int length;
};

/* structure declaration : camera interfce attribute */
struct cam_attribute
{
    /* external interface */
    struct cam_interface extif;

    /* internal attribute */
    int camid;
    int input;
    int v4l2FileDesc;
    int srcWidth;
    int outWidth;
    int srcHeight;
    int outHeight;
    int format;
    int fps;
    char v4l2Device[128];
    struct cam_buffer buffers[CAMIF_NUM_BUFFERS];
};


static int
camif_configure(struct cam_interface *camif)
{
    int ret = 0;
    unsigned int capture_mode = 0;
    unsigned int pxfmt = 0;
    struct cam_attribute *this = (struct cam_attribute *) camif;

    struct v4l2_control ctrl;
    struct v4l2_crop    crop;
    struct v4l2_format  fmt;
    struct v4l2_fmtdesc fmtdsc;

    struct v4l2_mxc_offset      offset;
    struct v4l2_frmsizeenum     frmsz;
    struct v4l2_streamparm      parm;
    struct v4l2_requestbuffers  reqbuf;
    struct v4l2_dbg_chip_ident  chip;
    struct v4l2_buffer          buf;

    if (this)
    {
        memset(&ctrl, 0x00, sizeof(struct v4l2_control));
        memset(&crop, 0x00, sizeof(struct v4l2_crop));
        memset(&fmt, 0x00, sizeof(struct v4l2_format));
        memset(&fmtdsc, 0x00, sizeof(struct v4l2_fmtdesc));
        memset(&offset, 0x00, sizeof(struct v4l2_mxc_offset));
        memset(&frmsz, 0x00, sizeof(struct v4l2_frmsizeenum));
        memset(&parm, 0x00, sizeof(struct v4l2_streamparm));
        memset(&reqbuf, 0x00, sizeof(struct v4l2_requestbuffers));
        memset(&chip, 0x00, sizeof(struct v4l2_dbg_chip_ident));
        memset(&buf, 0x00, sizeof(struct v4l2_buffer));

        /* open v4l2 capture device */
        this->v4l2FileDesc = open(this->v4l2Device, O_RDWR, 0);

        if (this->v4l2FileDesc < 0)
        {
            TLOGMSG(1, (DBGINFOFMT "filed to open v4l2 capture device\n", DBGINFO));
            return -1;
        }

        /* get v4l2 capture device info */
        if (ioctl(this->v4l2FileDesc, VIDIOC_DBG_G_CHIP_IDENT, &chip) != 0)
        {
            close(this->v4l2FileDesc);
            TLOGMSG(1, (DBGINFOFMT "ioctl return fail(VIDIOC_DBG_G_CHIP_INDENT)\n", DBGINFO));
            return -1;
        }

        TLOGMSG(1, ("camif sensor chip = %s\n", chip.match.name));

        /* enumerate frame size */
        while(ioctl(this->v4l2FileDesc, VIDIOC_ENUM_FRAMESIZES, &frmsz) >= 0)
        {
            TLOGMSG(1, ("supported frame size = %d x %d\n", frmsz.discrete.width, frmsz.discrete.height));

            if ((this->srcWidth == (int)frmsz.discrete.width) && (this->srcHeight == (int)frmsz.discrete.height))
                break;
            else
            {
                frmsz.index++;
                capture_mode++;
            }
        }

        /* enumerate pixel format */
        while (ioctl(this->v4l2FileDesc, VIDIOC_ENUM_FMT, &fmtdsc) >= 0)
        {
            pxfmt = fmtdsc.pixelformat;
            TLOGMSG(1, ("camif supported pixel format = %c%c%c%c\n", pxfmt & 0xFF,
                      (pxfmt >> 8) && 0xFF, (pxfmt >> 16) && 0xFF, (pxfmt >> 24) && 0xFF));

            fmtdsc.index++;
        }

        /* set streaming parameters */
        parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        parm.parm.capture.timeperframe.numerator = 1;
        parm.parm.capture.timeperframe.denominator = this->fps;
        parm.parm.capture.capability = capture_mode;

        if (ioctl(this->v4l2FileDesc, VIDIOC_S_PARM, &parm) < 0)
        {
            close(this->v4l2FileDesc);
            TLOGMSG(1, (DBGINFOFMT "ioctl return fail (VIDIOC_S_PARM)\n", DBGINFO));
            return -1;
        }

        /* set courrent video input */
        if (ioctl(this->v4l2FileDesc, VIDIOC_S_INPUT, &this->input) < 0)
        {
            close(this->v4l2FileDesc);
            TLOGMSG(1, (DBGINFOFMT "ioctl return fail (VIDIOC_S_INPUT)\n", DBGINFO));
            return -1;
        }

        /* set cropping rectangle */
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c.width  = this->srcWidth;
        crop.c.height = this->srcHeight;
        crop.c.top    = 0;
        crop.c.left   = 0;

        if (ioctl(this->v4l2FileDesc, VIDIOC_S_CROP, &crop) < 0)
        {
            close(this->v4l2FileDesc);
            TLOGMSG(1, (DBGINFOFMT "ioctl return fail (VIDIOC_S_CROP)\n", DBGINFO));
            return -1;
        }

        /* set data format */
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.pixelformat  = this->format;
        fmt.fmt.pix.width        = this->outWidth;
        fmt.fmt.pix.height       = this->outHeight;
        fmt.fmt.pix.bytesperline = this->outWidth;
        fmt.fmt.pix.priv         = 0;
        fmt.fmt.pix.sizeimage    = 0;

        if (ioctl(this->v4l2FileDesc, VIDIOC_S_FMT, &fmt) < 0)
        {
            close(this->v4l2FileDesc);
            TLOGMSG(1, (DBGINFOFMT "ioctl return fail (VIDIOC_S_FMT)\n", DBGINFO));
            return -1;
        }

        /* set rotate cotrol value */
        ctrl.id = V4L2_CID_PRIVATE_BASE + 0;
        ctrl.value = 0;

        if (ioctl(this->v4l2FileDesc, VIDIOC_S_CTRL, &ctrl) < 0)
        {
            close(this->v4l2FileDesc);
            TLOGMSG(1, (DBGINFOFMT "ioctl return fail (VIDIOC_S_CTRL)\n", DBGINFO));
            return -1;
        }

        /* request video capture buffers */
        reqbuf.count  = CAMIF_NUM_BUFFERS;
        reqbuf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        reqbuf.memory = V4L2_MEMORY_MMAP;

        if (ioctl(this->v4l2FileDesc, VIDIOC_REQBUFS, &reqbuf) < 0)
        {
            close(this->v4l2FileDesc);
            TLOGMSG(1, (DBGINFOFMT "ioctl return fail (VIDIOC_REQBUFS)\n", DBGINFO));
            return -1;
        }

        /* query buffers */
        for (int i = 0; i < CAMIF_NUM_BUFFERS; i++)
        {
            memset(&buf, 0x00, sizeof(buf));
            buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index  = i;

            if (ioctl(this->v4l2FileDesc, VIDIOC_QUERYBUF, &buf) < 0)
            {
                close(this->v4l2FileDesc);
                TLOGMSG(1, ("camif : %s, ioctl return fail(VIDIOC_QUERYBUF)\n", __func__));
                return -1;
            }

            this->buffers[i].length = buf.length;
            this->buffers[i].offset = (size_t) buf.m.offset;
            this->buffers[i].start  = mmap(NULL, this->buffers[i].length, PROT_READ| PROT_WRITE,
                                           MAP_SHARED, this->v4l2FileDesc, this->buffers[i].offset);

            memset(this->buffers[i].start, 0xFF, this->buffers[i].length);
        }

        /* queue buffers */
        for(int i = 0; i < CAMIF_NUM_BUFFERS; i++)
        {
            memset(&buf, 0x00, sizeof(buf));
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index  = i;
            buf.m.offset = this->buffers[i].offset;

            if (ioctl(this->v4l2FileDesc, VIDIOC_QBUF, &buf) < 0)
            {
                close(this->v4l2FileDesc);
                TLOGMSG(1, (DBGINFOFMT "ioctl return faile(VIDIOC_QBUF)\n", DBGINFO));
                return -1;
            }
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null cam interface\n", DBGINFO));
    }

    return ret;
}


static int
camif_stream_on(struct cam_interface *camif)
{
    int ret = 0;
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    struct cam_attribute *this = (struct cam_attribute *) camif;

    if (this)
    {
        if (ioctl(this->v4l2FileDesc, VIDIOC_STREAMON, &type) == 0)
            TLOGMSG(1, ("camera stream on\n"));
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "ioctl return fail (VIDIOC_STREAMON)\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null cam interface\n", DBGINFO));
    }

    return ret;
}


static int
camif_stream_off(struct cam_interface *camif)
{
    int ret = 0;
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    struct cam_attribute *this = (struct cam_attribute *) camif;

    if (camif)
    {
        if (ioctl(this->v4l2FileDesc, VIDIOC_STREAMOFF, &type) == 0)
            TLOGMSG(1, ("camera stream off\n"));
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "ioctl return fail (VIDIOC_STREAMOFF)\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null cam interface\n", DBGINFO));
    }

    return ret;
}


static int
camif_dqbuf(struct cam_interface *camif, void *buf)
{
    int ret = 0;
    struct cam_attribute *this = (struct cam_attribute *) camif;

    if (this)
    {
        do
            ret = ioctl(this->v4l2FileDesc, VIDIOC_DQBUF, buf);
        while ( -1 == ret && EINTR == errno);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null cam interface\n", DBGINFO));
    }

    return ret;
}


static int
camif_qbuf(struct cam_interface *camif, void *buf)
{
    int ret = 0;
    struct cam_attribute *this = (struct cam_attribute *) camif;

    if (this)
    {
        do
            ret = ioctl(this->v4l2FileDesc, VIDIOC_QBUF, buf);
        while (-1 == ret && EINTR == errno);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null cam interface\n", DBGINFO));
    }

    return ret;
}


static int
camif_getfd(struct cam_interface *camif)
{
    int ret = 0;
    struct cam_attribute *this = (struct cam_attribute *) camif;

    if (this)
        ret = this->v4l2FileDesc;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null cam interface\n", DBGINFO));
    }

    return ret;
}


static void *
camif_getbuf(struct cam_interface *camif, int bufidx)
{
    void *buf = NULL;
    struct cam_attribute *this = (struct cam_attribute *) camif;

    if (this)
        buf = this->buffers[bufidx].start;
    else
        TLOGMSG(1, (DBGINFOFMT "null cam interface \n", DBGINFO));

    return buf;
}



struct cam_interface *
camif_create(int width, int height, int fps)
{
    struct cam_interface *camif = NULL;
    struct cam_attribute *this = malloc(sizeof(struct cam_attribute));

    if (this)
    {
        camif = &(this->extif);
        this->camid      = CAMIF_CAMID_PRIMARY;
        this->format     = V4L2_PIX_FMT_YUV420;
        this->input      = 1;
        this->srcWidth  = width;
        this->outWidth  = width;
        this->srcHeight = height;
        this->outHeight = height;
        this->fps        = fps;
        snprintf(this->v4l2Device, sizeof(this->v4l2Device), "/dev/video%d", this->camid);

        if (camif_configure(camif) == 0)
        {
            camif->getFileDesc = camif_getfd;
            camif->getBuf      = camif_getbuf;
            camif->onStream    = camif_stream_on;
            camif->offStream   = camif_stream_off;
            camif->queBuf      = camif_qbuf;
            camif->deqBuf    = camif_dqbuf;

            TLOGMSG(1, ("init camif interface\n"));
        }
        else
        {
            free(this);
            camif = NULL;
            TLOGMSG(1, ("camif : %s, failed to configure cam interface\n", __func__));
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null camif interface\n", DBGINFO));

    return camif;
}


int
camif_destroy(struct cam_interface *camif)
{
    int ret = 0;
    struct cam_attribute *this = (struct cam_attribute *) camif;

    if (this)
    {
        close(this->v4l2FileDesc);

        for (int i = 0; i < CAMIF_NUM_BUFFERS; i++)
            munmap(this->buffers[i].start, this->buffers[i].length);

        free(this);
        camif = NULL;
        TLOGMSG(1, ("destroy camera interface\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null cam interface\n", DBGINFO));
    }

    return ret;
}
