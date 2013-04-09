#include "j2kPacketIterator.h"

//PacketIterator::PacketIterator(jp2Image *img,CodeParam *cp,int tileno)
piIterator* PacketIterator::piCreate(jp2Image *img,CodeParam *cp,int tileno)
{
	TileCodeParam *tcp=&cp->tcps[tileno];
//	piIterator *pi=(piIterator*)malloc((tcp->numPocs+1)*sizeof(piIterator));

//	pi=new piIterator[tcp->numPocs+1];

		pi=(piIterator*)malloc((tcp->numPocs+1)*sizeof(piIterator));

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
		//pi[pino].comps=new piComponent[img->numComponents];

		for(int compno=0;compno<pi->numComponents;compno++)
		{
			piComponent *comp=&pi[pino].comps[compno];
			tccp=&tcp->tccps[compno];
			comp->dx=img->comps[compno].XRsiz;
			comp->dy=img->comps[compno].YRsiz;
			comp->numResolutions=tccp->numResolutions;
			comp->resolutions=(piResolution*)malloc(comp->numResolutions*sizeof(piResolution));
			//comp->resolutions=new piResolution[comp->numResolutions];

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
			pi[pino].include=(short int *)malloc(img->numComponents*maxResolution*tcp->numLayers*100*sizeof(short int));//!!!!!!!这里有问题了 
			//pi[pino].include=new short int(img->numComponents*maxResolution);

			for(int i=0;i<img->numComponents*maxResolution*tcp->numLayers*100;i++)
			{
				pi[pino].include[i]=0;
			}
		}else{
			pi[pino].include=pi[pino-1].include;
		}

		if(!tcp->pocUse)//标记是不
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

void PacketIterator::piDestory()
{
	free(pi);
}

PacketIterator::PacketIterator()
{

}
PacketIterator::~PacketIterator()
{
//	free(pi);
}

