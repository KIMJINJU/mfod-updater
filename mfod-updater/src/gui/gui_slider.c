/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_slider.c
        external/internal function implementations of slider widget interface
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "core/logger.h"
#include "gui/gui_slider.h"

/* structure declarations */
struct gui_slider_attribute
{
	/* external attribute */
	struct gui_slider_interface extif;

	/* internal attribute */
	char *title;
	int maxValue;
	int minValue;
	int currentValue;
	int (*increaseCallback) (void *arg);
	int (*decreaseCallback) (void *arg);
	rect_t position;
	pthread_mutex_t mtx;
};


static int
gui_slider_set_title(struct gui_slider_interface *slider, char *title)
{
	int ret = 0;
	struct gui_slider_attribute *this = (struct gui_slider_attribute *)slider;

	if (this)
	{
		pthread_mutex_lock(&this->mtx);

		if (title)
		{
			if (this->title)
				free(this->title);

			ret = strlen(title);
			this->title = malloc(ret + 1);

			if (this->title)
			{
				memset(this->title, 0x00, ret + 1);
				memcpy(this->title, title, ret);
			}
			else
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));
			}
		}
		else
		{
			if (this->title)
				free(this->title);

			this->title = NULL;
		}

		pthread_mutex_unlock(&this->mtx);
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null slider widget interface\n", DBGINFO));
	}

	return ret;
}


static char *
gui_slider_get_title(struct gui_slider_interface *slider)
{
	char *title = NULL;
	struct gui_slider_attribute *this = (struct gui_slider_attribute *)slider;

	if (this)
		title = this->title;
	else
		TLOGMSG(1, (DBGINFOFMT "null slider widget interface\n", DBGINFO));

	return title;
}


static int
gui_slider_set_maxval(struct gui_slider_interface *slider, int value)
{
	int ret = 0;
	struct gui_slider_attribute *this = (struct gui_slider_attribute *)slider;

	if (this)
	{
		if (value <= this->minValue)
		{
			this->maxValue = value;
			this->minValue = value;
			this->currentValue = value;
		}
		else
		{
			if (value < this->currentValue)
				this->currentValue = value;

			this->maxValue = value;
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null slider widget interface\n", DBGINFO));
	}

	return ret;
}


static int
gui_slider_get_maxval(struct gui_slider_interface *slider, int *value)
{
	int ret = 0;
	struct gui_slider_attribute *this = (struct gui_slider_attribute *)slider;

	if (this)
		*value = this->maxValue;
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null slider widget interface\n", DBGINFO));
	}

	return ret;
}


static int
gui_slider_set_minval(struct gui_slider_interface *slider, int value)
{
	int ret = 0;
	struct gui_slider_attribute *this = (struct gui_slider_attribute *)slider;

	if (this)
	{
		if (value >= this->maxValue)
		{
			this->maxValue = value;
			this->minValue = value;
			this->currentValue = value;
		}
		else
		{
			if (value > this->currentValue)
				this->currentValue = value;

			this->minValue = value;
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null slider widget interface\n", DBGINFO));
	}

	return ret;
}


static int
gui_slider_get_minval(struct gui_slider_interface *slider, int *value)
{
	int ret = 0;
	struct gui_slider_attribute *this = (struct gui_slider_attribute *)slider;

	if (this)
		*value = this->minValue;
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null slider widget interface\n", DBGINFO));
	}

	return ret;
}


static int
gui_slider_set_currval(struct gui_slider_interface *slider, int value)
{
	int ret = 0;
	struct gui_slider_attribute *this = (struct gui_slider_attribute *)slider;

	if (this)
	{
		if (value < this->minValue)
			this->currentValue = this->minValue;
		else if (value > this->maxValue)
			this->currentValue = this->maxValue;
		else
		 	this->currentValue = value;
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null slider widget interface\n", DBGINFO));
	}

	return ret;
}


static int
gui_slider_get_currval(struct gui_slider_interface *slider, int *value)
{
	int ret = 0;
	struct gui_slider_attribute *this = (struct gui_slider_attribute *)slider;

	if (this)
		*value = this->currentValue;
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null slider widget interface\n", DBGINFO));
	}

	return ret;
}


static int
gui_slider_set_position(struct gui_slider_interface *slider, rect_t *pos)
{
	int ret = 0;
	struct gui_slider_attribute *this = (struct gui_slider_attribute *)slider;

	if (this)
		memcpy(&this->position, pos, sizeof(rect_t));
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null slider widget interface\n", DBGINFO));
	}

	return ret;
}


static int
gui_slider_get_position(struct gui_slider_interface *slider, rect_t *pos)
{
	int ret = 0;
	struct gui_slider_attribute *this = (struct gui_slider_attribute *)slider;

	if (this)
		memcpy(pos, &this->position, sizeof(rect_t));
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null slider widget interface\n", DBGINFO));
	}

	return ret;
}


static int
gui_slider_set_callback(struct gui_slider_interface *slider, int cbid, int (*callback)(void *))
{
	int ret = 0;
	struct gui_slider_attribute *this = (struct gui_slider_attribute *)slider;

	if (this)
	{
		switch (cbid)
		{
		case SLIDER_CALLBACK_INCREASE_VALUE:
			this->increaseCallback = callback;
			break;

		case SLIDER_CALLBACK_DECREASE_VALUE:
			this->decreaseCallback = callback;
			break;

		default:
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "invalid callback function\n", DBGINFO));
			break;
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null slider widget interface\n", DBGINFO));
	}

	return ret;
}


static int
gui_slider_execute_callback(struct gui_slider_interface *slider, int cbid, void *arg)
{
	int ret = 0;
	struct gui_slider_attribute *this = (struct gui_slider_attribute *)slider;

	if (this)
	{
		switch (cbid)
		{
		case SLIDER_CALLBACK_INCREASE_VALUE:
			if (this->increaseCallback != NULL)
			{
				if (this->currentValue < this->maxValue)
				{
					ret = this->increaseCallback(arg);

					if (ret == 0)
						this->currentValue++;
					else
						TLOGMSG(1, (DBGINFOFMT "increase callback return fail\n", DBGINFO));
				}
				else
					TLOGMSG(0, ("currval(%d) reached maxval\n", this->currentValue));
			}
			else
				TLOGMSG(1, (DBGINFOFMT "null callback function\n", DBGINFO));

			break;

		case SLIDER_CALLBACK_DECREASE_VALUE:
			if (this->decreaseCallback != NULL)
			{
				if (this->currentValue > this->minValue)
				{
					ret = this->decreaseCallback(arg);

					if (ret == 0)
						this->currentValue--;
					else
						TLOGMSG(1, (DBGINFOFMT "decrease callback return fail\n", DBGINFO));
				}
				else
					TLOGMSG(0, ("currval reached minval\n"));
			}
			else
				TLOGMSG(1, (DBGINFOFMT "null callback function\n", DBGINFO));

			break;

		default:
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "invalid callback function\n", DBGINFO));
			break;
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null slider widget interface\n", DBGINFO));
	}

	return ret;
}


struct gui_slider_interface *
gui_slider_create(void)
{
	struct gui_slider_interface *slider = NULL;
	struct gui_slider_attribute *this = malloc(sizeof(struct gui_slider_attribute));

    if (this)
    {
		memset(this, 0x00, sizeof(struct gui_slider_attribute));
		pthread_mutex_init(&this->mtx, NULL);
		slider = &(this->extif);
		slider->setTitle        = gui_slider_set_title;
		slider->getTitle        = gui_slider_get_title;
		slider->setMaxValue	    = gui_slider_set_maxval;
		slider->getMaxValue     = gui_slider_get_maxval;
		slider->setMinValue     = gui_slider_set_minval;
		slider->getMinValue     = gui_slider_get_minval;
		slider->setCurrentValue = gui_slider_set_currval;
		slider->getCurrentValue	= gui_slider_get_currval;
		slider->setPosition 	= gui_slider_set_position;
		slider->getPosition 	= gui_slider_get_position;
		slider->setCallback 	= gui_slider_set_callback;
		slider->execCallback 	= gui_slider_execute_callback;
		TLOGMSG(0, ("create slider widget interface\n"));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));

    return slider;
}


int
gui_slider_destroy(struct gui_slider_interface *slider)
{
	int ret = 0;
	struct gui_slider_attribute *this = (struct gui_slider_attribute *)slider;

    if (this)
    {
        pthread_mutex_destroy(&this->mtx);
        free(this->title);
        free(slider);
        TLOGMSG(0, ("destroy slider widget interface\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null slider widget interface\n", DBGINFO));
    }

    return ret;
}
