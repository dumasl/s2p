
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <netcdf.h>
#include "netcdf.h"

double min(int * tab, int size) {
    double min = tab[0];
    for(int i=1; i<size; i++)
    {
        if(tab[i]<min)
            min = tab[i];
    }
    return min;
}

double max(int * tab, int size) {
    double max = tab[0];
    for(int i=1; i<size; i++)
    {
        if(tab[i]>max)
            max = tab[i];
    }
    return max;
}

void writeExtent(char * extent_path, int w, int h, int * coord_x, int * coord_y) {

    double xmin, xmax, ymin, ymax;
    int size = w * h;

    xmin = min(coord_x, size);
    xmax = max(coord_x, size);
    ymin = min(coord_y, size);
    ymax = max(coord_y, size);

    FILE *f = fopen(extent_path, "w");
    fprintf(f, "%lf %lf %lf %lf\n", xmin, ymin, xmax, ymax);
    fprintf(f, "%d %d %d %d", 0, 0, w, h);
    fclose(f);
}

// Create a netcdf file
int writeNetCDFFile(const char * netcdfFile, int width, int height, int *ecef, type_variables var)
{
   /* When we create netCDF variables and dimensions, we get back an
    * ID for each one. */
   char name[50];
   int nbr_variables = 3;
   int nbView = var.nb_sights;
   //fprintf(stdout, "nbView=%d\n", nbView);
   int ncid;
   int varid_ecef, varid[nbr_variables], varid_colors[var.nb_bands], varid_dir_view[2*nbView], varid_view[nbView-1];

   int i_dimid, j_dimid, dimids_ij[2];
   int x_dimid, y_dimid, z_dimid, dimids_xyz[3];

   // This is the data array we will write.
   int *coord_x = malloc(width*height*sizeof(double));
   int *coord_y = malloc(width*height*sizeof(double));
   //int *coord_z = malloc(width*height*sizeof(double));

   /* Create some pretend data. If this wasn't an example program, we
    * would have some real data to write, for example, model
    * output. */
   for(int y=0; y<height; y++) {
    for(int x=0; x<width; x++) {
        coord_x[x+width*y] = ecef[y*width+x];
        coord_y[x+width*y] = ecef[y*width+x+(width*height)];
        //coord_z[x+width*y] = ecef[y*width+x+(2*width*height)];
    }
   }

   /* Always check the return code of every netCDF function call.
    * Any retval which is not equal to NC_NOERR(0) will cause the program
    * to print an error message and exit with a non-zero return code. */
   int retval;

    // The NC_CLOBBER parameter tells netCDF to overwrite the file, if it already exists.
   if((retval = nc_create(netcdfFile, NC_NETCDF4, &ncid))) {
    fprintf(stderr, "Error: %s\n", nc_strerror(retval));
    exit(2);
   }

   // Define the dimensions. NetCDF will hand back an ID for each.
   if((retval = nc_def_dim(ncid, "i", width, &i_dimid))) {
    fprintf(stderr, "Error: %s\n", nc_strerror(retval));
    exit(2);
   }
   if((retval = nc_def_dim(ncid, "j", height, &j_dimid))) {
    fprintf(stderr, "Error: %s\n", nc_strerror(retval));
    exit(2);
   }

   if((retval = nc_def_dim(ncid, "x", 3, &x_dimid))) {
    fprintf(stderr, "Error: %s\n", nc_strerror(retval));
    exit(2);
   }
   if((retval = nc_def_dim(ncid, "y", width, &y_dimid))) {
    fprintf(stderr, "Error: %s\n", nc_strerror(retval));
    exit(2);
   }
   if((retval = nc_def_dim(ncid, "z", height, &z_dimid))) {
    fprintf(stderr, "Error: %s\n", nc_strerror(retval));
    exit(2);
   }

   // The dimids array is used to pass IDs of the dimensions of the variable
   dimids_ij[0] = i_dimid;
   dimids_ij[1] = j_dimid;
   dimids_xyz[0] = x_dimid;
   dimids_xyz[1] = y_dimid;
   dimids_xyz[2] = z_dimid;

   // Define the variables
   /*if((retval = nc_def_var(ncid, "coord_x", NC_INT, 2,
            dimids_ij, &varid[0]))) {
    fprintf(stderr, "Error: %s\n", nc_strerror(retval));
    exit(2);
   }
   if((retval = nc_def_var(ncid, "coord_y", NC_INT, 2,
            dimids_ij, &varid[1]))) {
    fprintf(stderr, "Error: %s\n", nc_strerror(retval));
    exit(2);
   }
   if((retval = nc_def_var(ncid, "coord_z", NC_INT, 2,
            dimids_ij, &varid[2]))) {
    fprintf(stderr, "Error: %s\n", nc_strerror(retval));
    exit(2);
   }*/
   for(int i=0; i<var.nb_bands; i++) {
    sprintf(name, "band_%d", i);
    if((retval = nc_def_var(ncid, name, NC_INT, 2,
            dimids_ij, &varid_colors[i]))) {
        fprintf(stderr, "Error: %s\n", i, nc_strerror(retval));
        exit(2);
    }
   }
   if((retval = nc_def_var(ncid, "nb_views", NC_INT, 2,
            dimids_ij, &varid[0]))) {
    fprintf(stderr, "Error: %s\n", nc_strerror(retval));
    exit(2);
   }
   if((retval = nc_def_var(ncid, "nodata", NC_INT, 2,
            dimids_ij, &varid[1]))) {
    fprintf(stderr, "Error: %s\n", nc_strerror(retval));
    exit(2);
   }
   if((retval = nc_def_var(ncid, "radius", NC_FLOAT, 2,
            dimids_ij, &varid[2]))) {
    fprintf(stderr, "Error: %s\n", nc_strerror(retval));
    exit(2);
   }
   for(int v=0; v<nbView-1; v++) {
        sprintf(name, "secondary_%d", (v+1));
        if((retval = nc_def_var(ncid, name, NC_INT, 2,
            dimids_ij, &varid_view[v]))) {
         fprintf(stderr, "Error: %s\n", nc_strerror(retval));
         exit(2);
        }
   }
   for(int v=0; v<nbView; v++) {
        sprintf(name, "dir_view_%d_vector", (v+1));
        if((retval = nc_def_var(ncid, name, NC_INT, 3,
            dimids_xyz, &varid_dir_view[v]))) {
        fprintf(stderr, "Error: %s\n", nc_strerror(retval));
        exit(2);
        }
        sprintf(name, "dir_view_%d_origin", (v+1));
        if((retval = nc_def_var(ncid, name, NC_INT, 3,
            dimids_xyz, &varid_dir_view[v+nbView]))) {
        fprintf(stderr, "Error: %s\n", nc_strerror(retval));
        exit(2);
        }
   }
   if((retval = nc_def_var(ncid, "coord_ECEF", NC_DOUBLE, 3,
            dimids_xyz, &varid_ecef))) {
    fprintf(stderr, "Error: %s\n", nc_strerror(retval));
    exit(2);
   }

   // End define mode. This tells netCDF we are done defining metadata.
   if((retval = nc_enddef(ncid))) {
    fprintf(stderr, "Error: %s\n", nc_strerror(retval));
    exit(2);
   }

   // Write the data into the file.
   /*if((retval = nc_put_var_int(ncid, varid[0], &coord_x[0]))) {
    fprintf(stderr, "Error: %s\n", nc_strerror(retval));
    exit(2);
   }
   if((retval = nc_put_var_int(ncid, varid[1], &coord_y[0]))) {
    fprintf(stderr, "Error: %s\n", nc_strerror(retval));
    exit(2);
   }
   if((retval = nc_put_var_int(ncid, varid[2], &coord_z[0]))) {
    fprintf(stderr, "Error: %s\n", nc_strerror(retval));
    exit(2);
   }*/
   int *bands = malloc(width*height*sizeof(int));
   for(int i=0; i<var.nb_bands; i++) {
    for(int y=0; y<height; y++) {
            for(int x=0; x<width; x++) {
                bands[x+width*y] = var.colors[width*var.nb_bands*y+var.nb_bands*x+i];
            }
        }
    if((retval = nc_put_var_int(ncid, varid_colors[i], &bands[0]))) {
        fprintf(stderr, "Error: %s\n", i, nc_strerror(retval));
        exit(2);
    }
   }
   if((retval = nc_put_var_int(ncid, varid[0], &var.nbViews[0]))) {
    fprintf(stderr, "Error: %s\n", nc_strerror(retval));
    exit(2);
   }
   if((retval = nc_put_var_int(ncid, varid[1], &var.nodata[0]))) {
    fprintf(stderr, "Error: %s\n", nc_strerror(retval));
    exit(2);
   }
   if((retval = nc_put_var_float(ncid, varid[2], &var.radiusMap[0]))) {
    fprintf(stderr, "Error: %s\n", nc_strerror(retval));
    exit(2);
   }
   int *view = malloc(width*height*sizeof(int));
   for(int v=0; v<nbView-1; v++) {
        for(int y=0; y<height; y++) {
            for(int x=0; x<width; x++) {
                view[x+width*y] = var.views[width*nbView*y+nbView*x+v];
            }
        }
        if((retval = nc_put_var_int(ncid, varid_view[v], &view[0]))) {
            fprintf(stderr, "Error: %s\n", nc_strerror(retval));
            exit(2);
        }
   }
   int ind=0;
   int *dir_view = malloc(3*width*height*sizeof(int));
   for(int v=0; v<nbView; v++) {
       // Vector
       ind = 0;
       for(int y=0; y<height; y++) {
          for(int x=0; x<width; x++) {
            dir_view[ind] = var.dir_view_x_vector[width*nbView*y+nbView*x+v];
            dir_view[ind+width*height] = var.dir_view_y_vector[width*nbView*y+nbView*x+v];
            dir_view[ind+2*width*height] = var.dir_view_z_vector[width*nbView*y+nbView*x+v];
            ind++;
          }
       }
       if((retval = nc_put_var_int(ncid, varid_dir_view[v], &dir_view[0]))) {
        fprintf(stderr, "Error: %s\n", nc_strerror(retval));
        exit(2);
       }

       // Origin
       ind=0;
       for(int y=0; y<height; y++) {
          for(int x=0; x<width; x++) {
            dir_view[ind] = var.dir_view_x_origin[width*nbView*y+nbView*x+v];
            dir_view[ind+width*height] = var.dir_view_y_origin[width*nbView*y+nbView*x+v];
            dir_view[ind+2*width*height] = var.dir_view_z_origin[width*nbView*y+nbView*x+v];
            ind++;
          }
       }
       if((retval = nc_put_var_int(ncid, varid_dir_view[v+nbView], &dir_view[0]))) {
        fprintf(stderr, "Error: %s\n", nc_strerror(retval));
        exit(2);
       }
   }

   if((retval = nc_put_var_int(ncid, varid_ecef, &ecef[0]))) {
    fprintf(stderr, "Error: %s\n", nc_strerror(retval));
    exit(2);
   }

   // Close the file
   if((retval = nc_close(ncid))) {
    fprintf(stderr, "Error: %s\n", nc_strerror(retval));
    exit(2);
   }

   // Write extent into a text file
   const char *outdir = dirname(strdup(netcdfFile));
   char buf[1000];
   sprintf(buf, "%s/extent.txt", outdir);
   writeExtent(buf, width, height, coord_x, coord_y);

   free(coord_x);
   free(coord_y);
   //free(coord_z);
   free(view);
   free(dir_view);

   return 0;
}

