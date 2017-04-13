
#include "types.h"
#include "core/taqmgr.h"
#include "modules/gnss.h"

/* constant macro define */
#define SVPLOT_INDEX_BGIMG          0x00
#define SVPLOT_INDEX_LATITUDE       0x01
#define SVPLOT_INDEX_LONGITUDE      0x02
#define SVPLOT_INDEX_ALTITUDE       0x03
#define SVPLOT_INDEX_NSV            0x04
#define SVPLOT_INDEX_NSV_GPS        0x05
#define SVPLOT_INDEX_NSV_GLNS       0x06
#define SVPLOT_INDEX_NSU            0x07
#define SVPLOT_INDEX_PDOP           0x08
#define SVPLOT_INDEX_HDOP           0x09
#define SVPLOT_INDEX_VDOP           0x0A
#define SVPLOT_INDEX_MAGDECL        0x0B
#define SVPLOT_INDEX_GRIDVAR        0x0C
#define SVPLOT_GNSS_GPS             0
#define SVPLOT_GNSS_GLONASS         1
#define SVPLOT_SNR_EXCELENT         0
#define SVPLOT_SNR_GOOD             1
#define SVPLOT_SNR_MODERATE         2
#define SVPLOT_SNR_POOR             3
#define SVPLOT_SNR_NONE             4


struct svcoord
{
    int x;
    int y;
    int snr;
    int id;
};


/* type defines : svplot interface */
typedef struct gui_svplot_interface
{
    int  (*update)     (struct gui_svplot_interface *);
    int  (*getSatInfo) (struct gui_svplot_interface *, int *, int *, struct svcoord **, struct svcoord **);
    char *(*getValue)  (struct gui_svplot_interface *, int, rect_t *);
    char *(*getTag)    (struct gui_svplot_interface *, int, rect_t *);

#ifdef  _ENABLE_SVPLOT_INTERNAL_INTERFACE_

#endif /* _ENABLE_SVPLOT_INTERNAL_INTERFACE_ */
}
svplot_t;


/* external functions for create/destroy svplot widget interface */
extern struct gui_svplot_interface *gui_svplot_create(struct gnss_interface *, struct taqmgr_interface *);
extern int gui_svplot_destroy(struct gui_svplot_interface *);
