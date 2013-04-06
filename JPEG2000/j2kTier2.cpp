#include "j2kTier2.h"

int j2kTierTwo::tierTwoEncodePackets(jp2Image *img,CodeParam *cp,int tileno,j2kTileCoder_Tile *tile,int maxlayers,unsigned char *dest,int len)
{
	unsigned char *c=dest;
	piIterator *pi=piCreate(img,cp,tileno);

	int e=0;
	for(int pino=0;pino<=cp->tcps[tileno].numPocs;pino++)
	{
		while(piNext(&pi[pino]))
		{
			if(pi[pino].layerno<maxlayers)
			{
				tierTwoEncodePacket(tile,&cp->tcps[tileno],pi[pino].compno,pi[pino].resno,pi[pino].precno,pi[pino].layerno,c,dest+len-c,tileno);

				if(e==-999)
					break;
				else
					c+=e;
			}
		}

		for(int compno=0;compno<pi[pino].numComponents;compno++)
		{
			free(pi[pino].comps[compno].resolutions);

		}
		free(pi[pino].comps);
	}

	free(pi[0].include);
	free(pi);
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

	j2kTag *tag=new j2kTag();

	if(!layerno)
	{
		for(int bandno=0;bandno<res->numBands;bandno++)
		{
			j2kTileCoder_Band *band=&res->bands[bandno];
			j2kTileCoder_Precinct *prec=&band->precincts[precno];

			tag->resetTag(prec->inclusionTree);
			tag->resetTag(prec->imsbTree);

			for(int cbno=0;cbno<prec->codeBlockNumInWidth*prec->codeBlockNumInHeight;cbno++)
			{
				j2kTileCoder_CodeBlock *codeblock=&prec->codeBlockInfo[cbno];

				codeblock->numpasses=0;
				tag->setValue(prec->imsbTree,cbno,band->numbps-codeblock->numbps);
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
				tag->setValue(prec->inclusionTree,cbno,layerno);
		}

		for(int cbno=0;cbno<prec->codeBlockNumInHeight*prec->codeBlockNumInWidth;cbno++)
		{
			j2kTileCoder_CodeBlock *codeblock=&prec->codeBlockInfo[cbno];
			j2kTileCoder_Layer *layer=&codeblock->layers[layerno];

			
			
			
			int passno;

			if(!codeblock->numpasses)
			{
				tag->encode(prec->inclusionTree,cbno,layerno+1);
			}else{
				bitInputOutput::writeBits(layer->numpasses!=0,1);
			}

			if(!layer->numpasses)
			{
				continue;;
			}

			if(!codeblock->numpasses)
			{
				codeblock->numlenbits=3;
				tag->encode(prec->imsbTree,cbno,999);
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

			for(int passno=codeblock->numpasses;passno<codeblock->numpasses;passno++)
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

			if(bitInputOutput::flush())
				return -999;

			c+=bitInputOutput::getPosition();
		}
	}
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
		bitInputOutput::writeBits(0xc||(n-3),4);
	else if(n<=36)
		bitInputOutput::writeBits(0x1e0|(n-6),9);
	else if(n<=164)
		bitInputOutput::writeBits(0xff80|(n-37),16);

}
piIterator* j2kTierTwo::piCreate(jp2Image *img,CodeParam *cp,int tileno)
{
	TileCodeParam *tcp=&cp->tcps[tileno];
	piIterator *pi=(piIterator*)malloc((tcp->numPocs+1)*sizeof(piIterator));

	int row,col;
	TileCompCodeParam *tccp;
	int maxResolution=0;

	for(int pino=0;pino<tcp->numPocs+1;pino++)
	{
		
		row=tileno/cp->tw;
		col=tileno%cp->tw;;

		pi[pino].tx0=int_max(cp->XTOsiz+col*cp->XTsiz,img->XOsiz);
		pi[pino].ty0=int_max(cp->YTOsiz+row*cp->YTsiz,img->YOsiz);
		pi[pino].tx1=int_max(cp->XTOsiz+(col+1)*cp->XTsiz,img->Xsiz);
		pi[pino].ty1=int_max(cp->YTOsiz+(row+1)*cp->YTsiz,img->Ysiz);

		pi[pino].numComponents=img->numComponents;
		pi[pino].comps=(piComponent*)malloc(img->numComponents*sizeof(piComponent));

		for(int compno=0;compno<pi->numComponents;compno++)
		{
			piComponent *comp=&pi[pino].comps[compno];
			tccp=&tcp->tccps[compno];
			comp->dx=img->comps[compno].XRsiz;
			comp->dy=img->comps[compno].YRsiz;
			comp->numResolutions=tccp->numResolutions;
			comp->resolutions=(piResolution*)malloc(comp->numResolutions*sizeof(piResolution));

			if(comp->numResolutions>maxResolution)
			{
				maxResolution=comp->numResolutions;
			}

			int tcx0=int_ceildiv(pi->tx0,comp->dx);
			int tcy0=int_ceildiv(pi->ty0,comp->dy);
			int tcx1=int_ceildiv(pi->tx1,comp->dx);
			int tcy1=int_ceildiv(pi->ty1,comp->dy);

			for(int resno=0;resno<comp->numResolutions;resno++)
			{
				piResolution *res=&comp->resolutions[resno];
				if(tccp->codingStyle&J2K_CCP_CSTY_PRT)
				{
					res->precinctWidth =tccp->precinctWidth[resno];
					res->precinctHeight=tccp->precinctHeight[resno];

				}else{
					res->precinctWidth=15;
					res->precinctHeight=15;
				}

				int levelno=comp->numResolutions-1-resno;

				int rx0=int_ceildivpow2(tcx0,levelno);
				int ry0=int_ceildivpow2(tcy0,levelno);
				int rx1=int_ceildivpow2(tcx1,levelno);
				int ry1=int_ceildivpow2(tcy1,levelno);

				int px0=int_floordivpow2(rx0,res->precinctWidth)<<res->precinctWidth;
				int py0=int_floordivpow2(ry0,res->precinctHeight)<<res->precinctHeight;
				int px1=int_ceildivpow2(rx1,res->precinctWidth)<<res->precinctWidth;
				int py1=int_ceildivpow2(ry1,res->precinctHeight)<<res->precinctHeight;

				if(rx0==rx1)
					res->pw=0;
				else
					res->pw=(px1-px0)>>res->precinctWidth;

				if(ry0==ry1)
					res->ph=0;
				else
					res->ph=(py1-py0)>>res->precinctHeight;
				
			}
		}

		tccp=&tcp->tccps[0];
		pi[pino].stepPrecinct=1;
		pi[pino].stepComponent=100*pi[pino].stepPrecinct;
		pi[pino].stepResolution=img->numComponents*pi[pino].stepComponent;
		pi[pino].stepLayer=maxResolution*pi[pino].stepResolution;

		if(!pino)
		{
			pi[pino].include=(short int *)malloc(img->numComponents*maxResolution*sizeof(short int));

			for(int i=0;i<img->numComponents*maxResolution*tcp->numLayers*100;i++)
			{
				pi[pino].include[i]=0;
			}
		}else{
			pi[pino].include=pi[pino-1].include;
		}

		if(tcp->pocUse)//标记是不
		{
			pi[pino].first=1;
			pi[pino].poc.resolutionStart=0;
			pi[pino].poc.componentStart=0;
			pi[pino].poc.layerEnd=tcp->numLayers;
			pi[pino].poc.resolutionEnd=maxResolution;
			pi[pino].poc.componentEnd=img->numComponents;
			pi[pino].poc.progressionOrder=tcp->progressionOrder;
		}else{
			pi[pino].first=1;
			pi[pino].poc.resolutionStart=tcp->pocs[pino].resolutionStart;
			pi[pino].poc.componentStart=tcp->pocs[pino].componentStart;
			pi[pino].poc.layerEnd=tcp->pocs[pino].layerEnd;
			pi[pino].poc.resolutionEnd=tcp->pocs[pino].resolutionEnd;
			pi[pino].poc.componentEnd=tcp->pocs[pino].componentEnd;
			pi[pino].poc.progressionOrder=tcp->progressionOrder;
		}
		}
		return pi;
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
		return 0;
	}else
		pi->first=0;

	for(pi->layerno=0;pi->layerno<pi->poc.layerEnd;pi->layerno++)
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
				for(pi->precno=0;pi->precno<res->precinctWidth*res->ph;pi->precno++)
				{
					//TODO:可去判断
					if(!pi->include[
						pi->layno*pi->stepLayer+
						pi->resno*pi->stepResolution+
						pi->compno*pi->stepComponent+
						pi->precno*pi->stepPrecinct
					])
					{
						pi->include[pi->layno*pi->stepLayer+
							pi->resno*pi->stepResolution+
							pi->compno*pi->stepComponent+
							pi->precno*pi->stepPrecinct]=1;

					}
						return 1;
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
	for(pi->layerno=0;pi->layerno<pi->poc.layerEnd;pi->layerno++)
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
					if(!pi->include[
						pi->layno*pi->stepLayer+
							pi->resno*pi->stepResolution+
							pi->compno*pi->stepComponent+
							pi->precno*pi->stepPrecinct
					])
					{
						pi->include[pi->layno*pi->stepLayer+
							pi->resno*pi->stepResolution+
							pi->compno*pi->stepComponent+
							pi->precno*pi->stepPrecinct]=1;

						}
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