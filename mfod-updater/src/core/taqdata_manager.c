/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    taqdata_manager.c
        external/internal function implementations of taqdata manager interface
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "core/logger.h"
#include "core/taqdata_manager.h"

/* constant macro defines */
#define MAXNUM_TAQDATA		128
#define TAQDATA_FILE_PATH	"/mnt/mmc/amod-data/target/taqdata.dat"


/* struct declaration : taqdata container */
typedef struct taqdata_container
{
    taqdata_t *data;
    struct taqdata_container *prev;
    struct taqdata_container *next;
}
taqdata_container_t;


/* struct declaration : taqdata manager attribute */
struct taqdata_manager_attribute
{
    /* external interface */
    struct taqdata_manager_interface extif;

    /* internal interface */
    int count;
    taqdata_container_t *head;
    taqdata_container_t *tail;
    taqdata_container_t *focus;
};


static int
taqdata_manager_save_taqdata(struct taqdata_manager_interface *mgr, char *path)
{
    int fd = 0;
    int ret = 0;
    int nwr = 0;
    struct taqdata_container *cont = 0;
    struct taqdata_manager_attribute *this = (struct taqdata_manager_attribute *) mgr;

    if (this)
    {
        fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

        if (fd != -1)
        {
            cont = this->tail;

            for (int i = 0; i < this->count; i++)
            {
                if (write(fd, cont->data, sizeof(taqdata_t)) == sizeof(taqdata_t))
                {
                    nwr = nwr + sizeof(taqdata_t);
                    cont = cont->prev;
                }
                else
                {
                    ret = -1;
                    break;
                }
            }

            if (ret == 0)
                TLOGMSG(1, ("saved target data, written %d bytes\n", nwr));
            else
                TLOGMSG(1, (DBGINFOFMT "failed to write, %s\n", DBGINFO, strerror(errno)));

            close(fd);
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to open file\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqdata manager interface\n", DBGINFO));
    }

    return ret;
}


static int
taqdata_manager_load_taqdata(struct taqdata_manager_interface *mgr, char *path)
{
    int fd = 0;
    int ret = 0;
    char buf[sizeof(taqdata_t)] = {0};
    taqdata_t *data = NULL;
    struct taqdata_manager_attribute *this = (struct taqdata_manager_attribute *) mgr;

    if (this)
    {
        fd = open(path, O_RDONLY);

        if (fd != -1)
        {
            if (this->count)
            {
                while(this->count != 0)
                    mgr->removeData(mgr);
            }

            while (read(fd, buf, sizeof(taqdata_t)) == sizeof(taqdata_t))
            {
                data = mgr->createData(mgr);

                if (data)
                {
                    memcpy(data, buf, sizeof(taqdata_t));
                    mgr->addData(mgr, data);
                }
                else
                {
                    ret = -1;
                    break;
                }
            }

            close(fd);

            if (this->count)
                TLOGMSG(1, ("load %d taqdata\n", this->count));
            else
                TLOGMSG(1, (DBGINFOFMT "failed to load taqdata or nothing to load\n"));
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to open file, %s\n", DBGINFO, strerror(errno)));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqdata manager interface\n", DBGINFO));
    }

    return ret;
}


static int
taqdata_manager_add_taqdata(struct taqdata_manager_interface *mgr, taqdata_t *data)
{
    int ret = 0;
    taqdata_container_t *container = NULL;
    taqdata_container_t *delete = NULL;
    struct taqdata_manager_attribute *this = (struct taqdata_manager_attribute *) mgr;

    if (this && data)
    {
        container = malloc(sizeof(taqdata_container_t));

        if (container)
        {
            container->data = data;

            switch (this->count)
            {
            case 0:
                container->next = container;
                container->prev = container;
                this->head = container;
                this->tail = container;
                this->focus = container;
                this->count++;
                break;

            case MAXNUM_TAQDATA:
                delete = this->tail;
                this->tail = this->tail->prev;
                this->tail->next = container;
                container->next = this->head;
                container->prev = this->tail;
                this->head->prev = container;
                this->head = container;
                free(delete->data);
                free(delete);
                break;

            default:
                container->next = this->head;
                container->prev = this->tail;
                this->head->prev = container;
                this->tail->next = container;
                this->head = container;
                this->count++;
                break;
            }

            this->focus = container;
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to create taqdata container\n"));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqdata manager interface or data\n", DBGINFO));
    }

    return ret;
}


static int
taqdata_manager_remove_taqdata(struct taqdata_manager_interface *mgr)
{
    int ret = 0;
    taqdata_container_t *container = NULL;
    struct taqdata_manager_attribute *this = (struct taqdata_manager_attribute *) mgr;

    if (this)
    {
        if (this->count != 0)
        {
            container = this->focus;

            if (this->count > 1)
            {
                if (container == this->head)
                {
                    this->focus = this->focus->next;
                    this->head = this->head->next;
                }
                else if (container == this->tail)
                {
                    this->focus = this->focus->prev;
                    this->tail = this->tail->prev;
                }
                else
                    this->focus = this->focus->prev;
            }
            else
            {
                this->focus = NULL;
                this->head = NULL;
                this->tail = NULL;
            }

            container->prev->next = container->next;
            container->next->prev = container->prev;
            this->count--;
            free(container->data);
            free(container);
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "no taqdata in taqdata manager\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqdata manager interface\n", DBGINFO));
    }

    return ret;
}


static taqdata_t *
taqdata_manager_get_taqdata(struct taqdata_manager_interface *mgr, int idx)
{
    taqdata_t *data = NULL;
    taqdata_container_t *container = NULL;
    struct taqdata_manager_attribute *this = (struct taqdata_manager_attribute *) mgr;

    if (mgr)
    {
        if (this->count != 0)
        {
            if (idx < this->count)
            {
                container = this->head;

                for (int i = 0; i < idx; i++)
                    container = container->next;

                data = container->data;
            }
            else
                TLOGMSG(1, (DBGINFOFMT "invalid index\n", DBGINFO));
        }
        else
            TLOGMSG(1, (DBGINFOFMT "no taqdata in taqdata manager\n", DBGINFO));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null taqdata manager interface\n", DBGINFO));

    return data;
}


static taqdata_t *
taqdata_manager_focus_next(struct taqdata_manager_interface *mgr)
{
    taqdata_t *data = NULL;
    struct taqdata_manager_attribute *this = (struct taqdata_manager_attribute *) mgr;

    if (this)
    {
        if (this->count != 0)
        {
            this->focus = this->focus->next;
            data = this->focus->data;
        }
        else
            TLOGMSG(1, (DBGINFOFMT "no taqdata in taqdata manager\n", DBGINFO));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null taqdata manager interface\n", DBGINFO));

    return data;
}


static taqdata_t *
taqdata_manager_focus_prev(struct taqdata_manager_interface *mgr)
{
    taqdata_t *data = NULL;
    struct taqdata_manager_attribute *this = (struct taqdata_manager_attribute *) mgr;

    if (this)
    {
        if (this->count != 0)
        {
            this->focus = this->focus->prev;
            data = this->focus->data;
        }
        else
            TLOGMSG(1, (DBGINFOFMT "no taqdata in taqdata manager\n", DBGINFO));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null taqdata manager interface\n", DBGINFO));

    return data;
}


static taqdata_t *
taqdata_manager_get_focus(struct taqdata_manager_interface *mgr)
{
    taqdata_t *data = NULL;
    struct taqdata_manager_attribute *this = (struct taqdata_manager_attribute *) mgr;

    if (this)
    {
        if (this->count != 0)
            data = this->focus->data;
        else
            TLOGMSG(1, (DBGINFOFMT "no taqdata in taqdata manager\n", DBGINFO));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null taqdata manager interface\n", DBGINFO));

    return data;
}


static taqdata_t *
taqdata_manager_set_focus(struct taqdata_manager_interface *mgr, int idx)
{
    taqdata_t *data = NULL;
    taqdata_container_t *container = NULL;
    struct taqdata_manager_attribute *this = (struct taqdata_manager_attribute *) mgr;

    if (this)
    {
        if (this->count != 0)
        {
            if (idx < this->count)
            {
                container = this->head;

                for (int i = 0; i < idx; i++)
                    container = container->next;

                this->focus = container;
                data = container->data;
            }
            else
                TLOGMSG(1, (DBGINFOFMT "invalid index\n", DBGINFO));
        }
        else
            TLOGMSG(1, (DBGINFOFMT "no taqdata in taqdata manager\n", DBGINFO));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null taqdata manager interface\n", DBGINFO));

    return data;
}


static int
taqdata_manager_get_count(struct taqdata_manager_interface *mgr)
{
    int count = 0;
    struct taqdata_manager_attribute *this = (struct taqdata_manager_attribute *) mgr;

    if (this)
        count = this->count;
    else
    {
        count = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqdata manager interface\n", DBGINFO));
    }

    return count;
}


static taqdata_t *
taqdata_manager_create_taqdata(struct taqdata_manager_interface *mgr)
{
    taqdata_t *data = NULL;
    struct taqdata_manager_attribute *this = (struct taqdata_manager_attribute *) mgr;

    if (this)
    {
        data = malloc(sizeof(taqdata_t));

        if (data)
        {
            memset(data, 0x00, sizeof(taqdata_t));
            data->observer.altitude  = 0.0;
            data->observer.latitude  = 0.0;
            data->observer.longitude = 0.0;
            data->observer.fwdaz     = 0.0;
            data->observer.fwdel     = 0.0;
            data->target.altitude    = 0.0;
            data->target.latitude    = 0.0;
            data->target.longitude   = 0.0;
        }
        else
            TLOGMSG(1, (DBGINFOFMT "failed create taqdata, malloc return null\n", DBGINFO));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null taqdata manager interface\n", DBGINFO));

    return data;
}


struct taqdata_manager_interface *
taqdata_manager_create(void)
{
    struct taqdata_manager_interface *mgr = NULL;
    struct taqdata_manager_attribute *this = malloc(sizeof(struct taqdata_manager_attribute));

    if (this)
    {
        this->count = 0;
        this->head  = NULL;
        this->tail  = NULL;
        this->focus = NULL;
        mgr = &(this->extif);
        mgr->saveData   = taqdata_manager_save_taqdata;
        mgr->loadData   = taqdata_manager_load_taqdata;
        mgr->getNumData = taqdata_manager_get_count;
        mgr->createData = taqdata_manager_create_taqdata;
        mgr->removeData = taqdata_manager_remove_taqdata;
        mgr->addData	= taqdata_manager_add_taqdata;
        mgr->getData    = taqdata_manager_get_taqdata;
        mgr->setFocus   = taqdata_manager_set_focus;
        mgr->getFocus	= taqdata_manager_get_focus;
        mgr->focusNext  = taqdata_manager_focus_next;
        mgr->focusPrev  = taqdata_manager_focus_prev;
    }
    else
        TLOGMSG(1, (DBGINFOFMT, "failed to create taqdata manager interface, malloc return null\n", DBGINFO));

    return mgr;
}


int
taqdata_manager_destroy(struct taqdata_manager_interface *mgr)
{
    int ret = 0;
    struct taqdata_manager_attribute *this = (struct taqdata_manager_attribute *) mgr;

    if (this)
    {
        while(this->count != 0)
            taqdata_manager_remove_taqdata(mgr);

        free(this);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqdata manager interface\n", DBGINFO));
    }

    return ret;
}
