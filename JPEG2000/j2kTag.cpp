#include "j2kTag.h"

void j2kTag::resetTag(j2kTagTree *tree)
{
	int i;
	/* new */
	if (!tree || tree == NULL)
		return;

	for (i = 0; i < tree->numnodes; i++) {
		tree->nodes[i].value = 999;
		tree->nodes[i].low = 0;
		tree->nodes[i].known = 0;
	}
}

j2kTagTree * j2kTag::createTagTree(int sh,int sv)
{
	int nplh[32];
	int nplv[32];
	j2kTagNode *node;
	j2kTagNode *parentnode;
	j2kTagNode *parentnode0;
	j2kTagTree *tree;
	int i, j, k;
	int numlvls;
	int n;

	tree = (j2kTagTree *) malloc(sizeof(j2kTagTree));
	tree->numleafsh = sh;
	tree->numleafsv = sv;

	numlvls = 0;
	nplh[0] = sh;
	nplv[0] = sv;
	tree->numnodes = 0;
	do {
		n = nplh[numlvls] * nplv[numlvls];
		nplh[numlvls + 1] = (nplh[numlvls] + 1) / 2;
		nplv[numlvls + 1] = (nplv[numlvls] + 1) / 2;
		tree->numnodes += n;
		++numlvls;
	} while (n > 1);

	/* ADD */
	if (tree->numnodes == 0) {
		free(tree);
		return NULL;
	}

	tree->nodes = (j2kTagNode *) malloc(tree->numnodes * sizeof(j2kTagNode));

	node = tree->nodes;
	parentnode = &tree->nodes[tree->numleafsh * tree->numleafsv];
	parentnode0 = parentnode;

	for (i = 0; i < numlvls - 1; ++i) {
		for (j = 0; j < nplv[i]; ++j) {
			k = nplh[i];
			while (--k >= 0) {
				node->parent = parentnode;
				++node;
				if (--k >= 0) {
					node->parent = parentnode;
					++node;
				}
				++parentnode;
			}
			if ((j & 1) || j == nplv[i] - 1) {
				parentnode0 = parentnode;
			} else {
				parentnode = parentnode0;
				parentnode0 += nplh[i];
			}
		}
	}
	node->parent = 0;

	resetTag(tree);

	return tree;
}

