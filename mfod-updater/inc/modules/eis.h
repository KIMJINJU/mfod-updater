/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    eis.h
        external function/variables/defines for eis interface
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/



/* structure declarations : eis interface */
struct eis_interface
{
    /* external interface */
    int    (*start)    (struct eis_interface *this);
    int    (*stop)     (struct eis_interface *this);
    int    (*process)  (struct eis_interface *this);
    void * (*getSrcbuf)(struct eis_interface *this);
    void * (*getOutbuf)(struct eis_interface *this);

#ifdef _EIS_INTERNAL_INTERFACE_
#endif
};


/* external functions for create / destroy eis interface */
extern struct eis_interface * eis_create(void);
extern int eis_destroy(struct eis_interface *eis);
