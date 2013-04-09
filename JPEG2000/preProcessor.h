#ifndef J2K_PRE_PROCESSOR
#define J2K_PRE_PROCESSOR

#include "TileCompCodeParam.h"
#include "CodeParam.h"
#include "jpegMath.h"
#include "j2kDWT.h"
#include "bmpReader.h"
#include "jp2Writer.h"

class PreProcessor
{
private:
	void encodeStepSize(int stepsize,int numbps,int *expn,int *mant);

public:
	int calProgression(char progression[4]);
	void calStepSizes(TileCompCodeParam *tccp,int prec);
};
#endif