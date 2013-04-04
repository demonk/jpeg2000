#include "jpegMath.h"


int floorlog2(int a)
{
	int l;
	for(l=0;a>1;l++)
	{
		a>>=1;
	}
	return l;
}

int int_ceildiv(int a,int b)
{
  return (a + b - 1) / b;

}

int int_floorlog2(int a)
{
	int l;
	for (l = 0; a > 1; l++) {
		a >>= 1;//ÓÒÒÆ
	}
	return l;
}

int int_min(int a, int b)
{
	return a < b ? a : b;
}

int int_max(int a, int b)
{
	return a > b ? a : b;

}

int int_ceildivpow2(int a, int b)
{
	return (a + (1 << b) - 1) >> b;
}

int int_floordivpow2(int a, int b)
{
	return a >> b;
}

int fix_mul(int a, int b)
{
	double tmp= (double) ((int64) a * (int64) b);
	int64 v = (int64) ((fabs(tmp/8192.0)>=floor(fabs(tmp/8192.0))+0.5)?fabs(tmp/8192.0)+1.0:fabs(tmp/8192.0));
	v = (tmp<0)?-v:v;
	return (int) v;
}

int int_abs(int a)
{
	return a < 0 ? -a : a;
}