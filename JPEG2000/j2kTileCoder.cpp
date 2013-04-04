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

		int offset_x=int_ceildiv(img->x0,component.XRsiz);//(图像域原点/当前分量的X采样率)
		int offset_y=int_ceildiv(img->y0,component.YRsiz);//公式5.17, 在当前分量采样率下图像域原点的偏移值

		int tileWidth=tilec->x1-tilec->x0;//tile分量的宽度(采样前)
		int tileWidthOnComp=int_ceildiv(img->x1-img->x0,component.XRsiz);//(真实图像的宽度/横向采样点)=在对应分量上的对应宽度(采样后)

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

	return 1000;
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
		int u=-fix_mul(r,1382)-fix_mul(g,2714)+fix_mul(b,4096);//Cr
		int v=fix_mul(r,4096),fix_mul(g,3430)-fix_mul(b,666);

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
		j2kTileCoder_Tile *tilec=&tile->comps[compno];
		if(tcp->tccps[compno].isReversibleDWT==0)
		{
			dwtEncodeReal(tilec);
		}else if(tcp->tccps[compno].isReversibleDWT==1)
		{
			dwtEncode(tilec);
		}
	}
}
void j2kTileCoder::dwtEncodeReal(j2kTileCoder_Tile *tilec)
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
void j2kTileCoder::dwtEncode(j2kTileCoder_Tile *tilec)
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

	if(iv==1)
	{
		if(!sn&&dn==1)
		{
			S(i)*=2;
		}else
		{
			for(int i=0;i<dn;i++)
				S(i)-=(DD_(i)+DD_(i-1))>>1;// 除以2

			for(int i=0;i<sn;i++)
				D(i)+=(SS_(i)+SS_(i+1)+2)>>2;//除以4
		}
	}else{
		if(dn>0||sn>!)
		{
			for(int i=0;i<dn;i++)
				D(i)-=(S_(i)+S_(i+1))>>1;//除以2

			for(int i=0;i<sn;i++)
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
	int sn=res;

	if(lastSizeOfB!=n)
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
			widthData[sn+i]=data[(2*i+1)*x];
	}

	for(int i=0;i<resWidth;i++)
	{
		data[i*resWidth]=resWidth[i];
	}
}
void j2kTileCoder::dwtClean()
{
	if(widthData!=NULL)
		free(widthData);

	widthData=NULL;
	lastSizeOfB=0;
}

void j2kTileCoder::tierOneInitContext()
{
	//初始化ZC上下文,有9个
	for(int bandno=0;bandno<4;bandno++)
	{
		for(int ctx=0;ctx<256;++ctx)
		{
			zcContextNo[(bandno<<8)|ctx]=initCtxZC(ctx,bandno);
		}
	}

	//初始化SC上下文,有5个
	for(int ctx=0;ctx<256;ctx++)
	{
		scContextNo[ctx]=initCtxSC(ctx<<4);//左移为移出低位4位
	}

	//初始化MR中的上下文,有3个
	for(int i=0;i<2;i++)
	{
		for(int ctx=0;i<2048;++ctx)
		{
			int j=(i<<11)+ctx;//(1<<11)为作标记准备
			if(i)
				maContextNo[ctx+(i<<11)]=initCtxMAG(T1_REFINE|ctx);//添加 refine标记
			else
				maContextNo[ctx+(i<<11)]=initCtxMAG(ctx);
		}
	}

	for(int ctx;ctx<256;++i)
		spbContextNo[ctx]=initSPB(ctx<<4);//腾出低4位

	int t,u,v;

	for(int i=0;i<(1<<T1_NMSEDEC_BITS);i++)
	{
		t=i/pow(2,T1_NMSEDEC_FRACBITS);
		u=t;
		v=t-1.5;

		nmSeDecSig[i]=int_max(0, (int) (floor((u * u - v * v) * pow(2, T1_NMSEDEC_FRACBITS) + 0.5) / pow(2, T1_NMSEDEC_FRACBITS) * 8192.0));
		nmSeDecSig0[i]=int_max(0, (int) (floor((u * u) * pow(2, T1_NMSEDEC_FRACBITS) + 0.5) / pow(2, T1_NMSEDEC_FRACBITS) * 8192.0));

		u=t-1.0;

		if(i&(i<<T1_NMSEDEC_BITS-1))
		{
			v=t-1.5;
		}else
		{
			v=t-0.5;
		}

		nmSeDecRef[i]=int_max(0, (int) (floor((u * u - v * v) * pow(2, T1_NMSEDEC_FRACBITS) + 0.5) / pow(2, T1_NMSEDEC_FRACBITS) * 8192.0));
		nmSeDecRef0[i]=int_max(0,(int) (floor((u * u) * pow(2, T1_NMSEDEC_FRACBITS) +0.5) / pow(2, T1_NMSEDEC_FRACBITS) * 8192.0));
	}
}
void j2kTileCoder::tierEncodCodeBlocks(j2kTileCoder_Tile *tile,TileCodeParam *tcp)
{
	tile->distotile=0;

	j2kTileCoder_Component *tilec;
	j2kTileCoder_Resolution *resolution;
	j2kTileCoder_Band *band;
	j2kTileCoder_Precinct *precinct;
	j2kTileCoder_CodeBlock *codeBlock;

	int x,y;

	for(int compno=0;compno<tile->numComponents;compno++)
	{
		tilec=&tile->comps[compno];

		for(int resno=0;resno<tilec->numResolutions;resno++)
		{
			resolution=&tilec->numResolutions[resno];
			
			for(int bandno=0;bandno<resolution->numBands;bandno++)
			{
				band=&resolution->bands[bandno];

				for(int preno=0;preno<resolution->precinctWidth*resolution->precinctHeight;preno++)
				{
					precinct=&band->precincts[preno];

					for(int cbno=0;cbno<precinct->codeBlockNumInWidth*precinct->codeBlockNumInHeight;cbno++)
					{
						codeBlock=&precinct->codeBlockInfo[cbno];

						switch(bandno)
						{
						case 0://LL
							x=codeBlock->x0-band->x0;
							y=codeBlock->y0-band->y0;
							break;
						case 1://LH
							j2kTileCoder_Resolution *pres=&tilec->resolutions[resno-1];//选取上一个分辨率(大)
							x=(pres->x1-pres->x0)+(codeBlock->x0-band->x0);
							y=codeBlock->y0-band->y0;
							break;
						case 2://HL
							j2kTileCoder_Resolution *pres=&tilec->resolutions[resno-1];//选取上一个分辨率(大)
							x=codeBlock->x0-band->x0;
							y=(pres->y1-pres->y0)+(codeBlock->y0-band->y0);
							break;
						case 3://HH
							j2kTileCoder_Resolution *pres=&tilec->resolutions[resno-1];//选取上一个分辨率(大)
							x=(pres->x1-pres->x0)+(codeBlock->x0-band->x0);
							y=(pres->y1-pres->y0)+(codeBlock->y0-band->y0);
							break;
						}

						if(tcp->tccps[compno].isReversibleDWT==1)
						{
							for(int i=0;i<codeBlock->y1-codeBlock->y0;i++)
							{
								for(int j=0;j<codeBlock->x1-codeBlock->x0;j++)
								{
									int dataLoc=x+j+(y+i)*(tilec->x1-tilec->x0);
									tierOneData[i][j]=tilec->data[dataLoc]<<T1_NMSEDEC_FRACBITS;
								}
							}
						}else if(tcp->tccps[compno].isReversibleDWT==0)
						{
							for(int i=0;i<codeBlock->y1->codeBlock->y0;i++)
							{
								for(int j=0;j<codeBlock->x1->codeBlock->x0;j++)
								{
									int dataLoc=x+j+(y+i)*(tilec->x1-tilec->x0);
									tierOneData[i][j]=fix_mul(tilec->data[dataLoc],8192*8192/band->stepSize)>>(13-T1_NMSEDEC_FRACBITS);
								}
							}
						}

						int orient=band->bandno;
						if(orient==1)
							orient=2;
						else if(orient==2)
							orient=1;

						tierEncodeCodeBlock(codeBlock,tile,orient,compno,tilec->numResolutions-1-resno,tcp->tccps[compno].isReversibleDWT,band->stepSize,tcp->tccps[compno].codeBlockStyle,tile->numComponents);
					}
				}
			}
		}
	}
}
void j2kTileCoder::tierEncodeCodeBlock(j2kTileCoder_CodeBlock *codeBlock,j2kTileCoder_Tile *tile,int orient,int compno,int level,int isDWT,double stepSize,int codeBlockStyle,int numComps)
{
	int cbWidth=codeBlock->x1-codeBlock->x0;
	int cbHeight=codeBlock->y1-codeBlock->y0;

	int max=0;

	for(int i=0;i<cbHeight;i++)
	{
		for(int j=0;j<cbWidth;j++)
		{
			max=int_max(max,int_abs(tierOneData[i][j]));
		}
	}

	if(max)
		codeBlock->numbps=int_floorlog2(max)+1-T1_NMSEDEC_FRACBITS;
	else
		codeBlock->numbps=0;

	for(int i=0;i<sizeof(tierOneFlags)/sizeof(int);i++)
	{
		((int*)tierOneFlags)[i]=0;
	}

	int bpsno=codeBlock->numbps-1;
	int passType=2;

	mqcResetStates();
	mqcSetState(T1_CTXNO_UNI,0,46);
	mqcSetState(T1_CTXNO_AGG,0,3);
	mqcSetState(T1_CTXNO_ZC,0,4);
	mqcInitEncode(codeBlock->data);

	int type;
	int nmSeDec;
	int curWmSeDec;//?
	for(int passno=0;bpsno>=0;passno++)
	{
		j2kTileCoder_Pass *pass=&codeBlock->passes[passno];
		int correction=3;
		type=((bpsno<(codeBlock->numbps-4))&&(passType<2)&&(codeBlockStyle&J2K_CCP_CBLKSTY_LAZY))?T1_TYPE_RAW:T1_TYPE_MQ;

		switch(passType)
		{
		case 0:
			tierOneSignPass(cbWidth,cbHeight,bpsno,orient,&nmSeDec,type,codeBlockStyle);
			break;
		case 1:
			tierOneRefPass(cbWidth,cbHeight,bpsno,&nmSeDec,type,codeBlockStyle);
			break;
		case 2:
			tierOneCleanPass(cbWidth,cbHeight,bpsno,orient,&nmSeDec,codeBlockStyle);
			
			if(codeBlockStyle&J2K_CCP_CBLKSTY_SEGSYM)
				mqcSegMarkEncode();/* 模式变化 SEGMARK*/

			break;
		}
		curWmSeDec+=getWmSeDec(nmSeDec,compno,level,orient,bpsno,isDWT,stepSize,numComps);//mod fixed_quality
		tile->distotile+=getWmSeDec(nmSeDec,compno,level,orient,bpsno,isDWT,stepSize,numComps);//add antonin quality


		//以下开始模式变换
		if(codeBlockStyle*J2K_CCP_CBLKSTY_TERMALL)&&!((passType==2)&&(bpsno-1<0))
		{
				mqcFlush();
				correction=1;
				pass->term=1;
		}else{
			if(((bpsno<(codeBlock->numbps-4)&&
				(passType>0))||
				((bpsno==(codeBlock->numbps-4))&&
				(passType==2)))&&(codeBlock&J2K_CCP_CBLKSTY_LAZY))
			{
				mqcFlush();
				correction=1;
				pass->term=0;
			}else{
				pass->term=0;
			}
		}

		if(++passType==3)
		{
			passType=0;
			bpsno--;
		}

		if(pass->term&&bpsno>0)
		{
			type=((bpsno<(codeBlock->numbps-4))&&(passType<2)&&(codeBlockStyle&J2K_CCP_CBLKSTY_LAZY))?T1_TYPE_RAW:T1_TYPE_MQ;
			if(type==T1_TYPE_RAW)
				mqcByPassInitEncode();//BYPASS模式初始化
			else
				mqcRestartInitEncode();
		}

		pass->distortiondec=curWmSeDec;
		pass->rate=mqcNumBytes()+correction;
		if(passno==0)
		{
			pass->len=pass->rate-0;
		}else{
			pass->len=pass->rate-codeBlock->passes[passno-1].rate;
		}

		//模式变换 RESET
		if(codeBlock&J2K_CCP_CBLKSTY_RESET)
			mqcResetEncode();
	}

	//模式变换ERTERM
	if(codeBlock&J2K_CCP_CBLKSTY_PTERM)
		mqcErtermEncode();
	else if(!(codeBlock&J2K_CCP_CBLKSTY_LAZY))//默认模式
		mqcFlush();

	codeBlock->totalpasses=passno;
}
void j2kTileCoder::mqcByPassInitEncode()
{
	mqc_c=0;
	mqc_ct=8;
}
void j2kTileCoder::mqcRestartInitEncode()
{
	mqcSetCurCtx(0);
	mqc_a=0x8000;
	mqc_c=0;
	mqc_ct=12;
	mqc_bp--;
	if(*mqc_bp==0xff);{
		mqc_ct=13;
	}
}
int j2kTileCoder::mqcNumBytes()
{
	return mqc_bp-mqc_start;
}
void j2kTileCoder::mqcFlush()
{
	mqcSetBits();
	mqc_c=mqc_c<<mqc_ct;
	mqcByteOut();
	mqc_c=mqc_c<<mqc_ct;
	mqcByteOut();

	if(*mqc_bp!=0xff)
		mqc_bp++;
}
void j2kTileCoder::mqcResetEncode()
{
	mqcResetStates();
	mqcSetState(18,0,46);
	mqcSetState(0,0,3);
	mqcSetState(1,0,4);
}
void j2kTileCoder::mqcErtermEncode()
{
	int k=11-mqc_ct+1;
	while(k>0)
	{
		mqc_c=mqc_c<<mqc_ct;
		mqc_ct=0;
		mqcByteOut();
		k-=mqc_ct;
	}
	if(*mqc_bp!=0xff)
		mqcByteOut();
}

void j2kTileCoder::mqcSetBits()
{
	unsigned int tempc=mqc_c+mqc_a;
	mqc_c|+0xffff;
	if(mqc_c>=tempc)
		mqc_c-=0x8000;
}
double j2kTileCoder::getWmSeDec(int nmSeDec,int compno,int level,int orient,int bpsno,int isDWT,int stepSize,int numComps)
{
	double w1,w2;

	if(isDWT)
	{
		if(numComps>1)
		{
			w1=mctGetNorm(compno);
		}else{
			w1=1;
		}
		w2=dwt_getnorm(level,orient);
	}else{
		if(numComps>1)
			w1=mctGetNormReal(compno);
		else
			w1=1;
		w2=dwt_getnorm_real(level,orient);
	}
	double wmSeDec=w1*w2*(stepSize/8192.0)*(1<bpsno);
	wmSeDec*=wmSeDec*nmSeDec/8192;
	return wmSeDec;
}

void j2kTileCoder::mqcSegMarkEncode()
{
	mqcSetCurCtx(18);
	for(int i=0;i<5;i++)
		mqcEncode(i%2);
}

int j2kTileCoder::initCtxZC(int ctx,int bandno)
{
	//八邻域上下文矢量
	int h=((ctx&T1_SIG_W)!=0)+((ctx&T1_SIG_E)!=0);//水平位置状态
	int v=((ctx&T1_SIG_N)!=0)+((ctx&T1_SIG_S)!=0);//垂直位置状态
	int d=((ctx&T1_SIG_NW)!=0)+((ctx&T1_SIG_NE)!=0)+((ctx&T1_SIG_SE)!=0)+((ctx&T1_SIG_SW)!=0);

	int n=0;//上下文相对于ZC初始位置的偏移
	switch(bandno)
	{
	case 2://如果是HL部分子带,则先交换垂直与水平位置状态
		int t=h;
		h=v;
		v=t;

	case 0://LL
	case 1://LH
		if(!h)
		{
			if(!v)
			{
				if(!d)
					n=0;
				else if(d==1)
					n=1;
				else
					n=2;
			}else if (v==1)
				n=3;
			else
				n=4;
		}else if(h==1)
		{
			if(!v)
			{
				if(!d)
					n=5;
				else
					n=6;
			}else 
				n=7;
			
		}else
			n=8;

		break;

	case 3://HH
		int hv=h+v;//合并水平与垂直

		if(!d)
		{
			if(!hv)
				n=0;
			else if(hv==1)
				n=1;
			else
				n=2;
		}else if(d==1)
		{
			if(!hv)
				n=3;
			else if(hv==1)
				n=4;
			else 
				n=5;
		}else if(d==2)
		{
			if(!hv)
				n=6;
			else 
				n=7;
		}else
			n=8;
		break;
	}
return T1_CTXNO_ZC+n;//返回上下文标记
}
int j2kTileCoder::initCtxSC(int ctx)
{
	int hc = int_min(((ctx & (T1_SIG_E | T1_SGN_E)) == T1_SIG_E) + ((ctx & (T1_SIG_W | T1_SGN_W)) == T1_SIG_W), 1) -
		int_min(((ctx & (T1_SIG_E | T1_SGN_E)) ==  (T1_SIG_E | T1_SGN_E)) +  ((ctx & (T1_SIG_W | T1_SGN_W)) ==(T1_SIG_W | T1_SGN_W)), 1);//水平方向,最大为1

	int vc = int_min(((ctx & (T1_SIG_N | T1_SGN_N)) == T1_SIG_N) + ((ctx & (T1_SIG_S | T1_SGN_S)) == T1_SIG_S),1) - 
		int_min(((ctx & (T1_SIG_N | T1_SGN_N)) == (T1_SIG_N | T1_SGN_N)) +((ctx & (T1_SIG_S | T1_SGN_S)) ==  (T1_SIG_S | T1_SGN_S)), 1);//垂直方向,最大为1
	//以上此举是为了用一定的数值去表达

	if(hc<0)
	{
		hc=-hc;
		vc=-vc;
	}

	if(!hc)
	{
		if(vc==-1)
			n=1;
		else if(!vc)
			n=0;
		else
			n=1;
	}else if(hc==1)
	{
		if(vc==-1)
			n=2;
		else if(!vc)
			n=3;
		else 
			n=4;
	}
	return T1_CTXNO_SC+n;
}
int j2kTileCoder::initCtxMAG(int ctx)
{
	int n;//magnitude refinement 上下文偏移
	if(ctx&T1_REFINE)//查看是否有refine标记
		n=2;
	else
		if(ctx&T1_SIG_OTH)
			n=1;
		else
			n=0;
}
int j2kTileCoder::initSPB(int ctx)
{
	int hc = int_min(((ctx & (T1_SIG_E | T1_SGN_E)) == 	T1_SIG_E) + ((ctx & (T1_SIG_W | T1_SGN_W)) == T1_SIG_W),1) -
		int_min(((ctx & (T1_SIG_E | T1_SGN_E)) == (T1_SIG_E | T1_SGN_E)) + ((ctx & (T1_SIG_W | T1_SGN_W)) == (T1_SIG_W | T1_SGN_W)), 1);

	int vc = int_min(((ctx & (T1_SIG_N | T1_SGN_N)) == T1_SIG_N) + ((ctx & (T1_SIG_S | T1_SGN_S)) == T1_SIG_S), 1) -
		int_min(((ctx & (T1_SIG_N | T1_SGN_N)) == (T1_SIG_N | T1_SGN_N)) + ((ctx & (T1_SIG_S | T1_SGN_S)) == (T1_SIG_S | T1_SGN_S)), 1);

	if(!hc&&!vc)
		n=0;
	else
	{
		n=!(hc>0||(!hc&&vc>0));//(hc>0)或者(hc=0且vc>0)时n=0
	}
}

void j2kTileCoder::mqcResetStates()
{
	for(int i=0;i<MQC_NUMCTXS;i++)
		mqcCtxs[i]=mqc_states;
}
void j2kTileCoder::mqcSetState(int ctx,int msb,int prob)
{
	mqcCtxs[ctx]=&mqc_states[msb+(prob<<1)];//[msb+prob*2]
}
void j2kTileCoder::mqcInitEncode(unsigned char *outputbuffer)
{
	mqcSetCurCtx(0);
	mqc_a=0x8000;
	mqc_c=0;
	mqc_bp=bp-1;
	mqc_ct=12;
	if(*mqc_bp==0xff)
		mqc_ct=13;
	mqc_start=outputbuffer;
}

void j2kTileCoder::int  mqcSetCurCtx(int ctx);
{
	mqcCurCtx=&mqcCtxs[ctx];
}

void j2kTileCoder::tierOneSignPass(int width,int height,int bpsno,int orient,int *nmSeDec ,char type,int cbstyle)
{
	*nmSeDec=0;
	int one=1<<(bpsno+T1_NMSEDEC_FRACBITS);

	int vsc;
	for(int i=0;i<height;i+=4)
	{
		for(int j=0;j<width;j++)
		{
			for(int k=0;k<i+4&&k<height;k++)
			{
				vsc=((cbstyle&J2K_CCP_CBLKSTY_VSC)&&(k=i+3||k=height-1))?1:0;

				tierOneSignPassStep(&tierOneFlags[1+k][1+j],&tierOneData[k][j],orient,bpsno,one,nmSeDec,type,vsc);
			}
		}
	}
}
void j2kTileCoder::tierOneSignPassStep(int *fp, int *dp, int orient, int bpno, int one,int *nmsedec, char type, int vsc)
{
	int flag;
	if(vsc)
	{
		flag=((*fp)&(~(T1_SIG_S | T1_SIG_SE | T1_SIG_SW | T1_SGN_S)))
	}else{
		flag=*fp;
	}

	if((flag&T1_SIG_OTH)&&!(flag&(T1_SIG|T1_VISIT)))
	{
		int v;
		if(int_abs(*dp)&one)
		{
			v=1;
		}else
		{
			v=0;
		}

		if(type==T1_TYPE_RAW)
		{
			/* BYPASS/LAZY MODE*/
			mqcSetCurCtx(getCtxNoZC(orient,flag));/* ESSAI */
			mqcByPassEncode(v);
		}else{
			mqcSetCurCtx(getCtxNoZC(orient,flag));
			mqcEncode(v);
		}

		if(v)
		{
			if(*dp<0)
				v=1;
			else
				v=0;
			*nmsedec+=tierOneGetNmSeDecSig(int_abs(*dp),bpno+T1_NMSEDEC_FRACBITS);
			if(type==T1_TYPE_RAW)//BYPASS/LAZY MODE
			{
				mqcSetCurCtx(getCtxNoSC(flag));/* ESSAI*/
				mqcByPassEncode(v);
			}else{
				mqcSetCurCtx(getCtxNoSC(flag));
				mqcEncode(v^getSPB(flag));
			}
			tierOneUpdateFlags(fp,v);
			*fp|=T1_SIG;
		}
		*fp|=T1_SIG;
	}
}
int j2kTileCoder::tierOneGetNmSeDecSig(int x,int bitpos)
{
	if(bitpos>T1_NMSEDEC_FRACBITS)
	{
		int sigLoc=(x>>(bitpos-T1_NMSEDEC_FRACBITS))&((1<<T1_NMSEDEC_FRACBITS)-1);
		return nmSeDecSig[sigLoc];
	}else{
		int sig0Loc=x&((1<<T1_NMSEDEC_FRACBITS)-1);
		return nmSeDecSig0[sig0Loc];
	}
}

void j2kTileCoder::tierOneUpdateFlags(int *fp,int s)
{
	int *np=fp-(T1_MAXCBLKW+2);
	int *sp=fp+(T1_MAXCBLKW+2);
	np[-1]|=T1_SIG_SE;
	np[1]|=T1_SIG_SW;

	sp[-1]|=T1_SIG_NE;
	sp[1]|=T1_SIG_NW;

	*np|=T1_SIG_S;
	*sp|=T1_SIG_N;

	fp[-1]|=T1_SIG_E;
	fp[1]|=T1_SIG_W;

	if(s)
	{
		*np|=T1_SGN_S;
		*sp|=T1_SGN_N;
		fp[-1]|=T1_SGN_E;
		fp[1]|=T1_SGN_W;
	}
}

void j2kTileCoder::mqcEncode(int d)
{
	if((*mqcCurCtx)->mps==d)
		mqcCodeMPS();
	else
		mqcCodeLPS();
}

void j2kTileCoder::mqcCodeMPS()
{
	mqc_a-=(*mqcCurCtx)->qeval;

	if((mqc_a&0x8000)==0)
	{
		if(mqc_a=(*mqcCurCtx)->qeval)
			mqc_a=(*mqcCurCtx)->qeval;
		else
			mqc_c+=(*mqcCurCtx)->qeval;

		*mqcCurCtx=(*mqcCurCtx)->nmps;
		mqcRenormalize();
	}else{
		mqc_c+=(*mqcCurCtx)->qeval;
	}
}
void j2kTileCoder::mqcRenormalize()
{
	do{
		mqc_a=mqc_a<<1;
		mqc_c=mqc_c<<1;
		mqc_ct--;
		if(mqc_ct==0)
		{
			mqcByteOut();
		}
	}while((mqc_a&0x8000)==0);
}
void j2kTileCoder::mqcByteOut()
{
	if(*mqc_bp==0xff)
	{
		mqc_bp++;
		*mqc_bp=mqc_c>>20;
		mqc_c=mqc_c&0xfffff;
		mqc_ct=7;
	}else{
		if((mqc_c&0x8000000)==0)
		{
			mqc_bp++;
			*mqc_bp=mqc_c>>19;
			mqc_c=mqc_c&0x7ffff;
			mqc_Ct=8;
		}else{
			(*mqc_bp)++;
			if(*mqc_bp==0xff)
			{
				mqc_c=mqc_c&0x7ffffff;
				mqc_bp++;
				*mqc_bp=mqc_c>>20;
				mqc_c=mqc_c&0xfffff;
				mqc_ct=7;
			}else{
				mqc_bp++;
				*mqc_bp=mqc_c>>19;
				mqc_c=mqc_c&0x7ffff;
				mqc_ct=8;
			}
		}
	}
}

void j2kTileCoder::mqcCodeLPS()
{
	mqc_a=mqc_a-(*mqcCurCtx)->qeval;
	if(mqc_a<(*mqcCurCtx)->qeval)
	{
		mqc_c+=(*mqcCurCtx)->qeval;
	}else{
		mqc_a=(*mqcCurCtx)->qeval;
	}
	*mqcCurCtx=(*mqcCurCtx)->nlps;
	mqcRenormalize();
}
void j2kTileCoder::mqcByPassEncode(int d)
{
	mqc_ct--;
	mqc_c=mqc_c+(d<<mqc_ct);

	if(mqc_ct==0)
	{
		mqc_bp++;
		*mqc_bp=mqc_c;
		mqc_ct=8;
		if(*mqc_bp==0xff){
			mqc_ct=7;
		}
		mqc_c=0;
	}
}

int j2kTileCoder::getCtxNoZC(int orient,int flag)
{
	return zcContextNo[(orient<<8)|(flag&T1_SIG_OTH)];
}
int j2kTileCoder::getCtxNoSC(int flag)
{
	return scContextNo[(flag&(T1_SIG_PRIM|T1_SGN))>>4];
}
int j2kTileCoder::getSPB(int flag)
{
	return spbNo[(f&(T1_SIG_PRIM|T1_SGN))>>4];
}

void j2kTileCoder::tierOneRefPass(int width,int height,int bpsno,int *nmSeDec,char type,int cbstyle)
{
	*nmSeDec=0;
	int one=1<<(bpsno+T1_NMSEDEC_FRACBITS);

	int vsc;
	for(int i=0;i<height;i++)
	{
		for(int j=0;j<width;j++)
		{
			for(int k=i;k<i+4&&k<height;k++)
			{
				if((cbstyle&J2K_CCP_CBLKSTY_VSC)&&(k==i+3||k==height-1))
					vsc=1;
				else
					vsc=0;

				tierOneRefPassStep(&tierOneFlags[k+1][j+1],&tierOneData[k][j],bpsno,one,nmSeDec,type,vsc);
			}
		}
	}
}
void j2kTileCoder::tierOneRefPassStep(int *fp, int *dp, int orient, int bpno, int one,int *nmsedec, char type, int vsc)
{
	int flag;
	if(vsc)
	{
		flag=((*fp)&(~(T1_SIG_S | T1_SIG_SE | T1_SIG_SW | T1_SGN_S)))
	}else{
		flag=*fp;
	}

	//if((flag&T1_SIG_OTH)&&!(flag&(T1_SIG|T1_VISIT)))
	if((flag&(T1_SIG|T1_VISIT))==T1_SIG)
	{
		*nmsedec+=tierOneGetNmSeDecRef(int_abs(*dp),bpno+T1_NMSEDEC_FRACBITS);
		int v;
		if(int_abs(*dp)&one)
		{
			v=1;
		}else
		{
			v=0;
		}

		if(type==T1_TYPE_RAW)
		{
			/* BYPASS/LAZY MODE*/
			mqcSetCurCtx(getCtxNoZC(orient,flag));/* ESSAI */
			mqcByPassEncode(v);
		}else{
			mqcSetCurCtx(getCtxNoZC(orient,flag));
			mqcEncode(v);
		}
		*fp|=T1_REFINE;
	}
}
int j2kTileCoder::tierOneGetNmSeDecRef(int x,int bitpos)
{
	if(bitpos>T1_NMSEDEC_FRACBITS)
	{
		int refLoc=(x>>(bitpos-T1_NMSEDEC_FRACBITS))&((1<<T1_NMSEDEC_BITS)-1);
		return nmSeDecRef[refLoc];
	}else{
		int ref0Loc=x&((1<<T1_NMSEDEC_BITS)-1);
		return nmSeDecRef0[ref0Loc];
	}
}

void j2kTileCoder::tierOneCleanPass(int width,int height,int bpsno,int orient,int *nmSeDec,int cbstyle)
{
	*nmSeDec=0;
	int one=1<<(bpsno+T1_NMSEDEC_FRACBITS);

	int agg;
	int vsc;
	int t1Sign=T1_SIG|T1_VISIT|T1_SIG_OTH;
	for(int i=0;i<height;i+=4)
	{
		for(int j=0;j<width;j++)
		{
			if(i+3<height)
			{
				
				if(cbstyle&J2K_CCP_CBLKSTY_VSC){
					
					agg=!(tierOneFlags[i+1][j+1]&t1Sign||
						tierOneFlags[i+2][j+1]&t1Sign||
						tierOneFlags[i+3][j+1]&t1Sign||
						(tierOneFlags[i+4][j+1]&(~(T1_SIG_S|T1_SIG_SE|T1_SIG_SW|T1_SGN_S)))&t1Sign);
			}else{
				agg=!(tierOneFlags[i+1][j+1]&t1Sign||
					tierOneFlags[i+2][j+1]&t1Sign||
					tierOneFlags[i+3][j+1]&t1Sign||
					tierOneFlags[i+4][j+1]&t1Sign);
				}
		}else{
			agg=0;
			}

			if(agg)
			{
				for(int runlen=0;runlen<4;runlen++)
				{
					if(int_abs(tierOneData[i+runlen][j])&one)
						break;
				}
				mqcSetCurCtx(T1_CTXNO_AGG);
				mqcEncode(runlen!=4);
				if(runlen==4)
					continue;

				mqcSetCurCtx(T1_CTXNO_UNI);
				mqcEncode(runlen>>1);
				mqcEncode(runlen&1);
			}else
			{
				runlen=0;
			}

			for(int k=i+runlen;k<i+4&&k<height;k++)
			{
				if((cbstyle&J2K_CCP_CBLKSTY_VSC)&&(k+i+3||k=height-1))
					vsc=1
				else
					vsc=0;

				tierOneClnPassStep(&tierOneFlags[k+1][j+1],&tierOneData[k][j],orient,bpsno,one,nmSeDec,agg&&(k==i+runlen),vsc);
			}
		}
	}
}
void j2kTileCoder::tierOneClnPassStep(int *fp, int *dp, int orient, int bpsno, int one,int *nmsedec, int partial, int vsc)
{
	int flag;
	int v;
	if(vsc)
		flag=(*fp)&(~T1_SIG_S|T1_SIG_SE|T1_SIG_SW|T1_SGN_S);
	else
		flag=*fp;

	if(partial)
	{
		*nmsedec+=tierOneGetNmSeDecSig(int_abs(*dp),bpsno+T1_NMSEDEC_FRACBITS);
		mqcSetCurCtx(getCtxNoSC(flag));
		if(*dp<0)
			v=1;
		else
			v=0;
		mqcEncode((v^getSPB(flag)));
		tierOneUpdateFlags(fp,v);
		*fp|=T1_SIG;
	}else{
		if(!(*fp&(T1_SIG|T1_VISIT)))
		{
			mqcSetCurCtx(getCtxNoZC(orient,flag));
			if(int_abs(*dp)&one)
				v=1;
			else
				v=0;
			mqcEncode(v);
			if(v)
			{
				*nmsedec+=tierOneGetNmSeDecSig(int_abs(*dp),bpsno+T1_NMSEDEC_FRACBITS);
				mqcSetCurCtx(getCtxNoSC(flag));
				if(*dp<0)
					v=1;
				else
					v=0;
				mqcEncode((v^getSPB(flag)));
				tierOneUpdateFlags(fp,v);
				*fp|=T1_SIG;
			}
		}
	}
	*fp&=~T1_VISIT;
}

int j2kTileCoder::dwtGetGain(int orient)
{
	if (orient == 0)//LL
		return 0;
	if (orient == 1 || orient == 2)//HL,LH
		return 1;
	return 2;//HH
}