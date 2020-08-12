#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


int main (void);

int
main (void)
{
  FILE *pp, *pp2, *fp;
  char buffer[2048], buffer2[2048];
  char command[2048], fname[1024];
  char outbuf[4096];
  int site, mir, image, year, month, day, hour, minute, second;
  int nhits, n, i, j;
  int nimages[3][15];
  int yyyymmdd[3][15][2000];
  int nimage[3][15][2000];
  int status[3][15][2000];
  int i0, tmp;
  char *sname[3] = { "MD", "BR", "LR" };
  char *csval[2] = { " missing csv", " have csv" };
  int nfiles, mfiles, cfiles;


  for (site = 0; site < 3; site++)
    {
      for (mir = 0; mir < 15; mir++)
	{
	  nimages[site][mir] = 0;
	}
    }

  pp =
    popen
    ("find /home/thomas/Desktop/Max/Mirror_Survey_Photos -type f -name \"img_????.jpg\" -exec exiftool -p \'$TA_SiteNumber $TA_MirrorNumber $filename $dateTimeOriginal\' \'{}\' \\;",
     "r");

  while (fgets (buffer, 2047, pp) != NULL)
    {
      buffer[strlen (buffer) - 1] = '\0';
      if (sscanf
	  (buffer, "%d %d img_%d.jpg %d:%d:%d %d:%d:%d", &site, &mir, &image,
	   &year, &month, &day, &hour, &minute, &second) != 9)
	continue;

      sprintf (command,
	       "find /home/thomas/Desktop/Max/Mirror_Survey_Photos -type f -name \"img_%.4d.???.csv*\"",
	       image);

      pp2 = popen (command, "r");
      nhits = 0;
      while (fgets (buffer2, 2047, pp2) != NULL)
	nhits++;
      pclose (pp2);

      n = nimages[site][mir];

      yyyymmdd[site][mir][n] = year * 10000 + month * 100 + day;
      nimage[site][mir][n] = image;
      status[site][mir][n] = (int) (nhits > 0);

      nimages[site][mir]++;
    }

  pclose (pp);


  for (site = 0; site < 3; site++)
    {
      for (mir = 0; mir < 15; mir++)
	{
	  if ((n = nimages[site][mir]) <= 0)
	    continue;

	  // Brute force sort

	  for (i = 0; i < n; i++)
	    {
	      for (j = i + 1; j < n; j++)
		{
		  if (nimage[site][mir][j] < nimage[site][mir][i])
		    {
		      tmp = nimage[site][mir][j];
		      nimage[site][mir][j] = nimage[site][mir][i];
		      nimage[site][mir][i] = tmp;

		      tmp = yyyymmdd[site][mir][j];
		      yyyymmdd[site][mir][j] = yyyymmdd[site][mir][i];
		      yyyymmdd[site][mir][i] = tmp;

		      tmp = status[site][mir][j];
		      status[site][mir][j] = status[site][mir][i];
		      status[site][mir][i] = tmp;
		    }
		}
	    }

	  outbuf[0] = '\0';
	  mfiles = 0;
	  cfiles = 0;
	  i = 0;

	  for (i0 = 0; i0 < n; i0 = i + 1)
	    {
	      nfiles = 1;
	      for (i = i0; i + 1 < n; i++)
		{
		  if (yyyymmdd[site][mir][i0] != yyyymmdd[site][mir][i + 1])
		    break;
		  if (status[site][mir][i0] != status[site][mir][i + 1])
		    break;
		  nfiles++;
		}

	      if (i0 == i)
		{
		  if (status[site][mir][i0] == 0)
		    {
		      sprintf (buffer, " %.8d/img_%.4d.jpg",
			       yyyymmdd[site][mir][i0],
			       nimage[site][mir][i0]);
		      strcat (outbuf, buffer);
		    }
		  if (status[site][mir][i0] == 0)
		    mfiles++;
		  if (status[site][mir][i0] == 1)
		    cfiles++;
		  continue;
		}

	      if ((yyyymmdd[site][mir][i0] != yyyymmdd[site][mir][i])
		  || (status[site][mir][i0] != status[site][mir][i]))
		{
		  if (status[site][mir][i0] == 0)
		    {
		      sprintf (buffer, " %.8d/img_%.4d.jpg",
			       yyyymmdd[site][mir][i0],
			       nimage[site][mir][i0]);
		      strcat (outbuf, buffer);
		    }

		  if (status[site][mir][i] == 0)
		    {
		      sprintf (buffer, " %.8d/img_%.4d.jpg",
			       yyyymmdd[site][mir][i], nimage[site][mir][i]);
		      strcat (outbuf, buffer);
		    }
		  if (status[site][mir][i0] == 0)
		    mfiles++;
		  if (status[site][mir][i0] == 1)
		    cfiles++;
		  if (status[site][mir][i] == 0)
		    mfiles++;
		  if (status[site][mir][i] == 1)
		    cfiles++;
		  continue;
		}

	      if (status[site][mir][i0] == 0)
		{
		  sprintf (buffer, " %.8d/img_%.4d.jpg-img_%.4d.jpg (%d)",
			   yyyymmdd[site][mir][i0], nimage[site][mir][i0],
			   nimage[site][mir][i], nfiles);
		  strcat (outbuf, buffer);
		}
	      if (status[site][mir][i0] == 0)
		mfiles += nfiles;
	      if (status[site][mir][i0] == 1)
		cfiles += nfiles;
	    }

	  printf ("%s;m%.2d;", sname[site], mir);
	  printf ("%d/%d images missing csv files;", mfiles, mfiles + cfiles);
	  puts (outbuf);
	}
      printf ("\n");
    }


  return 0;
}
