/*
   Routine to determine transformation parameters to adjust measured star positions. -SBT
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include "julian.h"

#define __USE_GNU
#include <string.h>

#define d0(A,B) sqrt(pow((A).x-(B).x,2.0e0)+pow((A).y-(B).y,2.0e0))
#define xoff(A,B) ((B).x - ((A).x*cos(theta) - (A).y*sin(theta))/scale)
#define yoff(A,B) ((B).y - ((A).x*sin(theta) + (A).y*cos(theta))/scale)
#define xtrans(A) ((((A).x-x0)*cos(theta) + ((A).y-y0)*sin(theta))*scale)
#define ytrans(A) ((-((A).x-x0)*sin(theta) + ((A).y-y0)*cos(theta))*scale)

#define RMAX 1.5e0

#define RAD2DEG  (180.0e0/M_PI)
#define DEG2RAD  (M_PI/180.0e0)
#define NMIRS 25
#define NSITES 3
#define dot(A,B)  (((A).e)*((B).e)+((A).n)*((B).n)+((A).u)*((B).u))
#define mag(A)    sqrt(dot(A,A))

typedef struct GEOVECTOR
{
  double e, n ,u;
}GEOVECTOR;

typedef struct VECTOR
{
  double x, y;
  double x0, y0;
} VECTOR;

typedef struct TUBE
{
  char name[12];
  double theta, phi;
  double x, y;
  double x0, y0;
  double xc, yc;
  double vmag;
  int itube;
  int st, sub, tub, rc, row, col;
} TUBE;

int main (int argc, char *argv[]);
void find_tube( TUBE *tb);
int orient(int mir, TUBE *tb);

int main (int argc, char *argv[])
{
  FILE *fp, *pp, *lp;
  VECTOR tmp;
  VECTOR ul, ur, ll, lr, cc;
  VECTOR ul0, ur0, ll0, lr0, cc0;
  VECTOR ul1, ur1, ll1, lr1, cc1;
  static VECTOR s[5000], corner[5];	// Up to 2000 star
  double scale;
  double theta, thetamin, Smin, S, x0, y0, x0min, y0min;
  int site, mir, i, j, N, NC, NCC, NTUBES;
  char buffer[1025], fname[256], *token;
  char command[1025];
  int mtype, MX, MY, M;
  int year, month, day, hour, minute, second, duration;
  double seconds;
  double pressure = 840.0e0;
  double temperature = 20.0e0;
  static TUBE star[5000], s1[5000];
  int NM = 0;
  int MTUBES;
  double SRADIUS;
  double vmin, vmax;
  double sdist, smin;
  char *fileroot, *filepath, *csvfile, *fullroot;
  double WIDTH, HEIGHT;
  char *sitename[3] = { "Middle Drum", "Black Rock", "Long Ridge" };
  char tmpname[20];
  double dMAX, dERR;
  int DUPLICATE;
  double dist, dmin;
  int jstar, ilast;
  double XOFFSET, YOFFSET;
  double jday;
  int img;

  if (argc < 1)
    {
      fprintf (stderr, "ERROR: Usage: %s </path/to/files/>\7\7\n", argv[0]);
      exit (-1);
    }

  fprintf (stderr, "path: %s\n", argv[1]);

  sprintf (command, "find %s -type f -name \"img_????.md.m??.csv\"", argv[1]);
  fprintf (stderr, "command: %s\n", command);
  lp = popen (command, "r");


  printf("site mir img st sub tub rc row col orient x0 y0 xm ym dx dy\n");

  while (fgets (buffer, 1024, lp) != NULL)
    {
      buffer[strlen (buffer) - 1] = '\0';
      csvfile = strdup (buffer);
      filepath = strndup (buffer, strlen (buffer) - 20);

      fprintf (stderr, "path: %s\n", filepath);
      fileroot = strndup (&buffer[strlen (buffer) - 20], 9);
      fullroot = strndup (&buffer[strlen (buffer) - 20], 13);


      fprintf (stderr, "root: \"%s\"\n", fileroot);
      if(sscanf(fileroot,"/img_%d", &img)!=1)exit(-1);
      if(img<1)exit(-1);
      if(img>9999)exit(-1);

      // Extract image metadata

      // Creation date

      sprintf (fname, "exiftool -CreateDate %s/%s.jpg", filepath, fileroot);
      pp = popen (fname, "r");

      if (fgets (buffer, 1023, pp) == NULL)
	{
	  fprintf (stderr,
		   "ERROR: Failed to extract Create Date from %s/%s.jpg\7\7\n",
		   filepath, fileroot);
	  pclose (pp);
	  continue;
	}
      pclose (pp);

      if (sscanf(buffer, "Create Date : %d:%d:%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) != 6)
	{
	  fprintf (stderr, "Failed to read Create Date from %s/%s.jpg\7\7\n", filepath, fileroot);
	  exit (-1);
	}


      // Exposure time

      sprintf (fname, "exiftool -ExposureTime %s/%s.jpg", filepath, fileroot);
      pp = popen (fname, "r");

      if (fgets (buffer, 1023, pp) == NULL)
	{
	  fprintf (stderr, "ERROR: Failed to extract Exposure Time from %s/%s.jpg\7\7\n",
		   filepath, fileroot);
	  pclose (pp);
	  continue;
	}
      pclose (pp);

      if (sscanf (buffer, "Exposure Time : %d", &duration) != 1)
	{
	  fprintf (stderr, "ERROR: Failed to read Exposure Time from %s/%s.jpg\7\7\n",
		   filepath, fileroot);
	  continue;
	}


      // TA Site Number

      sprintf (fname, "exiftool -TA_SiteNumber %s/%s.jpg", filepath, fileroot);
      pp = popen (fname, "r");

      if (fgets (buffer, 1023, pp) == NULL)
	{
	  fprintf (stderr, "ERROR: Failed to extract TA Site Number from %s/%s.jpg\7\7\n",
		   filepath, fileroot);
	  pclose (pp);
	  continue;
	}
      pclose (pp);

      if (sscanf (buffer, "TA Site Number : %d", &site) != 1)
	{
	  fprintf (stderr, "ERROR: Failed to read TA Site Number from %s/%s.jpg\7\7\n",
		   filepath, fileroot);
	  continue;
	}

      // TA Site Name

      sprintf (fname, "exiftool -TA_SiteName %s/%s.jpg", filepath, fileroot);
      pp = popen (fname, "r");

      if (fgets (buffer, 1023, pp) == NULL)
	{
	  fprintf (stderr, "ERROR: Failed to extract TA Site Name from %s/%s.jpg\7\7\n",
		   filepath, fileroot);
	  pclose (pp);
	  continue;
	}
      pclose (pp);

      if (strstr(buffer,"TA Site Name")==NULL)
	{
	  fprintf (stderr, "ERROR: Failed to locate TA Site Name in %s/%s.jpg\7\7\n",
		   filepath, fileroot);
	  continue;
	}
      

      buffer[strlen(buffer)-1] = '\0';
      token = strstr(buffer,": ");
      strcpy(tmpname, &token[2]);

      // TA Mirror Number

      sprintf (fname, "exiftool -TA_MirrorNumber %s/%s.jpg", filepath, fileroot);
      pp = popen (fname, "r");

      if (fgets (buffer, 1023, pp) == NULL)
	{
	  fprintf (stderr, "ERROR: Failed to extract TA Mirror Number from %s/%s.jpg\7\7\n",
		   filepath, fileroot);
	  pclose (pp);
	  continue;
	}
      pclose (pp);

      if (sscanf (buffer, "TA Mirror Number : %d", &mir) != 1)
	{
	  fprintf (stderr, "ERROR: Failed to read TA Mirror Number from %s/%s.jpg\7\7\n",
		   filepath, fileroot);
	  continue;
	}

      fprintf (stderr,"Extracted image metadata from %s/%s.jpg\n\n", filepath, fileroot);

      fprintf (stderr, "root: %s\n", fileroot);

      if((site<0)||(site>2))
	{
	  fprintf (stderr, "ERROR: Invalid site #%d found in image\7\7\n", site);
	  continue;
	}

      if(strcmp(sitename[site],tmpname)!=0)
	{
	  fprintf (stderr, "ERROR: Site name from image \"%s\" does not match site #%d = \"%s\"\7\7\n",  tmpname, site, sitename[site]);
	  continue;
	}

      if(site == 0)
	{
	  if((mir < 1) || (mir > 24))
	    {
	      fprintf (stderr, "ERROR: Invalid mirror number %s m%.2d\7\7\n",  sitename[site], mir);
	      continue;
	    }
	}
      else
	{
	  if ((mir < 0) || (mir > 11))
	    {
	      fprintf (stderr, "ERROR: Invalid mirror number %s m%.2d\7\7\n", sitename[site], mir);
	      continue;
	    }
	}

      // At this point I should have a validated site number and mirror number extracted from the image metadata
      // Now I can look through my corner_geometry.dat file to see if I can find matching corner data.

      WIDTH = HEIGHT = 0.0e0;

      sprintf (fname, "grep \"%d%.2d%.2d\t%d\t%d\" ./corner_geometry.dat", year, month, day, site, mir);
      pp = popen (fname, "r");

      if (fgets (buffer, 1023, pp) == NULL)
	{
	  fprintf (stderr, "ERROR: Failed to find corner data for %d%.2d%.2d site: %d mir: %d\7\7\n", year, month, day, site, mir);
	  continue;
	}

      if (sscanf (buffer, "%*d %*d %*d %lf %lf %lf %lf", &WIDTH, &HEIGHT, &XOFFSET, &YOFFSET) != 4)
	{
	  fprintf (stderr, "ERROR: Failed to read WIDTH, HEIGHT, XOFFSET, and YOFFSET from corner_geometry.dat\7\7\n");

	  continue;
	}

      pclose (pp);

      if(site>0)  // Convert to inches for BR & LR sites
	{
	  WIDTH /= 25.4e0;
	  HEIGHT /= 25.4e0;
	  XOFFSET /= 25.4e0;
	  YOFFSET /= 25.4e0;
	}

      ul0.x = -WIDTH / 2.0e0;
      ul0.y = HEIGHT / 2.0e0;

      ur0.x = WIDTH / 2.0e0;
      ur0.y = HEIGHT / 2.0e0;

      ll0.x = -WIDTH / 2.0e0;
      ll0.y = -HEIGHT / 2.0e0;

      lr0.x = WIDTH / 2.0e0;
      lr0.y = -HEIGHT / 2.0e0 ;

      cc0.x = 0.0e0;
      cc0.y = 0.0e0;

      sprintf (fname, "%s", csvfile);

      if ((fp = fopen (fname, "r")) == NULL)
	{
	  fprintf (stderr, "ERROR: Unable to open measurement file \"%s\"\7\7\n", fname);
	  continue;
	}
      else
	{
	  fprintf (stderr, "Reading measurement file %s\n", fname);
	}

      MX = MY = 0;
      NCC = NC = N = 0;

      while (fgets (buffer, 1023, fp) != NULL)
	{
	  buffer[strlen(buffer)-1] = '\0';

	  mtype = 0;
	  if (strcasestr (buffer, "center") != NULL)mtype = 1;
	  else if (strcasestr (buffer, "corner") != NULL)mtype = 2;
	  else if (strcasestr (buffer, "tube") != NULL)mtype = 3;

	  M = 0;
	  tmp.x = tmp.y = (1000.0e0);

	  if ((token = strtok (buffer, ", ;\t")) == NULL)continue;
	  if(strcasestr(token,"area")!=NULL)M=2;

	  while ((token = strtok (NULL, ", ;\t")) != NULL)
	    {
	      M++;
	  
	      if (MX == 0)
		{
		  if (strstr (token, "X") != NULL)MX = M;
		}
	      if (MY == 0)
		{
		  if (strstr (token, "Y") != NULL)MY = M;
		}

	      if ((MX > 0) && (MY > 0))
		{
		  if (site == 0)
		    {
		      if(mtype==1)
			{
			  if (M == (MX + 1))tmp.x = atof (token);
			  if (M == (MY + 1))tmp.y = atof (token);
			}
		      else
			{
			  if (M == (MX - 1))tmp.x = atof (token);
			  if (M == (MY - 1))tmp.y = atof (token);
			}
		    }
		  else
		    {
		      if (M == (MX - 1))tmp.x = atof (token) / 25.4e0;
		      if (M == (MY - 1))tmp.y = atof (token) / 25.4e0;
		    }
		}
	    }

	  if (abs(tmp.x) > 2.0e0 * WIDTH)continue;
	  if (abs(tmp.y) > 2.0e0 * HEIGHT)continue;

	  if (mtype == 1)
	    {
	      cc.x = tmp.x;
	      cc.y = tmp.y;
	      NCC = 1;
	    }			// Center
	  else if (mtype == 3)
	    {
	      s[N].x0 = s[N].x = tmp.x;
	      s[N].y0 = s[N].y = tmp.y;

	      if(site>0)
		{
		  s[N].x0 *= 25.4e0;
		  s[N].y0 *= 25.4e0;
		}
	      N++;
	    }			// Star
	  else if (mtype == 2)
	    {
	      corner[NC].x = tmp.x;
	      corner[NC].y = tmp.y;
	      NC++;
	    }			// Corner
	}

      fclose (fp);

      if (NCC != 1)
	{
	  sprintf(fname,"%s.processing_report", csvfile, csvfile);

	  if ((fp = fopen (fname, "w")) == NULL)
	    {
	      fprintf (stderr, "ERROR: Unable to write processing report file %s\7\7\n", fname);
	      exit(-1);
	    }

	  fprintf(fp,"Processing report:\n\n");
	  fprintf (fp, "Failed to find center.\7\7\n");

	  fclose(fp);
	  continue;
	}

      if (NC != 4)
	{
	  sprintf(fname,"%s.processing_report", csvfile, csvfile);

	  if ((fp = fopen (fname, "w")) == NULL)
	    {
	      fprintf (stderr, "ERROR: Unable to write processing report file %s\7\7\n", fname);
	      exit(-1);
	    }

	  fprintf(fp,"Processing report:\n\n");
	  fprintf (fp, "Failed to find four corners.\7\7\n");

	  fclose(fp);

	  continue;
	}

      // Set the origin to the center

      for (i = 0; i < NC; i++)
	{
	  fprintf (stderr,"%d: (%.2lf, %.2lf) ", i + 1, corner[i].x, corner[i].y);
	  fprintf (stderr,"- (%.2lf, %.2lf) ", cc.x, cc.y);
	  corner[i].x -= cc.x;
	  corner[i].y -= cc.y;
	  fprintf (stderr,"= (%.2lf, %.2lf)\n", corner[i].x, corner[i].y);
	}
      for (i = 0; i < N; i++)
	{
	  s[i].x -= cc.x;
	  s[i].y -= cc.y;
	}

      // Set center to zero

      cc.x = 0.0e0;
      cc.y = 0.0e0;

      // Identify the corners

      NM = 0;
      for (i = 0; i < NC; i++)
	{
	  if (d0 (corner[i], ul0) < WIDTH / 10.0e0)
	    {
	      ul.x = corner[i].x;
	      ul.y = corner[i].y;
	      NM++;
	    }
	  if (d0 (corner[i], ur0) < WIDTH / 10.0e0)
	    {
	      ur.x = corner[i].x;
	      ur.y = corner[i].y;
	      NM++;
	    }
	  if (d0 (corner[i], ll0) < WIDTH / 10.0e0)
	    {
	      ll.x = corner[i].x;
	      ll.y = corner[i].y;
	      NM++;
	    }
	  if (d0 (corner[i], lr0) < WIDTH / 10.0e0)
	    {
	      lr.x = corner[i].x;
	      lr.y = corner[i].y;
	      NM++;
	    }
	}

      if (NM == 4)
	{
	  fprintf (stderr,"ul: %.3lf %.3lf\n", ul.x, ul.y);
	  fprintf (stderr,"ur: %.3lf %.3lf\n", ur.x, ur.y);
	  fprintf (stderr,"ll: %.3lf %.3lf\n", ll.x, ll.y);
	  fprintf (stderr,"lr: %.3lf %.3lf\n", lr.x, lr.y);
	  fprintf (stderr,"cc: %.3lf %.3lf\n", cc.x, cc.y);
	}
      else
	{
	  fprintf (stderr, "ERROR: Failed to identify the four corners\7\7\n");

	  sprintf(fname,"%s.processing_report", csvfile, csvfile);

	  if ((fp = fopen (fname, "w")) == NULL)
	    {
	      fprintf (stderr, "ERROR: Unable to write processing report file %s\7\7\n", fname);
	      exit(-1);
	    }

	  fprintf(fp,"Processing report:\n\n");
	  fprintf (fp,"ul0: %.3lf %.3lf\n", ul0.x, ul0.y);
	  fprintf (fp,"ur0: %.3lf %.3lf\n", ur0.x, ur0.y);
	  fprintf (fp,"ll0: %.3lf %.3lf\n", ll0.x, ll0.y);
	  fprintf (fp,"lr0: %.3lf %.3lf\n", lr0.x, lr0.y);

	  for (i = 0; i < NC; i++)
	    {
	      fprintf (fp,"corner[%d]: %.3lf %.3lf\n", i+1, corner[i].x, corner[i].y);
	    }

	  fclose(fp);

	  continue;
	}

      for (i = 0; i < N; i++)
	fprintf (stderr, "tube[%d]: %.3lf %.3lf\n", i + 1, s[i].x, s[i].y);
      fprintf (stderr, "\n");

      fflush (stdout);

      // Use rotation and offset invariant distances to set the scale

      scale = d0 (ul0, lr0) / d0 (ul, lr);
      scale += d0 (ur0, ll0) / d0 (ur, ll);
      scale += d0 (ur0, ul0) / d0 (ur, ul);
      scale += d0 (lr0, ll0) / d0 (lr, ll);
      scale += d0 (ul0, ll0) / d0 (ul, ll);
      scale += d0 (ur0, lr0) / d0 (ur, lr);
      scale += d0 (ul0, cc0) / d0 (ul, cc);
      scale += d0 (ur0, cc0) / d0 (ur, cc);
      scale += d0 (ll0, cc0) / d0 (ll, cc);
      scale += d0 (lr0, cc0) / d0 (lr, cc);
      scale /= 10.0e0;

      if (fabs (100.0e0 * (scale - 1.0e0)) > 1.0e0)	// 1 percent maximum
	{
	  fprintf (stderr, "Scale \"%.3lf\" is out of range!\7\7\n", scale);
	  fprintf(stderr,"mv %s %s.badcorners\n", csvfile, csvfile);
	  sprintf(command,"mv %s %s.badcorners\n", csvfile, csvfile);
	  system(command);

	  sprintf(fname,"%s.processing_report", csvfile, csvfile);

	  if ((fp = fopen (fname, "w")) == NULL)
	    {
	      fprintf (stderr, "ERROR: Unable to write processing report file %s\7\7\n", fname);
	      exit(-1);
	    }

	  fprintf(fp,"Processing report:\n\n");
	  fprintf(fp,"Scale \"%.3lf\" is out of range! Should be close to 1.000.\n\n", scale);
	  fprintf(fp,"Are you sure you set the correct distance for measuring?\n");
	  fclose(fp);

	  continue;
	}

      Smin = 1.0e23;
      thetamin = -1.0e0;

      for (theta = -2.0e0 * M_PI * 5.0e0 / 360.0e0;
	   theta < 2.0e0 * M_PI * 5.0e0 / 360.0e0;
	   theta += 2.0e0 * M_PI / 36.0e4)
	{
	  x0 = xoff (ul0, ul);
	  y0 = yoff (ul0, ul);
	  x0 += xoff (ur0, ur);
	  y0 += yoff (ur0, ur);
	  x0 += xoff (ll0, ll);
	  y0 += yoff (ll0, ll);
	  x0 += xoff (lr0, lr);
	  y0 += yoff (lr0, lr);
	  x0 += xoff (cc0, cc);
	  y0 += yoff (cc0, cc);
	  x0 /= 5.0e0;
	  y0 /= 5.0e0;

	  S = pow (ul0.x - xtrans (ul), 2.0e0);
	  S += pow (ul0.y - ytrans (ul), 2.0e0);
	  S += pow (ur0.x - xtrans (ur), 2.0e0);
	  S += pow (ur0.y - ytrans (ur), 2.0e0);
	  S += pow (ll0.x - xtrans (ll), 2.0e0);
	  S += pow (ll0.y - ytrans (ll), 2.0e0);
	  S += pow (lr0.x - xtrans (lr), 2.0e0);
	  S += pow (lr0.y - ytrans (lr), 2.0e0);
	  S += pow (cc0.x - xtrans (cc), 2.0e0);
	  S += pow (cc0.y - ytrans (cc), 2.0e0);

	  if (S < Smin)
	    {
	      thetamin = theta;
	      Smin = S;
	      x0min = x0;
	      y0min = y0;
	    }
	}

      theta = thetamin;
      x0 = x0min;
      y0 = y0min;

      fprintf (stderr,"\n\n");
      fprintf (stderr,"scale = %.5lf\n", scale);
      fprintf (stderr,"x0, y0 = %.5lf, %.5lf\n", x0, y0);
      fprintf (stderr,"theta = %.5lf\n", theta * 180.0e0 / M_PI);
      fprintf (stderr,"Smin = %.6le\n\n", Smin);

      ul1.x = xtrans (ul);
      ul1.y = ytrans (ul);
      ur1.x = xtrans (ur);
      ur1.y = ytrans (ur);
      ll1.x = xtrans (ll);
      ll1.y = ytrans (ll);
      lr1.x = xtrans (lr);
      lr1.y = ytrans (lr);
      cc1.x = xtrans (cc);
      cc1.y = ytrans (cc);
      
      fprintf (stderr,"dUL = %.3lf %.3lf\n", ul0.x - ul1.x, ul0.y - ul1.y);
      fprintf (stderr,"dUR = %.3lf %.3lf\n", ur0.x - ur1.x, ur0.y - ur1.y);
      fprintf (stderr,"dLL = %.3lf %.3lf\n", ll0.x - ll1.x, ll0.y - ll1.y);
      fprintf (stderr,"dLR = %.3lf %.3lf\n", lr0.x - lr1.x, lr0.y - lr1.y);
      fprintf (stderr,"dCC = %.3lf %.3lf\n", cc0.x - cc1.x, cc0.y - cc1.y);
      fprintf (stderr,"\n");
      fprintf (stderr,"dUL-LR = %.3lf\n", d0 (ul1, lr1) - d0 (ul0, lr0));
      fprintf (stderr,"dUR-LL = %.3lf\n", d0 (ur1, ll1) - d0 (ur0, ll0));
      fprintf (stderr,"dUL-LL = %.3lf\n", d0 (ul1, ll1) - d0 (ul0, ll0));
      fprintf (stderr,"dUR-LR = %.3lf\n", d0 (ur1, lr1) - d0 (ur0, lr0));
      fprintf (stderr,"dUL-UR = %.3lf\n", d0 (ul1, ur1) - d0 (ul0, ur0));
      fprintf (stderr,"dLL-LR = %.3lf\n", d0 (ll1, lr1) - d0 (ll0, lr0));
      fprintf (stderr,"dUL-CC = %.3lf\n", d0 (ul1, cc1) - d0 (ul0, cc0));
      fprintf (stderr,"dUR-CC = %.3lf\n", d0 (ur1, cc1) - d0 (ur0, cc0));
      fprintf (stderr,"dLL-CC = %.3lf\n", d0 (ll1, cc1) - d0 (ll0, cc0));
      fprintf (stderr,"dLR-CC = %.3lf\n", d0 (lr1, cc1) - d0 (lr0, cc0));
      fprintf (stderr,"\n");

      dMAX = sqrt(pow(ul0.x - ul1.x, 2.0e0)+pow(ul0.y - ul1.y,2.0e0));
      dERR = sqrt(pow(ur0.x - ur1.x, 2.0e0)+pow(ur0.y - ur1.y,2.0e0)); if(dERR>dMAX)dMAX=dERR;
      dERR = sqrt(pow(ll0.x - ll1.x, 2.0e0)+pow(ll0.y - ll1.y,2.0e0)); if(dERR>dMAX)dMAX=dERR;
      dERR = sqrt(pow(lr0.x - lr1.x, 2.0e0)+pow(lr0.y - lr1.y,2.0e0)); if(dERR>dMAX)dMAX=dERR;
      dERR = sqrt(pow(cc0.x - cc1.x, 2.0e0)+pow(cc0.y - cc1.y,2.0e0)); if(dERR>dMAX)dMAX=dERR;
      dERR = fabs(d0 (ul1, lr1) - d0 (ul0, lr0)); if(dERR>dMAX)dMAX=dERR;
      dERR = fabs(d0 (ur1, ll1) - d0 (ur0, ll0)); if(dERR>dMAX)dMAX=dERR;
      dERR = fabs(d0 (ul1, ll1) - d0 (ul0, ll0)); if(dERR>dMAX)dMAX=dERR;
      dERR = fabs(d0 (ur1, lr1) - d0 (ur0, lr0)); if(dERR>dMAX)dMAX=dERR;
      dERR = fabs(d0 (ul1, ur1) - d0 (ul0, ur0)); if(dERR>dMAX)dMAX=dERR;
      dERR = fabs(d0 (ll1, lr1) - d0 (ll0, lr0)); if(dERR>dMAX)dMAX=dERR;
      dERR = fabs(d0 (ul1, cc1) - d0 (ul0, cc0)); if(dERR>dMAX)dMAX=dERR;
      dERR = fabs(d0 (ur1, cc1) - d0 (ur0, cc0)); if(dERR>dMAX)dMAX=dERR;
      dERR = fabs(d0 (ll1, cc1) - d0 (ll0, cc0)); if(dERR>dMAX)dMAX=dERR;
      dERR = fabs(d0 (lr1, cc1) - d0 (lr0, cc0)); if(dERR>dMAX)dMAX=dERR;

      // Maximum allowed corner error is 0.125"

      if(dMAX > 0.260e0) // Increased to 0.260"
	{
	  fprintf(stderr,"mv %s %s.badcorners\n", csvfile, csvfile);
	  sprintf(command,"mv %s %s.badcorners\n", csvfile, csvfile);
	  system(command);

	  sprintf(fname,"%s.processing_report", csvfile, csvfile);

	  if ((fp = fopen (fname, "w")) == NULL)
	    {
	      fprintf (stderr, "ERROR: Unable to write processing report file %s\7\7\n", fname);
	      exit(-1);
	    }

	  fprintf(fp,"Processing report:\n\n");
	  fprintf(fp,"Maximum corner error exceeded err = %.3lf\n\n", dMAX);

	  fprintf(fp,"dUL = %.3lf %.3lf\n", ul0.x - ul1.x, ul0.y - ul1.y);
	  fprintf(fp,"dUR = %.3lf %.3lf\n", ur0.x - ur1.x, ur0.y - ur1.y);
	  fprintf(fp,"dLL = %.3lf %.3lf\n", ll0.x - ll1.x, ll0.y - ll1.y);
	  fprintf(fp,"dLR = %.3lf %.3lf\n", lr0.x - lr1.x, lr0.y - lr1.y);
	  fprintf(fp,"dCC = %.3lf %.3lf\n", cc0.x - cc1.x, cc0.y - cc1.y);
	  fprintf(fp,"\n");
	  fprintf(fp,"dUL-LR = %.3lf\n", d0 (ul1, lr1) - d0 (ul0, lr0));
	  fprintf(fp,"dUR-LL = %.3lf\n", d0 (ur1, ll1) - d0 (ur0, ll0));
	  fprintf(fp,"dUL-LL = %.3lf\n", d0 (ul1, ll1) - d0 (ul0, ll0));
	  fprintf(fp,"dUR-LR = %.3lf\n", d0 (ur1, lr1) - d0 (ur0, lr0));
	  fprintf(fp,"dUL-UR = %.3lf\n", d0 (ul1, ur1) - d0 (ul0, ur0));
	  fprintf(fp,"dLL-LR = %.3lf\n", d0 (ll1, lr1) - d0 (ll0, lr0));
	  fprintf(fp,"dUL-CC = %.3lf\n", d0 (ul1, cc1) - d0 (ul0, cc0));
	  fprintf(fp,"dUR-CC = %.3lf\n", d0 (ur1, cc1) - d0 (ur0, cc0));
	  fprintf(fp,"dLL-CC = %.3lf\n", d0 (ll1, cc1) - d0 (ll0, cc0));
	  fprintf(fp,"dLR-CC = %.3lf\n", d0 (lr1, cc1) - d0 (lr0, cc0));
	  fprintf(fp,"\n");
	  
	  fclose(fp);

	  continue;
	}

      for (i = 0; i < N; i++)
	{
	  s1[i].x = xtrans(s[i]) + XOFFSET;  // Add offset to true cluster center
	  s1[i].y = ytrans(s[i]) + YOFFSET;  //
	  s1[i].x0 = s[i].x0;
	  s1[i].y0 = s[i].y0;
	  s1[i].vmag = 30.0e0;

	  find_tube(&s1[i]);

	  printf("%d %d ", site, mir);
	  printf("%.4d ", img);
	  printf("%d ", s1[i].st);
	  printf("%d ", s1[i].sub);
	  printf("%d ", s1[i].tub);
	  printf("%d ", s1[i].rc);
	  printf("%d ", s1[i].row);
	  printf("%d ", s1[i].col);
	  printf("%d ", orient(mir, &s1[i]));
	  printf("%.4lf ", s1[i].xc);
	  printf("%.4lf ", s1[i].yc);
	  printf("%.4lf ", s1[i].x);
	  printf("%.4lf ", s1[i].y);
	  printf("%.4lf ", s1[i].x-s1[i].xc);
	  printf("%.4lf ", s1[i].y-s1[i].yc);
	  printf("\n");
	  fflush (stdout);
	}


      // Let my people go

      free (fileroot);
      free (fullroot);
      free (filepath);
      free (csvfile);
    }

  pclose (lp);

  return 0;
}

void find_tube( TUBE *tb)
{
  FILE *op;
  char buffer[1024];
  double DX[NSITES] = {1.645e0, 62.0e0, 62.0e0}; // PMT apothem (i.e. center to center distance, inches)
  double DY; // (sqrt(3.0e0)/2.0e0)*DX  -  PMT vertical distance
  double xc, yc;
  int mir, site;
  int st, sub, tub;
  int rc, row, col;
  double x0, y0, z0;
  double dist;
  double theta, phi;
  double rtube, rmin;
  int stmin;

  GEOVECTOR xhat, yhat, zhat, that;

  const int MAXMIR[NSITES] = { 24, 11, 11}; 
  const int MINMIR[NSITES] = { 1, 0, 0};

  site = 0;

  // Compute the cluster center position
  
  row = 7; col = 8;
  x0  = ((double)((row+1)%2)*0.5e0+(double)col)*DX[site];
  y0  = (double)row*(sqrt(3.0e0)/2.0e0)*DX[site];
  row = 8; col = 7;
  x0 += ((double)((row+1)%2)*0.5e0+(double)col)*DX[site];
  y0 += (double)row*(sqrt(3.0e0)/2.0e0)*DX[site];
  x0 /= 2.0e0;
  y0 /= 2.0e0;
  
  rmin = 1.0e23;
  stmin = 0;

  for(st=0;st<256;st++)
    {
      sub = st/16;
      tub = st%16;
      
      rc = (st&0xC3)|((st>>2)&0x0C)|((st<<2)&0x30);
      row = rc/16;
      col = rc%16;
      
      xc = x0 - ((double)((row+1)%2)*0.5e0+(double)col)*DX[site];
      yc = y0 - (double)row*(sqrt(3.0e0)/2.0e0)*DX[site];
      
      rtube = sqrt(pow(tb->x-xc,2.0e0)+pow(tb->y-yc,2.0e0));

      if(rtube<rmin)
	{
	  rmin = rtube;
	  stmin = st;
	}
    }

  st = stmin;
  sub = st/16;
  tub = st%16;
  
  rc = (st&0xC3)|((st>>2)&0x0C)|((st<<2)&0x30);
  row = rc/16;
  col = rc%16;
  
  xc = x0 - ((double)((row+1)%2)*0.5e0+(double)col)*DX[site];
  yc = y0 - (double)row*(sqrt(3.0e0)/2.0e0)*DX[site];
  
  tb->st = st;
  tb->sub = sub+1;
  tb->tub = tub+1;
  tb->rc = rc+1;
  tb->row = row+1;
  tb->col = col+1;
  tb->xc = xc;
  tb->yc = yc;
  
  return;
}


int orient(int mir, TUBE *tb)
{
  int row, tube, rot;

  row  = 17 - tb->row;
  tube = 17 - tb->col;

  if(mir<7)
    rot = 1;
  else
    rot = 3;
  
  if((mir==1)&&(row==1)&&(tube==1))rot = 5;
  else if((mir==1)&&(row==1)&&(tube==2))rot = 0;
  else if((mir==1)&&(row==1)&&(tube==16))rot = 2;
  else if((mir==1)&&(row==2)&&(tube==16))rot = 2;
  else if((mir==1)&&(row==3)&&(tube==2))rot = 0;
  else if((mir==1)&&(row==6)&&(tube==1))rot = 0;
  else if((mir==1)&&(row==6)&&(tube==4))rot = 0;
  else if((mir==1)&&(row==8)&&(tube==8))rot = 0;
  else if((mir==1)&&(row==12)&&(tube==5))rot = 2;
  else if((mir==1)&&(row==12)&&(tube==6))rot = 5;
  else if((mir==1)&&(row==13)&&(tube==3))rot = 3;
  else if((mir==1)&&(row==14)&&(tube==3))rot = 0;
  else if((mir==1)&&(row==14)&&(tube==4))rot = 2;
  else if((mir==1)&&(row==14)&&(tube==16))rot = 5;
  else if((mir==1)&&(row==16)&&(tube==1))rot = 2;
  else if((mir==1)&&(row==16)&&(tube==14))rot = 2;
  else if((mir==2)&&(row==1)&&(tube==16))rot = 2;
  else if((mir==2)&&(row==3)&&(tube==8))rot = 3;
  else if((mir==2)&&(row==4)&&(tube==5))rot = 0;
  else if((mir==2)&&(row==4)&&(tube==13))rot = 4;
  else if((mir==2)&&(row==9)&&(tube==12))rot = 0;
  else if((mir==2)&&(row==9)&&(tube==15))rot = 5;
  else if((mir==2)&&(row==11)&&(tube==2))rot = 2;
  else if((mir==2)&&(row==12)&&(tube==9))rot = 3;
  else if((mir==2)&&(row==14)&&(tube==12))rot = 5;
  else if((mir==3)&&(row==1)&&(tube==3))rot = 2;
  else if((mir==3)&&(row==3)&&(tube==7))rot = 5;
  else if((mir==3)&&(row==10)&&(tube==9))rot = 5;
  else if((mir==3)&&(row==12)&&(tube==11))rot = 2;
  else if((mir==3)&&(row==16)&&(tube==4))rot = 0;
  else if((mir==3)&&(row==16)&&(tube==8))rot = 0;
  else if((mir==4)&&(row==3)&&(tube==15))rot = 3;
  else if((mir==4)&&(row==5)&&(tube==1))rot = 5;
  else if((mir==4)&&(row==8)&&(tube==7))rot = 3;
  else if((mir==4)&&(row==10)&&(tube==9))rot = 4;
  else if((mir==4)&&(row==10)&&(tube==16))rot = 4;
  else if((mir==4)&&(row==11)&&(tube==2))rot = 3;
  else if((mir==4)&&(row==11)&&(tube==7))rot = 0;
  else if((mir==4)&&(row==12)&&(tube==16))rot = 2;
  else if((mir==4)&&(row==14)&&(tube==4))rot = 2;
  else if((mir==4)&&(row==15)&&(tube==16))rot = 4;
  else if((mir==5)&&(row==1)&&(tube==6))rot = 4;
  else if((mir==5)&&(row==1)&&(tube==13))rot = 0;
  else if((mir==5)&&(row==2)&&(tube==1))rot = 0;
  else if((mir==5)&&(row==2)&&(tube==14))rot = 2;
  else if((mir==5)&&(row==6)&&(tube==8))rot = 4;
  else if((mir==5)&&(row==6)&&(tube==9))rot = 4;
  else if((mir==5)&&(row==8)&&(tube==3))rot = 2;
  else if((mir==5)&&(row==8)&&(tube==4))rot = 4;
  else if((mir==5)&&(row==8)&&(tube==8))rot = 4;
  else if((mir==5)&&(row==8)&&(tube==15))rot = 0;
  else if((mir==5)&&(row==9)&&(tube==9))rot = 5;
  else if((mir==5)&&(row==15)&&(tube==7))rot = 0;
  else if((mir==5)&&(row==15)&&(tube==12))rot = 2;
  else if((mir==5)&&(row==15)&&(tube==14))rot = 0;
  else if((mir==5)&&(row==16)&&(tube==3))rot = 5;
  else if((mir==6)&&(row==1)&&(tube==5))rot = 5;
  else if((mir==6)&&(row==1)&&(tube==7))rot = 4;
  else if((mir==6)&&(row==1)&&(tube==16))rot = 2;
  else if((mir==6)&&(row==3)&&(tube==1))rot = 2;
  else if((mir==6)&&(row==3)&&(tube==12))rot = 2;
  else if((mir==6)&&(row==4)&&(tube==4))rot = 3;
  else if((mir==6)&&(row==5)&&(tube==13))rot = 4;
  else if((mir==6)&&(row==5)&&(tube==14))rot = 0;
  else if((mir==6)&&(row==6)&&(tube==15))rot = 4;
  else if((mir==6)&&(row==7)&&(tube==6))rot = 2;
  else if((mir==6)&&(row==8)&&(tube==3))rot = 5;
  else if((mir==6)&&(row==9)&&(tube==14))rot = 3;
  else if((mir==6)&&(row==11)&&(tube==1))rot = 2;
  else if((mir==6)&&(row==11)&&(tube==3))rot = 2;
  else if((mir==6)&&(row==11)&&(tube==9))rot = 4;
  else if((mir==6)&&(row==11)&&(tube==15))rot = 2;
  else if((mir==6)&&(row==13)&&(tube==7))rot = 5;
  else if((mir==6)&&(row==13)&&(tube==11))rot = 5;
  else if((mir==6)&&(row==14)&&(tube==14))rot = 2;
  else if((mir==6)&&(row==16)&&(tube==3))rot = 0;
  else if((mir==6)&&(row==16)&&(tube==16))rot = 3;
  else if((mir==7)&&(row==8)&&(tube==8))rot = 0;
  else if((mir==7)&&(row==10)&&(tube==7))rot = 4;
  else if((mir==7)&&(row==10)&&(tube==8))rot = 5;
  else if((mir==7)&&(row==11)&&(tube==5))rot = 1;
  else if((mir==7)&&(row==12)&&(tube==11))rot = 4;
  else if((mir==8)&&(row==1)&&(tube==5))rot = 0;
  else if((mir==8)&&(row==2)&&(tube==15))rot = 1;
  else if((mir==8)&&(row==3)&&(tube==15))rot = 4;
  else if((mir==8)&&(row==9)&&(tube==8))rot = 5;
  else if((mir==8)&&(row==12)&&(tube==10))rot = 0;
  else if((mir==8)&&(row==16)&&(tube==9))rot = 2;
  else if((mir==9)&&(row==3)&&(tube==14))rot = 5;
  else if((mir==9)&&(row==5)&&(tube==13))rot = 1;
  else if((mir==9)&&(row==5)&&(tube==16))rot = 4;
  else if((mir==9)&&(row==6)&&(tube==3))rot = 4;
  else if((mir==9)&&(row==7)&&(tube==2))rot = 4;
  else if((mir==9)&&(row==7)&&(tube==8))rot = 4;
  else if((mir==9)&&(row==9)&&(tube==8))rot = 2;
  else if((mir==9)&&(row==9)&&(tube==10))rot = 1;
  else if((mir==9)&&(row==11)&&(tube==6))rot = 2;
  else if((mir==9)&&(row==12)&&(tube==6))rot = 2;
  else if((mir==9)&&(row==13)&&(tube==10))rot = 1;
  else if((mir==9)&&(row==13)&&(tube==11))rot = 4;
  else if((mir==9)&&(row==14)&&(tube==8))rot = 1;
  else if((mir==10)&&(row==4)&&(tube==8))rot = 0;
  else if((mir==10)&&(row==6)&&(tube==5))rot = 2;
  else if((mir==10)&&(row==6)&&(tube==9))rot = 0;
  else if((mir==10)&&(row==7)&&(tube==15))rot = 2;
  else if((mir==10)&&(row==8)&&(tube==8))rot = 1;
  else if((mir==10)&&(row==8)&&(tube==9))rot = 0;
  else if((mir==10)&&(row==9)&&(tube==7))rot = 4;
  else if((mir==10)&&(row==9)&&(tube==8))rot = 2;
  else if((mir==10)&&(row==10)&&(tube==5))rot = 0;
  else if((mir==10)&&(row==13)&&(tube==9))rot = 5;
  else if((mir==10)&&(row==14)&&(tube==6))rot = 1;
  else if((mir==10)&&(row==14)&&(tube==10))rot = 2;
  else if((mir==11)&&(row==5)&&(tube==16))rot = 1;
  else if((mir==11)&&(row==9)&&(tube==13))rot = 4;
  else if((mir==11)&&(row==9)&&(tube==16))rot = 2;
  else if((mir==11)&&(row==13)&&(tube==2))rot = 2;
  else if((mir==12)&&(row==1)&&(tube==2))rot = 2;
  else if((mir==12)&&(row==3)&&(tube==13))rot = 4;
  else if((mir==12)&&(row==4)&&(tube==15))rot = 4;
  else if((mir==12)&&(row==7)&&(tube==13))rot = 4;
  else if((mir==12)&&(row==7)&&(tube==16))rot = 4;
  else if((mir==12)&&(row==8)&&(tube==2))rot = 2;
  else if((mir==12)&&(row==8)&&(tube==5))rot = 2;
  else if((mir==12)&&(row==8)&&(tube==8))rot = 0;
  else if((mir==12)&&(row==8)&&(tube==9))rot = 4;
  else if((mir==12)&&(row==9)&&(tube==9))rot = 5;
  else if((mir==12)&&(row==9)&&(tube==11))rot = 4;
  else if((mir==12)&&(row==11)&&(tube==10))rot = 1;
  else if((mir==12)&&(row==12)&&(tube==3))rot = 1;
  else if((mir==12)&&(row==12)&&(tube==5))rot = 0;
  else if((mir==12)&&(row==12)&&(tube==12))rot = 0;
  else if((mir==12)&&(row==13)&&(tube==1))rot = 0;
  else if((mir==12)&&(row==13)&&(tube==2))rot = 5;
  else if((mir==12)&&(row==16)&&(tube==8))rot = 0;
  else if((mir==13)&&(row==3)&&(tube==11))rot = 4;
  else if((mir==13)&&(row==7)&&(tube==6))rot = 0;
  else if((mir==13)&&(row==10)&&(tube==10))rot = 2;
  else if((mir==13)&&(row==12)&&(tube==3))rot = 5;
  else if((mir==13)&&(row==13)&&(tube==5))rot = 4;
  else if((mir==14)&&(row==4)&&(tube==8))rot = 0;
  else if((mir==14)&&(row==8)&&(tube==8))rot = 0;
  else if((mir==14)&&(row==9)&&(tube==10))rot = 5;
  else if((mir==14)&&(row==16)&&(tube==12))rot = 5;
  
  return rot;
}
