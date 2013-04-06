#ifndef J2K_POC
#define J2K_POC
class j2kPOC
{
public:
	int resolutionStart;
	int componentStart;
	int layerEnd;
	int resolutionEnd;
	int componentEnd;
	int progressionOrder;
	int tile;
	char progorder[4];
};
#endif
