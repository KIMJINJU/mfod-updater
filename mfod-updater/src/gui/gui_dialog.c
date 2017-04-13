/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_dialog.c
        external/internal function implementations of dialog widget interface
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/


#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/logger.h"
#include "gui/gui_dialog.h"


/* structure declaration : dialog widget attribute */
struct gui_dialog_attribute
{
    /* external interface */
    struct gui_dialog_interface extif;

    /* internal interface */
    int id;
    char *title;
    rect_t position;
    list_t *widgetList;
    list_node_t *focus;
    pthread_mutex_t mutex;
};


static int
gui_dialog_get_id(struct gui_dialog_interface *dlg)
{
    int ret = 0;
    struct gui_dialog_attribute *this = (struct gui_dialog_attribute *) dlg;

    if (this)
        ret = this->id;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dialog interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_dialog_set_title(struct gui_dialog_interface *dlg, char *title)
{
    int ret = 0;
    int length = 0;
    struct gui_dialog_attribute *this = (struct gui_dialog_attribute *) dlg;

    if (this)
    {
        pthread_mutex_lock(&this->mutex);

        if (this->title)
            free(this->title);

        if (title)
        {
            length = strlen(title);
            this->title = malloc(sizeof(char) * (length + 1));

            if (this->title != NULL)
            {
                memset(this->title, 0x00, length + 1);
                memcpy(this->title, title, length);
            }
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "malloc return null. set button title to null\n", DBGINFO));
            }
        }
        else
            this->title = NULL;

        pthread_mutex_unlock(&this->mutex);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dialog interface\n", DBGINFO));
    }

    return ret;
}


static char *
gui_dialog_get_title(struct gui_dialog_interface *dlg)
{
    char *title = NULL;
    struct gui_dialog_attribute *this = (struct gui_dialog_attribute *) dlg;

    if (this)
        title = this->title;
    else
        TLOGMSG(1, (DBGINFOFMT "null dialog interface\n", DBGINFO));

    return title;
}


static int
gui_dialog_set_position(struct gui_dialog_interface *dlg, rect_t *pos)
{
    int ret = 0;
    struct gui_dialog_attribute *this = (struct gui_dialog_attribute *) dlg;

    if (this)
        memcpy(&this->position, pos, sizeof(rect_t));
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dialog interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_dialog_get_position(struct gui_dialog_interface *dlg, rect_t *pos)
{
    int ret = 0;
    struct gui_dialog_attribute *this = (struct gui_dialog_attribute *) dlg;

    if (this)
        memcpy(pos, &(this->position), sizeof(rect_t));
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dialog interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_dialog_add_widget(struct gui_dialog_interface *dlg, int widget_id, int widget_type, void *widget_data)
{
    int ret = 0;
    struct gui_dialog_widget *widget = NULL;
    struct gui_dialog_attribute *this = (struct gui_dialog_attribute *) dlg;

    if (dlg)
    {
        widget = malloc(sizeof(struct gui_dialog_widget));

        if (widget != NULL)
        {
            widget->id   = widget_id;
            widget->type = widget_type;
            widget->data = widget_data;

            if (list_insert_node(this->widgetList, (void *) widget) == 0)
                this->focus = this->widgetList->head;
            else
            {
                ret = -1;
                free(widget);
                TLOGMSG(1, (DBGINFOFMT "failed to add widget to widget list\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to create widget\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dialog interface\n", DBGINFO));
    }

    return ret;
}

static int
gui_dialog_remove_widgets(struct gui_dialog_interface *dlg)
{
    int ret = 0;
    list_node_t *node = NULL;
    struct gui_dialog_widget *widget = NULL;
    struct gui_dialog_attribute *this = (struct gui_dialog_attribute *) dlg;

    if (this)
    {
        for (int i = 0; i < this->widgetList->count; i++)
        {
            node = list_get_node(this->widgetList, i);

            if (node)
            {
                widget = (struct gui_dialog_widget *) node->data;

                if (widget)
                {
                    switch (widget->type)
                    {
                    case WIDGET_BUTTON:
                        gui_button_destroy((button_t *) widget->data);
                        break;

                    case WIDGET_CAPTION:
                        gui_caption_destroy((caption_t *) widget->data);
                        break;

                    case WIDGET_GRID:
                        break;

                    case WIDGET_QMENU:
                        break;

                    case WIDGET_QSEL:
                        gui_qsel_destroy((qsel_t *) widget->data);
                        break;

                    case WIDGET_SLIDER:
                        gui_slider_destroy((slider_t *) widget->data);
                        break;

                    case WIDGET_SPINBOX:
                        gui_spinbox_destroy((spinbox_t *) widget->data);
                        break;

                    case WIDGET_SVPLOT:
                        gui_svplot_destroy((svplot_t *) widget->data);
                        break;
                    }
                }
                else
                    continue;
            }
            else
                continue;
        }

        list_destroy(this->widgetList);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dialog interface\n", DBGINFO));
    }

    return ret;
}


static struct gui_dialog_widget *
gui_dialog_find_widget(struct gui_dialog_interface *dlg, int widget_id)
{
    list_node_t *node = NULL;
    struct gui_dialog_widget *widget = NULL;
    struct gui_dialog_attribute *this = (struct gui_dialog_attribute *) dlg;

    if (this)
    {
        for (int i = 0; this->widgetList->count; i++)
        {
            node = list_get_node(this->widgetList, i);

            if (node)
            {
                widget = (struct gui_dialog_widget *) node->data;

                if (widget)
                {
                    if (widget->id == widget_id)
                        break;
                    else
                        widget = NULL;
                }
                else
                    continue;
            }
            else
                continue;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null dialog interface\n", DBGINFO));

    return widget;
}


static struct gui_dialog_widget *
gui_dialog_set_focus(struct gui_dialog_interface *dlg, int widget_id)
{
    list_node_t *node = NULL;
    struct gui_dialog_widget *widget = NULL;
    struct gui_dialog_attribute *this = (struct gui_dialog_attribute *) dlg;

    if (this)
    {
        for (int i = 0; i < this->widgetList->count; i++)
        {
            node = list_get_node(this->widgetList, i);

            if (node)
            {
                widget = (struct gui_dialog_widget *) node->data;

                if (widget)
                {
                    if (widget->id == widget_id)
                    {
                        this->focus = node;
                        break;
                    }
                    else
                        widget = NULL;
                }
                else
                    continue;
            }
            else
                continue;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null dialog interface\n", DBGINFO));

    return widget;
}


static struct gui_dialog_widget *
gui_dialog_get_focus(struct gui_dialog_interface *dlg)
{
    struct gui_dialog_widget *widget = NULL;
    struct gui_dialog_attribute *this = (struct gui_dialog_attribute *) dlg;

    if (this)
    {
        if (this->focus)
            widget = (struct gui_dialog_widget *) this->focus->data;
        else
            TLOGMSG(1, (DBGINFOFMT "null focus\n", DBGINFO));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null dialog interface\n", DBGINFO));

    return widget;
}


static struct gui_dialog_widget *
gui_dialog_focus_next_widget(struct gui_dialog_interface *dlg)
{
    struct gui_dialog_widget *widget = NULL;
    struct gui_dialog_attribute *this = (struct gui_dialog_attribute *) dlg;

    if (this)
    {
        do
        {
            this->focus = this->focus->next;
            widget = (struct gui_dialog_widget *) this->focus->data;
        }
        while((widget->type == WIDGET_CAPTION) || (widget->type == WIDGET_GRID) || (widget->type == WIDGET_SVPLOT));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null dialog interface\n", DBGINFO));

    return widget;
}


static struct gui_dialog_widget *
gui_dialog_focus_prev_widget(struct gui_dialog_interface *dlg)
{
    struct gui_dialog_widget *widget = NULL;
    struct gui_dialog_attribute *this = (struct gui_dialog_attribute *) dlg;

    if (this)
    {
        do
        {
            this->focus = this->focus->prev;
            widget = (struct gui_dialog_widget *) this->focus->data;
        }
        while((widget->type == WIDGET_CAPTION) || (widget->type == WIDGET_GRID) || (widget->type == WIDGET_SVPLOT));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null dialog interface\n", DBGINFO));

    return widget;
}


static list_t *
gui_dialog_get_widget_list(struct gui_dialog_interface *dlg)
{
    list_t *list = NULL;
    struct gui_dialog_attribute *this = (struct gui_dialog_attribute *) dlg;

    if (this)
        list = this->widgetList;
    else
        TLOGMSG(1, (DBGINFOFMT "null dialog interface\n", DBGINFO));

    return list;
}


struct gui_dialog_interface *
gui_dialog_create(int id)
{
    struct gui_dialog_interface *dlg = NULL;
    struct gui_dialog_attribute *this = malloc(sizeof(struct gui_dialog_attribute));

    if (this)
    {
        memset(this, 0x00, sizeof(struct gui_dialog_attribute));
        this->widgetList = list_create();

        if (this->widgetList)
        {
            this->id = id;
            pthread_mutex_init(&this->mutex, NULL);

            dlg = &(this->extif);
            dlg->getID         = gui_dialog_get_id;
            dlg->addWidget     = gui_dialog_add_widget;
            dlg->setTitle      = gui_dialog_set_title;
            dlg->getTitle      = gui_dialog_get_title;
            dlg->setPosition   = gui_dialog_set_position;
            dlg->getPosition   = gui_dialog_get_position;
            dlg->setFocus      = gui_dialog_set_focus;
            dlg->getFocus      = gui_dialog_get_focus;
            dlg->focusNext     = gui_dialog_focus_next_widget;
            dlg->focusPrev     = gui_dialog_focus_prev_widget;
            dlg->findWidget    = gui_dialog_find_widget;
            dlg->getWidgetList = gui_dialog_get_widget_list;
            TLOGMSG(0, ("create dialog interface (0x%X)\n", id));
        }
        else
        {
            free(this);
            TLOGMSG(1, (DBGINFOFMT "failed to init dialog item list\n", DBGINFO));
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));

    return dlg;
}


int
gui_dialog_destroy(struct gui_dialog_interface *dlg)
{
    int ret = 0;
    struct gui_dialog_attribute *this = (struct gui_dialog_attribute *) dlg;

    if (this)
    {
        gui_dialog_remove_widgets(dlg);
        pthread_mutex_destroy(&this->mutex);
        free(this->title);
        free(this);
        TLOGMSG(0, ("destroy dialog interface\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null dialog interface\n", DBGINFO));
    }

    return ret;
}
