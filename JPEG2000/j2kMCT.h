#ifndef J2K_MCT
#define J2K_MCT
static double mctNorms[3] = { 1.732, .8292, .8292 };
static double mctNormsReal[3] = { 1.732, 1.805, 1.573 };

static double mctGetNorm(int compno)
{
	return mctNorms[compno];
}

static double mctGetNormReal(int compno)
{
	return mctNormsReal[compno];
}
#endif