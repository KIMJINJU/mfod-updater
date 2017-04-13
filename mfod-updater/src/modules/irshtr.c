/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    irshtr.c
        external/internal function implementations of IRSHTR interface
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "core/logger.h"
#include "etc/util.h"
#include "modules/gpio.h"
#include "modules/irshtr.h"

/* macro defines : i2c device path and slave address */
#define IRSHTR_I2C_DEVPATH		"/dev/i2c-0"
#define IRSHTR_I2C_ADDR			0x52

/* macro defines : convert parameter */
#define CONVPARM(parm, buf) \
		buf[0] = parm & 0x00FF; buf[1] = (parm >> 8) & 0x00FF;

/* macro defines : shutter command id */
#define IRSHTR_ID_OPENLOOP				0x07
#define IRSHTR_ID_CALIBRATE				0x08
#define IRSHTR_ID_SLEEP					0x09
#define IRSHTR_ID_SAVE_PARAM			0x0C
#define IRSHTR_ID_RESTR_PARAM			0x0D
#define IRSHTR_ID_GET_INFO				0x13
#define IRSHTR_ID_SET_POS				0x17
#define IRSHTR_ID_SET_TIMEOUT			0x19
#define IRSHTR_ID_SET_VELOCITY			0x21
#define IRSHTR_ID_PWR_SAVE				0x2D
#define IRSHTR_ID_KEEP_POS				0x2E
#define IRSHTR_ID_TEMP_PROC				0x2F
#define IRSHTR_ID_PWM_LIMIT				0x30
#define IRSHTR_ID_HOME					0x32
#define IRSHTR_ID_SET_LOW_VELOCITY		0x34
#define IRSHTR_ID_SET_VELOCITY_RAMP		0x35
#define IRSHTR_ID_GET_VAR_BY_ID			0xF8

/* macro defines : command status */
#define IRSHTR_CMD_STATUS_IDLE			1
#define IRSHTR_CMD_STATUS_ERROR			2
#define IRSHTR_CMD_STATUS_BUSY			3

/* macro defines : motor status bit */
#define IRSHTR_MOTOR_STBIT_IS			0x01
#define IRSHTR_MOTOR_STBIT_MOTION		0x02
#define IRSHTR_MOTOR_STBIT_VELOCITY		0x04
#define IRSHTR_MOTOR_STBIT_TIMEOUT		0x08
#define IRSHTR_MOTOR_STBIT_CAL			0x10
#define IRSHTR_MOTOR_STBIT_POS			0x20

/* macro defines : status bit */
#define IRSHTR_BIT_MOTION_IDLE			0
#define IRSHTR_BIT_MOTION_IN_MOTION		1
#define IRSHTR_BIT_VELOCITY_OK			0
#define IRSHTR_BIT_VELOCITY_BELOW		1
#define IRSHTR_BIT_TIMEOUT_NOT			0
#define IRSHTR_BIT_TIMEOUT_IS			1
#define IRSHTR_BIT_CAL_FAIL				0
#define IRSHTR_BIT_CAL_SUCCESS			1
#define IRSHTR_BIT_POS_OPEN				0
#define IRSHTR_BIT_POS_CLOSE			1

/* macro defines : default setting values */
#define DEFAULT_PWM_LIMIT				30000
#define DEFAULT_VELOCITY				2000
#define DEFAULT_LOW_VELOCITY			600

/* macro defines : status buffer length */
#define IRSHTR_STATUS_BUFLEN			18

/* macro defines : command execution timeout (ms) */
#define IRSHTR_CMDEXE_TIMEOUT			3000


/* structure declaration : irshtr interface attribute */
struct irshtr_attribute
{
	/* external interface */
	struct irshtr_interface extif;

	/* internal attribute */
	bool open;
	int  i2cfd;
	struct gpio_interface *reset;
};


static int
irshtr_write_i2c(struct irshtr_interface *irshtr, unsigned char cmd, unsigned char *parm)
{
	int ret = 0;
	unsigned char buf[3] = {0, 0, 0};
	struct irshtr_attribute *this = (struct irshtr_attribute *) irshtr;

	if (this)
	{
		buf[0] = cmd;

		if (parm == NULL)
		{
			buf[1] = 0;
			buf[2] = 0;
		}
		else
		{
			buf[1] = *parm;
			buf[2] = *(parm + 1);
		}

		ret = write(this->i2cfd, buf, sizeof(buf));

		if (ret < 0)
			TLOGMSG(0, (DBGINFOFMT "ret = %d, %s\n", DBGINFO, ret, strerror(errno)));
		else
			MSLEEP(10);
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null irshtr interface\n", DBGINFO));
	}

	return ret;
}


static int
irshtr_read_i2c(struct irshtr_interface *irshtr, unsigned char *buf, int buflen)
{
	int ret = 0;
	int nread = 0;
	struct irshtr_attribute *this = (struct irshtr_attribute *) irshtr;

	if (this)
	{
		nread = read(this->i2cfd, buf, buflen);

		if (nread < 0)
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "%s\n", DBGINFO, strerror(errno)));
		}
		else
			ret = nread;
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null irshtr interface\n", DBGINFO));
	}

	return ret;
}


static int
irshtr_open_i2c(struct irshtr_interface *irshtr)
{
	int ret = 0;
	struct irshtr_attribute *this = (struct irshtr_attribute *) irshtr;

	if (this)
	{
		this->i2cfd = open(IRSHTR_I2C_DEVPATH, O_RDWR);
		TLOGMSG(0, ("i2cfd = %d\n", this->i2cfd));

		if (this->i2cfd != -1)
		{
			if (ioctl(this->i2cfd, I2C_SLAVE, IRSHTR_I2C_ADDR) == 0)
				TLOGMSG(0, ("open i2c slave channel (%s, address = 0x%02X )\n", IRSHTR_I2C_DEVPATH, IRSHTR_I2C_ADDR));
			else
			{
				ret = -1;
				close(this->i2cfd);
				TLOGMSG(1, (DBGINFOFMT "%s\n", DBGINFO, strerror(errno)));
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "%s\n", DBGINFO, strerror(errno)));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null irshtr interface\n", DBGINFO));
	}

	return ret;
}


static int
irshtr_close_i2c(struct irshtr_interface *irshtr)
{
	int ret = 0;
	struct irshtr_attribute *this = (struct irshtr_attribute *) irshtr;

	if (this)
	{
		if (this->i2cfd != -1)
		{
			close(this->i2cfd);
			this->i2cfd = -1;
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "i2cfd is not open\n", DBGINFO));
		}
	}
	else
		TLOGMSG(1, (DBGINFOFMT "null irshtr interface\n", DBGINFO));

	return ret;
}


static int
irshtr_check_cmdstatus(struct irshtr_interface *irshtr, int timeo)
{
	int ret = 0;
	int count = 0;
	unsigned char buf[IRSHTR_STATUS_BUFLEN] = {0};
	struct irshtr_attribute *this = (struct irshtr_attribute *) irshtr;

	if (this)
	{
		while (true)
		{
			if (count < timeo)
			{
				irshtr_read_i2c(irshtr, buf, IRSHTR_STATUS_BUFLEN - 1);

				if (buf[1] == IRSHTR_CMD_STATUS_IDLE)
				{
					TLOGMSG(0, ("irshtr command execution success (cmd = 0x%02X)\n", buf[0]));
					break;
				}
				else if (buf[1] == IRSHTR_CMD_STATUS_ERROR)
				{
					ret = -1;
					TLOGMSG(1, (DBGINFOFMT "command execution failed (cmd = 0x%02X)\n", buf[0], DBGINFO));
					break;
				}
				else
				{
					count = count + 100;
					MSLEEP(100);
				}
			}
			else
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "timeout occurred while waiting command execution status\n", DBGINFO));
				break;
			}
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null irshtr interface\n", DBGINFO));
	}

	return ret;
}


static int
irshtr_set_pwm_limit(struct irshtr_interface *irshtr, unsigned int val)
{
	int ret = 0;
	unsigned char parm[2] = {0, 0};
	struct irshtr_attribute *this = (struct irshtr_attribute *) irshtr;

	if (this)
	{
		CONVPARM(val, parm);

		if (irshtr_write_i2c(irshtr, IRSHTR_ID_PWM_LIMIT, parm) != -1)
		{
			if (irshtr_check_cmdstatus(irshtr, IRSHTR_CMDEXE_TIMEOUT) == 0)
				TLOGMSG(1, ("irshtr pwm limit = %d\n", val));
			else
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "failed to set pwm limit\n", DBGINFO));
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "irshtr_write_i2c return fail\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null irshtr interface\n", DBGINFO));
	}

	return ret;
}


static int
irshtr_set_velocity(struct irshtr_interface *irshtr, unsigned int val)
{
	int ret = 0;
	unsigned char parm[2] = {0, 0};
	struct irshtr_attribute *this = (struct irshtr_attribute *) irshtr;

	if (this)
	{
		CONVPARM(val, parm);

		if (irshtr_write_i2c(irshtr, IRSHTR_ID_SET_VELOCITY, parm) != -1)
		{
			if (irshtr_check_cmdstatus(irshtr, IRSHTR_CMDEXE_TIMEOUT) == 0)
				TLOGMSG(1, ("irshtr velocity = %d deg/sec\n", val));
			else
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "failed to set velocity\n", DBGINFO));
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "irshtr_write_i2c return fail\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null irshtr interface\n", DBGINFO));
	}

	return ret;
}


static int
irshtr_set_low_velocity(struct irshtr_interface *irshtr, unsigned int val)
{
	int ret = 0;
	unsigned char parm[2] = {0};
	struct irshtr_attribute *this = (struct irshtr_attribute *) irshtr;

	if (this)
	{
		CONVPARM(val, parm);

		if (irshtr_write_i2c(irshtr, IRSHTR_ID_SET_LOW_VELOCITY, parm) != -1)
		{
			if (irshtr_check_cmdstatus(irshtr, IRSHTR_CMDEXE_TIMEOUT) == 0)
				TLOGMSG(1, ("irshtr low velocity = %d deg/sec\n", val));
			else
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "failed to set low velocity\n", DBGINFO));
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "irshtr_write_i2c return fail\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null irshtr interface\n", DBGINFO));
	}

	return ret;
}


static int
irshtr_keep_position(struct irshtr_interface *irshtr, unsigned int val)
{
	int ret = 0;
	unsigned char parm[2] = {0, 0};
	struct irshtr_attribute *this = (struct irshtr_attribute *) irshtr;

	if (this)
	{
		CONVPARM(val, parm)

		if (irshtr_write_i2c(irshtr, IRSHTR_ID_KEEP_POS, parm) != -1)
		{
			if (irshtr_check_cmdstatus(irshtr, IRSHTR_CMDEXE_TIMEOUT) == 0)
				TLOGMSG(1, ("irshtr keep position = %d\n", val));
			else
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "failed to set keep position\n", DBGINFO));
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "irshtr_write_i2c return fail\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null irshtr interface\n", DBGINFO));
	}

	return ret;
}


static int
irshtr_reset(struct irshtr_interface *irshtr)
{
	int ret = 0;
	struct gpio_interface *reset = NULL;
	struct irshtr_attribute *this = (struct irshtr_attribute *) irshtr;

	if (irshtr)
	{
		reset = this->reset;
		reset->setVal(reset, GPIO_VALUE_LOW);
		MSLEEP(250);
		reset->setVal(reset, GPIO_VALUE_HIGH);
		MSLEEP(250);
		TLOGMSG(1, ("reset internal irshtr\n"));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null irshtr interface\n", DBGINFO));
	}

	return ret;
}


static int
irshtr_open(struct irshtr_interface *irshtr)
{
	int ret = 0;
	unsigned char parm[2] = {0, 0};
	struct irshtr_attribute *this = (struct irshtr_attribute *) irshtr;

	if (this)
	{
		CONVPARM(IRSHTR_OPEN, parm);

		if (irshtr_write_i2c(irshtr, IRSHTR_ID_SET_POS, parm) != 0)
		{
			if (irshtr_check_cmdstatus(irshtr, IRSHTR_CMDEXE_TIMEOUT) == 0)
			{
				MSLEEP(300);
				this->open = true;
				TLOGMSG(1, ("open irshtr\n"));
			}
			else
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "failed to open shutter\n", DBGINFO));
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "irshtr_write_i2c return fail\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null irshtr interface\n", DBGINFO));
	}

	return ret;
}


static int
irshtr_close(struct irshtr_interface *irshtr)
{
	int ret = 0;
	unsigned char parm[2] = {0, 0};
	struct irshtr_attribute *this = (struct irshtr_attribute *) irshtr;

	if (this)
	{
		CONVPARM(IRSHTR_CLOSE, parm);

		if (irshtr_write_i2c(irshtr, IRSHTR_ID_SET_POS, parm) != 0)
		{
			if (irshtr_check_cmdstatus(irshtr, IRSHTR_CMDEXE_TIMEOUT) == 0)
			{
				MSLEEP(300);
				this->open = false;
				TLOGMSG(1, ("close irshtr\n"));
			}
			else
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "failed to close shutter\n", DBGINFO));
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "irshtr_write_i2c return fail\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null irshtr interface\n", DBGINFO));
	}

	return ret;
}


static int
irshtr_get_position(struct irshtr_interface *irshtr)
{
	int ret = 0;
	struct irshtr_attribute *this = (struct irshtr_attribute *) irshtr;

	if (this)
	{
		if (this->open)
			ret = IRSHTR_OPEN;
		else
			ret = IRSHTR_CLOSE;
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null irshtr interface\n", DBGINFO));
	}

	return ret;
}


static int
irshtr_test_module(struct irshtr_interface *irshtr)
{
	int ret = 0;
	struct irshtr_attribute *this = (struct irshtr_attribute *) irshtr;

	if (this)
	{
		if (irshtr_close(irshtr) == 0)
		{
			irshtr_open(irshtr);
			TLOGMSG(1, ("ir shutter is ok\n"));
		}
		else
		{
			ret = -1;
			TLOGMSG(1, ("ir shutter error\n"));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null irshtr interface\n", DBGINFO));
	}

	return ret;
}


static int
irshtr_init_attribute(struct irshtr_interface *irshtr)
{
	int ret = 0;
	struct irshtr_attribute *this = (struct irshtr_attribute *) irshtr;

	if (this)
	{
		this->reset = gpio_create();

		if (this->reset)
		{
			this->reset->export(this->reset, GPIO3_SHUTTER_RESET);
			this->reset->setDir(this->reset, GPIO_DIR_OUTPUT);
			this->reset->setVal(this->reset, GPIO_VALUE_HIGH);
			MSLEEP(500);
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "gpio_create return fail\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null irshtr interface\n", DBGINFO));
	}

	return ret;
}


static int
irshtr_deinit_attribute(struct irshtr_interface *irshtr)
{
	int ret = 0;
	struct irshtr_attribute *this = (struct irshtr_attribute *) irshtr;

	if (this)
		gpio_destroy(this->reset);
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null irshtr attribute\n", DBGINFO));
	}

	return ret;
}


/* external functions for create/destroy IRSHTR interface */
struct irshtr_interface *
irshtr_create(void)
{
	struct irshtr_interface *irshtr = NULL;
	struct irshtr_attribute *this = malloc(sizeof(struct irshtr_attribute));

	if (this)
	{
		memset(this, 0x00, sizeof(struct irshtr_attribute));

		irshtr = &(this->extif);
		irshtr->open  = irshtr_open;
		irshtr->close = irshtr_close;
		irshtr->getPosition = irshtr_get_position;
		irshtr->testModule  = irshtr_test_module;

		if (irshtr_init_attribute(irshtr) == 0)
		{
			irshtr_reset(irshtr);

			if (irshtr_open_i2c(irshtr) == 0)
			{
				this->open = true;
				irshtr_set_pwm_limit(irshtr, DEFAULT_PWM_LIMIT);
				irshtr_set_velocity(irshtr, DEFAULT_VELOCITY);
				irshtr_set_low_velocity(irshtr, DEFAULT_LOW_VELOCITY);
				irshtr_keep_position(irshtr, true);
				TLOGMSG(1, ("create irshtr interface\n"));
			}
			else
			{
				irshtr_deinit_attribute(irshtr);
				free(this);
				irshtr = NULL;
				TLOGMSG(1, (DBGINFOFMT "failed to open i2c\n", DBGINFO));
			}
		}
		else
		{
			free(this);
			irshtr = NULL;
			TLOGMSG(1, (DBGINFOFMT "failed to init irshtr attribute\n", DBGINFO));
		}
	}
	else
		TLOGMSG(1, (DBGINFOFMT ", %s\n", DBGINFO, strerror(errno)));

	return irshtr;
}


int
irshtr_destroy(struct irshtr_interface *irshtr)
{
	int ret = 0;
	struct irshtr_attribute *this = (struct irshtr_attribute *) irshtr;

	if (this)
	{
		irshtr_close_i2c(irshtr);
		irshtr_deinit_attribute(irshtr);
		free(this);
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null irshtr interface\n", DBGINFO));
	}

	return ret;
}
