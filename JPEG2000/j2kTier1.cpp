#include "j2kTier1.h"

void j2kTierOne::tierOneInitContext()
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
		for(int ctx=0;ctx<2048;++ctx)
		{
			int j=(i<<11)+ctx;//(1<<11)为作标记准备
			if(i)
				maContextNo[ctx+(i<<11)]=initCtxMAG(T1_REFINE|ctx);//添加 refine标记
			else
				maContextNo[ctx+(i<<11)]=initCtxMAG(ctx);
		}
	}

	for(int ctx=0;ctx<256;++ctx)
		spbNo[ctx]=initSPB(ctx<<4);//腾出低4位

	double t,u,v;

	for(int i=0;i<(1<<T1_NMSEDEC_BITS);i++)
	{
		t=i/pow(2.0,T1_NMSEDEC_FRACBITS);
		u=t;
		v=t-1.5;

		double sig=(floor((u * u - v * v) * pow(2.0, T1_NMSEDEC_FRACBITS) + 0.5) / pow(2.0, T1_NMSEDEC_FRACBITS) * 8192.0);
		nmSeDecSig[i]=int_max(0,(int)sig );
		sig=(floor((u * u) * pow(2.0, T1_NMSEDEC_FRACBITS) + 0.5) / pow(2.0, T1_NMSEDEC_FRACBITS) * 8192.0);
		nmSeDecSig0[i]=int_max(0, (int)sig );

		u=t-1.0;

		if(i&(1<<T1_NMSEDEC_BITS-1))
		{
			v=t-1.5;
		}else
		{
			v=t-0.5;
		}

		nmSeDecRef[i]=int_max(0, (int) (floor((u * u - v * v) * pow(2.0, T1_NMSEDEC_FRACBITS) + 0.5) / pow(2.0, T1_NMSEDEC_FRACBITS) * 8192.0));
		nmSeDecRef0[i]=int_max(0,(int) (floor((u * u) * pow(2.0, T1_NMSEDEC_FRACBITS) +0.5) / pow(2.0, T1_NMSEDEC_FRACBITS) * 8192.0));
	}
}
void j2kTierOne::tierOneEncodeCodeBlocks(j2kTileCoder_Tile *tile,TileCodeParam *tcp)
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
		//printf ("comp====%d\n",codeBlock->x0);
		for(int resno=0;resno<tilec->numResolutions;resno++)
		{
			resolution=&tilec->resolutions[resno];
			//printf ("reso====%d\n",codeBlock->x0);
			for(int bandno=0;bandno<resolution->numBands;bandno++)
			{
				band=&resolution->bands[bandno];
				//printf ("band====%d\n",codeBlock->x0);
				for(int preno=0;preno<resolution->precinctWidth*resolution->precinctHeight;preno++)
				{
					precinct=&band->precincts[preno];
					//printf ("pre====%d\n",temp->x0);
					for(int cbno=0;cbno<precinct->codeBlockNumInWidth*precinct->codeBlockNumInHeight;cbno++)
					{
						//printf ("cbno==%d==%d\n",cbno,ttemp->x0);
						codeBlock=&precinct->codeBlockInfo[cbno];

						j2kTileCoder_Resolution *pres;
						switch(band->bandno)
						{
						case 0://LL
							x=codeBlock->x0-band->x0;
							y=codeBlock->y0-band->y0;
							break;
						case 1://LH
							pres=&tilec->resolutions[resno-1];//选取上一个分辨率(大)
							x=(pres->x1-pres->x0)+(codeBlock->x0-band->x0);
							y=codeBlock->y0-band->y0;
							break;
						case 2://HL
							pres=&tilec->resolutions[resno-1];//选取上一个分辨率(大)
							x=codeBlock->x0-band->x0;
							y=(pres->y1-pres->y0)+(codeBlock->y0-band->y0);
							break;
						case 3://HH
							pres=&tilec->resolutions[resno-1];//选取上一个分辨率(大)
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
									int dataLoc=(x+j)+(y+i)*(tilec->x1-tilec->x0);
									tierOneData[i][j]=tilec->data[dataLoc]<<T1_NMSEDEC_FRACBITS;
								}
							}
						}else if(tcp->tccps[compno].isReversibleDWT==0)
						{
							for(int i=0;i<codeBlock->y1-codeBlock->y0;i++)
							{
								for(int j=0;j<codeBlock->x1-codeBlock->x0;j++)
								{
									int dataLoc=(x+j)+(y+i)*(tilec->x1-tilec->x0);
									tierOneData[i][j]=fix_mul(tilec->data[dataLoc],8192*8192/band->stepSize)>>(13-T1_NMSEDEC_FRACBITS);
								}
							}
						}

						int orient=band->bandno;
						if(orient==1)
							orient=2;
						else if(orient==2)
							orient=1;

						//printf ("cbno==%d==%d\n",cbno,ttemp->x0);

						tierOneEncodeCodeBlock(codeBlock,tile,orient,compno,tilec->numResolutions-1-resno,tcp->tccps[compno].isReversibleDWT,band->stepSize,tcp->tccps[compno].codeBlockStyle,tile->numComponents);
					}
				}
			}
		}
	}
}
void j2kTierOne::tierOneEncodeCodeBlock(j2kTileCoder_CodeBlock *codeBlock,j2kTileCoder_Tile *tile,int orient,int compno,int level,int isDWT,double stepSize,int codeBlockStyle,int numComps)
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
	double curWmSeDec=0;//?
	int passno;
	
	for(passno=0;bpsno>=0;passno++)
	{
		//printf ("cb2==%d==%d\n",passno,ttemp->x0);
		j2kTileCoder_Pass *pass=&codeBlock->passes[passno];
		int correction=3;
		type=((bpsno<(codeBlock->numbps-4))&&(passType<2)&&(codeBlockStyle&J2K_CCP_CBLKSTY_LAZY))?T1_TYPE_RAW:T1_TYPE_MQ;

		switch(passType)
		{
		case 0:
			tierOneSignPass(cbWidth,cbHeight,bpsno,orient,&nmSeDec,type,codeBlockStyle);
			break;
		case 1:
			tierOneRefPass(cbWidth,cbHeight,bpsno,&nmSeDec,type,codeBlockStyle);//此项有问题
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
		if((codeBlockStyle&J2K_CCP_CBLKSTY_TERMALL)&&
			!((passType==2)&&
			(bpsno-1<0)))
		{
			mqcFlush();
			correction=1;
			pass->term=1;
		}else{
			if(((bpsno<(codeBlock->numbps-4)&&(passType>0))||((bpsno==(codeBlock->numbps-4))&&(passType==2)))&&
				(codeBlockStyle&J2K_CCP_CBLKSTY_LAZY))
			{
				mqcFlush();
				correction=1;
				pass->term=1;
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
			pass->len=pass->rate;
		}else{
			pass->len=pass->rate-codeBlock->passes[passno-1].rate;
		}

		//模式变换 RESET
		if(codeBlockStyle&J2K_CCP_CBLKSTY_RESET)
			mqcResetEncode();
	}

	//模式变换ERTERM
	if(codeBlockStyle&J2K_CCP_CBLKSTY_PTERM)
		mqcErtermEncode();
	else if(!(codeBlockStyle&J2K_CCP_CBLKSTY_LAZY))//默认模式
		mqcFlush();
	
	codeBlock->totalpasses=passno;

}
void j2kTierOne::mqcResetEncode()
{
	mqcResetStates();
	mqcSetState(18,0,46);
	mqcSetState(0,0,3);
	mqcSetState(1,0,4);
}
void j2kTierOne::mqcByPassInitEncode()
{
	mqc_c=0;
	mqc_ct=8;
}
void j2kTierOne::mqcRestartInitEncode()
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

int j2kTierOne::mqcNumBytes()
{
	return mqc_bp-mqc_start;
}
void j2kTierOne::mqcFlush()
{
	mqcSetBits();
	mqc_c=mqc_c<<mqc_ct;
	mqcByteOut();
	mqc_c=mqc_c<<mqc_ct;
	mqcByteOut();

	if(*mqc_bp!=0xff)
		mqc_bp++;
}
void j2kTierOne::mqcErtermEncode()
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
void j2kTierOne::mqcSetBits()
{
	unsigned int tempc=mqc_c+mqc_a;
	mqc_c|+0xffff;
	if(mqc_c>=tempc)
		mqc_c-=0x8000;
}

double j2kTierOne::getWmSeDec(int nmSeDec,int compno,int level,int orient,int bpsno,int isDWT,int stepSize,int numComps)
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
		w2=dwtGetNorm(level,orient);
	}else{
		if(numComps>1)
			w1=mctGetNormReal(compno);
		else
			w1=1;
		w2=dwtGetNormReal(level,orient);
	}
	double wmSeDec=w1*w2*(stepSize/8192.0)*(1<<bpsno);
	wmSeDec*=wmSeDec*nmSeDec/8192;
	return wmSeDec;
}

void j2kTierOne::mqcSegMarkEncode()
{
	mqcSetCurCtx(18);
	for(int i=0;i<5;i++)
		mqcEncode(i%2);
}

int j2kTierOne::initCtxZC(int ctx,int bandno)
{
	//八邻域上下文矢量
	int h=((ctx&T1_SIG_W)!=0)+((ctx&T1_SIG_E)!=0);//水平位置状态
	int v=((ctx&T1_SIG_N)!=0)+((ctx&T1_SIG_S)!=0);//垂直位置状态
	int d=((ctx&T1_SIG_NW)!=0)+((ctx&T1_SIG_NE)!=0)+((ctx&T1_SIG_SE)!=0)+((ctx&T1_SIG_SW)!=0);

	int n=0;//上下文相对于ZC初始位置的偏移
	int t;
	switch(bandno)
	{
	case 2://如果是HL部分子带,则先交换垂直与水平位置状态
		t=h;
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
int j2kTierOne::initCtxSC(int ctx)
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

	int n=0;
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
int j2kTierOne::initCtxMAG(int ctx)
{
	int n;//magnitude refinement 上下文偏移
	if(ctx&T1_REFINE)//查看是否有refine标记
		n=2;
	else
		if(ctx&T1_SIG_OTH)
			n=1;
		else
			n=0;
	return T1_CTXNO_MAG+n;
}
int j2kTierOne::initSPB(int ctx)
{
	int hc = int_min(((ctx & (T1_SIG_E | T1_SGN_E)) == 	T1_SIG_E) + ((ctx & (T1_SIG_W | T1_SGN_W)) == T1_SIG_W),1) -
		int_min(((ctx & (T1_SIG_E | T1_SGN_E)) == (T1_SIG_E | T1_SGN_E)) + ((ctx & (T1_SIG_W | T1_SGN_W)) == (T1_SIG_W | T1_SGN_W)), 1);

	int vc = int_min(((ctx & (T1_SIG_N | T1_SGN_N)) == T1_SIG_N) + ((ctx & (T1_SIG_S | T1_SGN_S)) == T1_SIG_S), 1) -
		int_min(((ctx & (T1_SIG_N | T1_SGN_N)) == (T1_SIG_N | T1_SGN_N)) + ((ctx & (T1_SIG_S | T1_SGN_S)) == (T1_SIG_S | T1_SGN_S)), 1);

	int n=0;
	if(!hc&&!vc)
		n=0;
	else
	{
		n=!(hc>0||(!hc&&vc>0));//(hc>0)或者(hc=0且vc>0)时n=0
	}

	return n;
}

void j2kTierOne::mqcResetStates()
{
	for(int i=0;i<MQC_NUMCTXS;i++)
		mqcCtxs[i]=mqc_states;
}
void j2kTierOne::mqcSetState(int ctx,int msb,int prob)
{
	mqcCtxs[ctx]=&mqc_states[msb+(prob<<1)];//[msb+prob*2]
}
void j2kTierOne::mqcInitEncode(unsigned char *outputbuffer)
{
	mqcSetCurCtx(0);
	mqc_a=0x8000;
	mqc_c=0;
	mqc_bp=outputbuffer-1;
	mqc_ct=12;
	if(*mqc_bp==0xff)
		mqc_ct=13;
	mqc_start=outputbuffer;
}

void j2kTierOne::mqcSetCurCtx(int ctx)
{
	mqcCurCtx=&mqcCtxs[ctx];
}

void j2kTierOne::tierOneSignPass(int width,int height,int bpsno,int orient,int *nmSeDec ,char type,int cbstyle)
{
	*nmSeDec=0;
	int one=1<<(bpsno+T1_NMSEDEC_FRACBITS);

	int vsc;
	for(int i=0;i<height;i+=4)
	{
		for(int j=0;j<width;j++)
		{
			for(int k=i;k<i+4&&k<height;k++)
			{
				vsc=((cbstyle&J2K_CCP_CBLKSTY_VSC)&&((k==i+3)||(k==height-1)))?1:0;

				tierOneSignPassStep(&tierOneFlags[1+k][1+j],&tierOneData[k][j],orient,bpsno,one,nmSeDec,type,vsc);
			}
		}
	}
}
void j2kTierOne::tierOneSignPassStep(int *fp, int *dp, int orient, int bpno, int one,int *nmsedec, char type, int vsc)
{
	int flag;
	if(vsc)
	{
		flag=((*fp)&(~(T1_SIG_S | T1_SIG_SE | T1_SIG_SW | T1_SGN_S)));
	}else{
		flag=*fp;
	}
	if((flag&T1_SIG_OTH)&&!(flag&(T1_SIG|T1_VISIT))){
		int v=int_abs(*dp)&one?1:0;
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
		*fp|=T1_VISIT;
	}
}


void j2kTierOne::tierOneRefPass(int width,int height,int bpsno,int *nmSeDec,char type,int cbstyle)
{
	*nmSeDec=0;
	int one=1<<(bpsno+T1_NMSEDEC_FRACBITS);
	
	int vsc;

	for(int i=0;i<height;i+=4)
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
void j2kTierOne::tierOneRefPassStep(int *fp, int *dp, int bpno, int one,int *nmsedec, char type, int vsc)
{
	int flag;
	if(vsc)
	{
		flag=((*fp)&(~(T1_SIG_S | T1_SIG_SE | T1_SIG_SW | T1_SGN_S)));
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
			mqcSetCurCtx(getCtxNoMAG(flag));
			mqcByPassEncode(v);
		}else{
			mqcSetCurCtx(getCtxNoMAG(flag));
			mqcEncode(v);
		}
		*fp|=T1_REFINE;
	}
}

int j2kTierOne::tierOneGetNmSeDecRef(int x,int bitpos)
{
	int refLoc;
	int returns;
	if(bitpos>T1_NMSEDEC_FRACBITS)
	{
		refLoc=(x>>(bitpos-T1_NMSEDEC_FRACBITS))&((1<<T1_NMSEDEC_BITS)-1);
		returns=nmSeDecRef[refLoc];
		return nmSeDecRef[refLoc];
	}else{
		refLoc=x&((1<<T1_NMSEDEC_BITS)-1);
		returns=nmSeDecRef0[refLoc];
		return nmSeDecRef0[refLoc];
	}
}

void j2kTierOne::tierOneCleanPass(int width,int height,int bpsno,int orient,int *nmSeDec,int cbstyle)
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
			int runlen=0;
			if(agg)
			{
				for(runlen=0;runlen<4;runlen++)
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
				vsc=((cbstyle&J2K_CCP_CBLKSTY_VSC)&&((k==i+3)||(k==height-1)))?1:0;
				tierOneClnPassStep(&tierOneFlags[k+1][j+1],&tierOneData[k][j],orient,bpsno,one,nmSeDec,agg&&(k==i+runlen),vsc);
			}
		}
	}
}
void j2kTierOne::tierOneClnPassStep(int *fp, int *dp, int orient, int bpsno, int one,int *nmsedec, int partial, int vsc)
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
		mqcEncode(v^getSPB(flag));
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
				mqcEncode(v^getSPB(flag));
				tierOneUpdateFlags(fp,v);
				*fp|=T1_SIG;
			}
		}
	}
	*fp&=~T1_VISIT;
}

void j2kTierOne::tierOneUpdateFlags(int *fp,int s)
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

void j2kTierOne::mqcEncode(int d)
{
	if((*mqcCurCtx)->mps==d)
		mqcCodeMPS();
	else
		mqcCodeLPS();
}

void j2kTierOne::mqcCodeMPS()
{
	mqc_a-=(*mqcCurCtx)->qeval;

	if((mqc_a&0x8000)==0)
	{
		if(mqc_a<(*mqcCurCtx)->qeval)
			mqc_a=(*mqcCurCtx)->qeval;
		else
			mqc_c+=(*mqcCurCtx)->qeval;

		*mqcCurCtx=(*mqcCurCtx)->nmps;
		mqcRenormalize();
	}else{
		mqc_c+=(*mqcCurCtx)->qeval;
	}
}
void j2kTierOne::mqcRenormalize()
{
	do{
		mqc_a<<=1;
		mqc_c<<=1;
		mqc_ct--;
		if(mqc_ct==0)
		{
			mqcByteOut();
		}
	}while((mqc_a&0x8000)==0);
}
void j2kTierOne::mqcByteOut()
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
			mqc_ct=8;
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

void j2kTierOne::mqcCodeLPS()
{
	mqc_a-=(*mqcCurCtx)->qeval;
	if(mqc_a<(*mqcCurCtx)->qeval)
	{
		mqc_c+=(*mqcCurCtx)->qeval;
	}else{
		mqc_a=(*mqcCurCtx)->qeval;
	}
	*mqcCurCtx=(*mqcCurCtx)->nlps;
	mqcRenormalize();
}
void j2kTierOne::mqcByPassEncode(int d)
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

int j2kTierOne::getCtxNoZC(int orient,int flag)
{
	return zcContextNo[(orient<<8)|(flag&T1_SIG_OTH)];
}
int j2kTierOne::getCtxNoSC(int flag)
{
	return scContextNo[(flag&(T1_SIG_PRIM|T1_SGN))>>4];
}
int j2kTierOne::getCtxNoMAG(int flag)
{
	return maContextNo[(flag&T1_SIG_OTH)|(((flag&T1_REFINE)!=0)<<11)];
}
int j2kTierOne::getSPB(int flag)
{
	return spbNo[(flag&(T1_SIG_PRIM|T1_SGN))>>4];
}
int j2kTierOne::tierOneGetNmSeDecSig(int x,int bitpos)
{
	int sigLoc;
	if(bitpos>T1_NMSEDEC_FRACBITS)
	{
		sigLoc=(x>>(bitpos-T1_NMSEDEC_FRACBITS))&((1<<T1_NMSEDEC_BITS)-1);
		return nmSeDecSig[sigLoc];
	}else{
		sigLoc=x&((1<<T1_NMSEDEC_BITS)-1);
		return nmSeDecSig0[sigLoc];
	}
}

