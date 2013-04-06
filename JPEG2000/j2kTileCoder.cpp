#include "j2kTileCoder.h"

j2kTileCoder::j2kTileCoder(CodeParam *comp,jp2Image *image,int currentTile)
{
	cp=comp;
	img=image;
	currentTileNo=currentTile;
	widthData=NULL;
	lastSizeOfB=0;
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
	TileCodeParam *tcp=&cp->tcps[0];
	TileCompCodeParam *tccp=&tcp->tccps[0];

	TileCodeParam *currentTile;

	currentTileNo=tileno;
	j2ktile=image.tiles;
	tcp=&cp->tcps[tileno];
	j2kTileCoder_Tile *tile=j2ktile;

	for(int compno=0;compno<tile->numComponents;compno++)
	{
		//从文件读入图像原始数据并写入各层分量下整个tile的数据
		j2kTileCoder_Component *tilec=&tile->comps[compno];

		Component component=img->comps[compno];
		int adjust=component.sgnd?0:1<<(component.precision-1);

		int offset_x=int_ceildiv(img->XOsiz,component.XRsiz);//(图像域原点/当前分量的X采样率)
		int offset_y=int_ceildiv(img->YOsiz,component.YRsiz);//公式5.17, 在当前分量采样率下图像域原点的偏移值

		int tileWidth=tilec->x1-tilec->x0;//tile分量的宽度(采样前)
		int tileWidthOnComp=int_ceildiv(img->Xsiz-img->XOsiz,component.XRsiz);//(真实图像的宽度/横向采样点)=在对应分量上的对应宽度(采样后)
		
		char tmp[256];//缓冲区
		sprintf(tmp,"Compo%d",compno);//字符串格式化
		FILE *src=fopen(tmp,"rb");//打开文件Compno0/1/2
		if(!src)
		{
			return -1;
		}

		fseek(src,(tilec->x0-offset_x)+(tilec->y0-offset_y)*tileWidthOnComp,SEEK_SET);

		int k=(tilec->x0-offset_x)+(tilec->y0-offset_y)*tileWidthOnComp;//记录当前原点的偏移位置(采样分辨率下)

		for(int row=tilec->y0;row<tilec->y1;row++)
		{
			for(int col=tilec->x0;col<tilec->x1;col++)
			{
				unsigned char element;
				if(tcp->tccps[compno].isReversibleDWT==1)
				{
					element=fgetc(src);//从文件处读入一个字节
					tilec->data[col-tilec->x0+(row-tilec->y0)*tileWidth]=element-adjust;
					k++;
				}else if(tcp->tccps[compno].isReversibleDWT==0)
				{
					element=fgetc(src);
					tilec->data[col-tilec->x0+(row-tilec->y0)*tileWidth]=(element-adjust)<<13;
					k++;
				}
			}

			fseek(src,
				(tilec->x0-offset_x)+(row+1-offset_y)*tileWidthOnComp -k,
				SEEK_CUR);
			k= tilec->x0-offset_x+(row+1-offset_y)*tileWidthOnComp;
		}
		fclose(src);
	}

	multiCompTransform(tcp,tile);
	discreteWaveletTransform(tcp,tile);

	/*--------Tier1------*/
	/*--------MQEncode---*/
	j2kTierOne *tierOne=new j2kTierOne();
	tierOne->tierOneInitContext();//初始化上下文
	tierOne->tierOneEncodeCodeBlocks(tile,tcp);

	j2kTierTwo *tierTwo=new j2kTierTwo();

	if(tcp->distoratioAlloc||cp->fixed_quality)
	{
		rateAllocate(dest,len,tierTwo);
	}else
	{
		rateAllocateFixed();
	}

	int l=tierTwo->tierTwoEncodePackets(img,cp,tileno,tile,tcp->numLayers,dest,len);

	for(int compno=0;compno<tile->numComponents;compno++)
		{
			tilec=&tile->comps[compno];
			free(tilec->data);
	}

	delete tierOne;
	delete tierTwo;

	return l;
}
void j2kTileCoder::rateAllocateFixed()
{
	for (int layerno=0;layerno<tcp->numLayers;layerno++)
		makeLayerFixed(layerno,1);
}
void j2kTileCoder::rateAllocate(unsigned char *dest,int len,j2kTierTwo *tierTwo)
{
	double min=DBL_MAX;
	double max=0;
	double maxSE=0;

	j2ktile->fixedQuality=0;

	for(int compno=0;compno<j2ktile->numComponents;compno++)
	{
		j2kTileCoder_Component *tilec=&j2ktile->comps[compno];

		tilec->fixedQuality=0;
		for(int resno=0;resno<tilec->numResolutions;resno++)
		{
			j2kTileCoder_Resolution *res=&tilec->resolutions[resno];

			for(int bandno=0;bandno<res->numBands;bandno++)
			{
				j2kTileCoder_Band *band=&res->bands[bandno];

				for(int precno=0;precno<res->precinctWidth*res->precinctHeight;precno++)
				{
					j2kTileCoder_Precinct *prec=&band->precincts[precno];

					for(int cbno=0;cbno<prec->codeBlockNumInWidth*prec->codeBlockNumInHeight;cbno++)
					{
						j2kTileCoder_CodeBlock *codeblock=&prec->codeBlockInfo[cbno];

						for(int passno=0;passno<codeblock->totalpasses;passno++)
						{
							j2kTileCoder_Pass *pass=&codeblock->passes[passno];

							int dr;
							double dd;
							double rdslope;
							if(passno==0)
							{
								dr=pass->rate;
								dd=pass->distortiondec;
							}else{
								dr=pass->rate-codeblock->passes[passno-1].rate;
								dd=pass->distortiondec-codeblock->passes[passno-1].distortiondec;
							}

							if(!dr)
								continue;

							rdslope=dd/dr;

							if(rdslope<min)
								min=rdslope;

							if(rdslope>max)
								max=rdslope;
						}

						//增加fix_quality
						j2ktile->fixedQuality+=((codeblock->x1-codeblock->x0)*(codeblock->y1-codeblock->y0));
						tilec->fixedQuality+=((codeblock->x1-codeblock->x0)*(codeblock->y1-codeblock->y0));
					}
				}
			}
		}

		Component imgc=img->comps[compno];

		maxSE+=(double)((1<<imgc.precision)-1)*((1<<imgc.precision)-1)*tilec->fixedQuality;

	}

	double cumDisto[100];

	for(int layerno=0;layerno<tcp->numLayers;layerno++)
	{
		volatile double lo=min;
		volatile double hi=max;
		volatile int success=0;
		volatile int maxLen=int_min(tcp->rates[layerno],len);
		volatile double goodThresh;
		volatile int goodLen;
		volatile int i;
		double distoTarget=j2ktile->distotile-((K*maxSE)/pow(10.0,tcp->distoratio[layerno]/10.0));

		for(int i=0;i<32;i++)
		{
			volatile double thresh=(lo+hi)/2;
			int l=0;
			double distoAchieved=0;

			makeLayer(layerno,thresh,0);

			if(cp->fixed_quality)
			{
				if(layerno==0)
					distoAchieved=j2ktile->distolayer[0];
				else
					distoAchieved=cumDisto[layerno-1]+j2ktile->distolayer[layerno];

				if(distoAchieved<distoTarget)
				{
					hi=thresh;
					continue;
				}
				lo=thresh;
			}else{
				l=tierTwo->tierTwoEncodePackets(img,cp,currentTileNo,j2ktile,layerno+1,dest,maxLen);

				if(l==-999)
				{
					lo=thresh;
					continue;
				}
				hi=thresh;
			}

			success=1;
			goodThresh=thresh;
			goodLen=1;
		}
		if(!success)
			printf ("ERROR\n");

		makeLayer(layerno,goodThresh,1);

		if(layerno==0)
		{
			cumDisto[layerno]=j2ktile->distolayer[0];
		}else{
			cumDisto[layerno]=cumDisto[layerno-1]+j2ktile->distolayer[layerno];
		}
	}
}
void j2kTileCoder::makeLayer(int layerno,double thresh,int final)
{
	j2ktile->distolayer[layerno]=0;//设置fixed_quality;

	for(int compno=0;compno<j2ktile->numComponents;compno++)
	{
		j2kTileCoder_Component *tilec=&j2ktile->comps[compno];
		for(int resno=0;resno<tilec->numResolutions;resno++)
		{
			j2kTileCoder_Resolution *res=&tilec->resolutions[resno];
			for(int bandno=0;bandno<res->numBands;bandno++)
			{
				j2kTileCoder_Band *band=&res->bands[bandno];
				for(int precno=0;precno<res->precinctHeight*res->precinctWidth;precno++)
				{
					j2kTileCoder_Precinct *prec=&band->precincts[precno];
					for(int cbno=0;cbno<prec->codeBlockNumInWidth*prec->codeBlockNumInHeight;cbno++)
					{
						j2kTileCoder_CodeBlock *codeblock=&prec->codeBlockInfo[cbno];
						j2kTileCoder_Layer *layer=&codeblock->layers[layerno];

						if(!layerno)
							codeblock->numpassesinlayers=0;

						int n=codeblock->numpassesinlayers;
						for(int passno=codeblock->numpassesinlayers;passno<codeblock->totalpasses;passno++)
						{
							j2kTileCoder_Pass *pass=&codeblock->passes[passno];
							int dr;
							double dd;
							if(n==0)
							{
								dr=pass->rate;
								dd=pass->distortiondec;
							}else{
								dr=pass->rate-codeblock->passes[n-1].rate;
								dd=pass->distortiondec-codeblock->passes[n-1].distortiondec;
							}

							if(!dr)
							{	if(dd)
									n=passno+1;
								continue;
							}

							if(dd/dr>thresh)
								n=passno+1;
						}
						layer->numpasses=n-codeblock->numpassesinlayers;

						if(!layer->numpasses)
						{
							layer->disto=0;
							continue;
						}

						if(!codeblock->numpassesinlayers)
						{
							layer->len=codeblock->passes[n-1].rate;
							layer->data=codeblock->data;
							layer->disto=codeblock->passes[n-1].distortiondec;
						}else{
							layer->len=codeblock->passes[n-1].rate-codeblock->passes[codeblock->numpassesinlayers-1].rate;
							layer->data=codeblock->data+codeblock->passes[codeblock->numpassesinlayers-1].rate;
							layer->disto=codeblock->passes[n-1].distortiondec-codeblock->passes[codeblock->numpassesinlayers-1].distortiondec;
						}
						j2ktile->distolayer[layerno]+=layer->disto;

						if(final)
							codeblock->numpassesinlayers=n;
						}
					}
				}
			}
		}
	}
void j2kTileCoder::makeLayerFixed(int layerno,int final)
{
	int matrice[10][10][3];
	int value;

	for(int compno=0;compno<j2ktile->numComponents;compno++)
	{
		j2kTileCoder_Component *tilec=&j2ktile->comps[compno];
		for(int i=0;i<tcp->numLayers;i++)
		{
			for(int j=0;j<tilec->numResolutions;j++)
			{
				for(int k=0;k<3;k++)
				{
					matrice[i][j][k]=(int)(cp->matrice[i+tilec->numResolutions*3+
					j*3+
					k]*
					(float)(img->comps[compno].precision/16.0));
					
				}
			}
		}

		for(int resno=0;resno<tilec->numResolutions;resno++)
		{
			j2kTileCoder_Resolution *res=&tilec->resolutions[resno];
			for(int bandno=0;bandno<res->numBands;bandno++)
			{
				j2kTileCoder_Band *band=&res->bands[bandno];
				for(int precno=0;precno<res->precinctHeight*res->precinctWidth;precno++)
				{
					j2kTileCoder_Precinct *prec=&band->precincts[precno];
					for(int cbno=0;cbno<prec->codeBlockNumInHeight*prec->codeBlockNumInWidth;cbno++)
					{
						j2kTileCoder_CodeBlock *codeblock=&prec->codeBlockInfo[cbno];
						j2kTileCoder_Layer *layer=&codeblock->layers[layerno];

						int imsb=img->comps[compno].precision-codeblock->numbps;

						if(!layerno)
						{
							value=matrice[layerno][resno][bandno];
							if(imsb>=value)
								value=0;
							else
								value-=imsb;
						}else{
							value=matrice[layerno][resno][bandno]-
								  matrice[layerno-1][resno][bandno];

							if(imsb>=matrice[layerno-1][resno][bandno])
							{
								value-=(imsb-matrice[layerno-1][resno][bandno]);

								if(value<0)
									value=0;
							}
						}

						int n;
						if(!layerno)
							codeblock->numpassesinlayers=0;
						
						n=codeblock->numpassesinlayers;
						if(!codeblock->numpassesinlayers){
							if(value!=0)
								n=3*value-2+codeblock->numpassesinlayers;
							else
								n=codeblock->numpassesinlayers;
						}else
							n=3*value+codeblock->numpassesinlayers;

						layer->numpasses=n-codeblock->numpassesinlayers;
						if(!layer->numpasses)
							continue;

						if(!codeblock->numpassesinlayers)
						{
							layer->len=codeblock->passes[n-1].rate;
							layer->data=codeblock->data;
						}else{
							layer->len=codeblock->passes[n-1].rate-
									   codeblock->passes[codeblock->numpassesinlayers-1].rate;

							layer->data=codeblock->data+
										codeblock->passes[codeblock->numpassesinlayers-1].rate;
						}

						if(final)
							codeblock->numpassesinlayers=n;

						}
					}
				}
			}
		}
	}

void j2kTileCoder::multiCompTransform(TileCodeParam *tcp,j2kTileCoder_Tile *tile)
{
	if(tcp->isMCT)
	{
		if(tcp->tccps[0].isReversibleDWT==0)
		{
			multiEncodeReal(tile);
		}else{
			multiEncode(tile);
		}
	}
}
void j2kTileCoder::multiEncodeReal(j2kTileCoder_Tile *tile)
{
	int *data0=tile->comps[0].data;
	int *data1=tile->comps[1].data;
	int *data2=tile->comps[2].data;
	int size=(tile->comps[0].x1-tile->comps[0].x0)*(tile->comps[0].y1-tile->comps[0].y0);

	for (int i=0;i<size;i++)
	{
		int r=data0[i];
		int g=data1[i];
		int b=data2[i];

		//RGB转YUV
		int y=fix_mul(r,2449)+fix_mul(g,4809)+fix_mul(b,934);
		int u=-fix_mul(r,1382)-fix_mul(g,2714)+fix_mul(b,4096);
		int v=fix_mul(r,4096)-fix_mul(g,3430)-fix_mul(b,666);

		data0[i]=y;
		data1[i]=u;
		data2[i]=v;
	}
}
void j2kTileCoder::multiEncode(j2kTileCoder_Tile *tile)
{
	int *data0=tile->comps[0].data;
	int *data1=tile->comps[1].data;
	int *data2=tile->comps[2].data;
	int size=(tile->comps[0].x1-tile->comps[0].x0)*(tile->comps[0].y1-tile->comps[0].y0);

	for (int i=0;i<size;i++)
	{
		int r=data0[i];
		int g=data1[i];
		int b=data2[i];

		//RCT
		int y=(r+(g<<1)+b)>>2;//(r+g*2+b)/4
		int u=b-g;
		int v=r-g;

		data0[i]=y;
		data1[i]=u;
		data2[i]=v;
	}
}

void j2kTileCoder::discreteWaveletTransform(TileCodeParam *tcp,j2kTileCoder_Tile *tile)
{
	for(int compno=0;compno<tile->numComponents;compno++)
	{
		j2kTileCoder_Component *tilec=&tile->comps[compno];
		if(tcp->tccps[compno].isReversibleDWT==0)
		{
			dwtEncodeReal(tilec);
		}else if(tcp->tccps[compno].isReversibleDWT==1)
		{
			dwtEncode(tilec);
		}
	}
}
void j2kTileCoder::dwtEncodeReal(j2kTileCoder_Component *tilec)
{
	int *data=tilec->data;
	int width=tilec->x1-tilec->x0;
	int height=tilec->y1-tilec->y0;
	int numResolution=tilec->numResolutions-1;

	int resolutionWidth;
	int resolutionHeight;
	int resolutionLowWidth;/*比上一级(即上面两个)分辨率层次低*/
	int resolutionLowHeight;
	for(int i=0;i<numResolution;i++)
	{
		j2kTileCoder_Resolution curRes=tilec->resolutions[numResolution-i];
		j2kTileCoder_Resolution lowRes=tilec->resolutions[numResolution-i-1];

		resolutionWidth=curRes.x1-curRes.x0;
		resolutionHeight=curRes.y1-curRes.y0;

		resolutionLowWidth=lowRes.x1-lowRes.x0;
		resolutionLowHeight=lowRes.y1-lowRes.y0;

		int ivCol=curRes.x0%2;//{0,1},=0:在水平上非反转滤波;=1:在低通与高通间反转滤波
		int ivRow=curRes.y0%2;//{0,1},=0:在垂直上非反转滤波;=1:在低通与高通间反转滤波

		for(int j=0;j<resolutionWidth;j++)
		{
			dwtEncodeLowReal(data+j,resolutionHeight,width,resolutionLowHeight,ivCol);
		}

		for(int j=0;j<resolutionHeight;j++)
		{
			dwtEncodeLowReal(data+j*width,resolutionWidth,numResolution,resolutionLowWidth,ivRow);
		}
	}
}
void j2kTileCoder::dwtEncode(j2kTileCoder_Component *tilec)
{
	int *data=tilec->data;
	int width=tilec->x1-tilec->x0;
	int height=tilec->y1-tilec->y0;
	int numResolution=tilec->numResolutions-1;

	int resolutionWidth;
	int resolutionHeight;
	int resolutionLowWidth;/*比上一级(即上面两个)分辨率层次低*/
	int resolutionLowHeight;
	for(int i=0;i<numResolution;i++)
	{
		j2kTileCoder_Resolution curRes=tilec->resolutions[numResolution-i];
		j2kTileCoder_Resolution lowRes=tilec->resolutions[numResolution-i-1];

		resolutionWidth=curRes.x1-curRes.x0;
		resolutionHeight=curRes.y1-curRes.y0;

		resolutionLowWidth=lowRes.x1-lowRes.x0;
		resolutionLowHeight=lowRes.y1-lowRes.y0;

		int ivCol=curRes.x0%2;//{0,1},=0:在水平上非反转滤波;=1:在低通与高通间反转滤波
		int ivRow=curRes.y0%2;//{0,1},=0:在垂直上非反转滤波;=1:在低通与高通间反转滤波

		for(int j=0;j<resolutionWidth;j++)
		{
			dwtEncodeLow(data+j,resolutionHeight,width,resolutionLowHeight,ivCol);
		}

		for(int j=0;j<resolutionHeight;j++)
		{
			dwtEncodeLow(data+j*width,resolutionWidth,numResolution,resolutionLowWidth,ivRow);
		}
	}
	dwtClean();
}
void j2kTileCoder::dwtEncodeLow(int *data,int resWidth,int numRes,int resLowWidth,int iv)
{
	int dn=resWidth-resLowWidth;
	int sn=resLowWidth;
	int i=0;
	if(iv==1)
	{
		if(!sn&&dn==1)
		{
			S(i)*=2;
		}else
		{
			for(i=0;i<dn;i++)
				S(i)-=(DD_(i)+DD_(i-1))>>1;// 除以2

			for(i=0;i<sn;i++)
				D(i)+=(SS_(i)+SS_(i+1)+2)>>2;//除以4
		}
	}else{
		if(dn>0||sn>1)
		{
			for(i=0;i<dn;i++)
				D(i)-=(S_(i)+S_(i+1))>>1;//除以2

			for(i=0;i<sn;i++)
				S(i)+=(D_(i-1)+D_(i)+2)>>2;//除以4
		}
	}
	dwtLazyTransform(data,resWidth,numRes,resLowWidth,iv);
}
void j2kTileCoder::dwtEncodeLowReal(int *data,int resWidth,int numRes,int resLowWidth,int iv)
{
	int dn=resWidth-resLowWidth;//偶信号集
	int sn=resLowWidth;//奇信号集

	//判断是否反转
	if(iv==1)
	{
		if(sn>0||dn>1)
		{
			//{dn}:偶信号集,{sn}:   奇信号集
			//基于9/7可逆滤波器的正变换,公式(6.14~6.19)
			for(int i=0;i<dn;i++)
				S(i)-=fix_mul(DD_(i)+DD_(i-1),alpha);

			for(int i=0;i<sn;i++)
				D(i)-=fix_mul(SS_(i)+SS_(i+1),beta);

			for(int i=0;i<dn;i++)
				S(i)+=fix_mul(DD_(i)+DD_(i-1),gama);

			for(int i=0;i<sn;i++)
				D(i)+=fix_mul(SS_(i)+SS_(i+1),sita);

			for(int i=0;i<dn;i++)
				S(i)=fix_mul(S(i),K0);

			for(int i=0;i<sn;i++)
				D(i)=fix_mul(D(i),K1);
		}
	}else{
		if(dn>0||sn>1)
		{
			for(int i=0;i<dn;i++)
				D(i)-=fix_mul(S_(i)+S_(i+1),alpha);

			for(int i=0;i<sn;i++)
				S(i)-=fix_mul(D_(i-1)+D_(i),beta);

			for(int i=0;i<dn;i++)
				D(i)+=fix_mul(S_(i)+S_(i+1),gama);

			for(int i=0;i<sn;i++)
				S(i)+=fix_mul(D_(i-1)+D_(i),sita);

			for(int i=0;i<dn;i++)
				D(i)=fix_mul(D(i),K0);

			for(int i=0;i<sn;i++)
				S(i)=fix_mul(S(i),K1);
		}
	}
	dwtLazyTransform(data,resWidth, numRes,resLowWidth, iv);
}
void j2kTileCoder::dwtLazyTransform(int *data,int resWidth,int numRes,int resLowWidth,int iv)
{
	int dn=resWidth-resLowWidth;
	int sn=resLowWidth;

	if(lastSizeOfB!=resWidth)
	{
		if(widthData!=NULL)
			free(widthData);
		widthData=(int*)malloc(resWidth*sizeof(int));
		lastSizeOfB=resWidth;
	}

	if(iv==1)
	{
		for(int i=0;i<sn;i++)
			widthData[i]=data[(2*i+1)*resWidth];
		
		for(int i=0;i<dn;i++)
			widthData[sn+i]=data[2*i*resWidth];
	}else{
		for(int i=0;i<sn;i++)
			widthData[i]=data[2*i*resWidth];

		for(int i=0;i<dn;i++)
			widthData[sn+i]=data[(2*i+1)*numRes];
	}

	for(int i=0;i<resWidth;i++)
	{
		data[i*resWidth]=widthData[i];
	}
}
void j2kTileCoder::dwtClean()
{
	if(widthData!=NULL)
		free(widthData);

	widthData=NULL;
	lastSizeOfB=0;
}

int j2kTileCoder::dwtGetGain(int orient)
{
	if (orient == 0)//LL
		return 0;
	if (orient == 1 || orient == 2)//HL,LH
		return 1;
	return 2;//HH
}