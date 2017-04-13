/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    uart.c
        external/internal function implementations of UART interface
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "core/logger.h"
#include "modules/uart.h"


#define BUFLEN		256


/* structure declarations : internal attributes of uart interface */
struct uart_attribute
{
	/* external interface */
	struct uart_interface extif;

	/* internal attribute */
	int  fd;
	int  baud;
	char tty[BUFLEN];
	struct termios options;
};


/* internal functions */
static int
uart_open(struct uart_interface *uart, char *tty, int baud)
{
	int ret = 0;
    struct uart_attribute *this = (struct uart_attribute *) uart;

	if (this)
	{
		if (this->fd == -1)
		{
			if (tty)
			{
				strncpy(this->tty, tty, sizeof(char) * BUFLEN);
				this->baud = baud;
				this->fd = open(this->tty, O_RDWR | O_NOCTTY);

				if (this->fd != -1)
				{
					fcntl(this->fd, F_SETFL, 0);
					tcgetattr(this->fd, &(this->options));
					this->options.c_cflag |= CS8;
					this->options.c_cflag &= ~CSTOPB;
					this->options.c_cflag &= ~CSIZE;
					this->options.c_cflag &= ~PARENB;
					this->options.c_cflag &= ~PARODD;
					this->options.c_cflag &= ~CRTSCTS;
					this->options.c_lflag &= ~(ICANON | IEXTEN | ISIG | ECHO);
					this->options.c_oflag &= ~OPOST;
					this->options.c_iflag &= ~(ICRNL | INPCK | ISTRIP | IXON | BRKINT);
					this->options.c_cc[VMIN] = 1;
					this->options.c_cflag |= (CLOCAL | CREAD);

					tcflush(this->fd, TCIOFLUSH);
					cfsetispeed(&(this->options), this->baud);
					cfsetospeed(&(this->options), this->baud);
					tcsetattr(this->fd, TCSAFLUSH, &(this->options));
				}
				else
				{
					ret = -1;
					TLOGMSG(1, (DBGINFOFMT "failed to open uart device, %s\n", DBGINFO, strerror(errno)));
				}
			}
			else
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "null tty string\n", DBGINFO));

			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "uart is already open\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null uart interface\n", DBGINFO));
	}

	return ret;
}


static int
uart_close(struct uart_interface *uart)
{
	int ret = 0;
	struct uart_attribute *this = (struct uart_attribute *) uart;

	if (this)
	{
		if (this->fd != -1)
		{
			close(this->fd);
			this->fd = -1;
			memset(this->tty, 0x00, sizeof(char) * BUFLEN);
			memset(&this->options, 0x00, sizeof(struct termios));
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "uart is already closed\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null uart interface\n", DBGINFO));
	}

	return ret;
}


static int
uart_write(struct uart_interface *uart, char *buf, int nwr)
{
	int ret = 0;
	struct uart_attribute *this = (struct uart_attribute *) uart;

	if (this)
	{
		ret = write(this->fd, buf, nwr);

		if (ret != nwr)
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "write return fail, %s\n", DBGINFO, strerror(errno)));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null uart interface\n", DBGINFO));
	}

	return ret;
}


static int
uart_read(struct uart_interface *uart, char *buf, int nread)
{
	int ret = 0;
	struct uart_attribute *this = (struct uart_attribute *) uart;

	if (this)
	{
		if (nread > 0)
		{
			ret = read(this->fd, buf, nread);

			if (ret < 0)
				TLOGMSG(1, (DBGINFOFMT "read return fail, %s\n", DBGINFO, strerror(errno)));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null uart interface\n", DBGINFO));
	}

	return ret;
}


static int
uart_get_nread(struct uart_interface *uart)
{
	int nread = 0;
	struct uart_attribute *this = (struct uart_attribute *) uart;

	if (this)
	{
		if (ioctl(this->fd, FIONREAD, &nread) < 0)
		{
			nread = -1;
			TLOGMSG(1, (DBGINFOFMT "ioctl return fail(FIONREAD)\n", DBGINFO));
		}
	}
	else
	{
		nread = -1;
		TLOGMSG(1, (DBGINFOFMT "null uart interface\n", DBGINFO));
	}

	return nread;
}


static int uart_getfd(struct uart_interface *uart)
{
	int fd = 0;
	struct uart_attribute *this = (struct uart_attribute *) uart;

	if (this)
	{
		if (this->fd != -1)
			fd = this->fd;
		else
			fd = -1;
	}
	else
	{
		fd = -1;
		TLOGMSG(1, (DBGINFOFMT "null uart interface\n", DBGINFO));
	}

	return fd;
}


/* external functions for create/destroy uart interface */
struct uart_interface *
uart_create(void)
{
	struct uart_interface *uart = NULL;
	struct uart_attribute *this = malloc(sizeof(struct uart_attribute));

	if (this)
	{
		/* initialize internal data member */
		this->fd   = -1;
		this->baud = -1;
		memset(this->tty, 0x00, sizeof(char) * BUFLEN);
		memset(&this->options, 0x00, sizeof(struct termios));

		/* set external interface */
		uart = &(this->extif);
		uart->open  = uart_open;
		uart->close = uart_close;
		uart->read  = uart_read;
		uart->write = uart_write;
		uart->getNumRead  = uart_get_nread;
		uart->getFileDesc = uart_getfd;
		TLOGMSG(1, ("create uart interface\n"));
	}
	else
		TLOGMSG(1, (DBGINFOFMT "failed to create uart interface, malloc return null\n", DBGINFO));

	return uart;
}


int
uart_destroy(struct uart_interface *uart)
{
	int ret = 0;
	struct uart_attribute *this = (struct uart_attribute *) uart;

	if (this)
	{
		if(this->fd != -1)
			uart->close(uart);

		free(this);
		TLOGMSG(1, ("destroy uart interface\n"));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null uart interface\n", DBGINFO));
	}

	return ret;
}
