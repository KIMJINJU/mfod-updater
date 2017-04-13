#ifndef _GEOMAG_H_
#define _GEOMAG_H_

#include <time.h>

/* constant macro defines - return code */
#define GEOMAG_OK				0
#define GEOMAG_ERR_LOAD_WMMCOF	-1
#define GEOMAG_ERR_ALLOC_MDLMEM	-2
#define GEOMAG_ERR_NULL_MODEL	-3
#define GEOMAG_ERR_INIT_GEOMAG	-4
#define GEOMAG_ERR_SET_DATE		-5
#define GEOMAG_ERR_SET_POSTION	-6

/* external functions */
extern int geomag_init(char *cof);
extern int geomag_deinit(void);
extern void geomag_set_date(time_t t);
extern void geomag_set_position(double phi, double lamda, double alt);
extern int geomag_calculate(void);
extern double geomag_get_magdecl(void);
extern double geomag_get_gridvar(void);


#endif /* _GEOMAG_H_ */
