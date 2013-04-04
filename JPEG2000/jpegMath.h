
#ifndef J2K_MATH
#define J2K_MATH

#ifdef WIN32
#define int64 __int64
#else
#define int64 long long
#endif

#include <math.h>
int floorlog2(int a);
int int_ceildiv(int a,int b);
int int_floorlog2(int a);
 int int_min(int a, int b);
int int_max(int a, int b);
int int_floordivpow2(int a, int b);
int int_ceildivpow2(int a, int b);
int fix_mul(int a, int b);/* 用于简化RGB转YUV的计算 */
int int_abs(int a);
#endif