//	Altirra - Atari 800/800XL/5200 emulator
//	Copyright (C) 2008-2011 Avery Lee
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
#include "soundboard.h"
#include "scheduler.h"
#include "audiooutput.h"
#include "memorymanager.h"

ATSoundBoardEmulator::ATSoundBoardEmulator()
	: mpMemory(NULL)
	, mpMemLayerControl(NULL)
	, mpScheduler(NULL)
	, mpMemMan(NULL)
	, mpAudioOut(NULL)
	, mMemBase(0xD2C0)
{
}

ATSoundBoardEmulator::~ATSoundBoardEmulator() {
	Shutdown();
}

void ATSoundBoardEmulator::SetMemBase(uint32 membase) {
	mMemBase = membase;
}

void ATSoundBoardEmulator::Init(ATMemoryManager *memMan, ATScheduler *sch, IATAudioOutput *audioOut) {
	mpMemory = new uint8[524288];
	mpMemMan = memMan;
	mpScheduler = sch;
	mpAudioOut = audioOut;

	ColdReset();
}

void ATSoundBoardEmulator::Shutdown() {
	if (mpMemMan) {
		if (mpMemLayerControl) {
			mpMemMan->DeleteLayer(mpMemLayerControl);
			mpMemLayerControl = NULL;
		}

		mpMemMan = NULL;
	}

	if (mpMemory) {
		delete[] mpMemory;
		mpMemory = NULL;
	}
}

void ATSoundBoardEmulator::ColdReset() {
	mLoadAddress = 0;
	mpCurChan = &mChannels[0];

	mAccumLevel = 0;
	mAccumPhase = 0;
	mAccumOffset = 0;
	mGeneratedCycles = 0;
	mLastUpdate = ATSCHEDULER_GETTIME(mpScheduler);
	mCycleAccum = 0;

	if (mpMemLayerControl) {
		mpMemMan->DeleteLayer(mpMemLayerControl);
		mpMemLayerControl = NULL;
	}

	ATMemoryHandlerTable handlers = {};
	handlers.mpThis = this;

	switch(mMemBase) {
		case 0xD2C0:
		default:
			handlers.mbPassAnticReads = true;
			handlers.mbPassReads = true;
			handlers.mbPassWrites = true;
			handlers.mpDebugReadHandler = StaticDebugReadD2xxControl;
			handlers.mpReadHandler = StaticReadD2xxControl;
			handlers.mpWriteHandler = StaticWriteD2xxControl;
			mpMemLayerControl = mpMemMan->CreateLayer(kATMemoryPri_HardwareOverlay, handlers, 0xD2, 0x01);
			break;

		case 0xD500:
			handlers.mbPassAnticReads = false;
			handlers.mbPassReads = false;
			handlers.mbPassWrites = false;
			handlers.mpDebugReadHandler = StaticDebugReadD5xxControl;
			handlers.mpReadHandler = StaticReadD5xxControl;
			handlers.mpWriteHandler = StaticWriteD5xxControl;
			mpMemLayerControl = mpMemMan->CreateLayer(kATMemoryPri_HardwareOverlay, handlers, 0xD5, 0x01);
			break;

		case 0xD600:
			handlers.mbPassAnticReads = false;
			handlers.mbPassReads = false;
			handlers.mbPassWrites = false;
			handlers.mpDebugReadHandler = StaticDebugReadD5xxControl;
			handlers.mpReadHandler = StaticReadD5xxControl;
			handlers.mpWriteHandler = StaticWriteD5xxControl;
			mpMemLayerControl = mpMemMan->CreateLayer(kATMemoryPri_HardwareOverlay, handlers, 0xD6, 0x01);
			break;
	}

	mpMemMan->EnableLayer(mpMemLayerControl, true);

	WarmReset();
}

void ATSoundBoardEmulator::WarmReset() {
	memset(mChannels, 0, sizeof mChannels);

	memset(mAccumBufferLeft, 0, sizeof mAccumBufferLeft);
	memset(mAccumBufferRight, 0, sizeof mAccumBufferRight);
}

uint8 ATSoundBoardEmulator::DebugReadControl(uint8 addr) const {
	switch(addr & 0x1F) {
		case 0x00:
			return 0x53;

		case 0x01:
			return 0x42;

		case 0x02:		// major version
			return 0x01;

		case 0x03:		// minor version
			return 0x02;

		case 0x13:
			return mpMemory[mLoadAddress & 0x7ffff];

		default:
			return 0xFF;
	}
}

uint8 ATSoundBoardEmulator::ReadControl(uint8 addr) {
	switch(addr & 0x1F) {
		case 0x00:
			return 0x53;

		case 0x01:
			return 0x42;

		case 0x02:		// major version
			return 0x01;

		case 0x03:		// minor version
			return 0x02;

		case 0x13:
			return mpMemory[mLoadAddress++ & 0x7ffff];

		default:
			return 0xFF;
	}
}

void ATSoundBoardEmulator::WriteControl(uint8 addr, uint8 value) {
	if (addr < 0x14)
		Flush();

	switch(addr) {
		case 0x00:	// address low
			mpCurChan->mAddress = (mpCurChan->mAddress & 0x7ff00) + value;
			break;

		case 0x01:	// address mid
			mpCurChan->mAddress = (mpCurChan->mAddress & 0x700ff) + ((uint32)value << 8);
			break;

		case 0x02:	// address high
			mpCurChan->mAddress = (mpCurChan->mAddress & 0xffff) + ((uint32)(value & 0x07) << 16);
			break;

		case 0x03:	// length low
			mpCurChan->mLength = (mpCurChan->mLength & 0xff00) + value;
			break;

		case 0x04:	// length high
			mpCurChan->mLength = (mpCurChan->mLength & 0x00ff) + ((uint32)value << 8);
			break;

		case 0x05:	// repeat low
			mpCurChan->mRepeat = (mpCurChan->mRepeat & 0xff00) + value;
			break;

		case 0x06:	// repeat high
			mpCurChan->mRepeat = (mpCurChan->mRepeat & 0x00ff) + ((uint32)value << 8);
			break;

		case 0x07:	// freq low
			mpCurChan->mFreq = (mpCurChan->mFreq & 0xff00) + value;
			break;

		case 0x08:	// freq high
			mpCurChan->mFreq = (mpCurChan->mFreq & 0x00ff) + ((uint32)value << 8);
			break;

		case 0x09:	// volume
			mpCurChan->mVolume = value;
			break;

		case 0x0A:	// pan
			mpCurChan->mPan = value;
			break;

		case 0x0B:	// attack
			mpCurChan->mAttack = value;
			break;

		case 0x0C:	// decay
			mpCurChan->mDecay = value;
			break;

		case 0x0D:	// sustain
			mpCurChan->mSustain = value;
			break;

		case 0x0E:	// release
			mpCurChan->mRelease = value;
			break;

		case 0x0F:	// control
			mpCurChan->mControl = value;

			if (!(value & 1))
				mpCurChan->mPhase = 0;
			break;

		case 0x10:	// load address low
			mLoadAddress = (mLoadAddress & 0x7ff00) + value;
			break;

		case 0x11:	// load address med
			mLoadAddress = (mLoadAddress & 0x700ff) + ((uint32)value << 8);
			break;

		case 0x12:	// load address high
			mLoadAddress = (mLoadAddress & 0x0ffff) + ((uint32)(value & 0x07) << 16);
			break;

		case 0x13:	// load byte
			mpMemory[mLoadAddress++ & 0x7ffff] = value;
			break;

		case 0x14:	// channel select
			mpCurChan = &mChannels[value & 7];
			break;
	}
}

void ATSoundBoardEmulator::Run(uint32 cycles) {
	cycles += mCycleAccum;

	while(cycles) {
		// compute samples we can generate based on time
		uint32 samplesToGenerate = cycles / 6;

		// don't generate more than a sample buffer at a time
		if (samplesToGenerate > kSampleBufferSize)
			samplesToGenerate = kSampleBufferSize;

		// if we can't generate anything, we're done
		if (!samplesToGenerate)
			break;

		// subtract cycles from budget
		cycles -= samplesToGenerate * 6;

		// determine how many samples we can accumulate at 1/28 rate
		mGeneratedCycles += samplesToGenerate * 6;

		const uint32 samplesToAccum = mGeneratedCycles / 28;

		mGeneratedCycles -= samplesToAccum * 28;

		VDASSERT(mAccumLevel + samplesToAccum <= kAccumBufferSize);

		uint32 samplesEatenByAccum = 0;

		if (samplesToAccum) {
			samplesEatenByAccum += 5 * samplesToAccum;
			switch(mAccumPhase) {
				case 0:
					samplesEatenByAccum -= (samplesToAccum + 2) / 3;
					break;
				case 1:
					samplesEatenByAccum -= (samplesToAccum + 0) / 3;
					break;
				case 2:
					samplesEatenByAccum -= (samplesToAccum + 1) / 3;
					break;
			}
		}

		for(int chidx = 0; chidx < 8; ++chidx) {
			Channel *__restrict ch = &mChannels[chidx];
			const uint32 vol = ch->mVolume;

			if (!vol || !(ch->mControl & 0x01)) {
				// If the channel has DMA enabled but is shut off, we still have to update
				// the phase.
				if (ch->mControl & 0x01) {
					uint32 phase = ch->mPhase;
					uint32 freq = ch->mFreq;
					uint32 length = ch->mLength << 16;
					uint32 repeat = ch->mRepeat << 16;

					// this is pretty lame, but it correctly handles the lost phase on
					// the repeat
					for(uint32 i = 0; i < samplesToGenerate; ++i) {
						phase += freq;

						if (phase >= length)
							phase = repeat;
					}

					ch->mPhase = phase;
				}

				// Shift the overlap buffer.
				if (samplesEatenByAccum) {
					if (samplesEatenByAccum >= kSampleBufferOverlap)
						memset(ch->mOverlapBuffer, 0, sizeof ch->mOverlapBuffer);
					else {
						memmove(ch->mOverlapBuffer, ch->mOverlapBuffer + samplesEatenByAccum, sizeof(ch->mOverlapBuffer[0]) * (kSampleBufferOverlap - samplesEatenByAccum));
						memset(ch->mOverlapBuffer + (kSampleBufferOverlap - samplesEatenByAccum), 0, sizeof(ch->mOverlapBuffer[0]) * samplesEatenByAccum);
					}
				}

				continue;
			}

			const uint32 panleft = 255 - ch->mPan;
			const uint32 panright = ch->mPan;
			const uint32 baseAddr = ch->mAddress;
			uint32 phase = ch->mPhase;
			uint32 freq = ch->mFreq;
			uint32 length = ch->mLength << 16;
			uint32 repeat = ch->mRepeat << 16;
			sint16 *sdst = mSampleBuffer;
			uint8 *const mem = mpMemory;

			memcpy(sdst, ch->mOverlapBuffer, kSampleBufferOverlap*sizeof(sdst[0]));
			sdst += kSampleBufferOverlap;

			for(uint32 i = 0; i < samplesToGenerate; ++i) {
				const uint8 sample = mem[(baseAddr + (phase >> 16)) & 0x7ffff];
				phase += freq;

				if (phase >= length)
					phase = repeat;

				*sdst++ = (sint16)((uint32)sample - 0x80);
			}

			ch->mPhase = phase;

			const sint16 *ssrc = mSampleBuffer + mAccumOffset;
			float *dstLeft = mAccumBufferLeft + mAccumLevel;
			float *dstRight = mAccumBufferRight + mAccumLevel;
			float volf = (float)vol * (1.0f / 255.0f / 64.0f);
			float volPanLeft = volf * (float)panleft;
			float volPanRight = volf * (float)panright;

			uint32 accumCounter = samplesToAccum;

			if (accumCounter) {
				// 6 6 6 6 4
				//         2 6 6 6 6 2
				//                   4 6 6 6 6
				//
				// We consume 14 samples in 84 cycles, or one sample every 6 cycles.
				switch(mAccumPhase) {
				case 0:
					do {
						float sample;

						sample = (float)ssrc[0] + (float)ssrc[1] + (float)ssrc[2] + (float)ssrc[3] + (2.0f/3.0f)*(float)ssrc[4];
						*dstLeft++ += sample * volPanLeft;
						*dstRight++ += sample * volPanRight;
						ssrc += 4;

						if (!--accumCounter)
							break;

				case 1:
						sample = (1.0f/3.0f)*(float)ssrc[0] + (float)ssrc[1] + (float)ssrc[2] + (float)ssrc[3] + (float)ssrc[4] + (1.0f/3.0f)*(float)ssrc[5];
						*dstLeft++ += sample * volPanLeft;
						*dstRight++ += sample * volPanRight;
						ssrc += 5;

						if (!--accumCounter)
							break;
				case 2:
						sample = (2.0f/3.0f)*(float)ssrc[0] + (float)ssrc[1] + (float)ssrc[2] + (float)ssrc[3] + (float)ssrc[4];
						*dstLeft++ += sample * volPanLeft;
						*dstRight++ += sample * volPanRight;
						ssrc += 5;
					} while(--accumCounter);
				}
			}

			memcpy(ch->mOverlapBuffer, sdst - kSampleBufferOverlap, sizeof ch->mOverlapBuffer);
		}

		// update accumulator phase and offset
		mAccumPhase = (mAccumPhase + samplesToAccum) % 3;
		mAccumOffset += samplesToGenerate - samplesEatenByAccum;

		VDASSERT(mAccumOffset < kSampleBufferOverlap);

		// update accumulator level
		mAccumLevel += samplesToAccum;
		VDASSERT(mAccumLevel <= kAccumBufferSize);
	}

	mCycleAccum = cycles;
}

void ATSoundBoardEmulator::WriteAudio(const float *left, const float *right, uint32 count, bool pushAudio, uint32 timestamp) {
	Flush();

	VDASSERT(count <= kAccumBufferSize);

	// if we don't have enough samples, pad out; eventually we'll catch up enough
	if (mAccumLevel < count) {
		memset(mAccumBufferLeft + mAccumLevel, 0, sizeof(mAccumBufferLeft[0]) * (count - mAccumLevel));
		memset(mAccumBufferRight + mAccumLevel, 0, sizeof(mAccumBufferRight[0]) * (count - mAccumLevel));

		mAccumLevel = count;
	}

	// add POKEY output into output buffers
	if (right) {
		for(uint32 i=0; i<count; ++i) {
			mAccumBufferLeft[i] += left[i];
			mAccumBufferRight[i] += right[i];
		}
	} else {
		for(uint32 i=0; i<count; ++i) {
			mAccumBufferLeft[i] += left[i];
			mAccumBufferRight[i] += left[i];
		}
	}

	// send mixed audio to output device
	mpAudioOut->WriteAudio(mAccumBufferLeft, mAccumBufferRight, count, pushAudio, timestamp);

	// shift down accumulation buffers
	uint32 samplesLeft = mAccumLevel - count;

	if (samplesLeft) {
		memmove(mAccumBufferLeft, mAccumBufferLeft + count, samplesLeft * sizeof(mAccumBufferLeft[0]));
		memmove(mAccumBufferRight, mAccumBufferRight + count, samplesLeft * sizeof(mAccumBufferRight[0]));
	}

	memset(mAccumBufferLeft + samplesLeft, 0, sizeof(mAccumBufferLeft[0]) * count);
	memset(mAccumBufferRight + samplesLeft, 0, sizeof(mAccumBufferRight[0]) * count);

	mAccumLevel = samplesLeft;
}

void ATSoundBoardEmulator::Flush() {
	uint32 t = ATSCHEDULER_GETTIME(mpScheduler);
	uint32 dt = t - mLastUpdate;
	mLastUpdate = t;

	Run(dt);
}

sint32 ATSoundBoardEmulator::StaticDebugReadD2xxControl(void *thisptr, uint32 addr) {
	uint8 addr8 = (uint8)addr;
	if (addr8 < 0xC0)
		return -1;

	return ((const ATSoundBoardEmulator *)thisptr)->DebugReadControl(addr8 & 0x3f);
}

sint32 ATSoundBoardEmulator::StaticReadD2xxControl(void *thisptr, uint32 addr) {
	uint8 addr8 = (uint8)addr;
	if (addr8 < 0xC0)
		return -1;

	return ((ATSoundBoardEmulator *)thisptr)->ReadControl(addr8 & 0x3f);
}

bool ATSoundBoardEmulator::StaticWriteD2xxControl(void *thisptr, uint32 addr, uint8 value) {
	uint8 addr8 = (uint8)addr;
	if (addr8 < 0xC0)
		return false;

	((ATSoundBoardEmulator *)thisptr)->WriteControl(addr8 & 0x3f, value);
	return true;
}

sint32 ATSoundBoardEmulator::StaticDebugReadD5xxControl(void *thisptr, uint32 addr) {
	return ((const ATSoundBoardEmulator *)thisptr)->DebugReadControl((uint8)addr);
}

sint32 ATSoundBoardEmulator::StaticReadD5xxControl(void *thisptr, uint32 addr) {
	return ((ATSoundBoardEmulator *)thisptr)->ReadControl((uint8)addr);
}

bool ATSoundBoardEmulator::StaticWriteD5xxControl(void *thisptr, uint32 addr, uint8 value) {
	((ATSoundBoardEmulator *)thisptr)->WriteControl((uint8)addr, value);
	return true;
}