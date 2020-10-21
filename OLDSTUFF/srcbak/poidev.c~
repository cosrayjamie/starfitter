#include <math.h>
#include <stdlib.h>

#ifndef PI
#define PI 3.14159265358979323846
#endif

double poidev(double xm)
{
  double gammln(double xx);
  static double sq,alxm,g,oldm=(-1.0);
  double em,t,y;
  
  if (xm < 12.0) 
    {
      if (xm != oldm) 
	{
	  oldm=xm;
	  g=exp(-xm);
	}
      em = -1;
      t=1.0;
      do 
	{
	  ++em;
	  t *= drand48();
	} 
      while (t > g);
    } 
  else 
    {
      if (xm != oldm) 
	{
	  oldm=xm;
	  sq=sqrt(2.0*xm);
	  alxm=log(xm);
	  g=xm*alxm-gammln(xm+1.0);
	}
      do 
	{
	  do 
	    {
	      y=tan(PI*drand48());
	      em=sq*y+xm;
	    }
	  while (em < 0.0);
	  em=floor(em);
	  t=0.9*(1.0+y*y)*exp(em*alxm-gammln(em+1.0)-g);
	} 
      while (drand48() > t);
    }
  return em;
}

