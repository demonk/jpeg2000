#include "j2kTileCoder.h"

j2kTileCoder::j2kTileCoder(CodeParam *comp,jp2Image *image,int currentTile)
{
	cp=comp;
	img=image;
	currentTileNo=currentTile;
}

void j2kTileCoder::tcdMallocEncode()
{
	image.tw=cp->tw;
	image.th=cp->th;
	image.tiles=(j2kTileCoder_Tile*)malloc(sizeof(j2kTileCoder_Tile));

	for(int tileno=0;tileno<1;tileno++)
	{
		TileCodeParam *tcp=&cp->tcps[currentTileNo];

		int row=currentTileNo/cp->tw;
		int col=currentTileNo%cp->tw;

		tile=image.tiles;

		tile->x0=int_max(cp->XTOsiz+col*cp->XTsiz,img->XOsiz);
		tile->y0=int_max(cp->YTOsiz+row*cp->YTsiz,img->YOsiz);
		tile->x1=int_max(cp->XTOsiz+(col+1)*cp->XTsiz,img->Xsiz);
		tile->y1=int_max(cp->YTOsiz+(row+1)*cp->YTsiz,img->Ysiz);

		tile->numComponents=img->numComponents;

		for(int j=0;j<tcp->numLayers;j++)
		{
			tcp->rates[j]=int_ceildiv(
				tile->numComponents*(tile->x1-tile->x0)*(tile->y1-tile->y0)*img->comps[0].precision,
				tcp->rates[j]*8*img->comps[0].XRsiz*img->comps[0].YRsiz
				);
			if(j&&tcp->rates[j]<tcp->rates[j-1]+10)
			{
				tcp->rates[j]=tcp->rates[j-1]+20;
			}else{
				if(!j&&tcp->rates[j]<30)
				{
					tcp->rates[j]=30;
				}
			}
		}
		tile->comps=(j2kTileCoder_Component*)malloc(img->numComponents*sizeof(j2kTileCoder_Component));

		j2kTag *tag=new j2kTag();

		for(int compno=0;compno<tile->numComponents;compno++)
		{
			//遍历分量
			TileCompCodeParam *tccp=&tcp->tccps[compno];

			tilec=&tile->comps[compno];
			tilec->x0=int_ceildiv(tile->x0,img->comps[compno].XRsiz);
			tilec->y0=int_ceildiv(tile->y0,img->comps[compno].YRsiz);
			tilec->x1=int_ceildiv(tile->x1,img->comps[compno].XRsiz);
			tilec->y1=int_ceildiv(tile->y1,img->comps[compno].YRsiz);

			tilec->data=(int *)malloc((tilec->x1-tilec->x0)*(tilec->y1-tilec->y0)*sizeof(int));
			tilec->numResolutions=tccp->numResolutions;
			tilec->resolutions=(j2kTileCoder_Resolution*)malloc(tilec->numResolutions*sizeof(j2kTileCoder_Resolution));

			for(int resno=0;resno<tilec->numResolutions;resno++)
			{
				//遍历分辨率
				int precinctWidth;
				int precinctHeight;

				int levelno=tilec->numResolutions-1-resno;

				resolution=&tilec->resolutions[resno];//当前分辨率层次下的引用 
				resolution->x0=int_ceildiv(tilec->x0,levelno);
				resolution->y0=int_ceildiv(tilec->y0,levelno);
				resolution->x1=int_ceildiv(tilec->x1,levelno);
				resolution->y1=int_ceildiv(tilec->y1,levelno);

				if(resno==0)
				{
					resolution->numBands=1;
				}else{
					resolution->numBands=3;
				}
				if(tccp->codeBlockStyle&J2K_CCP_CSTY_PRT)
				{
					precinctWidth=tccp->precinctWidth[resno];
					precinctHeight=tccp->precinctHeight[resno];
				}else{
					precinctWidth=15;
					precinctHeight=15;
				}

				//公式5.27
				int topLeftPreX0=int_floordivpow2(resolution->x0,precinctWidth)<<precinctWidth;
				int topLeftPreY0=int_floordivpow2(resolution->y0,precinctHeight)<<precinctHeight;
				int bottomRightPreX1=int_ceildivpow2(resolution->x1,precinctWidth)<<precinctWidth;
				int bottomRightPreY1=int_ceildivpow2(resolution->y1,precinctHeight)<<precinctHeight;

				resolution->precinctWidth=(bottomRightPreX1-topLeftPreX0)>>precinctWidth;
				resolution->precinctHeight=(bottomRightPreY1-topLeftPreY0)>>precinctHeight;

				//整体而言代码块(区域)的原点与终点
				int topLeftCodeBlockX0;
				int topLeftCodeBlockY0;
				int bottomRightCodeBlockX1;
				int bottomRightCodeBlockY1;

				int PPx,PPy;

				if(resno==0)
				{
					topLeftCodeBlockX0=topLeftPreX0;
					topLeftCodeBlockY0=topLeftPreY0;
					bottomRightCodeBlockX1=bottomRightPreX1;
					bottomRightCodeBlockY1=bottomRightPreY1;

					PPx=precinctWidth;
					PPy=precinctHeight;
				}else{
					topLeftCodeBlockX0=int_ceildivpow2(topLeftPreX0,1);
					topLeftCodeBlockY0=int_ceildivpow2(topLeftPreY0,1);
					bottomRightCodeBlockX1=int_ceildivpow2(bottomRightPreX1,1);
					bottomRightCodeBlockY1=int_ceildivpow2(bottomRightPreY1,1);

					PPx=precinctWidth-1;
					PPy=precinctHeight-1;
				}

				//公式5.28,即代码块的大小(2^xcb_,2^ycd_),大书P321
				int xcb_=int_min(tccp->codeBlockWidth,PPx);
				int ycb_=int_min(tccp->codeBlockHeight,PPy);

				for(int bandno=0;bandno<resolution->numBands;bandno++)
				{
					//遍历子带
					band=&resolution->bands[bandno];
					if(resno==0)
						band->bandno=0;
					else
						band->bandno=bandno+1;//从1开始 

					int x0b,y0b;

					if(band->bandno==1||band->bandno==3)
					{
						x0b=1;
					}else{
						x0b=0;
					}

					if(band->bandno==2||band->bandno==3)
					{
						x0b=1;
					}else{
						y0b=0;
					}

					if(band->bandno==0)
					{
						band->x0=int_ceildivpow2(tilec->x0,levelno);
						band->y0=int_ceildivpow2(tilec->y0,levelno);
						band->x1=int_ceildivpow2(tilec->x1,levelno);
						band->y1=int_ceildivpow2(tilec->y1,levelno);
					}else{
						int a=1<<levelno;//=2^levelno
						band->x0=int_ceildivpow2(tilec->x0-a*x0b,levelno+1);
						band->y0=int_ceildivpow2(tilec->y0-a*y0b,levelno+1);
						band->x1=int_ceildivpow2(tilec->x1-a*x0b,levelno+1);
						band->y1=int_ceildivpow2(tilec->y1-a*y0b,levelno+1);
					}

					j2kStepSize *stepSize;
					if(resno==0)
					{
						stepSize=&tccp->stepsizes[0];
					}else{
						stepSize=&tccp->stepsizes[3*(resno-1)+bandno+1];
					}

					int gain;
					if(tccp->isReversibleDWT==0)
					{
						gain=0;
					}else{
						dwtGetGain(band->bandno);
					}

					int numbps=img->comps[compno].precision+gain;
					band->stepSize=(int)floor((1.0+stepSize->mant/2048.0)*pow(2.0,numbps-stepSize->expn)*8192.0);
					band->numbps=stepSize->expn+tccp->numGuardBits-1;
					band->precincts=(j2kTileCoder_Precinct*)malloc(3*resolution->precinctWidth*resolution->precinctHeight*sizeof(j2kTileCoder_Precinct));

					for(int i=0;i<resolution->precinctWidth*resolution->precinctHeight*3;i++)
					{
						band->precincts[i].imsbTree=NULL;
						band->precincts[i].inclusionTree=NULL;
					}

					for(int precno=0;precno<resolution->precinctWidth*resolution->precinctHeight;precno++)
					{
						//遍历分区,计算
						int row=precno/resolution->precinctWidth;
						int col=precno%resolution->precinctHeight;

						//当前代码块的起始点与终结点,其中(1<<PPx)是指代码块的大小为2^PPx
						int codeBlockX0=topLeftCodeBlockX0+col*(1<<PPx);
						int codeBlockY0=topLeftCodeBlockY0+row*(1<<PPy);
						int codeBlockX1=codeBlockX0+(1<<PPx);
						int codeBlockY1=codeBlockY0+(1<<PPy);

						precinct=&band->precincts[precno];

						//使子区的范围收在子带范围内
						precinct->x0=int_max(codeBlockX0,band->x0);
						precinct->y0=int_max(codeBlockY0,band->y0);
						precinct->x1=int_min(codeBlockX1,band->x1);
						precinct->y1=int_min(codeBlockY1,band->y1);

						int topLeftCodeBlockNumStartX=int_floordivpow2(precinct->x0,xcb_)<<xcb_;//计算当前分区起点处和原点的水平之间有可以有多少个子区
						int topLeftCodeBlockNumStartY=int_floordivpow2(precinct->y0,ycb_)<<ycb_;
						int bottomRightCodeBlockNumEndX=int_ceildivpow2(precinct->x1,xcb_)<<xcb_;
						int bottomRightCodeBlockNumEndY=int_ceildivpow2(precinct->y1,ycb_)<<ycb_;

						//计算当前子区下有多少个代码块(水平/垂直)
						precinct->codeBlockNumInWidth=(bottomRightCodeBlockNumEndX-topLeftCodeBlockNumStartX)>>xcb_;
						precinct->codeBlockNumInHeight=(bottomRightCodeBlockNumEndY-topLeftCodeBlockNumStartY)>>ycb_;

						precinct->codeBlockInfo=(j2kTileCoder_CodeBlock*)malloc(precinct->codeBlockNumInWidth*precinct->codeBlockNumInHeight*sizeof(j2kTileCoder_CodeBlock));

						precinct->inclusionTree=tag->createTagTree(precinct->codeBlockNumInWidth,precinct->codeBlockNumInHeight);
						precinct->imsbTree=tag->createTagTree(precinct->codeBlockNumInWidth,precinct->codeBlockNumInHeight);

						for(int codeBlockNo=0;codeBlockNo<precinct->codeBlockNumInWidth*precinct->codeBlockNumInHeight;codeBlockNo++)
						{
							//遍历代码块
							int row=codeBlockNo/precinct->codeBlockNumInWidth;
							int col=codeBlockNo%precinct->codeBlockNumInWidth;

							//在分区下每个代码块的位置(全局位置下)
							int codeBlockX0=
								topLeftCodeBlockNumStartX+//分区左边还拥有多少个代码块大小(已经乘上大小<<xcb_)
								col*(1<<xcb_); //分区内间隔多少个代码块大小 

							int codeBlockY0=
								topLeftCodeBlockNumStartY+
								row*
								(1<<ycb_);

							int codeBlockX1=codeBlockX0+(1<<xcb_);
							int codeBlockY1=codeBlockY0+(1<<ycb_);

							codeblock=&precinct->codeBlockInfo[codeBlockNo];

							//限制代码块在分区内
							codeblock->x0=int_max(codeBlockX0,precinct->x0);
							codeblock->y0=int_max(codeBlockY0,precinct->y0);
							codeblock->x1=int_min(codeBlockX1,precinct->x1);
							codeblock->y1=int_min(codeBlockY1,precinct->y1);
						}
					}
				}
			}
		}
		delete tag;
	}
}

void j2kTileCoder::tcdInitEncode()
{

}

int j2kTileCoder::tcdEncodeTilePxm(int tileno,unsigned char *dest,int len)
{
	return 1000;
}

int j2kTileCoder::dwtGetGain(int orient)
{
	if (orient == 0)//LL
		return 0;
	if (orient == 1 || orient == 2)//HL,LH
		return 1;
	return 2;//HH
}