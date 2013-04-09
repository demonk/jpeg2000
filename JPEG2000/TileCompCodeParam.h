
#ifndef TCCP
#define TCCP
#include "StepSize.h"

class TileCompCodeParam
{
public:
	int codingStyle;
	int numResolutions;
	int codeBlockWidth;
	int codeBlockHeight;
	int codeBlockStyle;
	int quantisationStyle;
	int numGuardBits;/* number of Guard bits */
	int isROI;
	int isReversibleDWT;
	int precinctWidth[33];
	int precinctHeight[33];

	j2kStepSize stepsizes[3*33-2];
};
#endif
