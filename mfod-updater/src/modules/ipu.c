/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    ipu.c
        external/internal function implementations of ipu interface
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/ipu.h>
#include <linux/mxcfb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <ipu.h>

#include "core/logger.h"
#include "modules/ipu.h"


#define PAGE_ALIGN(x)       (((x) + 4095) & ~4095)

#define IPU_FB0_NAME        "DISP3_BG"
#define IPU_FB1_NAME        "DISP3_FB"
#define IPU_FB0_PATH        "/dev/fb0"
#define IPU_FB1_PATH        "/dev/fb1"

#define IPU_OLED_XRES       800
#define IPU_OLED_YRES       600

/* structure declarations : ipu attributes */
struct ipu_attribute
{
    /* external interface */
    struct ipu_interface extif;

    /* internal attribute */
    int blank;
    int fbsel;
    int ipufd;
    int fb0fd;
    int fb1fd;

    int alphaSize;
    int inSize;
    int outSize;
    int ovSize;

    void *alphaBuffer;
    void *inBuffer0;
    void *inBuffer1;
    void *outBuffer;
    void *ovBuffer;
    void *pfb0;
    void *pfb1;
    void *vdiBuffer;

    pthread_mutex_t mtx;

    dma_addr_t  outpaddr;
    dma_addr_t  outpaddr2;
    dma_addr_t  outpaddr3;
    struct fb_var_screeninfo fb0VarScrInfo;
    struct fb_var_screeninfo fb1VarScrInfo;
    struct fb_fix_screeninfo fb0FixScrInfo;
    struct fb_fix_screeninfo fb1FixScrInfo;
    struct ipu_task task;
};


static unsigned int
ipu_fmt2bpp(unsigned int fmt)
{
    unsigned int bpp = 0;

    switch (fmt)
    {
    case IPU_PIX_FMT_RGB565:
    case IPU_PIX_FMT_YUYV:
    case IPU_PIX_FMT_YUV422P:
    case IPU_PIX_FMT_UYVY:
    case IPU_PIX_FMT_YVU422P:
        bpp = 16;
        break;

    case IPU_PIX_FMT_BGR24:
    case IPU_PIX_FMT_RGB24:
    case IPU_PIX_FMT_YUV444:
    case IPU_PIX_FMT_YUV444P:
        bpp = 24;
        break;

    case IPU_PIX_FMT_BGR32:
    case IPU_PIX_FMT_BGRA32:
    case IPU_PIX_FMT_RGB32:
    case IPU_PIX_FMT_RGBA32:
    case IPU_PIX_FMT_ABGR32:
        bpp = 32;
        break;

    case IPU_PIX_FMT_YUV420P:
    case IPU_PIX_FMT_YVU420P:
    case IPU_PIX_FMT_YUV420P2:
    case IPU_PIX_FMT_NV12:
    case IPU_PIX_FMT_TILED_NV12:
        bpp = 12;
        break;

    default:
        bpp = 8;
        break;
    }

    return bpp;
}


static int
ipu_set_alpha_channel(struct ipu_interface *ipu, int flag, int alpha)
{
    int ret = 0;
    struct ipu_attribute *this = (struct ipu_attribute *) ipu;
    struct mxcfb_gbl_alpha galp = {.enable = 0, .alpha = 0};

    if (this)
    {
        this->fb0fd = open(IPU_FB0_PATH, O_RDWR, 0);

        if (this->fb0fd > 0)
        {
            galp.enable = flag;
            galp.alpha = alpha;

            if (ioctl(this->fb0fd, MXCFB_SET_GBL_ALPHA, &galp) == 0)
                ioctl(this->fb0fd, MXCFB_WAIT_FOR_VSYNC, &galp);
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "ioctl return fail(MXCFB_SET_GBL_ALPHA)\n", DBGINFO));
            }

            close(this->fb0fd);
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to open fb0\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ipu interface\n", DBGINFO));
    }

    return ret;
}


static int
ipu_open_framebuf(struct ipu_interface *ipu)
{
    int ret = 0;
    struct ipu_attribute *this = (struct ipu_attribute *) ipu;

    if (this)
    {
        if (this->fbsel == 0)
        {
            this->fb0fd = open(IPU_FB0_PATH, O_RDWR, 0);

            if (this->fb0fd != -1)
            {
                ioctl(this->fb0fd, FBIOGET_FSCREENINFO, &this->fb0FixScrInfo);

                if (strcmp(this->fb0FixScrInfo.id, IPU_FB0_NAME))
                {
                    ioctl(this->fb0fd, FBIOGET_VSCREENINFO, &this->fb0VarScrInfo);
                    this->fb0VarScrInfo.xres = IPU_OLED_XRES;
                    this->fb0VarScrInfo.yres = IPU_OLED_YRES;
                    this->fb0VarScrInfo.xres_virtual = IPU_OLED_XRES;
                    this->fb0VarScrInfo.yres_virtual = IPU_OLED_YRES;
                    this->fb0VarScrInfo.activate |= FB_ACTIVATE_FORCE;
                    this->fb0VarScrInfo.vmode  |= FB_VMODE_YWRAP;
                    this->fb0VarScrInfo.nonstd = this->task.output.format;
                    this->fb0VarScrInfo.bits_per_pixel = ipu_fmt2bpp(this->task.output.format);

                    if (ioctl(this->fb0fd, FBIOPUT_VSCREENINFO, &this->fb0VarScrInfo) < 0 )
                    {
                        ret = -1;
                        close(this->fb0fd);
                        TLOGMSG(1, (DBGINFOFMT "failed to set fb0 parameters\n", DBGINFO));
                    }
                    else
                    {
                        ioctl(this->fb0fd, FBIOGET_VSCREENINFO, &this->fb0VarScrInfo);
                        ioctl(this->fb0fd, FBIOGET_FSCREENINFO, &this->fb0FixScrInfo);

                        this->pfb0 =
                            mmap(NULL, this->fb0VarScrInfo.xres * this->fb0VarScrInfo.yres * this->fb0VarScrInfo.bits_per_pixel / 8,
                                 PROT_READ | PROT_WRITE, MAP_SHARED, this->fb0fd, 0);

                        this->outpaddr = this->fb0FixScrInfo.smem_start;
                        this->blank = FB_BLANK_UNBLANK;
                        ioctl(this->fb0fd, FBIOBLANK, this->blank);
                    }
                }
                else
                {
                    ret = -1;
                    close(this->fb0fd);
                    TLOGMSG(1, (DBGINFOFMT "failed to found fb0 device\n", DBGINFO));
                }
            }
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "failed to open fb0\n", DBGINFO));
            }
        }
        else
        {
            this->fb1fd = open(IPU_FB1_PATH, O_RDWR, 0);

            if (this->fb1fd > 0)
            {
                ioctl(this->fb1fd, FBIOGET_FSCREENINFO, &this->fb1FixScrInfo);

                if (strcmp(this->fb1FixScrInfo.id, IPU_FB1_NAME))
                {
                    ioctl(this->fb1fd, FBIOGET_VSCREENINFO, &this->fb1VarScrInfo);
                    this->fb1VarScrInfo.xres = IPU_OLED_XRES;
                    this->fb1VarScrInfo.yres = IPU_OLED_YRES;
                    this->fb1VarScrInfo.xres_virtual = IPU_OLED_XRES;
                    this->fb1VarScrInfo.yres_virtual = IPU_OLED_YRES;
                    this->fb1VarScrInfo.activate |= FB_ACTIVATE_FORCE;
                    this->fb1VarScrInfo.vmode  |= FB_VMODE_YWRAP;
                    this->fb1VarScrInfo.nonstd = this->task.output.format;
                    this->fb1VarScrInfo.bits_per_pixel = ipu_fmt2bpp(this->task.output.format);

                    if (ioctl(this->fb1fd, FBIOPUT_VSCREENINFO, &this->fb1VarScrInfo) != 0 )
                    {
                        ret = -1;
                        close(this->fb1fd);
                        TLOGMSG(1, (DBGINFOFMT "failed to set fb1 parameters\n", DBGINFO));
                    }
                    else
                    {
                        ioctl(this->fb1fd, FBIOGET_VSCREENINFO, &this->fb1VarScrInfo);
                        ioctl(this->fb1fd, FBIOGET_FSCREENINFO, &this->fb1FixScrInfo);

                        this->pfb1 =
                            mmap(NULL, this->fb1VarScrInfo.xres * this->fb1VarScrInfo.yres * this->fb1VarScrInfo.bits_per_pixel / 8,
                                 PROT_READ | PROT_WRITE, MAP_SHARED, this->fb1fd, 0);

                        this->outpaddr2 = this->fb1FixScrInfo.smem_start;
                        this->blank = FB_BLANK_UNBLANK;
                        ioctl(this->fb1fd, FBIOBLANK, this->blank);
                    }
                }
                else
                {
                    ret = -1;
                    close(this->fb1fd);
                    TLOGMSG(1, (DBGINFOFMT "failed to found fb1 device\n", DBGINFO));
                }
            }
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "failed to open fb1\n", DBGINFO));
            }
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ipu attribute\n"));
    }

    return ret;
}


static int
ipu_adjust_crop_size(struct ipu_interface *ipu)
{
    int ret = 0;
    struct ipu_attribute *this = (struct ipu_attribute *) ipu;

    if (this)
    {
        while (1)
        {
            ret = ioctl(this->ipufd, IPU_CHECK_TASK, &this->task);

            if (ret == IPU_CHECK_OK)
                break;
            if (ret == IPU_CHECK_ERR_SPLIT_INPUTW_OVER)
                this->task.input.crop.w -= 8;
            else if (ret == IPU_CHECK_ERR_SPLIT_INPUTH_OVER)
                this->task.input.crop.h -= 8;
            else if (ret == IPU_CHECK_ERR_SPLIT_OUTPUTW_OVER)
                this->task.output.crop.w -= 8;
            else if (ret == IPU_CHECK_ERR_SPLIT_OUTPUTH_OVER)
                this->task.output.crop.h -= 8;
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "failed to adjust crop size\n", DBGINFO));
                break;
            }
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ipu interface\n", DBGINFO));
    }

    return ret;
}


static int
ipu_deinit_attribute(struct ipu_interface *ipu)
{
    int ret = 0;
    struct ipu_attribute *this = (struct ipu_attribute *) ipu;

    if (this)
    {
        if (this->fb0fd > 0)
        {
            this->blank = FB_BLANK_POWERDOWN;
            ioctl(this->fb0fd, FBIOBLANK, this->blank);
        }

        if (this->fb1fd > 0)
        {
            this->blank = FB_BLANK_POWERDOWN;
            ioctl(this->fb1fd, FBIOBLANK, this->blank);
        }

        if (this->outBuffer)
            munmap(this->outBuffer, this->outSize);

        if (this->fb0fd > 0)
            close(this->fb0fd);

        if (this->fb1fd > 0)
            close(this->fb1fd);

        if (this->task.output.paddr)
            ioctl(this->ipufd, IPU_FREE, &this->task.output.paddr);

        if (this->alphaBuffer)
            munmap(this->alphaBuffer, this->alphaSize);

        if (this->task.overlay.alpha.loc_alp_paddr)
            ioctl(this->ipufd, IPU_FREE, &this->task.overlay.alpha.loc_alp_paddr);

        if (this->ovBuffer)
            munmap(this->ovBuffer, this->ovSize);

        if (this->task.overlay.paddr)
            ioctl(this->ipufd, IPU_FREE, &this->task.overlay.paddr);

        if (this->vdiBuffer)
            munmap(this->vdiBuffer, this->inSize);

        if (this->task.input.paddr_n)
            ioctl(this->ipufd, IPU_FREE, &this->task.input.paddr_n);

        if (this->inBuffer0)
            munmap(this->inBuffer0, this->inSize);

        if (this->task.input.paddr)
            ioctl(this->ipufd, IPU_FREE, &this->task.input.paddr);

        if (this->ipufd > 0)
            close(this->ipufd);

        pthread_mutex_destroy(&this->mtx);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ipu attribute\n", DBGINFO));
    }

    return ret;
}


static int
ipu_init_attribute(struct ipu_interface *ipu, int inwid, int inhgt, unsigned int infmt, int outwid, int outhgt, unsigned int outfmt)
{
    struct ipu_task *it = NULL;
    struct ipu_attribute *this = (struct ipu_attribute *) ipu;

    if (this)
    {
        pthread_mutex_init(&this->mtx, NULL);
        pthread_mutex_lock(&this->mtx);

        it = &this->task;
        it->priority = 0;
        it->task_id  = 0;
        it->timeout  = 1000;
        it->input.width   = inwid;
        it->input.height  = inhgt;
        it->input.format  = infmt;
        it->input.crop.w  = 0;
        it->input.crop.h  = 0;
        it->input.crop.pos.x = 0;
        it->input.crop.pos.y = 0;
        it->input.deinterlace.enable = 0;
        it->input.deinterlace.motion = 0;
        it->output.width  = outwid;
        it->output.height = outhgt;
        it->output.format = outfmt;
        it->output.rotate = 0;

        this->fbsel = 1;
        this->ipufd = open("/dev/mxc_ipu", O_RDWR, 0);

        if (this->ipufd < 0)
        {
            pthread_mutex_unlock(&this->mtx);
            ipu_deinit_attribute(ipu);
            TLOGMSG(1, (DBGINFOFMT "failed open ipu\n", DBGINFO));
            return -1;
        }

        if (it->input.format == IPU_PIX_FMT_TILED_NV12F)
        {
            this->inSize = PAGE_ALIGN(it->input.width * it->input.height / 2) + PAGE_ALIGN(it->input.width * it->input.height / 4);
            this->inSize = this->inSize * 2;
            it->input.paddr = this->inSize;
        }
        else
        {
            this->inSize = it->input.width * it->input.height * ipu_fmt2bpp(it->output.format) / 8;
            it->input.paddr = this->inSize;
        }

        if (ioctl(this->ipufd, IPU_ALLOC, &it->input.paddr) < 0)
        {
            pthread_mutex_unlock(&this->mtx);
            ipu_deinit_attribute(ipu);
            TLOGMSG(1, (DBGINFOFMT "ioctl return fail (IPU_ALLOC)\n", DBGINFO));
            return -1;
        }

        this->inBuffer0 = mmap(0, this->inSize, PROT_READ | PROT_WRITE, MAP_SHARED, this->ipufd, it->input.paddr);

        if (!this->inBuffer0)
        {
            pthread_mutex_unlock(&this->mtx);
            ipu_deinit_attribute(ipu);
            TLOGMSG(1, (DBGINFOFMT "%s\n", DBGINFO, strerror(errno)));
            return -1;
        }

        TLOGMSG(1, ("ipu input buffer = %p\n", this->inBuffer0));
        TLOGMSG(1, ("ipu input buffer size = %d\n", this->inSize));

        /* open frame buffer */
        if (ipu_open_framebuf(ipu) != 0)
        {
            pthread_mutex_unlock(&this->mtx);
            ipu_deinit_attribute(ipu);
            TLOGMSG(1, (DBGINFOFMT "failed to open framebuf\n", DBGINFO));
            return -1;
        }

        if (!this->outBuffer)
        {
            this->outSize = this->fb1VarScrInfo.xres * (this->fb1VarScrInfo.yres + 8) * ipu_fmt2bpp(IPU_PIX_FMT_ABGR32) / 8;
            this->outpaddr3 = this->outSize;

            if (ioctl(this->ipufd, IPU_ALLOC, &this->outpaddr3) == 0)
            {
                this->outBuffer = mmap(NULL, this->outSize, PROT_READ | PROT_WRITE, MAP_SHARED, this->ipufd, this->outpaddr3);

                if ((int)this->outBuffer != -1)
                {
                    TLOGMSG(1, ("ipu output buffer = %p\n", this->outBuffer));
                    TLOGMSG(1, ("ipu output buffer size = %d\n", this->outSize));
                }
                else
                {
                    pthread_mutex_unlock(&this->mtx);
                    ipu_deinit_attribute(ipu);
                    TLOGMSG(1, (DBGINFOFMT "mmap return fail\n", DBGINFO));
                    return -1;
                }
            }
            else
            {
                pthread_mutex_unlock(&this->mtx);
                ipu_deinit_attribute(ipu);
                TLOGMSG(1, (DBGINFOFMT "ioctl return fail (IPU_ALLOC)\n", DBGINFO));
                return -1;
            }
        }

        if (ipu_adjust_crop_size(ipu) != 0)
        {
            pthread_mutex_unlock(&this->mtx);
            ipu_deinit_attribute(ipu);
            TLOGMSG(1, (DBGINFOFMT "ipu_adjust_crop_size return fail\n", DBGINFO));
            return -1;
        }

        if (this->pfb0)
            memset(this->pfb0, 0x01, this->fb0VarScrInfo.xres * this->fb0VarScrInfo.yres * this->fb0VarScrInfo.bits_per_pixel / 8);

        if (this->pfb1)
            memset(this->pfb1, 0x01, this->fb1VarScrInfo.xres * this->fb1VarScrInfo.yres * this->fb1VarScrInfo.bits_per_pixel / 8);

        pthread_mutex_unlock(&this->mtx);
    }
    else
    {
        TLOGMSG(1, (DBGINFOFMT "null ipu interface\n", DBGINFO));
        return -1;
    }

    return 0;
}


static int
ipu_set_colorkey(struct ipu_interface *ipu, int flag, unsigned char r, unsigned char g, unsigned char b)
{
    int ret = 0;
    struct ipu_attribute *this = (struct ipu_attribute *) ipu;
    struct mxcfb_color_key ckey = {.enable = 0, .color_key = 0};

    if (this)
    {
        if (ipu_set_alpha_channel(ipu, 1, 0xFF) == 0)
        {
            this->fb0fd = open(IPU_FB0_PATH, O_RDWR, 0);

            if (this->fb0fd)
            {
                ckey.enable = flag;
                ckey.color_key = ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);

                if (ioctl(this->fb0fd, MXCFB_SET_CLR_KEY, &ckey) != 0)
                {
                    ret = -1;
                    TLOGMSG(1, (DBGINFOFMT "ioctl return fail (MXCFB_SET_CLR_KEY)\n", DBGINFO));
                }

                close(this->fb0fd);
            }
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "failed to open fb0\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to set alpha channel\n", DBGINFO));
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null ipu interface\n", DBGINFO));

    return ret;
}


static int
ipu_blit2fb(struct ipu_interface *ipu, struct blit_parm *parm)
{
    int ret = 0;
    struct ipu_task *it = NULL;
    struct ipu_attribute *this = (struct ipu_attribute *) ipu;

    if (this && parm)
    {
        pthread_mutex_lock(&this->mtx);

        it = &this->task;
        it->input.width       = parm->input.width;
        it->input.height      = parm->input.height;
        it->input.format      = parm->input.format;
        it->input.crop.w      = (parm->input.crop.w >> 3) << 3;
        it->input.crop.h      = (parm->input.crop.h >> 3) << 3;
        it->input.crop.pos.x  = (parm->input.crop.x >> 3) << 3;
        it->input.crop.pos.y  = (parm->input.crop.y >> 3) << 3;
        it->output.width      = parm->output.width;
        it->output.height     = parm->output.height;
        it->output.crop.w     = ((parm->output.crop.w) >> 3) << 3;
        it->output.crop.h     = ((parm->output.crop.h) >> 3) << 3;
        it->output.crop.pos.x = (parm->output.crop.x >> 3) << 3;
        it->output.crop.pos.y = (parm->output.crop.y >> 3) << 3;
        it->output.format     = parm->output.format;
        it->output.rotate     = parm->output.rotate;
        memcpy(this->inBuffer0, parm->input.buf, parm->input.size);

        if (this->fbsel)
        {
            it->output.paddr = this->outpaddr2;
            ioctl(this->fb1fd, MXCFB_WAIT_FOR_VSYNC, 0);
        }
        else
        {
            it->output.paddr = this->outpaddr;
            ioctl(this->fb0fd, MXCFB_WAIT_FOR_VSYNC, 0);
        }

        if (ioctl(this->ipufd, IPU_QUEUE_TASK, it) < 0)
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "ioctl return fail(IPU_QUEUE_TASK)\n", DBGINFO));
            TLOGMSG(1, (DBGINFOFMT "%s\n", DBGINFO, strerror(errno)));
        }

        pthread_mutex_unlock(&this->mtx);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ipu interface or null blit parm\n", DBGINFO));
    }

    return ret;
}



static int
ipu_blit2mem(struct ipu_interface *ipu, struct blit_parm *parm)
{
    int ret = 0;
    struct ipu_task *it = NULL;
    struct ipu_attribute *this = (struct ipu_attribute *) ipu;

    if (this && parm)
    {
        pthread_mutex_lock(&this->mtx);

        it = &this->task;

        it->input.format      = parm->input.format;
        it->input.width       = parm->input.width;
        it->input.height      = parm->input.height;
        it->input.crop.w      = (parm->input.crop.w >> 3) << 3;
        it->input.crop.h      = (parm->input.crop.h >> 3) << 3;
        it->input.crop.pos.x  = (parm->input.crop.x >> 3) << 3;
        it->input.crop.pos.y  = (parm->input.crop.y >> 3) << 3;
        it->output.format     = parm->output.format;
        it->output.rotate     = parm->output.rotate;
        it->output.width      = parm->output.width;
        it->output.height     = parm->output.height;
        it->output.crop.w     = parm->output.crop.w;
        it->output.crop.h     = parm->output.crop.h;
        it->output.crop.pos.x = 0;
        it->output.crop.pos.y = 0;
        it->output.paddr      = this->outpaddr3;
        it->overlay_en        = 0;

        memcpy(this->inBuffer0, parm->input.buf, parm->input.size);

        if (ioctl(this->ipufd, IPU_QUEUE_TASK, it) != -1)
            memcpy(parm->output.buf, this->outBuffer, it->output.width * it->output.height * ipu_fmt2bpp(it->output.format) / 8);
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "ioctl return fail(IPU_QUEUE_TASK)\n", DBGINFO));
        }

        pthread_mutex_unlock(&this->mtx);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ipu interface or null blit parm\n", DBGINFO));
    }

    return ret;
}


struct ipu_interface *
ipu_create(int inwid, int inhgt, unsigned int infmt, int outwid, int outhgt, unsigned int outfmt)
{
    struct ipu_interface *ipu = NULL;
    struct ipu_attribute *this = malloc(sizeof(struct ipu_attribute));

    if (this)
    {
        memset(this, 0x00, sizeof(struct ipu_attribute));
        ipu = &(this->extif);
        ipu->setColorKey = ipu_set_colorkey;
        ipu->blitToMemory = ipu_blit2mem;
        ipu->blitToFrameBuf = ipu_blit2fb;

        if (ipu_init_attribute(ipu, inwid, inhgt, infmt, outwid, outhgt, outfmt) == 0)
            TLOGMSG(1, ("create ipu interface\n"));
        else
        {
            free(this);
            ipu = NULL;
            TLOGMSG(1, (DBGINFOFMT "failed to init attribute\n", DBGINFO));
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "%s\n", DBGINFO, strerror(errno)));

    return ipu;
}


int
ipu_destroy(struct ipu_interface *ipu)
{
    int ret = 0;
    struct ipu_attribute *this = (struct ipu_attribute *) ipu;

    if (this)
    {
        ipu_deinit_attribute(ipu);
        free(this);
        TLOGMSG(1, ("IPU : destroy module interface\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null ipu interface\n", DBGINFO));
    }

    return ret;
}
