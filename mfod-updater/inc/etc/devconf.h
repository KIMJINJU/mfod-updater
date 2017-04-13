#ifndef _DEVCONF_H_
#define _DEVCONF_H_

#define DEVCONF_FILE_PATH       "/mnt/mmc/mfod-data/devparm.conf"

#define DEVCONF_KEY_SRCCA       "srccA"
#define DEVCONF_KEY_SRCCB       "srccB"
#define DEVCONF_KEY_MRCCA       "mrccA"
#define DEVCONF_KEY_MRCCB       "mrccB"
#define DEVCONF_KEY_LRCCA       "lrccA"
#define DEVCONF_KEY_LRCCB       "lrccB"
#define DEVCONF_KEY_AZOFF       "azoff"
#define DEVCONF_KEY_ELOFF       "eloff"
#define DEVCONF_KEY_IRHSHIFT    "irhshift"
#define DEVCONF_KEY_IRVSHIFT    "irvshift"
#define DEVCONF_KEY_IRUPSCALE   "irupscale"
#define DEVCONF_KEY_PROXOPEN    "proxopen"
#define DEVCONF_KEY_PROXCLOSE   "proxclose"
#define DEVCONF_KEY_COORDSYS    "coordsys"

#define DEVCONF_VALUE_LENGTH    32

#define DEVCONF_COORDSYS_GEODETIC   0
#define DEVCONF_COORDSYS_UTM        1
#define DEVCONF_COORDSYS_MGRS       2

#define DEVCONF_IRUPSCALE_OFF       0
#define DEVCONF_IRUPSCALE_ON        1

/* external functions */
extern void devconf_enumerate_parameters(void);
extern void devconf_reset_parameters(void);
extern int devconf_load_parameters(char *path);
extern int devconf_save_parameters(char *path);
extern int devconf_set_value(char *key, char *value);
extern char *devconf_get_value(char *key);

#endif /* _DEVCONF_H */