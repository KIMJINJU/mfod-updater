/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    uart.h
        external function/variables/defines for UART interface
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#ifndef _UART_H_
#define _UART_H_

/* constant macro defines : tty device path */
#define UART_TTYMXC0		"/dev/ttymxc0"		/* linux console terminal	*/
#define UART_TTYMXC1		"/dev/ttymxc1"		/* to ircam      			*/
#define UART_TTYMXC2		"/dev/ttymxc2"		/* to mcu sub system		*/
#define UART_TTYMXC3		"/dev/ttymxc3"		/* to external rs232-1		*/
#define UART_TTYMXC4		"/dev/ttymxc4"		/* to external rs232-2		*/
#define UART_TTYUSB0		"/dev/ttyUSB0"		/* to gps-1					*/
#define UART_TTYUSB1		"/dev/ttyUSB1"		/* to dmc-1					*/
#define UART_TTYUSB2		"/dev/ttyUSB2"		/* to lrf-1					*/
#define UART_TTYUSB3		"/dev/ttyUSB3"		/* to gps-2					*/
#define UART_TTYUSB4		"/dev/ttyUSB4"		/* to dmc-2					*/
#define UART_TTYUSB5		"/dev/ttyUSB5"		/* to lrf-3					*/
#define UART_TTYUSB6		"/dev/ttyUSB6"
#define UART_TTYUSB7		"/dev/ttyUSB7"
#define UART_TTYUSB8		"/dev/ttyUSB8"
#define UART_TTYUSB9		"/dev/ttyUSB9"


/* structure declaration : uart interface defines */
struct uart_interface
{
	int (*getFileDesc)(struct uart_interface *);
	int (*open)  	  (struct uart_interface *, char *, int);
	int (*close) 	  (struct uart_interface *);
	int (*write) 	  (struct uart_interface *, char *, int);
	int (*read)       (struct uart_interface *, char *, int);
	int (*getNumRead) (struct uart_interface *);

#ifdef _UART_INTERNAL_ATTRIBUTE_

#endif
};


/* external fucntions for create/destory uart interface */
extern struct uart_interface *uart_create(void);
extern int uart_destroy(struct uart_interface *uart);


#endif /* _UART_H_ */
