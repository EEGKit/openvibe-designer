/*************************************************************************************/
/* spline_sph.h                                                                      */
/*************************************************************************************/

/*************************************************************************************/
/* spline  subroutines                                                               */
/*************************************************************************************/
#pragma once

int spline_tables(int order, double* pot_table, double* scd_table);
int spline_coef(int nb_value, double** xyz, const double* values, const double* table, double* coef);
double spline_interp(int nb_value, double** xyz, const double* table, const double* coef, double xx, double yy, double zz);
