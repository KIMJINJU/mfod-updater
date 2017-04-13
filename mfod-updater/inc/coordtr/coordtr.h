#ifndef _COORDTR_H_
#define _COORDTR_H_


/* external functions prototype */
extern int coordtr_geodetic2mgrs (double latitude, double longitude, char *mgrs, char *utm);
extern int coordtr_geodetic2utm (double latitude, double longitude, char *utm);
extern int coordtr_mgrs2geodetic(char *mgrs, double *latitude, double *longitude);

#endif /* _COORDTR_H_ */
