/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    vgss.c
        external/internal function implementations of vgss interface
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/


#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <linux/ipu.h>
#include <linux/mxcfb.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <etc/util.h>

#include "core/logger.h"
#include "modules/camif.h"
#include "modules/eis.h"
#include "modules/ipu.h"
#include "modules/vgss.h"


/* VGAENC : video encoder for internal display		*/
/* TVENC  : video encoder for external video out	*/
#define TVENC_PATH			"/sys/devices/platform/imx-i2c.2/i2c-2/2-0076/"
#define VGAENC_PATH			"/dev/ch7026"
#define TVENC_RESET			"/sys/devices/platform/imx-i2c.2/i2c-2/2-0076/ResetChip"
#define TVENC_SETINSIG		"/sys/devices/platform/imx-i2c.2/i2c-2/2-0076/SetInputSignal"

/* macro define : IOCTL */
#define VGAENC_WRITE		_IO('C', 1)


/* macro define : OLED */
#define OLED_XRES			800
#define OLED_YRES			600

/* marcro define : capture source parameters */
#define CAPSRC_WIDTH			640
#define CAPSRC_HEIGHT			480
#define CAPSRC_CROPX			23
#define CAPSRC_CROPY			18
#define CAPSRC_CROPW			552
#define CAPSRC_CROPH			416
#define CAPSRC_EZOOM_CROPX1		167
#define CAPSRC_EZOOM_CROPX2		140
#define CAPSRC_EZOOM_CROPY1		128
#define CAPSRC_EZOOM_CROPY2		106
#define CAPSRC_EZOOM_CROPW		272
#define CAPSRC_EZOOM_CROPH		204
#define CAPSRC_FPS				30
#define CAPSRC_PIXFMT			IPU_PIX_FMT_YUV420P

/* macro defines : eis input size */
#define EIS_INPUT_CROPX			0		//
#define EIS_INPUT_CROPY			0		//
#define EIS_INPUT_CROPW			608
#define EIS_INPUT_CROPH			456
#define EIS_OUTPUT_CROPX		0
#define EIS_OUTPUT_CROPY		0
#define EIS_OUTPUT_CROPW		552
#define EIS_OUTPUT_CROPH		416
#define EIS_PIXFMT				IPU_PIX_FMT_NV12

/* macro defines : preview parameters */
#define PREVIEW_WIDTH			640
#define PREVIEW_HEIGHT			480
#define PREVIEW_TOP				65
#define PREVIEW_LEFT			80
#define PREVIEW_FPS				30
#define PREVIEW_PIXFMT			IPU_PIX_FMT_ABGR32

/* macro defines : eis state */
#define EIS_STOP				0
#define EIS_RUN					1
#define EIS_PAUSE				2


/* enumeration decalartion : tvenc output formats */
enum TVENC_OUTFMT
{
	TVENC_OUTFMT_NONE = -1,
	TVENC_OUTFMT_NTSCM,
	TVENC_OUTFMT_NTSCJ,
	TVENC_OUTFMT_NTSC433,
	TVENC_OUTFMT_PALB,
	TVENC_OUTFMT_PALM,
	TVENC_OUTFMT_PALN,
	TVENC_OUTFMT_PALNC,
	TVENC_OUTFMT_PAL60,
	TVENC_OUTFMT_VGA
};

/* enumeration declaration : tvenc source formats */
enum TVENC_SRCFMT
{
	TVENC_SRCFMT_RGB888,
	TVENC_SRCFMT_RGB666,
	TVENC_SRCFMT_RGB565,
	TVENC_SRCFMT_RGB555,
	TVENC_SRCFMT_DVO,
	TVENC_SRCFMT_YCC422B8,		/* YCbCr422 - 8bit		*/
	TVENC_SRCFMT_YCC422B10,		/* YCbCr422 - 10bit		*/
	TVENC_SRCFMT_YCC444B8,		/* YCbCr444 - 8bit		*/
	TVENC_SRCFMT_CRGB666 = 9,	/* Consecutive RGB666	*/
	TVENC_SRCFMT_CRGB565,		/* Consecutive RGB565	*/
	TVENC_SRCFMT_CRGB555		/* Consecutive RGB555	*/
};

/* structure declaration : vga encoder (ch7026) parameter */
struct vgaenc_parm
{
    unsigned char reg;
    unsigned char wrdata;
    unsigned char rddata;
    unsigned char lendata;
};

/* structure declaration : capture source property */
struct capsrc
{
	int width;
	int height;
	struct rect crop;
};

/* structure declaration : camera preview property */
struct camprv
{
	int width;
	int height;
	struct rect pos;
};

/* structure declaration : vgss attribute */
struct vgss_attribute
{
	/* external interface */
	struct vgss_interface extif;

	/* internal attribute */
	int eis;
    int ezoom;
    int vgaenc;
    int evout;
	int preview;
	int error;
	void *midbuf; //

	pthread_t tid;
	pthread_cond_t cond;
	pthread_mutex_t mtx;

	struct capsrc capsrc;
	struct camprv camprv;
    struct cam_interface *camif;
    struct eis_interface *istab;
    struct ipu_interface *ipu;
};



static int
vgss_reset_tvenc(int flag)
{
	int fd = 0;
	int nwr = 0;
	int ret = 0;
	char buf[8] = {0};

	fd = open(TVENC_RESET, O_RDWR);

	if (fd < 0)
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "open return fail\n", DBGINFO));
	}
	else
	{
		memset(buf, 0x00, sizeof(buf));
		nwr = snprintf(buf, sizeof(buf), "%d", flag == true ? 1 : 0);

		if (nwr > 0)
		{
			if (write(fd, buf, nwr) == nwr)
				TLOGMSG(1, ("tvenc %s\n", flag == true ? "enable" : "disable"));
			else
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "write return fail\n", DBGINFO));
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "snprintf return fail\n", DBGINFO));
		}

		close(fd);
	}

	return ret;
}


static int
vgss_set_tvenc_parm(int inwid, int inhgt, int infmt, int outfmt, int order)
{
	int fd = 0;
	int ret = 0;
	int nwr = 0;
	char buf[64] = {0};

	fd = open(TVENC_SETINSIG, O_RDWR);

	if (fd < 0)
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "open return fail\n", DBGINFO));
	}
	else
	{
		memset(buf, 0x00, sizeof(buf));
		nwr = snprintf(buf, sizeof(buf), "%d %d %d %d %d", inwid, inhgt, infmt, outfmt, order);

		if (nwr > 0)
		{
			if (write(fd, buf, nwr) == nwr)
				TLOGMSG(1, ("set tvenc param (%s)\n", buf));
			else
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "write return fail\n", DBGINFO));
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "snprintf return fail\n", DBGINFO));
		}

		close(fd);
	}

	return ret;
}


static int
vgss_set_capsrc_crop(struct vgss_interface *vgss, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
	int ret = 0;
	struct vgss_attribute *this = (struct vgss_attribute *) vgss;

	if (this)
	{
		this->capsrc.crop.x = x;
		this->capsrc.crop.y = y;
		this->capsrc.crop.w = w;
		this->capsrc.crop.h = h;
		TLOGMSG(1, ("capture source crop region = (%d, %d, %d, %d)\n", x, y, w, h));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null vgss interface\n", DBGINFO));
	}

	return ret;
}


static int
vgss_enable_eis(struct vgss_interface *vgss, int flag)
{
	int ret = 0;
	struct rect pos;
	struct vgss_attribute *this = (struct vgss_attribute *) vgss;

	if (this)
	{
		if (this->eis == EIS_STATE_DISABLE)
		{
			if (flag == EIS_STATE_ENABLE)
			{
				memcpy(&pos, &this->camprv.pos, sizeof(struct rect));

				if (this->ezoom == EZOOM_STATE_DISABLE)
					vgss_set_capsrc_crop(vgss, EIS_OUTPUT_CROPX, EIS_OUTPUT_CROPY, EIS_OUTPUT_CROPW, EIS_OUTPUT_CROPH);
				else
					vgss_set_capsrc_crop(vgss, CAPSRC_EZOOM_CROPX2, CAPSRC_EZOOM_CROPY2, CAPSRC_EZOOM_CROPW, CAPSRC_EZOOM_CROPH);

				this->istab = eis_create();

				if (this->istab)
				{
					this->istab->start(this->istab);
					this->eis = EIS_STATE_ENABLE;
				}
				else
				{
					ret = -1;
					vgss_set_capsrc_crop(vgss, pos.x, pos.y, pos.w, pos.h);
					TLOGMSG(1, (DBGINFOFMT "failed to enable image stabilizer\n", DBGINFO));
				}
			}
			else
			{
				ret = -1;
				TLOGMSG(1, ("image stabilizer already disabled\n"));
			}
		}
		else
		{
			if (flag == EIS_STATE_DISABLE)
			{
				if (this->eis == EIS_STATE_EXIT)
				{
					this->istab->stop(this->istab);
					eis_destroy(this->istab);
					this->istab = NULL;
					this->eis = EIS_STATE_DISABLE;

					if (this->ezoom == EZOOM_STATE_DISABLE)
						vgss_set_capsrc_crop(vgss, CAPSRC_CROPX, CAPSRC_CROPY, CAPSRC_CROPW, CAPSRC_CROPH);
					else
						vgss_set_capsrc_crop(vgss, CAPSRC_EZOOM_CROPX1, CAPSRC_EZOOM_CROPY1, CAPSRC_EZOOM_CROPW, CAPSRC_EZOOM_CROPH);

					TLOGMSG(1, ("disable image stabilizer\n"));
				}
				else
				{
					this->eis = flag;
					TLOGMSG(1, ("set image stabilizer exit flag\n"));
				}
			}
			else if (flag == EIS_STATE_EXIT)
			{
				this->eis = flag;
				TLOGMSG(1, ("set image stabilizer exit flag\n"));
			}
			else
			{
				ret = -1;
				TLOGMSG(1, ("image stabilizer already enabled\n"));
			}
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null vgss interface\n", DBGINFO));
	}

	return ret;
}


static int
vgss_get_eis_state(struct vgss_interface *vgss)
{
	int ret = 0;
	struct vgss_attribute *this = (struct vgss_attribute *) vgss;

	if (this)
		ret = this->eis;
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null vgss interface\n", DBGINFO));
	}

	return ret;
}


static int
vgss_ctrl_eis(struct vgss_interface *vgss, int code)
{
	int ret = 0;
	struct vgss_attribute *this = (struct vgss_attribute *) vgss;

	if (this)
	{
		switch(code)
		{
		case EIS_CTRL_DISABLE:
			ret = vgss_enable_eis(vgss, EIS_STATE_DISABLE);
			break;

		case EIS_CTRL_ENABLE:
			ret = vgss_enable_eis(vgss, EIS_STATE_ENABLE);
			break;

		case EIS_CTRL_EXIT:
			ret = vgss_enable_eis(vgss, EIS_STATE_EXIT);
			break;

		case EIS_CTRL_STATE:
			ret = vgss_get_eis_state(vgss);
			break;

		default:
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "invalid eis control code\n", DBGINFO));
			break;
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null vgss interface\n", DBGINFO));
	}

	return ret;
}


static int
vgss_blit_preview(struct vgss_interface *vgss, void *buf, unsigned int size)
{
	int ret = VGSS_FAIL;
	struct timespec t1, t2;
	struct ipu_interface *ipu = NULL;
	struct eis_interface *istab = NULL;
	struct vgss_attribute *this = (struct vgss_attribute *) vgss;
	struct blit_parm parm_blit2fb;
	struct blit_parm parm_blit2mem;

	if (!this)
	{
		TLOGMSG(1, (DBGINFOFMT "null vgss interface\n"));
		goto fail_exit;
	}

	ipu = this->ipu;

	if (this->eis == EIS_STATE_ENABLE)
	{
		istab = this->istab;

		/* set ipu blit to memory param */
		parm_blit2mem.input.buf    = buf;
		parm_blit2mem.input.size   = size;
		parm_blit2mem.input.width  = this->capsrc.width;
		parm_blit2mem.input.height = this->capsrc.height;
		parm_blit2mem.input.format = CAPSRC_PIXFMT;
		parm_blit2mem.input.crop.x = EIS_INPUT_CROPX;	//
		parm_blit2mem.input.crop.y = EIS_INPUT_CROPY;	//
		parm_blit2mem.input.crop.w = EIS_INPUT_CROPW;
		parm_blit2mem.input.crop.h = EIS_INPUT_CROPH;
		parm_blit2mem.output.width  = EIS_INPUT_CROPW;
		parm_blit2mem.output.height = EIS_INPUT_CROPH;
		parm_blit2mem.output.format = EIS_PIXFMT;
		parm_blit2mem.output.buf    = istab->getSrcbuf(istab);
		parm_blit2mem.output.rotate = IPU_ROTATE_NONE;
		parm_blit2mem.output.crop.x = EIS_INPUT_CROPX;
		parm_blit2mem.output.crop.y = EIS_INPUT_CROPY;
		parm_blit2mem.output.crop.w = EIS_INPUT_CROPW;
		parm_blit2mem.output.crop.h = EIS_INPUT_CROPH;

		/* set ipu blit to frame buffer param */
		parm_blit2fb.input.buf    = istab->getOutbuf(istab);
		parm_blit2fb.input.size   = EIS_OUTPUT_CROPW * EIS_OUTPUT_CROPH * 1.5;
		parm_blit2fb.input.width  = EIS_OUTPUT_CROPW;
		parm_blit2fb.input.height = EIS_OUTPUT_CROPH;
		parm_blit2fb.input.format = EIS_PIXFMT;
		parm_blit2fb.input.crop.x = this->capsrc.crop.x;
		parm_blit2fb.input.crop.y = this->capsrc.crop.y;
		parm_blit2fb.input.crop.w = this->capsrc.crop.w;
		parm_blit2fb.input.crop.h = this->capsrc.crop.h;
		parm_blit2fb.output.width  = this->camprv.width;
		parm_blit2fb.output.height = this->camprv.height;
		parm_blit2fb.output.format = PREVIEW_PIXFMT;
		parm_blit2fb.output.buf    = NULL;
		parm_blit2fb.output.rotate = IPU_ROTATE_NONE;
		parm_blit2fb.output.crop.x = this->camprv.pos.x;
		parm_blit2fb.output.crop.y = this->camprv.pos.y;
		parm_blit2fb.output.crop.w = this->camprv.pos.w;
		parm_blit2fb.output.crop.h = this->camprv.pos.h;


		if (ipu->blitToMemory(ipu, &parm_blit2mem) == 0)
		{
			if (istab->process(istab) == 0)
			{
				if (ipu->blitToFrameBuf(ipu, &parm_blit2fb) == 0)
					ret = VGSS_SUCCESS;
				else
					TLOGMSG(1, (DBGINFOFMT "failed to blit frame buffer\n", DBGINFO));
			}
			else
				TLOGMSG(1, (DBGINFOFMT "failed to process eis\n", DBGINFO));
		}
		else
			TLOGMSG(1, (DBGINFOFMT "failed to blit memory\n", DBGINFO));


		if (this->eis == EIS_STATE_EXIT)
			vgss_ctrl_eis(vgss, EIS_CTRL_DISABLE);
	}
	else
	{
		parm_blit2fb.input.buf    = buf;
		parm_blit2fb.input.size   = size;
		parm_blit2fb.input.width  = this->capsrc.width;
		parm_blit2fb.input.height = this->capsrc.height;
		parm_blit2fb.input.format = CAPSRC_PIXFMT;
		parm_blit2fb.input.crop.x = this->capsrc.crop.x;
		parm_blit2fb.input.crop.y = this->capsrc.crop.y;
		parm_blit2fb.input.crop.w = this->capsrc.crop.w;
		parm_blit2fb.input.crop.h = this->capsrc.crop.h;
		parm_blit2fb.output.width  = this->camprv.width;
		parm_blit2fb.output.height = this->camprv.height;
		parm_blit2fb.output.format = PREVIEW_PIXFMT;
		parm_blit2fb.output.buf    = NULL;
		parm_blit2fb.output.rotate = IPU_ROTATE_NONE;
		parm_blit2fb.output.crop.x = this->camprv.pos.x;
		parm_blit2fb.output.crop.y = this->camprv.pos.y;
		parm_blit2fb.output.crop.w = this->camprv.pos.w;
		parm_blit2fb.output.crop.h = this->camprv.pos.h;

		if (ipu->blitToFrameBuf(ipu, &parm_blit2fb) == 0)
			ret = VGSS_SUCCESS;
		else
			TLOGMSG(1, (DBGINFOFMT "failed to blit to frame buffer\n", DBGINFO));
	}

fail_exit:
	return ret;
}


static void *
vgss_process_preview(void *arg)
{
	fd_set reads;
	struct v4l2_format fmt;
	struct v4l2_buffer buf;
	unsigned int sizeimage = 0;

	struct timespec timeout = {.tv_sec = 1, .tv_nsec = 0};
	struct vgss_interface *vgss = (struct vgss_interface *) arg;
	struct vgss_attribute *this = (struct vgss_attribute *) vgss;
	struct cam_interface *camif = this->camif;

	memset(&buf, 0x00, sizeof(struct v4l2_buffer));
	memset(&fmt, 0x00, sizeof(struct v4l2_format));

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ioctl(camif->getFileDesc(camif), VIDIOC_G_FMT, &fmt);

	sizeimage = fmt.fmt.pix.sizeimage;
	this->preview = PREVIEW_STATE_PAUSE;

	if (camif->onStream(camif) != 0)
	{
		this->error = VGSS_ERROR_CAMIF_STREAM;
		TLOGMSG(1, (DBGINFOFMT "failed to on camera data stream\n", DBGINFO));
		goto fail_exit;
	}

	while (this->preview != PREVIEW_STATE_STOP)
	{
		if (this->preview == PREVIEW_STATE_RUN)
		{
			FD_ZERO(&reads);
			FD_SET(camif->getFileDesc(camif), &reads);

			if (pselect(camif->getFileDesc(camif) + 1, &reads, NULL, NULL, &timeout, NULL) != -1)
			{
				buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				buf.memory = V4L2_MEMORY_MMAP;

				if (camif->deqBuf(camif, &buf) == -1)
				{
					this->error = VGSS_ERROR_CAMIF_DQBUF;
					this->preview = PREVIEW_STATE_STOP;
					TLOGMSG(1, (DBGINFOFMT "failed to dequeue buffer from cam driver's outgoing queue\n", DBGINFO));
				}

				if (vgss_blit_preview(vgss, camif->getBuf(camif, buf.index), sizeimage) != 0)
				{
					this->error = VGSS_ERROR_IPU;
					this->preview = PREVIEW_STATE_STOP;
					TLOGMSG(1, (DBGINFOFMT "failed to blit preview\n", DBGINFO));
				}

				if (camif->queBuf(camif, &buf) == -1)
				{
					this->error = VGSS_ERROR_CAMIF_QBUF;
					this->preview = PREVIEW_STATE_STOP;
					TLOGMSG(1, (DBGINFOFMT "failed to enqueue buffer to cam driver's incomming queue\n", DBGINFO));
				}
			}
			else
			{
				this->error = VGSS_ERROR_CAMIF_STREAM;
				this->preview = PREVIEW_STATE_STOP;
				TLOGMSG(1, (DBGINFOFMT "timeout occurred while wait capture source data\n"));
			}
		}
		//else
		//{
		//	pthread_mutex_lock(&vgss->attr->mtx);
		//	pthread_cond_wait(&vgss->attr->cond, &vgss->attr->mtx);
		//	pthread_mutex_lock(&vgss->attr->mtx);
		//}
	}

	if (camif->offStream(camif) != 0)
	{
		this->error = VGSS_ERROR_CAMIF_STREAM;
		TLOGMSG(1, (DBGINFOFMT "failed to off camera data stream\n", DBGINFO));
	}

	TLOGMSG(1, ("exit ircam preview thread\n"));

fail_exit:
	return NULL;
}


static int
vgss_run_preview(struct vgss_interface *vgss)
{
	int ret = 0;
	struct vgss_attribute *this = (struct vgss_attribute *) vgss;

	if (this)
	{
		if (this->preview != PREVIEW_STATE_RUN)
		{
			pthread_mutex_lock(&this->mtx);
			this->preview = PREVIEW_STATE_RUN;
			pthread_mutex_unlock(&this->mtx);
			pthread_cond_signal(&this->cond);
			TLOGMSG(1, ("run camera preview\n"));
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "camera preview is already in running state\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null vgss interface\n", DBGINFO));
	}

	return ret;
}


static int
vgss_pause_preview(struct vgss_interface *vgss)
{
	int ret = 0;
	struct vgss_attribute *this = (struct vgss_attribute *) vgss;

	if (this)
	{
		if (this->preview == PREVIEW_STATE_RUN)
		{
			pthread_mutex_lock(&this->mtx);
			this->preview = PREVIEW_STATE_PAUSE;
			pthread_mutex_unlock(&this->mtx);
			pthread_cond_signal(&this->cond);
			TLOGMSG(1, ("pause camera preview\n"));
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "camera preview is already in pause state\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null vgss interface\n", DBGINFO));
	}

	return ret;
}


static int
vgss_stop_preview(struct vgss_interface *vgss)
{
	int ret = 0;
	struct vgss_attribute *this = (struct vgss_attribute *) vgss;

	if (this)
	{
		pthread_mutex_lock(&this->mtx);
		this->preview = PREVIEW_STATE_STOP;
		pthread_mutex_unlock(&this->mtx);
		pthread_cond_signal(&this->cond);
		TLOGMSG(1, ("stop camera preview\n"));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT " null vgss interface\n", DBGINFO));
	}

	return ret;
}


static int
vgss_get_preview_state(struct vgss_interface *vgss)
{
	int ret = 0;
	struct vgss_attribute *this = (struct vgss_attribute *) vgss;

	if (this)
		ret = this->preview;
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null vgss interface\n", DBGINFO));
	}

	return ret;
}


static int
vgss_init_preview(struct vgss_interface *vgss)
{
	struct vgss_attribute *this = (struct vgss_attribute *) vgss;

	if (!this)
	{
		TLOGMSG(1, (DBGINFOFMT " null vgss interface\n", DBGINFO));
		goto fail_exit;
	}

	this->ipu = ipu_create(this->capsrc.width, this->capsrc.height, CAPSRC_PIXFMT, OLED_XRES, OLED_YRES, PREVIEW_PIXFMT);

	if (!this->ipu)
	{
		TLOGMSG(1, (DBGINFOFMT " failed to create ipu interface\n", DBGINFO));
		goto fail_exit;
	}

	this->ipu->setColorKey(this->ipu, 1, 8, 8, 8);
	this->camif = camif_create(this->capsrc.width, this->capsrc.height, CAPSRC_FPS);

	if (!this->camif)
	{
		TLOGMSG(1, (DBGINFOFMT " failed to create cam interface\n", DBGINFO));
		goto fail_exit;
	}

	if (pthread_create(&this->tid, NULL, vgss_process_preview, (void *)vgss) != 0)
	{
		TLOGMSG(1, (DBGINFOFMT " failed to create preview thread\n", DBGINFO));
		goto fail_exit;
	}

	this->preview = PREVIEW_STATE_PAUSE;
	TLOGMSG(1, ("init ircam preview\n"));
	return VGSS_SUCCESS;

fail_exit:
	if (this)
	{
		camif_destroy(this->camif);
		ipu_destroy(this->ipu);
	}

	return VGSS_FAIL;
}


static int
vgss_deinit_preview(struct vgss_interface *vgss)
{
	int ret = 0;
	struct vgss_attribute *this = (struct vgss_attribute *) vgss;

	if (this)
	{
		if (this->preview != PREVIEW_STATE_STOP)
		{
			vgss_stop_preview(vgss);
			pthread_join(this->tid, NULL);
			camif_destroy(this->camif);
			ipu_destroy(this->ipu);
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT " null vgss interface\n", DBGINFO));
	}

	return ret;
}


static int
vgss_enable_vgaenc(struct vgss_interface *vgss, int flag)
{
	int ret = 0;
	int fd = 0;
	struct vgss_attribute *this = (struct vgss_attribute *) vgss;
	struct vgaenc_parm parm = {.reg = 0x04, .wrdata = 0x00, .rddata = 0x00, .lendata = 0x01};

	if (this)
	{
		if (flag != this->vgaenc)
		{
			fd = open(VGAENC_PATH, O_RDWR);

			if (fd < 0)
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "failed to open vgaenc\n", DBGINFO));
			}
			else
			{
				if (flag)
					parm.wrdata = 0x00;
				else
					parm.wrdata = 0x01;

				if (ioctl(fd, VGAENC_WRITE, &parm) == 0)
				{
					this->vgaenc = flag;
					TLOGMSG(1, ("%s vgaenc\n", this->vgaenc ? "enable" : "disable"));
				}
				else
				{
					ret = -1;
					TLOGMSG(1, (DBGINFOFMT "ioctl return fail (VGAENC_WRITE)\n", DBGINFO));
				}

				close(fd);
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, ("vga encoder is already %s\n", flag ? "enabled" : "disabled"));
		}
	}
	else
		TLOGMSG(1, (DBGINFOFMT "null vgss interface\n", DBGINFO));

	return ret;
}


static int
vgss_enable_evout(struct vgss_interface *vgss, int flag)
{
    int ret = 0;
	struct vgss_attribute *this = (struct vgss_attribute *) vgss;

    if (this)
    {
		if (this->evout != flag)
		{
			if (flag)
			{
				vgss_reset_tvenc(EVOUT_STATE_DISABLE);
				vgss_set_tvenc_parm(800, 600, 0, TVENC_OUTFMT_VGA, 0);
				vgss_reset_tvenc(EVOUT_STATE_ENABLE);
				this->evout = EVOUT_STATE_ENABLE;
				TLOGMSG(1, ("enable external video out\n"));
			}
			else
			{
				vgss_reset_tvenc(EVOUT_STATE_DISABLE);
				this->evout = EVOUT_STATE_DISABLE;
				TLOGMSG(1, ("disable external video out\n"));
			}
		}
		else
			TLOGMSG(1, ("external video out is already %s \n", flag ? "enabled" : "disabled"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null vgss interface\n", DBGINFO));
    }

    return ret;
}


static int
vgss_enable_ezoom(struct vgss_interface *vgss, int flag)
{
	int ret = 0;
	struct vgss_attribute *this = (struct vgss_attribute *) vgss;

	if (this)
	{
		if (flag)
		{
			if (this->eis == EIS_STATE_ENABLE)
			{
				this->capsrc.crop.x = CAPSRC_EZOOM_CROPX2;
				this->capsrc.crop.y = CAPSRC_EZOOM_CROPY2;
				this->capsrc.crop.w = CAPSRC_EZOOM_CROPW;
				this->capsrc.crop.h = CAPSRC_EZOOM_CROPH;
			}
			else
			{
				this->capsrc.crop.x = CAPSRC_EZOOM_CROPX1;
				this->capsrc.crop.y = CAPSRC_EZOOM_CROPY1;
				this->capsrc.crop.w = CAPSRC_EZOOM_CROPW;
				this->capsrc.crop.h = CAPSRC_EZOOM_CROPH;
			}

			this->ezoom = EZOOM_STATE_ENABLE;
		}
		else
		{
			if (this->eis == EIS_STATE_ENABLE)
			{
				this->capsrc.crop.x = 0;
				this->capsrc.crop.y = 0;
				this->capsrc.crop.w = CAPSRC_CROPW;
				this->capsrc.crop.h = CAPSRC_CROPH;
			}
			else
			{
				this->capsrc.crop.x = CAPSRC_CROPX;
				this->capsrc.crop.y = CAPSRC_CROPY;
				this->capsrc.crop.w = CAPSRC_CROPW;
				this->capsrc.crop.h = CAPSRC_CROPH;
			}

			this->ezoom = EZOOM_STATE_DISABLE;
		}

		TLOGMSG(1, ("%s electronic zoom\n", flag ? "enable" : "disable"));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null vgss interface\n", DBGINFO));
	}

	return ret;
}


static int
vgss_get_ezoom_state(struct vgss_interface *vgss)
{
	int ret = 0;
	struct vgss_attribute *this = (struct vgss_attribute *) vgss;

	if (this)
		ret = this->ezoom;
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null vgss interface\n", DBGINFO));
	}

	return ret;
}


static int
vgss_ctrl_preview(struct vgss_interface *vgss, int code)
{
	int ret = 0;
	struct vgss_attribute *this = (struct vgss_attribute *) vgss;

	if (this)
	{
		switch(code)
		{
		case PREVIEW_CTRL_INIT:
			ret = vgss_init_preview(vgss);
			break;

		case PREVIEW_CTRL_DEINIT:
			ret = vgss_deinit_preview(vgss);
			break;

		case PREVIEW_CTRL_RUN:
			ret = vgss_run_preview(vgss);
			break;

		case PREVIEW_CTRL_PAUSE:
			ret = vgss_pause_preview(vgss);
			break;

		case PREVIEW_CTRL_STATE:
			ret = vgss_get_preview_state(vgss);
			break;

		default:
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "invalid preview control code\n", DBGINFO));
			break;
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null vgss interface\n", DBGINFO));
	}

	return ret;
}


static int
vgss_ctrl_ezoom(struct vgss_interface *vgss, int code)
{
	int ret = 0;
	struct vgss_attribute *this = (struct vgss_attribute *) vgss;

	if (this)
	{
		switch(code)
		{
		case EZOOM_CTRL_DISABLE:
			ret = vgss_enable_ezoom(vgss, false);
			break;

		case EZOOM_CTRL_ENABLE:
			ret = vgss_enable_ezoom(vgss, true);
			break;

		case EZOOM_CTRL_STATE:
			ret = vgss_get_ezoom_state(vgss);
			break;

		default:
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "invalid ezoom control code\n", DBGINFO));
			break;
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null vgss interface\n", DBGINFO));
	}

	return ret;
}


static int
vgss_ctrl_evout(struct vgss_interface *vgss, int code)
{
	int ret = 0;
	struct vgss_attribute *this = (struct vgss_attribute *) vgss;

	if (this)
	{
		switch (code)
		{
		case EVOUT_CTRL_ENABLE:
			ret = vgss_enable_evout(vgss, EVOUT_STATE_ENABLE);
			break;

		case EVOUT_CTRL_DISABLE:
			ret = vgss_enable_evout(vgss, EVOUT_STATE_DISABLE);
			break;

		case EVOUT_CTRL_STATE:
			ret = this->evout;
			break;

		default:
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "invalid evout control code\n", DBGINFO));
			break;
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null vgss interface \n", DBGINFO));
	}

	return ret;
}


static int
vgss_test_camif(struct vgss_interface *vgss)
{
	int ret = 0;
	struct vgss_attribute *this = (struct vgss_attribute *) vgss;

	if (this)
	{
		if (this->preview == PREVIEW_STATE_STOP)
		{
			vgss_init_preview(vgss);
			vgss_run_preview(vgss);
			MSLEEP(1500);
			vgss_stop_preview(vgss);
			vgss_deinit_preview(vgss);
			ret = this->error;
		}
		else
		{
			MSLEEP(1500);
			ret = this->error;
		}

		TLOGMSG(1, ("camera interface test result = 0x%02X\n", ret));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null vgss interface\n", DBGINFO));
	}

	return ret;
}


struct vgss_interface *
vgss_create(void)
{
    struct vgss_interface *vgss = NULL;
	struct vgss_attribute *this = malloc(sizeof(struct vgss_attribute));

    if (this)
    {
        memset(this, 0x00, sizeof(struct vgss_attribute));
		pthread_mutex_init(&this->mtx, NULL);
		pthread_cond_init(&this->cond, NULL);

		this->vgaenc  = true;
		this->evout   = EVOUT_STATE_DISABLE;
		this->ezoom   = EZOOM_STATE_DISABLE;
		this->error   = VGSS_ERROR_NONE;
		this->preview = PREVIEW_STATE_STOP;
		this->eis     = EIS_STATE_DISABLE;
		this->capsrc.width  = CAPSRC_WIDTH;
		this->capsrc.height = CAPSRC_HEIGHT;
		this->capsrc.crop.x = CAPSRC_CROPX;
		this->capsrc.crop.y = CAPSRC_CROPY;
		this->capsrc.crop.w = CAPSRC_CROPW;
		this->capsrc.crop.h = CAPSRC_CROPH;
		this->camprv.width  = OLED_XRES;
		this->camprv.height = OLED_YRES;
		this->camprv.pos.x  = PREVIEW_LEFT;
		this->camprv.pos.y  = PREVIEW_TOP;
		this->camprv.pos.w  = PREVIEW_WIDTH;
		this->camprv.pos.h  = PREVIEW_HEIGHT;

		vgss = &(this->extif);
		vgss->ctrlPreview  = vgss_ctrl_preview;
		vgss->ctrlEIS	   = vgss_ctrl_eis;
		vgss->ctrlZoom    = vgss_ctrl_ezoom;
		vgss->ctrlExtVideo = vgss_ctrl_evout;
		vgss->testCamIF    = vgss_test_camif;
		TLOGMSG(1, ("create vgss interface\n"));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "failed to create vgss interface, malloc return null\n", DBGINFO));

    return vgss;
}


int
vgss_destroy(struct vgss_interface *vgss)
{
	int ret = 0;
	struct vgss_attribute *this = (struct vgss_attribute *) vgss;

	if (this)
	{
		vgss_deinit_preview(vgss);
		vgss_enable_vgaenc(vgss, true);
		vgss_enable_evout(vgss, EVOUT_STATE_DISABLE);
		pthread_mutex_destroy(&this->mtx);
		pthread_cond_destroy(&this->cond);
		free(this);
		TLOGMSG(1, ("destroy vgss interface\n"));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null vgss interface\n", DBGINFO));
	}

	return ret;
}
