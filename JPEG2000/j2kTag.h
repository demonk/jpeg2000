#ifndef J2K_TAG
#define J2K_TAG
#define J2K_TAG_TREE
#define J2K_TAG_NODE
#define NULL 0

#include <malloc.h>

class j2kTagNode
{
public:
	j2kTagNode *parent;
	int value;
	int low;
	int known;
};

class j2kTagTree
{
public:
	int numleafsh;
	int numleafsv;
	int numnodes;
	j2kTagNode *nodes;
};

class j2kTag
{
public:


public:
	static void resetTag(j2kTagTree *tree);
	static void setValue(j2kTagTree *tree,int leafno,int value);
	static void encode(j2kTagTree *tree,int leafno,int threashOld);
	static void destory(j2kTagTree *tree);
	static j2kTagTree *createTagTree(int numleafsh,int numleafsv);/* numleafsh=在分区下水平代码块数目,sv是垂直下*/

};




#endif
