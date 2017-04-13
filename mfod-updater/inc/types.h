/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    types.h
        various type declarations.
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/


#ifndef _TYPES_H_
#define _TYPES_H_


/* structure declaration : point */
struct point
{
    int x;
    int y;
};


/* structure declaration : rectangle */
struct rect
{
    int x;
    int y;
    int w;
    int h;
};

typedef struct point point_t;
typedef struct rect  rect_t;

#endif /* _TYPES_H_ */
