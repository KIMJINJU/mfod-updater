/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gpio.h
        external function/variables/defines for GPIO interface
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#ifndef _GPIO_H_
#define _GPIO_H_


/* REPLACING TEXT MACROS */
#define GPIO_NR(bank, nr)   (((bank) - 1) * 32 + (nr))

/* 	GPIO1 GROUP */
#define GPIO1_CPLD_PWR      GPIO_NR(1, 2)
#define GPIO1_KEY_IMCAP     GPIO_NR(1, 3)
#define GPIO1_KEY_OBMD      GPIO_NR(1, 4)
#define GPIO1_CPLD_READY    GPIO_NR(1, 5)   // C_Ready
#define GPIO1_CPLD_DATARDY  GPIO_NR(1, 6)   // Range_data_read
#define GPIO1_SD3_DET       GPIO_NR(1, 7)
#define GPIO1_SD3_WP        GPIO_NR(1, 8)
#define GPIO1_LRF_RXPWR     GPIO_NR(1, 9)
#define GPIO1_KEY_LEFT      GPIO_NR(1, 10)
#define GPIO1_KEY_LRF       GPIO_NR(1, 11)
#define GPIO1_KEY_MENU      GPIO_NR(1, 12)
#define GPIO1_KEY_UP        GPIO_NR(1, 13)
#define GPIO1_KEY_DOWN      GPIO_NR(1, 14)
#define GPIO1_KEY_RIGHT     GPIO_NR(1, 15)

/* 	GPIO2 GROUP */
#define GPIO2_LAN8720_RST   GPIO_NR(2, 0)
#define GPIO2_OLED_RST      GPIO_NR(2, 3)
#define GPIO2_OLED_VDD      GPIO_NR(2, 4)
#define GPIO2_OLED_VAA      GPIO_NR(2, 5)
#define GPIO2_OLED_VAN      GPIO_NR(2, 6)

/* 	GPIO3 GROUP */
#define GPIO3_VDAC_RESET    GPIO_NR(3, 16)  // DAC_RSTn
#define GPIO3_OLED_DE       GPIO_NR(3, 19)
#define GPIO3_EIM_D20       GPIO_NR(3, 20)
#define GPIO3_AVID_RESET    GPIO_NR(3, 22)  // TVOUT_RSTn
#define GPIO3_SHUTTER_RESET GPIO_NR(3, 23)  // SHUTTER_RESET
#define GPIO3_EIM_D29       GPIO_NR(3, 29)

/* 	GPIO4 GROUP */
#define GPIO4_ENET_PWR      GPIO_NR(4, 5)
#define GPIO4_CPLD_SPARE1   GPIO_NR(4, 10)
#define GPIO4_CPLD_SPARE2   GPIO_NR(4, 11)
#define GPIO4_UART_RESET    GPIO_NR(4, 14)  // UART_nRST
#define GPIO4_IRCAM_PWR     GPIO_NR(4, 15)
#define GPIO4_VBUF_PWR      GPIO_NR(4, 20)

/* 	GPIO5 GROUP */
#define GPIO5_TW8809_RESET  GPIO_NR(5, 2)   // TW8809_RSTb
#define GPIO5_CSI0_RESET    GPIO_NR(5, 22)  // CSI0_RST_B
#define GPIO5_CPLD_FCON     GPIO_NR(5, 23)  // C_Fire_control
#define GPIO5_CPLD_DATAREAD GPIO_NR(5, 24)  // Data_read_OK
#define GPIO5_CPLD_RESET    GPIO_NR(5, 25)  // C_nReset
#define GPIO5_CPLD_START    GPIO_NR(5, 26)  // C_Start

/* 	GPIO6 GROUP */
#define GPIO6_GPS_RSVD2     GPIO_NR(6, 7)
#define GPIO6_GPS_RSVD1     GPIO_NR(6, 8)
#define GPIO6_VCONV_1V8     GPIO_NR(6, 9)   // TW8809_1V8
#define GPIO6_ENC_PWR       GPIO_NR(6, 11)
#define GPIO6_MAIN_5V       GPIO_NR(6, 14)
#define GPIO6_LRF_TXPWR     GPIO_NR(6, 15)  // LRF_PWR_ONOFF
#define GPIO6_SD2_DET2      GPIO_NR(6, 16)

/* 	GPIO7 GROUP */
#define GPIO7_PMII_INT      GPIO_NR(7, 12)
#define GPIO7_PMIC_INTB     GPIO_NR(7, 13)

/* GPIO DIRECTIONS */
#define GPIO_DIR_NONE		-1
#define GPIO_DIR_INPUT		0
#define GPIO_DIR_OUTPUT		1

/* GPIO VALUE */
#define GPIO_VALUE_NONE		-1
#define GPIO_VALUE_LOW		0
#define GPIO_VALUE_HIGH		1

/* GPIO EDGE */
#define GPIO_EDGE_NONE		0
#define GPIO_EDGE_FALL		1
#define GPIO_EDGE_RISE		2
#define GPIO_EDGE_BOTH		3


/* STRUCT DECLARATIONS : GPIO INTERFACE */
struct gpio_interface
{
	int (*export)   (struct gpio_interface *, int);
	int (*unexport) (struct gpio_interface *);
	int (*setDir)   (struct gpio_interface *, int);
	int (*getDir)	(struct gpio_interface *, int *);
	int (*setVal)	(struct gpio_interface *, int);
	int (*getVal)	(struct gpio_interface *, int *);
	int (*setEdge)	(struct gpio_interface *, int);
	int (*open)		(struct gpio_interface *);
	int (*close)	(struct gpio_interface *);
	int (*read)		(struct gpio_interface *, int *);

#ifdef _GPIO_INTERNAL_INTERFACE_
#endif
};


/* EXTERNAL FUNCTION DECLARATIONS : CREATE/DESTROY GPIO INTERFACE */
extern struct gpio_interface *gpio_create(void);
extern int gpio_destroy(struct gpio_interface *gpio);

#endif /*_GPIO_H_ */
