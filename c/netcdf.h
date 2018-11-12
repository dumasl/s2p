#ifndef _NETCDF_H
#define _NETCDF_H
#include <inttypes.h>

typedef struct
{
  int *nbViews;
  int *views;
  int *nodata;

  int nb_sights;
  int nb_bands;

  int *colors;

  // origin
  int *dir_view_x_origin;
  int *dir_view_y_origin;
  int *dir_view_z_origin;
  // vector
  int *dir_view_x_vector;
  int *dir_view_y_vector;
  int *dir_view_z_vector;

  // radius of sigths intersection
  float *radiusMap;

} type_variables;

int writeNetCDFFile(const char * outputfilename, int width, int height, int *ecef, type_variables var);

#endif // _NETCDF_H