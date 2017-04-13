/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gpio.c
        external/internal function implementations of GPIO interface
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
#include <unistd.h>

#include "core/logger.h"
#include "modules/gpio.h"


/* internal macro defines */
#define	BUFLEN    			128
#define STR_GPIO_SYSDIR    "/sys/class/gpio/"
#define STR_EDGE_NONE  		"none"
#define STR_EDGE_FALL  		"falling"
#define STR_EDGE_RISE  		"rising"
#define STR_EDGE_BOTH  		"both"
#define MAX_GPIO_NUMBER		GPIO_NR(7, 13)
#define MIN_GPIO_NUMBER		GPIO_NR(1, 2)


/* structure declaration : internal attributes of gpio interface */
struct gpio_attribute
{
    /* external interface */
    struct gpio_interface extif;

    /* internal attribute */
	int  fd;
	int  number;
	int  dir;
	int  value;
	int  edge;
	bool export;
};


/* internal functions */
static int
gpio_export(struct gpio_interface *gpio, int ngpio)
{
	int fd = 0;
    int nwr = 0;
	int ret = 0;
    char buffer[BUFLEN] = {0};
    struct gpio_attribute *this = (struct gpio_attribute *)gpio;

	if (this)
	{
		if ((ngpio >= MIN_GPIO_NUMBER) && (ngpio <= MAX_GPIO_NUMBER))
		{
			if (!this->export)
			{
				fd  = open(STR_GPIO_SYSDIR "export", O_WRONLY);

				if (fd != -1)
				{
					memset(buffer, 0x00, sizeof(buffer));
					nwr = snprintf(buffer, sizeof(buffer), "%d", ngpio);

					if (nwr > 0)
					{
						if (write(fd, buffer, nwr) == nwr)
						{
							this->export = true;
							this->number = ngpio;
							TLOGMSG(0, ("export gpio%d \n", this->number));
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
						TLOGMSG(1, (DBGINFOFMT "snprintf return fail, %s\n", DBGINFO, strerror(errno)));
					}

					close(fd);
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
				TLOGMSG(1, (DBGINFOFMT "gpio is already exported\n", DBGINFO));
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "invalid gpio number\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null gpio interface\n", DBGINFO));
	}

	return ret;
}


static int
gpio_unexport(struct gpio_interface *gpio)
{
	int fd  = 0;
	int nwr = 0;
	int ret = 0;
	char buf[BUFLEN] = {0};
    struct gpio_attribute *this = (struct gpio_attribute *)gpio;

	if (this)
	{
		if (this->export)
		{
			fd  = open(STR_GPIO_SYSDIR "unexport", O_WRONLY);

			if (fd != -1)
			{
				memset(buf, 0x00, sizeof(buf));
				nwr = snprintf(buf, sizeof(buf), "%d", this->number);

				if (nwr > 0)
				{
					if (write(fd, buf, nwr) == nwr)
					{
						this->export = false;
						TLOGMSG(0, ("unexport gpio%d\n", this->number));
					}
					else
					{
						ret = -1;
						TLOGMSG(1, (DBGINFOFMT", %s\n", DBGINFO, strerror(errno)));
					}
				}
				else
				{
					ret = -1;
					TLOGMSG(1, (DBGINFOFMT "snprintf return fail, %s\n", DBGINFO, strerror(errno)));
				}

				close(fd);
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
			TLOGMSG(1, (DBGINFOFMT "gpio isn't exported\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null gpio interface\n", DBGINFO));
	}

	return ret;
}


static int
gpio_set_direction(struct gpio_interface *gpio, int dir)
{
	int fd  = 0;
	int ret = 0;
	char buf[BUFLEN] = {0};
    struct gpio_attribute *this = (struct gpio_attribute *)gpio;

	if (this)
	{
		if (this->export)
		{
			memset(buf, 0x00, sizeof(buf));
			snprintf(buf, sizeof(buf), STR_GPIO_SYSDIR "gpio%d/direction", this->number);
			fd = open(buf, O_WRONLY);

			if (fd != -1)
			{
				switch(dir)
				{
				case GPIO_DIR_INPUT:
					ret = write(fd, "in", 2);
					this->dir = GPIO_DIR_INPUT;
					TLOGMSG(0, ("gpio%d direction = input\n", this->number));
					break;

				case GPIO_DIR_OUTPUT:
					ret = write(fd, "out", 3);
					this->dir = GPIO_DIR_OUTPUT;
					TLOGMSG(0, ("gpio%d direction = output\n", this->number));
					break;

				default:
					ret = -1;
					TLOGMSG(1, (DBGINFOFMT "invalid direction value\n", DBGINFO));
					break;
				}

				close(fd);
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
			TLOGMSG(1, (DBGINFOFMT "gpio isn't exported\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null gpio interface\n", DBGINFO));
	}

	return ret;
}


static int
gpio_get_direction(struct gpio_interface *gpio, int *dir)
{
	int fd  = 0;
	int ret = 0;
	char buf[BUFLEN] = {0};
    struct gpio_attribute *this = (struct gpio_attribute *)gpio;

	if (this)
	{
		if (this->export)
		{
			 memset(buf, 0x00, sizeof(buf));
			 snprintf(buf, sizeof(buf), STR_GPIO_SYSDIR "gpio%d/direction", this->number);
			 fd = open(buf, O_RDONLY);

			 if (fd != -1)
			 {
				 read(fd, buf, sizeof(buf));

				 if (strcmp(buf, "in") == 0)
				 {
					 *dir = GPIO_DIR_INPUT;
					 this->dir = GPIO_DIR_INPUT;
					 TLOGMSG(0, ("gpio%d direction = input\n", this->number));
				 }
				 else
				 {
					 *dir = GPIO_DIR_OUTPUT;
					 this->dir = GPIO_DIR_OUTPUT;
					 TLOGMSG(0, ("gpio%d direction = output\n", this->number));
				 }

				 close(fd);
			 }
			 else
			 {
				 ret = -1;
				 TLOGMSG(1, (DBGINFOFMT ", %s\n", DBGINFO, strerror(errno)));
			 }
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "gpio isn't exported\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null gpio interface\n", DBGINFO));
	}

	return ret;
}


static int
gpio_set_value(struct gpio_interface *gpio, int val)
{
	int fd  = 0;
	int ret = 0;
	char buf[BUFLEN] = {0};
    struct gpio_attribute *this = (struct gpio_attribute *)gpio;

	if (this)
	{
		if (this->export)
		{
			memset(buf, 0x00, sizeof(buf));
			snprintf(buf, sizeof(buf), STR_GPIO_SYSDIR "gpio%d/value", this->number);
			fd = open(buf, O_WRONLY);

			if (fd != -1)
			{
				switch(val)
				{
				case GPIO_VALUE_HIGH:
					write(fd, "1", 1);
					this->value = GPIO_VALUE_HIGH;
					TLOGMSG(0, ("gpio%d value = high\n", this->number));
					break;

				case GPIO_VALUE_LOW:
					write(fd, "0", 1);
					this->value = GPIO_VALUE_LOW;
					TLOGMSG(0, ("gpio%d value = low\n", this->number));
					break;

				default:
					ret = -1;
					TLOGMSG(1, (DBGINFOFMT "invalid value for gpio\n", DBGINFO));
					break;
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
			TLOGMSG(1, (DBGINFOFMT "gpio isn't exported\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null gpio interface\n", DBGINFO));
	}

	return ret;
}


static int
gpio_get_value(struct gpio_interface *gpio, int *val)
{
	int fd  = 0;
	int ret = 0;
	char retval = 0;
	char buf[BUFLEN] = {0};
    struct gpio_attribute *this = (struct gpio_attribute *)gpio;

	if (this)
	{
		if (this->export)
		{
			memset(buf, 0x00, sizeof(buf));
			snprintf(buf, sizeof(buf), STR_GPIO_SYSDIR "gpio%d/value", this->number);
			fd = open(buf, O_RDONLY);

			if (fd != -1)
			{
				read(fd, &retval, 1);

				if (retval == '0')
				{
					*val = GPIO_VALUE_LOW;
					this->value = GPIO_VALUE_LOW;
					TLOGMSG(0, ("gpio%d value = low\n", this->number));
				}
				else if (retval == '1')
				{
				    *val = GPIO_VALUE_HIGH;
				    this->value = GPIO_VALUE_HIGH;
				    TLOGMSG(0, ("gpio%d value = high\n", this->number));
				}
				else
				{
					ret = -1;
					*val = GPIO_VALUE_NONE;
					this->value = GPIO_VALUE_NONE;
					TLOGMSG(1, (DBGINFOFMT "invalid gpio value\n", DBGINFO));
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
			TLOGMSG(1, (DBGINFOFMT "gpio isn't exported\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null gpio interface\n", DBGINFO));
	}

	return ret;
}


static int
gpio_set_edge(struct gpio_interface *gpio, int edge)
{
	int fd = 0;
	int ret = 0;
	char *str = NULL;
	char buf[BUFLEN] = {0};
    struct gpio_attribute *this = (struct gpio_attribute *)gpio;

	if (this)
	{
		memset(buf, 0x00, sizeof(buf));
		snprintf(buf, sizeof(buf), STR_GPIO_SYSDIR "gpio%d/edge", this->number);
		fd = open(buf, O_WRONLY);

		if (fd < 0)
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "open return fail, %s\n", DBGINFO, strerror(errno)));
		}
		else
		{
			switch (edge)
			{
			case GPIO_EDGE_RISE:
				str = STR_EDGE_RISE;
				break;

			case GPIO_EDGE_FALL:
				str = STR_EDGE_FALL;
				break;

			case GPIO_EDGE_BOTH:
				str = STR_EDGE_BOTH;
				break;

			default:
				edge = GPIO_EDGE_NONE;
				str = STR_EDGE_NONE;
				break;
			}

			if (write(fd, str, strlen(str) + 1) < 0)
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "%s\n", DBGINFO, strerror(errno)));
			}
			else
			{
				this->edge = edge;
				TLOGMSG(0, ("gpio%d edge = %s\n", this->number, str));
			}

			close(fd);
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null gpio interface\n", DBGINFO));
	}

	return ret;
}


static int
gpio_open(struct gpio_interface *gpio)
{
	int ret = 0;
	char buf[BUFLEN] = {0};
    struct gpio_attribute *this = (struct gpio_attribute *)gpio;

	if (this)
	{
		if (this->fd == -1)
		{
			memset(buf, 0x00, sizeof(buf));
			snprintf(buf, sizeof(buf), STR_GPIO_SYSDIR "gpio%d/value", this->number);
			this->fd = open(buf, O_RDONLY | O_NONBLOCK);

			if (this->fd < 0)
			{
				ret = -1;
				this->fd = -1;
				TLOGMSG(0, (DBGINFOFMT "%s\n", DBGINFO, strerror(errno)));
			}
			else
				TLOGMSG(0, ("open gpio%d\n", this->number));
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "gpio%d is already open\n", DBGINFO, this->number));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null gpio interface\n", DBGINFO));
	}

	return ret;
}


static int
gpio_close(struct gpio_interface *gpio)
{
	int ret = 0;
    struct gpio_attribute *this = (struct gpio_attribute *)gpio;

	if (this)
	{
		if (this->fd != -1)
		{
			if (close(this->fd) == 0)
			{
				this->fd = -1;
				TLOGMSG(0, ("gpio%d is closed\n", this->number));
			}
			else
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "close return fail, %s\n", DBGINFO, strerror(errno)));
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "gpio%d is already closed\n", DBGINFO, this->number));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null gpio interface\n", DBGINFO));
	}

	return ret;
}


static int
gpio_read(struct gpio_interface *gpio, int *val)
{
	int ret = 0;
	char retval = 0;
    struct gpio_attribute *this = (struct gpio_attribute *)gpio;

	if (this)
	{
		if(this->fd != -1)
		{
			lseek(this->fd, 0, SEEK_SET);

			if (read(this->fd, &retval, 1) < 0)
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "read return fail, %s\n", DBGINFO, strerror(errno)));
			}
			else
			{
				if (retval != '0')
					*val = GPIO_VALUE_HIGH;
				else
					*val = GPIO_VALUE_LOW;

				TLOGMSG(0, ("read gpio%d value = %s\n", this->number, retval == '0' ? "low" : "high"));
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "gpio%d is not open\n", DBGINFO, this->number));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null gpio interface\n", DBGINFO));
	}

	return ret;
}


/* external functions for create/destroy gpio interface */
struct gpio_interface *
gpio_create(void)
{
	struct gpio_interface *gpio = NULL;
	struct gpio_attribute *this = malloc(sizeof(struct gpio_attribute));

	if (this)
	{
        /* initailize attributes */
        this->fd = -1;
        this->dir = GPIO_DIR_NONE;
        this->edge = GPIO_EDGE_NONE;
        this->export = false;
        this->value  = GPIO_VALUE_NONE;
        this->number = -1;

        /* initialize external interface */
        gpio = &(this->extif);
        gpio->export   = gpio_export;
        gpio->unexport = gpio_unexport;
        gpio->setDir   = gpio_set_direction;
        gpio->getDir   = gpio_get_direction;
        gpio->setVal   = gpio_set_value;
        gpio->getVal   = gpio_get_value;
        gpio->setEdge  = gpio_set_edge;
        gpio->open	   = gpio_open;
        gpio->close	   = gpio_close;
        gpio->read	   = gpio_read;

        TLOGMSG(0, ("create gpio interface\n"));
	}
	else
		TLOGMSG(1, (DBGINFOFMT "failed to create gpio interface, malloc return null\n", DBGINFO));

	return gpio;
}


int
gpio_destroy(struct gpio_interface *gpio)
{
	int ret = 0;
    struct gpio_attribute *this = (struct gpio_attribute *) gpio;

	if (this)
	{
		if (this->fd != -1)
			gpio->close(gpio);

		if (this->export)
			gpio->unexport(gpio);

		free(this);
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null gpio interface\n", DBGINFO));
	}

	return ret;
}
