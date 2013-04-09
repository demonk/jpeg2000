#include "j2kCoder.h"
#include "charInputOutput.h"
int j2kCoder::j2kEncode(jp2Image *img,CodeParam *comp,char *output,int len)
{

	FILE *file=NULL;
	char *dest=NULL;

	image=img;
	cp=comp;
// 	if(cp->isIntermedFile==1)
// 	{
// 		file=fopen(output,"wb");
// 		if(!file)
// 			return -1;
// 
// 		unsigned char *dest=(unsigned char *)malloc(len);
// 		charInputOutput::init(dest,len);
// 	}

	state=J2K_STATE_MHSOC;//SOC码流开始 
	writeSOC();
	state = J2K_STATE_MHSIZ;//SIZ标记,图像和拼接块大小
	writeSIZ();

	state = J2K_STATE_MH;

	writeCOD();//COD标记,写入默认编码形式
	writeQCD();//QCD标记,写入默认量化 
	//以上为必要的主标头
	//////////////////////////////////////////////////////////////////////////

	//遍历分量看对应的tile是否有开ROI,有就设置标记
	for(int compno=0;compno<image->numComponents;compno++)
	{
		TileCodeParam *tcp=&cp->tcps[0];
		if(tcp->tccps[compno].isROI)
			writeROI(compno,0);
	}

	if(cp->comment!=NULL)
		writeComment();

	//////////////////////////////////////////////////////////////////////////
	//以上为可选的主标头

// 	if(cp->isIntermedFile==1)
// 	{
// 		currectPos=charInputOutput::getPosition();
// 		fwrite(dest,1,currectPos,file);
// 	}

	tileCoder=new j2kTileCoder(comp,img,currectTileNo);
	/*
	拼接块码流:
	SOT->POC(可选)->PPT(可选)->PLT(可选)->COM(可选)->SOD
	*/
	for(int tileno=0;tileno<cp->tw*cp->th;tileno++)
	{
// 		if(cp->isIntermedFile==1)
// 		{
// 			free(dest);
// 			dest=(char*)malloc(len);
// 			charInputOutput::init((unsigned char*)dest,len);
// 		}
		currectTileNo=tileno;
		
		tileCoder->setCurrentTile(tileno);
		if(tileno==0)
		{	
			tileCoder->tcdMallocEncode();
		}else{
			tileCoder->tcdInitEncode();
		}

			state = J2K_STATE_TPHSOT;//SOT标记,拼接块标头
			writeSOT();//必需
			state = J2K_STATE_TPH;

			for(int compno=1;compno<img->numComponents;compno++)
			{
				//可选
				writeCOC(compno);
				writeQCC(compno);
			}

			//设置写入POC数据到TCP中
			if(cp->tcps[tileno].numPocs)
				writePOC();//可选

			
			writeSOD();//必须
		
// 			if(cp->isIntermedFile==1)
// 			{
// 				fwrite(dest,1,charInputOutput::getPosition(),file);
// 				currectPos=charInputOutput::getPosition()+currectPos;
// 			}
		}

// 		if(cp->isIntermedFile==1)
// 		{
// 			free(dest);
// 			dest=(char*)malloc(len*sizeof(char));
// 			charInputOutput::init((unsigned char *)dest,len);
// 		}

		writeEOC();

// 		if(cp->isIntermedFile==1)
// 		{
// 			fwrite(dest,1,2,file);
// 			free(dest);
// 			fclose(file);
// 		}

		delete tileCoder;
		return charInputOutput::getPosition()+currectPos;
	
}
void j2kCoder::writeSOC()
{
	charInputOutput::writeBytes(J2K_MS_SOC, 2);
}
void j2kCoder::writeSIZ()
{
	int i;
	int lenp, len;

	charInputOutput::writeBytes(J2K_MS_SIZ, 2);	/* SIZ                 */
	lenp = charInputOutput::getPosition();
	charInputOutput::skipBytes(2);
	charInputOutput::writeBytes(0, 2);		/* Rsiz (capabilities) */
	charInputOutput::writeBytes(image->Xsiz, 4);	/* Xsiz                */
	charInputOutput::writeBytes(image->Ysiz, 4);	/* Ysiz                */
	charInputOutput::writeBytes(image->XOsiz, 4);	/* X0siz               */
	charInputOutput::writeBytes(image->YOsiz, 4);	/* Y0siz               */
	charInputOutput::writeBytes(cp->XTsiz, 4);	/* XTsiz               */
	charInputOutput::writeBytes(cp->YTsiz, 4);	/* YTsiz               */
	charInputOutput::writeBytes(cp->XTOsiz, 4);	/* XT0siz              */
	charInputOutput::writeBytes(cp->YTOsiz, 4);	/* YT0siz              */
	charInputOutput::writeBytes(image->numComponents, 2);	/* Csiz                */
	for (i = 0; i < image->numComponents; i++) {
		charInputOutput::writeBytes(image->comps[i].precision - 1 + (image->comps[i].sgnd << 7), 1);	/* Ssiz_i */
		charInputOutput::writeBytes(image->comps[i].XRsiz, 1);	/* XRsiz_i             */
		charInputOutput::writeBytes(image->comps[i].YRsiz, 1);	/* YRsiz_i             */
	}
	len = charInputOutput::getPosition() - lenp;
	charInputOutput::setPosition(lenp);
	charInputOutput::writeBytes(len, 2);		/* Lsiz                */
	charInputOutput::setPosition(lenp + len);
}
void j2kCoder::writeSOT()
{
	int lenp, len;

	SOT_Start = charInputOutput::getPosition();
	charInputOutput::writeBytes(J2K_MS_SOT, 2);	/* SOT */
	lenp = charInputOutput::getPosition();
	charInputOutput::skipBytes(2);			/* Lsot (further) */
	charInputOutput::writeBytes(currectTileNo, 2);	/* Isot */
	charInputOutput::skipBytes(4);			/* Psot (further in j2k_write_sod) */
	charInputOutput::writeBytes(0, 1);		/* TPsot */
	charInputOutput::writeBytes(1, 1);		/* TNsot */
	len = charInputOutput::getPosition() - lenp;
	charInputOutput::setPosition(lenp);
	charInputOutput::writeBytes(len, 2);		/* Lsot */
	charInputOutput::setPosition(lenp + len);
}
void j2kCoder::writeEOC()
{
	charInputOutput::writeBytes(J2K_MS_EOC, 2);
}
void j2kCoder::writeCOD()
{
	TileCodeParam *tcp;
	int lenp, len;

	charInputOutput::writeBytes(J2K_MS_COD, 2);	/* COD */

	lenp = charInputOutput::getPosition();
	charInputOutput::skipBytes(2);

	tcp = &cp->tcps[currectTileNo];
	charInputOutput::writeBytes(tcp->codingStyle, 1);	/* Scod */
	charInputOutput::writeBytes(tcp->progressionOrder, 1);	/* SGcod (A) */
	charInputOutput::writeBytes(tcp->numLayers, 2);	/* SGcod (B) */
	charInputOutput::writeBytes(tcp->isMCT, 1);	/* SGcod (C) */

	writeCOX(0);
	len = charInputOutput::getPosition()- lenp;
	charInputOutput::setPosition(lenp);
	charInputOutput::writeBytes(len, 2);		/* Lcod */
	charInputOutput::setPosition(lenp + len);
}
void j2kCoder::writeQCD()
{
	int lenp, len;

	charInputOutput::writeBytes(J2K_MS_QCD, 2);	/* QCD */
	lenp = charInputOutput::getPosition();
	charInputOutput::skipBytes(2);
	writeQCX(0);
	len = charInputOutput::getPosition() - lenp;
	charInputOutput::setPosition(lenp);
	charInputOutput::writeBytes(len, 2);		/* Lqcd */
	charInputOutput::setPosition(lenp + len);

}
void j2kCoder::writeROI(int compno,int tileno)
{
	TileCodeParam *tcp = &cp->tcps[tileno];

	charInputOutput::writeBytes(J2K_MS_RGN, 2);	/* RGN  */
	charInputOutput::writeBytes(image->numComponents<= 256 ? 5 : 6, 2);	/* Lrgn */
	charInputOutput::writeBytes(compno, image->numComponents <= 256 ? 1 : 2);	/* Crgn */
	charInputOutput::writeBytes(0, 1);		/* Srgn */
	charInputOutput::writeBytes(tcp->tccps[compno].isROI, 1);	/* SPrgn */
}
void j2kCoder::writeComment()
{
	unsigned int i;
	int lenp, len;
	char str[256];
	sprintf(str, "%s", cp->comment);

	charInputOutput::writeBytes(J2K_MS_COM, 2);
	lenp = charInputOutput::getPosition();
	charInputOutput::skipBytes(2);
	charInputOutput::writeBytes(0, 2);
	for (i = 0; i < strlen(str); i++) {
		charInputOutput::writeBytes(str[i], 1);
	}
	len = charInputOutput::getPosition() - lenp;
	charInputOutput::setPosition(lenp);
	charInputOutput::writeBytes(len, 2);
	charInputOutput::setPosition(lenp + len);
}
void j2kCoder::writePOC()
{
	int len, numpchgs, i;
	TileCodeParam *tcp;
	TileCompCodeParam *tccp;

	tcp = &cp->tcps[currectTileNo];
	tccp = &tcp->tccps[0];
	numpchgs = tcp->numPocs;
	charInputOutput::writeBytes(J2K_MS_POC, 2);	/* POC  */
	len = 2 + (5 + 2 * (image->numComponents <= 256 ? 1 : 2)) * numpchgs;
	charInputOutput::writeBytes(len, 2);		/* Lpoc */
	for (i = 0; i < numpchgs; i++) {
		// MODIF
		j2kPOC *poc;
		poc = &tcp->pocs[i];
		charInputOutput::writeBytes(poc->resolutionStart, 1);	/* RSpoc_i */
		charInputOutput::writeBytes(poc->componentStart, (image->numComponents <= 256 ? 1 : 2));	/* CSpoc_i */
		charInputOutput::writeBytes(poc->layerEnd, 2);	/* LYEpoc_i */
		poc->layerEnd = int_min(poc->layerEnd, tcp->numLayers);
		charInputOutput::writeBytes(poc->resolutionEnd, 1);	/* REpoc_i */
		poc->resolutionEnd = int_min(poc->resolutionEnd, tccp->numResolutions);
		charInputOutput::writeBytes(poc->componentEnd, (image->numComponents <= 256 ? 1 : 2));	/* CEpoc_i */
		poc->componentEnd = int_min(poc->componentEnd, image->numComponents);
		charInputOutput::writeBytes(poc->progressionOrder, 1);	/* Ppoc_i */
	}
}
void j2kCoder::writeQCC(int compno){
	int lenp, len;

	charInputOutput::writeBytes(J2K_MS_QCC, 2);	/* QCC */
	lenp = charInputOutput::getPosition();
	charInputOutput::skipBytes(2);
	charInputOutput::writeBytes(compno, image->numComponents <= 256 ? 1 : 2);	/* Cqcc */
	writeQCX(compno);
	len = charInputOutput::getPosition() - lenp;
	charInputOutput::setPosition(lenp);
	charInputOutput::writeBytes(len, 2);		/* Lqcc */
	charInputOutput::setPosition(lenp + len);
}
void j2kCoder::writeCOC(int compno)
{
	TileCodeParam *tcp;
	int lenp, len;

	charInputOutput::writeBytes(J2K_MS_COC, 2);	/* COC */
	lenp = charInputOutput::getPosition();
	charInputOutput::skipBytes(2);
	tcp = &cp->tcps[currectTileNo];
	charInputOutput::writeBytes(compno, image->numComponents <= 256 ? 1 : 2);	/* Ccoc */
	charInputOutput::writeBytes(tcp->tccps[compno].codingStyle, 1);	/* Scoc */
	writeCOX(compno);
	len = charInputOutput::getPosition() - lenp;
	charInputOutput::setPosition(lenp);
	charInputOutput::writeBytes(len, 2);		/* Lcoc */
	charInputOutput::setPosition(lenp + len);
}
void j2kCoder::writeCOX(int compno)
{
	int i;
	TileCodeParam *tcp;
	TileCompCodeParam *tccp;
	tcp = &cp->tcps[currectTileNo];
	tccp = &tcp->tccps[compno];

	charInputOutput::writeBytes(tccp->numResolutions - 1, 1);	/* SPcox (D) */
	charInputOutput::writeBytes(tccp->codeBlockWidth - 2, 1);	/* SPcox (E) */
	charInputOutput::writeBytes(tccp->codeBlockWidth - 2, 1);	/* SPcox (F) */
	charInputOutput::writeBytes(tccp->codeBlockStyle, 1);	/* SPcox (G) */
	charInputOutput::writeBytes(tccp->isReversibleDWT, 1);	/* SPcox (H) */

	if (tccp->codeBlockStyle & J2K_CCP_CSTY_PRT) {
		for (i = 0; i < tccp->numResolutions; i++) {
			charInputOutput::writeBytes(tccp->precinctWidth[i] + (tccp->precinctHeight[i] << 4), 1);	/* SPcox (I_i) */
		}
	}
}
void j2kCoder::writeQCX(int compno)
{
	TileCodeParam *tcp;
	TileCompCodeParam *tccp;
	int bandno, numbands;
	int expn, mant;

	tcp = &cp->tcps[currectTileNo];
	tccp = &tcp->tccps[compno];

	charInputOutput::writeBytes(tccp->quantisationStyle + (tccp->numGuardBits << 5), 1);	/* Sqcx */
	numbands =
		tccp->quantisationStyle ==J2K_CCP_QNTSTY_SIQNT ? 1 : tccp->numResolutions * 3 - 2;

	for (bandno = 0; bandno < numbands; bandno++) {
		expn = tccp->stepsizes[bandno].expn;
		mant = tccp->stepsizes[bandno].mant;

		if (tccp->quantisationStyle == J2K_CCP_QNTSTY_NOQNT) {
			charInputOutput::writeBytes(expn << 3, 1);	/* SPqcx_i */
		} else {
			charInputOutput::writeBytes((expn << 11) + mant, 2);	/* SPqcx_i */
		}
	}
}
void j2kCoder::writeSOD()
{
	int l, layno;
	int totlen;
	TileCodeParam *tcp;

	charInputOutput::writeBytes(J2K_MS_SOD, 2);//写入SOD码流标记 

	if (currectTileNo == 0) {
		//如果是第一个tile
		SOD_Start = charInputOutput::getPosition() + currectPos;//记录SOD开始的位置
	}

	tcp = &cp->tcps[currectTileNo];
	for (layno = 0; layno < tcp->numLayers; layno++) {
		//遍历质量层,对每层质量层赋值
		tcp->rates[layno] = tcp->rates[layno]-(SOD_Start / (cp->th * cp->tw));
	}

	if (cp->imageType)//输入图像后缀为非pgx
		l = tileCoder->tcdEncodeTilePxm(currectTileNo,charInputOutput::getCurrectChar(),charInputOutput::getLeftBytesLength() -2 );//jp2用的是这种编码方式方式

	/* Writing Psot in SOT marker */
	totlen = charInputOutput::getPosition() + l - SOT_Start;
	charInputOutput::setPosition(SOT_Start + 6);
	charInputOutput::writeBytes(totlen, 4);
	charInputOutput::setPosition(SOT_Start + totlen);
}
