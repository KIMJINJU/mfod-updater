/*
	Copyright (C) 2015-2016 EOSYSTEM CO., LTD. (www.eosystem.com)

  	geodesic.c
  		geodesic functions for calculate target location, etc.
  		This file is part of amod-mainapp.

  	Written by
  		Seung-hwan, Park (seunghwan.park@me.com)
 */

#include <math.h>

#include "core/geodesic.h"
#include "core/logger.h"
#include "etc/util.h"

#define POLAR_RADIOUS		6356.7523142	/* polar radious of the Earth defined WGS84 (Kilometers)	  		*/
#define MEAN_RADIOUS		6371008.7714	/* mean radious of Earth (meters)							  		*/
#define EQUATORIAL_RADIOUS	6378.1370 		/* equatorial radious of the Earth defined WGS84 (Kilometers) 		*/
#define INVERSE_FLATTENING	298.257223563	/* inverse flattening of the Earth defined WGS84		  	  		*/
#define FLATTENING			0.003352810		/* flattening of the ellipsoid										*/
#define SEMI_MAJOR_AXIS		6378137.000000	/* length of semi-major axis of the ellipsoid (radius at equator) 	*/
#define SEMI_MINOR_AXIS		6356752.314140 	/* length of semi-minor axis of the ellipsoid (radius at pole)    	*/

#define DEGREE_OF_ACCURACY	1e-12
#define MAX_ITERATIONS		200

#define MAX_ALTITUDE		19975
#define MIN_ALTITUDE		-402


int
geodesic_solve_harvesine_formula(const geod_coord_t *org, geod_coord_t *dst, double dist, double fwdaz, double fwdel)
{
	int ret = 0;
	double hdist = 0.0;

	if (org && dst)
	{
		hdist = dist * cos(RADIANS(fwdel));

		dst->latitude =
			DEGREES(asin(sin(RADIANS(org->latitude)) * cos(hdist / MEAN_RADIOUS) +
			cos(RADIANS(org->latitude)) * sin(hdist / MEAN_RADIOUS) * cos(RADIANS(fwdaz))));

		dst->longitude =
			org->longitude + DEGREES(atan2(sin(RADIANS(fwdaz)) * sin(hdist / MEAN_RADIOUS) * cos(RADIANS(org->latitude)),
			cos(hdist / MEAN_RADIOUS) - sin(RADIANS(org->latitude)) * sin(RADIANS(dst->latitude))));

		dst->altitude = org->altitude + (dist * sin(RADIANS(fwdel)));

		if (dst->altitude > MAX_ALTITUDE)
		{
			dst->altitude = MAX_ALTITUDE;
			TLOGMSG(1, ("altitude set to MAX_ALTITUDE (%d)\n", MAX_ALTITUDE));
		}
		else if (dst->altitude < MIN_ALTITUDE)
		{
			dst->altitude = MIN_ALTITUDE;
			TLOGMSG(1, ("altitude set to MIN_ALTITUDE (%d)\n", MIN_ALTITUDE));
		}

		TLOGMSG(1, ("solution of harvesine formula\n"));
		TLOGMSG(1, ("org = (%f, %f), distance = %fm, fwdaz = %f\n", org->latitude, org->longitude, dist, DEG2MIL(fwdaz)));
		TLOGMSG(1, ("dst = (%f, %f, %fm)\n", dst->latitude, dst->longitude, dst->altitude));
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null geodetic coordinate\n", DBGINFO));
	}

	return ret;
}


int
geodesic_solve_direct_problem(const geod_coord_t *org, geod_coord_t *dst, double dist, double fwdaz, double fwdel)
{
	int ret = 0;
	int num_iterations = 0;

	double temp = 0.0;
	double phi = 0.0;
	double lamda = 0.0;
	double lamda1 = RADIANS(org->longitude);
	double lamda2 = 0.0;
	double a = SEMI_MAJOR_AXIS;
	double b = SEMI_MINOR_AXIS;
	double sinAlpha1 = sin(RADIANS(fwdaz));
	double cosAlpha1 = cos(RADIANS(fwdaz));
	double tanU1 = (1.0 - FLATTENING) * tan(RADIANS(org->latitude));
	double cosU1 = 1.0 / sqrt((1 + tanU1 * tanU1));
	double sinU1 = tanU1 * cosU1;
	double sigma1 = atan2(tanU1, cosAlpha1);
	double sinAlpha = cosU1 * sinAlpha1;
	double cosSqAlpha = 1.0 - (sinAlpha * sinAlpha);
	double uSq = cosSqAlpha * (a * a - b * b) / (b * b);
	double A = 1.0 + uSq / 16384.0 * (4096.0 + uSq * (-768.0 + uSq * (320.0 - 175.0 * uSq )));
	double B = uSq / 1024.0 * (256.0 + uSq * (-128.0 + uSq * (74.0 - 47.0 * uSq)));
	double sigma = dist / (b * A);
	double sigmaPrime = 0.0;
	double cos2SigmaM = 0.0;
	double sinSigma = 0.0;
	double cosSigma = 0.0;
	double deltaSigma = 0.0;
	double C = 0.0;
	double L = 0.0;

	do
	{
		cos2SigmaM = cos(RADIANS(2 * sigma1 + sigma));
		sinSigma = sin(RADIANS(sigma));
		cosSigma = cos(RADIANS(sigma));
		deltaSigma = B * sinSigma *(cos2SigmaM + B / 4.0 * (cosSigma *( -1.0 + 2 * cos2SigmaM * cos2SigmaM) -
			B / 6.0 * cos2SigmaM * (-3.0 + 4.0 *sinSigma * sinSigma) * (-3.0 + 4.0 * cos2SigmaM * cos2SigmaM)));

		sigmaPrime = sigma;
		sigma = dist / (b * A) + deltaSigma;
	}
	while((fabs(sigma - sigmaPrime) > DEGREE_OF_ACCURACY) && (++num_iterations < MAX_ITERATIONS));

	if (num_iterations >= MAX_ITERATIONS)
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "direct problem failed to converge!\n", DBGINFO));
	}
	else
	{
		temp = sinU1 * sinSigma - cosU1 * cosSigma * cosAlpha1;
		phi = atan2(sinU1 * cosSigma + cosU1 * sinSigma * cosAlpha1, (1.0 - FLATTENING) * sqrt(sinAlpha * sinAlpha + temp * temp));
		lamda = atan2(sinSigma * sinAlpha1, cosU1 * cosSigma - sinU1 * sinSigma * cosAlpha1);
		C = FLATTENING / 16.0 * cosSqAlpha * (4.0 + FLATTENING * (4.0 - 3.0 * cosSqAlpha));
		L = lamda - (1.0 - C ) * FLATTENING * sinAlpha * (sigma + C * sinSigma *
			(cos2SigmaM + C * cosSigma * (-1.0 + 2.0 * cos2SigmaM * cos2SigmaM)));

		lamda2 = fmod(lamda1 + L + 3.0 * PI, 2 * PI) - PI;
		dst->latitude = DEGREES(phi);
		dst->longitude = DEGREES(lamda2);
		dst->altitude = org->altitude + (dist * sin(RADIANS(fwdel)));

		TLOGMSG(1, ("vincenty solution of direct problem\n"));
		TLOGMSG(1, ("num_iterations = %d\n", num_iterations));
		TLOGMSG(1, ("org = (%f, %f), distance = %fm, fwdaz = %f\n", org->latitude, org->longitude, dist, fwdaz));
		TLOGMSG(1, ("dst = (%f, %f, %fm)\n", dst->latitude, dst->longitude, dst->altitude));
	}

	return ret;
}


int
geodesic_solve_inverse_problem(const geod_coord_t *org, const geod_coord_t *dst, double *alpha_1, double *alpha_2, double *distance)
{
	int num_iterations = 0;

	double a = 0.0;
	double b = 0.0;
	double c = 0.0;
	double sin_u1 = 0.0;
	double sin_u2 = 0.0;
	double cos_u1 = 0.0;
	double cos_u2 = 0.0;
	double tan_u1 = 0.0;
	double tan_u2 = 0.0;
	double sin_alpha = 0.0;
	double cos_alpha_sq = 0.0;
	double lamda = 0.0;
	double delta_lamda = 0.0;
	double lamda_prime = 0.0;
	double sin_lamda = 0.0;
	double cos_lamda = 0.0;
	double sigma = 0.0;
	double delta_sigma = 0.0;
	double sin_sigma = 0.0;
	double cos_sigma = 0.0;
	double cos_2sigma_m = 0.0;
	double u_sq = 0.0;
	double phi_1 = org->latitude;
	double lamda_1 = org->longitude;
	double phi_2 = dst->latitude;
	double lamda_2 = dst->longitude;

	tan_u1 = (1.0 - FLATTENING) * tan(RADIANS(phi_1));
	cos_u1 = 1.0 / (sqrt(1 + pow(tan_u1, 2)));
	sin_u1 = tan_u1 * cos_u1;
	tan_u2 = (1.0 - FLATTENING) * tan(RADIANS(phi_2));
	cos_u2 = 1.0 / (sqrt(1 + pow(tan_u2, 2)));
	sin_u2 = tan_u2 * cos_u2;

	delta_lamda = RADIANS(lamda_2) - RADIANS(lamda_1);
	lamda = delta_lamda;

	do
	{
		sin_lamda = sin(lamda);
		cos_lamda = cos(lamda);
		sin_sigma = sqrt(pow(cos_u2 * sin_lamda, 2) + pow((cos_u1 * sin_u2 - sin_u1 * cos_u2 * cos_lamda), 2));

		if (fpclassify(sin_sigma) == FP_ZERO)	/* if (sin_sigma == 0.0) */
		{
			TLOGMSG(1, (DBGINFOFMT "failed to solve inverse problem : co-incident points\n", DBGINFO));
			return -1;
		}

		cos_sigma = sin_u1 * sin_u2 + cos_u1 * cos_u2 * cos_lamda;
		sigma = atan2(sin_sigma, cos_sigma);
		sin_alpha = cos_u1 * cos_u2 * sin_lamda / sin_sigma;
		cos_alpha_sq = 1.0 - pow(sin_alpha, 2);
		cos_2sigma_m = cos_sigma - 2.0 * sin_u1 * sin_u2 / cos_alpha_sq;

		if (isnan(cos_2sigma_m) != 0)
			cos_2sigma_m = 0.0;

		c = FLATTENING / 16.0 * cos_alpha_sq * (4.0 + FLATTENING * (4.0 - 3.0 * cos_alpha_sq));
		lamda_prime = lamda;
		lamda = delta_lamda + (1 - c) * FLATTENING * sin_alpha * (sigma + c * sin_sigma *
				(cos_2sigma_m + c * cos_sigma * (-1.0 + 2.0 * pow(cos_2sigma_m, 2))));

	}
	while ((fabs(lamda - lamda_prime) > DEGREE_OF_ACCURACY) && (++num_iterations < MAX_ITERATIONS));

	if (num_iterations >= MAX_ITERATIONS)
	{
		TLOGMSG(1, (DBGINFOFMT "failed to solve inverse problem : failed to converge\n", DBGINFO));
		return -1;
	}
	else
	{
		u_sq = cos_alpha_sq * (pow(SEMI_MAJOR_AXIS, 2) - pow(SEMI_MINOR_AXIS, 2)) / pow(SEMI_MINOR_AXIS, 2);
		a = 1.0 + u_sq / 16384.0 * (4096.0 + u_sq * (-768.0 + u_sq * (320.0 - 175.0 * u_sq)));
		b = u_sq / 1024.0 * (256.0 + u_sq * (-128.0 + u_sq * (74.0 - 47.0 * u_sq)));

		delta_sigma = b * sin_sigma * (cos_2sigma_m + b / 4.0 * (cos_sigma * (-1.0 + 2.0 * pow(cos_2sigma_m, 2)) -
					 b / 6.0 * cos_2sigma_m * (-3.0 + 4.0 * pow(sin_sigma, 2)) * (-3.0 + 4.0 * pow(cos_2sigma_m, 2))));

		*distance = SEMI_MINOR_AXIS * a * (sigma - delta_sigma);

		*alpha_1 = atan2(cos_u2 * sin_lamda, cos_u1 * sin_u2 - sin_u1 * cos_u2 * cos_lamda);
		*alpha_2 = atan2(cos_u1 * sin_lamda, -sin_u1 * cos_u2 + cos_u1 *sin_u2 * cos_lamda);
		*alpha_1 = DEGREES(*alpha_1);
		*alpha_2 = DEGREES(*alpha_2);

		if (*alpha_1 < 0.0)
			*alpha_1 = *alpha_1 + 360.0;

		if (*alpha_2 < 0.0)
			*alpha_2 = *alpha_2 + 360.0;

		TLOGMSG(1, ("vincenty solution of inverse problem\n"));
		TLOGMSG(1, ("num_iterations = %d\n", num_iterations));
		TLOGMSG(1, ("org = (%f, %f)\n", org->latitude, org->longitude));
		TLOGMSG(1, ("dst = (%f, %f)\n", dst->latitude, dst->longitude));
		TLOGMSG(1, ("distance between org and dst = %fm\n", *distance));
		TLOGMSG(1, ("initail bearing = %f\n", *alpha_1));
		TLOGMSG(1, ("final bearing = %f\n", *alpha_2));
	}

	return 0;
}


int
geodesic_solve_midpoint_problem(const geod_coord_t *org, const geod_coord_t *dst, geod_coord_t *mid)
{
	int ret = 0;
	double initial_bearing = 0.0;
	double final_bearing = 0.0;
	double distance = 0.0;

	if (org && dst && mid)
	{
		if (geodesic_solve_inverse_problem(org, dst, &initial_bearing, &final_bearing, &distance) == 0)
		{
			if (geodesic_solve_direct_problem(org, mid, distance / 2.0, initial_bearing, 0.0) == 0)
			{
				TLOGMSG(1, ("midpoint = (%f, %f)\n", mid->latitude, mid->longitude));
				TLOGMSG(1, ("distance between org and mid = %f\n", distance / 2.0));
			}
			else
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "failed to solve midpoint problem : failed to get direct solution\n", DBGINFO));
			}
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "failed to solve midpoint problem : failed to get inverse solution\n", DBGINFO));
		}
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null geodetic coordinate\n", DBGINFO));
	}

	return ret;
}


double
geodesic_get_gridcvg(double latitude, double longitude)
{
	double cm = 0;

	if (longitude > 0.0)
		cm = (longitude - fmod(longitude, 6.0)) + 3.0;
	else
		cm = (longitude - fmod(longitude, 6.0)) - 3.0;
	
	return ((longitude - cm) * sin(RADIANS(latitude)));
}