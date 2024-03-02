//	Altirra - Atari 800/800XL emulator
//	Copyright (C) 2008 Avery Lee
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
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef AT_ANTIC_H
#define AT_ANTIC_H

#ifdef _MSC_VER
	#pragma once
#endif

#include <vd2/system/vdtypes.h>
#include <at/atcore/scheduler.h>

class ATGTIAEmulator;
class ATSaveStateReader;
class ATSaveStateWriter;
class ATScheduler;
class ATSimulatorEventManager;

class ATAnticEmulatorConnections {
public:
	VDFORCEINLINE uint8 AnticReadByteFast(uint32 address) {
		uintptr readPage = mpAnticReadPageMap[address >> 8];
		return *mpAnticBusData = (!(readPage & 1) ? *(const uint8 *)(readPage + address) : AnticReadByte(address));
	}

	virtual uint8 AnticReadByte(uint32 address) = 0;
	virtual void AnticAssertNMI_DLI() = 0;
	virtual void AnticAssertNMI_VBI() = 0;
	virtual void AnticAssertNMI_RES() = 0;
	virtual void AnticEndFrame() = 0;
	virtual void AnticEndScanline() = 0;
	virtual bool AnticIsNextCPUCycleWrite() = 0;
	virtual uint8 AnticGetCPUHeldCycleValue() = 0;
	virtual void AnticForceNextCPUCycleSlow() = 0;
	virtual void AnticOnVBlank() =0;

	uint8 *mpAnticBusData;

protected:
	const uintptr *mpAnticReadPageMap;
};

struct ATAnticRegisterState {
	uint8	mDMACTL;
	uint8	mCHACTL;
	uint8	mDLISTL;
	uint8	mDLISTH;
	uint8	mHSCROL;
	uint8	mVSCROL;
	uint8	mPMBASE;
	uint8	mCHBASE;
	uint8	mNMIEN;
};

class ATAnticEmulator final : public IATSchedulerCallback {
public:
	ATAnticEmulator();
	~ATAnticEmulator();

	enum AnalysisMode {
		kAnalyzeOff,
		kAnalyzeDMATiming,
		kAnalyzeModeCount
	};

	uint16	GetDisplayListPointer() const { return mDLIST; }
	uint32	GetTimestamp() const { return (mFrame << 20) + (mY << 8) + mX; }
	uint32	GetFrameStartTime() const { return mFrameStart; }
	uint32	GetFrameCounter() const { return mFrame; }
	uint32	GetBeamX() const { return mX; }
	uint32	GetBeamY() const { return mY; }
	uint32	GetHaltedCycleCount() const { return mHaltedCycles; }

	AnalysisMode	GetAnalysisMode() const { return mAnalysisMode; }
	void			SetAnalysisMode(AnalysisMode mode) { mAnalysisMode = mode; }

	bool IsPlayfieldDMAEnabled() const {
		return (mDMACTL & 0x03) != 0;
	}

	bool IsVBIEnabled() const {
		return (mNMIEN & 0x40) != 0;
	}

	void	SetPALMode(bool pal);

	struct DLHistoryEntry {
		uint16	mDLAddress;
		uint16	mPFAddress;
		uint8	mHVScroll;
		uint8	mDMACTL;
		uint8	mControl;
		uint8	mCHBASE : 7;
		uint8	mbValid : 1;
	};

	const DLHistoryEntry *GetDLHistory() const { return mDLHistory; }
	const uint8 *GetActivityMap() const { return mActivityMap[0]; }

	void Init(ATAnticEmulatorConnections *conn, ATGTIAEmulator *gtia, ATScheduler *sch, ATSimulatorEventManager *simevmgr);
	void ColdReset();
	void WarmReset();
	void RequestNMI();

	void SetLightPenPosition(bool phase);
	void SetLightPenPosition(int x, int y);

	VDFORCEINLINE uint8 GetWSYNCFlag() const { return (uint8)mbWSYNCActive; }
	VDFORCEINLINE uint8 PreAdvance();
	VDFORCEINLINE void PostAdvance(uint8 mode);
	void SyncWithGTIA(int offset);
	void Decode(int offset);

	uint8 ReadByte(uint8 reg) const;
	void WriteByte(uint8 reg, uint8 value);

	void DumpStatus();
	void DumpDMALineBuffer();
	void DumpDMAPattern();
	void DumpDMAActivityMap();

	void	BeginLoadState(ATSaveStateReader& reader);
	void	LoadStateArch(ATSaveStateReader& reader);
	void	LoadStatePrivate(ATSaveStateReader& reader);
	void	EndLoadState(ATSaveStateReader& reader);

	void	BeginSaveState(ATSaveStateWriter& writer);
	void	SaveStateArch(ATSaveStateWriter& writer);
	void	SaveStatePrivate(ATSaveStateWriter& writer);

	void	GetRegisterState(ATAnticRegisterState& state) const;

protected:
	template<class T>
	void	ExchangeState(T& io);

	uint8	AdvanceSpecial();
	void	AdvanceScanline();
	void	UpdateDMAPattern();
	void	LatchPlayfieldEdges();
	void	UpdateCurrentCharRow();
	void	UpdatePlayfieldTiming();
	void	UpdatePlayfieldDataPointers();

	void	OnScheduledEvent(uint32 id);
	void	QueueRegisterUpdate(uint32 delay, uint8 reg, uint8 value);
	void	ExecuteQueuedUpdates();

	// critical fields written in Advance()
	uint32	mHaltedCycles;
	uint32	mX;
	uint16	mPFRowDMAPtrOffset;

	// critical fields read in Advance()
	ATAnticEmulatorConnections *mpConn;
	uint8	*mpPFDataWrite;
	uint8	*mpPFDataRead;
	uint32	mPFCharFetchPtr;
	uint8	mPFCharMask;
	uint8	mbWSYNCActive;
	uint16	mPFRowDMAPtrBase;

	uint32	mY;
	uint32	mFrame;
	uint32	mFrameStart;
	uint32	mScanlineLimit;
	uint32	mScanlineMax;
	uint32	mVSyncStart;

	bool	mbDLExtraLoadsPending;
	bool	mbDLActive;
	bool	mbDLDMAEnabledInTime;
	int		mPFDisplayCounter;
	int		mPFDecodeCounter;
	int		mPFDecodeOffset;
	int		mPFDecodeCharOffset;
	int		mPFDMALastCheckX;
	uint8	mPFDecodeAbCharInv;		// character inversion byte (abnormal DMA only)
	bool	mbPFDMAEnabled;
	bool	mbPFDMAActive;
	bool	mbPFRendered;			// true if any pixels have been rendered this scanline by the playfield
	bool	mbWSYNCRelease;
	uint8	mWSYNCHoldValue;
	bool	mbHScrollEnabled;
	bool	mbHScrollDelay;
	bool	mbRowStopUseVScroll;
	bool	mbRowAdvance;
	bool	mbLateNMI;
	bool	mbInBuggedVBlank;
	bool	mbPhantomPMDMA;
	bool	mbPhantomPMDMAActive;
	bool	mbPhantomPlayerDMA;
	bool	mbMissileDMADisabledLate;
	uint8	mPendingNMIs;
	uint8	mEarlyNMIEN;
	uint8	mEarlyNMIEN2;
	uint32	mRowCounter;
	uint32	mRowCount;
	uint8	mLatchedVScroll;		// latched VSCROL at cycle 109 from previous scanline -- used to detect end of vs region
	uint8	mLatchedVScroll2;		// latched VSCROL at cycle 6 from current scanline -- used to control DLI

	uint32	mPFPushCycleMask;

	/// Bitmask indicating which bits are still flying around the DMA clock; bit N is
	/// corresponds to a character name fetch happening on cycle N of the scanline.
	/// The pattern repeats every 8 cycles.
	uint8	mAbnormalDMAPattern;
	uint8	mEndingDMAPattern;
	uint8	mAbnormalDecodePattern;
	uint8	mAbnormalDecodeShifter;

	int		mPFWidthShift;
	int		mPFHScrollDMAOffset;
	enum PFPushMode {
		kBlank,
		k160,
		k160Alt,
		k320
	} mPFPushMode;

	bool	mPFHiresMode;

	enum PFWidthMode {
		kPFDisabled,
		kPFNarrow,
		kPFNormal,
		kPFWide
	};
	
	PFWidthMode	mPFWidth;
	PFWidthMode	mPFFetchWidth;

	uint32	mPFDisplayStart;
	uint32	mPFDisplayEnd;
	uint32	mPFDMAStart;
	uint8	*mpPFCharFetchPtr;
	uint32	mPFDMAVEnd;
	uint32	mPFDMAVEndWide;
	uint32	mPFDMAEnd;
	uint32	mPFDMALatchedStart;
	uint32	mPFDMALatchedVEnd;
	uint32	mPFDMALatchedEnd;
	uint32	mPFDMAPatternCacheKey;

	AnalysisMode	mAnalysisMode;

	uint8	mDMACTL;	// bit 5 = enable display list DMA
						// bit 4 = single line player resolution
						// bit 3 = enable player DMA
						// bit 2 = enable missile DMA
						// bit 1,0 = playfield width (00 = disabled, 01 = narrow, 10 = normal, 11 = wide)

	uint8	mCHACTL;	//

	uint16	mDLIST;		// display list pointer
	uint16	mDLISTLatch;// latched display list pointer
	uint8	mDLControlPrev;
	uint8	mDLControl;
	uint8	mDLNext;

	uint8	mHSCROL;	// horizontal scroll enable

	uint8	mVSCROL;	// vertical scroll enable

	uint8	mPMBASE;	// player missile base MSB

	uint8	mCHBASE;	// character base address
	uint32	mCharBaseAddr128;
	uint32	mCharBaseAddr64;
	uint8	mCharInvert;
	uint8	mCharBlink;

	uint8	mNMIEN;		// bit 7 = DLI enabled
						// bit 6 = VBI enabled

	uint8	mNMIST;		// bit 7 = DLI pending
						// bit 6 = VBI pending
						// bit 5 = RESET key pending

	uint8	mPENH;
	uint8	mPENV;

	uint8	mVCOUNT;
	int mWSYNCPending;

	uint32	mGTIAHSyncOffset;
	uint32	mVSyncShiftTime;

	ATGTIAEmulator *mpGTIA;
	ATScheduler *mpScheduler;
	ATSimulatorEventManager *mpSimEventMgr;

	struct QueuedRegisterUpdate {
		uint32 mTime;
		uint8 mReg;
		uint8 mValue;
	};

	typedef vdfastvector<QueuedRegisterUpdate> RegisterUpdates;
	RegisterUpdates mRegisterUpdates;
	uint32	mRegisterUpdateHeadIdx;

	ATEvent *mpRegisterUpdateEvent;
	ATEvent *mpEventWSYNC;

	// This is a VERY CRITICAL array as it is read for each and every cycle. The contents are
	// as follows:
	//
	//	bit 7	Special cycle processing (note that this may change the flags byte!).
	//	bit 6	Playfield stop latch check bit (to alter or end abnormal DMA)
	//	bit 5	Suppressed fetch (cycles 0-7)
	//	bit 4	Virtual fetch (playfield DMA >= cycle 105)
	//	bit 3	Abnormal DMA fetch
	//	bit 2	Character data fetch
	//	bit 1	Bitmap graphic or character name fetch
	//	bit 0	DMA cycle occurring (does not include special DMA)
	//
	VDALIGN(128) uint8	mDMAPattern[115 + 13];

	VDALIGN(256) uint8	mPFDataBuffer[114 + 14];	// MUST be aligned to 256 as we are checking the pointer low byte!
	VDALIGN(128) uint8	mPFCharBuffer[114 + 14];

	VDALIGN(16) uint8	mPFDecodeBuffer[228 + 12];

	DLHistoryEntry	mDLHistory[312];
	uint8	mActivityMap[312][114];
};

VDFORCEINLINE uint8 ATAnticEmulator::PreAdvance() {
	uint8 fetchMode = mDMAPattern[++mX];

	if (fetchMode & 0xc0)
		fetchMode = AdvanceSpecial();

	return fetchMode;
}

VDFORCEINLINE void ATAnticEmulator::PostAdvance(uint8 fetchMode) {
	if (fetchMode) {
		switch(fetchMode) {
			case 1:		// refresh cycle
				break;

			case 3:		// bitmap graphic or character name fetch
				*mpPFDataWrite++ = mpConn->AnticReadByteFast(mPFRowDMAPtrBase + ((mPFRowDMAPtrOffset++) & 0x0fff));
				break;

			case 5:		// character data fetch
				{
					uint8 c = *mpPFDataRead++;
					*mpPFCharFetchPtr++ = mpConn->AnticReadByteFast(mPFCharFetchPtr + ((uint32)(c & mPFCharMask) << 3));
				}
				break;

			case 8+3:		// bitmap graphic or character name fetch (abnormal DMA)
				*mpPFDataWrite++ = mpConn->AnticReadByteFast(mPFRowDMAPtrBase + ((mPFRowDMAPtrOffset++) & 0x0fff));
				if ((uint8)(uintptr)mpPFDataWrite == 63)
					mpPFDataWrite -= 63;
				break;

			case 8+5:		// character data fetch (abnormal DMA)
				{
					uint8 c = *mpPFDataRead | (((uintptr)mpPFDataRead & 48) == 48 ? 0xFF : 0x00);
					++mpPFDataRead;
					if ((uint8)(uintptr)mpPFDataRead == 63)
						mpPFDataRead -= 63;
					*mpPFCharFetchPtr++ = mpConn->AnticReadByteFast(mPFCharFetchPtr + ((uint32)(c & mPFCharMask) << 3));
				}
				break;

			case 8+7:		// character name + data fetch (abnormal DMA)
				{
					uint8 c = *mpPFDataRead | (((uintptr)mpPFDataRead & 48) == 48 ? 0xFF : 0x00);
					++mpPFDataRead;
					if ((uint8)(uintptr)mpPFDataRead == 63)
						mpPFDataRead -= 63;
					uint32 addr1 = mPFCharFetchPtr + ((uint32)(c & mPFCharMask) << 3);
					uint32 addr2 = mPFRowDMAPtrBase + ((mPFRowDMAPtrOffset++) & 0x0fff);

					*mpPFDataWrite++ = (*mpPFCharFetchPtr++ = mpConn->AnticReadByteFast(addr1 & addr2));
					if ((uint8)(uintptr)mpPFDataWrite == 63)
						mpPFDataWrite -= 63;
				}
				break;

			case 18:	// virtual bitmap graphic or character name fetch
			case 19:	// virtual bitmap graphic or character name fetch with refresh DMA
				*mpPFDataWrite++ = *mpConn->mpAnticBusData;
				++mPFRowDMAPtrOffset;
				break;

			case 20:	// virtual character data fetch
			case 21:	// virtual character data fetch with refresh DMA
				++mpPFDataRead;
				*mpPFCharFetchPtr++ = *mpConn->mpAnticBusData;
				break;

			case 26:	// virtual bitmap graphic or character name fetch
			case 27:	// virtual bitmap graphic or character name fetch with refresh DMA
				*mpPFDataWrite++ = *mpConn->mpAnticBusData;
				if ((uint8)(uintptr)mpPFDataWrite == 63)
					mpPFDataWrite -= 63;
				++mPFRowDMAPtrOffset;
				break;

			case 28:	// virtual character data fetch
			case 29:	// virtual character data fetch with refresh DMA
				++mpPFDataRead;
				if ((uint8)(uintptr)mpPFDataRead == 63)
					mpPFDataRead -= 63;
				*mpPFCharFetchPtr++ = *mpConn->mpAnticBusData;
				break;

			case 30:	// virtual character data + character name fetch (abnormal DMA)
			case 31:	// virtual character data + character name fetch with refresh DMA (abnormal DMA)
				*mpPFDataWrite++ = (*mpPFCharFetchPtr++ = *mpConn->mpAnticBusData);
				if ((uint8)(uintptr)mpPFDataWrite == 63)
					mpPFDataWrite -= 63;
				++mPFRowDMAPtrOffset;
				++mpPFDataRead;
				if ((uint8)(uintptr)mpPFDataRead == 63)
					mpPFDataRead -= 63;
				break;

			case 32+8+2:		// suppressed bitmap graphic or character name fetch, display DMA disabled (abnormal DMA)
				++mPFRowDMAPtrOffset;
				break;

			case 32+8+2+1:		// suppressed bitmap graphic or character name fetch (abnormal DMA)
				mpConn->AnticReadByteFast(mPFRowDMAPtrBase + (mPFRowDMAPtrOffset & 0x0fff));
				++mPFRowDMAPtrOffset;
				break;

			case 32+8+4:		// suppressed character data fetch, display DMA disabled (abnormal DMA)
				break;

			case 32+8+4+1:		// suppressed character data fetch (abnormal DMA)
				{
					uint8 c = mPFDataBuffer[0];
					mpConn->AnticReadByteFast(mPFCharFetchPtr + ((uint32)(c & mPFCharMask) << 3));
				}
				break;

			case 32+8+4+2:	// suppressed character name + data fetch, display DMA disabled (abnormal DMA)
				++mPFRowDMAPtrOffset;
				break;

			case 32+8+4+2+1:	// suppressed character name + data fetch (abnormal DMA)
				{
					uint8 c = mPFDataBuffer[0];
					uint32 addr1 = mPFCharFetchPtr + ((uint32)(c & mPFCharMask) << 3);
					uint32 addr2 = mPFRowDMAPtrBase + (mPFRowDMAPtrOffset & 0x0fff);
					++mPFRowDMAPtrOffset;

					mpConn->AnticReadByteFast(addr1 & addr2);
				}
				break;

			case 32+16+2:		// virtual bitmap graphic or character name fetch
			case 32+16+2+1:		// virtual bitmap graphic or character name fetch + refresh
				++mPFRowDMAPtrOffset;
				++mpPFDataWrite;
				break;

			case 32+16+4:		// virtual character data fetch
			case 32+16+4+1:		// virtual character data fetch+refresh
				++mpPFDataRead;
				++mpPFCharFetchPtr;
				break;

			case 32+16+6:		// virtual character data + character name fetch
			case 32+16+6+1:		// virtual character data + character name fetch+refresh
				++mPFRowDMAPtrOffset;
				++mpPFDataRead;
				++mpPFCharFetchPtr;
				++mpPFDataWrite;
				break;

			case 32+16+8+2:		// virtual bitmap graphic or character name fetch (abnormal DMA)
			case 32+16+8+2+1:	// virtual bitmap graphic or character name fetch+refresh (abnormal DMA)
				++mPFRowDMAPtrOffset;
				++mpPFDataWrite;
				if ((uint8)(uintptr)mpPFDataWrite == 63)
					mpPFDataWrite -= 63;
				break;

			case 32+16+8+4:		// virtual character data fetch (abnormal DMA)
			case 32+16+8+4+1:	// virtual character data fetch+refresh (abnormal DMA)
				++mpPFDataRead;
				if ((uint8)(uintptr)mpPFDataRead == 63)
					mpPFDataRead -= 63;
				++mpPFCharFetchPtr;
				break;

			case 32+16+8+6:		// virtual character data + character name fetch (abnormal DMA)
			case 32+16+8+6+1:	// virtual character data + character name fetch + refresh (abnormal DMA)
				++mPFRowDMAPtrOffset;
				++mpPFCharFetchPtr;
				++mpPFDataRead;
				if ((uint8)(uintptr)mpPFDataRead == 63)
					mpPFDataRead -= 63;
				++mpPFDataWrite;
				if ((uint8)(uintptr)mpPFDataWrite == 63)
					mpPFDataWrite -= 63;
				break;

			default:
				VDNEVERHERE;
		}
	}

	uint8 busActive = fetchMode & 1;
	busActive |= (uint8)mbWSYNCActive;

	mHaltedCycles += busActive;
}

#endif
