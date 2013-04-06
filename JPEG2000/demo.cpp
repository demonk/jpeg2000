// JPEG2000_Demo.cpp : main project file.
#include <iostream>
#include "jp2Writer.h"
#include "CodeParam.h"

using namespace std;

int bmptoimage(char *filename,jp2Image *img,int subsampling_dx,int subsampling_dy);
int give_progression(char progression[4]);
int jp2_init_stdjp2(jp2Struct * jp2_struct, jp2Image * img);

int main(int argc, char **argv)
{
	CodeParam cp;
	TileCodeParam tcp_init;
	TileCompCodeParam tccp;
	j2kPOC *tcp_poc;
	jp2Image img;

	int subSamplingX=400;
	int subSamplingY=400;//样本数据大小 

	char *inFile="1.bmp";
	char *outFile="1.jp2";

	//初始化
	cp.imageType=2;//bmp
	cp.format=1;//jp2
	cp.comment="JPEG2000 Test";
	cp.disto_alloc = 0;
	cp.fixed_alloc = 0;
	cp.fixed_quality = 0;		//add fixed_quality
	cp.isIntermedFile=0;
	cp.XTsiz=300;
	cp.YTsiz=300;
	cp.XTOsiz=100;
	cp.YTOsiz=100;

	tcp_init.numLayers=3;
	tcp_init.rates[0]=20;
	tcp_init.rates[1]=21;
	tcp_init.rates[2]=22;
	tcp_init.distoratio[0]=66.5;
	tcp_init.distoratio[1]=55.5;
	tcp_init.distoratio[2]=44.5;
	tcp_init.progressionOrder=1;/* -1~4 */

	tccp.numResolutions=5;
	tccp.codeBlockWidth=64*64;
	tccp.codeBlockHeight=64*64;
	tccp.isROI=false;
	
	bmptoimage(inFile,&img,subSamplingX,subSamplingY);

	cp.tw=int_ceildiv(img.Xsiz-cp.XTOsiz,cp.XTsiz);
	cp.th=int_ceildiv(img.Ysiz-cp.YTOsiz,cp.YTsiz);

	cp.ppm=(j2kPPM*)malloc(sizeof(j2kPPM));
	cp.ppm->data=0;
	cp.ppm->data=NULL;
	cp.ppm->previous=0;
	cp.ppm->store=0;

	cp.tcps=(TileCodeParam*)malloc(cp.tw*cp.th*sizeof(TileCodeParam));

	int CodingStyle=0;
	
	int numResolution=6;
	int codeBlockWidth=64;
	int codeBlockHeight=64;

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
		tcp->progressionOrder=give_progression("LRCP");
		tcp->isMCT=img.numComponents==3?1:0;

		tcp->ppt->ppt=0;
		tcp->ppt->data=NULL;
		tcp->ppt->store=0;

		//POC可选
		//tcp->pocs=(j2kPOC*)malloc(numPocs*sizeof(j2kPOC));

		int numPocs=0;//先不开POC

/*		for(int i=0;i<numPocs;i++)
		{
			// T1=0,0,1,5,3,CPRL
			j2kPOC *poc=&tcp->pocs[i];

			poc->resolutionStart=0;
			poc->componentStart=0;
			poc->layerEnd=1;
			poc->resolutionEnd=5;
			poc->componentEnd=3;
			poc->progressionOrder=give_progression("CPRL");
			poc->tile=1;
		}*/

		tcp->numPocs=numPocs;

		tcp->tccps=(TileCompCodeParam*)malloc(img.numComponents*sizeof(TileCompCodeParam));
		for(int i=0;i<img.numComponents;i++)
		{
			TileCompCodeParam *tccp=&tcp->tccps[i];

			tccp->codingStyle=CodingStyle&0x01;
			tccp->numResolutions=numResolution;
			tccp->codeBlockWidth=int_floorlog2(codeBlockWidth);
			tccp->codeBlockHeight=int_floorlog2(codeBlockHeight);

			tccp->codeBlockStyle=0;//模式组合开关
			tccp->isReversibleDWT=0;//非可逆
			tccp->quantisationStyle=0;//量化模式
			tccp->numGuardBits=2;

			tccp->isROI=0;//非感兴趣

			for(int j=0;j<tccp->numResolutions;j++)
			{
				tccp->precinctWidth[j]=15;
				tccp->precinctHeight[j]=15;
			}

		}
	}

	jp2Struct *jp2struct=(jp2Struct*)malloc(sizeof(jp2Struct));
	jp2struct->image=&img;

	jp2_init_stdjp2(jp2struct,&img);//初始化框结构
	
	if(cp.isIntermedFile==1)
		cp.isIntermedFile=0;

	char *outbuf=(char*)malloc(cp.XTsiz*cp.YTsiz*cp.tw*cp.th*10*sizeof(char));
	charInputOutput::init((unsigned char*)outbuf,cp.XTsiz*cp.YTsiz*cp.tw*cp.th*10);

	jp2Writer *write=new jp2Writer();
	int len=write->encode(jp2struct,&cp,outbuf);

	if(len>0)
	{
		FILE* file=fopen(outbuf,"wb");
		if(file)
		{
			fwrite(outbuf,1,len,file);
			free(outbuf);
			fclose(file);
		}
	}

	free(img.comps);
	free(cp.tcps);

	cout<<img.numComponents<<endl<<img.colorSpace<<endl;
	system("pause");
}

typedef struct {
	unsigned long int biSize;			/* Size of the structure in bytes */
	unsigned long int biWidth;		/* Width of the image in pixels */
	unsigned long int biHeight;		/* Heigth of the image in pixels */
	unsigned short int biPlanes;		/* 1 */
	unsigned short int biBitCount;		/* Number of color bits by pixels */
	unsigned long int biCompression;		/* Type of encoding 0: none 1: RLE8 2: RLE4 */
	unsigned long int biSizeImage;		/* Size of the image in bytes */
	unsigned long int biXpelsPerMeter;	/* Horizontal (X) resolution in pixels/meter */
	unsigned long int biYpelsPerMeter;	/* Vertical (Y) resolution in pixels/meter */
	unsigned long int biClrUsed;		/* Number of color used in the image (0: ALL) */
	unsigned long int biClrImportant;		/* Number of important color (0: ALL) */
} BITMAPINFOHEADER_t;

typedef struct {
	unsigned short bfType;			/* 'BM' for Bitmap (19776) */
	unsigned long int bfSize;			/* Size of the file        */
	unsigned short bfReserved1;		/* Reserved : 0            */
	unsigned short bfReserved2;		/* Reserved : 0            */
	unsigned long int bfOffBits;		/* Offset                  */
} BITMAPFILEHEADER_t;

#ifndef _FILE_DEFINED
struct _iobuf {
	char *_ptr;
	int   _cnt;
	char *_base;
	int   _flag;
	int   _file;
	int   _charbuf;
	int   _bufsiz;
	char *_tmpfname;
};
typedef struct _iobuf FILE;
#define _FILE_DEFINED
#endif

int bmptoimage(char *filename,jp2Image *img,int subsampling_dx,int subsampling_dy)
{
	int Dim[2]={100,100};

	FILE *IN;
	FILE *Compo0 = 0, *Compo1 = 0, *Compo2 = 0;
	BITMAPFILEHEADER_t File_h;
	BITMAPINFOHEADER_t Info_h;//记录BMP的尺寸等信息
	unsigned char *RGB;
	unsigned char *table_R, *table_G, *table_B;
	unsigned int j, w, h, PAD, type = 0;

	int i;
	int gray_scale = 1, not_end_file = 1; 

	unsigned int line = 0, col = 0;
	unsigned char v, v2;
	unsigned long int W, H;

	IN = fopen(filename, "rb");
	if (!IN) {
		fprintf(stderr,
			"\033[0;33mFailed to open %s for reading !!\033[0;39m\n",
			filename);
		return 0;
	}

	//以下都为小端保存,低位在前
	File_h.bfType = getc(IN);//读取第一个字节,BMP为B,0x42
	File_h.bfType = (getc(IN) << 8) + File_h.bfType;//0x4d42

	//验证是否BMP
	if (File_h.bfType != 19778) {
		printf("Error, not a BMP file!\n");
		return 0;
	} else {
		/* 读取BMP的头 */
		/* ------------- */
		File_h.bfSize = getc(IN);//0x36
		File_h.bfSize = (getc(IN) << 8) + File_h.bfSize;
		File_h.bfSize = (getc(IN) << 16) + File_h.bfSize;
		File_h.bfSize = (getc(IN) << 24) + File_h.bfSize;//0x240036

		File_h.bfReserved1 = getc(IN);
		File_h.bfReserved1 = (getc(IN) << 8) + File_h.bfReserved1;//0x0000

		File_h.bfReserved2 = getc(IN);
		File_h.bfReserved2 = (getc(IN) << 8) + File_h.bfReserved2;//0x0000

		File_h.bfOffBits = getc(IN);
		File_h.bfOffBits = (getc(IN) << 8) + File_h.bfOffBits;
		File_h.bfOffBits = (getc(IN) << 16) + File_h.bfOffBits;
		File_h.bfOffBits = (getc(IN) << 24) + File_h.bfOffBits;//0x36

		/* INFO HEADER */
		/* ------------- */

		Info_h.biSize = getc(IN);
		Info_h.biSize = (getc(IN) << 8) + Info_h.biSize;
		Info_h.biSize = (getc(IN) << 16) + Info_h.biSize;
		Info_h.biSize = (getc(IN) << 24) + Info_h.biSize;

		Info_h.biWidth = getc(IN);
		Info_h.biWidth = (getc(IN) << 8) + Info_h.biWidth;
		Info_h.biWidth = (getc(IN) << 16) + Info_h.biWidth;
		Info_h.biWidth = (getc(IN) << 24) + Info_h.biWidth;
		w = Info_h.biWidth;

		Info_h.biHeight = getc(IN);
		Info_h.biHeight = (getc(IN) << 8) + Info_h.biHeight;
		Info_h.biHeight = (getc(IN) << 16) + Info_h.biHeight;
		Info_h.biHeight = (getc(IN) << 24) + Info_h.biHeight;
		h = Info_h.biHeight;

		Info_h.biPlanes = getc(IN);
		Info_h.biPlanes = (getc(IN) << 8) + Info_h.biPlanes;

		//每个像素所需的位数
		Info_h.biBitCount = getc(IN);
		Info_h.biBitCount = (getc(IN) << 8) + Info_h.biBitCount;

		Info_h.biCompression = getc(IN);
		Info_h.biCompression = (getc(IN) << 8) + Info_h.biCompression;
		Info_h.biCompression = (getc(IN) << 16) + Info_h.biCompression;
		Info_h.biCompression = (getc(IN) << 24) + Info_h.biCompression;

		Info_h.biSizeImage = getc(IN);
		Info_h.biSizeImage = (getc(IN) << 8) + Info_h.biSizeImage;
		Info_h.biSizeImage = (getc(IN) << 16) + Info_h.biSizeImage;
		Info_h.biSizeImage = (getc(IN) << 24) + Info_h.biSizeImage;

		Info_h.biXpelsPerMeter = getc(IN);
		Info_h.biXpelsPerMeter = (getc(IN) << 8) + Info_h.biXpelsPerMeter;
		Info_h.biXpelsPerMeter = (getc(IN) << 16) + Info_h.biXpelsPerMeter;
		Info_h.biXpelsPerMeter = (getc(IN) << 24) + Info_h.biXpelsPerMeter;

		Info_h.biYpelsPerMeter = getc(IN);
		Info_h.biYpelsPerMeter = (getc(IN) << 8) + Info_h.biYpelsPerMeter;
		Info_h.biYpelsPerMeter = (getc(IN) << 16) + Info_h.biYpelsPerMeter;
		Info_h.biYpelsPerMeter = (getc(IN) << 24) + Info_h.biYpelsPerMeter;

		Info_h.biClrUsed = getc(IN);
		Info_h.biClrUsed = (getc(IN) << 8) + Info_h.biClrUsed;
		Info_h.biClrUsed = (getc(IN) << 16) + Info_h.biClrUsed;
		Info_h.biClrUsed = (getc(IN) << 24) + Info_h.biClrUsed;

		Info_h.biClrImportant = getc(IN);
		Info_h.biClrImportant = (getc(IN) << 8) + Info_h.biClrImportant;
		Info_h.biClrImportant = (getc(IN) << 16) + Info_h.biClrImportant;
		Info_h.biClrImportant = (getc(IN) << 24) + Info_h.biClrImportant;

		/* Read the data and store them in the OUT file */
		if (Info_h.biBitCount == 24) {//24位图
			img->XOsiz = Dim[0];
			img->YOsiz = Dim[1];
			img->Xsiz =!Dim[0] ? (w - 1) * subsampling_dx + 1 : Dim[0] + (w -1) *subsampling_dx + 1;
			img->Ysiz =!Dim[1] ? (h - 1) * subsampling_dy + 1 : Dim[1] + (h - 1) *subsampling_dy + 1;

			img->numComponents = 3;//分量为3
			img->colorSpace = 1;
			img->comps =(Component *) malloc(img->numComponents * sizeof(Component));//分量的内存地址 

			for (i = 0; i < img->numComponents; i++) {
				//初始化分量
				img->comps[i].precision = 8;//精准度
				img->comps[i].bpp = 8;//深度
				img->comps[i].sgnd = 0;
				img->comps[i].XRsiz = subsampling_dx;
				img->comps[i].YRsiz = subsampling_dy;
			}

			//创建三个分量文件
			Compo0 = fopen("Compo0", "wb");
			if (!Compo0) {
				fprintf(stderr,
					"\033[0;33mFailed to open Compo0 for writing !\033[0;39m\n");
			}
			Compo1 = fopen("Compo1", "wb");
			if (!Compo1) {
				fprintf(stderr,
					"\033[0;33mFailed to open Compo1 for writing !\033[0;39m\n");
			}
			Compo2 = fopen("Compo2", "wb");
			if (!Compo2) {
				fprintf(stderr,
					"\033[0;33mFailed to open Compo2 for writing !\033[0;39m\n");
			}

			/* Place the cursor at the beginning of the image information */
			fseek(IN, 0, SEEK_SET);
			fseek(IN, File_h.bfOffBits, SEEK_SET);//跳到位置数据的起始位置 

			//位图大小 
			W = Info_h.biWidth;
			H = Info_h.biHeight;

			// PAD = 4 - (3 * W) % 4;
			// PAD = (PAD == 4) ? 0 : PAD;

			//当是24位图的时候,一个像素占用3位
			PAD = (3 * W) % 4 ? 4 - (3 * W) % 4 : 0;//0~3,一行位数必须是4的倍数,不够就补0,计算出一行还有几个才够4的倍数
			RGB =(unsigned char *) malloc((3 * W + PAD) * H * sizeof(unsigned char));

			fread(RGB, sizeof(unsigned char), (3 * W + PAD) * H, IN);//从BMP的数据起始位置开始读入sizeof(unsigned char)*(3*W+PAD)个数据到RGB

			for (j = 0; j < (3 * W + PAD) * H; j++) {
				//分散流到三个文件当中
				unsigned char elmt;
				int Wp = 3 * W + PAD;//一行的字节数

				elmt = RGB[(H - (j / Wp + 1)) * Wp + j % Wp];//j/Wp是所在行数,最后得出来的就是当前j所在的位置,倒序
				if ((j % Wp) < (3 * W)) {
					switch (type) {
					case 0:
						fprintf(Compo2, "%c", elmt);
						type = 1;
						break;
					case 1:
						fprintf(Compo1, "%c", elmt);
						type = 2;
						break;
					case 2:
						fprintf(Compo0, "%c", elmt);
						type = 0;
						break;
					}
				}
			}

			fclose(Compo0);
			fclose(Compo1);
			fclose(Compo2);
			free(RGB);
		} else if (Info_h.biBitCount == 8 && Info_h.biCompression == 0) {
			img->XOsiz = Dim[0];
			img->YOsiz = Dim[1];
			img->Xsiz =!Dim[0] ? (w - 1) * subsampling_dx + 1 : Dim[0] + (w -subsampling_dx )+ 1;
			img->Ysiz =!Dim[1] ? (h - 1) * subsampling_dy + 1 : Dim[1] + (h - 1) *subsampling_dy + 1;

			table_R = (unsigned char *) malloc(256 * sizeof(unsigned char));
			table_G = (unsigned char *) malloc(256 * sizeof(unsigned char));
			table_B = (unsigned char *) malloc(256 * sizeof(unsigned char));

			for (j = 0; j < Info_h.biClrUsed; j++) {
				table_B[j] = getc(IN);
				table_G[j] = getc(IN);
				table_R[j] = getc(IN);
				getc(IN);
				if (table_R[j] != table_G[j] && table_R[j] != table_B[j]
				&& table_G[j] != table_B[j])
					gray_scale = 0;
			}

			/* Place the cursor at the beginning of the image information */
			fseek(IN, 0, SEEK_SET);
			fseek(IN, File_h.bfOffBits, SEEK_SET);

			W = Info_h.biWidth;
			H = Info_h.biHeight;
			if (Info_h.biWidth % 2)
				W++;

			RGB = (unsigned char *) malloc(W * H * sizeof(unsigned char));

			fread(RGB, sizeof(unsigned char), W * H, IN);

			if (gray_scale) {
				img->numComponents = 1;
				img->comps =(Component *) malloc(img->numComponents * sizeof(Component));
				img->comps[0].precision = 8;
				img->comps[0].bpp = 8;
				img->comps[0].sgnd = 0;
				img->comps[0].XRsiz = subsampling_dx;
				img->comps[0].YRsiz = subsampling_dy;
				Compo0 = fopen("Compo0", "wb");
				if (!Compo0) {
					fprintf(stderr,
						"\033[0;33mFailed to open Compo0 for writing !\033[0;39m\n");
				}
				for (j = 0; j < W * H; j++) {
					if ((j % W < W - 1 && Info_h.biWidth % 2)
						|| !(Info_h.biWidth % 2))
						fprintf(Compo0, "%c",
						table_R[RGB[W * H - ((j) / (W) + 1) * W + (j) % (W)]]);
				}
				fclose(Compo0);
			} else {
				img->numComponents = 3;
				img->comps =(Component *) malloc(img->numComponents * sizeof(Component));
				for (i = 0; i < img->numComponents; i++) {
					img->comps[i].precision = 8;
					img->comps[i].bpp = 8;
					img->comps[i].sgnd = 0;
					img->comps[i].XRsiz = subsampling_dx;
					img->comps[i].YRsiz = subsampling_dy;
				}

				Compo0 = fopen("Compo0", "wb");
				if (!Compo0) {
					fprintf(stderr,
						"\033[0;33mFailed to open Compo0 for writing !\033[0;39m\n");
				}
				Compo1 = fopen("Compo1", "wb");
				if (!Compo1) {
					fprintf(stderr,
						"\033[0;33mFailed to open Compo1 for writing !\033[0;39m\n");
				}
				Compo2 = fopen("Compo2", "wb");
				if (!Compo2) {
					fprintf(stderr,
						"\033[0;33mFailed to open Compo2 for writing !\033[0;39m\n");
				}

				for (j = 0; j < W * H; j++) {
					if ((j % W < W - 1 && Info_h.biWidth % 2)
						|| !(Info_h.biWidth % 2)) {
							fprintf(Compo0, "%c",
								table_R[RGB[W * H - ((j) / (W) + 1) * W + (j) % (W)]]);
							fprintf(Compo1, "%c",
								table_G[RGB[W * H - ((j) / (W) + 1) * W + (j) % (W)]]);
							fprintf(Compo2, "%c",
								table_B[RGB[W * H - ((j) / (W) + 1) * W + (j) % (W)]]);
					}

				}
				fclose(Compo0);
				fclose(Compo1);
				fclose(Compo2);
			}
			free(RGB);

		} else if (Info_h.biBitCount == 8 && Info_h.biCompression == 1) {
			img->XOsiz = Dim[0];
			img->YOsiz = Dim[1];
			img->Xsiz =!Dim[0] ? (w - 1) * subsampling_dx + 1 : Dim[0] + (w -1) *subsampling_dx + 1;
			img->Ysiz =!Dim[1] ? (h - 1) * subsampling_dy + 1 : Dim[1] + (h -1) *subsampling_dy + 1;

			table_R = (unsigned char *) malloc(256 * sizeof(unsigned char));
			table_G = (unsigned char *) malloc(256 * sizeof(unsigned char));
			table_B = (unsigned char *) malloc(256 * sizeof(unsigned char));

			for (j = 0; j < Info_h.biClrUsed; j++) {
				table_B[j] = getc(IN);
				table_G[j] = getc(IN);
				table_R[j] = getc(IN);
				getc(IN);
				if (table_R[j] != table_G[j] && table_R[j] != table_B[j]
				&& table_G[j] != table_B[j])
					gray_scale = 0;
			}

			/* Place the cursor at the beginning of the image information */
			fseek(IN, 0, SEEK_SET);
			fseek(IN, File_h.bfOffBits, SEEK_SET);

			if (gray_scale) {
				img->numComponents = 1;
				img->comps = (Component *) malloc(sizeof(Component));
				img->comps[0].precision = 8;
				img->comps[0].bpp = 8;
				img->comps[0].sgnd = 0;
				img->comps[0].XRsiz = subsampling_dx;
				img->comps[0].YRsiz = subsampling_dy;
				Compo0 = fopen("Compo0", "wb");
				if (!Compo0) {
					fprintf(stderr,
						"\033[0;33mFailed to open Compo0 for writing !\033[0;39m\n");
				}
			} else {
				img->numComponents = 3;
				img->comps =(Component *) malloc(img->numComponents * sizeof(Component));
				for (i = 0; i < img->numComponents; i++) {
					img->comps[i].precision = 8;
					img->comps[i].bpp = 8;
					img->comps[i].sgnd = 0;
					img->comps[i].XRsiz = subsampling_dx;
					img->comps[i].YRsiz = subsampling_dy;
				}
				Compo0 = fopen("Compo0", "wb");
				if (!Compo0) {
					fprintf(stderr,
						"\033[0;33mFailed to open Compo0 for writing !\033[0;39m\n");
				}
				Compo1 = fopen("Compo1", "wb");
				if (!Compo1) {
					fprintf(stderr,
						"\033[0;33mFailed to open Compo1 for writing !\033[0;39m\n");
				}
				Compo2 = fopen("Compo2", "wb");
				if (!Compo2) {
					fprintf(stderr,
						"\033[0;33mFailed to open Compo2 for writing !\033[0;39m\n");
				}
			}

			RGB =
				(unsigned char *) malloc(Info_h.biWidth * Info_h.biHeight *
				sizeof(unsigned char));

			while (not_end_file) {
				v = getc(IN);
				if (v) {
					v2 = getc(IN);
					for (i = 0; i < (int) v; i++) {
						RGB[line * Info_h.biWidth + col] = v2;
						col++;
					}
				} else {
					v = getc(IN);
					switch (v) {
					case 0:
						col = 0;
						line++;
						break;
					case 1:
						line++;
						not_end_file = 0;
						break;
					case 2:
						printf("No Delta supported\n");
						return 1;
						break;
					default:
						for (i = 0; i < v; i++) {
							v2 = getc(IN);
							RGB[line * Info_h.biWidth + col] = v2;
							col++;
						}
						if (v % 2)
							v2 = getc(IN);
					}
				}
			}
			if (gray_scale) {
				for (line = 0; line < Info_h.biHeight; line++)
					for (col = 0; col < Info_h.biWidth; col++)
						fprintf(Compo0, "%c", table_R[(int)
						RGB[(Info_h.biHeight - line -
						1) * Info_h.biWidth +
						col]]);
				fclose(Compo0);
			} else {
				for (line = 0; line < Info_h.biHeight; line++)
					for (col = 0; col < Info_h.biWidth; col++) {
						fprintf(Compo0, "%c", table_R[(int)
							RGB[(Info_h.biHeight - line -
							1) * Info_h.biWidth +
							col]]);
						fprintf(Compo1, "%c", table_G[(int)
							RGB[(Info_h.biHeight - line -
							1) * Info_h.biWidth +
							col]]);
						fprintf(Compo2, "%c", table_B[(int)
							RGB[(Info_h.biHeight - line -
							1) * Info_h.biWidth +
							col]]);
					}
					fclose(Compo0);
					fclose(Compo1);
					fclose(Compo2);
			}
			free(RGB);
		} else
			fprintf(stderr,
			"Other system than 24 bits/pixels or 8 bits (no RLE coding) is not yet implemented [%d]\n",
			Info_h.biBitCount);

		fclose(IN);
		return 1;
	}
}

int give_progression(char progression[4])
{
	if (progression[0] == 'L' && progression[1] == 'R'
		&& progression[2] == 'C' && progression[3] == 'P') {
			//LRCP
			return 0;
	} else {
		if (progression[0] == 'R' && progression[1] == 'L'
			&& progression[2] == 'C' && progression[3] == 'P') {
				//RLCP
				return 1;
		} else {
			if (progression[0] == 'R' && progression[1] == 'P'
				&& progression[2] == 'C' && progression[3] == 'L') {
					//RPCL
					return 2;
			} else {
				if (progression[0] == 'P' && progression[1] == 'C'
					&& progression[2] == 'R' && progression[3] == 'L') {
						//PCRL
						return 3;
				} else {
					if (progression[0] == 'C' && progression[1] == 'P'
						&& progression[2] == 'R' && progression[3] == 'L') {
							//CPRL
							return 4;
					} else {
						return -1;
					}
				}
			}
		}
	}
}

//标准框标识
#define JPIP_JPIP 0x6a706970
#define JP2_JP   0x6a502020
#define JP2_FTYP 0x66747970
#define JP2_JP2H 0x6a703268
#define JP2_IHDR 0x69686472
#define JP2_COLR 0x636f6c72
#define JP2_JP2C 0x6a703263
#define JP2_URL  0x75726c20
#define JP2_DBTL 0x6474626c
#define JP2_BPCC 0x62706363
#define JP2_JP2  0x6a703220

int jp2_init_stdjp2(jp2Struct * jp2_struct, jp2Image * img)
{
	int depth_0, sign, depth, i;

	jp2_struct->height = img->Ysiz - img->YOsiz;	// HEIGHT
	jp2_struct->width = img->Xsiz - img->XOsiz;	// WIDTH
	jp2_struct->numComponents = img->numComponents;	// NC
	jp2_struct->comps = (jp2Component *) malloc(jp2_struct->numComponents * sizeof(jp2Component));

	depth_0 = img->comps[0].precision - 1;//图像其中一个分量的分区数
	sign = img->comps[0].sgnd;
	jp2_struct->bpc = depth_0 + (sign << 7);

	for (i = 1; i < img->numComponents; i++) {
		depth = img->comps[i].precision - 1;//获取其他分量的分区数
		sign = img->comps[i].sgnd;
		if (depth_0 != depth)//如果其他位深与第一个分量的位深不等
			jp2_struct->bpc = 255;// 设置每个图像分量位尝试为255
	}

	jp2_struct->C = 7;		// C : Always 7
	jp2_struct->UnkC = 0;		// UnkC, colorspace specified in colr box
	jp2_struct->IPR = 0;		// IPR, no intellectual property,知识产权

	for (i = 0; i < img->numComponents; i++)
		jp2_struct->comps[i].bpcc =
		img->comps[i].precision - 1 + (img->comps[i].sgnd << 7);

	jp2_struct->precedence = 0;	// PRECEDENCE,优先级 
	jp2_struct->approx = 0;	// APPROX

	if ((img->numComponents== 1 || img->numComponents == 3)//灰度图或者 RGB图
		&& (jp2_struct->bpc != 255))//以及位深不等于255
		jp2_struct->meth = 1;//ECS
	else
		jp2_struct->meth = 2;//ICP

	if (jp2_struct->meth == 1) {
		if (img->colorSpace == 1)
			//如果是通过ECS发送且色域为sRGB
			jp2_struct->ECS = 16;
		else if (img->colorSpace == 2)
			//单色空间
			jp2_struct->ECS = 17;
		else if (img->colorSpace == 3)
			jp2_struct->ECS = 18;	// YUV                          
	} else
		jp2_struct->ECS = 0;	// PROFILE (??)

	jp2_struct->brand = JP2_JP2;	/* BR         */
	jp2_struct->minversion = 0;	/* MinV       */

	jp2_struct->numcl = 1;//兼容列表
	jp2_struct->cl = (unsigned int *) malloc(jp2_struct->numcl * sizeof(unsigned int));
	jp2_struct->cl[0] = JP2_JP2;	/* CL0 : JP2  */
	return 0;
}