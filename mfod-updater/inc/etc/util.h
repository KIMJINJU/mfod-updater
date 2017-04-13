
#ifndef _UTIL_H_
#define _UTIL_H_

#include <sys/time.h>
#include "types.h"

/* constant macro defines */
#define PI              3.141592654

/* function macro defines */
#define MEMBERSIZE(type, member)    sizeof(((type *)0)->member)
#define DIM(x)                      (int)(sizeof(x) / sizeof(x[0]))
#define MSLEEP(t)                   usleep(1000 * t)
#define MIL2DEG(mil)	            0.056250000 * mil
#define DEG2MIL(degree)	            17.77777778 * degree
#define RADIANS(degrees)	        degrees * 0.017453293
#define DEGREES(radians)	        radians * 57.295779506
#define ROUND(x)				((x) >= 0 ? (long)((x*1000.0)+0.5) : (long)((x*1000.0)-0.5))


/* external functions */
void set_utc_offset(int offset);
extern int get_utc_offset(void);
extern void set_time_offset(time_t);
extern time_t get_time_offset(void);
extern int check_timeout(struct timespec *tref, double timeo);
extern double get_elapsed_time(struct timespec *t1, struct timespec *t2);
extern char calculate_checksum(const char *msg, int length);
extern unsigned short crc16_ccitt(const void *buf, int len);
extern unsigned int crc32(const void *buf, unsigned int size);
extern int get_lastday(int year, int month);
extern double dms2deg(int deg, int min, double sec);
extern void deg2dms(double decdeg, int *deg, int *min, double *sec);
extern double compensate_fwdaz(double az, double magdecl, double gridvar);

#endif /* _UTIL_H_*/
