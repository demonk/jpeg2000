#ifndef J2K_TIER_ONE
#define J2K_TIER_ONE
//用8位定义方向
#define T1_SIG_NE 0x0001//North East
#define T1_SIG_SE 0x0002//South East
#define T1_SIG_SW 0x0004//South West
#define T1_SIG_NW 0x0008//North West
#define T1_SIG_N 0x0010//North
#define T1_SIG_E 0x0020//East
#define T1_SIG_S 0x0040//South
#define T1_SIG_W 0x0080//West

#define T1_SIG_OTH (T1_SIG_N|T1_SIG_NE|T1_SIG_E|T1_SIG_SE|T1_SIG_S|T1_SIG_SW|T1_SIG_W|T1_SIG_NW)//定义全方向8个领域
#define T1_SIG_PRIM (T1_SIG_N|T1_SIG_E|T1_SIG_S|T1_SIG_W)//只定义样本点上下左右点

#define T1_SGN_N 0x0100
#define T1_SGN_E 0x0200
#define T1_SGN_S 0x0400
#define T1_SGN_W 0x0800
#define T1_SGN (T1_SGN_N|T1_SGN_E|T1_SGN_S|T1_SGN_W)

//定义每个原语下有多少个上下文
#define T1_NUMCTXS_AGG 1
#define T1_NUMCTXS_ZC 9/* ZC */
#define T1_NUMCTXS_MAG 3 /* MR */
#define T1_NUMCTXS_SC 5 /* SC */
#define T1_NUMCTXS_UNI 1 /* RLC,UNIFORM上下文 */

#define T1_CTXNO_AGG 0 
#define T1_CTXNO_ZC (T1_CTXNO_AGG+T1_NUMCTXS_AGG)
#define T1_CTXNO_MAG (T1_CTXNO_ZC+T1_NUMCTXS_ZC)
#define T1_CTXNO_SC (T1_CTXNO_MAG+T1_NUMCTXS_MAG)//符号上下文标记起始=MAG起始+长度
#define T1_CTXNO_UNI (T1_CTXNO_SC+T1_NUMCTXS_SC)
#define T1_NUMCTXS (T1_CTXNO_UNI+T1_NUMCTXS_UNI)

#define T1_NMSEDEC_BITS 7
#define T1_NMSEDEC_FRACBITS (T1_NMSEDEC_BITS-1)

#define T1_SIG 0x1000
#define T1_REFINE 0x2000 //标记在magnitude refinement 中是否已经标记
#define T1_VISIT 0x4000 //标记是否已经访问

#define T1_TYPE_MQ 0
#define T1_TYPE_RAW 1

#define T1_MAXCBLKW 1024
#define T1_MAXCBLKH 1024

/*--------MQC--------*/
#define MQC_NUMCTXS 32

#include "j2kCustom.h"
#include "j2kMCT.h"
#include "j2kDWT.h"
#include "TileCodeParam.h"
#include "j2kTileCoder_Custom.h"

/* 这个数组定义了上下文所有可能出现的情况 */

class j2kTierOne
{
private:
	/*---------TIER1--------*/
	//上下文
	int zcContextNo[1024];
	int scContextNo[256];
	int maContextNo[4096];
	int spbNo[256];

	int nmSeDecSig[1<<T1_NMSEDEC_BITS];
	int nmSeDecSig0[1<<T1_NMSEDEC_BITS];
	int nmSeDecRef[1<<T1_NMSEDEC_BITS];
	int nmSeDecRef0[1<<T1_NMSEDEC_BITS];

	int tierOneData[1024][1024];
	int tierOneFlags[1026][1026];

	/*----------MQC----------*/
	mqc_state_t *mqcCtxs[32];
	mqc_state_t **mqcCurCtx;/*当前的上下文信息*/
	unsigned int mqc_a;
	unsigned int mqc_c;
	unsigned int mqc_ct;
	unsigned char *mqc_bp;
	unsigned char *mqc_start;
	unsigned char *mqc_end;

public:
	void tierOneInitContext();/* 初始化上下文 */
	void tierOneEncodeCodeBlocks(j2kTileCoder_Tile *tile,TileCodeParam *tcp);
	void tierOneEncodeCodeBlock(j2kTileCoder_CodeBlock *codeBlock,j2kTileCoder_Tile *tile,int orient,int compno,int level,int isDWT,double stepSize,int codeBlockStyle,int numComps);
	int initCtxZC(int ctx,int bandno);
	int initCtxSC(int ctx);
	int initCtxMAG(int ctx);
	int initSPB(int ctx);
	int getCtxNoZC(int orient,int flag);
	int getCtxNoSC(int flag);
	int getCtxNoMAG(int flag);
	int getSPB(int flag);

	void mqcResetStates();
	void mqcSetState(int ctxno,int msb,int prob);/* (上下文标记,上下文的MSB新state,标识上下文中新state中symbols的可能性) */
	void mqcInitEncode(unsigned char *data);
	void mqcSegMarkEncode();/*模式变化 ,SEGMARK*/
	void mqcByPassInitEncode();/*模式变化 ,BYPASS*/
	void mqcByPassEncode(int d);/* BYPASS 模式切换*/
	void mqcRestartInitEncode();/* 模式变化 ,RESTART */
	void mqcErtermEncode();/*模式变化,ENTREM*/
	void mqcFlush();/* 输出所有数据 */
	void mqcSetBits();/* 填充mqc_C*/
	void mqcSetCurCtx(int ctx);/*设置当前上下文标记*/
	int mqcNumBytes();/*获取自初始化以来所读/写的数据长度 */
	double getWmSeDec(int nmSeDec,int compno,int level,int orient,int bpsno,int isDWT,int stepSize,int numComps);

	void tierOneSignPass(int w,int h,int bpsno,int orient,int *nmSeDec ,char type,int cbstyle);/*重要性编码过程 */
	void tierOneSignPassStep(int *fp, int *dp, int orient, int bpno, int one,int *nmsedec, char type, int vsc);
	int tierOneGetNmSeDecSig(int x,int bitpos);

	void tierOneRefPass(int w,int h,int bpsno,int *nmSeDec,char type,int cbstyle);/*细化编码过程 */
	void tierOneRefPassStep(int *fp, int *dp, int bpno, int one,int *nmsedec, char type, int vsc);
	int tierOneGetNmSeDecRef(int x,int bitpos);

	void tierOneCleanPass(int w,int h,int bpsno,int orient,int *nmSeDec,int cbstyle);/*清理编码过程 */
	void tierOneClnPassStep(int *fp, int *dp, int orient, int bpsno, int one,int *nmsedec, int partial, int vsc);

	void tierOneUpdateFlags(int *fp,int s);
	void mqcEncode(int d);/*编码一个位symbol*/
	void mqcCodeMPS();/*对most probales symbol进行编码*/
	void mqcCodeLPS();/*对most least probales symbol进行编码 */
	void mqcRenormalize();/* 将mqc_A重正化,使其一起在0x8000~0x10000*/
	void mqcByteOut();/* 输出一个字节,如必要则进行位填充,在0xff之后,下一个字节一定得小于0x90*/

	void mqcResetEncode();
	
};
#endif