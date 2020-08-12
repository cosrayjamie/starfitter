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
#define NumPMTTrans 18

typedef struct VECTOR
{
  double x, y;
  double x0, y0;
} VECTOR;

typedef struct STAR
{
  char name[12];
  double theta, phi;
  double x, y;
  double x0, y0;
  double vmag;
  int istar;
} STAR;

int main (int argc, char *argv[]);
#ifdef HOMO
void BldPMTMat(double TAX4Tran[][17], char *filepath, double Homo[][9]);
#else
void BldPMTMat(double TAX4Tran[][17], char *filepath);
#endif
//void AvShift(double Shift[], double TAX4Tran[10][15], VECTOR corner[5], int mir);

int
main (int argc, char *argv[])
{
  FILE *fp, *pp, *lp, *jp;
  VECTOR tmp;
  VECTOR ul, ur, ll, lr, cc;
  VECTOR ul0, ur0, ll0, lr0, cc0;
  VECTOR ul1, ur1, ll1, lr1, cc1;
  static VECTOR s[2000], corner[5];	// Up to 2000 star
  double scale;
  double theta, thetamin, Smin, S, x0, y0, x0min, y0min;
  int site, mir, i, j, N, NC, NCC, NSTARS;
  char buffer[1025], tbuf[2015], fname[256], *token;
  char command[1025], command2[1025];
  int mtype, MX, MY, M;
  int year, month, day, hour, minute, second, duration;
  double seconds;
  double pressure = 840.0e0;
  double temperature = 20.0e0;
  static STAR star[5000], s1[5000];
  int NM = 0;
  int MSTARS;
  double SRADIUS;
  double vmin, vmax;
  double sdist, smin;
  char *fileroot, *filepath, *csvfile, *fullroot, *file;
  double WIDTH, HEIGHT;
  char *sitename[4] = { "Middle Drum", "Black Rock", "Long Ridge","Black Rock" };
  char tmpname[20];
  double dMAX, dERR;
  int DUPLICATE;
  double dist, dmin;
  int jstar, ilast;
  double XOFFSET, YOFFSET;
  double jday;
  double sitenum, mirnum, trial;
  int index;
  double TAX4Tran[NumPMTTrans][17];
  double Homo[NumPMTTrans][9];
  double normy;

  //check if a file argument was provided
  if (argc < 1)
    {
      fprintf (stderr, "ERROR: Usage: %s </path/to/files/>\7\7\n", argv[0]);
      exit (-1);
    }
  
  fprintf (stderr, "path: %s\n", argv[1]);
  
#ifdef TAx4
 
  //determine transformations from PMT files
  printf("\n");
  printf("Building matrix of TAX4 Transformations\n");
#ifdef HOMO
  BldPMTMat(TAX4Tran, argv[1], Homo);
#else
  BldPMTMat(TAX4Tran, argv[1]);
#endif

#endif
 
  //look for star files in the specified file path
  sprintf (command, "find %s -type f -name \"img_????.???.csv\"", argv[1]);
  fprintf (stderr, "command: %s\n", command);
  lp = popen (command, "r");
  //run transformation for all star files
  while (fgets (buffer, 1024, lp) != NULL)
    {
      //read and store a mirror star file name
      buffer[strlen (buffer) - 1] = '\0'; //create a terminator
      csvfile = strdup (buffer);
      //filepath up to the directory containing the star photos
      filepath = strndup (buffer, strlen (buffer) - 17);

      fprintf (stderr, "path: %s\n", filepath);
      fileroot = strndup (&buffer[strlen (buffer) - 16], 8); //img_****
      fullroot = strndup (&buffer[strlen (buffer) - 16], 12); //img_****.***
      //create a file to contain results
      sprintf (fname, "results/%s.stars.txt", fullroot);
      if (access (fname, F_OK) != -1)
	continue;		// Results file exists

      // Extract image metadata from .jpg files

      // Creation date

      //prepare and execute exiftool command
      sprintf (fname, "exiftool -CreateDate %s/%s.jpg", filepath, fileroot);
      pp = popen (fname, "r");
      //retrieve create date
      if (fgets (buffer, 1023, pp) == NULL)
	{
	  fprintf (stderr,
		   "ERROR: Failed to extract Create Date from %s/%s.jpg\7\7\n",
		   filepath, fileroot);
	  pclose (pp);
	  continue;
	}
      pclose (pp);
      //store create date into proper variables
      if (sscanf
	  (buffer, "Create Date : %d:%d:%d %d:%d:%d", &year, &month, &day,
	   &hour, &minute, &second) != 6)
	{
	  fprintf (stderr, "Failed to read Create Date from %s/%s.jpg\7\7\n",
		   filepath, fileroot);
	  exit (-1);
	}


      // Exposure time

      //prepare and execute exiftool command
      sprintf (fname, "exiftool -ExposureTime %s/%s.jpg", filepath, fileroot);
      pp = popen (fname, "r");
      //read in exposure time
      if (fgets (buffer, 1023, pp) == NULL)
	{
	  fprintf (stderr,
		   "ERROR: Failed to extract Exposure Time from %s/%s.jpg\7\7\n",
		   filepath, fileroot);
	  pclose (pp);
	  continue;
	}
      pclose (pp);
      //store exposure time
      if (sscanf (buffer, "Exposure Time : %d", &duration) != 1)
	{
	  fprintf (stderr,
		   "ERROR: Failed to read Exposure Time from %s/%s.jpg\7\7\n",
		   filepath, fileroot);
	  continue;
	}

      
      // TA Site Number

      //prepare and execute exiftool command
      sprintf (fname, "exiftool -TA_SiteNumber %s/%s.jpg", filepath,
	       fileroot);
      pp = popen (fname, "r");
      //read in site number
      if (fgets (buffer, 1023, pp) == NULL)
	{
	  fprintf (stderr,
		   "ERROR: Failed to extract TA Site Number from %s/%s.jpg\7\7\n",
		   filepath, fileroot);
	  pclose (pp);
	  continue;
	}
      pclose (pp);
      //store site number
      if (sscanf (buffer, "TA Site Number : %d", &site) != 1)
	{
	  fprintf (stderr,
		   "ERROR: Failed to read TA Site Number from %s/%s.jpg\7\7\n",
		   filepath, fileroot);
	  continue;
	}

      // TA Site Name

      //prepare and execute exiftool command
      sprintf (fname, "exiftool -TA_SiteName %s/%s.jpg", filepath, fileroot);
      pp = popen (fname, "r");
      //read in site name
      if (fgets (buffer, 1023, pp) == NULL)
	{
	  fprintf (stderr,
		   "ERROR: Failed to extract TA Site Name from %s/%s.jpg\7\7\n",
		   filepath, fileroot);
	  pclose (pp);
	  continue;
	}
      pclose (pp);
      //store site name on buffer
      if (strstr (buffer, "TA Site Name") == NULL)
	{
	  fprintf (stderr,
		   "ERROR: Failed to locate TA Site Name in %s/%s.jpg\7\7\n",
		   filepath, fileroot);
	  continue;
	}


      buffer[strlen (buffer) - 1] = '\0'; //create terminator
      token = strstr (buffer, ": "); //search for the first occurance of ": "
      strcpy (tmpname, &token[2]); //store the site name
      
      // TA Mirror Number

      //prepare and execute exiftool command
      sprintf (fname, "exiftool -TA_MirrorNumber %s/%s.jpg", filepath,
	       fileroot);
      pp = popen (fname, "r");
      //read in mirror number
      if (fgets (buffer, 1023, pp) == NULL)
	{
	  fprintf (stderr,
		   "ERROR: Failed to extract TA Mirror Number from %s/%s.jpg\7\7\n",
		   filepath, fileroot);
	  pclose (pp);
	  continue;
	}
      pclose (pp);
      //store mirror number
      if (sscanf (buffer, "TA Mirror Number : %d", &mir) != 1)
	{
	  fprintf (stderr,
		   "ERROR: Failed to read TA Mirror Number from %s/%s.jpg\7\7\n",
		   filepath, fileroot);
	  continue;
	}
  
      printf ("Extracted image metadata from %s/%s.jpg\n\n", filepath,
	      fileroot);

      fprintf (stderr, "root: %s\n", fileroot);
      //check if site number is valid
      if ((site < 0) || (site > 3))
	{
	  fprintf (stderr, "ERROR: Invalid site #%d found in image\7\7\n",
		   site);
	  continue;
	}
 
      //check if the site name matches the site number
      if (strcmp (sitename[site], tmpname) != 0)
	{
	  fprintf (stderr,
		   "ERROR: Site name from image \"%s\" does not match site #%d = \"%s\"\7\7\n",
		   tmpname, site, sitename[site]);
	  continue;
	}

      //check if mirror numbers are correct
      if (site == 0)
	{
	  if ((mir < 1) || (mir > 28))
	    {
	      fprintf (stderr, "ERROR: Invalid mirror number %s m%.2d\7\7\n",
		       sitename[site], mir);
	      continue;
	    }
	}
      else
	{
	  if ((mir < 0) || (mir > 11))
	    {
	      fprintf (stderr, "ERROR: Invalid mirror number %s m%.2d\7\7\n",
		       sitename[site], mir);
	      continue;
	    }
	}

      // At this point I should have a validated site number and mirror number extracted from the image metadata
      // Now I can look through my corner_geometry.dat file to see if I can find matching corner data.

#ifdef TAx4

      //throw out any photos that aren't TAx4

      if (site == 1 || site == 2)
	{
	  continue;
	}

      if (site == 0 && (mir != 25 && mir != 26 && mir != 27 && mir != 28))
	{
	  continue;
	}

      printf("TAX4 Mirror %d Found\n", mir);

#else
	
      WIDTH = HEIGHT = 0.0e0;
      //look in corner_geometry.dat for the corner information of the mirror that was scanned in
      sprintf (fname, "grep \"%d%.2d%.2d\t%d\t%d\" ./corner_geometry.dat",
	       year, month, day, site, mir);
      pp = popen (fname, "r");
      //get the corner information if available
      if (fgets (buffer, 1023, pp) == NULL)
	{
	  fprintf (stderr,
		   "ERROR: Failed to find corner data for %d%.2d%.2d site: %d mir: %d\7\7\n",
		   year, month, day, site, mir);
	  continue;
	}
      //store cover width, height, and offset
      if (sscanf
	  (buffer, "%*d %*d %*d %lf %lf %lf %lf", &WIDTH, &HEIGHT, &XOFFSET,
	   &YOFFSET) != 4)
	{
	  fprintf (stderr,
		   "ERROR: Failed to read WIDTH, HEIGHT, XOFFSET, and YOFFSET from corner_geometry.dat\7\7\n");
	  
	  continue;
	}
      
      pclose (pp);
      
      if (site > 0)		// Convert to inches for BR & LR sites
	{
	  WIDTH /= 25.4e0;
	  HEIGHT /= 25.4e0;
	  XOFFSET /= 25.4e0;
	  YOFFSET /= 25.4e0;
	}
      
      // Hacks to correct for parallax
      
      if ((site == 0) && (mir < 15))
	{
	  WIDTH *= 0.9971e0;
	  HEIGHT *= 0.9971e0;
	}
      if ((site == 0) && (mir > 14))
	{
	  WIDTH *= 0.9923e0;
	  HEIGHT *= 0.9923e0;
	}
      //set the coordinates of the ideal corners of the cover
      ul0.x = -WIDTH / 2.0e0;
      ul0.y = HEIGHT / 2.0e0;
	  
      ur0.x = WIDTH / 2.0e0;
      ur0.y = HEIGHT / 2.0e0;
      
      ll0.x = -WIDTH / 2.0e0;
      ll0.y = -HEIGHT / 2.0e0;
      
      lr0.x = WIDTH / 2.0e0;
      lr0.y = -HEIGHT / 2.0e0;
      //set the coordinate of the ideal center of the cover
      cc0.x = 0.0e0;
      cc0.y = 0.0e0;
    
#endif

      //open cvs file
      sprintf (fname, "%s", csvfile);
      
      if ((fp = fopen (fname, "r")) == NULL)
	{
	  fprintf (stderr,
		   "ERROR: Unable to open measurement file \"%s\"\7\7\n",
		   fname);
	  exit (-1);
	}
      else
	{
	  printf ("Reading measurement file %s\n", fname);
	}
      
      MX = MY = (-1);
      NCC = NC = N = 0;
      //read in cvs file information
      while (fgets (buffer, 1023, fp) != NULL)
	{
	  buffer[strlen (buffer) - 1] = '\0'; //create terminator
	  
	  mtype = 0;
	  if (strcasestr (buffer, "center") != NULL){ //check if read in center
	    mtype = 1;
	  }
	  else if (strcasestr (buffer, "corner") != NULL){ //check if read in corner
	    mtype = 2;

	  }
	  else if (strcasestr (buffer, "star") != NULL){ //check if read in star
	    mtype = 3;
	  }
	  M = 0;
	  tmp.x = tmp.y = (1000.0e0);
	  
	  // Lets see if this is the header
	  //determine data layout by header and record location of coordinates
	  if (strstr (buffer, "XM") != NULL)
	    {
	      if (strstr (buffer, "YM") != NULL)
		{
		  if (strstr (buffer, "StdDev") != NULL)
		    {
		      MX = 9;
		      MY = 10;
		    }
		  else if (strstr (buffer, "Area X Y XM YM") != NULL)
		    {
		      MX = 5;
		      MY = 6;
		    }
		  else if (strstr
			   (buffer, "Area	X	Y	XM	YM") != NULL)
		    {
		      MX = 5;
		      MY = 6;
		    }
		  else if (strcasestr (buffer, "area") != NULL)
		    {
		      MX = 8;
		      MY = 9;
		    }
		  else if (strcasestr (buffer, "XM,YM,label") != NULL)
		    {
		      MX = 1;
		      MY = 2;
		      
		    }
		}
	    }
	  
	  if (mtype == 0) //if line is the header move on
	    continue;
	  
	  M = 0;
	  if ((token = strtok (buffer, ", ;\t")) == NULL)
	    continue;
	  //break up the line and store coordinates in a temporary vector
	  do
	    {
	      M++;
	      if ((MX >= 0) && (MY >= 0))
		{
		  if (site == 0 || site==3) //store location in metric for MD
		    {
		      if (M == MX)
			tmp.x = atof (token);
		      if (M == MY)
			tmp.y = atof (token);
		    }
		  else //store in inches for BR or LR
		    {
		      if (M == MX)
			tmp.x = atof (token) / 25.4e0;
		      if (M == MY)
		    tmp.y = atof (token) / 25.4e0;
		    }
		}
	    }while ((token = strtok (NULL, ", ;\t")) != NULL);
      //check if coordinates make sense
      
#ifdef TAx4
	  //cannot make a check
#else
	
	  if (abs (tmp.x) > 2.0e0 * WIDTH)
	    continue;
	  if (abs (tmp.y) > 2.0e0 * HEIGHT)
	    continue;
	
#endif      

      //use temporary vector to store info in appropriate vectors based on prior identification
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
	      
	      if (site > 0) //convert back to metric if BR or MD
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
#ifdef TAx4
      //The program was being weird at this section so Josh cut out this check for TAx4 telescopes because they aren't used in the computations anyways.
#else
      //if the corners aren't specified move on to the next file
      if (NC != 4)
	{
	  //write a report
	  sprintf (fname, "%s.processing_report", csvfile);
	  
	  if ((fp = fopen (fname, "w")) == NULL)
	    {
	      fprintf (stderr,
		       "ERROR: Unable to write processing report file %s\7\7\n",
		       fname);
	      exit (-1);
	    }
	  
	  fprintf (fp, "Processing report:\n\n");
	  fprintf (fp, "Failed to find four corners. Found %d corners\7\7\n",NC);
	  fprintf (fp, "Found %d stars\7\7\n",N);
	  fclose (fp);
	  
	  exit (-1);
	}
#endif      
      //if the center isn't specified move on to next file unless it is a TAX4 mirror
      
#ifdef TAx4

      //if a TAX4 mirror is encountered find the correct transformation
      
      //find the PMT file in the same directory as the photo and pull out mirror number and trial number
      
      sprintf (command2, "find %s -type f -name \"s?_m??_t?_p?.csv\"", filepath);
      
      fprintf (stderr, "command: %s\n", command2);
      jp = popen (command2, "r");
      //read in the PMT file name
      
      while (fgets (buffer, 1024, jp) != NULL){
	
	buffer[strlen (buffer) - 1] = '\0';
	//here we recover the mirror number and trial number
	
	file = strndup (&buffer[strlen (buffer) - 16], 12);
	printf("\n");
	printf("Found %s.csv\n",file);
	
	token = strtok(file, " ,\t_.smtp/");
	sitenum = atoi(token); //store site number
	token = strtok(NULL, " ,\t_.smtp/");
	mirnum = atoi(token); //store mirror number
	token = strtok(NULL, " ,\t_.smtp/");
	trial = atoi(token); //store trial number
	
	free(file);
	
      }
      fclose(jp);
      
      printf("Site Number %.0f Found\n",sitenum);
      printf("Mirror Number %.0f Found\n",mirnum);
      printf("Trial %.0f Found\n", trial);

      if (abs(sitenum - site) > (1.0/10000.0))
	{
	  printf("Recovered incorrect site number from PMT file\n");
	  printf("Be sure that the correct PMT file is with the photo set\n");
	  continue;
	}
      
      if (abs(mirnum - mir) > (1.0/10000.0))
	{
	  printf("Recovered incorrect mirror number from PMT file\n");
	  printf("Be sure that the correct PMT file is with the photo set\n");
	  continue;
	}
      
      index = 1000;
      //find correct transformation index
      for (i=0; i<NumPMTTrans; i++)
	{
	  if (mirnum == TAX4Tran[i][0] && trial == TAX4Tran[i][15] && sitenum == TAX4Tran[i][16])
	    {
	      printf("Using row %d of the PMT transformation matrix\n",i);
	      index = i;
	      break;
	    }
	}
      
      if (index == 1000)
	{
	  printf("Could not find a PMT transformation for %s\n",fullroot);
	  continue;
	}
      //look for camera movement
      //AvShift(Tran, TAX4Tran, corner, mir);
      //index = Tran[0];
      
      //if a TAX4 mirror set the origin to the computed cluster center and account
      //for camera movement if detected

      printf("Using computed cluster center for TAX4 Mirror %d\n",mir);
      cc.x = TAX4Tran[index][10];
      cc.y = TAX4Tran[index][11];
      
      printf("center used = (%.3f,%.3f)\n",cc.x,cc.y);
      
      for (i = 0; i < N; i++)
	{
	  /*
		    if(Tran[1] != 1000.0 && Tran[2] != 1000.0)
		    {
		    s[i].x += Tran[1];
		    s[i].y += Tran[2];
		    }
	  */
	  s[i].x -= cc.x;
	  s[i].y -= cc.y;
	}
      
      //here we determine the parameters we will use to transform the star coordinates properly

      printf("Setting transformation parameters to those in the matrix\n");
      printf("\n");
      scale = TAX4Tran[index][9];
      x0 = TAX4Tran[index][12];
      y0 = TAX4Tran[index][13];
      theta = TAX4Tran[index][14];

      //transform the star vector
      for (i = 0; i < N; i++)
	{
	  s1[i].x = xtrans (s[i]);
	  s1[i].y = ytrans (s[i]);
	  s1[i].x0 = s[i].x0;
	  s1[i].y0 = s[i].y0;
	  s1[i].vmag = 30.0e0;

#ifdef HOMO
	  //here we apply the proper homographic transformation
	  
	  normy = s1[i].x*Homo[index][6]+s1[i].y*Homo[index][7]+Homo[index][8];
	  s1[i].x = (s1[i].x*Homo[index][0]+s1[i].y*Homo[index][1]+Homo[index][2])/normy;
	  s1[i].y = (s1[i].x*Homo[index][3]+s1[i].y*Homo[index][4]+Homo[index][5])/normy;

#endif
	  //convert to inches
	  s1[i].x = s1[i].x/25.4;
	  s1[i].y = s1[i].y/25.4;

	  printf
	    ("star[%d] = %d %.4d/%.2d/%.2d %.2d:%.2d:%.2d %d %.4lf %.4lf\n",
	     i + 1, mir, year, month, day, hour, minute, second, duration,
	     s1[i].x, s1[i].y);
	}
      
      hour += 6; //this is to convert to UTC time
      if (hour >= 24)
	{
	  hour -= 24;
	  day += 1;
	}
      printf("Converted time to UTC\n");

#else


      if (NCC != 1)
	{
	 
	  //write a report
	  sprintf (fname, "%s.processing_report", csvfile);
	  
	  if ((fp = fopen (fname, "w")) == NULL)
	    {
	      fprintf (stderr,
		       "ERROR: Unable to write processing report file %s\7\7\n",
		       fname);
	      exit (-1);
	    }
	  
	  fprintf (fp, "Processing report:\n\n");
	  fprintf (fp, "Failed to find center.\7\7\n");
	  
	  fclose (fp);
	  exit (-1);
	  
	}
      

      // Set the origin to the center

      //transform corner coordinates
      for (i = 0; i < NC; i++)
	{
	  printf ("%d: (%.2lf, %.2lf) ", i + 1, corner[i].x, corner[i].y);
	  printf ("- (%.2lf, %.2lf) ", cc.x, cc.y);
	  corner[i].x -= cc.x;
	  corner[i].y -= cc.y;
	  printf ("= (%.2lf, %.2lf)\n", corner[i].x, corner[i].y);
	}
      //transform star coordinates
      for (i = 0; i < N; i++)
	{
	  s[i].x -= cc.x;
	  s[i].y -= cc.y;
	}
      
      // Set center to zero
      
      cc.x = 0.0e0;
      cc.y = 0.0e0;

      // Identify the corners
      //if a measured corner is close enough to a theoretical corner, store it as that corner
      NM = 0;
      for (i = 0; i < NC; i++)
	{
	  //upper left
	  if (d0 (corner[i], ul0) < WIDTH / 10.0e0)
	    {
	      ul.x = corner[i].x;
	      ul.y = corner[i].y;
	      NM++;
	    }
	  //upper right
	  if (d0 (corner[i], ur0) < WIDTH / 10.0e0)
	    {
	      ur.x = corner[i].x;
	      ur.y = corner[i].y;
	      NM++;
	    }
	  //lower left
	  if (d0 (corner[i], ll0) < WIDTH / 10.0e0)
	    {
	      ll.x = corner[i].x;
	      ll.y = corner[i].y;
	      NM++;
	    }
	  //lower right
	  if (d0 (corner[i], lr0) < WIDTH / 10.0e0)
	    {
	      lr.x = corner[i].x;
	      lr.y = corner[i].y;
	      NM++;
	    }
	}
      //make sure that all of the  corners were assigned
      if (NM == 4)
	{
	  printf ("ul: %.3lf %.3lf\n", ul.x, ul.y);
	  printf ("ur: %.3lf %.3lf\n", ur.x, ur.y);
	  printf ("ll: %.3lf %.3lf\n", ll.x, ll.y);
	  printf ("lr: %.3lf %.3lf\n", lr.x, lr.y);
	  printf ("cc: %.3lf %.3lf\n", cc.x, cc.y);
	}
      else
	{
	  fprintf (stderr,
		   "ERROR: Failed to identify the four corners\7\7\n");
	  //rename file to show it is bad
	  fprintf (stderr, "mv %s %s.badcorners\n", csvfile, csvfile);
	  sprintf (command, "mv %s %s.badcorners\n", csvfile, csvfile);
	  system (command);
	  //write a report
	  sprintf (fname, "%s.processing_report", csvfile);
	  
	  if ((fp = fopen (fname, "w")) == NULL)
	    {
	      fprintf (stderr,
		       "ERROR: Unable to write processing report file %s\7\7\n",
		       fname);
	      exit (-1);
	    }
	  
	  fprintf (fp, "Processing report:\n");
	  fprintf (fp, "Failed to identify four corners.\n\n");
	  fprintf (fp, "ul0: %.3lf %.3lf\n", ul0.x, ul0.y);
	  fprintf (fp, "ur0: %.3lf %.3lf\n", ur0.x, ur0.y);
	  fprintf (fp, "ll0: %.3lf %.3lf\n", ll0.x, ll0.y);
	  fprintf (fp, "lr0: %.3lf %.3lf\n", lr0.x, lr0.y);
	  
	  for (i = 0; i < NC; i++)
	    {
	      fprintf (fp, "corner[%d]: %.3lf %.3lf\n", i + 1, corner[i].x,
		       corner[i].y);
	    }
	  
	  fclose (fp);
	  
	  
	}
      
      for (i = 0; i < N; i++)
	printf ("star[%d]: %.3lf %.3lf\n", i + 1, s[i].x, s[i].y);
      printf ("\n");
      
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
      //check and see if the corners were measured badly
      if (fabs (100.0e0 * (scale - 1.0e0)) > 1.5e0)	// 1.5 percent maximum
	{
	  //rename file to show it is bad
	  fprintf (stderr, "Scale \"%.3lf\" is out of range!\7\7\n", scale);
	  fprintf (stderr, "mv %s %s.badcorners\n", csvfile, csvfile);
	  sprintf (command, "mv %s %s.badcorners\n", csvfile, csvfile);
	  system (command);
	  //write a report
	  sprintf (fname, "%s.processing_report", csvfile);
	  
	  if ((fp = fopen (fname, "w")) == NULL)
	    {
	      fprintf (stderr,"ERROR: Unable to write processing report file %s\7\7\n",fname);
	      exit (-1);
	    }
	  
	  fprintf (fp, "Processing report:\n\n");
	  fprintf (fp,"Scale \"%.3lf\" is out of range! Should be close to 1.000.\n\n",scale);
	  fprintf (fp,"Are you sure you set the correct distance for measuring?\n");
	  fclose (fp);
	  
	}
      
      Smin = 1.0e23;
      thetamin = -1.0e0;
      //find proper rotation angle and offset by minimisation
      for (theta = -2.0e0 * M_PI * 5.0e0 / 360.0e0;
	   theta < 2.0e0 * M_PI * 5.0e0 / 360.0e0;
	   theta += 2.0e0 * M_PI / 36.0e4)
	{
	  //xoff/yoff rotates the original vector and scales it for the measured coordinate system  and then finds the difference in this vector and the measured one
	  //find an average offset for a given theta
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
	  //xtrans/ytrans sets the average offset to the origin, rotates counterclockwise by theta, and scales the coordinate to the ideal coordinate system
	  //find the difference in the ideal and the transformed measured coordinates
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
	  //if the sum of the squares is smaller than the previous sum then record the sum, the current value of theta, and the offset
	  if (S < Smin)
	    {
	      thetamin = theta;
	      Smin = S;
	      x0min = x0;
	      y0min = y0;
	    }
	}
      //save proper transformation angle and offset
      theta = thetamin;
      x0 = x0min;
      y0 = y0min;
      
      printf ("\n\n");
      printf ("scale = %.5lf\n", scale);
      printf ("x0, y0 = %.5lf, %.5lf\n", x0, y0);
      printf ("theta = %.5lf\n", theta * 180.0e0 / M_PI);
      printf ("Smin = %.6le\n\n", Smin);
      //transform all measured corners and center
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
      //print a whole bunch of numbers
      printf ("dUL = %.3lf %.3lf\n", ul0.x - ul1.x, ul0.y - ul1.y);
      printf ("dUR = %.3lf %.3lf\n", ur0.x - ur1.x, ur0.y - ur1.y);
      printf ("dLL = %.3lf %.3lf\n", ll0.x - ll1.x, ll0.y - ll1.y);
      printf ("dLR = %.3lf %.3lf\n", lr0.x - lr1.x, lr0.y - lr1.y);
      printf ("dCC = %.3lf %.3lf\n", cc0.x - cc1.x, cc0.y - cc1.y);
      printf ("\n");
      printf ("dUL-LR = %.3lf\n", d0 (ul1, lr1) - d0 (ul0, lr0));
      printf ("dUR-LL = %.3lf\n", d0 (ur1, ll1) - d0 (ur0, ll0));
      printf ("dUL-LL = %.3lf\n", d0 (ul1, ll1) - d0 (ul0, ll0));
      printf ("dUR-LR = %.3lf\n", d0 (ur1, lr1) - d0 (ur0, lr0));
      printf ("dUL-UR = %.3lf\n", d0 (ul1, ur1) - d0 (ul0, ur0));
      printf ("dLL-LR = %.3lf\n", d0 (ll1, lr1) - d0 (ll0, lr0));
      printf ("dUL-CC = %.3lf\n", d0 (ul1, cc1) - d0 (ul0, cc0));
      printf ("dUR-CC = %.3lf\n", d0 (ur1, cc1) - d0 (ur0, cc0));
      printf ("dLL-CC = %.3lf\n", d0 (ll1, cc1) - d0 (ll0, cc0));
      printf ("dLR-CC = %.3lf\n", d0 (lr1, cc1) - d0 (lr0, cc0));
      printf ("\n");
      //find the maximum error in the ideal and transformed coordinates
      dMAX = sqrt (pow (ul0.x - ul1.x, 2.0e0) + pow (ul0.y - ul1.y, 2.0e0));
      dERR = sqrt (pow (ur0.x - ur1.x, 2.0e0) + pow (ur0.y - ur1.y, 2.0e0));
      if (dERR > dMAX)
	dMAX = dERR;
      dERR = sqrt (pow (ll0.x - ll1.x, 2.0e0) + pow (ll0.y - ll1.y, 2.0e0));
      if (dERR > dMAX)
	dMAX = dERR;
      dERR = sqrt (pow (lr0.x - lr1.x, 2.0e0) + pow (lr0.y - lr1.y, 2.0e0));
      if (dERR > dMAX)
	dMAX = dERR;
      dERR = sqrt (pow (cc0.x - cc1.x, 2.0e0) + pow (cc0.y - cc1.y, 2.0e0));
      if (dERR > dMAX)
	dMAX = dERR;
      dERR = fabs (d0 (ul1, lr1) - d0 (ul0, lr0));
      if (dERR > dMAX)
	dMAX = dERR;
      dERR = fabs (d0 (ur1, ll1) - d0 (ur0, ll0));
      if (dERR > dMAX)
	dMAX = dERR;
      dERR = fabs (d0 (ul1, ll1) - d0 (ul0, ll0));
      if (dERR > dMAX)
	dMAX = dERR;
      dERR = fabs (d0 (ur1, lr1) - d0 (ur0, lr0));
      if (dERR > dMAX)
	dMAX = dERR;
      dERR = fabs (d0 (ul1, ur1) - d0 (ul0, ur0));
      if (dERR > dMAX)
	dMAX = dERR;
      dERR = fabs (d0 (ll1, lr1) - d0 (ll0, lr0));
      if (dERR > dMAX)
	dMAX = dERR;
      dERR = fabs (d0 (ul1, cc1) - d0 (ul0, cc0));
      if (dERR > dMAX)
	dMAX = dERR;
      dERR = fabs (d0 (ur1, cc1) - d0 (ur0, cc0));
      if (dERR > dMAX)
	dMAX = dERR;
      dERR = fabs (d0 (ll1, cc1) - d0 (ll0, cc0));
      if (dERR > dMAX)
	dMAX = dERR;
      dERR = fabs (d0 (lr1, cc1) - d0 (lr0, cc0));
      if (dERR > dMAX)
	dMAX = dERR;
      
      // Maximum allowed corner error is 0.125"
      
      if (dMAX > 0.50e0)	// Increased to 0.260"
	{
	  //rename file to show it is bad
	  fprintf (stderr, "mv %s %s.badcorners\n", csvfile, csvfile);
	  sprintf (command, "mv %s %s.badcorners\n", csvfile, csvfile);
	  system (command);
	  //write a report
	  sprintf (fname, "%s.processing_report", csvfile);
	  
	  if ((fp = fopen (fname, "w")) == NULL)
	    {
	      fprintf (stderr,"ERROR: Unable to write processing report file %s\7\7\n",fname);
	      exit (-1);
	    }
	  
	  fprintf (fp, "Processing report:\n\n");
	  fprintf (fp, "Maximum corner error exceeded err = %.3lf\n\n", dMAX);
	  
	  fprintf (fp, "dUL = %.3lf %.3lf\n", ul0.x - ul1.x, ul0.y - ul1.y);
	  fprintf (fp, "dUR = %.3lf %.3lf\n", ur0.x - ur1.x, ur0.y - ur1.y);
	  fprintf (fp, "dLL = %.3lf %.3lf\n", ll0.x - ll1.x, ll0.y - ll1.y);
	  fprintf (fp, "dLR = %.3lf %.3lf\n", lr0.x - lr1.x, lr0.y - lr1.y);
	  fprintf (fp, "dCC = %.3lf %.3lf\n", cc0.x - cc1.x, cc0.y - cc1.y);
	  fprintf (fp, "\n");
	  fprintf (fp, "dUL-LR = %.3lf\n", d0 (ul1, lr1) - d0 (ul0, lr0));
	  fprintf (fp, "dUR-LL = %.3lf\n", d0 (ur1, ll1) - d0 (ur0, ll0));
	  fprintf (fp, "dUL-LL = %.3lf\n", d0 (ul1, ll1) - d0 (ul0, ll0));
	  fprintf (fp, "dUR-LR = %.3lf\n", d0 (ur1, lr1) - d0 (ur0, lr0));
	  fprintf (fp, "dUL-UR = %.3lf\n", d0 (ul1, ur1) - d0 (ul0, ur0));
	  fprintf (fp, "dLL-LR = %.3lf\n", d0 (ll1, lr1) - d0 (ll0, lr0));
	  fprintf (fp, "dUL-CC = %.3lf\n", d0 (ul1, cc1) - d0 (ul0, cc0));
	  fprintf (fp, "dUR-CC = %.3lf\n", d0 (ur1, cc1) - d0 (ur0, cc0));
	  fprintf (fp, "dLL-CC = %.3lf\n", d0 (ll1, cc1) - d0 (ll0, cc0));
	  fprintf (fp, "dLR-CC = %.3lf\n", d0 (lr1, cc1) - d0 (lr0, cc0));
	  fprintf (fp, "\n");
	  
	  fclose (fp);
	  
	  exit (-1);
	}

      //transform the star vector
      for (i = 0; i < N; i++)
	{
	  s1[i].x = xtrans (s[i]); + XOFFSET;	// Add offset to true cluster center
	  s1[i].y = ytrans (s[i]); + YOFFSET;	//
	  s1[i].x0 = s[i].x0;
	  s1[i].y0 = s[i].y0;
	  s1[i].vmag = 30.0e0;
	  
	  printf
	    ("star[%d] = %d %.4d/%.2d/%.2d %.2d:%.2d:%.2d %d %.4lf %.4lf\n",
	     i + 1, mir, year, month, day, hour, minute, second, duration,
	     s1[i].x, s1[i].y);
	}
      
#endif
      
      printf ("\n\n");
      fflush (stdout);

      
      // Now lets see if we can identify any of these stars
      // According to Tom the time stamp is the beginning of the exposure

      jday = julian (year, month, day, hour, minute, (double) second);
      jday += (double) duration / 2.0e0 / 86400.0e0;

      jdate (jday, &year, &month, &day, &hour, &minute, &seconds);

      // Simple temperature and pressure model from Middle Drum data

      pressure = 840.0e0;
      temperature =
	12.4846316595379e0 *
	cos ((fmod (jday, 365.25e0) -
	      61.9431212259993e0) / 55.8541567811241e0) + 12.8472530333894e0;
      //run the ta_stars program for the mirror currently being analyzed
      sprintf (fname,
	       "./ta_stars %d %d %d %d %d %d %d %.6lf %.2lf %.2lf | sort -k 6 -g",
	       site, mir, year, month, day, hour, minute,
	       floor (seconds * 1.0e5) / 1.0e5, temperature, pressure);

      printf ("Looking for stars: %s\n", fname);

      pp = popen (fname, "r");

      NSTARS = 0;
      while (fgets (buffer, 1023, pp) != NULL)
	{
	  //read in star into the star vector
	  if (sscanf (buffer, "%s %lf %lf %lf %lf %lf",
		      star[NSTARS].name,
		      &(star[NSTARS].theta), &(star[NSTARS].phi),
		      &(star[NSTARS].x), &(star[NSTARS].y),
		      &(star[NSTARS].vmag)) == 6)
	    NSTARS++;
	}

      pclose (pp);

      // Clear list of matched stars

      for (i = 0; i < NSTARS; i++)
	star[i].istar = (-1);

      // Look for stars matching measured locations

      for (j = 0; j < N; j++)
	{
	  // Find the closest star to this location
	  smin = 1.0e23;
	  ilast = (-1);
	  for (i = 0; i < NSTARS; i++)
	    {
	      if (star[i].istar < 0)	// Not all ready matched
		{
		  sdist = d0 (star[i], s1[j]);
		  //if the distance from measured to star is smaller than the previous minimum
		  if ((sdist < RMAX) && (sdist < smin))
		    {
		      //if the measured vector magnitude +1 is larger than the star vector mag
		      if (s1[j].vmag + 1.0e0 > star[i].vmag)
			{
			  s1[j].vmag = star[i].vmag; //set measured v mag as current star v mag
			  star[i].istar = j; //set star as matched
			  if (ilast >= 0) //reset previous star as unmatched
			    star[ilast].istar = (-1);
			  //set current star as most likely match
			  ilast = i;
			  smin = sdist;
			}
		    }
		}
	    }
	}

      // Look for duplicate matches

      for (i = 0; i < NSTARS; i++)
	{
	  if (star[i].istar < 0)
	    continue;		// No match
	  for (j = 0; j < NSTARS; j++)
	    {
	      if (star[j].istar < 0)
		continue;	// No match
	      {
		if (i == j)
		  continue;	// Same star
		if (star[i].istar == star[j].istar)	// Duplicate star match - danger
		  {
		    //set closest star as the match
		    if (d0 (star[i], s1[star[i].istar]) <
			d0 (star[j], s1[star[j].istar]))
		      {
			star[j].istar = (-1);
		      }
		    else
		      {
			star[i].istar = (-1);
		      }
		  }
	      }
	    }
	}
      //count the number of matched stars
      MSTARS = 0;
      DUPLICATE = (-1);
      for (i = 0; i < NSTARS; i++)
	{
	  if (star[i].istar < 0)
	    continue;		// No match
	  for (j = 0; j < NSTARS; j++)
	    {
	      if (star[j].istar < 0)
		continue;	// No match
	      {
		if (i == j)
		  continue;	// Same star
		if (star[i].istar == star[j].istar)	// Duplicate star match - danger
		  {
		    DUPLICATE = 1;
		  }
	      }
	    }
	  if (DUPLICATE == 1)
	    {
	      MSTARS = 0;
	      break;
	    }
	  else
	    {
	      MSTARS++;
	    }
	}
      //check if we matched all of the measured stars
      if (MSTARS < N)
	{
	  //rename file to show it is bad
	  fprintf (stderr, "mv %s %s.matched_%d_out_of_%d_stars\n", csvfile,
		   csvfile, MSTARS, N);
	  sprintf (command, "mv %s %s.matched_%d_out_of_%d_stars\n", csvfile,
		   csvfile, MSTARS, N);
	  system (command);
	  //write a report
	  sprintf (fname, "%s.processing_report", csvfile);

	  if ((fp = fopen (fname, "w")) == NULL)
	    {
	      fprintf (stderr,
		       "ERROR: Unable to write processing report file %s\7\7\n",
		       fname);
	      exit (-1);
	    }

	  fprintf (fp, "Processing report:\n\n");
	  fprintf (fp, "%s m%.2d %d/%.2d/%d %.2d:%.2d:%.2lf\n",
		   sitename[site], mir, month, day, year, hour, minute,
		   seconds);
	  fprintf (fp, "Photo: %s\n\n", csvfile);

	  fprintf (fp, "%d stars were found in the CSV file.\n\n", N);

	  for (j = 0; j < N; j++)
	    {
	      fprintf (fp, "%d. (%.3lf,%.3lf)\n", j + 1, s1[j].x0, s1[j].y0);
	    }

	  fprintf (fp,
		   "\n%d out of %d potential matches were identified.\n\n",
		   MSTARS, NSTARS);
	  //print matched star info in report
	  for (j = 0; j < N; j++)
	    {
	      for (i = 0; i < NSTARS; i++)
		{
		  if (star[i].istar == j) //if a matched star is found print star info
		    {
		      fprintf (fp, "%d. (%.3lf,%.3lf) ", j + 1,
			       s1[star[i].istar].x0, s1[star[i].istar].y0);
		      fprintf (fp, "=> (%.3lf,%.3lf) ", s1[star[i].istar].x,
			       s1[star[i].istar].y);
		      fprintf (fp, "-> %s mag: %.2lf (%.3lf,%.3lf) ",
			       star[i].name, star[i].vmag, star[i].x,
			       star[i].y);
		      fprintf (fp, "delta: (%.3lf" ",%.3lf" ")\n",
			       s1[star[i].istar].x - star[i].x,
			       s1[star[i].istar].y - star[i].y);
		      break;
		    }
		}
	      //if no match find the nearest star
	      if (i == NSTARS)	// No matching star found
		{
		  dmin = 1.0e23;
		  jstar = (-1);
		  for (i = 0; i < NSTARS; i++)
		    {
		      if (star[i].istar < 0)	// No match
			{
			  if (star[i].istar < (-1))
			    continue;	// All ready matched another missing star
			  dist = d0 (star[i], s1[j]);
			  if (dist < dmin)
			    {
			      dmin = dist;
			      jstar = i;
			    }
			}
		    }

		  fprintf (fp, "%d. (%.3lf,%.3lf) ", j + 1, s1[j].x0,
			   s1[j].y0);
		  fprintf (fp,
			   "=> (%.3lf,%.3lf) not matched, closest star is ",
			   s1[j].x, s1[j].y);
		  fprintf (fp, "%s mag: %.2lf (%.3lf,%.3lf) ",
			   star[jstar].name, star[jstar].vmag, star[jstar].x,
			   star[jstar].y);
		  fprintf (fp, "delta: (%.3lf" ",%.3lf" ")\n",
			   s1[j].x - star[jstar].x, s1[j].y - star[jstar].y);
		  //mark that the star has been "matched" with a missing star
		  star[jstar].istar = (-2);
		}
	    }
	  //print unmatched stars
	  jstar = 1;
	  for (i = 0; i < NSTARS; i++)
	    {
	      if (star[i].istar == (-1))
		{
		  fprintf (fp, "%d. %s mag: %.2lf (%.3lf,%.3lf)\n",
			   N + jstar++, star[i].name, star[i].vmag, star[i].x,
			   star[i].y);
		}
	    }

	  //print duplication error if there was one
	  if (DUPLICATE == 1)
	    fprintf (fp,
		     "\n\nDuplicate matches confused the analysis program. Do not include stars which are too close together.\n");

	  fclose (fp);
	  continue;
	}

#ifdef TAx4
      sprintf (fname, "results/m%.0f.t%.0f.%s.stars.txt", mirnum,trial,fullroot);
#else
      sprintf (fname, "results/%s.stars.txt", fullroot);
#endif

      if ((fp = fopen (fname, "w")) == NULL)
	{
	  fprintf (stderr, "ERROR: Unable to write results file %s\7\7\n",
		   fname);
	  exit (-1);
	}
      //print star information for matched stars
      for (j = 0; j < NSTARS; j++)
	{
	  if (star[j].istar >= 0)
	    {
	      if (d0 (star[j], s1[star[j].istar]) > RMAX)
		continue;
	      printf ("star[%d].istar = %d\n", j + 1, star[j].istar + 1);
	      fprintf (fp, "%d %d ", site + 1, mir);
	      fprintf (fp, "%.4d/%.2d/%.2d %.2d:%.2d:%.2lf %d ", year, month,
		       day, hour, minute, seconds, duration);
	      fprintf (fp, "%s %.4lf %.4lf %.4lf %.4lf ", star[j].name,
		       star[j].theta, star[j].phi, star[j].x, star[j].y);
	      fprintf (fp, "%.4lf %.4lf %.4lf\n", s1[star[j].istar].x,
		       s1[star[j].istar].y, d0 (star[j], s1[star[j].istar]));
	    }
	}

      fclose (fp);

      // Results file successfully written.

      fprintf (stderr, "mv %s %s.passed\n", csvfile, csvfile);
      sprintf (command, "mv %s %s.passed\n", csvfile, csvfile);
      system (command);
      
      // Let my people go  

      free (fileroot);
      free (fullroot);
      free (filepath);
      free (csvfile);
      
    }

  pclose (lp);

  return 0;
}

//This looks at PMT photos in the MDTAX4PMT directory and stores coordinate transformations.  I'm making this to minimize FLOPS by not computing these same coordinate transformations for every star photograph
//need to make photo file an input

#ifdef TAx4

#ifdef HOMO
void BldPMTMat(double TAX4Tran[][17], char *filepath, double Homo[][9]){ 
#else
void BldPMTMat(double TAX4Tran[][17], char *filepath){ 
#endif  

  FILE *lp, *fp, *jp, *pp;
  char buffer[1025], *token, fname[256];
  char command[1025];
  char *csvfile, *copyfile, *file;
  int cntr, pos, lngthu;
  double PMTMat[71][3], IPMTMat[256][3];
  int i,j;
  int Numbo;
  VECTOR Tmp1,Tmp2;
  VECTOR CCenter,PointA,PointB;
  double scalesum1, scalesum2, scale;
  double theta, thetamin, Smin, S, x0, y0, x0min, y0min;
  double h0, h1, h2, h3, h4, h5, h6, h7 ,h8;

  //here we open the ideal PMT file and read in the information
 
  sprintf(fname, "./IdealPMT.txt");
  if ((fp = fopen (fname, "r")) == NULL)
    {
     fprintf (stderr,"ERROR: Unable to open measurement file \"%s\"\7\7\n",fname);
    }
  else
    {
      printf ("Reading Ideal PMT file %s\n", fname);
      cntr = 0;
      while (fgets (buffer, 1023, fp) != NULL)
	{
	  buffer[strlen (buffer) - 1] = '\0'; //create terminator
	  //if the header is found move on
	  if (strstr(buffer,"X") != NULL){ 
	    continue;
	  }
    
	  //start breaking the line up
	  token = strtok(buffer, " ,\t");
	  for(i=0; i<=2; i++){
	    IPMTMat[cntr][i] = atof(token); //store information into a matrix
	    token = strtok(NULL, " ,\t");
	  }
    
	  cntr++;
	}
    }

  fclose(fp);

  //find the PMT Files.
  sprintf (command, "find %s -type f -name \"s?_m??_t?_p?.csv\"", filepath);
  //fprintf (stderr, "command: %s\n", command);
  lp = popen (command, "r");

  //read in the PMT files
  pos = 0;
  while (fgets (buffer, 1024, lp) != NULL){
    
    buffer[strlen (buffer) - 1] = '\0';
    csvfile = strdup(buffer);
    //here we recover the mirror number and trial number
    
    file = strndup (&buffer[strlen (buffer) - 16], 12);
    printf("\n");
    printf("Found %s.csv\n",file);
    fprintf(stderr,"Found %s.csv\n",file);
    token = strtok(file, " ,\t_.smtp/");
   
    TAX4Tran[pos][16] = atoi(token); //store site number into the matrix
    token = strtok(NULL, " ,\t_.smtp/");
    TAX4Tran[pos][0] = atoi(token); //store mirror number into the matrix
    token = strtok(NULL, " ,\t_.smtp/");
    TAX4Tran[pos][15] = atoi(token); //store trial number into matrix
    
    printf("row = %d\n",pos);
    printf("site number = %.0f\n",TAX4Tran[pos][16]);
    printf("mirror number = %.0f\n",TAX4Tran[pos][0]);
    printf("trial number = %.0f\n",TAX4Tran[pos][15]);
    
    sprintf(fname, "%s", csvfile);
    if ((fp = fopen (fname, "r")) == NULL)
	{
	  fprintf (stderr,
		   "ERROR: Unable to open measurement file \"%s\"\7\7\n",
		   fname);
	  continue;
	}
    else
	{
	  printf ("Reading measurement file %s\n", fname);
	}
    cntr = 0;
    while (fgets (buffer, 1023, fp) != NULL)
	{
	  buffer[strlen (buffer) - 1] = '\0'; //create terminator
	  //if the header is found move on
	  if (strstr(buffer,"X") != NULL){ 
	    continue;
	  }
	  
	  //start breaking the line up
	  token = strtok(buffer, " ,\t");
	  
	  for(i=0; i<=2; i++){
	    PMTMat[cntr][i] = atof(token); //store information into a matrix
	    token = strtok(NULL, " ,\t");
	  }
	  
	  cntr++;
	  
	}
    fclose(fp);

    //here we store the corners of the cover

    lngthu = cntr - 7; //if corners are included 
    //lngthu = cntr - 3; //if corners aren't included
    cntr -= 1;
    
    for(i=0;i<4;i++)
      {
	TAX4Tran[pos][2*i+1] = PMTMat[cntr-i][1];
	TAX4Tran[pos][2*i+2] = PMTMat[cntr-i][2];
      }
    /*
    //flip the y axis because Josh flipped it in ImageJ and didn't flip back
    for(i=0;i<=62;i++)
    {
      PMTMat[i][2] = -PMTMat[i][2];
    }
    */
    //now we compute the refined scale and the cluster center simultaneously
    //CCenter will be the average of all of the bisections of pairs of  PMTs that 
    //should lie on the center
    //the scale equation was determined with least squares minimization
    CCenter.x = 0.0;
    CCenter.y = 0.0;
    scalesum1 = 0.0;
    scalesum2 = 0.0;
    for(i=0; i<(lngthu/2); i++)
      {
	//look for the ideal center of the PMT we are currently considering
	j = 0;
	Tmp1.x = M_PI;
	Tmp2.x = M_PI;
	do{
	  if(PMTMat[i][0] == IPMTMat[j][0])
	    {
	      Tmp1.x = IPMTMat[j][1];
	      Tmp1.y = IPMTMat[j][2];
	    }
	  j++;
	}while(Tmp1.x == M_PI);
	
	//look for the the PMT center such that the bisection of the two centers
	//should be at the center of the cluster
	j = 0;
	do{
	  if(Tmp1.x == -IPMTMat[j][1] && Tmp1.y == -IPMTMat[j][2])
	    {
	      Tmp2.x = IPMTMat[j][1];
	      Tmp2.y = IPMTMat[j][2];
	      Numbo = IPMTMat[j][0];
	    }
	  j++;
	}while(Tmp2.x == M_PI);
	
	//look for the partner PMT in the measured PMT list
	for(j=(lngthu/2); j<lngthu; j++)
	  {
	    if(PMTMat[j][0] == Numbo)
	      {
		break;
	      }
	  }
	
	PointA.x = PMTMat[i][1];
	PointA.y = PMTMat[i][2];
	PointB.x = PMTMat[j][1];
	PointB.y = PMTMat[j][2];
	//add the bisection to the sum in the numerator.  The 1/2 will be absorbed
	//into the denominator and accounted for at the end
	CCenter.x += PointA.x+PointB.x;
	CCenter.y += PointA.y+PointB.y;
	
	scalesum1 += d0(Tmp1,Tmp2)*d0(PointA,PointB);
	scalesum2 += pow(d0(PointA,PointB),2.0e0);
      }
    
    //compute scale according to chi2 minimization
    scale = scalesum1/scalesum2;
    printf("scale = %.3f\n",scale);
    TAX4Tran[pos][9] = scale; //store the scale in the matrix

    //add the bisector of PMT 137 and 120 to the average
    CCenter.x += PMTMat[lngthu+2][1]+PMTMat[lngthu+1][1];
    CCenter.y += PMTMat[lngthu+2][2]+PMTMat[lngthu+1][2];
    //add center determined from PMT 136 to the average
    CCenter.x += 2.0*(PMTMat[lngthu][1]+((6.58/8.0)*25.4)*cos(M_PI/3.0));
    CCenter.y += 2.0*(PMTMat[lngthu][2]+((6.58/8.0)*25.4)*sin(M_PI/2.0));
    //finish the average by dividing by the number of points multiplied by 2
    CCenter.x /= 1.0*lngthu+4.0;
    CCenter.y /= 1.0*lngthu+4.0;
    printf("cluster center = (%.3f,%.3f)\n",CCenter.x,CCenter.y);
    //store the center in the matrix
    TAX4Tran[pos][10] = CCenter.x;
    TAX4Tran[pos][11] = CCenter.y;

    //shift all of the measured PMT values so that the cluster center is the origin
    for(i=0;i<(lngthu+3);i++)
      {
	PMTMat[i][1] -= CCenter.x;
	PMTMat[i][2] -= CCenter.y;
      }

    //Now we find the rotation and additional translation to best match the measured points to the ideal points
    
     Smin = 1.0e23;
     thetamin = -1.0e0;
     //find proper rotation angle and offset by minimisation
     for (theta = -2.0e0 * M_PI * 5.0e0 / 360.0e0;
	  theta < 2.0e0 * M_PI * 5.0e0 / 360.0e0;
	  theta += 2.0e0 * M_PI / 36.0e4)
       {
	 //xoff/yoff rotates the original vector and scales it for the measured coordinate system  and then finds the difference in this vector and the measured one
	 //find an average offset for a given theta
	 x0 = 0.0e0;
	 y0 = 0.0e0;
	 for(i=0;i<(lngthu+3);i++)
	   {
	     for(j=0;j<256;j++)
	       {
		 if(PMTMat[i][0] == IPMTMat[j][0])
		   {
		     Tmp1.x = PMTMat[i][1];
		     Tmp1.y = PMTMat[i][2];
		     Tmp2.x = IPMTMat[j][1];
		     Tmp2.y = IPMTMat[j][2];
		     x0 += xoff (Tmp2, Tmp1);
		     y0 += yoff (Tmp2, Tmp1);
		     break;
		   }
	       }
	   }	
	 Tmp1.x = 0.0e0;
	 Tmp1.y = 0.0e0;
	 Tmp2.x = 0.0e0;
	 Tmp2.y = 0.0e0;
	 x0 += xoff (Tmp2, Tmp1);
	 y0 += yoff (Tmp2, Tmp1);
	 x0 /= 1.0*lngthu+4.0;
	 y0 /= 1.0*lngthu+4.0;
	 //xtrans/ytrans sets the average offset to the origin, rotates counterclockwise by theta, and scales the coordinate to the ideal coordinate system
	 //find the difference in the ideal and the transformed measured coordinates
	 S = 0.0e0;
	 for(i=0;i<(lngthu+3);i++)
	   {
	     for(j=0;j<256;j++)
	       {
		 if(PMTMat[i][0] == IPMTMat[j][0])
		   {
		     Tmp1.x = PMTMat[i][1];
		     Tmp1.y = PMTMat[i][2];
		     S += pow(IPMTMat[j][1] - xtrans(Tmp1), 2.0e0);
		     S += pow(IPMTMat[j][2] - ytrans(Tmp1), 2.0e0);
		     break;
		   }
	       }
	   }
	 S += pow(0.0e0 - xtrans(Tmp2), 2.0e0);
	 S += pow(0.0e0 - ytrans(Tmp2), 2.0e0);
	 //if the sum of the squares is smaller than the previous sum then record the sum, the current value of theta, and the offset
	 if (S < Smin)
	   {
	     thetamin = theta;
	     Smin = S;
	     x0min = x0;
	     y0min = y0;
	}
       }
     //save proper transformation angle and offset
     theta = thetamin;
     x0 = x0min;
     y0 = y0min;

     TAX4Tran[pos][12] = x0;
     TAX4Tran[pos][13] = y0;
     TAX4Tran[pos][14] = theta;
     printf("theta = %.5f\n",theta);
     printf("x0 = %.5f\n",x0);
     printf("y0 = %.5f\n",y0);
     printf("Smin = %.5f\n",Smin);
     printf("\n");

#ifdef HOMO

     //make the homography matrix the identity matrix.  This means if we fail to find a good homographic transformation we default to the affine transformation.

     h0 = 1.0;
     h1 = 0.0;
     h2 = 0.0;
     h3 = 0.0;
     h4 = 1.0;
     h5 = 0.0;
     h6 = 0.0;
     h7 = 0.0;
     h8 = 1.0;

     //write the affine tranformed points into a file that the homography program can read

     sprintf (fname, "TranPMT.txt");

     if ((jp = fopen (fname, "w")) == NULL)
       {
	 fprintf (stderr, "ERROR: Unable to write TranPMT.txt %s\7\7\n",
		  fname);
	 exit (-1);
       }

     for (i=0; i<(lngthu+3); i++)
       {
	 Tmp1.x = PMTMat[i][1];
	 Tmp1.y = PMTMat[i][2];
	 fprintf(jp,"%.0f %.5f %.5f\n", PMTMat[i][0], xtrans(Tmp1), ytrans(Tmp1));
       }

     fclose(jp);

     //now we run PMTHomography.py to obtain the homographic transformation parameters

     sprintf(fname, "python ./src/PMTHomography.py");

     pp = popen (fname, "r");
     //read in homographic matrix entries
     if (fgets (buffer, 1023, pp) == NULL)
       {
	 fprintf (stderr,
		  "ERROR: Failed to obtain homographic matrix entries\7\7\n");
	 pclose (pp);
	 
       }
     pclose (pp);

     if (sscanf (buffer, "%lf %lf %lf %lf %lf %lf %lf %lf %lf", &h0, &h1, &h2, &h3, &h4, &h5, &h6, &h7, &h8) != 9)
	{
	  fprintf (stderr,
		   "ERROR: Failed to obtain homographic matrix entries\7\7\n");

	}

     //store the homography matrix entries for the mirror in question
     Homo[pos][0] = h0;
     Homo[pos][1] = h1;
     Homo[pos][2] = h2;
     Homo[pos][3] = h3;
     Homo[pos][4] = h4;
     Homo[pos][5] = h5;
     Homo[pos][6] = h6;
     Homo[pos][7] = h7;
     Homo[pos][8] = h8;

     printf("Homography matrix:\n");
     printf("\n");
     printf("%.4f %.4f %.4f\n", h0, h1, h2);
     printf("%.4f %.4f %.4f\n", h3, h4, h5);
     printf("%.4f %.4f %.4f\n", h6, h7, h8);
     printf("\n");

#endif
      
     pos++;
  }
      
  pclose(lp);
  
  return;
}

/*
void AvShift(double Tran[], double TAX4Tran[10][15], VECTOR corner[5], int mir){
  int i,j,k, index, TranNum;
  double xdiff, ydiff, mag, tmpmag, AvX, AvY, biggo;
  VECTOR temp;
  
  AvX = 1000.0;
  AvY = 1000.0;
  biggo = 1000.0;

  printf("Checking for unaccounted camera movement\n");

  for(i=0;i<10;i++) //iterate over all transformations
    {
      //if it finds the mirror we want compute the average shift in the corners
      if(mir == TAX4Tran[i][0])
	{
	  mag = 1000.0;
	  xdiff = 0.0;
	  ydiff = 0.0;
	  //here we look at each ideal corner and pair it with a measured corner
	  for(j=0;j<4;j++)
	    {
	      temp.x = TAX4Tran[i][2*j+1];
	      temp.y = TAX4Tran[i][2*j+2];
	      //here we find the closest measured corner to the ideal corner
	      for(k=0;k<4;k++)
		{
		  tmpmag = d0(corner[k],temp);
		  if(tmpmag <= mag)
		    {
		      mag = tmpmag;
		      index = k; //store the closest corner
		    }
		}
	      //add difference of the paired corners to a sum
	      xdiff += temp.x - corner[index].x;
	      ydiff += temp.y - corner[index].y;
	      
	    }
	  //divide the sum by four to compute the average corner shift
	  xdiff /= 4.0;
	  ydiff /= 4.0;

	}else
	{
	  continue; //if the transformation is for a different mirror move on
	}
      //if we found a smaller shift than previously then record the transformation and shift
      if(sqrt(pow(xdiff,2.0)+pow(ydiff,2.0)) < biggo)
	{
	  AvX = xdiff;
	  AvY = ydiff;
	  TranNum = i;
	  biggo = sqrt(pow(xdiff,2.0)+pow(xdiff,2.0));
	}
    }
  //note the correct transformation for the image we are considering
  Tran[0] = TranNum;
  //if the average shift is greater than the determined maximum error then we
  //have found an image where the camera moved to a position not corresponding
  //to a PMT image and we record the shift.
  if(abs(AvX) >= 0.997 || abs(AvY) >= 1.108)
    {
      Tran[1] = AvX;
      Tran[2] = AvY;
      printf("movement = (%.3f,%.3f)\n",AvX,AvY);
      printf("Unaccounted Movement Detected!\n");
    }else
    {
      Tran[1] = 1000.0;
      Tran[2] = 1000.0;
    }
 
  if(Tran[1] == 1000.0)
    {
      printf("No significant movement detected");
    }

  return;
}
*/

#endif
