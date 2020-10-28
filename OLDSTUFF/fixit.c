#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int main (int argc, char *argv[]);

int
main (int argc, char *argv[])
{
  FILE *fp, *op;
  char buffer[1204], fname[1024], command[1024], oname[1024];
  char *s;

  if (argc != 2)
    exit (-1);

  if ((fp = fopen (argv[1], "r")) == NULL)
    exit (-1);
  sprintf (fname, "%s.new", argv[1]);
  if ((op = fopen (fname, "w")) == NULL)
    exit (-1);

  while (fgets (buffer, 1023, fp) != NULL)
    {
      if (strstr
	  (buffer,
	   "0	Area	X	Y	XM	YM	IntDen	RawIntDen	Slice	Label")
	  != NULL)
	{
	  fprintf (op,
		   "	Area	X	Y	XM	YM	IntDen	RawIntDen	Slice		Label\n");
	}
      else
	fputs (buffer, op);
    }

  fclose (op);
  fclose (fp);

  strcpy (oname, argv[1]);
  if ((s = strstr (oname, ".badcorners")) != NULL)
    *s = '\0';
  sprintf (command, "mv %s %s", fname, oname);


  return system (command);
}
