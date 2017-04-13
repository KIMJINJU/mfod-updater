/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_caption.h
        external function/variables/defines for caption widget
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/


#ifndef _GUI_CAPTION_H_
#define _GUI_CAPTION_H_

#include "types.h"

/* macro defines : caption align */
#define CAPTION_ALIGN_LEFT          0
#define CAPTION_ALIGN_CENTER        1
#define CAPTION_ALIGN_RIGHT         2

/* macro defines : caption font type */
#define CAPTION_FONT_NORMAL         0
#define CAPTION_FONT_MONOSPACED     1


/* structure declaration : caption widget interface */
typedef struct gui_caption_interface
{
    int    (*setPosition)    (struct gui_caption_interface *, rect_t *);
    int    (*getPosition)    (struct gui_caption_interface *, rect_t *);
    int    (*setString)      (struct gui_caption_interface *, char *);
    char * (*getString)      (struct gui_caption_interface *);
    int    (*setAlign)       (struct gui_caption_interface *, int);
    int    (*getAlign)       (struct gui_caption_interface *);
    int    (*setFontHeight)  (struct gui_caption_interface *, int);
    int    (*getFontHeight)  (struct gui_caption_interface *);
    int    (*setFont)        (struct gui_caption_interface *, int);
    int    (*getFont)        (struct gui_caption_interface *);
    int    (*getStringLength)(struct gui_caption_interface *);

#ifdef _ENABLE_CAPTION_INTERNAL_INTERFACE_

#endif /* _ENABLE_CAPTION_INTERNAL_INTERFACE_ */
}
caption_t;


/* external functions for create/destroy caption widget */
extern struct gui_caption_interface *gui_caption_create(void);
extern int gui_caption_destroy(struct gui_caption_interface *caption);


#endif /* _GUI_CAPTION_H_ */
