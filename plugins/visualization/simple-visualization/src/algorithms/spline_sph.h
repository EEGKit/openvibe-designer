/*************************************************************************************/
/* spline_sph.h                                                                      */
/*************************************************************************************/

/*************************************************************************************/
/* spline  subroutines                                                               */
/*************************************************************************************/
#pragma once

int spline_tables(const int order, double* pot_table, double* scd_table);
int spline_coef(const int nb_value, double** xyz, const double* values, const double* table, double* coef);
double spline_interp(const int nb_value, double** xyz, const double* table, const double* coef, const double xx, const double yy, const double zz);
