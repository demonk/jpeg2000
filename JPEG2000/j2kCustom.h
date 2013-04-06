#ifndef J2K_CUSTOM_DEFINE
#define J2K_CUSTOM_DEFINE

#define J2K_MS_SOC 0xff4f
#define J2K_MS_SOT 0xff90
#define J2K_MS_SOD 0xff93
#define J2K_MS_EOC 0xffd9
#define J2K_MS_SIZ 0xff51
#define J2K_MS_COD 0xff52
#define J2K_MS_COC 0xff53
#define J2K_MS_RGN 0xff5e
#define J2K_MS_QCD 0xff5c
#define J2K_MS_QCC 0xff5d
#define J2K_MS_POC 0xff5f
#define J2K_MS_TLM 0xff55
#define J2K_MS_PLM 0xff57
#define J2K_MS_PLT 0xff58
#define J2K_MS_PPM 0xff60
#define J2K_MS_PPT 0xff61
#define J2K_MS_SOP 0xff91
#define J2K_MS_EPH 0xff92
#define J2K_MS_CRG 0xff63
#define J2K_MS_COM 0xff64
#define J2K_STATE_MHSOC 0x0001
#define J2K_STATE_MHSIZ 0x0002
#define J2K_STATE_MH 0x0004
#define J2K_STATE_TPHSOT 0x0008
#define J2K_STATE_TPH 0x0010
#define J2K_STATE_MT 0x0020
#define J2K_STATE_NEOC 0x0040
#define J2K_STATE_MHEPB 0x0080
#define J2K_STATE_MHEPB 0x0080
#define J2K_STATE_TPHEPB 0x0100

#define J2K_MAXRLVLS 33		/* Number of maximum resolution level authorized                   */
#define J2K_MAXBANDS (3*J2K_MAXRLVLS-2)	/* Number of maximum sub-band linked to number of resolution level */

#define J2K_CP_CSTY_PRT 0x01 /* 0 => one precinct || 1 => custom precinct  */
#define J2K_CP_CSTY_SOP 0x02 /* This option offers the possibility to add a specific marker before each packet.
								It is the marker SOP (Start of packet). 
								If the option is not used no SOP marker will be added. */
#define J2K_CP_CSTY_EPH 0x04 /* This option offers the possibility to add a specific marker at the head of each packet header. 
								It is the marker EPH (End of packet Header). 
								If the option is not used no EPH marker will be added. */
#define J2K_CCP_CSTY_PRT 0x01

#define J2K_CCP_CBLKSTY_LAZY 0x01
#define J2K_CCP_CBLKSTY_RESET 0x02
#define J2K_CCP_CBLKSTY_TERMALL 0x04
#define J2K_CCP_CBLKSTY_VSC 0x08
#define J2K_CCP_CBLKSTY_PTERM 0x10
#define J2K_CCP_CBLKSTY_SEGSYM 0x20
#define J2K_CCP_QNTSTY_NOQNT 0
#define J2K_CCP_QNTSTY_SIQNT 1
#define J2K_CCP_QNTSTY_SEQNT 2

#define NULL 0
#include "charInputOutput.cpp"
#include "bitInputOutput.cpp"
#include "jp2Struct.h"
#include "jpegMath.h"

#endif