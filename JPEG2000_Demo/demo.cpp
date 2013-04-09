// JPEG2000_Demo.cpp : main project file.
#include "stdafx.h"
#include "preProcessor.h"
#include <time.h>

using namespace std;

int main(int argc, char **argv)
{
	CodeParam cp;
	TileCodeParam tcp_init;
	TileCompCodeParam tccp_init;
	j2kPOC *tcp_poc;
	jp2Image img;

	PreProcessor *preProcessor=new PreProcessor();

	int subSamplingX=1;
	int subSamplingY=1;//样本数据大小 

	char *inFile="3.bmp";
	char *outFile="1.jp2";

	//初始化
	cp.imageType=2;//bmp
	cp.format=1;//jp2
	cp.comment="Created by demonk";
	
	cp.fixed_alloc = 0;
	cp.fixed_quality = 0;		//add fixed_quality,use rate,not distoratio
	cp.isIntermedFile=0;
	cp.XTsiz=200;
	cp.YTsiz=400;//这个关系到图像原分辨率,要匹配,否则错误
	cp.XTOsiz=0;
	cp.YTOsiz=0;//先不偏移

	tcp_init.numLayers=1;
	tcp_init.rates[0]=1;//第一层一定无损,取值大于1
// 	tcp_init.rates[1]=1;
// 	tcp_init.rates[2]=77;//对应无损
	cp.disto_alloc = 1;//开启质量层

	tcp_init.progressionOrder=1;/* -1~4 */
	tcp_init.codingStyle=0;
	tcp_init.numPocs=0;

	tccp_init.numResolutions=6;
	tccp_init.codeBlockWidth=64;
	tccp_init.codeBlockHeight=64;
	tccp_init.isROI=0;/*开了就浮雕了*/
	tccp_init.codingStyle=0;
	tccp_init.isReversibleDWT=1;
	tccp_init.quantisationStyle=0;
	tccp_init.codeBlockStyle=0;
	tccp_init.numGuardBits=2;

	bmpReader::bmpToImage(inFile,&img,subSamplingX,subSamplingY);

	cp.tw=int_ceildiv(img.Xsiz-cp.XTOsiz,cp.XTsiz);
	cp.th=int_ceildiv(img.Ysiz-cp.YTOsiz,cp.YTsiz);

	cp.tcps=(TileCodeParam*)malloc(cp.tw*cp.th*sizeof(TileCodeParam));
	cp.tcps->tccps=(TileCompCodeParam*)malloc(img.numComponents*sizeof(TileCompCodeParam));

	int CodingStyle=0;

	for(int tileno=0;tileno<cp.th*cp.tw;tileno++)
	{
		TileCodeParam *tcp =&cp.tcps[tileno];
		tcp->numLayers=tcp_init.numLayers;

		for(int j=0;j<tcp_init.numLayers;j++)
		{
			if(cp.fixed_quality)
			{
				tcp->distoratio[j]=tcp_init.distoratio[j];
			}else{
				tcp->rates[j]=tcp_init.rates[j];
			}
		}

		tcp->codingStyle=CodingStyle;
		tcp->progressionOrder=preProcessor->calProgression("LRCP");//默认的渐进
		tcp->isMCT=img.numComponents==3?1:0;
		tcp->numLayers=1 ;


		//POC可选
		int numPocs=0;//先不开POC
		tcp->pocUse=0;//标记此POC可用

		for(int i=0;i<numPocs;i++)
		{
			// T1=0,0,1,5,3,CPRL
			j2kPOC *poc=&tcp->pocs[i];

			poc->resolutionStart=0;
			poc->componentStart=0;
			poc->layerEnd=1;
			poc->resolutionEnd=5;//表示只对0~3分辨率层进行调整
			poc->componentEnd=3;
			poc->progressionOrder=preProcessor->calProgression("LRCP");
			poc->tile=1;
		}

		tcp->numPocs=numPocs;

		tcp->tccps=(TileCompCodeParam*)malloc(img.numComponents*sizeof(TileCompCodeParam));
		for(int i=0;i<img.numComponents;i++)
		{
			TileCompCodeParam *tccp=&tcp->tccps[i];

			tccp->codingStyle=CodingStyle&0x01;
			tccp->numResolutions=tccp_init.numResolutions;
			tccp->codeBlockWidth=int_floorlog2(tccp_init.codeBlockWidth);
			tccp->codeBlockHeight=int_floorlog2(tccp_init.codeBlockHeight);

			tccp->codeBlockStyle=tccp_init.codeBlockStyle;//模式组合开关
			tccp->isReversibleDWT=tccp_init.isReversibleDWT;//可逆
			tccp->quantisationStyle=tccp_init.quantisationStyle;//量化模式
			tccp->numGuardBits=tccp_init.numGuardBits;

			if(i%2==0)
			tccp->isROI=tccp_init.isROI;//非感兴趣
			else
				tccp->isROI=0;
			for(int j=0;j<tccp->numResolutions;j++)
			{
				tccp->precinctWidth[j]=15;
				tccp->precinctHeight[j]=15;
			}

		preProcessor->calStepSizes(tccp,img.comps[i].precision);//计算子带量化步长
		}
	}

	jp2Struct *jp2struct=new jp2Struct();
	jp2struct->image=&img;
	jp2struct->jp2StructInit(&img);//初始化框结构

// 	if(cp.isIntermedFile==1)
// 		cp.isIntermedFile=0;

	char *outbuf=(char*)malloc(cp.XTsiz*cp.YTsiz*cp.tw*cp.th*10*sizeof(char));
	
	jp2Writer *write=new jp2Writer();

	clock_t startTime=clock();
	int len=write->encode(jp2struct,&cp,outbuf);
	clock_t endTime=clock();

	int second=(endTime-startTime)/1000;
	int microsecond=(endTime-startTime)%1000;
	cout<<"Time Consuming:"<<second<<"."<<microsecond<<"s"<<endl;

	if(len>0)
	{
		FILE* file=fopen(outFile,"wb");
		if(file)
		{
			fwrite(outbuf,1,len,file);
			free(outbuf);
			fclose(file);
		}
	}

	free(img.comps);
	free(cp.tcps);
	delete jp2struct;
	delete preProcessor;

	system("pause");
}


