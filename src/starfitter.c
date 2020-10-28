/* 

  Starfit.c - Stan Thomas 08-16-2011

  Routine for photographic surveying.

----------------------------------------------------------------------------
  
----------------------------------------------------------------------------

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "starfitter.h"
#include "nrutil.h"

void print_chi2(float x[]);
int reject_star(float x[]);

VECTOR s[NSTARS];
double xmeas[NSTARS], ymeas[NSTARS];
double sigmax[NSTARS], sigmay[NSTARS];
char sname[NSTARS][12];

const int MAXMIR[NSITES] = { 28, 11, 11,8}; 
const int MINMIR[NSITES] = { 1, 0, 0,1};
#ifdef TAx4
const double CUTX[NSITES] = {12.691, 19.0e0, 19.0e0, 12.691};  // Cut on x position
const double CUTY[NSITES] = {11.174, 16.0e0, 16.0e0, 11.174};  // Cut on y position
#else
const double CUTX[NSITES] = {11.5e0, 19.0e0, 19.0e0, 11.5e0};  // Cut on x position
const double CUTY[NSITES] = {10.7e0, 16.0e0, 16.0e0,10.7e0 };  // Cut on y position
#endif
const double CUTD = 0.5e0;  // Absolute cut on initial error          

double azi[NSITES][NMIRS];  // Cluster azimuth angle
double elev[NSITES][NMIRS]; // Cluster elevation angle
double rot[NSITES][NMIRS];  // Cluster rotation angle
double mcd[NSITES][NMIRS];  // Mirror-cluster distances (meters)
double Rmir[NSITES][NMIRS]; // Mirror radius of curvature (meters)

int Nstars;

int main(int argc, char *argv[])
{
  FILE *fp, *op;
  double parms[NPARMS];
  int i;
  double stheta, sphi;
  double xstar, ystar;
  double theta, phi, psi, x0, y0, D0;
  float x[NSTARS+1];
  double chi2min;
  char buffer[1025];
  int site, mir, year, month, day, hour, minutes, exposure;
  double seconds, delta;
  int iter;
  int Nstars0 = 0;

  srand48(time(NULL));
  //check if too few arguments were given
  if(argc<2)
    {
      fprintf(stderr,"Usage: %s <*.stars.txt>\7\7\n", argv[0]);
      exit(-1);
    }
  //open the given .stars.txt file
  if((fp=fopen(argv[1],"r"))==NULL)
    {
      fprintf(stderr,"Unable to open %s\7\7\n", argv[1]);
      exit(-1);
    }
  //open the mirror_geometry.dat file
  if((op=fopen("mirror_geometry.dat","r"))==NULL)
    {
      fprintf(stderr,"Unable to open mirror_geometry.dat\7\7\n");
      exit(-1);
    }
  //read in the mirror_geometry.dat file
  while(fgets(buffer,1023,op)!=NULL)
    {
      //read in the site and mirror numbers
      if(sscanf(buffer,"%d %d", &site, &mir) != 2)
	{
	  fprintf(stderr,"Error reading mirror_geometry.dat\7\7\n");
	  exit(-1);
	}
      //read in mirror information
      if(sscanf(buffer,"%*d %*d %lf %lf %lf %lf %lf", &azi[site][mir], &elev[site][mir], 
		&rot[site][mir], &mcd[site][mir], &Rmir[site][mir])!=5)
	{
	  fprintf(stderr,"Error reading mirror_geometry.dat\7\7\n");
	  exit(-1);
	}
    }

  fclose(op);
  //correct the azimuth angle
  for(site=0;site<NSITES;site++)
    {
      for(mir=0;mir<NMIRS;mir++)
        {
          azi[site][mir] = 90.0e0 - azi[site][mir];
          if(azi[site][mir] < 0.0e0)azi[site][mir] += 360.0e0;
        }
    }
  //read in the .stars.txt file
  Nstars = 0;
  while(fgets(buffer,1023,fp)!=NULL)
    {
      //read in star information
      if(sscanf(buffer," %d %d %d/%d/%d %d:%d:%lf %d %s %lf %lf %lf %lf %lf %lf %lf",
		&site, &mir, 
		&year, &month, &day, &hour, &minutes, &seconds, &exposure,
		sname[Nstars],
		&stheta, &sphi, 
		&xstar, &ystar,
		&xmeas[Nstars], &ymeas[Nstars],
		&delta) != 17)continue;
      
      site = site-1;
      //check if site number is wrong
      if((site<0)||(site>NSITES))
	{
	  fprintf(stderr,"Invalid site number\7\7\n");
	  exit(-1);
	}
      //check if mirror number is wrong
      if((mir<MINMIR[site])||(mir>MAXMIR[site]))
	{
	  fprintf(stderr,"Invalid mirror number\7\7\n");
	  exit(-1);
	}
      //convert angles to radians
      stheta *= D2R;
      sphi *= D2R;
      //compute star normal vector in east/north/up basis
      s[Nstars].e = cos(stheta)*cos(sphi);
      s[Nstars].n = cos(stheta)*sin(sphi);
      s[Nstars].u = sin(stheta);
      
      sigmax[Nstars] = 0.081e0;
      sigmay[Nstars] = 0.081e0;

      // Apply quality cuts to throw out stars that are likely to have been poorly measured.
      // This includes stars that are too close to the edge of the cameras. If part of the 
      // spot is cut off at the edge of the mirror it will cause the centroid to be mis-measured.
      // These cuts were determined by looking at the residuals in the data.
      // The cut on delta was determined by looking at the delta distribution for all stars and
      // all mirrors. Hopefully, this will help to throw out stars where the centroid was mis-measured
      // because they are partially darkened by Tom's wooden support bars.

      if(fabs(xstar)>CUTX[site])continue;
      if(fabs(ystar)>CUTY[site])continue;
      if(fabs(xmeas[Nstars])>CUTX[site])continue;
      if(fabs(ymeas[Nstars])>CUTY[site])continue;
      if(fabs(delta)>CUTD)continue;

      Nstars++;
    }

  fclose(fp);
  //check if there are enough stars to preform a fit
  if(Nstars<NPARMS+1)
    {
      fprintf(stderr,"Insufficient stars N = %d found.\7\7\n", Nstars);
      exit(-1);
    }
  //set up free parameters and fixed parameters
  parms[0] = theta = elev[site][mir]*D2R;
  parms[1] = phi   = azi[site][mir]*D2R;
  parms[2] = psi   = drand48()-0.5e0;
  x0 = 0.0e0; //parms[3] = x0    = drand48()-0.5e0;
  y0 = 0.0e0; //parms[4] = y0    = drand48()-0.5e0;
  parms[3] = D0 = (Rmir[site][mir] - mcd[site][mir])*100.0e0/2.54e0;

  for(i=0;i<NPARMS;i++)x[i+1] = (float)parms[i]; //save fit parameters

  for(iter=0;iter<100;iter++)
    {
      Nstars0 = Nstars;
      //check if there are enough stars to run the fit
      if(Nstars<NPARMS+1)
	{
	  fprintf(stderr,"Insufficient stars N = %d found.\7\7\n", Nstars);
	  exit(-1);
	}
      //construct the chi2 function and minimize
      chi2min = fit_func(NPARMS, parms, chi2_func);
      //printf("chi2min = %.03f\n",chi2min);
      for(i=0;i<NPARMS;i++)x[i+1] = (float)parms[i]; //save minimum parameters
      //check if any stars were rejected. If so repeat.
      if(Nstars0 == reject_star(x))break;
    }
  //run mimimization a few more time with only the good stars
  for(iter=0;iter<4;iter++)
    {
      if(Nstars<NPARMS+1)
	{
	  fprintf(stderr,"Insufficient stars N = %d found.\7\7\n", Nstars);
	  exit(-1);
	}

      chi2min = fit_func(NPARMS, parms, chi2_func);
      //printf("chi2min = %.03f\n",chi2min);
      for(i=0;i<NPARMS;i++)x[i+1] = (float)parms[i];
    }

  //convert results to degrees
  theta = parms[0]*R2D;
  phi   = 90.0e0 - parms[1]*R2D;
  if(phi<0.0e0)phi += 360.0e0;
  psi   = parms[2]*R2D;
  x0 = 0.0e0; //x0    = parms[3];
  y0 = 0.0e0; //y0    = parms[4];
  D0    = parms[3];
  //save final updated parameters
  for(i=0;i<NPARMS;i++)x[i+1] = (float)parms[i];
  //print the minimized chi2 value
  print_chi2(x);
  //print out the fit results
  printf("\n\n%d %d ", site, mir);
  printf("%.3lf ", phi);
  printf("%.3lf ", theta);
  printf("%.3lf ", psi);
  //  printf("%.3lf ", x0);
  //  printf("%.3lf ", y0);
  printf("%.3lf ", mcd[site][mir]);
  printf("%.3lf ", mcd[site][mir]+D0*2.54e0/100.0e0);
  printf("%d ", Nstars);
  printf("%.8lf\n", chi2min);
  fflush(stdout);

  return 0;
}

int reject_star(float x[])
{
  double derr, xstar, ystar;
  int i, itmp, ierr = (-1);
  double maxderr = 0.0e0;
  //if too few stars don't do anything
  if(Nstars<NPARMS+1)return Nstars;
  //find worst error star
  for(i=0;i<Nstars;i++)
    {
      //find star position
      starpos(i, &xstar, &ystar, x);
      //compute error
      derr  = pow(xmeas[i] - xstar,2.0e0);
      derr += pow(ymeas[i] - ystar,2.0e0);
      derr = sqrt(derr);
      //record error if bigger then last biggest error
      if(derr>maxderr)
	{
	  maxderr = derr;
	  ierr = i;
	}
    }

  // Reject the worst star if the error is greater than 0.235"

  if(maxderr>0.235e0)
    {
      //replace rejected star with the star at the end of the list.  Next run will check up to one less then the number checked previously.
      xmeas[ierr] = xmeas[Nstars-1];
      ymeas[ierr] = ymeas[Nstars-1];
      s[ierr].e = s[Nstars-1].e;
      s[ierr].n = s[Nstars-1].n;
      s[ierr].u = s[Nstars-1].u;
      sigmax[ierr] = sigmax[Nstars-1];
      sigmay[ierr] = sigmay[Nstars-1];
      strcpy(sname[ierr], sname[Nstars-1]);
      Nstars--;
      fprintf(stderr,"Rejected star\n");
    }

  return Nstars;
}

void print_chi2(float x[])
{
  double chi2, xstar, ystar;
  int i;

  if(Nstars<NPARMS+1)return;


  for(i=0;i<Nstars;i++)
    {
      starpos(i, &xstar, &ystar, x);
      chi2 = pow((xmeas[i] - xstar)/sigmax[i],2.0e0);
      chi2 += pow((ymeas[i] - ystar)/sigmay[i],2.0e0);
      printf("%d %.3lf %.3lf %.3lf %.3lf %lf\n", i+1, xmeas[i], ymeas[i], xstar, ystar, chi2);
    }

  return;
}

float chi2_func(float x[])
{
  double chi2, xstar, ystar;
  int i;

  if(Nstars<NPARMS+1)return 1.0e0;

  chi2 = 0.0e0;

  for(i=0;i<Nstars;i++)
    {
      starpos(i, &xstar, &ystar, x);
      chi2 += pow((xmeas[i] - xstar)/sigmax[i],2.0e0);
      chi2 += pow((ymeas[i] - ystar)/sigmay[i],2.0e0);
    }

  chi2 /= (double)(2*Nstars-NPARMS);

  return (float)chi2;
}

double fit_func(int N, double param[], float (*funk)(float []))
{
  int i, j, imin, nfunc;
  float *x, *y, **p;
  double fval;

  // Allocate memory

  x = (float *)vector(1,N);
  y = (float *)vector(1,N+1);
  p = (float **)matrix(1,N+1,1,N);

  // Set initial simplex values from the passed values of the parameters

  for(i=1;i<=N+1;i++)
    {
      for(j=1;j<=N;j++)
	{
	  x[j] = p[i][j] = (float)(param[j-1]*(1.0e0+(0.5e0-drand48())/10000.0e0));
	}
      y[i] = funk(x);
    }

  // Use amoeba to minimize

  nfunc = N+1; // Number of function evaluations

  amoeba(p, y, N, 1.0e-14, funk, &nfunc);

  // Find the minimum vertex

  imin = 1; for(i=2;i<=N+1;i++)if(y[i]<y[imin])imin = i;
  
  fval = (double)y[imin];

  // Set the parameters to the minimum values

  for(i=1;i<=N;i++)param[i-1] = (double)p[imin][i];

  // Free allocated memory

  free_matrix(p,1,N+1,1,N);
  free_vector(y,1,N+1);
  free_vector(x,1,N);

  // Return the minimum function value

  return fval;
}

void starpos(int istar, double *xstar, double *ystar, float x[])
{
  VECTOR m, xp, yp;
  double theta, phi, psi, x0, y0, D0;
  
  theta = (double)x[1];
  phi   = (double)x[2];
  psi   = (double)x[3];
  x0 = 0.0e0; //x0    = (double)x[4];
  y0 = 0.0e0; //y0    = (double)x[5];
  D0    = (double)x[4];

  m.e = cos(theta)*cos(phi);
  m.n = cos(theta)*sin(phi);
  m.u = sin(theta);
  
  xp.e = cos(psi)*sin(phi) - sin(psi)*sin(theta)*cos(phi);
  xp.n = -(cos(psi)*cos(phi) + sin(psi)*sin(theta)*sin(phi));
  xp.u = sin(psi)*cos(theta);
  
  yp.e = -(sin(psi)*sin(phi) + cos(psi)*sin(theta)*cos(phi));
  yp.n = sin(psi)*cos(phi) - cos(psi)*sin(theta)*sin(phi);
  yp.u = cos(psi)*cos(theta);
  
  *xstar = D0*dot(s[istar],xp)/dot(s[istar],m)+x0;
  *ystar = D0*dot(s[istar],yp)/dot(s[istar],m)+y0;

  return;
}
