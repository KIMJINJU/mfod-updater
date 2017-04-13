/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    ipu.h
        external function/variables/defines for ipu interface
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#include <stdbool.h>
#include "types.h"


/* structure declarations : blitting parameter */
struct blit_input
{
    unsigned int width;
    unsigned int height;
    unsigned int format;
    unsigned int size;

    void *buf;
    struct rect crop;
};

struct blit_output
{
    unsigned int width;
    unsigned int height;
    unsigned int rotate;
    unsigned int format;

    void *buf;
    struct rect crop;
};

struct blit_parm
{
    struct blit_input input;
    struct blit_output output;
};

/* structure declarations : ipu interface */
struct ipu_interface
{
    int (*setColorKey)   (struct ipu_interface *this, int flag, unsigned char r, unsigned char g, unsigned char b);
    int (*blitToMemory)  ( struct ipu_interface *this, struct blit_parm *parm);
    int (*blitToFrameBuf)(struct ipu_interface *this, struct blit_parm *parm);

#ifdef _IPU_INTERNAL_INTERFACE_

#endif
};

/* external functions */
extern struct ipu_interface *ipu_create(int inwid, int inhgt, unsigned int infmt, int outwid, int outhgt, unsigned int outfmt);
extern int ipu_destroy(struct ipu_interface *ipu);
