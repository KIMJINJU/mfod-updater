/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    display.c
        external/internal function implementations of display interface
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "core/logger.h"
#include "etc/util.h"
#include "modules/display.h"
#include "modules/gpio.h"

/* constant macro defines  : i2c device path */
#define DEVPATH_DISPLAY			"/sys/devices/platform/imx-i2c.2/i2c-2/2-0032/write"
#define DEVPATH_POTENTIOMETER	"/sys/devices/platform/imx-i2c.2/i2c-2/2-002f/Brightness"
#define DEVPATH_VGAENC			"/dev/ch7026"

/* constant macro defines : brightness values */
#define BRIGHT_MAX				0
#define BRIGHT_MIN				255

/* constant macro defines : display control gpio index */
#define NUM_DISPCTL_GPIO		5
#define DISPCTL_OLED_VAA		0
#define DISPCTL_OLED_VAN		1
#define DISPCTL_OLED_VDD		2
#define DISPCTL_OLED_RST		3
#define DISPCTL_VBUF_PWR		4


#define VGAENC_I2C_WRITE		_IO('C', 1)
#define VGAENC_I2C_READ			_IO('C', 2)


/* structure declarations : display attributes */
struct display_attribute
{
/* external interface */
    struct display_interface extif;

/* internal attributes of display interface */
	int  error;
	int  bright[2];
	bool power;
	bool vgaenc;
	struct gpio_interface *gpio[NUM_DISPCTL_GPIO];
};


static int
disp_deinit_gpio(struct display_interface *disp)
{
	int ret = 0;
    struct display_attribute *this = (struct display_attribute *) disp;

	if (this)
	{
		for (int i = 0; i < NUM_DISPCTL_GPIO; i++)
			gpio_destroy(this->gpio[i]);
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null display interface\n", DBGINFO));
	}

	return ret;
}


static int
disp_init_gpio(struct display_interface *disp)
{
	struct gpio_init_info
	{
		int pin;
		int dir;
		int val;
	};

	struct gpio_init_info info[] =
	{
		{ .pin = GPIO2_OLED_VAA, .dir = GPIO_DIR_OUTPUT, .val= GPIO_VALUE_HIGH },
		{ .pin = GPIO2_OLED_VAN, .dir = GPIO_DIR_OUTPUT, .val= GPIO_VALUE_LOW  },
		{ .pin = GPIO2_OLED_VDD, .dir = GPIO_DIR_OUTPUT, .val= GPIO_VALUE_HIGH },
		{ .pin = GPIO2_OLED_RST, .dir = GPIO_DIR_OUTPUT, .val= GPIO_VALUE_HIGH },
		{ .pin = GPIO4_VBUF_PWR, .dir = GPIO_DIR_OUTPUT, .val= GPIO_VALUE_HIGH },
	};

	int ret = 0;
	struct display_attribute *this = (struct display_attribute *) disp;

	if (this)
    {
		for (int i = 0; i < NUM_DISPCTL_GPIO; i++)
		{
			this->gpio[i] = gpio_create();

			if (this->gpio[i])
			{
				if (this->gpio[i]->export(this->gpio[i], info[i].pin) == 0)
				{
					TLOGMSG(0, (" "));
					//attr->gpio[i]->setdir(attr->gpio[i], info[i].dir);
					//attr->gpio[i]->setval(attr->gpio[i], info[i].val);
				}
				else
				{
					ret = -1;
					disp_deinit_gpio(disp);
					TLOGMSG(1, (DBGINFOFMT "gpio->export return fail (DISPCTL = %d)\n", DBGINFO, i));
					break;
				}
			}
			else
			{
				ret = -1;
				disp_deinit_gpio(disp);
				TLOGMSG(1, (DBGINFOFMT "gpio_create return fail\n", DBGINFO));
				break;
			}
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null display interface\n", DBGINFO));
	}

	return ret;
}


static int
disp_write_register(struct display_interface *disp)
{
	int fd = 0;
	int nwr = 0;
	int ret = 0;
	char buf[32] = {0};
    struct display_attribute *this = (struct display_attribute *) disp;

	char disp_regs[27] =
	{
		0x00,		// STATUS	[00]
		0x78,		// RGAIN	[01]
		0x30,		// ROFF		[02]
		0x78,		// GGAIN	[03]

		0x30,		// GOFF		[04]
		0x78,		// BGAIN	[05]
		0x30,		// BOFF		[06]
		0x80,		// MGAIN	[07]

		0x38,		// MOFF		[08]
		0x60,		// VMODE	[09]
		0x02,		// HMODE	[0A]
		0x00,		// BR_L		[0B]

		0x02,		// BR_U		[0C]
		0x00,		// HRATE_L	[0D]
		0x80,		// HRATE_U	[0E]
		0x1E,		// PLL_L	[0F]

		0x0C,		// PLL_U	[10]
		0x0C,		// PIF		[11]
		0x00,		// PI2		[12]
		0xA8,		// HSTART	[13]

		0x04,		// VSTART	[14]
		0xD8,		// HBLK		[15]
		0x0C,		// HDEL		[16]
		0x72,		// PWDN		[17]

		0x80,		// ATB		[18]
		0x00,		// AMTEST	[19]
		0x08		// TRIM		[1A]
	};


	if (this)
	{
		fd = open(DEVPATH_DISPLAY, O_RDWR);

		if (fd != -1)
		{
			for (int i = 0; i < (int) sizeof(disp_regs); i++)
			{
				memset(buf, 0x00, sizeof(buf));
				nwr = snprintf(buf, sizeof(buf), "0x%02x 0x%02x", i, disp_regs[i]);

				if (nwr > 0)
				{
					if (write(fd, buf, nwr) != nwr)
					{
						ret = -1;
						this->error |= DISP_ERROR_SETREG;
						TLOGMSG(1, (DBGINFOFMT "%s \n", DBGINFO, strerror(errno)));
						break;
					}
				}
				else
					TLOGMSG(1, (DBGINFOFMT "snprintf return fail, %s\n", DBGINFO, strerror(errno)));
			}

			close(fd);
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "%s \n", DBGINFO, strerror(errno)));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null display interface\n", DBGINFO));
	}

	return ret;
}


static int
disp_enable_vgaenc(struct display_interface *disp, bool flag)
{
	int fd = -1;
	int ret = 0;
    struct display_attribute *this = (struct display_attribute *) disp;

	struct ch7026_i2cinfo
	{
		unsigned char reg;
		unsigned char wr;
		unsigned char rd;
		unsigned int  len;
	}
	i2cinfo =
	{
		.reg = 0x04,
		.wr  = 0x00,
		.rd  = 0x00,
		.len = 0x01
	};

	if (this)
	{
		if (this->vgaenc != flag)
		{
			fd = open(DEVPATH_VGAENC, O_RDWR);

			if (fd != -1)
			{
				if (flag)
					i2cinfo.wr = 0x00;
				else
					i2cinfo.wr = 0x01;

				if (ioctl(fd, VGAENC_I2C_WRITE, &i2cinfo) < 0)
				{
					ret = -1;
					TLOGMSG(1, (DBGINFOFMT "ioctl return fail, %s\n", DBGINFO, strerror(errno)));
				}
				else
				{
					this->vgaenc = flag;
					TLOGMSG(1, ("%s vgaenc\n", flag ? "enable" : "disable"));
				}

				close(fd);
			}
			else
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "open return fail\n", DBGINFO));
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "vgaenc is already %s\n", DBGINFO, flag ? "enabled" : "disabled"));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null display interface\n", DBGINFO));
	}

	return ret;
}


static int
disp_set_bright(struct display_interface *disp, int mode, int value)
{
	int fd = 0;
	int ret = 0;
	int nwr = 0;
	int cval = 0;
	int delay = 0;
	char *delim = NULL;
	char buf[32] = {0};
    struct display_attribute *this = (struct display_attribute *) disp;

	if (this)
	{
		if ((mode == DISP_BRIGHT_DV) || (mode == DISP_BRIGHT_IR))
		{
			if (mode == DISP_BRIGHT_DV)
			{
				if (value < DISP_BRIGHT_DV_MAX)
					value = DISP_BRIGHT_DV_MAX;
				else if (value > DISP_BRIGHT_DV_MIN)
					value = DISP_BRIGHT_DV_MIN;
			}
			else
			{
				if (value < DISP_BRIGHT_IR_MAX)
					value = DISP_BRIGHT_IR_MAX;
				else if (value > DISP_BRIGHT_IR_MIN)
					value = DISP_BRIGHT_DV_MIN;
			}

			fd = open(DEVPATH_POTENTIOMETER, O_RDWR);

			if (fd != -1)
			{

				if (read(fd, buf, sizeof(buf)) < 0)
				{
					ret = -1;
					TLOGMSG(1, (DBGINFOFMT "%s\n", DBGINFO, strerror(errno)));
				}
				else
				{
					delim = strstr(buf, "=");

					if (delim)
					{
						cval = atoi(delim + 2);

						if (cval > value)
						{
							memset(buf, 0x00, sizeof(buf));

							if ((cval - value) == 1)
								delay = 1;
							else
								delay = 10;

							for (int i = cval; i > value; i--)
							{
								nwr = snprintf(buf, sizeof(buf), "%d", i - 1);

								if (nwr > 0)
								{
									write(fd, buf, nwr);
									MSLEEP(delay);
								}
								else
									TLOGMSG(1, (DBGINFOFMT "snprintf return fail, %s\n", DBGINFO, strerror(errno)));
							}
						}
						else if (cval < value)
						{
							memset(buf, 0x00, sizeof(buf));

							if ((value - cval) == 1)
								delay = 1;
							else
								delay = 10;

							for (int i = cval; i < value; i++)
							{
								nwr = snprintf(buf, sizeof(buf), "%d", i + 1);

								if (nwr > 0)
								{
									write(fd, buf, nwr);
									MSLEEP(delay);
								}
								else
									TLOGMSG(1, (DBGINFOFMT "snprintf return fail, %s\n", DBGINFO, strerror(errno)));
							}
						}
						else
						{
							memset(buf, 0x00, sizeof(buf));
							nwr = snprintf(buf, sizeof(buf), "%d", value);

							if (nwr > 0)
								write(fd, buf, nwr);
							else
								TLOGMSG(1, (DBGINFOFMT "snprintf return fail, %s\n", DBGINFO, strerror(errno)));
						}

						this->bright[mode] = value;
						TLOGMSG(1, ("display brightness = %d\n", this->bright[mode]));
					}
					else
					{
						ret = -1;
						TLOGMSG(1, (DBGINFOFMT "strstr return fail\n", DBGINFO));
					}
				}

				close(fd);
			}
			else
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "failed to open brightness adjust device\n", DBGINFO));
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "invalid display brightness mode\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null display interface\n", DBGINFO));
	}

	return ret;
}


static int
disp_get_bright(struct display_interface *disp, int mode, int *value)
{
	int ret = 0;
    struct display_attribute *this = (struct display_attribute *) disp;

	if (this)
	{
		if ((mode == DISP_BRIGHT_DV) || (mode == DISP_BRIGHT_IR))
			*value = this->bright[mode];
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "invalid display brightness mode\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null display interface\n", DBGINFO));
	}

	return ret;
}



static int
disp_enable_power(struct display_interface *disp)
{
	int ret = 0;
	struct gpio_interface *vbuf_pwr = NULL;
	struct gpio_interface *oled_vdd = NULL;
	struct gpio_interface *oled_van = NULL;
	struct gpio_interface *oled_vaa = NULL;
	struct gpio_interface *oled_rst = NULL;
    struct display_attribute *this = (struct display_attribute *) disp;

	if (this)
	{
		vbuf_pwr = this->gpio[DISPCTL_VBUF_PWR];
		oled_vdd = this->gpio[DISPCTL_OLED_VDD];
		oled_van = this->gpio[DISPCTL_OLED_VAN];
		oled_vaa = this->gpio[DISPCTL_OLED_VAA];
		oled_rst = this->gpio[DISPCTL_OLED_RST];

		if (this->power == false)
		{
			disp_enable_vgaenc(disp, true);
			vbuf_pwr->setVal(vbuf_pwr, GPIO_VALUE_HIGH);
			oled_vdd->setVal(oled_vdd, GPIO_VALUE_HIGH);
			oled_vaa->setVal(oled_vaa, GPIO_VALUE_LOW);
			oled_van->setVal(oled_van, GPIO_VALUE_HIGH);
			oled_rst->setVal(oled_rst, GPIO_VALUE_HIGH);
			MSLEEP(500);

			oled_vaa->setVal(oled_vaa, GPIO_VALUE_HIGH);
			oled_van->setVal(oled_van, GPIO_VALUE_LOW);
			MSLEEP(10);

			if (disp_write_register(disp) == 0)
			{
				//MSLEEP(100);
				//this->setbr(this, DISP_BRIGHT_DV, DISP_BRIGHT_DV_DEFAULT);
				this->power = true;
				TLOGMSG(1, ("enable display power\n"));
			}
			else
			{
				ret = -1;
				this->error = DISP_ERROR_SETREG;
				oled_vaa->setVal(oled_vaa, GPIO_VALUE_LOW);
				oled_van->setVal(oled_van, GPIO_VALUE_HIGH);
				oled_vdd->setVal(oled_vdd, GPIO_VALUE_LOW);
				oled_rst->setVal(oled_rst, GPIO_VALUE_LOW);
				vbuf_pwr->setVal(vbuf_pwr, GPIO_VALUE_LOW);
				TLOGMSG(1, ("failed to set display register\n"));
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "display is already in power on state\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null display interface\n", DBGINFO));
	}

	return ret;
}


static int
disp_disable_power(struct display_interface *disp)
{
	int ret = 0;
	struct gpio_interface *vbuf_pwr = NULL;
	struct gpio_interface *oled_vdd = NULL;
	struct gpio_interface *oled_van = NULL;
	struct gpio_interface *oled_vaa = NULL;
	struct gpio_interface *oled_rst = NULL;
    struct display_attribute *this = (struct display_attribute *) disp;

	if (disp)
	{
		vbuf_pwr = this->gpio[DISPCTL_VBUF_PWR];
		oled_vdd = this->gpio[DISPCTL_OLED_VDD];
		oled_van = this->gpio[DISPCTL_OLED_VAN];
		oled_vaa = this->gpio[DISPCTL_OLED_VAA];
		oled_rst = this->gpio[DISPCTL_OLED_RST];

		if (this->power == true)
		{
			oled_vaa->setVal(oled_vaa, GPIO_VALUE_LOW);
			oled_van->setVal(oled_van, GPIO_VALUE_HIGH);
			oled_vdd->setVal(oled_vdd, GPIO_VALUE_LOW);
			oled_rst->setVal(oled_rst, GPIO_VALUE_LOW);
			vbuf_pwr->setVal(vbuf_pwr, GPIO_VALUE_LOW);
			disp_enable_vgaenc(disp, false);
			this->power = false;
			TLOGMSG(1, ("disable display power\n"));
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "display is already in power off state\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null display interface\n", DBGINFO));
	}

	return ret;
}


static int
disp_get_pwrst(struct display_interface *disp)
{
	int ret = -1;
    struct display_attribute *this = (struct display_attribute *) disp;

	if (this)
	{
		if (this->power)
			ret = 1;
		else
			ret = 0;
	}
	else;
		TLOGMSG(1, (DBGINFOFMT "null display interface\n", DBGINFO));

	return ret;
}


static int
disp_get_error(struct display_interface *disp)
{
	int ret = 0;
    struct display_attribute *this = (struct display_attribute *) disp;

	if (this)
		ret = this->error;
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null display interface\n", DBGINFO));
	}

	return ret;
}


static int
disp_test_module(struct display_interface *disp)
{
	int ret = 0;
    struct display_attribute *this = (struct display_attribute *) disp;

	if (this)
	{
		if (disp_write_register(disp) == 0)
			TLOGMSG(1, ("display module test passed\n"));
		else
		{
			ret = -1;
			this->error = DISP_ERROR_SETREG;
			TLOGMSG(1, ("display module test failed (DISP_ERROR_REG)\n"));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null display interface\n", DBGINFO));
	}

	return ret;
}


/* EXTERNAL FUNCTIONS FOR CREATE/DESTROY DISPLAY INTERFACE */
struct display_interface *
disp_create(void)
{
	struct display_interface *disp = NULL;
    struct display_attribute *this = malloc(sizeof(struct display_attribute));

	if (this)
	{
        disp = &(this->extif);

        if (disp_init_gpio(disp) == 0)
		{
            /* init internal attributes */
            this->power  = true;
            this->vgaenc = true;
            this->bright[DISP_BRIGHT_DV] = DISP_BRIGHT_DV_DEFAULT;
            this->bright[DISP_BRIGHT_IR] = DISP_BRIGHT_IR_DEFAULT;

            /* init external interface */
            disp->setBright      = disp_set_bright;
            disp->getBright      = disp_get_bright;
            disp->getError       = disp_get_error;
            disp->powerOn        = disp_enable_power;
            disp->powerOff       = disp_disable_power;
            disp->getPowerStatus = disp_get_pwrst;
            disp->testModule	 = disp_test_module;

            /* set default brightness */
            //disp->poweron(disp);
            disp->setBright(disp, DISP_BRIGHT_DV, DISP_BRIGHT_DV_DEFAULT);

            TLOGMSG(1, ("create display interface\n"));
        }
        else
        {
            free(this);
            disp = NULL;
            TLOGMSG(1, (DBGINFOFMT "disp_init_gpio return fail\n", DBGINFO));
        }
	}
	else
		TLOGMSG(1, (DBGINFOFMT "%s\n", DBGINFO, strerror(errno)));

	return disp;
}


int
disp_destroy(struct display_interface *disp)
{
	int ret = 0;
    struct display_attribute *this = (struct display_attribute *) disp;

	if (this)
	{
		if (disp_deinit_gpio(disp) == 0)
        {
            free(this);
            TLOGMSG(1, ("destroy display interface\n"));
        }
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "failed to deinit display gpio\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null display interface\n", DBGINFO));
	}

	return ret;
}
