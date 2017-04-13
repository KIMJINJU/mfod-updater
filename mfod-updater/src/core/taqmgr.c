/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    taqmgr.c
        external/internal function implementations of target acquistion manager interface
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "core/geodesic.h"
#include "core/logger.h"
#include "core/taqmgr.h"
#include "coordtr/coordtr.h"

#include "etc/util.h"
#include "etc/devconf.h"

/* constant macro defines */
#define NUM_COORDSYS            3
#define MAXLEN_COORDSTR         128
#define NUM_AZOFF               7
#define NUM_ACCUMULATE_ANGLES   7
#define GEODATA_DEFAULT         0
#define GEODATA_USER            1
#define MIN_USER_RANGE          70
#define SHIFT_MAX               4095
#define SHIFT_MIN               -4095
#define TARGET_RADIUS_MAX       16383
#define TARGET_SIZE_MAX         16383



/* structure declarations : geodesic data for target acquition */
struct geodesic_data
{
    int range[2];
    geod_coord_t coord;
};

/* structure declarations : target manger attributes */
struct taqmgr_attribute
{
    /* external interface */
    struct taqmgr_interface extif;

    /* internal interface */
    bool run;
    bool online;

    int fixLoc;
    int error;
    int status;
    int mrst;
    int userData;
    int taqMode;
    int dopLevel;
    int coordSystem;
    int azMode;
    int azOffsetIndex;

    double azOffset[NUM_AZOFF];
    double elOffset;
    double gridVar;
    double magDecl;
    double gridCvg;

    char coordString[2][NUM_COORDSYS][MAXLEN_COORDSTR];

    pthread_t tid;
    pthread_mutex_t mtx;

    struct geodesic_data geodata[2];
    struct dmc_interface  *dmc;
    struct gnss_interface *gnss;
    struct lrf_interface  *lrf;
    struct taqdata_manager_interface *targetList;
    struct taqdata_manager_interface *taqBuffer;
};


static int
taqmgr_load_factory_mode_parameter(struct taqmgr_interface *taqmgr)
{
	int ret = 0;
	struct taqmgr_attribute *this = (struct taqmgr_attribute *)taqmgr;

	ret = devconf_load_parameters(DEVCONF_FILE_PATH);
	if(ret == 0)
		devconf_enumerate_parameters();
	else
		ret = -1;

	return  ret;
}


static int
taqmgr_load_range_comensation_parameter(struct taqmgr_interface *taqmgr)
{
    int ret = 0;
    FILE *file = NULL;
    double rcc[4] = {0.0, 0.0, 0.0, 0.0};
    char *parm = "/mnt/mmc/amod-data/param/rcomp.parm";
    struct taqmgr_attribute *this = (struct taqmgr_attribute *)taqmgr;

    if (this)
    {
        file = fopen(parm, "r");

        if (file)
        {
            fscanf(file, "%lf %lf %lf %lf", &rcc[0], &rcc[1], &rcc[2], &rcc[3]);
            this->lrf->setRcc(this->lrf, &rcc[0], sizeof(rcc) / sizeof(double));
            fclose(file);
        }
        else
        {
            file = fopen(parm, "w");

            if (file)
            {
                fprintf(file, "%lf %lf %lf %lf", rcc[0], rcc[1], rcc[2], rcc[3]);
                this->lrf->setRcc(this->lrf, &rcc[0], sizeof(rcc) / sizeof(double));
                fclose(file);
                TLOGMSG(1, ("load default range compensation coefficient\n"));
            }
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "failed to open file, %s\n", DBGINFO, strerror(errno)));
            }
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_load_angular_compensation_parameter(struct taqmgr_interface *taqmgr)
{
    int ret = 0;
    FILE *file = NULL;
    char *parm = "/mnt/mmc/amod-data/param/angcomp.parm";
    struct taqmgr_attribute *this = (struct taqmgr_attribute *)taqmgr;

    if (this)
    {
        file = fopen(parm, "r");

        if (file)
        {
            fscanf(file, "%lf %lf", &this->azOffset[0], &this->elOffset);
            fclose(file);
            TLOGMSG(1, ("azimuth offset = %f, elevation offset =%f\n", this->azOffset[0], this->elOffset));
        }
        else
        {
            file = fopen(parm, "w");

            if (file)
            {
                fprintf(file, "%lf %lf", this->azOffset[0], this->elOffset);
                fclose(file);
                TLOGMSG(1, ("load default angular compensation coefficient\n"));
            }
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "failed to open file, %s\n", DBGINFO, strerror(errno)));
            }
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}




static void
taqmgr_update_doplv(struct taqmgr_interface *taqmgr, double pdop)
{
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    if (pdop < 2.0)
        this->dopLevel = DOPLV_EXCELLENT;
    else if (pdop < 5.0)
        this->dopLevel = DOPLV_GOOD;
    else if (pdop < 10.0)
        this->dopLevel = DOPLV_MODERATE;
    else if (pdop < 20.0)
        this->dopLevel = DOPLV_FAIR;
    else
        this->dopLevel = DOPLV_POOR;
}


static void
taqmgr_update_latitude(struct taqmgr_interface *taqmgr, int deg, int min, double sec, char ns)
{
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    if (ns == 'N')
        this->geodata[GEODATA_DEFAULT].coord.latitude = dms2deg(deg, min, sec);
    else
        this->geodata[GEODATA_DEFAULT].coord.latitude = dms2deg(-deg, -min, -sec);
}


static void
taqmgr_update_longitude(struct taqmgr_interface *taqmgr, int deg, int min, double sec, char ew)
{
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    if (ew == 'E')
        this->geodata[GEODATA_DEFAULT].coord.longitude = dms2deg(deg, min, sec);
    else
        this->geodata[GEODATA_DEFAULT].coord.longitude = dms2deg(-deg, -min, -sec);
}


static void
taqmgr_update_observer_location(struct taqmgr_interface *taqmgr)
{
    int latdeg = 0;
    int lngdeg = 0;
    int latmin = 0;
    int lngmin = 0;
    int utmlen = 0;
    int mgrslen = 0;
    char ns = 0;
    char ew = 0;
    double latsec = 0.0;
    double lngsec = 0.0;
    double pdop = 0.0;
    double hdop = 0.0;
    double vdop = 0.0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;
    char *utm  = &(this->coordString[GEODATA_DEFAULT][COORDSYS_UTM][0]);
    char *mgrs = &(this->coordString[GEODATA_DEFAULT][COORDSYS_MGRS][0]);
    char *geodetic = &(this->coordString[GEODATA_DEFAULT][COORDSYS_GEODETIC][0]);
    struct geodesic_data *geodata = &(this->geodata[GEODATA_DEFAULT]);

    this->gnss->getLatitude(this->gnss, &latdeg, &latmin, &latsec, &ns);
    this->gnss->getLongitude(this->gnss, &lngdeg, &lngmin, &lngsec, &ew);
    this->gnss->getAltitude(this->gnss, &(geodata->coord.altitude));
    this->gnss->getDOP(this->gnss, &pdop, &hdop, &vdop);
    taqmgr_update_latitude(taqmgr, latdeg, latmin, latsec, ns);
    taqmgr_update_longitude(taqmgr, lngdeg, lngmin, lngsec, ew);
    taqmgr_update_doplv(taqmgr, pdop);

    coordtr_geodetic2mgrs(geodata->coord.latitude, geodata->coord.longitude, mgrs, utm);
    utmlen = strlen(utm);
    mgrslen = strlen(mgrs);
    snprintf((utm + utmlen) ,(size_t)(MAXLEN_COORDSTR - utmlen), " %+dm (%.2f)", (int)geodata->coord.altitude, pdop);
    snprintf((mgrs + mgrslen), (size_t)(MAXLEN_COORDSTR - mgrslen), " %+dm (%.2f)", (int)geodata->coord.altitude, pdop);
    snprintf(geodetic, (size_t)MAXLEN_COORDSTR, "%02d\u00B0%02d\'%02.2f\"%c %03d\u00B0%02d\'%02.2f\"%c %+dm (%.2f)",
             latdeg, latmin, latsec, ns, lngdeg, lngmin, lngsec, ew, (int)geodata->coord.altitude, pdop);
}


static void
taqmgr_update_time(struct taqmgr_interface *taqmgr)
{
    time_t utc = 0;
    struct timespec ts = {.tv_sec = 0, .tv_nsec = 0};
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    this->gnss->getUTC(this->gnss, &utc);
    utc = utc + get_utc_offset() * 3600;

    if (abs(utc - (time(NULL) + get_time_offset())) > 1)
        set_time_offset(utc - time(NULL));
}


static void *
taqmgr_update_status(void *arg)
{
    double latitude = 0.0;
    double longitude = 0.0;
    struct taqmgr_interface *taqmgr = (struct taqmgr_interface *) arg;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;
    //struct geodetic_coordinate *coord = &(this->geodata[GEODATA_DEFAULT].coord);

    while (this->run)
    {
        pthread_mutex_lock(&this->mtx);

        if (this->online)
        {
            if ((this->gnss->valid(this->gnss) == 0) || (this->gnss->fix(this->gnss) == GNSS_FIX_NONE))
                this->online = false;
            else
            {
                taqmgr_update_time(taqmgr);
                taqmgr_update_observer_location(taqmgr);
            }
        }
        else
        {
            if ((this->gnss->valid(this->gnss) == 1) && (this->gnss->fix(this->gnss) != GNSS_FIX_NONE))
            {
                taqmgr_update_observer_location(taqmgr);

                if ((this->userData & USE_USER_INPUT_GRIDVAR))
                {
                    if (!(this->userData & USE_USER_INPUT_OBLOC))
                    {
                        latitude = this->geodata[GEODATA_DEFAULT].coord.latitude;
                        longitude = this->geodata[GEODATA_DEFAULT].coord.longitude;
                        this->gridCvg = geodesic_get_gridcvg(latitude, longitude);
                        this->magDecl = this->gridCvg + this->gridVar;
                    }
                }

                this->online = true;
            }
        }

        pthread_mutex_unlock(&this->mtx);
        MSLEEP(3000);
    }

    return NULL;
}


static int
taqmgr_set_coordsys(struct taqmgr_interface *taqmgr, int coordsys)
{
    int ret = 0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    if (this)
        this->coordSystem = coordsys;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_get_coordsys(struct taqmgr_interface *taqmgr)
{
    int coordsys = 0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    if (this)
        coordsys = this->coordSystem;
    else
    {
        coordsys = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return coordsys;
}


static char *
taqmgr_get_coordstr(struct taqmgr_interface *taqmgr)
{
    char *coordstr = NULL;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    if (this)
    {
        if (this->userData & USE_USER_INPUT_OBLOC)
            coordstr = &(this->coordString[GEODATA_USER][this->coordSystem][0]);
        else
            coordstr = &(this->coordString[GEODATA_DEFAULT][this->coordSystem][0]);
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));

    return coordstr;
}


static int
taqmgr_get_doplv(struct taqmgr_interface *taqmgr)
{
    int ret = 0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    if (this)
        ret = this->dopLevel;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_get_error(struct taqmgr_interface *taqmgr)
{
    int ret = 0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    if (this)
        ret = this->error;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_is_online(struct taqmgr_interface *taqmgr)
{
    int ret = 0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    if (this)
        ret = this->online;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_set_status(struct taqmgr_interface *taqmgr, int status)
{
    int ret = 0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    if (this)
        this->status = status;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_get_status(struct taqmgr_interface *taqmgr)
{
    int ret = 0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    if (taqmgr)
        ret = this->status;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_set_taqmode(struct taqmgr_interface *taqmgr, int mode)
{
    int ret = 0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    if (this)
    {
        if ((mode >= TAQMODE_POINT_TARGET) && (mode <= TAQMODE_FOS_CORRECTION))
            this->taqMode = mode;
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid taqmode\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_get_taqmode(struct taqmgr_interface *taqmgr)
{
    int ret = 0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    if (this)
        ret = this->taqMode;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_get_foward_angles(struct taqmgr_interface *taqmgr, double *fwdaz, double *fwdel)
{
    int ret = 0;
    int *azbuf = NULL;
    int *elbuf = NULL;
    int *bkbuf = NULL;
    double az = 0.0;
    double el = 0.0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    if (this)
    {
        this->dmc->accumulate(this->dmc, true);

        while (this->dmc->getAccIndex(this->dmc) != NUM_ACCUMULATE_ANGLES)
            MSLEEP(10);

        this->dmc->accumulate(this->dmc, false);
        this->dmc->getAccBuf(this->dmc, &azbuf, &elbuf, &bkbuf);

        for (int i = 0; i < NUM_ACCUMULATE_ANGLES; i++)
        {
            az = az + *(azbuf + i);
            el = el + *(elbuf + i);
        }

        az = MIL2DEG((az / NUM_ACCUMULATE_ANGLES + this->azOffset[this->azOffsetIndex])) + this->magDecl;

        if (az >= 360.0)
            *fwdaz = az - 360.0;
        else if (az < 0.0)
            *fwdaz = az + 360.0;
        else
            *fwdaz = az;

        *fwdel = MIL2DEG((el / NUM_ACCUMULATE_ANGLES + this->elOffset));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_start_target_acquistion(struct taqmgr_interface *taqmgr)
{
    int ret = 0;
    int mil[2] = {0};
    double deg[2] = {0};
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    taqmgr->getBank(taqmgr, &mil[1], &deg[1]);
    taqmgr->getElevation(taqmgr, &mil[0], &deg[0]);

    if (taqmgr->getAzMode(taqmgr) != AZMODE_GRID_NORTH)
    {
        ret = TAQERROR_GRIDVAR;
        TLOGMSG(1, (DBGINFOFMT "gridvar is not set\n", DBGINFO));
    }
    else if ((-800 == mil[0]) || (800 == mil[0]) || (-800 == mil[1]) || (800 == mil[1]))
    {
        ret = TAQERROR_ANGLE_LIMIT;
        TLOGMSG(1, (DBGINFOFMT "elevation or bank error\n", DBGINFO));
    }
    else
    {
        if (taqmgr->getUserData(taqmgr) & USE_USER_INPUT_RANGE)
        {
            this->mrst = MANUAL_RANGING_INPROC;
            this->dmc->setDamping(this->dmc, DMC_DAMPING_NONE);
            this->dmc->setItime(this->dmc, DMC_ITIME_TAQ);
        }
        else
        {
            if (this->lrf->startMeasuring(this->lrf) == 0)
            {
                this->geodata[GEODATA_DEFAULT].range[0] = 0;
                this->geodata[GEODATA_DEFAULT].range[1] = 0;
                this->dmc->setDamping(this->dmc, DMC_DAMPING_NONE);
                this->dmc->setItime(this->dmc, DMC_ITIME_TAQ);
            }
            else
            {
                ret = TAQERROR_START_MEASRUING_RANGE;
                TLOGMSG(1, (DBGINFOFMT "failed to start range measuring\n", DBGINFO));
            }
        }
    }

    return ret;
}


static void
taqmgr_fill_coordstr(taqdata_t *data)
{
    int deglat = 0;
    int deglong = 0;
    int minlat = 0;
    int minlong = 0;
    double seclat = 0.0;
    double seclong = 0.0;
    char *mgrs = &data->observer.coordstr[COORDSYS_MGRS][0];
    char *utm = &data->observer.coordstr[COORDSYS_UTM][0];
    char *geodetic = &data->observer.coordstr[COORDSYS_GEODETIC][0];

    coordtr_geodetic2mgrs(data->observer.latitude, data->observer.longitude, mgrs, utm);
    deg2dms(data->observer.latitude, &deglat, &minlat, &seclat);
    deg2dms(data->observer.longitude, &deglong, &minlong, &seclong);
    snprintf(geodetic, MAXLEN_COORDSTR, "%02d\u00B0%02d\'%02.2f\"%s %03d\u00B0%02d\'%02.2f\"%s",
             deglat, minlat, seclat, data->observer.latitude < 0 ? "S" : "N",
             deglong, minlong, seclong, data->observer.longitude < 0 ? "W" : "E");

    mgrs = &data->target.coordstr[COORDSYS_MGRS][0];
    utm = &data->target.coordstr[COORDSYS_UTM][0];
    geodetic = &data->target.coordstr[COORDSYS_GEODETIC][0];
    coordtr_geodetic2mgrs(data->target.latitude, data->target.longitude, mgrs, utm);
    deg2dms(data->target.latitude, &deglat, &minlat, &seclat);
    deg2dms(data->target.longitude, &deglong, &minlong, &seclong);
    snprintf(geodetic, MAXLEN_COORDSTR, "%02d\u00B0%02d\'%02.2f\"%s %03d\u00B0%02d\'%02.2f\"%s",
             deglat, minlat, seclat, data->target.latitude < 0 ? "S" : "N",
             deglong, minlong, seclong, data->target.longitude < 0 ? "W" : "E");
}


static int
taqmgr_calculate_target_position(struct taqmgr_interface *taqmgr, int range, double fwdaz, double fwdel)
{
    int ret = 0;
    taqdata_t *data = NULL;
    geod_coord_t target = {0.0, 0.0, 0.0};
    geod_coord_t *observer = NULL;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    if (this->userData & USE_USER_INPUT_OBLOC)
        observer = &(this->geodata[GEODATA_USER].coord);
    else
        observer = &(this->geodata[GEODATA_DEFAULT].coord);

    if (geodesic_solve_harvesine_formula(observer, &target, (double)range, fwdaz, fwdel) == 0)
    {
        data = this->taqBuffer->createData(this->taqBuffer);
        clock_gettime(CLOCK_REALTIME, &data->taqtime);
        data->taqtime.tv_sec += get_time_offset();
        data->observer.altitude = observer->altitude;
        data->observer.latitude = observer->latitude;
        data->observer.longitude = observer->longitude;
        data->observer.fwdaz = fwdaz;
        data->observer.fwdel = fwdel;
        data->observer.magdecl = this->magDecl;
        data->observer.gridvar = this->gridVar;
        data->observer.gridcvg = this->gridCvg;
        data->observer.sdist = range;
        data->observer.hdist = (int)lround(range * cos(RADIANS(fwdel)));
        data->target.altitude = target.altitude;
        data->target.latitude = target.latitude;
        data->target.longitude = target.longitude;
        taqmgr_fill_coordstr(data);
        this->taqBuffer->addData(this->taqBuffer, data);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "failed to solve direct problem\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_calculate_target_radius(struct taqmgr_interface *taqmgr)
{
    int ret = TAQ_RETCODE_OK;
    double a1 = 0.0;    // initial bearing
    double a2 = 0.0;    // final bearing
    double radius = 0.0;
    taqdata_t *data = NULL;
    geod_coord_t p1 = {0.0, 0.0, 0.0};
    geod_coord_t p2  = {0.0, 0.0, 0.0};
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    data = this->taqBuffer->getData(this->taqBuffer, 0);
    p1.latitude  = data->target.latitude;
    p1.longitude = data->target.longitude;
    p1.altitude  = data->target.altitude;
    data = this->taqBuffer->getData(this->taqBuffer, 1);
    p2.latitude  = data->target.latitude;
    p2.longitude = data->target.longitude;
    p2.altitude  = data->target.altitude;

    if (geodesic_solve_inverse_problem(&p2, &p1, &a1, &a2, &radius) == 0)
    {
        TLOGMSG(1, ("target radious = %fm\n", radius));

        if (radius > TARGET_RADIUS_MAX)
        {
            ret = TAQ_RETCODE_TARGET_SIZE_OVER;
            TLOGMSG(1, ("target radius range over! (1 ~ %dm)\n", TARGET_RADIUS_MAX));
        }
        else
        {
            if (radius < 1.0)
                radius = 1.0;

            data->target.radius = lround(radius);
            data->target.shape = TARGET_SHAPE_CIRCLE;
        }
    }
    else
    {
        ret = TAQ_RETCODE_FAIL_GEODCALC;
        TLOGMSG(1, (DBGINFOFMT "failed to solve inverse problem\n", DBGINFO));
    }

    this->taqBuffer->setFocus(this->taqBuffer, 0);
    this->taqBuffer->removeData(this->taqBuffer);

    return ret;
}


static int
taqmgr_calculate_sqaure_target_size(struct taqmgr_interface *taqmgr)
{
    int ret = TAQ_RETCODE_OK;
    double length[2] = {0};
    double bearing[4] = {0};
    taqdata_t *data = NULL;
    geod_coord_t point[3] = {0};
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    for (int i = 0; i < 3; i++)
    {
        data = this->taqBuffer->getData(this->taqBuffer, i);
        point[i].latitude  = data->target.latitude;
        point[i].longitude = data->target.longitude;
        point[i].altitude  = data->target.altitude;
    }

    for (int i = 0; i < 2; i++)
    {
        if (geodesic_solve_inverse_problem(&point[2], &point[i], &bearing[i * 2], &bearing[i * 2 + 1], &length[i]) == 0)
        {
            length[i] = length[i] * 2.0;

            if (length[i] < 1.0)
                length[i] = 1.0;
        }
        else
        {
            ret = TAQ_RETCODE_FAIL_GEODCALC;
            TLOGMSG(1, (DBGINFOFMT "failed to solve inverse prolbem\n", DBGINFO));
            break;
        }
    }

    if (ret == TAQ_RETCODE_OK)
    {
        if ((length[0] > TARGET_SIZE_MAX) || (length[1] > TARGET_SIZE_MAX))
        {
            ret = TAQ_RETCODE_TARGET_SIZE_OVER;
            TLOGMSG(1, (DBGINFOFMT "target size range over! (1 ~ %d)\n", DBGINFO, TARGET_SIZE_MAX));
        }
        else
        {
            if (length[0] > length[1])
            {
                data->target.width = lround(length[1]);
                data->target.length = lround(length[0]);
                data->target.attitude = lround(DEG2MIL(bearing[0]));
            }
            else
            {
                data->target.width = lround(length[0]);
                data->target.length = lround(length[1]);
                data->target.attitude = lround(DEG2MIL(bearing[2]));
            }

            data->target.shape = TARGET_SHAPE_SQUARE;
            TLOGMSG(1, ("square target length = %dm, width = %dm, attitude = %d mils\n",
                    data->target.length, data->target.width, data->target.attitude));
        }
    }

    for (int i = 0; i < 2; i++)
    {
        this->taqBuffer->setFocus(this->taqBuffer, 0);
        this->taqBuffer->removeData(this->taqBuffer);
    }

    return ret;
}

static int
taqmgr_calculate_target_shift(struct taqmgr_interface *taqmgr)
{
    int de = 0;
    int dn = 0;
    int ret = TAQ_RETCODE_OK;
    double fwdaz = 0.0;
    double e[2] = {0.0, 0.0};
    double n[2] = {0.0, 0.0};
    taqdata_t *data = NULL;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    for (int i = 0; i < 2; i++)
    {
        data = this->taqBuffer->getData(this->taqBuffer, i);
        n[i] = data->observer.hdist * cos(RADIANS(data->observer.fwdaz));
        e[i] = data->observer.hdist * sin(RADIANS(data->observer.fwdaz));
        TLOGMSG(1, ("E%d = %fm, N%d = %fm\n", i, e[i], i, n[i]));
    }

    dn = lround(n[1] - n[0]);
    de = lround(e[1] - e[0]);
    TLOGMSG(1, ("dE = %dm, dN = %dm\n", de, dn));

    if ((de > SHIFT_MAX) || (de < SHIFT_MIN))
    {
        ret = TAQ_RETCODE_SHIFT_RANGE_OVER;
        TLOGMSG(1, ("lateral shift range over! (-4,095 ~ 4,095m)\n"));
    }
    else if ((dn > SHIFT_MAX) || (dn < SHIFT_MIN))
    {
        ret = TAQ_RETCODE_SHIFT_RANGE_OVER;
        TLOGMSG(1, ("range shift range over! (-4,095 ~ 4,095m)\n"));
    }
    else
    {
        data = this->taqBuffer->getData(this->taqBuffer, 0);
        fwdaz = data->observer.fwdaz;
        data = this->taqBuffer->getData(this->taqBuffer, 1);
        data->shift.fwdaz = fwdaz;
        data->shift.range = dn;
        data->shift.lateral = de;
    }

    this->taqBuffer->setFocus(this->taqBuffer, 0);
    this->taqBuffer->removeData(this->taqBuffer);

    return ret;
}


static int
taqmgr_check_taqcond(struct taqmgr_interface *taqmgr)
{
    int ret = TAQERROR_NONE;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    if (this->dmc->getError(this->dmc) != DMC_ERROR_NONE)
    {
        ret |= TAQERROR_DMC_OFFLINE;
        TLOGMSG(1, (DBGINFOFMT "dmc is offline\n", DBGINFO));
    }

    if (!(taqmgr->getUserData(taqmgr) & USE_USER_INPUT_OBLOC))
    {
        if (!this->online)
        {
            ret |= TAQERROR_GNSS_OFFLINE;
            TLOGMSG(1, (DBGINFOFMT "gnss is offline\n", DBGINFO));
        }
    }

    if (taqmgr->getUserData(taqmgr) & USE_USER_INPUT_RANGE)
    {
        if (this->mrst == MANUAL_RANGING_INPROC)
        {
            this->mrst = MANUAL_RANGING_IDLE;

            if (this->geodata[GEODATA_USER].range[0] == 0)
            {
                ret |= TAQERROR_ZERO_RANGE;
                TLOGMSG(1, (DBGINFOFMT "zero range\n", DBGINFO));
            }
        }
        else
        {
            ret |= TAQERROR_NOT_INPROC;
            TLOGMSG(1, (DBGINFOFMT "not in range measruing process\n", DBGINFO));
        }
    }
    else
    {
        this->lrf->fire(this->lrf);

        if (this->lrf->finalizeMeasuring(this->lrf) == 0)
        {
            this->lrf->getRange(this->lrf, &this->geodata[GEODATA_DEFAULT].range[0], &this->geodata[GEODATA_DEFAULT].range[1]);

            if (this->geodata[GEODATA_DEFAULT].range[0] == 0)
            {
                ret |= TAQERROR_ZERO_RANGE;
                TLOGMSG(1, (DBGINFOFMT "zero range\n", DBGINFO));
            }
        }
        else
        {
            ret |= TAQERROR_NOT_INPROC;
            TLOGMSG(1, (DBGINFOFMT "not in range measruing process\n", DBGINFO));
        }
    }

    return ret;
}


static int
taqmgr_finalize_target_acquistion(struct taqmgr_interface *taqmgr)
{
    int ret = 0;
    int range = 0;
    double fwdaz = 0.0;
    double fwdel = 0.0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    ret = taqmgr_check_taqcond(taqmgr);

    if (ret == TAQERROR_NONE)
    {
        MSLEEP(100);
        taqmgr_get_foward_angles(taqmgr, &fwdaz, &fwdel);

        if (this->userData & USE_USER_INPUT_RANGE)
            range = this->geodata[GEODATA_USER].range[0];
        else
            range = this->geodata[GEODATA_DEFAULT].range[0];

        switch (this->taqMode)
        {
        case TAQMODE_POINT_TARGET:
            if (taqmgr_calculate_target_position(taqmgr, range, fwdaz, fwdel) != 0)
            {
                ret = TAQ_RETCODE_FAIL_GEODCALC;
                TLOGMSG(1, (DBGINFOFMT "failed to calculate point target location\n", DBGINFO));
            }

            break;

        case TAQMODE_CIRCULAR_TARGET:
            if (taqmgr_calculate_target_position(taqmgr, range, fwdaz, fwdel) == 0)
                ret = taqmgr_calculate_target_radius(taqmgr);
            else
            {
                ret = TAQ_RETCODE_FAIL_GEODCALC;
                TLOGMSG(1, (DBGINFOFMT "failed to calculate point target location\n", DBGINFO));
            }

            break;

        case TAQMODE_SQUARE_TARGET_LENGTH:
            if (taqmgr_calculate_target_position(taqmgr, range, fwdaz, fwdel) != 0)
            {
                ret = TAQ_RETCODE_FAIL_GEODCALC;
                TLOGMSG(1, (DBGINFOFMT "failed to calculate point target location\n", DBGINFO));
            }

            break;

        case TAQMODE_SQUARE_TARGET_WIDTH:
            if (taqmgr_calculate_target_position(taqmgr, range, fwdaz, fwdel) == 0)
                ret = taqmgr_calculate_sqaure_target_size(taqmgr);
            else
            {
                ret = TAQ_RETCODE_FAIL_GEODCALC;
                TLOGMSG(1, (DBGINFOFMT "failed to calculate point target location\n", DBGINFO));
            }

            break;

        case TAQMODE_FOS_CORRECTION:
            if (taqmgr_calculate_target_position(taqmgr, range, fwdaz, fwdel) == 0)
                ret = taqmgr_calculate_target_shift(taqmgr);
            else
            {
                ret = TAQ_RETCODE_FAIL_GEODCALC;
                TLOGMSG(1, (DBGINFOFMT "failed to calculate point target location\n", DBGINFO));
            }

            break;

        default:
            ret = TAQ_RETCODE_INVALID_TAQMODE;
            TLOGMSG(1, (DBGINFOFMT "invalid taqmode\n", DBGINFO));
            break;
        }
    }
    else
    {
        if (ret & TAQERROR_ZERO_RANGE)
            ret = TAQ_RETCODE_ZERO_RANGE;
        else if (ret & TAQERROR_DMC_OFFLINE)
            ret = TAQ_RETCODE_DMC_ERROR;
        else if (ret & TAQERROR_GNSS_OFFLINE)
            ret = TAQ_RETCODE_GNSS_ERROR;
        else
            ret = TAQ_RETCODE_ERROR;

        TLOGMSG(1, (DBGINFOFMT "failed to satisfied taqcond\n", DBGINFO));
    }

    this->dmc->setItime(this->dmc, DMC_ITIME_DEFAULT);
    this->dmc->setDamping(this->dmc, DMC_DAMPING_DEFAULT);

    return ret;
}


static int
taqmgr_get_range(struct taqmgr_interface *taqmgr, int *first, int *last)
{
    int ret = 0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    if (this)
    {
        if (taqmgr->getUserData(taqmgr) & USE_USER_INPUT_RANGE)
        {
            *first = this->geodata[GEODATA_USER].range[0];
            *last = this->geodata[GEODATA_USER].range[1];
        }
        else
        {
            *first = this->geodata[GEODATA_DEFAULT].range[0];
            *last = this->geodata[GEODATA_DEFAULT].range[1];
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_get_azimuth(struct taqmgr_interface *taqmgr, int *mil, double *deg)
{
    int ret = 0;
    int temp_mil = 0;
    double temp = 0.0;
    double temp_deg = 0.0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    if (this)
    {
        this->dmc->getAzimuth(this->dmc, &temp_mil, &temp_deg);
        temp = (temp_mil + this->azOffset[this->azOffsetIndex]);

        if (this->azMode == AZMODE_GRID_NORTH)
            temp = temp + DEG2MIL(this->gridVar);

        temp_mil = (int) lround(temp);

        if (temp_mil > 6399)
            *mil = temp_mil - 6400;
        else if (temp_mil < 0)
            *mil = temp_mil + 6400;
        else
            *mil = temp_mil;

        *deg = MIL2DEG(*mil);

        TLOGMSG(0, ("azimuth = %d mil\n", *mil));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_get_bank(struct taqmgr_interface *taqmgr, int *mil, double *deg)
{
    int ret = 0;
    int temp_mil = 0;
    double temp_deg = 0.0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    if (this)
    {
        this->dmc->getBank(this->dmc, &temp_mil, &temp_deg);

        if (temp_mil > 800)
            *mil = 800;
        else if (temp_mil < -800)
            *mil = -800;
        else
            *mil = temp_mil;

        *deg = MIL2DEG(*mil);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_get_elevation(struct taqmgr_interface *taqmgr, int *mil, double *deg)
{
    int ret = 0;
    int temp_mil = 0;
    double temp_deg = 0.0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    if (this)
    {
        this->dmc->getElevation(this->dmc, &temp_mil, &temp_deg);
        temp_mil = lround(temp_mil + this->elOffset);

        if (temp_mil > 800)
            *mil = 800;
        else if (temp_mil < -800)
            *mil = -800;
        else
            *mil = temp_mil;

        *deg = MIL2DEG(*mil);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_get_azmode(struct taqmgr_interface *taqmgr)
{
    int ret = 0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    if (this)
        ret = this->azMode;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_set_azmode(struct taqmgr_interface *taqmgr, int mode)
{
    int ret = 0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    if (this)
    {
        switch (mode)
        {
        case AZMODE_TRUE_NORTH:
            this->azMode = mode;
            TLOGMSG(1, ("azimuth mode = true north\n"));
            break;

        case AZMODE_GRID_NORTH:
            this->azMode = mode;
            TLOGMSG(1, ("azimuth mode = grid north\n"));
            break;

        case AZMODE_MAGNETIC_NORTH:
            this->azMode = mode;
            TLOGMSG(1, ("azimuth mode = magnetic north\n"));
            break;

        default:
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid azimuth mode flag\n", DBGINFO));
            break;
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_get_magdecl(struct taqmgr_interface *taqmgr, double *magdecl)
{
    int ret = 0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    if (this)
    {
        if (this->userData & USE_USER_INPUT_GRIDVAR)
        {
            if (this->userData & USE_USER_INPUT_OBLOC)
                *magdecl = this->magDecl;
            else if (this->online)
                *magdecl = this->magDecl;
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "observer location is not set\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "grid varition is not set\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_set_azimuth_offset(struct taqmgr_interface *taqmgr, int offidx, double offset)
{
    int ret = 0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    if (this)
    {
        if ((offidx >= AZOFF_DV_PCELL) && (offidx <= AZOFF_IR_EXTDC))
        {
            this->azOffset[offidx] = offset;
            TLOGMSG(1, ("azimuth offset[%d] = %.3f\n", offidx, offset));
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid argument for offset index\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_set_elevation_offset(struct taqmgr_interface *taqmgr, double offset)
{
    int ret = 0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *)taqmgr;

    if (this)
    {
        this->elOffset = offset;
        TLOGMSG(1, ("elevation offset = %.3f\n", this->elOffset));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_get_angular_offset(struct taqmgr_interface *taqmgr, double *azoff, double *eloff)
{
    int ret = 0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *)taqmgr;

    if (this)
    {
        *azoff = this->azOffset[0];
        *eloff = this->elOffset;
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_set_user_gridvar(struct taqmgr_interface *taqmgr, double gridvar)
{
    int ret = 0;
    double latitude = 0.0;
    double longitude = 0.0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

    if (this)
    {
        if ((GRIDVAR_MIN <= gridvar) && (GRIDVAR_MAX >= gridvar))
        {
            if (this->fixLoc & USE_USER_INPUT_OBLOC)
            {
                latitude = this->geodata[GEODATA_USER].coord.latitude;
                longitude = this->geodata[GEODATA_USER].coord.longitude;
                this->gridVar = gridvar;
                this->gridCvg = geodesic_get_gridcvg(latitude, longitude);
                this->magDecl = this->gridCvg + this->gridVar;
                TLOGMSG(1, ("gridcvg = %f, gridvar = %f, magdecl = %f\n", this->gridCvg, this->gridVar, this->magDecl));
            }
            else
            {
                if (this->online)
                {
                    latitude = this->geodata[GEODATA_DEFAULT].coord.latitude;
                    longitude = this->geodata[GEODATA_DEFAULT].coord.longitude;
                    this->gridVar = gridvar;
                    this->gridCvg = geodesic_get_gridcvg(latitude, longitude);
                    this->magDecl = this->gridCvg + this->gridVar;
                    TLOGMSG(1, ("gridcvg = %f, gridvar = %f, magdecl = %f\n", this->gridCvg, this->gridVar, this->magDecl));
                }
                else
                    this->gridVar = gridvar;
            }

            taqmgr_set_azmode(taqmgr, AZMODE_GRID_NORTH);
            this->userData |= USE_USER_INPUT_GRIDVAR;
            TLOGMSG(1, ("user input grid variation = %+05d mil\n", lround(DEG2MIL(gridvar))));
        }
        else
        {
            ret = -1;
            TLOGMSG(1, ("invaild value for grid variation\n"));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_get_user_gridvar(struct taqmgr_interface *taqmgr, double *gridvar)
{
    int ret = 0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *)taqmgr;

    if (this)
    {
        if (this->userData & USE_USER_INPUT_GRIDVAR)
            *gridvar = this->gridVar;
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "grid variation is not set\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_set_user_obloc(struct taqmgr_interface *taqmgr, char *coordstr, int alt)
{
    int ret = 0;
    int latdeg = 0;
    int lngdeg = 0;
    int latmin = 0;
    int lngmin = 0;
    int mgrslen = 0;
    int utmlen = 0;
    char ns = 'N';
    char ew = 'E';
    char *utm = NULL;
    char *mgrs = NULL;
    char *geodetic = NULL;
    double latsec = 0.0;
    double lngsec = 0.0;
    double latitude = 0.0;
    double longitude = 0.0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *)taqmgr;

    if (this)
    {
        if (coordstr)
        {
            utm = &(this->coordString[GEODATA_USER][COORDSYS_UTM][0]);
            mgrs = &(this->coordString[GEODATA_USER][COORDSYS_MGRS][0]);
            geodetic = &(this->coordString[GEODATA_USER][COORDSYS_GEODETIC][0]);

            if ((*coordstr == '0') && (*(coordstr + 1) == '0'))
                ret = coordtr_mgrs2geodetic(coordstr + 2, &latitude, &longitude);
            else
                ret = coordtr_mgrs2geodetic(coordstr, &latitude, &longitude);

            if (ret == 0)
            {
                deg2dms(latitude, &latdeg, &latmin, &latsec);
                deg2dms(longitude, &lngdeg, &lngmin, &lngsec);

                if (latitude < 0.0) ns = 'S';
                if (longitude < 0.0) ew = 'W';

                coordtr_geodetic2mgrs(latitude, longitude, mgrs, utm);
                utmlen = strnlen(utm, (size_t)MAXLEN_COORDSTR);
                mgrslen = strnlen(mgrs, (size_t)MAXLEN_COORDSTR);
                snprintf(geodetic, (size_t)MAXLEN_COORDSTR, "%02d\u00B0%02d\'%02.2f\"%c %03d\u00B0%02d\'%02.2f\"%c %+dm",
                         latdeg, latmin, latsec, ns, lngdeg, lngmin, lngsec, ew, alt);

                snprintf(mgrs + mgrslen, (size_t)(MAXLEN_COORDSTR - mgrslen), " %+dm", alt);
                snprintf(utm + utmlen, (size_t)(MAXLEN_COORDSTR - utmlen), " %+dm", alt);

                this->geodata[GEODATA_USER].coord.latitude  = latitude;
                this->geodata[GEODATA_USER].coord.longitude = longitude;
                this->geodata[GEODATA_USER].coord.altitude  = (double)alt;

                if (this->userData & USE_USER_INPUT_GRIDVAR)
                {
                    this->gridCvg = geodesic_get_gridcvg(latitude, longitude);
                    this->magDecl = this->gridCvg + this->gridVar;
                    TLOGMSG(1, ("gridvar = %f, gridcvg = %f, magdecl = %f\n", this->gridVar, this->gridCvg, this->magDecl));
                }

                this->userData |= USE_USER_INPUT_OBLOC;
                TLOGMSG(1, ("user input observation location = %f, %f\n", latitude, longitude));
            }
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "invalid obloc\n", DBGINFO));
            }
        }
        else
        {
            this->userData &= (~USE_USER_INPUT_OBLOC);
            TLOGMSG(1, ("disable user obloc\n"));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_get_user_obloc(struct taqmgr_interface *taqmgr, char *buf, int *alt)
{
    int ret = 0;
    char utm[32] = {0};
    char mgrs[32] = {0};
    geod_coord_t *coord = NULL;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *)taqmgr;

    if (this)
    {
        memset(utm, 0x00, sizeof(utm));
        memset(mgrs, 0x00, sizeof(mgrs));
        coord = &(this->geodata[GEODATA_USER].coord);
        coordtr_geodetic2mgrs(coord->latitude, coord->longitude, mgrs, utm);

        if (isdigit(mgrs[0]))
        {
            memcpy(buf, &mgrs[0], sizeof(char) * 5);
            memcpy((buf + 5), &mgrs[6], sizeof(char) * 5);
            memcpy((buf + 10), &mgrs[12], sizeof(char) * 5);
        }
        else
        {
            memset(buf, 0x30, sizeof(char) * 2);
            memcpy((buf + 2), &mgrs[0], sizeof(char) * 3);
            memcpy((buf + 5), &mgrs[4], sizeof(char) * 5);
            memcpy((buf + 10), &mgrs[10], sizeof(char) * 5);
        }

        *alt = (int) coord->altitude;
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_set_user_range(struct taqmgr_interface *taqmgr, int range)
{
    int ret = 0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *)taqmgr;

    if (this)
    {
        if (range == -1)
        {
            this->userData &= (~USE_USER_INPUT_RANGE);
            TLOGMSG(1, ("disable user range\n"));
        }
        else if (range < MIN_USER_RANGE)
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid range\n", DBGINFO));
        }
        else
        {
            this->geodata[GEODATA_USER].range[0] = range;
            this->geodata[GEODATA_USER].range[1] = 0;
            this->userData |= USE_USER_INPUT_RANGE;
            TLOGMSG(1, ("enable user range = %dm\n", range));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_get_user_range(struct taqmgr_interface *taqmgr, int *range)
{
    int ret = 0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *)taqmgr;

    if (this)
        *range = this->geodata[GEODATA_USER].range[0];
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_get_obloc(struct taqmgr_interface *taqmgr, double *lat, double *lng, double *alt)
{
    int ret = 0;
    int idx = 0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *)taqmgr;

    if (this)
    {
        if (this->userData & USE_USER_INPUT_OBLOC)
            idx = GEODATA_USER;
        else
            idx = GEODATA_DEFAULT;

        *lat = this->geodata[idx].coord.latitude;
        *lng = this->geodata[idx].coord.longitude;
        *alt = this->geodata[idx].coord.altitude;
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_get_userdata_status(struct taqmgr_interface *taqmgr)
{
    int ret = 0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *)taqmgr;

    if (this)
        ret = this->userData;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_get_mrst(struct taqmgr_interface *taqmgr)
{
    int ret = 0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *)taqmgr;

    if (this)
        ret = this->mrst;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_get_result(struct taqmgr_interface *taqmgr, taqdata_t *data)
{
    int ret = 0;
    taqdata_t *temp = NULL;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *)taqmgr;

    if (this)
    {
        if (this->status == TAQMGR_STATUS_TARGET_ACQUIRED)
        {
            pthread_mutex_lock(&this->mtx);
            temp = this->taqBuffer->getData(this->taqBuffer, 0);

            if (temp)
                memcpy(data, temp, sizeof(taqdata_t));
            else
            {
                ret = -1;
                TLOGMSG(0, ("no taq result in taqdata buffer\n"));
            }

            pthread_mutex_unlock(&this->mtx);
        }
        else
        {
            ret = -1;
            TLOGMSG(0, ("target acquition is in processing\n"));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_terminate(struct taqmgr_interface *taqmgr)
{
    int ret = 0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *)taqmgr;

    if (this)
    {
        if (this->taqMode == TAQMODE_SQUARE_TARGET_WIDTH)
        {
            this->taqBuffer->setFocus(this->taqBuffer, 0);
            this->taqBuffer->removeData(this->taqBuffer);
        }

        this->taqMode = TAQMODE_POINT_TARGET;
        this->status = TAQMGR_STATUS_TARGET_ACQUIRED;
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_reset(struct taqmgr_interface *taqmgr)
{
    int ret = 0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *)taqmgr;

    if (this)
    {
        pthread_mutex_lock(&this->mtx);

        while(this->taqBuffer->getNumData(this->taqBuffer) != 0)
            this->taqBuffer->removeData(this->taqBuffer);

        taqmgr_set_taqmode(taqmgr, TAQMODE_POINT_TARGET);
        pthread_mutex_unlock(&this->mtx);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


static int
taqmgr_get_dist_offset_short(struct taqmgr_interface *taqmgr, double *A, double *b)
{
	int ret = 0;

	struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

	if(this)
	{
		*A = atof(devconf_get_value(DEVCONF_KEY_SRCCA));
		*b = atof(devconf_get_value(DEVCONF_KEY_SRCCB));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
	}

	return ret;
}


static int
taqmgr_set_dist_offset_short(struct taqmgr_interface *taqmgr, char *A, char *b)
{
	int ret = 0;
	struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

	if(this)
	{
		devconf_set_value(DEVCONF_KEY_SRCCA, A);
		devconf_set_value(DEVCONF_KEY_SRCCB, b);
		devconf_save_parameters(DEVCONF_FILE_PATH);
		TLOGMSG(1, ("set short distanc correction value A = %s, B = %s\n", A, b));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
	}

	return ret;
}


static int
taqmgr_get_dist_offset_middle(struct taqmgr_interface *taqmgr, double *A, double *b)
{
	int ret = 0;
	struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

	if(this)
	{
		*A = atof(devconf_get_value(DEVCONF_KEY_MRCCA));
		*b = atof(devconf_get_value(DEVCONF_KEY_MRCCB));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
	}

	return ret;
}


static int
taqmgr_set_dist_offset_middle(struct taqmgr_interface *taqmgr, char *A, char *b)
{
	int ret = 0;
	struct taqmgr_attribute *this = (struct taqmgr_attribute *)taqmgr;

	if(this)
	{
		devconf_set_value(DEVCONF_KEY_MRCCA, A);
		devconf_set_value(DEVCONF_KEY_MRCCB, b);
		devconf_save_parameters(DEVCONF_FILE_PATH);
		TLOGMSG(1, ("set middle distanc correction value A = %s, B = %s\n", A, b));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
	}


	return ret;
}

static int
taqmgr_get_dist_offset_long(struct taqmgr_interface *taqmgr, double *A, double *b)
{
	int ret = 0;
	struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

	if(this)
	{
		*A = atof(devconf_get_value(DEVCONF_KEY_LRCCA));
		*b = atof(devconf_get_value(DEVCONF_KEY_LRCCB));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
	}

	return ret;
}

static int
taqmgr_set_dist_offset_long(struct taqmgr_interface *taqmgr, char *A, char *b)
{
	int ret = 0;
	struct taqmgr_attribute *this = (struct taqmgr_attribute *)taqmgr;

	if(this)
	{
		devconf_set_value(DEVCONF_KEY_LRCCA, A);
		devconf_set_value(DEVCONF_KEY_LRCCB, b);
		devconf_save_parameters(DEVCONF_FILE_PATH);
		TLOGMSG(1, ("set long distanc correction value A = %s, B = %s\n", A, b));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
	}

	return ret;
}

static int
taqmgr_get_angle_offset_azimuth(struct taqmgr_interface *taqmgr, double *azimuth)
{
	int ret = 0;
	struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

	if(this)
		*azimuth = atof(devconf_get_value(DEVCONF_KEY_AZOFF));
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
	}

	return ret;
}


static int
taqmgr_set_angle_offset_azimuth(struct taqmgr_interface *taqmgr, char *azimuth)
{
	int ret = 0;
	struct taqmgr_attribute *this = (struct taqmgr_attribute *)taqmgr;

	if(this)
	{
		devconf_set_value(DEVCONF_KEY_AZOFF, azimuth);
		devconf_save_parameters(DEVCONF_FILE_PATH);
		TLOGMSG(1, ("set azimuth correction value A = %s\n", azimuth));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
	}


	return ret;
}

static int
taqmgr_get_angle_offset_altitude(struct taqmgr_interface *taqmgr, double *altitude)
{
	int ret = 0;
	struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

	if(this)
		*altitude = atof(devconf_get_value(DEVCONF_KEY_ELOFF));
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
	}

	return ret;
}


static int
taqmgr_set_angle_offset_altitude(struct taqmgr_interface *taqmgr, char *altitude)
{
	int ret = 0;
	struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

	if(this)
	{
		devconf_set_value(DEVCONF_KEY_ELOFF, altitude);
		devconf_save_parameters(DEVCONF_FILE_PATH);
		TLOGMSG(1, ("set altitude correction value A = %+.3lf\n", altitude));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
	}

	return ret;
}

static int
taqmgr_get_sensor_offset(struct taqmgr_interface *taqmgr, unsigned char *open, unsigned char *close)
{
	int ret = 0;
	struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

	if(this)
	{
		*open = (unsigned char)atoi(devconf_get_value(DEVCONF_KEY_PROXOPEN));
		*close = (unsigned char)atoi(devconf_get_value(DEVCONF_KEY_PROXCLOSE));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
	}

	return ret;
}

static int
taqmgr_set_sensor_offset(struct taqmgr_interface *taqmgr, char *open, char *close)
{
	int ret = 0;
	struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

	if(this)
	{
		devconf_set_value(DEVCONF_KEY_PROXOPEN, open);
		devconf_set_value(DEVCONF_KEY_PROXCLOSE, close);
		devconf_save_parameters(DEVCONF_FILE_PATH);
		TLOGMSG(1, ("set sensor correction value open = %s, close = %s\n", open, close));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
	}

	return ret;
}


static int
taqmgr_get_irarray_offset(struct taqmgr_interface *taqmgr, unsigned char *V, unsigned char *H)
{
	int ret = 0;
	struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

	if(this)
	{
		*H = atof(devconf_get_value(DEVCONF_KEY_IRHSHIFT));
		*V = atof(devconf_get_value(DEVCONF_KEY_IRVSHIFT));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
	}

	return ret;
}

static int
taqmgr_set_irarray_offset(struct taqmgr_interface *taqmgr, char *V, char *H)
{
	int ret = 0;
	struct taqmgr_attribute *this = (struct taqmgr_attribute *) taqmgr;

	if(this)
	{
		devconf_set_value(DEVCONF_KEY_IRHSHIFT, H);
		devconf_set_value(DEVCONF_KEY_IRVSHIFT, V);
		devconf_save_parameters(DEVCONF_FILE_PATH);
		TLOGMSG(1, ("set ir array correction value h = %s, v = %s\n", H, V));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
	}

	return ret;
}

static int
taqmgr_set_ir_scaleup(struct taqmgr_interface *taqmgr, char *scale)
{
	int ret = 0;
	struct taqmgr_attribute *this = (struct taqmgr_attribute *)taqmgr;

	if(this)
	{
		devconf_set_value(DEVCONF_KEY_IRUPSCALE, scale);
		devconf_save_parameters(DEVCONF_FILE_PATH);
		TLOGMSG(1, ("set ir array scaleup %s\n", scale));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
	}
	return ret;
}

static int
taqmgr_get_ir_scaleup(struct taqmgr_interface *taqmgr, int *scale)
{
	int ret = 0;
	struct taqmgr_attribute *this = (struct taqmgr_attribute *)taqmgr;

	if(this)
		*scale = atoi(devconf_get_value(DEVCONF_KEY_IRUPSCALE));

	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
	}

	return ret;
}


static int
taqmgr_save_result(struct taqmgr_interface *taqmgr)
{
    int ret = 0;
    taqdata_t *data = NULL;
    taqdata_t *temp = NULL;
    struct taqdata_manager_interface *taqbuf = NULL;
    struct taqdata_manager_interface *tlist = NULL;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *)taqmgr;

    if (this)
    {
        taqbuf = this->taqBuffer;
        tlist = this->targetList;
        data = tlist->createData(tlist);

        if (data)
        {
            temp = taqbuf->getData(taqbuf, 0);

            if (temp)
            {
                memcpy(data, temp, sizeof(taqdata_t));
                tlist->addData(tlist, data);
                tlist->saveData(tlist, PATH_TARGET_LIST);
                TLOGMSG(1, ("taqdata added to target list and saved\n"));
            }
            else
            {
                ret = -1;
                TLOGMSG(0, ("no taq result in taqdata buffer\n"));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to create target data\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}


taqdata_manager_t *
taqmgr_get_target_list(struct taqmgr_interface *taqmgr)
{
    taqdata_manager_t *tlist = NULL;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *)taqmgr;

    if (this)
        tlist = this->targetList;
    else
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));

    return tlist;
}


struct taqmgr_interface *
taqmgr_create(struct dmc_interface *dmc, struct gnss_interface *gnss, struct lrf_interface *lrf)
{
    struct taqmgr_interface *taqmgr = NULL;
    struct taqmgr_attribute *this = NULL;

    if (dmc && gnss && lrf)
    {
        this = malloc(sizeof(struct taqmgr_attribute));

        if (this)
        {
            this->online    = false;
            this->run       = true;
            this->error     = GNSS_ERROR_NONE;
            this->azMode    = AZMODE_MAGNETIC_NORTH;
            this->taqMode   = TAQMODE_POINT_TARGET;
            this->status    = TAQMGR_STATUS_IDLE;
            this->userData  = USE_DEVICE_ACQUIRED_DATA;
            this->mrst      = MANUAL_RANGING_IDLE;
            this->dmc       = dmc;
            this->gnss      = gnss;
            this->lrf       = lrf;
            this->dopLevel     = DOPLV_POOR;
            this->coordSystem  = COORDSYS_MGRS;
            this->elOffset   = 0.0;
            this->gridCvg    = 0.0;
            this->gridVar    = 0.0;
            this->magDecl    = 0.0;
            this->azOffset[0] = 0.0;
            this->azOffset[1] = 0.0;
            this->azOffset[2] = 0.0;
            this->azOffset[3] = 0.0;
            this->azOffset[4] = 0.0;
            this->azOffset[5] = 0.0;
            this->azOffset[6] = 0.0;
            this->azOffsetIndex   = 0;
            this->geodata[0].range[0] = 0;
            this->geodata[0].range[1] = 0;
            this->geodata[0].coord.altitude = 0.0;
            this->geodata[0].coord.latitude = 0.0;
            this->geodata[0].coord.longitude = 0.0;
            this->geodata[1].range[0] = 0;
            this->geodata[1].range[1] = 0;
            this->geodata[1].coord.altitude = 100.0;
            this->geodata[1].coord.latitude = GEODATA_USER_DEFAULT_LATITUDE;
            this->geodata[1].coord.longitude = GEODATA_USER_DEFAULT_LONGITUDE;
            memset(&(this->coordString[0][0][0]), 0x00, MAXLEN_COORDSTR);
            memset(&(this->coordString[0][1][0]), 0x00, MAXLEN_COORDSTR);
            memset(&(this->coordString[0][2][0]), 0x00, MAXLEN_COORDSTR);
            memset(&(this->coordString[1][0][0]), 0x00, MAXLEN_COORDSTR);
            memset(&(this->coordString[1][1][0]), 0x00, MAXLEN_COORDSTR);
            memset(&(this->coordString[1][2][0]), 0x00, MAXLEN_COORDSTR);
            devconf_reset_parameters();
            this->targetList = taqdata_manager_create();
            this->taqBuffer  = taqdata_manager_create();
            pthread_mutex_init(&this->mtx, NULL);

            taqmgr = &(this->extif);
            taqmgr->start           = taqmgr_start_target_acquistion;
            taqmgr->finalize        = taqmgr_finalize_target_acquistion;
            taqmgr->getStatus       = taqmgr_get_status;
            taqmgr->setStatus       = taqmgr_set_status;
            taqmgr->getError        = taqmgr_get_error;
            taqmgr->isOnline        = taqmgr_is_online;
            taqmgr->getMrst         = taqmgr_get_mrst;
            taqmgr->getRange        = taqmgr_get_range;
            taqmgr->getAzimuth      = taqmgr_get_azimuth;
            taqmgr->getBank         = taqmgr_get_bank;
            taqmgr->getElevation    = taqmgr_get_elevation;
            taqmgr->getAngleOffset  = taqmgr_get_angular_offset;
            taqmgr->getCoordSystem  = taqmgr_get_coordsys;
            taqmgr->setCoordSystem  = taqmgr_set_coordsys;
            taqmgr->getCoordString  = taqmgr_get_coordstr;
            taqmgr->getDopLevel     = taqmgr_get_doplv;
            taqmgr->getAzMode       = taqmgr_get_azmode;
            taqmgr->setAzMode       = taqmgr_set_azmode;
            taqmgr->setTaqMode      = taqmgr_set_taqmode;
            taqmgr->getTaqMode      = taqmgr_get_taqmode;
            taqmgr->getUserData     = taqmgr_get_userdata_status;
            taqmgr->setGridVar      = taqmgr_set_user_gridvar;
            taqmgr->getGridVar      = taqmgr_get_user_gridvar;
            taqmgr->getMagDecl      = taqmgr_get_magdecl;
            taqmgr->getGridCvg      = taqmgr_get_magdecl;
            taqmgr->setUserOrigin   = taqmgr_set_user_obloc;
            taqmgr->getUserOrigin   = taqmgr_get_user_obloc;
            taqmgr->setUserRange    = taqmgr_set_user_range;
            taqmgr->getUserRange    = taqmgr_get_user_range;
            taqmgr->getOrigin       = taqmgr_get_obloc;
            taqmgr->getResult       = taqmgr_get_result;
            taqmgr->reset           = taqmgr_reset;
            taqmgr->terminate       = taqmgr_terminate;
            taqmgr->saveResult      = taqmgr_save_result;
            taqmgr->getDistOffsetSH = taqmgr_get_dist_offset_short;
            taqmgr->setDistOffsetSH = taqmgr_set_dist_offset_short;
            taqmgr->getDistOffsetMD = taqmgr_get_dist_offset_middle;
            taqmgr->setDistOffsetMD = taqmgr_set_dist_offset_middle;
            taqmgr->getDistOffsetLG = taqmgr_get_dist_offset_long;
            taqmgr->setDistOffsetLG = taqmgr_set_dist_offset_long;
            taqmgr->getAngleOffsetAZ = taqmgr_get_angle_offset_azimuth;
            taqmgr->setAngleOffsetAZ = taqmgr_set_angle_offset_azimuth;
            taqmgr->getAngleOffsetAT = taqmgr_get_angle_offset_altitude;
            taqmgr->setAngleOffsetAT = taqmgr_set_angle_offset_altitude;
            taqmgr->getSensorOffset = taqmgr_get_sensor_offset;
            taqmgr->setSensorOffset = taqmgr_set_sensor_offset;
            taqmgr->getIrArrayOffset =  taqmgr_get_irarray_offset;
            taqmgr->setIrArrayOffset = taqmgr_set_irarray_offset;
            taqmgr->setIrScaleUp	= taqmgr_set_ir_scaleup;
            taqmgr->getIrScaleUp	= taqmgr_get_ir_scaleup;
            taqmgr->getTargetList   = taqmgr_get_target_list;

            if (pthread_create(&this->tid, NULL, taqmgr_update_status, (void *) taqmgr) == 0)
            {
//                taqmgr_load_range_comensation_parameter(taqmgr);
//                taqmgr_load_angular_compensation_parameter(taqmgr);
                taqmgr_load_factory_mode_parameter(taqmgr);
//                this->targetList->loadData(this->targetList, PATH_TARGET_LIST);
                TLOGMSG(1, ("create target acquisition manager interface\n"));
            }
            else
            {
                pthread_mutex_destroy(&this->mtx);
                taqdata_manager_destroy(this->targetList);
                taqdata_manager_destroy(this->taqBuffer);
                free(this);
                taqmgr = NULL;
                TLOGMSG(1, (DBGINFOFMT "failed to create taqmgr update status thread\n", DBGINFO));
            }
        }
        else
            TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));
    }
    else
    {
        if (!dmc)
            TLOGMSG(1, (DBGINFOFMT "null dmc interface\n", DBGINFO));

        if (!gnss)
            TLOGMSG(1, (DBGINFOFMT "null gnss interface\n", DBGINFO));

        if (!lrf)
            TLOGMSG(1, (DBGINFOFMT "null lrf interface\n", DBGINFO));
    }

    return taqmgr;
}


int
taqmgr_destroy(struct taqmgr_interface *taqmgr)
{
    int ret = 0;
    struct taqmgr_attribute *this = (struct taqmgr_attribute *)taqmgr;

    if (this)
    {
        this->run = false;
        pthread_join(this->tid, NULL);
        pthread_mutex_destroy(&this->mtx);
        taqdata_manager_destroy(this->targetList);
        taqdata_manager_destroy(this->taqBuffer);
        free(this);
        TLOGMSG(1, ("destroy target acquisition manager interface\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null taqmgr interface\n", DBGINFO));
    }

    return ret;
}
