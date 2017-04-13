/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    geodesic.h
        external function/variables/defines for geodesics
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#ifndef _GEODESIC_H_
#define _GEODESIC_H_


/* type defines */
typedef struct geodetic_coordinate
{
	double latitude;
	double longitude;
	double altitude;
} geod_coord_t ;


/* external functions */
extern int geodesic_solve_harvesine_formula(const geod_coord_t *org, geod_coord_t *dst, double dist, double fwdaz, double fwdel);
extern int geodesic_solve_direct_problem(const geod_coord_t *org, geod_coord_t *dst, double dist, double fwdaz, double fwdel);
extern int geodesic_solve_inverse_problem(const geod_coord_t *org, const geod_coord_t *dst, double *alpha_1, double *alpha_2, double *distance);
extern int geodesic_solve_midpoint_problem(const geod_coord_t *org, const geod_coord_t *dst, geod_coord_t *mid);
extern double geodesic_get_gridcvg(double latitude, double longitude);

#endif /* _GEODESIC_H_ */

