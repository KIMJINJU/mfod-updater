
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "etc/util.h"
#include "core/taqmgr.h"
#include "core/logger.h"
#include "gui/gui_str.h"
#include "gui/gui_svplot.h"


#define TAGLEN      32
#define VALLEN      128


/* structure declarations : svplot attribute */
struct gui_svplot_attribute
{
    /* external interface */
    struct gui_svplot_interface extif;

    /* internal interface */
    char value[13][VALLEN];
    char tag[13][TAGLEN];
    rect_t position[13][2];
    struct svcoord svcoord[64];
    struct gnss_interface *gnss;
    struct taqmgr_interface *taqmgr;
};


static rect_t pos[13][2] =
{
    {{.x =   0, .y =   0, .w =   0, .h  =   0}, {.x = 385, .y = 278, .w = 220, .h = 220}},
    {{.x = 200, .y = 280, .w =  40, .h  =  20}, {.x = 275, .y = 280, .w = 100, .h =  20}},
    {{.x = 200, .y = 300, .w =  40, .h  =  20}, {.x = 275, .y = 300, .w = 100, .h =  20}},
    {{.x = 200, .y = 320, .w =  40, .h  =  20}, {.x = 275, .y = 320, .w =  40, .h =  20}},
    {{.x = 200, .y = 340, .w =  40, .h  =  20}, {.x =   0, .y =   0, .w =   0, .h =   0}},
    {{.x = 275, .y = 340, .w =  60, .h  =  20}, {.x = 340, .y = 340, .w =  20, .h =  20}},
    {{.x = 275, .y = 360, .w =  60, .h  =  20}, {.x = 340, .y = 360, .w =  20, .h =  20}},
    {{.x = 200, .y = 380, .w =  40, .h  =  20}, {.x = 275, .y = 380, .w =  20, .h =  20}},
    {{.x = 200, .y = 400, .w =  40, .h  =  20}, {.x = 275, .y = 400, .w =  20, .h =  20}},
    {{.x = 200, .y = 420, .w =  40, .h  =  20}, {.x = 275, .y = 420, .w =  20, .h =  20}},
    {{.x = 200, .y = 440, .w =  40, .h  =  20}, {.x = 275, .y = 440, .w = 100, .h =  20}},
    {{.x = 200, .y = 460, .w =  40, .h  =  20}, {.x = 275, .y = 460, .w = 100, .h =  20}},
    {{.x = 200, .y = 480, .w =  40, .h  =  20}, {.x = 275, .y = 480, .w = 100, .h =  20}},
};


static int
gui_svplot_update(struct gui_svplot_interface *svp)
{
    int ret = 0;
    int deg = 0;
    int min = 0;
    int nsu = 0;
    int gps = 0;
    int glns = 0;
    char hemi = 0;
    double sec = 0.0;
    double alt = 0.0;
    double pdop = 0.0;
    double hdop = 0.0;
    double vdop = 0.0;
    double magdecl = 0.0;
    double gridvar = 0.0;
    struct gnss_interface *gnss = NULL;
    struct taqmgr_interface *taqmgr = NULL;
    struct gui_svplot_attribute *this = (struct gui_svplot_attribute *) svp;

    if (this)
    {
        gnss = this->gnss;
        taqmgr = this->taqmgr;

        if (taqmgr->isOnline(taqmgr))
        {
            gnss->getLatitude(gnss, &deg, &min, &sec, &hemi);
            snprintf(&this->value[SVPLOT_INDEX_LATITUDE][0], VALLEN, "%02d\u00B0%02d\'%02.2f\"%c", deg, min, sec, hemi);
            gnss->getLongitude(gnss, &deg, &min, &sec, &hemi);
            snprintf(&this->value[SVPLOT_INDEX_LONGITUDE][0], VALLEN, "%03d\u00B0%02d\'%02.2f\"%c", deg, min, sec, hemi);
            gnss->getAltitude(gnss, &alt);
            snprintf(&this->value[SVPLOT_INDEX_ALTITUDE][0], VALLEN, "%+05dm", (int)lround(alt));

            gnss->getDOP(gnss, &pdop, &hdop, &vdop);
            snprintf(&this->value[SVPLOT_INDEX_PDOP][0], VALLEN, "%.2f", pdop);
            snprintf(&this->value[SVPLOT_INDEX_HDOP][0], VALLEN, "%.2f", hdop);
            snprintf(&this->value[SVPLOT_INDEX_VDOP][0], VALLEN, "%.2f", vdop);

            taqmgr->getGridVar(taqmgr, &gridvar);
            taqmgr->getMagDecl(taqmgr, &magdecl);
            snprintf(&this->value[SVPLOT_INDEX_MAGDECL][0], VALLEN, "%+05dmil", (int)lround(DEG2MIL(magdecl)));
            snprintf(&this->value[SVPLOT_INDEX_GRIDVAR][0], VALLEN, "%+05dmil", (int)lround(DEG2MIL(gridvar)));
        }
        else
        {
            memset(&this->value[SVPLOT_INDEX_LATITUDE][0], 0x00, VALLEN);
            memset(&this->value[SVPLOT_INDEX_LONGITUDE][0], 0x00, VALLEN);
            memset(&this->value[SVPLOT_INDEX_ALTITUDE][0], 0x00, VALLEN);
            memset(&this->value[SVPLOT_INDEX_PDOP][0], 0x00, VALLEN);
            memset(&this->value[SVPLOT_INDEX_HDOP][0], 0x00, VALLEN);
            memset(&this->value[SVPLOT_INDEX_VDOP][0], 0x00, VALLEN);
            memset(&this->value[SVPLOT_INDEX_MAGDECL][0], 0x00, VALLEN);
            memset(&this->value[SVPLOT_INDEX_GRIDVAR][0], 0x00, VALLEN);
        }

        gnss->getNSU(gnss, &nsu);
        gnss->getNSV(gnss, &gps, &glns);
        snprintf(&this->value[SVPLOT_INDEX_NSV_GPS][0], VALLEN, "%d", gps);
        snprintf(&this->value[SVPLOT_INDEX_NSV_GLNS][0], VALLEN, "%d", glns);
        snprintf(&this->value[SVPLOT_INDEX_NSU][0], VALLEN, "%d", nsu);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null svplot widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_svplot_get_satinfo(struct gui_svplot_interface *svp, int *nsv_gps, int *nsv_glns, struct svcoord **gps, struct svcoord **glns)
{
    int az = 0;
    int el = 0;
    int snr = 0;
    int ret = 0;
    struct gnss_interface *gnss = NULL;
    struct gui_svplot_attribute *this = (struct gui_svplot_attribute *) svp;

    if (this)
    {
        gnss = this->gnss;
        gnss->getNSV(gnss, nsv_gps, nsv_glns);

        for (int i = 0; i < *nsv_gps; i++)
        {
            gnss->getSatAZ(gnss, GNSS_GPS, i, &az);
            gnss->getSatEL(gnss, GNSS_GPS, i, &el);
            gnss->getSatSNR(gnss, GNSS_GPS, i, &snr);
            gnss->getSatID(gnss, GNSS_GPS, i, &this->svcoord[i].id);

            if ((az == 0) && (el == 0))
                this->svcoord[i].snr = SVPLOT_SNR_NONE;
            else
            {
                this->svcoord[i].x = lround((90 - el) * cos(RADIANS(az - 90)));
                this->svcoord[i].y = lround((90 - el) * sin(RADIANS(az - 90)));
                this->svcoord[i].x += (this->position[0][1].x + this->position[0][1].w / 2);
                this->svcoord[i].y += (this->position[0][1].y + this->position[0][1].h / 2);

                if (snr < 10)
                    this->svcoord[i].snr = SVPLOT_SNR_POOR;
                else if (snr < 20)
                    this->svcoord[i].snr = SVPLOT_SNR_MODERATE;
                else if (snr < 30)
                    this->svcoord[i].snr = SVPLOT_SNR_GOOD;
                else
                    this->svcoord[i].snr = SVPLOT_SNR_EXCELENT;
            }
        }

        for (int i = 0; i < *nsv_glns; i++)
        {
            gnss->getSatAZ(gnss, GNSS_GLONASS, i, &az);
            gnss->getSatEL(gnss, GNSS_GLONASS, i, &el);
            gnss->getSatSNR(gnss, GNSS_GLONASS, i, &snr);
            gnss->getSatID(gnss, GNSS_GLONASS, i, &this->svcoord[32 * GNSS_GLONASS + i].id);

            if ((az == 0) && (el == 0))
                this->svcoord[i].snr = SVPLOT_SNR_NONE;
            else
            {
                this->svcoord[32 * GNSS_GLONASS + i].x = lround((90 - el) * cos(RADIANS(az - 90)));
                this->svcoord[32 * GNSS_GLONASS + i].y = lround((90 - el) * sin(RADIANS(az - 90)));
                this->svcoord[32 * GNSS_GLONASS + i].x += (this->position[0][1].x + this->position[0][1].w / 2);
                this->svcoord[32 * GNSS_GLONASS + i].y += (this->position[0][1].y + this->position[0][1].h / 2);

                if (snr < 10)
                    this->svcoord[32 * GNSS_GLONASS + i].snr = SVPLOT_SNR_POOR;
                else if (snr < 20)
                    this->svcoord[32 * GNSS_GLONASS + i].snr = SVPLOT_SNR_MODERATE;
                else if (snr < 30)
                    this->svcoord[32 * GNSS_GLONASS + i].snr = SVPLOT_SNR_GOOD;
                else
                    this->svcoord[32 * GNSS_GLONASS + i].snr = SVPLOT_SNR_EXCELENT;
            }
        }

        *gps = &this->svcoord[0];
        *glns = &this->svcoord[32];
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null svplot widget interface\n", DBGINFO));
    }

    return ret;
}


static char *
gui_svplot_get_value(struct gui_svplot_interface *svp, int idx, rect_t *rect)
{
    char *str = NULL;
    struct gui_svplot_attribute *this = (struct gui_svplot_attribute *) svp;

    if (this)
    {
        if ((SVPLOT_INDEX_BGIMG <= idx) && (SVPLOT_INDEX_GRIDVAR >= idx))
        {
            str = &(this->value[idx][0]);
            memcpy(rect, &this->position[idx][1], sizeof(rect_t));
        }
        else
            TLOGMSG(1, (DBGINFOFMT "invalid index\n", DBGINFO));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null svplot widget interface\n", DBGINFO));

    return str;
}


static char *
gui_svplot_get_tag(struct gui_svplot_interface *svp, int idx, rect_t *rect)
{
    char *str = NULL;
    struct gui_svplot_attribute *this = (struct gui_svplot_attribute *) svp;

    if (this)
    {
        if ((SVPLOT_INDEX_BGIMG <= idx) && (SVPLOT_INDEX_GRIDVAR >= idx))
        {
            str = &(this->tag[idx][0]);
            memcpy(rect, &this->position[idx][0], sizeof(rect_t));
        }
        else
            TLOGMSG(1, (DBGINFOFMT "invalid index\n", DBGINFO));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null svplot widget interface\n", DBGINFO));

    return str;
}


struct gui_svplot_interface *
gui_svplot_create(struct gnss_interface *gnss, struct taqmgr_interface *taqmgr)
{
    struct gui_svplot_interface *svp = NULL;
    struct gui_svplot_attribute *this = malloc(sizeof(struct gui_svplot_attribute));

    if (this)
    {
        memset(this, 0x00, sizeof(struct gui_svplot_attribute));
        snprintf(&this->tag[SVPLOT_INDEX_LATITUDE][0], 32, "%s", STR_LATITUDE);
        snprintf(&this->tag[SVPLOT_INDEX_LONGITUDE][0], 32, "%s", STR_LONGITUDE);
        snprintf(&this->tag[SVPLOT_INDEX_ALTITUDE][0], 32, "%s", STR_ALTITUDE);
        snprintf(&this->tag[SVPLOT_INDEX_NSV][0], 32, "%s", STR_NSV);
        snprintf(&this->tag[SVPLOT_INDEX_NSV_GPS][0], 32, "\u25CFGPS");
        snprintf(&this->tag[SVPLOT_INDEX_NSV_GLNS][0], 32, "\u25A0GLNS");
        snprintf(&this->tag[SVPLOT_INDEX_NSU][0], 32, "%s", STR_NSU);
        snprintf(&this->tag[SVPLOT_INDEX_PDOP][0], 32, "PDOP");
        snprintf(&this->tag[SVPLOT_INDEX_HDOP][0], 32, "HDOP");
        snprintf(&this->tag[SVPLOT_INDEX_VDOP][0], 32, "VDOP");
        snprintf(&this->tag[SVPLOT_INDEX_MAGDECL][0], 32, "%s", STR_MAGDECL);
        snprintf(&this->tag[SVPLOT_INDEX_GRIDVAR][0], 32, "%s", STR_GRIDVAR);
        memcpy(&this->position[0][0], &pos[0][0], sizeof(pos));
        this->gnss = gnss;
        this->taqmgr = taqmgr;

        svp = &(this->extif);
        svp->update = gui_svplot_update;
        svp->getTag = gui_svplot_get_tag;
        svp->getValue = gui_svplot_get_value;
        svp->getSatInfo = gui_svplot_get_satinfo;
        TLOGMSG(0, ("create svplot widget interface\n"));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "malloc return null\n" DBGINFO));

    return svp;
}


int
gui_svplot_destroy(struct gui_svplot_interface *svp)
{
    int ret = 0;
    struct gui_svplot_attribute *this = (struct gui_svplot_attribute *) svp;

    if (this)
    {
        free(this);
        TLOGMSG(0, ("destroy svplot widget interface\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null svplot widget interface\n", DBGINFO));
    }

    return ret;
}