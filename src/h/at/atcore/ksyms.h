//	Altirra - Atari 800/800XL emulator
//	Copyright (C) 2009-2022 Avery Lee
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License along
//	with this program. If not, see <http://www.gnu.org/licenses/>.
//
//	As a special exception, this library can also be redistributed and/or
//	modified under an alternate license. See COPYING.RMT in the same source
//	archive for details.

#ifndef f_AT_ATCORE_KSYMS_H
#define f_AT_ATCORE_KSYMS_H

namespace ATKernelSymbols {
	// page zero
	enum {
		CASINI = 0x0002,
		RAMLO  = 0x0004,
		TRAMSZ = 0x0006,
		WARMST = 0x0008,
		BOOT_  = 0x0009,	// actually BOOT?
		DOSVEC = 0x000A,
		DOSINI = 0x000C,
		APPMHI = 0x000E,
		POKMSK = 0x0010,
		BRKKEY = 0x0011,
		RTCLOK = 0x0012,
		BUFADR = 0x0015,
		ICHIDZ = 0x0020,
		ICDNOZ = 0x0021,
		ICCOMZ = 0x0022,
		ICSTAZ = 0x0023,
		ICBALZ = 0x0024,
		ICBAHZ = 0x0025,
		ICBLLZ = 0x0028,
		ICBLHZ = 0x0029,
		ICAX1Z = 0x002A,
		ICAX2Z = 0x002B,
		ICAX3Z = 0x002C,
		ICAX4Z = 0x002D,
		ICAX5Z = 0x002E,
		ICIDNO = 0x002E,
		CIOCHR = 0x002F,
		STATUS = 0x0030,
		CHKSUM = 0X0031,
		BUFRLO = 0X0032,
		BUFRHI = 0X0033,
		BFENLO = 0X0034,
		BFENHI = 0X0035,
		BUFRFL = 0X0038,
		RECVDN = 0X0039,
		XMTDON = 0x003A,
		CHKSNT = 0x003B,
		BPTR   = 0x003D,
		FTYPE  = 0x003E,
		FEOF   = 0x003F,
		SOUNDR = 0x0041,
		CRITIC = 0x0042,
		CKEY   = 0x004A,
		CASSBT = 0x004B,
		ATRACT = 0x004D,
		DRKMSK = 0x004E,
		COLRSH = 0x004F,
		HOLD1  = 0x0051,
		LMARGN = 0x0052,
		RMARGN = 0x0053,
		ROWCRS = 0x0054,
		COLCRS = 0x0055,
		DINDEX = 0x0057,
		SAVMSC = 0x0058,
		OLDROW = 0x005A,
		OLDCOL = 0x005B,
		OLDCHR = 0x005D,
		OLDADR = 0x005E,
		PALNTS = 0x0062,
		LOGCOL = 0x0063,
		ADRESS = 0x0064,
		TOADR  = 0x0066,
		RAMTOP = 0x006A,
		BUFCNT = 0x006B,
		BUFSTR = 0x006C,
		BITMSK = 0x006E,
		DELTAR = 0x0076,
		DELTAC = 0x0077,
		ROWINC = 0x0079,
		COLINC = 0x007A,
		KEYDEF = 0x0079,	// XL/XE
		SWPFLG = 0x007B,
		COUNTR = 0x007E,
		FR0	   = 0x00D4,
		FR1    = 0x00E0,
		CIX    = 0x00F2,
		INBUFF = 0x00F3,
		FLPTR  = 0x00FC
	};

	// page 2/3 symbols
	enum {
		VDSLST = 0x0200,
		VPRCED = 0x0202,
		VINTER = 0x0204,
		VBREAK = 0x0206,
		VKEYBD = 0x0208,
		VSERIN = 0x020A,
		VSEROR = 0x020C,
		VSEROC = 0x020E,
		VTIMR1 = 0x0210,
		VTIMR2 = 0x0212,
		VTIMR4 = 0x0214,
		VIMIRQ = 0x0216,
		CDTMV1 = 0x0218,
		CDTMV2 = 0x021A,
		CDTMV3 = 0x021C,
		CDTMV4 = 0x021E,
		CDTMV5 = 0x0220,
		VVBLKI = 0x0222,
		VVBLKD = 0x0224,
		CDTMA1 = 0x0226,
		CDTMA2 = 0x0228,
		CDTMF3 = 0x022A,
		CDTMF4 = 0x022C,
		CDTMF5 = 0x022E,
		SDMCTL = 0x022F,
		SDLSTL = 0x0230,
		SDLSTH = 0x0231,
		SSKCTL = 0x0232,
		LPENH  = 0x0234,
		LPENV  = 0x0235,
		BRKKY  = 0x0236,
		VPIRQ  = 0x0238,	// XL/XE
		COLDST = 0x0244,
		PDVMSK = 0x0247,	// XL/XE
		SHPDVS = 0x0248,	// XL/XE
		PDMSK  = 0x0249,	// XL/XE
		CHSALT = 0x026B,	// XL/XE
		GPRIOR = 0x026F,
		PADDL0 = 0x0270,
		PADDL1 = 0x0271,
		PADDL2 = 0x0272,
		PADDL3 = 0x0273,
		PADDL4 = 0x0274,
		PADDL5 = 0x0275,
		PADDL6 = 0x0276,
		PADDL7 = 0x0277,
		STICK0 = 0x0278,
		STICK1 = 0x0279,
		STICK2 = 0x027A,
		STICK3 = 0x027B,
		PTRIG0 = 0x027C,
		PTRIG1 = 0x027D,
		PTRIG2 = 0x027E,
		PTRIG3 = 0x027F,
		PTRIG4 = 0x0280,
		PTRIG5 = 0x0281,
		PTRIG6 = 0x0282,
		PTRIG7 = 0x0283,
		STRIG0 = 0x0284,
		STRIG1 = 0x0285,
		STRIG2 = 0x0286,
		STRIG3 = 0x0287,
		JVECK  = 0x028C,
		WMODE  = 0x0289,
		BLIM   = 0x028A,
		TXTROW = 0x0290,
		TXTCOL = 0x0291,
		TINDEX = 0x0293,
		TXTMSC = 0x0294,
		TXTOLD = 0x0296,
		CRETRY = 0x029C,
		HOLD2  = 0x029F,
		DMASK  = 0x02A0,
		ESCFLG = 0x02A2,
		TABMAP = 0x02A3,
		LOGMAP = 0x02B2,
		DRETRY = 0x02BD,
		SHFLOK = 0x02BE,
		BOTSCR = 0x02BF,
		PCOLR0 = 0x02C0,
		PCOLR1 = 0x02C1,
		PCOLR2 = 0x02C2,
		PCOLR3 = 0x02C3,
		COLOR0 = 0x02C4,
		COLOR1 = 0x02C5,
		COLOR2 = 0x02C6,
		COLOR3 = 0x02C7,
		COLOR4 = 0x02C8,
		DSCTLN = 0x02D5,
		KRPDEL = 0x02D9,	// XL/XE
		KEYREP = 0x02DA,	// XL/XE
		NOCLIK = 0x02DB,	// XL/XE
		HELPFG = 0x02DC,	// XL/XE
		DMASAV = 0x02DD,	// XL/XE
		RUNAD  = 0x02E0,
		INITAD = 0x02E2,
		MEMTOP = 0x02E5,
		MEMLO  = 0x02E7,
		DVSTAT = 0x02EA,
		CBAUDL = 0x02EE,
		CBAUDH = 0x02EF,
		CRSINH = 0x02F0,
		KEYDEL = 0x02F1,
		CH1    = 0x02F2,
		CHACT  = 0x02F3,
		CHBAS  = 0x02F4,
		ATACHR = 0x02FB,
		CH     = 0x02FC,
		FILDAT = 0x02FD,
		DSPFLG = 0x02FE,
		SSFLAG = 0x02FF,
		DDEVIC = 0x0300,
		DUNIT  = 0x0301,
		DCOMND = 0x0302,
		DSTATS = 0x0303,
		DBUFLO = 0X0304,
		DBUFHI = 0X0305,
		DTIMLO = 0X0306,
		DBYTLO = 0X0308,
		DBYTHI = 0X0309,
		DAUX1  = 0X030A,
		DAUX2  = 0X030B,
		TIMER1 = 0x030C,
		CASFLG = 0x030F,
		TIMER2 = 0x0310,
		TIMFLG = 0x0317,
		STACKP = 0x0318,
		HATABS = 0x031A,
		ICHID  = 0x0340,
		ICDNO  = 0x0341,
		ICCMD  = 0x0342,
		ICSTA  = 0x0343,
		ICBAL  = 0x0344,
		ICBAH  = 0x0345,
		ICPTL  = 0x0346,
		ICPTH  = 0x0347,
		ICBLL  = 0x0348,
		ICBLH  = 0x0349,
		ICAX1  = 0x034A,
		ICAX2  = 0x034B,
		ICAX3  = 0x034C,
		ICAX4  = 0x034D,
		ICAX5  = 0x034E,
		ICAX6  = 0x034F,
		BASICF = 0x03F8,	// XL/XE
		GINTLK = 0x03FA,	// XL/XE
		CASBUF = 0x03FD,
		LBUFF  = 0x0580
	};

	// hardware symbols
	enum : uint32 {
		COLPM0 = 0xD012,
		COLPM1 = 0xD013,
		COLPM2 = 0xD014,
		COLPM3 = 0xD015,
		COLPF0 = 0xD016,
		COLPF1 = 0xD017,
		COLPF2 = 0xD018,
		COLPF3 = 0xD019,
		COLBK  = 0xD01A,
		PRIOR  = 0xD01B,
		CONSOL = 0xD01F,
		AUDF1  = 0xD200,
		POT0   = 0xD200,
		AUDC1  = 0xD201,
		POT1   = 0xD201,
		AUDF2  = 0xD202,
		POT2   = 0xD202,
		AUDC2  = 0xD203,
		POT3   = 0xD203,
		AUDF3  = 0xD204,
		AUDC3  = 0xD205,
		AUDF4  = 0xD206,
		AUDC4  = 0xD207,
		AUDCTL = 0xD208,
		ALLPOT = 0xD208,
		STIMER = 0xD209,
		POTGO  = 0xD20B,
		SEROUT = 0xD20D,
		IRQST  = 0xD20E,
		IRQEN  = 0xD20E,
		SKCTL  = 0xD20F,
		PORTA  = 0xD300,
		PORTB  = 0xD301,
		PACTL  = 0xD302,
		PBCTL  = 0xD303,
		DMACTL = 0xD400,
		CHACTL = 0xD401,
		DLISTL = 0xD402,
		DLISTH = 0xD403,
		CHBASE = 0xD409,
		NMIEN  = 0xD40E,
		NMIRES = 0xD40F,
	};

	// floating-point library symbols
	enum {
		AFP    = 0xD800,
		FASC   = 0xD8E6,
		IPF    = 0xD9AA,
		FPI    = 0xD9D2,	// __ftol
		ZFR0   = 0xDA44,
		ZF1    = 0xDA46,
		ZFL    = 0xDA48,	// undocumented (used by Atari Basic) - zero Y bytes at (X)
		LDBUFA = 0xDA51,	// undocumented (used by Atari Basic) - mwa #ldbuf inbuff
		FADD   = 0xDA66,
		FSUB   = 0xDA60,
		FMUL   = 0xDADB,
		FDIV   = 0xDB28,
		SKPSPC = 0xDBA1,	// undocumented (used by Atari Basic) - skip spaces starting at INBUFF[CIX]
		ISDIGT = 0xDBAF,	// undocumented (used by Atari Basic) - set carry if INBUFF[CIX] is not a digit
		NORMALIZE = 0xDC00,	// undocumented (used by Atari Basic) - normalize mantissa/exponent in FR0
		PLYEVL = 0xDD40,
		FLD0R  = 0xDD89,
		FLD0P  = 0xDD8D,
		FLD1R  = 0xDD98,
		FLD1P  = 0xDD9C,
		FST0R  = 0xDDA7,
		FST0P  = 0xDDAB,
		FMOVE  = 0xDDB6,
		EXP    = 0xDDC0,
		EXP10  = 0xDDCC,
		REDRNG = 0xDE95,	// undocumented (used by Atari Basic) - reduce range via y = (x-1)/(x+1)
		LOG    = 0xDECD,
		LOG10  = 0xDED1
	};

	// kernel symbols
	enum {
		EDITRV = 0xE400,
		PRINTV = 0xE430,
		CASETV = 0xE440,
		DISKIV = 0xE450,
		DSKINV = 0xE453,
		CIOV   = 0xE456,
		SIOV   = 0xE459,
		SETVBV = 0xE45C,
		SYSVBV = 0xE45F,
		XITVBV = 0xE462,
		SIOINV = 0xE465,
		SENDEV = 0xE468,
		INTINV = 0xE46B,
		CIOINV = 0xE46E,
		BLKBDV = 0xE471,
		WARMSV = 0xE474,
		COLDSV = 0xE477,
		RBLOKV = 0xE47A,
		CSOPIV = 0xE47D,
		PUPDIV = 0xE480,
		SLFTSV = 0xE483,
		PENTV  = 0xE486,
		PHUNLV = 0xE489,
		PHINIV = 0xE48C,
		GPDVV  = 0xE48F
	};
}

namespace ATKernelSymbols5200 {
	// page zero
	enum {
		POKMSK = 0x0000,
		RTCLOK = 0x0001,
		CRITIC = 0x0003,
		ATRACT = 0x0004,
		SDLSTL = 0x0005,
		SDLSTH = 0x0006,
		SDMCTL = 0x0007,
		PCOLR0 = 0x0008,
		PCOLR1 = 0x0009,
		PCOLR2 = 0x000A,
		PCOLR3 = 0x000B,
		COLOR0 = 0x000C,
		COLOR1 = 0x000D,
		COLOR2 = 0x000E,
		COLOR3 = 0x000F,
		COLOR4 = 0x0010,
		PADDL0 = 0x0011,
		PADDL1 = 0x0012,
		PADDL2 = 0x0013,
		PADDL3 = 0x0014,
		PADDL4 = 0x0015,
		PADDL5 = 0x0016,
		PADDL6 = 0x0017,
		PADDL7 = 0x0018,
	};

	// page 2/3 symbols
	enum {
		VIMIRQ = 0x0200,
		VVBLKI = 0x0202,
		VVBLKD = 0x0204,
		VDSLST = 0x0206,
		VKYBDI = 0x0208,
		VKYBDF = 0x020A,
		VTRIGR = 0x020C,
		VBRKOP = 0x020E,
		VSERIN = 0x0210,
		VSEROR = 0x0212,
		VSEROC = 0x0214,
		VTIMR1 = 0x0216,
		VTIMR2 = 0x0218,
		VTIMR4 = 0x021A,
	};

	// hardware symbols
	enum {
		COLPM0 = 0xC012,
		COLPM1 = 0xC013,
		COLPM2 = 0xC014,
		COLPM3 = 0xC015,
		COLPF0 = 0xC016,
		COLPF1 = 0xC017,
		COLPF2 = 0xC018,
		COLPF3 = 0xC019,
		COLBK  = 0xC01A,
		PRIOR  = 0xC01B,
		CONSOL = 0xC01F,
		DMACTL = 0xD400,
		CHACTL = 0xD401,
		DLISTL = 0xD402,
		DLISTH = 0xD403,
		CHBASE = 0xD409,
		NMIEN  = 0xD40E,
		NMIRES = 0xD40F,
		AUDF1  = 0xE800,
		AUDC1  = 0xE801,
		AUDF2  = 0xE802,
		AUDC2  = 0xE803,
		AUDF3  = 0xE804,
		AUDC3  = 0xE805,
		AUDF4  = 0xE806,
		AUDC4  = 0xE807,
		AUDCTL = 0xE808,
		IRQST  = 0xE80E,
		IRQEN  = 0xE80E,
		SKCTL  = 0xE80F,
	};
}

#endif
