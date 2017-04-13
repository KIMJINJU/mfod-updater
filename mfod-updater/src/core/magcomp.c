/*
	Copyright (C) 2015-2016 EOSYSTEM CO., LTD. (www.eosystem.com)

  	compmgr.c
  		functions for dmc magnetic compensation.
  		This file is part of amod-mainapp.

  	Written by
  		Seung-hwan, Park (seunghwan.park@me.com)
 */

#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "core/evque.h"
#include "core/logger.h"
#include "core/magcomp.h"
#include "etc/util.h"


#define MAGCOMPDATA_AZIMUTH_LIMIT	20.0
#define MAGCOMPDATA_ELEVATION_LIMIT	10.0
#define MAGCOMPDATA_BANK_LIMIT		10.0

#define ALIGN_OK					1
#define ALIGN_FAIL					0


/*  sturcture declarations : magcomp attribute */
struct magcomp_attribute
{
    /* external interface */
    struct magcomp_interface extif;

    /* internal interface */
    int geometry;
    int	mvdir;
    int	task;
    int	qcp;
    int	daq;
    int position[3];
    int	align[3];
    pthread_t tid;
    pthread_mutex_t mtx;
    pthread_cond_t  cond;

    struct dmc_interface *dmc;
};


/* sturcture declarations : positions for magnetic compensation data aqusition */
struct magcomp_geometry
{
    int azimuth;
    int elevation;
    int bank;
};


/* magnetic compensation geometry -  tripod */
static struct magcomp_geometry tripod[] =
{
    {.azimuth =   0, .elevation =  30, .bank = 0},
    {.azimuth =   0, .elevation = -30, .bank = 0},
    {.azimuth =  60, .elevation = -30, .bank = 0},
    {.azimuth =  60, .elevation =  30, .bank = 0},
    {.azimuth = 120, .elevation =  30, .bank = 0},
    {.azimuth = 120, .elevation = -30, .bank = 0},
    {.azimuth = 180, .elevation = -30, .bank = 0},
    {.azimuth = 180, .elevation =  30, .bank = 0},
    {.azimuth = 240, .elevation =  30, .bank = 0},
    {.azimuth = 240, .elevation = -30, .bank = 0},
    {.azimuth = 300, .elevation = -30, .bank = 0},
    {.azimuth = 300, .elevation =  30, .bank = 0},
};

/* magnetic compensation geometry -  elevation only */
static struct magcomp_geometry elo[] =
{
    {.azimuth =   0, .elevation =  30, .bank =   0},
    {.azimuth =   0, .elevation =   0, .bank =   0},
    {.azimuth =   0, .elevation = -30, .bank =   0},
    {.azimuth =  90, .elevation = -30, .bank =   0},
    {.azimuth =  90, .elevation =   0, .bank =   0},
    {.azimuth =  90, .elevation =  30, .bank =   0},
    {.azimuth = 180, .elevation =  30, .bank =   0},
    {.azimuth = 180, .elevation =   0, .bank =   0},
    {.azimuth = 180, .elevation = -30, .bank =   0},
    {.azimuth = 270, .elevation = -30, .bank =   0},
    {.azimuth = 270, .elevation =   0, .bank =   0},
    {.azimuth = 270, .elevation =  30, .bank =   0},
};


static int
magcomp_find_start_position(struct magcomp_interface *magcomp, double azimuth)
{
    int ret = 0;
    int npos = 0;
    struct magcomp_geometry *pos = NULL;
    struct magcomp_attribute *this = (struct magcomp_attribute *) magcomp;

    if (this->geometry == MAGCOMP_GEOMETRY_TRIPOD)
    {
        pos = &tripod[0];
        npos = DIM(tripod);
    }
    else
    {
        pos = &elo[0];
        npos = DIM(elo);
    }

    for (int i = 0; i < npos; i++)
    {
        if (azimuth < (pos + i)->azimuth)
        {
            ret = i;
            break;
        }
        else
            continue;
    }

    TLOGMSG(1, ("magcomp start position = { %5.5lf, %5.5lf, %5.5lf}\n",
            (pos + ret)->azimuth, (pos + ret)->elevation, (pos + ret)->bank));

    return ret;
}


static int
magcomp_find_move_direction(struct magcomp_interface *magcomp, double azimuth, double elevation, double bank)
{
    int dir = -1;
    struct magcomp_geometry *pos = NULL;
    struct magcomp_attribute *this = (struct magcomp_attribute *) magcomp;
    int idx = this->position[1];

    if (this->geometry == MAGCOMP_GEOMETRY_TRIPOD)
        pos = &tripod[0];
    else
        pos = &elo[0];

    if (this->align[0] != ALIGN_OK)
    {
        realign_azimuth:

        if ((pos + idx)->azimuth - azimuth > 10.0)
            dir = MAGCOMP_MOVE_DIR_CW;
        else if ((pos + idx)->azimuth - azimuth < -10.0)
        {
            if (((pos + idx)->azimuth == 0) && (azimuth > 180.0))
                dir = MAGCOMP_MOVE_DIR_CW;
            else if ((pos + idx)->azimuth - azimuth < -180.0)
                dir = MAGCOMP_MOVE_DIR_CW;
            else
                dir = MAGCOMP_MOVE_DIR_CCW;
        }
        else
            this->align[0] = ALIGN_OK;

        if (dir != -1)
            return dir;
    }
    else
    {
        if (fabs((pos + idx)->azimuth - azimuth) > 20.0)
        {
            this->align[0] = 0;
            goto realign_azimuth;
        }
    }

    if (this->align[1] != ALIGN_OK)
    {
        realign_elevation:

        if ((pos + idx)->elevation - elevation > 5.0)
            dir = MAGCOMP_MOVE_DIR_UP;
        else if ((pos + idx)->elevation - elevation < -5.0)
            dir = MAGCOMP_MOVE_DIR_DOWN;
        else
            this->align[1] = ALIGN_OK;

        if (dir != -1)
            return dir;
    }
    else
    {
        if (fabs((pos + idx)->elevation - elevation) > 10.0)
        {
            this->align[1] = 0;
            goto realign_elevation;
        }
    }

    if (this->align[2] != ALIGN_OK)
    {
        realign_bank:

        if ((pos + idx)->bank - bank > 5.0)
            dir = MAGCOMP_MOVE_DIR_TCCW;
        else if ((pos + idx)->bank - bank < -5.0)
            dir = MAGCOMP_MOVE_DIR_TCW;
        else
            this->align[2] = ALIGN_OK;

        if (dir != -1)
            return dir;
    }
    else
    {
        if (fabs((pos + idx)->bank - bank) > 10.0)
        {
            this->align[2] = 0;
            goto realign_bank;
        }
    }

    //if ((this->align[0] == ALIGN_OK) && (this->align[1] == ALIGN_OK) && (this->align[2] == ALIGN_OK))
    dir = MAGCOMP_MOVE_DIR_STOP;

    return dir;
}


static void *
magcomp_process_compensation(void *arg)
{
    int count = 0;
    int mil[3] = {0, 0, 0};
    double deg[3] = {0.0, 0.0, 0.0};

    struct magcomp_interface *magcomp = (struct magcomp_interface *) arg;
    struct magcomp_attribute *this = (struct magcomp_attribute *) magcomp;
    struct dmc_interface *dmc = this->dmc;

    while (this->task != MAGCOMP_TASK_EXIT)
    {
        if (this->task == MAGCOMP_TASK_ALIGN)
        {
            //pthread_mutex_lock(&compmgr->mtx);

            dmc->measure(dmc, DMC_SINGLE_MEASURE);
            dmc->getAzimuth(dmc, &mil[0], &deg[0]);
            dmc->getElevation(dmc, &mil[1], &deg[1]);
            dmc->getBank(dmc, &mil[2], &deg[2]);

            if (this->position[0] == -1)
            {
                this->position[0] = magcomp_find_start_position(magcomp, deg[0]);
                this->position[1] = this->position[0];
                this->position[2] = this->position[0] - 1;

                if (this->position[2] == -1)
                {
                    if (this->geometry == MAGCOMP_GEOMETRY_TRIPOD)
                        this->position[2] = DIM(tripod) - 1;
                    else
                        this->position[2] = DIM(elo) - 1;
                }
            }

            this->mvdir = magcomp_find_move_direction(magcomp, deg[0], deg[1], deg[2]);
            TLOGMSG(0, ("align direction = %d\n", this->mvdir));

            if (count == 2)
            {
                count = 0;
                this->align[0] = 0;
                this->align[1] = 0;
                this->align[2] = 0;
                this->task = MAGCOMP_TASK_QCP;
                evque_set_event(EVCODE_QUERY_MAGCOMPDATA, 0);
            }
            else
            {
                if (this->mvdir == MAGCOMP_MOVE_DIR_STOP)
                {
                    if (count == 0)
                        dmc->setItime(dmc, 0x10);

                    count++;
                }
                else
                {
                    if (count != 0)
                        dmc->setItime(dmc, 0x02);

                    count = 0;
                }
            }

            //pthread_mutex_unlock(&compmgr->mtx);
        }
        else
            MSLEEP(100);
    }

    return NULL;
}


static int
magcomp_start_compensation(struct magcomp_interface *magcomp)
{
    int ret = 0;
    struct dmc_interface *dmc = NULL;
    struct magcomp_attribute *this = (struct magcomp_attribute *) magcomp;

    if (this)
    {
        dmc = this->dmc;
        pthread_mutex_lock(&this->mtx);
        dmc->reset(dmc);
        dmc->configure(dmc, DMC_CONFIG_COMPMODE);
        dmc->enterCompMode(dmc);
        this->qcp   = MAGCOMP_QCP01;
        this->mvdir = MAGCOMP_MOVE_DIR_STOP;
        this->position[0] = -1;
        this->position[1] = -1;
        this->position[2] = -1;
        this->task = MAGCOMP_TASK_ALIGN;
        pthread_mutex_unlock(&this->mtx);

        TLOGMSG(1, ("start magnetic compensation sequence\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null magcomp interfacer\n", DBGINFO));
    }

    return ret;
}


static int
magcomp_terminate_compensation(struct magcomp_interface *magcomp)
{
    int ret = 0;
    struct dmc_interface *dmc = NULL;
    struct magcomp_attribute *this = (struct magcomp_attribute *) magcomp;

    if (this)
    {
        dmc = this->dmc;
        pthread_mutex_lock(&this->mtx);
        this->task = MAGCOMP_TASK_IDLE;
        this->position[0] = -1;
        this->position[1] = -1;
        this->position[2] = -1;
        this->mvdir = MAGCOMP_MOVE_DIR_STOP;
        this->qcp   = MAGCOMP_QCP01;
        pthread_mutex_unlock(&this->mtx);
        MSLEEP(1000);
        dmc->exitCompMode(dmc);
        TLOGMSG(1, ("user terminated magnetic compensation sequence\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null magcomp interface\n", DBGINFO));
    }

    return ret;
}


static int
magcomp_request_compdata(struct magcomp_interface *magcomp)
{
    int ret = 0;
    struct dmc_interface *dmc = NULL;
    struct magcomp_attribute *this = (struct magcomp_attribute *) magcomp;

    if (this)
    {
        dmc = this->dmc;
        pthread_mutex_lock(&this->mtx);

        if (dmc->queryCompData(dmc) == 0)
        {
            this->qcp = dmc->getNcp(dmc);
            TLOGMSG(1, ("acquire magnetic compensation data #%d\n", this->qcp));
        }
        else
        {
            ret = -1;
            TLOGMSG(1, ("failed to acquire compensation data\n"));
        }

        if(this->qcp == MAGCOMP_QCP12)
        {
            this->position[0] = -1;
            this->position[1] = -1;
            this->position[2] = -1;
            this->task = MAGCOMP_TASK_ENDQCP;
        }
        else
        {
            this->position[1] = this->position[1] + 1;

            if (this->geometry == MAGCOMP_GEOMETRY_ELO)
            {
                if (this->position[1] == DIM(tripod))
                    this->position[1] = 0;
            }
            else
            {
                if (this->position[1] == DIM(elo))
                    this->position[1] = 0;
            }

            dmc->setItime(dmc, 0x02);
            this->qcp = this->qcp + 1;
            this->task = MAGCOMP_TASK_ALIGN;
        }

        pthread_mutex_unlock(&this->mtx);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null magcomp interface\n", DBGINFO));
    }

    return ret;
}


static int
magcomp_get_current_task(struct magcomp_interface *magcomp)
{
    int ret = 0;
    struct magcomp_attribute *this = (struct magcomp_attribute *) magcomp;

    struct task_string
    {
        int	 task;
        char *str;
    }
    task_string[] =
    {
        {MAGCOMP_TASK_IDLE	, "COMPMGR_TASK_IDLE"  },
        {MAGCOMP_TASK_ALIGN , "COMPMGR_TASK_ALIGN" },
        {MAGCOMP_TASK_QCP   , "COMPMGR_TASK_QCPDV" },
        {MAGCOMP_TASK_ENDQCP, "COMPMGR_TASK_ENDQCP"},
        {MAGCOMP_TASK_EXIT	, "COMPMGR_TASK_EXIT"  }
    };

    if (this)
    {
        ret = this->task;
        TLOGMSG(1, ("current compensation task = %s\n", task_string[ret].str));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null magcomp interface\n", DBGINFO));
    }

    return ret;
}


static int
magcomp_get_current_qcp(struct magcomp_interface *magcomp)
{
    int ret = 0;
    struct magcomp_attribute *this = (struct magcomp_attribute *) magcomp;

    if (this)
    {
        ret = this->qcp;
        TLOGMSG(0, ("currnet QCP = %d\n", ret));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null magcomp interface\n", DBGINFO));
    }

    return ret;
}


static int
magcomp_get_daqpos(struct magcomp_interface *magcomp, int *az, int *el, int *bk)
{
    int ret = 0;
    struct magcomp_attribute *this = (struct magcomp_attribute *) magcomp;

    if (this)
    {
        if (this->geometry == MAGCOMP_GEOMETRY_ELO)
        {
            if (this->position[1] == -1)
            {
                *az = 0;
                *az = 0;
                *bk = 0;
            }
            else
            {
                *az = elo[this->position[1]].azimuth;
                *el = elo[this->position[1]].elevation;
                *bk = elo[this->position[1]].bank;
            }
        }
        else if (this->geometry == MAGCOMP_GEOMETRY_TRIPOD)
        {
            if (this->position[1] == -1)
            {
                *az = 0;
                *az = 0;
                *bk = 0;
            }
            else
            {
                *az = tripod[this->position[1]].azimuth;
                *el = tripod[this->position[1]].elevation;
                *bk = tripod[this->position[1]].bank;
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid compensation geometry type\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null magcomp interface\n", DBGINFO));
    }

    return ret;
}


static int
magcomp_get_mvdir(struct magcomp_interface *magcomp)
{
    int ret = 0;
    struct magcomp_attribute *this = (struct magcomp_attribute *) magcomp;

    if (this)
        ret = this->mvdir;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null magcomp interface\n", DBGINFO));
    }

    return ret;
}


static int
magcomp_set_geometry(struct magcomp_interface *magcomp, int geometry)
{
    int ret = 0;
    struct magcomp_attribute *this = (struct magcomp_attribute *) magcomp;

    if (this)
    {
        if ((geometry == MAGCOMP_GEOMETRY_TRIPOD) || (geometry == MAGCOMP_GEOMETRY_ELO))
        {
            this->geometry = geometry;
            TLOGMSG(1, ("set magnetic compensation geometry  %s\n",
                    geometry == MAGCOMP_GEOMETRY_ELO ? "elevation only" : "tripod"));
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid geometry\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null magcomp interface\n",  DBGINFO));
    }

    return ret;
}



static int
magcomp_get_geometry(struct magcomp_interface *magcomp)
{
    int ret = 0;
    struct magcomp_attribute *this = (struct magcomp_attribute *) magcomp;

    if (this)
        ret = this->geometry;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null magcomp interface\n",  DBGINFO));
    }

    return ret;
}


struct magcomp_interface *
magcomp_create(struct dmc_interface *dmc)
{
    struct magcomp_interface *magcomp = NULL;
    struct magcomp_attribute *this = NULL;

    if (dmc)
    {
        this = malloc(sizeof(struct magcomp_attribute));

        if (this)
        {
            memset(this, 0x00, sizeof(struct magcomp_attribute));
            pthread_mutex_init(&this->mtx, NULL);
            pthread_cond_init(&this->cond, NULL);
            this->dmc = dmc;
            this->qcp = MAGCOMP_QCP01;
            magcomp = &(this->extif);

            if (pthread_create(&this->tid, NULL, magcomp_process_compensation, (void *)magcomp) == 0)
            {
                magcomp->setGeometry  = magcomp_set_geometry;
                magcomp->getGeometry  = magcomp_get_geometry;
                magcomp->getDirection = magcomp_get_mvdir;
                magcomp->getPosition  = magcomp_get_daqpos;
                magcomp->getCurrentQcp   = magcomp_get_current_qcp;
                magcomp->getCurrentTask  = magcomp_get_current_task;
                magcomp->acquireCompdata = magcomp_request_compdata;
                magcomp->startCompensation     = magcomp_start_compensation;
                magcomp->terminateCompensation = magcomp_terminate_compensation;
                TLOGMSG(1, ("create magnetic compensator interface\n"));
            }
            else
            {
                pthread_mutex_destroy(&this->mtx);
                pthread_cond_destroy(&this->cond);
                magcomp = NULL;
                free(this);
                TLOGMSG(1, (DBGINFOFMT "pthread_create return fail\n", DBGINFO));
            }
        }
        else
            TLOGMSG(1, (DBGINFOFMT "malloc return null, %s\n", DBGINFO));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));

    return magcomp;
}


int
magcomp_destroy(struct magcomp_interface *magcomp)
{
    int ret = 0;
    struct magcomp_attribute *this = (struct magcomp_attribute *) magcomp;

    if (this)
    {
        this->task = MAGCOMP_TASK_EXIT;
        pthread_join(this->tid, NULL);
        pthread_mutex_destroy(&this->mtx);
        pthread_cond_destroy(&this->cond);
        free(this);
        TLOGMSG(1, ("destroy magnetic compensator interface\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null magcomp interface\n", DBGINFO));
    }

    return ret;
}
