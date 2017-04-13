/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_keyin_handler.h
        external function/variables/defines for gui key input hander interface
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#ifndef _GUI_KEYIN_HANDLER_H_
#define _GUI_KEYIN_HANDLER_H_

/* macro defines : keycode */
#define KEYCODE_NULL                0xFFF
#define KEYCODE_UP                  0x100

#define KEYCODE_DOWN                0x200
#define KEYCODE_LEFT                0x300
#define KEYCODE_RIGHT               0x400
#define KEYCODE_ENTER               0x500
#define KEYCODE_TRIGGER             0x600
#define KEYCODE_CHANNEL             0x700
#define KEYCODE_CAPTURE             0x800
#define KEYCODE_UNKNOWN             0xFFE

/* constant macro defines : keyin status */
#define KEYIN_PUSH                  0x001
#define KEYIN_RELEASE               0x002
#define KEYIN_HOLD                  0x004
#define KEYIN_REPEAT                0x008

/* constant macro defines : keyin event parameters */
#define KEYIN_UP_PUSH               0x101
#define KEYIN_UP_RELEASE            0x102
#define KEYIN_UP_HOLD               0x104
#define KEYIN_UP_HOLD_RELEASE       0x106
#define KEYIN_UP_REPEAT             0x108

#define KEYIN_DOWN_PUSH             0x201
#define KEYIN_DOWN_RELEASE          0x202
#define KEYIN_DOWN_HOLD             0x204
#define KEYIN_DOWN_HOLD_RELEASE     0x206
#define KEYIN_DOWN_REPEAT           0x208

#define KEYIN_LEFT_PUSH             0x301
#define KEYIN_LEFT_RELEASE          0x302
#define KEYIN_LEFT_HOLD             0x304
#define KEYIN_LEFT_HOLD_RELEASE     0x306
#define KEYIN_LEFT_REPEAT           0x308

#define KEYIN_RIGHT_PUSH            0x401
#define KEYIN_RIGHT_RELEASE         0x402
#define KEYIN_RIGHT_HOLD            0x404
#define KEYIN_RIGHT_HOLD_RELEASE    0x406
#define KEYIN_RIGHT_REPEAT          0x408

#define KEYIN_ENTER_PUSH            0x501
#define KEYIN_ENTER_RELEASE         0x502
#define KEYIN_ENTER_HOLD            0x504
#define KEYIN_ENTER_HOLD_RELEASE    0x506
#define KEYIN_ENTER_REPEAT          0x508

#define KEYIN_TRIGGER_PUSH          0x601
#define KEYIN_TRIGGER_RELEASE       0x602
#define KEYIN_TRIGGER_HOLD          0x604
#define KEYIN_TRIGGER_HOLD_RELEASE  0x606
#define KEYIN_TRIGGER_REPEAT        0x608

#define KEYIN_CHANNEL_PUSH          0x701
#define KEYIN_CHANNEL_RELEASE       0x702
#define KEYIN_CHANNEL_HOLD          0x704
#define KEYIN_CHANNEL_HOLD_RELEASE  0x706
#define KEYIN_CHANNEL_REPEAT        0x708

#define KEYIN_CAPTURE_PUSH          0x801
#define KEYIN_CAPTURE_RELEASE       0x802
#define KEYIN_CAPTURE_HOLD          0x804
#define KEYIN_CAPTURE_HOLD_RELEASE  0x806
#define KEYIN_CAPTURE_REPEAT        0x808


/* structure declarations : gui key input handler inpterface */
typedef struct keyin_handler_interface
{
    void (*lock)   (struct keyin_handler_interface *);
    void (*unlock) (struct keyin_handler_interface *);

#ifdef _ENABLE_KEYIN_HANDLER_INTERNAL_INTERFACE_

#endif /* _ENABLE_KEYIN_HANDER_INTERNAL_INTERFACE_ */
}
keyin_handler_t;


/* external functions for create / destroy key input handler interface */
extern struct keyin_handler_interface *gui_keyin_handler_create(IDirectFB *dfb);
extern int gui_keyin_handler_destroy(struct keyin_handler_interface *handler);

#endif /* _GUI_KEYIN_HANDLER_H_ */
