#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "eph_manager.h"
#include "novas.h"
#include "julian.h"

#define R2D (180.0e0/M_PI)
#define D2R (M_PI/180.0e0)

typedef struct SUNPOS
{
  double jday;
  double az, zn;
} SUNPOS;

int main (int argc, char *argv[]);
double get_double (char buffer[], int ibeg, int iend);
int get_int (char buffer[], int ibeg, int iend);

int
main (int argc, char *argv[])
{
  const int accuracy = 0;	// 0 is for highest accuracy
  FILE *pp, *fp;
  char *pname[12] = { "none",
    "Mercury",
    "Venus",
    "Earth",
    "Mars",
    "Jupiter",
    "Saturn",
    "Uranus",
    "Neptune",
    "Pluto",
    "Sun",
    "Moon"
  };

  int year = 2011;
  int month = 4;
  int day = 24;
  double MJD;
  int leap_secs = 33;

  double latitude, longitude, height;

  int error = 0;
  short int de_num = 0;

  double ut1_utc = -0.387845;
  double tai_utc = 34.0e0;

  double x_pole = -0.002;
  double y_pole = +0.529;

  double jd_beg, jd_end, jd_utc, jd_tt, jd_ut1, delta_t, rat, dect, dist, zd,
    az, rar, decr;

  on_surface geo_loc;
  observer obs_loc;

  cat_entry dummy_star;
  object planet[12];

  int i, getP;
  char buffer[2048];

  double temperature = 20.0e0;
  double pressure = 840.0e0;

  double jd0;
  int year0, month0, day0;

  int site;

  double zen1, zen2, jday;
  double jdmin, jdmax;

  double JDNOW;

  int hour, minute;
  double second;

  double timespan;

  static SUNPOS max[10000], min[10000];
  static double curjday[10000], zntrack[10000], aztrack[10000];

  int Ntrk = 0;
  int max_Ntrk = 0;
  int min_Ntrk = 0;

  double znmin;
  double max_znmin;
  double min_znmin;

  int k;
  double fazi, falt;
  double dot, dotmax, jday_max;
  double sun_east, sun_north, sun_up;
  double flash_east, flash_north, flash_up;
  double sun_altmax, sun_azimax;

  if ((argc != 5) && (argc != 7))
    {
      fprintf (stderr,
	       "%s <longitude (deg)> <latitude (deg)> <height (m)> <timespan (months)> [<flasher azimuth (deg)> <flasher zenith (deg)>]\7\7\7\\n\n",
	       argv[0]);
      fprintf (stderr, "Version = 1.00\n");
      exit (-1);
    }

  longitude = atof (argv[1]);
  latitude = atof (argv[2]);
  height = atof (argv[3]);
  timespan = atof (argv[4]);

  if (argc == 7)
    {
      fazi = atof (argv[5]);
      falt = 90.0e0 - atof (argv[6]);
    }

  // Set observer location and weather conditions

  make_on_surface (latitude, longitude, height, temperature, pressure,
		   &geo_loc);
  make_observer_on_surface (latitude, longitude, height, temperature,
			    pressure, &obs_loc);

  //   Make structures of type 'object' for the planets, Sun, and Moon.

  make_cat_entry ("DUMMY", "xxx", 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
		  &dummy_star);

  for (i = 1; i < 12; i++)
    {
      if ((error =
	   make_object (0, i, pname[i], &dummy_star, &planet[i])) != 0)
	{
	  printf ("Error %d from make_object (%s)\n", error, pname[i]);
	  exit (-1);
	}
    }

  /*
     Open the JPL binary ephemeris file, here named "JPLEPH".
     Remove this block for use with solsys version 2.
   */

  if ((error = ephem_open ("JPLEPH", &jd_beg, &jd_end, &de_num)) != 0)
    {
      if (error == 1)
	fprintf (stderr, "JPL ephemeris file not found.\n");
      else
	fprintf (stderr, "Error reading JPL ephemeris file header.\n");
      exit (-1);
    }


  // Obtain time parameters

  leap_secs = getP = 0;

  if (access ("leapsec.dat", R_OK) != 0)
    {
      if (system
	  ("wget -O leapsec.dat http://maia.usno.navy.mil/ser7/leapsec.dat")
	  != 0)
	{
	  fprintf (stderr, " Failed to obtain leapsec.dat\n");
	  exit (-1);
	}
    }

  if ((pp = fopen ("leapsec.dat", "r")) != NULL)
    {
      while (fgets (buffer, 2047, pp) != NULL)
	{
	  if (sscanf (buffer, " %*d %*s %*d %*s %lf %*s %lf", &jd0, &tai_utc)
	      != 2)
	    continue;
	  if (jd0 < jd_utc)
	    leap_secs = (int) floor (tai_utc);
	}
      fclose (pp);
    }
  else
    {
      fprintf (stderr, " Failed to read leapsec.dat\n");
      exit (-1);
    }

  //  printf("Leap Seconds = %d\n", leap_secs);

  if (access ("finals2000A.data", R_OK) != 0)
    {
      if (system
	  ("wget -O finals2000A.data http://maia.usno.navy.mil/ser7/finals2000A.data 2>/dev/null")
	  != 0)
	{
	  fprintf (stderr, " Failed to obtain finals2000A.data\n");
	  exit (-1);
	}
    }

  getP = 0;
  if ((pp = fopen ("finals2000A.data", "r")) != NULL)
    {
      while (fgets (buffer, 2047, pp) != NULL)
	{
	  if (strlen (buffer) < 176)
	    continue;

	  month0 = get_int (buffer, 3, 4);
	  if (month0 != month)
	    continue;

	  day0 = get_int (buffer, 5, 6);
	  if (day0 != day)
	    continue;

	  year0 = get_int (buffer, 1, 2);
	  MJD = get_double (buffer, 8, 15);

	  if ((int) floor (MJD) <= 51543)
	    year0 += 1900;
	  else
	    year0 += 2000;
	  if (year0 != year)
	    continue;

	  x_pole = get_double (buffer, 19, 27);
	  y_pole = get_double (buffer, 38, 46);

	  ut1_utc = get_double (buffer, 59, 68);

	  getP = 1;
	  break;
	}
      fclose (pp);
    }
  else
    {
      fprintf (stderr, " Failed to read finals2000A.data\n");
      exit (-1);
    }

  // jd_utc = julian_date (year, month, day, hour);


  i = 10;			// Sun

  //  jd_utc = julian_date (year, month, day, hour);

  JDNOW = (time (NULL) + 2.0e0) / 86400.0e0 + 2440587.5e0 - 0.5e0;	// Move half a day into the past - assuming this is being run in the day

  Ntrk = 0;
  znmin = (100.0e0);

  max_Ntrk = 0;
  max_znmin = (-1.0e0);

  min_Ntrk = 0;
  min_znmin = (100.0e0);

  dotmax = 0.0e0;
  jday_max = 0.0e0;

  for (jday = JDNOW; jday < JDNOW + timespan * 31.0e0; jday += 60.0e0 / 86400.0e0)	// Every minute, for timespan months into the future
    {
      jd_utc = jday;

      if (jd_utc < 2414992.5e0)
	exit (-1);		// Ephemeris limits
      if (jd_utc > 2469808.5e0)
	exit (-1);		// Ephemeris limits


      jd_utc = jday;
      jd_tt = jd_utc + ((double) leap_secs + 32.184e0) / 86400.0e0;
      jd_ut1 = jd_utc + ut1_utc / 86400.0e0;
      delta_t = 32.184e0 + leap_secs - ut1_utc;

      if ((error =
	   topo_planet (jd_tt, &planet[i], delta_t, &geo_loc, accuracy, &rat,
			&dect, &dist)) != 0)
	{
	  fprintf (stderr, "Error %d from topo_planet.", error);
	  exit (-1);
	}

      equ2hor (jd_ut1, delta_t, accuracy, x_pole, y_pole, &geo_loc, rat, dect,
	       2, &zntrack[Ntrk], &aztrack[Ntrk], &rar, &decr);

      if (argc == 7)
	{
	  flash_up = sin (D2R * falt);
	  flash_north = cos (D2R * falt) * cos (D2R * fazi);
	  flash_east = cos (D2R * falt) * sin (D2R * fazi);

	  sun_up = cos (D2R * zntrack[Ntrk]);
	  sun_north = sin (D2R * zntrack[Ntrk]) * cos (D2R * aztrack[Ntrk]);
	  sun_east = sin (D2R * zntrack[Ntrk]) * sin (D2R * aztrack[Ntrk]);

	  dot =
	    flash_up * sun_up + flash_north * sun_north +
	    flash_east * sun_east;

	  if (dot > dotmax)
	    {
	      dotmax = dot;
	      jday_max = jd_ut1;
	      sun_altmax = 90.0e0 - zntrack[Ntrk];
	      sun_azimax = aztrack[Ntrk];
	    }
	}

      if (zntrack[Ntrk] < 90.0e0)	// Sun is above the horizon
	{
	  if (zntrack[Ntrk] < znmin)
	    znmin = zntrack[Ntrk];
	  curjday[Ntrk] = jday;
	  Ntrk++;
	}
      else			// Sun is below the horizon
	{
	  if (Ntrk > 150)	// We have a track
	    {
	      if (max_znmin < znmin)	// Is the minimum zenith for this track greater than the overall maximum, minimum zenith?
		{
		  for (k = 0; k < Ntrk; k++)
		    {
		      max[k].jday = curjday[k];
		      max[k].zn = zntrack[k];
		      max[k].az = aztrack[k];
		    }
		  max_Ntrk = Ntrk;
		  max_znmin = znmin;
		}

	      if (min_znmin > znmin)	// Is the minimum zenith for this track less than the overall minimum zenith?
		{
		  for (k = 0; k < Ntrk; k++)
		    {
		      min[k].jday = curjday[k];
		      min[k].zn = zntrack[k];
		      min[k].az = aztrack[k];
		    }
		  min_Ntrk = Ntrk;
		  min_znmin = znmin;
		}
	    }

	  Ntrk = 0;
	  znmin = (100.0e0);
	}

    }

  if (argc == 5)
    {
      // Print out the two tracks

      fp = fopen ("min_track.dat", "w");
      for (k = 0; k < min_Ntrk; k++)
	{
	  jdate ((min[k].jday - 2440000.0e0), &year, &month, &day, &hour,
		 &minute, &second);
	  fprintf (fp, "%.12lf %04d%02d%02d %02d:%02d:%.1lf %.3lf %.3lf\n",
		   jday, year, month, day, hour, minute, second, min[k].az,
		   min[k].zn);
	}
      fclose (fp);

      fp = fopen ("max_track.dat", "w");
      for (k = 0; k < max_Ntrk; k++)
	{
	  jdate ((max[k].jday - 2440000.0e0), &year, &month, &day, &hour,
		 &minute, &second);
	  fprintf (fp, "%.12lf %04d%02d%02d %02d:%02d:%.1lf %.3lf %.3lf\n",
		   jday, year, month, day, hour, minute, second, max[k].az,
		   max[k].zn);
	}
      fclose (fp);
    }

  ephem_close ();		/* remove this line for use with solsys version 2 */


  // Print out angle of closest approach and time of closest approach

  jdate ((jday_max - 2440000.0e0), &year, &month, &day, &hour, &minute,
	 &second);
  printf
    ("Closest: %.12lf %04d%02d%02d %02d:%02d:%.1lf %.3lf degs SUN: azi: %.3lf alt: %.3lf\n",
     jdmax, year, month, day, hour, minute, second, R2D * acos (dotmax),
     sun_azimax, sun_altmax);

  return (0);
}


double
get_double (char buffer[], int ibeg, int iend)
{
  char *sub;
  int len;
  double result;

  ibeg--;
  iend--;
  len = strlen (buffer);
  if (ibeg < 0)
    return 0.0e0;
  if (iend >= len)
    return 0.0e0;
  if (iend - ibeg < 0)
    return 0.0e0;

  sub = strndup (&buffer[ibeg], iend - ibeg + 1);

  result = atof (sub);
  free (sub);

  return result;
}

int
get_int (char buffer[], int ibeg, int iend)
{
  char *sub;
  int len;
  int result;

  ibeg--;
  iend--;

  len = strlen (buffer);
  if (ibeg < 0)
    return 0.0e0;
  if (iend >= len)
    return 0.0e0;
  if (iend - ibeg < 0)
    return 0.0e0;

  sub = strndup (&buffer[ibeg], iend - ibeg + 1);

  result = atoi (sub);
  free (sub);

  return result;
}
