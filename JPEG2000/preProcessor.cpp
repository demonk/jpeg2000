#include "preProcessor.h"

int PreProcessor::calProgression(char progression[4])
{
	if (progression[0] == 'L' && progression[1] == 'R'
		&& progression[2] == 'C' && progression[3] == 'P') {
			//LRCP
			return 0;
	} else {
		if (progression[0] == 'R' && progression[1] == 'L'
			&& progression[2] == 'C' && progression[3] == 'P') {
				//RLCP
				return 1;
		} else {
			if (progression[0] == 'R' && progression[1] == 'P'
				&& progression[2] == 'C' && progression[3] == 'L') {
					//RPCL
					return 2;
			} else {
				if (progression[0] == 'P' && progression[1] == 'C'
					&& progression[2] == 'R' && progression[3] == 'L') {
						//PCRL
						return 3;
				} else {
					if (progression[0] == 'C' && progression[1] == 'P'
						&& progression[2] == 'R' && progression[3] == 'L') {
							//CPRL
							return 4;
					} else {
						return -1;
					}
				}
			}
		}
	}
}

void PreProcessor::encodeStepSize(int stepsize,int numbps,int *expn,int *mant)
{
	int p=floorlog2(stepsize)-13;//log2(stepsize)-13;
	int n=11-floorlog2(stepsize);//11-log2(stepsize);
	*expn=numbps-p;
	if(n<0)
	{
		*mant=(stepsize>>-n)&0x7ff;
	}else
	{
		*mant=(stepsize<<n)&0x7ff;
	}
}

void PreProcessor::calStepSizes(TileCompCodeParam *tccp,int prec)
{
	int numbands=3*tccp->numResolutions-1;

	for(int bandno=0;bandno<numbands;bandno++)
	{
		int resno;
		int orient;
		int level;
		int gain;
		double stepsize;

		if(bandno==0)
		{
			resno=0;
			orient=0;
		}
		else
		{
			resno=(bandno-1)/3+1;
			orient=(bandno-1)%3+1;
		}

		level=tccp->numResolutions-1-resno;

		if(tccp->isReversibleDWT==0||orient==0)
			gain=0;
		else
			if(orient==1||orient==2)
				gain=1;
			else
				gain=2;

		double norm=dwtNormsReal[orient][level];
		stepsize=(1<<(gain+1))/norm;

		encodeStepSize(
			(int)floor(stepsize*8192.0),
			prec+gain,
			&tccp->stepsizes[bandno].expn,
			&tccp->stepsizes[bandno].mant
			);
	}
}
