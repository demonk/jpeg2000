#include "bmpReader.h"

namespace bmpReader
{
	int bmpToImage(char *filename,jp2Image *img,int subSamplingX,int subSamplingY)
	{
			int Dim[2]={0,0};//先不自定义偏移

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
			img->Xsiz =!Dim[0] ? (w - 1) * subSamplingX + 1 : Dim[0] + (w -1) *subSamplingX + 1;
			img->Ysiz =!Dim[1] ? (h - 1) * subSamplingY + 1 : Dim[1] + (h - 1) *subSamplingY + 1;

			img->numComponents = 3;//分量为3
			img->colorSpace = 1;
			img->comps =(Component *) malloc(img->numComponents * sizeof(Component));//分量的内存地址 

			for (i = 0; i < img->numComponents; i++) {
				//初始化分量
				img->comps[i].precision = 8;//精准度
				img->comps[i].bpp = 8;//深度
				img->comps[i].sgnd = 0;
				img->comps[i].XRsiz = subSamplingX;
				img->comps[i].YRsiz = subSamplingY;
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
		} 
/*		else if (Info_h.biBitCount == 8 && Info_h.biCompression == 0) {
			img->XOsiz = Dim[0];
			img->YOsiz = Dim[1];
			img->Xsiz =!Dim[0] ? (w - 1) * subSamplingX + 1 : Dim[0] + (w -subSamplingX )+ 1;
			img->Ysiz =!Dim[1] ? (h - 1) * subSamplingY + 1 : Dim[1] + (h - 1) *subSamplingY + 1;

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

			// Place the cursor at the beginning of the image information 
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
				img->comps[0].XRsiz = subSamplingX;
				img->comps[0].YRsiz = subSamplingY;
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
					img->comps[i].XRsiz = subSamplingX;
					img->comps[i].YRsiz = subSamplingY;
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
			img->Xsiz =!Dim[0] ? (w - 1) * subSamplingX + 1 : Dim[0] + (w -1) *subSamplingX + 1;
			img->Ysiz =!Dim[1] ? (h - 1) * subSamplingY + 1 : Dim[1] + (h -1) *subSamplingY + 1;

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

			// Place the cursor at the beginning of the image information 
			fseek(IN, 0, SEEK_SET);
			fseek(IN, File_h.bfOffBits, SEEK_SET);

			if (gray_scale) {
				img->numComponents = 1;
				img->comps = (Component *) malloc(sizeof(Component));
				img->comps[0].precision = 8;
				img->comps[0].bpp = 8;
				img->comps[0].sgnd = 0;
				img->comps[0].XRsiz = subSamplingX;
				img->comps[0].YRsiz = subSamplingY;
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
					img->comps[i].XRsiz = subSamplingX;
					img->comps[i].YRsiz = subSamplingY;
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
		} */
 else
			fprintf(stderr,
			"Other system than 24 bits/pixels or 8 bits (no RLE coding) is not yet implemented [%d]\n",
			Info_h.biBitCount);

		fclose(IN);
		
		return 1;
	}
	}
}