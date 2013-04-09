#include "j2kTier2.h"
#include "bitInputOutput.h"

j2kTierTwo::j2kTierTwo()
{
	packetIterator=new PacketIterator();
}

int j2kTierTwo::tierTwoEncodePackets(jp2Image *img,CodeParam *cp,int tileno,j2kTileCoder_Tile *tile,int maxlayers,unsigned char *dest,int len)
{
	unsigned char *c=dest;
	//piIterator *pi=piCreate(img,cp,tileno);

//	packetIterator=new PacketIterator(img,cp,tileno);

	//piIterator *pi=packetIterator->getIterator();
	piIterator *pi=packetIterator->piCreate(img,cp,tileno);

	int e=0;
	for(int pino=0;pino<=cp->tcps[tileno].numPocs;pino++)
	{
		while(piNext(&pi[pino]))
		{
			if(pi[pino].layno<maxlayers)
			{
				e=tierTwoEncodePacket(tile,
					&cp->tcps[tileno],
					pi[pino].compno,
					pi[pino].resno,
					pi[pino].precno,
					pi[pino].layno,
					c,
					dest+len-c,
					tileno);

				if(e==-999)
					break;
				else
					c+=e;
			}
		}
	}

	packetIterator->piDestory();
	if(e==-999)
		return e;
	else
		return c-dest;
}
int j2kTierTwo::tierTwoEncodePacket(j2kTileCoder_Tile *tile,TileCodeParam *tcp,int compno,int resno,int precno,int layerno,unsigned char *dest,int len,int tileno)
{
	j2kTileCoder_Component *tilec=&tile->comps[compno];
	j2kTileCoder_Resolution *res=&tilec->resolutions[resno];
	unsigned char *c=dest;

	if(!layerno)
	{
		for(int bandno=0;bandno<res->numBands;bandno++)
		{
			j2kTileCoder_Band *band=&res->bands[bandno];
			j2kTileCoder_Precinct *prec=&band->precincts[precno];

			j2kTag::resetTag(prec->inclusionTree);
			j2kTag::resetTag(prec->imsbTree);

			for(int cbno=0;cbno<prec->codeBlockNumInWidth*prec->codeBlockNumInHeight;cbno++)
			{
				j2kTileCoder_CodeBlock *codeblock=&prec->codeBlockInfo[cbno];

				codeblock->numpasses=0;
				j2kTag::setValue(prec->imsbTree,cbno,band->numbps-codeblock->numbps);
			}
		}
	}

	bitInputOutput::initEncoder(c,len);
	bitInputOutput::writeBits(1,1);//写出一个空的头比特 

	/*-------写包头-----*/
	for(int bandno=0;bandno<res->numBands;bandno++)
	{
		j2kTileCoder_Band *band=&res->bands[bandno];
		j2kTileCoder_Precinct *prec=&band->precincts[precno];

		for(int cbno=0;cbno<prec->codeBlockNumInWidth*prec->codeBlockNumInHeight;cbno++)
		{
			j2kTileCoder_CodeBlock *codeblock=&prec->codeBlockInfo[cbno];
			j2kTileCoder_Layer *layer=&codeblock->layers[layerno];

			if(!codeblock->numpasses&&layer->numpasses)
				j2kTag::setValue(prec->inclusionTree,cbno,layerno);
		}

		for(int cbno=0;cbno<prec->codeBlockNumInHeight*prec->codeBlockNumInWidth;cbno++)
		{
			j2kTileCoder_CodeBlock *codeblock=&prec->codeBlockInfo[cbno];
			j2kTileCoder_Layer *layer=&codeblock->layers[layerno];

			int passno;

			if(!codeblock->numpasses)
			{
				j2kTag::encode(prec->inclusionTree,cbno,layerno+1);
			}else{
				bitInputOutput::writeBits(layer->numpasses!=0,1);
			}

			if(!layer->numpasses)
			{
				continue;
			}

			if(!codeblock->numpasses)
			{
				codeblock->numlenbits=3;
				j2kTag::encode(prec->imsbTree,cbno,999);
			}

			putNumPasses(layer->numpasses);

			int nump=0;
			int len=0;
			int increment=0;
			for(int passno=codeblock->numpasses;passno<codeblock->numpasses+layer->numpasses;passno++)
			{
				j2kTileCoder_Pass *pass=&codeblock->passes[passno];
				nump++;
				len+=pass->len;
				if(pass->term||passno==(codeblock->numpasses+layer->numpasses)-1)
				{
					int tmp=int_floorlog2(len)+1-(codeblock->numlenbits+int_floorlog2(nump));
					increment=int_max(increment,tmp);
					len=0;
					nump=0;
				}
			}
			putCommaCode(increment);

			codeblock->numlenbits+=increment;

			for(int passno=codeblock->numpasses;passno<codeblock->numpasses+layer->numpasses;passno++)
			{
				j2kTileCoder_Pass *pass=&codeblock->passes[passno];
				nump++;
				len+=pass->len;
				if(pass->term||passno==(codeblock->numpasses+layer->numpasses)-1)
				{
					bitInputOutput::writeBits(len,codeblock->numlenbits+int_floorlog2(nump));
					len=0;
					nump=0;

				}
			}
		}
	}

	if(bitInputOutput::flush())
		return -999;

	c+=bitInputOutput::getPosition();

	/*------结束写包头-----*/

	/*------写包体------*/
	for(int bandno=0;bandno<res->numBands;bandno++)
	{
		j2kTileCoder_Band *band=&res->bands[bandno];
		j2kTileCoder_Precinct *prec=&band->precincts[precno];

		for(int cbno=0;cbno<prec->codeBlockNumInHeight*prec->codeBlockNumInWidth;cbno++)
		{
			j2kTileCoder_CodeBlock *codeblock=&prec->codeBlockInfo[cbno];
			j2kTileCoder_Layer *layer=&codeblock->layers[layerno];

			if(!layer->numpasses)
				continue;

			if((c+layer->len)>(dest+len))
				return -999;

			memcpy(c,layer->data,layer->len);//复制layer中的数据到缓冲区中

			codeblock->numpasses+=layer->numpasses;

			c+=layer->len;
		}
	}
	/*-------结束写包体-------*/
	return c-dest;
}

void j2kTierTwo::putCommaCode(int n)
{
	while(--n>=0)
		bitInputOutput::writeBits(1,1);

	bitInputOutput::writeBits(0,1);
}

void j2kTierTwo::putNumPasses(int n)
{
	if(n==1)
		bitInputOutput::writeBits(0,1);
	else if(n==2)
		bitInputOutput::writeBits(2,2);
	else if(n<=5)
		bitInputOutput::writeBits(0xc|(n-3),4);
	else if(n<=36)
		bitInputOutput::writeBits(0x1e0|(n-6),9);
	else if(n<=164)
		bitInputOutput::writeBits(0xff80|(n-37),16);

}

int j2kTierTwo::piNext(piIterator* pi)
{
	switch (pi->poc.progressionOrder)
	{
	case 0:
		return piNext_LRCP(pi);
	case 1:
		return piNext_RLCP(pi);
	case 2:
		return piNext_RPCL(pi);
	case 3:
		return piNext_PCRL(pi);
	case 4:
		return piNext_CPRL(pi);

	}
	return 0;
}

int j2kTierTwo::piNext_LRCP(piIterator* pi)
{
	piComponent *comp;
	piResolution *res;

	if(!pi->first)
	{
		// 如果是第一个
		comp=&pi->comps[pi->compno];
		res=&comp->resolutions[pi->resno];
		goto skip;
	}else
		pi->first=0;

	for(pi->layno=0;pi->layno<pi->poc.layerEnd;pi->layno++)
	{
		for(pi->resno=pi->poc.resolutionStart;pi->resno<pi->poc.resolutionEnd;pi->resno++)
		{
			for(pi->compno=pi->poc.componentStart;pi->compno<pi->poc.componentEnd;pi->compno++)
			{
				comp=&pi->comps[pi->compno];
				if(pi->resno>=comp->numResolutions)
				{
					continue;
				}

				res=&comp->resolutions[pi->resno];
				for(pi->precno=0;pi->precno<res->ph*res->pw;pi->precno++)
				{
					if(!pi->include[pi->layno*pi->stepLayer+pi->resno*pi->stepResolution+pi->compno*pi->stepComponent+pi->precno*pi->stepPrecinct])
					{    int a=     pi->layno*pi->stepLayer+pi->resno*pi->stepResolution+pi->compno*pi->stepComponent+pi->precno*pi->stepPrecinct;
					     pi->include[a]=1;
						 return 1;
					}
					skip:;
				}
			}
		}
	}
	return 0;
}

int j2kTierTwo::piNext_RLCP(piIterator* pi)
{
	piComponent *comp;
	piResolution *res;

	if(!pi->first)
	{
		// 如果是第一个
		comp=&pi->comps[pi->compno];
		res=&comp->resolutions[pi->resno];
		return 0;
	}else
		pi->first=0;
	for(pi->resno=pi->poc.resolutionStart;pi->resno<pi->poc.resolutionEnd;pi->resno++)
	{
		for(pi->layno=0;pi->layno<pi->poc.layerEnd;pi->layno++)
		{


			for(pi->compno=pi->poc.componentStart;pi->compno<pi->poc.componentEnd;pi->compno++)
			{
				comp=&pi->comps[pi->compno];
				if(pi->resno>=comp->numResolutions)
				{
					continue;
				}

				res=&comp->resolutions[pi->resno];
				for(pi->precno=0;pi->precno<res->precinctWidth*res->ph;pi->precno++)
				{
					//TODO:可去判断
					/*			if(!pi->include[
					pi->layno*pi->stepLayer+
					pi->resno*pi->stepResolution+
					pi->compno*pi->stepComponent+
					pi->precno*pi->stepPrecinct
					])
					{*/
					pi->include[pi->layno*pi->stepLayer+
						pi->resno*pi->stepResolution+
						pi->compno*pi->stepComponent+
						pi->precno*pi->stepPrecinct]=1;

					//	}
					return 1;
				}
			}
		}
	}
	return 0;
}

int j2kTierTwo::piNext_RPCL(piIterator* pi)
{
	//待实现
	return 0;
}

int j2kTierTwo::piNext_PCRL(piIterator* pi)
{
	//待实现
	return 0;
}

int j2kTierTwo::piNext_CPRL(piIterator* pi)
{
	//待实现
	return 0;
}

j2kTierTwo::~j2kTierTwo()
{
	delete packetIterator;
}