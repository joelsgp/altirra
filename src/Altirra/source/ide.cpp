//	Altirra - Atari 800/800XL/5200 emulator
//	Copyright (C) 2009-2011 Avery Lee
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

#include "stdafx.h"
#include <vd2/system/binary.h>
#include <vd2/system/error.h>
#include <vd2/system/hash.h>
#include <vd2/system/math.h>
#include "ide.h"
#include "idephysdisk.h"
#include "console.h"
#include "scheduler.h"
#include "uirender.h"
#include "simulator.h"
#include "debuggerlog.h"

ATDebuggerLogChannel g_ATLCIDE(false, false, "IDE", "IDE activity");
ATDebuggerLogChannel g_ATLCIDEError(false, false, "IDEERROR", "IDE interface errors");

extern ATSimulator g_sim;

namespace {
	enum {
		kATIDEStatus_BSY	= 0x80,		// busy
		kATIDEStatus_DRDY	= 0x40,		// drive ready
		kATIDEStatus_DWF	= 0x20,		// drive write fault
		kATIDEStatus_DSC	= 0x10,		// drive seek complete
		kATIDEStatus_DRQ	= 0x08,		// data request
		kATIDEStatus_CORR	= 0x04,		// corrected data
		kATIDEStatus_IDX	= 0x02,		// index
		kATIDEStatus_ERR	= 0x01		// error
	};

	enum {
		kATIDEError_BBK		= 0x80,		// bad block detected
		kATIDEError_UNC		= 0x40,		// uncorrectable data error
		kATIDEError_MC		= 0x20,		// media changed
		kATIDEError_IDNF	= 0x10,		// ID not found
		kATIDEError_MCR		= 0x08,		// media change request
		kATIDEError_ABRT	= 0x04,		// aborted command
		kATIDEError_TK0NF	= 0x02,		// track 0 not found
		kATIDEError_AMNF	= 0x01		// address mark not found
	};

	// These control the amount of time that BSY is asserted during a sector
	// read or write.
	const uint32 kIODelayFast = 100;	// 
	const uint32 kIODelaySlow = 10000;	// ~5.5ms
}

struct ATIDEEmulator::DecodedCHS {
	const char *c_str() const { return buf; }

	char buf[64];
};

ATIDEEmulator::ATIDEEmulator()
	: mpPhysDisk(NULL)
{
	mbReset = false;
	mTransferLength = 0;
	mTransferIndex = 0;
	mSectorCount = 0;
	mMaxSectorTransferCount = 32;
	mSectorsPerTrack = 0;
	mHeadCount = 0;
	mCylinderCount = 0;
	mIODelaySetting = 0;

	mTransferBuffer.resize(512 * mMaxSectorTransferCount);

	ColdReset();
}

ATIDEEmulator::~ATIDEEmulator() {
	CloseImage();
}

void ATIDEEmulator::Init(ATScheduler *scheduler, IATUIRenderer *uirenderer) {
	mpScheduler = scheduler;
	mpUIRenderer = uirenderer;
}

void ATIDEEmulator::OpenImage(bool write, bool fast, uint32 cylinders, uint32 heads, uint32 sectors, const wchar_t *filename) {
	CloseImage();

	// validate geometry
	if (!cylinders || !heads || !sectors || cylinders > 16777216 || heads > 16 || sectors > 255)
		throw MyError("Invalid IDE geometry: %u cylinders / %u heads / %u sectors", cylinders, heads, sectors);

	try {
		mbWriteEnabled = write;

		if (ATIDEIsPhysicalDiskPath(filename)) {
			mpPhysDisk = new ATIDEPhysicalDisk;
			mpPhysDisk->Init(filename);

			mSectorCount = mpPhysDisk->GetSectorCount();
			mHeadCount = heads;
			mSectorsPerTrack = sectors;
			mCylinderCount = mSectorCount / (mHeadCount * mSectorsPerTrack);
			mbWriteEnabled = false;
		} else {
			mFile.open(filename, write ? nsVDFile::kReadWrite | nsVDFile::kDenyAll | nsVDFile::kOpenAlways : nsVDFile::kRead | nsVDFile::kDenyWrite | nsVDFile::kOpenExisting);

			mSectorCount = cylinders * heads * sectors;

			if (sectors > 63)
				sectors = 63;

			mCylinderCount = cylinders;
			mHeadCount = heads;
			mSectorsPerTrack = sectors;
		}

		ResetCHSTranslation();

		mPath = filename;
		mIODelaySetting = fast ? kIODelayFast : kIODelaySlow;
		mbFastDevice = fast;

		ColdReset();
	} catch(const MyError&) {
		CloseImage();
		throw;
	}
}

void ATIDEEmulator::CloseImage() {
	if (mpPhysDisk) {
		delete mpPhysDisk;
		mpPhysDisk = NULL;
	}

	mFile.close();
	mSectorCount = 0;
	mCylinderCount = 0;
	mHeadCount = 0;
	mSectorsPerTrack = 0;

	ResetCHSTranslation();

	mPath.clear();

	ColdReset();
}

const wchar_t *ATIDEEmulator::GetImagePath() const {
	return mPath.c_str();
}

void ATIDEEmulator::ColdReset() {
	mbReset = false;
	ResetDevice();

	if (mpPhysDisk)
		mpPhysDisk->RequestUpdate();
}

void ATIDEEmulator::DumpStatus() const {
	ATConsoleWrite("IDE status:\n");
	ATConsolePrintf("Path:            %ls\n", mPath.c_str());
	ATConsolePrintf("Raw size:        %u sectors (%.1f MB)\n", mSectorCount, (float)mSectorCount / 2048.0f);
	ATConsolePrintf("Native geometry: %u cylinders, %u heads, %u sectors/track\n", mCylinderCount, mHeadCount, mSectorsPerTrack);
	ATConsolePrintf("CHS translation: %u cylinders, %u heads, %u sectors/track\n", mCurrentCylinderCount, mCurrentHeadCount, mCurrentSectorsPerTrack);
	ATConsolePrintf("Active command:  $%02x\n", mActiveCommand);
	ATConsolePrintf("Transfer mode:   %d-bit\n", mbTransfer16Bit ? 16 : 8);
	ATConsolePrintf("Reset line:      %s\n", mbReset ? "asserted" : "negated");
}

void ATIDEEmulator::SetReset(bool asserted) {
	if (mbReset == asserted)
		return;

	mbReset = asserted;

	if (asserted)
		ResetDevice();
}

uint32 ATIDEEmulator::ReadDataLatch(bool advance) {
	if (mbReset)
		return 0xFFFF;

	uint32 v = mTransferBuffer[mTransferIndex];

	if (mbTransfer16Bit)
		v += (uint32)mTransferBuffer[mTransferIndex + 1] << 8;
	else
		v += 0xFF00;

	if (!mbTransferAsWrites && mTransferIndex < mTransferLength) {
		++mTransferIndex;

		if (mbTransfer16Bit)
			++mTransferIndex;

		if (mTransferIndex >= mTransferLength)
			CompleteCommand();
	}

	return v;
}

void ATIDEEmulator::WriteDataLatch(uint8 lo, uint8 hi) {
	if (mbReset)
		return;

	if (mbTransferAsWrites && mTransferIndex < mTransferLength) {
		mTransferBuffer[mTransferIndex] = lo;

		++mTransferIndex;

		if (mbTransfer16Bit) {
			mTransferBuffer[mTransferIndex] = hi;
			++mTransferIndex;
		}

		if (mTransferIndex >= mTransferLength) {
			mRFile.mStatus &= ~kATIDEStatus_DRQ;
			mRFile.mStatus |= kATIDEStatus_BSY;

			UpdateStatus();
		}
	}
}

uint8 ATIDEEmulator::DebugReadByte(uint8 address) {
	if (mbReset)
		return 0xD0;

	if (address >= 8)
		return 0xFF;

	uint32 idx = address & 7;

	UpdateStatus();

	// ATA/ATAPI-4 9.16.0 -- when device 1 is selected with only device 0 present,
	// status and alternate status return 00h, while all other reads are the same as
	// device 0.
	if (mRFile.mHead & 0x10) {
		if (idx == 7)
			return 0;
	}

	// ATA-1 7.2.13 - if BSY=1, all reads of the command block return the status register
	if (mRFile.mStatus & kATIDEStatus_BSY)
		return mRFile.mStatus;

	if (idx == 0)
		return (uint8)ReadDataLatch(false);

	return mRegisters[idx];
}

uint8 ATIDEEmulator::ReadByte(uint8 address) {
	if (mbReset)
		return 0xD0;

	if (address >= 8)
		return 0xFF;

	uint32 idx = address & 7;

	UpdateStatus();

	// ATA/ATAPI-4 9.16.0 -- when device 1 is selected with only device 0 present,
	// status and alternate status return 00h, while all other reads are the same as
	// device 0.
	if (mRFile.mHead & 0x10) {
		if (idx == 7)
			return 0;
	}

	// ATA-1 7.2.13 - if BSY=1, all reads of the command block return the status register
	if (mRFile.mStatus & kATIDEStatus_BSY)
		return mRFile.mStatus;

	if (idx == 0) {
		uint8 v = (uint8)ReadDataLatch(true);
		return v;
	}

	return mRegisters[idx];
}

void ATIDEEmulator::WriteByte(uint8 address, uint8 value) {
	if (address >= 8 || mbReset)
		return;

	uint32 idx = address & 7;

	switch(idx) {
		case 0:		// data
			WriteDataLatch(value, 0xFF);
			break;

		case 1:		// features
			mFeatures = value;
			break;

		case 2:		// sector count
		case 3:		// sector number / LBA 0-7
		case 4:		// cylinder low / LBA 8-15
		case 5:		// cylinder high / LBA 16-23
		case 6:		// drive/head / LBA 24-27
			UpdateStatus();

			if (mRFile.mStatus & kATIDEStatus_BSY) {
				g_ATLCIDEError("IDE: Attempted write of $%02x to register file index $%02x while drive is busy.\n", value, idx);
//				g_sim.PostInterruptingEvent(kATSimEvent_VerifierFailure);
			} else {
				// bits 7 and 5 in the drive/head register are always 1
				if (idx == 6)
					value |= 0xa0;

				mRegisters[idx] = value;
			}
			break;

		case 7:		// command
			// ignore drive 1 commands except for EXECUTE DEVICE DIAGNOSTIC
			if ((mRFile.mHead & 0x10) && value != 0x90)
				return;

			// check if drive is busy
			UpdateStatus();

			if (mRFile.mStatus & kATIDEStatus_BSY) {
				g_ATLCIDEError("IDE: Attempt to start command $%02x while drive is busy.\n", value, idx);
			} else {
				StartCommand(value);
			}
			break;
	}
}

void ATIDEEmulator::ResetDevice() {
	mActiveCommand = 0;
	mActiveCommandNextTime = 0;
	mActiveCommandState = 0;

	// ATA-1 8.1 Reset Response / ATA-4 9.1 Signature and persistence
	mRFile.mData			= 0x00;
	mRFile.mErrors			= 0x01;
	mRFile.mSectorCount		= 0x01;
	mRFile.mSectorNumber	= 0x01;
	mRFile.mCylinderLow		= 0x00;
	mRFile.mCylinderHigh	= 0x00;
	mRFile.mHead			= 0x00;
	mRFile.mStatus			= kATIDEStatus_DRDY | kATIDEStatus_DSC;

	mFeatures = 0;
	mbTransfer16Bit = true;

	memset(mTransferBuffer.data(), 0, mTransferBuffer.size());
}

void ATIDEEmulator::UpdateStatus() {
	if (!mActiveCommandState)
		return;

	uint32 t = ATSCHEDULER_GETTIME(mpScheduler);

	if ((sint32)(t - mActiveCommandNextTime) < 0)
		return;

	switch(mActiveCommand) {
		case 0x10:		case 0x11:		case 0x12:		case 0x13:
		case 0x14:		case 0x15:		case 0x16:		case 0x17:
		case 0x18:		case 0x19:		case 0x1A:		case 0x1B:
		case 0x1C:		case 0x1D:		case 0x1E:		case 0x1F:
			// recalibrate (ATA-1 mandatory)
			switch(mActiveCommandState) {
				case 1:
					mRFile.mStatus |= kATIDEStatus_BSY;
					++mActiveCommandState;
					mActiveCommandNextTime = t + 100000;
					break;

				case 2:
					CompleteCommand();
					break;
			}
			break;

		case 0x20:	// read sector(s) w/retry
		case 0x21:	// read sector(s) w/o retry
		case 0xC4:	// read multiple
			switch(mActiveCommandState) {
				case 1:
					mRFile.mStatus |= kATIDEStatus_BSY;
					++mActiveCommandState;

					// BOGUS: FDISK.BAS requires a delay before BSY drops since it needs to see
					// BSY=1. ATA-4 7.15.6.1 BSY (Busy) states that this is not safe as the drive
					// may operate too quickly to spot this.
					mActiveCommandNextTime = t + mIODelaySetting;
					break;

				case 2:
					{
						uint32 lba;
						uint32 nsecs = mRFile.mSectorCount;

						if (!nsecs)
							nsecs = 256;

						if (!ReadLBA(lba)) {
							AbortCommand(0);
							return;
						}

						g_ATLCIDE("IDE: Reading %u sectors starting at LBA %u (CHS %s).\n", nsecs, lba, DecodeCHS(lba).c_str());

						mpUIRenderer->SetIDEActivity(false, lba);

						if (lba >= mSectorCount || mSectorCount - lba < nsecs || nsecs >= mMaxSectorTransferCount) {
							mRFile.mStatus |= kATIDEStatus_ERR;
							CompleteCommand();
						} else {
							try {
								if (mpPhysDisk) {
									mpPhysDisk->ReadSectors(mTransferBuffer.data(), lba, nsecs);
								} else {
									mFile.seek((sint64)lba << 9);

									uint32 requested = nsecs << 9;
									uint32 actual = mFile.readData(mTransferBuffer.data(), requested);

									if (requested < actual)
										memset(mTransferBuffer.data() + actual, 0, requested - actual);
								}
							} catch(const MyError& e) {
								g_ATLCIDEError("IDE: I/O ERROR: %s\n", e.gets());
								AbortCommand(kATIDEError_UNC);
								return;
							}

							WriteLBA(lba + nsecs - 1);

							BeginReadTransfer(nsecs << 9);
						}

						mActiveCommandState = 0;
					}
					break;

			}
			break;

		case 0x30:	// write sector(s) w/retry
		case 0x31:	// write sector(s) w/o retry
		case 0xC5:	// write multiple
			switch(mActiveCommandState) {
				case 1:
					mRFile.mStatus |= kATIDEStatus_BSY;
					++mActiveCommandState;
					mActiveCommandNextTime = t + 250;
					break;

				case 2:
					{
						uint32 lba;
						if (!ReadLBA(lba)) {
							AbortCommand(0);
							return;
						}

						uint32 nsecs = mRFile.mSectorCount;

						if (!nsecs)
							nsecs = 256;

						g_ATLCIDE("IDE: Writing %u sectors starting at LBA %u (CHS %s).\n", nsecs, lba, DecodeCHS(lba).c_str());

						if (!mbWriteEnabled) {
							g_ATLCIDEError("IDE: Write blocked due to read-only status.\n");
							AbortCommand(0);
						}

						mpUIRenderer->SetIDEActivity(true, lba);

						if (lba >= mSectorCount || mSectorCount - lba < nsecs || nsecs >= mMaxSectorTransferCount) {
							ATConsoleWrite("IDE: Returning error due to invalid command parameters.\n");
							mRFile.mStatus |= kATIDEStatus_ERR;
							CompleteCommand();
						} else {
							// Note that we are actually transferring 256 words, but the Atari only reads
							// the low bytes.
							mTransferLBA = lba;
							mTransferSectorCount = nsecs;
							BeginWriteTransfer(nsecs << 9);
							++mActiveCommandState;
						}
					}
					break;

				case 3:
					if (mTransferIndex < mTransferLength)
						break;

					try {
						if (mpPhysDisk) {
							mpPhysDisk->WriteSectors(mTransferBuffer.data(), mTransferLBA, mTransferSectorCount);
						} else {
							mFile.seek((sint64)mTransferLBA << 9);
							mFile.write(mTransferBuffer.data(), 512 * mTransferSectorCount);
						}
					} catch(const MyError& e) {
						g_ATLCIDEError("IDE: I/O ERROR: %s\n", e.gets());
						AbortCommand(kATIDEError_UNC);
						return;
					}

					WriteLBA(mTransferLBA + mTransferSectorCount - 1);

					mRFile.mStatus |= kATIDEStatus_BSY;
					++mActiveCommandState;
					mActiveCommandNextTime = t + mIODelaySetting;
					break;

				case 4:
					CompleteCommand();
					break;
			}
			break;

		case 0x40:	// read verify sectors w/retry (ATA-1 mandatory)
		case 0x41:	// read verify sectors w/o retry (ATA-1 mandatory)
			switch(mActiveCommandState) {
				case 1:
					mRFile.mStatus |= kATIDEStatus_BSY;
					++mActiveCommandState;

					// BOGUS: FDISK.BAS requires a delay before BSY drops since it needs to see
					// BSY=1. ATA-4 7.15.6.1 BSY (Busy) states that this is not safe as the drive
					// may operate too quickly to spot this.
					mActiveCommandNextTime = t + mIODelaySetting;
					break;

				case 2:
					{
						uint32 lba;
						uint32 nsecs = mRFile.mSectorCount;

						if (!nsecs)
							nsecs = 256;

						if (!ReadLBA(lba)) {
							AbortCommand(0);
							return;
						}

						g_ATLCIDE("IDE: Verifying %u sectors starting at LBA %u (CHS %s).\n", nsecs, lba, DecodeCHS(lba).c_str());

						mpUIRenderer->SetIDEActivity(false, lba);

						if (lba >= mSectorCount || mSectorCount - lba < nsecs || nsecs >= mMaxSectorTransferCount) {
							mRFile.mStatus |= kATIDEStatus_ERR;
							CompleteCommand();
						} else {
							WriteLBA(lba + nsecs - 1);
						}

						mActiveCommandState = 0;
					}
					break;

			}
			break;

		case 0x70:		case 0x71:		case 0x72:		case 0x73:
		case 0x74:		case 0x75:		case 0x76:		case 0x77:
		case 0x78:		case 0x79:		case 0x7A:		case 0x7B:
		case 0x7C:		case 0x7D:		case 0x7E:		case 0x7F:
			// seek (ATA-1 mandatory)
			switch(mActiveCommandState) {
				case 1:
					mRFile.mStatus |= kATIDEStatus_BSY;
					mRFile.mStatus &= ~kATIDEStatus_DSC;
					++mActiveCommandState;
					mActiveCommandNextTime = t + 5000;
					break;

				case 2:
					mRFile.mStatus |= kATIDEStatus_DSC;
					CompleteCommand();
					break;
			}
			break;

		case 0x90:	// execute device diagnostic
			switch(mActiveCommandState) {
				case 1:
					mRFile.mStatus |= kATIDEStatus_BSY;
					++mActiveCommandState;
					mActiveCommandNextTime = t + 500;
					break;

				case 2:
					// ATA/ATAPI-4 Table 10 (p.72)
					mRFile.mErrors = 0x01;
					mRFile.mSectorCount = 0x01;
					mRFile.mSectorNumber = 0x01;
					mRFile.mCylinderLow = 0;
					mRFile.mCylinderHigh = 0;
					CompleteCommand();
					break;
			}
			break;

		case 0x91:	// initialize device parameters
			switch(mActiveCommandState) {
				case 1:
					mRFile.mStatus |= kATIDEStatus_BSY;
					++mActiveCommandState;
					mActiveCommandNextTime = t + 500;
					break;

				case 2:
					if (!mRFile.mSectorCount) {
						AbortCommand(0);
						break;
					}

					mCurrentSectorsPerTrack = mRFile.mSectorCount;
					mCurrentHeadCount = (mRFile.mHead & 15) + 1;
					mCurrentCylinderCount = mSectorCount / (mCurrentHeadCount * mCurrentSectorsPerTrack);
					AdjustCHSTranslation();
					CompleteCommand();
					break;
			}
			break;

		case 0xc6:	// set multiple mode
			switch(mActiveCommandState) {
				case 1:
					mRFile.mStatus |= kATIDEStatus_BSY;
					++mActiveCommandState;
					mActiveCommandNextTime = t + 500;
					break;

				case 2:
					// sector count must be a power of two
					if (mRFile.mSectorCount >= 2 && !(mRFile.mSectorCount & (mRFile.mSectorCount - 1)))
						CompleteCommand();
					else
						AbortCommand(0);
					break;
			}
			break;

		case 0xec:	// identify drive
			switch(mActiveCommandState) {
				case 1:
					mRFile.mStatus |= kATIDEStatus_BSY;
					++mActiveCommandState;
					mActiveCommandNextTime = t + 10000;
					break;

				case 2:
					{
						uint8 *dst = mTransferBuffer.data();

						// See ATA-1 Table 11 for format.
						memset(dst, 0, 512);

						// word 0: capabilities
						dst[ 0*2+0] = 0x4c;		// soft sectored, not MFM encoded, fixed drive
						dst[ 0*2+1] = 0x04;		// xfer >10Mbps

						// word 1: cylinder count
						if (mSectorCount >= 15614064) {
							// ATA/ATAPI-4 6.2.1/1 (page 20) - must return 16384 for >8GB
							dst[ 1*2+1] = 0x3F;
							dst[ 1*2+0] = 0xFF;
						} else {
							dst[ 1*2+1] = (uint8)(mCylinderCount >> 8);
							dst[ 1*2+0] = (uint8)mCylinderCount;	// cylinder count
						}

						// word 2: reserved
						dst[ 2*2+0] = 0;			// reserved
						dst[ 2*2+1] = 0;

						// word 3: number of logical heads
						dst[ 3*2+0] = (uint8)mHeadCount;// number of heads
						dst[ 3*2+1] = 0;

						// word 4: number of unformatted bytes per track
						dst[ 4*2+0] = 0;
						dst[ 4*2+1] = (uint8)(2 * mSectorsPerTrack);

						// word 5: number of unformatted bytes per sector (ATA-1), retired (ATA-4)
						dst[ 5*2+0] = 0;
						dst[ 5*2+1] = 2;

						// word 6: number of sectors per track (ATA-1), retired (ATA-4)
						dst[ 6*2+0] = (uint8)mSectorsPerTrack;		// sectors per track
						dst[ 6*2+1] = 0;

						// words 7-9: vendor unique (ATA-1), retired (ATA-4)

						// words 10-19: serial number
						char buf[16];
						sprintf(buf, "%010u", (uint32)VDHashString32I(mPath.c_str()));

						for(int i=0; i<10; ++i)
							dst[10*2 + (i ^ 1)] = (uint8)buf[i];

						// word 20: buffer type (ATA-1), retired (ATA-4)
						dst[20*2+1] = 0x00;
						dst[20*2+0] = 0x03;		// dual ported w/ read caching

						// word 21: buffer size (ATA-1), retired (ATA-4)
						dst[21*2+1] = 0x00;
						dst[21*2+0] = 0x00;		// not specified

						// word 22: ECC bytes for read/write long commands (ATA-1), obsolete (ATA-4)
						dst[22*2+1] = 0x00;
						dst[22*2+0] = 0x04;

						// words 23-26: firmware revision
						dst[23*2+1] = '1';
						dst[23*2+0] = '.';
						dst[24*2+1] = '0';

						// words 27-46: model number
						dst[27*2+1] = 'G';
						dst[27*2+0] = 'E';
						dst[28*2+1] = 'N';
						dst[28*2+0] = 'E';
						dst[29*2+1] = 'R';
						dst[29*2+0] = 'I';
						dst[30*2+1] = 'C';
						dst[30*2+0] = ' ';

						// word 47
						dst[47*2+0] = 0xFF;		// max sectors/interrupt
						dst[47*2+1] = 0x80;

						// word 48: reserved
						// word 49: capabilities
						dst[49*2+1] = 0x0F;
						dst[49*2+0] = 0;		// capabilities (LBA supported, DMA supported)

						// word 50: reserved (ATA-1), capabilities (ATA-4)
						dst[50*2+1] = 0x40;
						dst[50*2+0] = 0x00;

						// word 51: PIO data transfer timing mode (ATA-1)
						dst[51*2+1] = 2;
						dst[51*2+0] = 0;		// PIO data transfer timing mode (PIO 2)

						// word 52: DMA data transfer timing mode (ATA-1)
						dst[52*2+1] = 0;
						dst[52*2+0] = 0;		// DMA data transfer timing mode (DMA 0)

						// word 53: misc
						dst[53*2+1] = 0x00;
						dst[53*2+0] = 0x03;		// words 54-58 are valid

						// word 54: number of current logical cylinders
						dst[54*2+1] = (uint8)(mCurrentCylinderCount >> 8);
						dst[54*2+0] = (uint8)(mCurrentCylinderCount     );

						// word 55: number of current logical heads
						dst[55*2+1] = (uint8)(mCurrentHeadCount >> 8);
						dst[55*2+0] = (uint8)(mCurrentHeadCount     );

						// word 56: number of current logical sectors per track
						dst[55*2+1] = (uint8)(mCurrentSectorsPerTrack >> 8);
						dst[55*2+0] = (uint8)(mCurrentSectorsPerTrack     );

						// words 57-58: current capacity in sectors
						VDWriteUnalignedLEU32(&dst[57*2], mCurrentCylinderCount * mCurrentHeadCount * mCurrentSectorsPerTrack);

						// word 59: misc

						// words 60-61: total number of user addressable LBA sectors (28-bit)
						VDWriteUnalignedLEU32(&dst[60*2], std::min<uint32>(mSectorCount, 0x0FFFFFFF));

						// words 62-63: single/multiword DMA status
						dst[62*2+1] = 0x01;		// DMA 0 active
						dst[62*2+0] = 0x07;		// DMA 0-2 supported
						dst[63*2+1] = 0x01;		// DMA 0 active
						dst[63*2+0] = 0x07;		// DMA 0-2 supported

						if (mbFastDevice) {
							// word 64: PIO transfer modes supported
							dst[64*2+1] = 0x00;
							dst[64*2+0] = 0x0F;		// PIO modes 3-6 supported

							// word 65: minimum multiword DMA transfer cycle time per word
							dst[65*2+1] = 0x00;
							dst[65*2+0] = 0x50;		// 80ns rate (25MB/sec)

							// word 66: recommended multiword DMA transfer cycle time
							dst[66*2+1] = 0x00;
							dst[66*2+0] = 0x50;		// 80ns rate (25MB/sec)

							// word 67: minimum PIO transfer without flow control cycle time
							dst[67*2+1] = 0x00;
							dst[67*2+0] = 0x50;		// 80ns rate (25MB/sec)

							// word 68: minimum PIO transfer with IORDY
							dst[68*2+1] = 0x00;
							dst[68*2+0] = 0x50;		// 80ns rate (25MB/sec)
						} else {
							// word 64: PIO transfer modes supported
							dst[64*2+1] = 0x00;
							dst[64*2+0] = 0x03;		// PIO modes 3-4 supported

							// word 65: minimum multiword DMA transfer cycle time per word
							dst[65*2+1] = 0x00;
							dst[65*2+0] = 0x78;		// 120ns rate (16.7MB/sec)

							// word 66: recommended multiword DMA transfer cycle time
							dst[66*2+1] = 0x00;
							dst[66*2+0] = 0x78;		// 120ns rate (16.7MB/sec)

							// word 67: minimum PIO transfer without flow control cycle time
							dst[67*2+1] = 0x00;
							dst[67*2+0] = 0x78;		// 120ns rate (16.7MB/sec)

							// word 68: minimum PIO transfer with IORDY
							dst[68*2+1] = 0x00;
							dst[68*2+0] = 0x78;		// 120ns rate (16.7MB/sec)
						}

						// words 100-103: total user addressable sectors in 48-bit LBA mode
						VDWriteUnalignedLEU64(&dst[100*2], mSectorCount);

						BeginReadTransfer(512);
						mActiveCommandState = 0;
					}
					break;

			}
			break;

		case 0xef:	// set features
			switch(mActiveCommandState) {
				case 1:
					mRFile.mStatus |= kATIDEStatus_BSY;
					++mActiveCommandState;
					mActiveCommandNextTime = t + 250;
					break;

				case 2:
					switch(mFeatures) {
						case 0x01:		// enable 8-bit data transfers
							mbTransfer16Bit = false;
							CompleteCommand();
							break;

						case 0x03:		// set transfer mode (based on sector count register)
							switch(mRFile.mSectorCount) {
								case 0x00:	// PIO default mode
								case 0x01:	// PIO default mode, disable IORDY
								case 0x08:	// PIO mode 0
								case 0x09:	// PIO mode 1
								case 0x0A:	// PIO mode 2
								case 0x0B:	// PIO mode 3
								case 0x0C:	// PIO mode 4
								case 0x20:	// DMA mode 0
								case 0x21:	// DMA mode 1
								case 0x22:	// DMA mode 2
									CompleteCommand();
									break;

								case 0x0D:	// PIO mode 5 (CF)
								case 0x0E:	// PIO mode 6 (CF)
									if (mbFastDevice) {
										CompleteCommand();
										break;
									}

									// fall through

								default:
									g_ATLCIDEError("Unsupported transfer mode: %02x", mRFile.mSectorCount);
									AbortCommand(0);
									break;
							}
							break;

						case 0x81:		// disable 8-bit data transfers
							mbTransfer16Bit = true;
							CompleteCommand();
							break;

						default:
							g_ATLCIDEError("Unsupported set feature parameter: %02x", mFeatures);
							AbortCommand(0);
							break;
					}
					break;
			}
			break;

		default:
			g_ATLCIDEError("IDE: Unrecognized command $%02x.\n", mActiveCommand);
			AbortCommand(0);
			break;
	}
}

void ATIDEEmulator::StartCommand(uint8 cmd) {
	mRFile.mStatus &= ~kATIDEStatus_ERR;

	// BOGUS: This is unfortunately necessary to get FDISK.BAS to work, but it shouldn't
	// be necessary: ATA-4 7.15.6.6 ERR (Error) states that the ERR register shall
	// be ignored by the host when the ERR bit is 0.
	mRFile.mErrors = 0;

	mActiveCommand = cmd;
	mActiveCommandState = 1;
	mActiveCommandNextTime = ATSCHEDULER_GETTIME(mpScheduler);

	UpdateStatus();
}

void ATIDEEmulator::BeginReadTransfer(uint32 bytes) {
	mRFile.mStatus |= kATIDEStatus_DRQ;
	mRFile.mStatus &= ~kATIDEStatus_BSY;
	mTransferIndex = 0;
	mTransferLength = bytes;
	mbTransferAsWrites = false;
}

void ATIDEEmulator::BeginWriteTransfer(uint32 bytes) {
	mRFile.mStatus |= kATIDEStatus_DRQ;
	mRFile.mStatus &= ~kATIDEStatus_BSY;
	mTransferIndex = 0;
	mTransferLength = bytes;
	mbTransferAsWrites = true;
}

void ATIDEEmulator::CompleteCommand() {
	mRFile.mStatus &= ~kATIDEStatus_BSY;
	mRFile.mStatus &= ~kATIDEStatus_DRQ;
	mActiveCommand = 0;
	mActiveCommandState = 0;
}

void ATIDEEmulator::AbortCommand(uint8 error) {
	mRFile.mStatus &= ~kATIDEStatus_BSY;
	mRFile.mStatus &= ~kATIDEStatus_DRQ;
	mRFile.mStatus |= kATIDEStatus_ERR;
	mRFile.mErrors = error | kATIDEError_ABRT;
	mActiveCommand = 0;
	mActiveCommandState = 0;
}

bool ATIDEEmulator::ReadLBA(uint32& lba) {
	if (mRFile.mHead & 0x40) {
		// LBA mode
		lba = ((uint32)(mRFile.mHead & 0x0f) << 24)
			+ ((uint32)mRFile.mCylinderHigh << 16)
			+ ((uint32)mRFile.mCylinderLow << 8)
			+ (uint32)mRFile.mSectorNumber;

		if (lba >= mSectorCount) {
			g_ATLCIDEError("IDE: Invalid LBA %u >= %u\n", lba, mSectorCount);
			return false;
		}

		return true;
	} else {
		// CHS mode
		uint32 head = mRFile.mHead & 15;
		uint32 sector = mRFile.mSectorNumber;
		uint32 cylinder = ((uint32)mRFile.mCylinderHigh << 8) + mRFile.mCylinderLow;

		if (!sector || sector > mCurrentSectorsPerTrack) {
			g_ATLCIDEError("IDE: Invalid CHS %u/%u/%u (bad sector number)\n", cylinder, head, sector);
			return false;
		}

		lba = (sector - 1) + (cylinder*mCurrentHeadCount + head)*mCurrentSectorsPerTrack;
		if (lba >= mSectorCount) {
			g_ATLCIDEError("IDE: Invalid CHS %u/%u/%u (beyond total sector count of %u)\n", cylinder, head, sector, mSectorCount);
			return false;
		}

		return true;
	}
}

void ATIDEEmulator::WriteLBA(uint32 lba) {
	if (mRFile.mHead & 0x40) {
		// LBA mode
		mRFile.mHead = (mRFile.mHead & 0xf0) + ((lba >> 24) & 0x0f);
		mRFile.mCylinderHigh = (uint8)(lba >> 16);
		mRFile.mCylinderLow = (uint8)(lba >> 8);
		mRFile.mSectorNumber = (uint8)lba;
	} else if (mCurrentSectorsPerTrack && mCurrentHeadCount) {
		// CHS mode
		uint32 track = lba / mCurrentSectorsPerTrack;
		uint32 sector = lba % mCurrentSectorsPerTrack;
		uint32 cylinder = track / mCurrentHeadCount;
		uint32 head = track % mCurrentHeadCount;

		mRFile.mHead = (mRFile.mHead & 0xf0) + head;
		mRFile.mCylinderHigh = (uint8)(cylinder >> 8);
		mRFile.mCylinderLow = (uint8)cylinder;
		mRFile.mSectorNumber = sector + 1;
	} else {
		// uh...

		mRFile.mHead = (mRFile.mHead & 0xf0);
		mRFile.mCylinderHigh = 0;
		mRFile.mCylinderLow = 0;
		mRFile.mSectorNumber = 1;
	}
}

void ATIDEEmulator::ResetCHSTranslation() {
	mCurrentCylinderCount = mCylinderCount;
	mCurrentSectorsPerTrack = mSectorsPerTrack;
	mCurrentHeadCount = mHeadCount;

	if (mCurrentSectorsPerTrack > 63) {
		mCurrentSectorsPerTrack = 63;
		mCurrentCylinderCount = mSectorCount / (mCurrentHeadCount * 63);
	}

	AdjustCHSTranslation();
}

void ATIDEEmulator::AdjustCHSTranslation() {
	if (mCurrentCylinderCount > 65535)
		mCurrentCylinderCount = 65535;

	if (mSectorCount >= 16514064) {
		uint32 limitCyl = 16514064 / (mCurrentHeadCount * mCurrentSectorsPerTrack);

		if (mCurrentCylinderCount > limitCyl)
			mCurrentCylinderCount = limitCyl;
	}
}

ATIDEEmulator::DecodedCHS ATIDEEmulator::DecodeCHS(uint32 lba) {
	DecodedCHS s;

	if (!mCurrentSectorsPerTrack || !mCurrentHeadCount) {
		strcpy(s.buf, "???");
	} else {
		uint32 track = lba / mCurrentSectorsPerTrack;
		uint32 sector = lba % mCurrentSectorsPerTrack;
		uint32 cylinder = track / mCurrentHeadCount;
		uint32 head = track % mCurrentHeadCount;
		sprintf(s.buf, "%u/%u/%u", cylinder, head, sector);
	}

	return s;
}
