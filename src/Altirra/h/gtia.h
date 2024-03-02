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

#ifndef AT_GTIA_H
#define AT_GTIA_H

#include <vd2/system/refcount.h>
#include <vd2/system/vdstl.h>
#include <vd2/Kasumi/pixmap.h>

class IVDVideoDisplay;
class VDVideoDisplayFrame;

class IATGTIAEmulatorConnections {
public:
	virtual uint32 GTIAGetXClock() = 0;
	virtual void GTIASetSpeaker(bool state) = 0;
	virtual void GTIARequestAnticSync() = 0;
};

class ATFrameTracker;
class ATArtifactingEngine;
class ATSaveStateReader;
class ATSaveStateWriter;
class ATGTIARenderer;

class ATGTIAEmulator {
public:
	ATGTIAEmulator();
	~ATGTIAEmulator();

	void Init(IATGTIAEmulatorConnections *);
	
	enum AnalysisMode {
		kAnalyzeNone,
		kAnalyzeColors,
		kAnalyzeLayers,
		kAnalyzeDList,
		kAnalyzePM,
		kAnalyzeCount
	};

	enum ArtifactMode {
		kArtifactNone,
		kArtifactNTSC,
		kArtifactPAL,
		kArtifactCount
	};

	void AdjustColors(double baseDelta, double rangeDelta);

	AnalysisMode GetAnalysisMode() const { return mAnalysisMode; }
	void SetAnalysisMode(AnalysisMode mode);

	void SetForcedBorder(bool forcedBorder) { mbForcedBorder = forcedBorder; }
	void SetFrameSkip(bool turbo) { mbTurbo = turbo; }

	bool ArePMCollisionsEnabled() const;
	void SetPMCollisionsEnabled(bool enable);

	bool ArePFCollisionsEnabled() const;
	void SetPFCollisionsEnabled(bool enable);

	void SetVideoOutput(IVDVideoDisplay *pDisplay);
	void SetStatusFlags(uint32 flags) { mStatusFlags |= flags; mStickyStatusFlags |= flags; }
	void ResetStatusFlags(uint32 flags) { mStatusFlags &= ~flags; }
	void PulseStatusFlags(uint32 flags) { mStickyStatusFlags |= flags; }

	void SetCassetteIndicatorVisible(bool vis) { mbShowCassetteIndicator = vis; }
	void SetCassettePosition(float pos);

	void SetPALMode(bool enabled);

	ArtifactMode GetArtifactingMode() const { return mArtifactMode; }
	void SetArtifactingMode(ArtifactMode mode) { mArtifactMode = mode; }

	void SetConsoleSwitch(uint8 c, bool down);
	void SetForcedConsoleSwitches(uint8 c);
	void SetControllerTrigger(int index, bool state) { mTRIG[index] = state ? 0x00 : 0x01; }

	const VDPixmap *GetLastFrameBuffer() const;

	uint32 GetBackgroundColor24() const { return mPalette[mPFBAK]; }
	uint32 GetPlayfieldColor24(int index) const { return mPalette[mPFColor[index]]; }
	uint32 GetPlayfieldColorPF2H() const { return mPalette[(mPFColor[2] & 0xf0) + (mPFColor[1] & 0x0f)]; }

	void DumpStatus();

	void LoadState(ATSaveStateReader& reader);
	void SaveState(ATSaveStateWriter& writer);

	bool BeginFrame(bool force);
	void BeginScanline(int y, bool hires);
	void EndScanline(uint8 dlControl);
	void UpdatePlayer(bool odd, int index, uint8 byte);
	void UpdateMissile(bool odd, uint8 byte);
	void UpdatePlayfield160(uint32 x, uint8 byte);
	void UpdatePlayfield320(uint32 x, uint8 byte);
	void EndPlayfield();
	void Sync();

	void RenderActivityMap(const uint8 *src);
	void UpdateScreen();
	void RecomputePalette();

	uint8 DebugReadByte(uint8 reg) const;
	uint8 ReadByte(uint8 reg);
	void WriteByte(uint8 reg, uint8 value);

protected:
	struct RegisterChange {
		uint8 mPos;
		uint8 mReg;
		uint8 mValue;
		uint8 mPad;
	};

	template<class T> void ExchangeState(T& io);
	void SyncTo(int x1, int targetX);
	void ApplyArtifacting();
	void AddRegisterChange(uint8 pos, uint8 addr, uint8 value);
	void UpdateRegisters(const RegisterChange *rc, int count);

	IATGTIAEmulatorConnections *mpConn; 
	IVDVideoDisplay *mpDisplay;
	uint32	mX;
	uint32	mY;
	uint32	mLastSyncX;

	AnalysisMode	mAnalysisMode;
	ArtifactMode	mArtifactMode;
	uint8	mArtifactModeThisFrame;

	uint8	mPlayerPos[4];
	uint8	mMissilePos[4];
	uint8	mPlayerWidth[4];
	uint8	mMissileWidth[4];

	uint32	mPlayerTriggerPos[4];
	uint32	mMissileTriggerPos[4];
	uint8	mPlayerShiftData[4];
	uint8	mMissileShiftData[4];

	uint8	mPlayerSize[4];
	uint8	mPlayerData[4];
	uint8	mMissileData;
	uint8	mMissileSize;

	// The following 9 registers must be contiguous.
	uint8	mPMColor[4];		// $D012-D015 player and missile colors
	uint8	mPFColor[4];		// $D016-D019 playfield colors
	uint8	mPFBAK;				// $D01A background color

	uint8	mPRIOR;				// $D01B priority
	uint8	mVDELAY;			// $D01C vertical delay
	uint8	mGRACTL;			// $D01D
								// bit 2: latch trigger inputs
								// bit 1: enable players
								// bit 0: enable missiles
	uint8	mSwitchOutput;		// $D01F (CONSOL) output from GTIA
								// bit 3: speaker output
	uint8	mSwitchInput;		// $D01F (CONSOL) input to GTIA
	uint8	mForcedSwitchInput;

	uint8	mTRIG[4];

	uint8	mPlayerCollFlags[4];
	uint8	mMissileCollFlags[4];
	uint8	mCollisionMask;

	uint8	*mpDst;
	vdrefptr<VDVideoDisplayFrame>	mpFrame;
	ATFrameTracker *mpFrameTracker;
	bool	mbANTICHiresMode;
	bool	mbHiresMode;
	bool	mbTurbo;
	bool	mbPALMode;
	bool	mbShowCassetteIndicator;
	bool	mbForcedBorder;

	uint32	mStatusFlags;
	uint32	mStickyStatusFlags;
	float	mCassettePos;

	const uint8 *mpPriTable;
	const uint8 *mpColorTable;

	uint8	mMergeBuffer[228];
	uint8	mAnticData[228];
	uint32	mPalette[256];
	bool	mbScanlinesWithHiRes[240];

	double	mPaletteColorBase;
	double	mPaletteColorRange;

	vdfastvector<uint8>		mPreArtifactFrameBuffer;
	VDPixmap	mPreArtifactFrame;
	VDPixmap	mPreArtifactFrameVisible;

	ATArtifactingEngine	*mpArtifactingEngine;
	vdrefptr<VDVideoDisplayFrame> mpLastFrame;

	ATGTIARenderer *mpRenderer;

	typedef vdfastvector<RegisterChange> RegisterChanges;
	RegisterChanges mRegisterChanges;
	int mRCIndex;
	int mRCCount;
};

#endif