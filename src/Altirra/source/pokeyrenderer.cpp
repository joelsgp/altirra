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

//=========================================================================
// POKEY renderer
//
// The POKEY renderer converts register change and timer events within
// the POKEY emulation to audio output. It exclusively handles portions of
// the audio circuits that have no feedback path to the 6502 and thus are
// not observable. Most of the logic simulated here is digital, except for
// downsample from 1.77/1.79MHz to 64KHz.
//
// The general approach is as follows:
//
// - Collected timing and register change events. Timing events are
//   accumulated, while register change events are handled immediately
//   and cause a flush.
//
// - The common cases of a timer running at a regular frequency are handled
//   by the deferred mechanism, where the POKEY emulator pushes the initial
//   timing parameters into the renderer and the renderer expands the
//   periodic ticks. This greatly reduces the timer overhead in the POKEY
//   emulator itself. The 16-bit linked timer case is a bit more
//   complicated to handle the uneven ticking of the low timer.
//
// - Timer events from each channel are converted to output change events.
//   This can involve sampling the polynomial counters for clocking and
//   output noise. The polynomial tables are offset by the initial offset
//   for the frame and then all channels independently sample noise off
//   of the tables.
//
// - Timer events from ch3/4 are also converted to high pass update events
//   for ch1/2. These have to be offset by half a cycle, so the output
//   section uses half-ticks (3.55/3.58MHz).
//
// - The output path XORs the high-pass flip/flops into ch1/2, converts the
//   four channel outputs to a mix level, then box filters the stairstep
//   waveform to 1/28 rate (64KHz). This output is then sent to the audio
//   sync mixer for mixing with non-POKEY sources, AC coupling filtering,
//   and downsample to 44/48KHz.
//

#include <stdafx.h>
#include <vd2/system/binary.h>
#include <vd2/system/bitmath.h>
#include <at/atcore/logging.h>
#include <at/atcore/scheduler.h>
#include <at/atcore/wraptime.h>
#include "pokey.h"
#include "pokeyrenderer.h"
#include "pokeytables.h"
#include "savestate.h"

ATLogChannel g_ATLCPokeyTEv(false, false, "POKEYTEV", "POKEY timer events (high traffic)");

namespace {
	const uint32 kAudioDelay = 2;
}

ATPokeyRenderer::ATPokeyRenderer()
	: mpScheduler(NULL)
	, mpTables(NULL)
	, mAccum(0)
	, mHighPassAccum(0)
	, mOutputLevel(0)
	, mLastOutputTime2(0)
	, mLastOutputSampleTime2(0)
	, mExternalInput(0)
	, mbSpeakerState(false)
	, mOutputSampleCount(0)
	, mpEdgeBuffer(new ATSyncAudioEdgeBuffer)
{
	mpEdgeBuffer->mVolume = 56.0f;

	for(int i=0; i<4; ++i) {
		mbChannelEnabled[i] = true;
		mChannelVolume[i] = 0;
	}
}

ATPokeyRenderer::~ATPokeyRenderer() {
}

void ATPokeyRenderer::Init(ATScheduler *sch, ATPokeyTables *tables) {
	mpScheduler = sch;
	mpTables = tables;

	mLastOutputTime2 = ATSCHEDULER_GETTIME(mpScheduler) * 2;
	mLastOutputSampleTime2 = mLastOutputTime2;

	mSerialPulse = 0.12f;

	ColdReset();
}

void ATPokeyRenderer::ColdReset() {
	mbInitMode = true;
	mPolyState.mInitMask = 0;

	// preset all noise and high-pass flip/flops
	mNoiseFlipFlops = 0x0F;
	mChannelOutputMask = 0x3F;

	for(int i=0; i<4; ++i) {
		mDeferredEvents[i].mbEnabled = false;
	}

	mOutputLevel = 0;
	mOutputSampleCount = 0;
	mAccum = 0;
	mHighPassAccum = 0;

	const uint32 t = ATSCHEDULER_GETTIME(mpScheduler);
	mLastFlushTime = t;
	mLastOutputSampleTime2 = t*2;
	mLastOutputTime2 = t*2;
	mPolyState.mLastPoly17Time = t;
	mPolyState.mLastPoly9Time = t;
	mPolyState.mLastPoly5Time = t;
	mPolyState.mLastPoly4Time = t;
	mPolyState.mPoly17Counter = 0;
	mPolyState.mPoly9Counter = 0;
	mPolyState.mPoly5Counter = 0;
	mPolyState.mPoly4Counter = 0;

	for(ChannelEdges& edges : mChannelEdges)
		edges.clear();

	for(uint32& base : mChannelEdgeBases)
		base = 0;

	mChangeQueue.clear();

	// This must be done after everything else is inited, as it will start recomputing
	// derived values.
	mArchState = {};
	mRenderState = {};

	mChannelOutputMask = 0x30;

	for(int i=0; i<4; ++i)
		UpdateVolume(i);

	UpdateOutput(t);
}

void ATPokeyRenderer::SyncTo(const ATPokeyRenderer& src) {
	mLastFlushTime = src.mLastFlushTime;
	mLastOutputSampleTime2 = src.mLastOutputSampleTime2;
	mLastOutputTime2 = src.mLastOutputTime2;
	mOutputSampleCount = src.mOutputSampleCount;

	memset(mRawOutputBuffer, 0, sizeof(mRawOutputBuffer[0]) * mOutputSampleCount);
}

void ATPokeyRenderer::SetChannelEnabled(int channel, bool enabled) {
	VDASSERT(channel < 4);
	if (mbChannelEnabled[channel] != enabled) {
		const uint32 t = ATSCHEDULER_GETTIME(mpScheduler);
		Flush(t);

		if (mpAudioLog)
			LogOutputChange(t*2);

		mbChannelEnabled[channel] = enabled;

		UpdateVolume(channel);
		UpdateOutput(t);
	}
}

void ATPokeyRenderer::SetAudioLog(ATPokeyAudioLog *log) {
	mpAudioLog = log;

	if (log) {
		log->mTicksPerSample = log->mCyclesPerSample * 2;
		log->mFullScaleValue = log->mTicksPerSample * 15;
		log->mSampleIndex = 0;
		log->mLastOutputMask = 0;
		log->mNumMixedSamples = 0;

		RestartAudioLog(true);
	}
}

void ATPokeyRenderer::RestartAudioLog(bool initial) {
	if (!mpAudioLog)
		return;

	const uint32 t = ATSCHEDULER_GETTIME(mpScheduler);
	if (!initial) {
		// finish remaining samples in log for this frame (note that the current sample has already
		// been cleared and may be partially populated)
		LogOutputChange(t*2);
	}

	// restart sample buffer for new frame
	const uint32 t2 = t*2;
	mpAudioLog->mLastFrameSampleCount = mpAudioLog->mSampleIndex;
	mpAudioLog->mStartingAudioTick = t2;
	mpAudioLog->mLastAudioTick = t2;
	mpAudioLog->mAccumulatedAudioTicks = 0;
	mpAudioLog->mSampleIndex = 0;
}

void ATPokeyRenderer::SetFiltersEnabled(bool enable) {
	if (!enable)
		mHighPassAccum = 0;
}

void ATPokeyRenderer::SetInitMode(bool init) {
	if (init == mbInitMode)
		return;

	mbInitMode = init;

	QueueChangeEvent(ChangeType::Init, init ? 1 : 0);
}

bool ATPokeyRenderer::SetSpeaker(bool newState) {
	if (mbSpeakerState == newState)
		return false;

	mbSpeakerState = newState;

	// The XL/XE speaker is about as loud peak-to-peak as a channel at volume 6.
	// However, it is added in later in the output circuitry and has different
	// audio characteristics, so we must treat it separately.
	float delta = mpTables->mMixTable[6];

	if (newState)
		delta = -delta;

	const uint32 t = ATSCHEDULER_GETTIME(mpScheduler);
	mpEdgeBuffer->mEdges.push_back(ATSyncAudioEdge { t, delta });
	return true;
}

void ATPokeyRenderer::SetAudioLine2(int v) {
	if (mExternalInput != v) {
		const uint32 t = ATSCHEDULER_GETTIME(mpScheduler);

		mpEdgeBuffer->mEdges.push_back(ATSyncAudioEdge { t, (float)(v - mExternalInput) / (60.0f * 56.0f) });

		mExternalInput = v;
	}
}

void ATPokeyRenderer::ResetTimers() {
	QueueChangeEvent(ChangeType::ResetOutputs, 0);
}

void ATPokeyRenderer::SetAUDCx(int index, uint8 value) {
	if (mArchState.mAUDC[index] == value)
		return;

	mArchState.mAUDC[index] = value;

	QueueChangeEvent((ChangeType)((int)ChangeType::Audc0 + index), value);
}

void ATPokeyRenderer::SetAUDCTL(uint8 value) {
	if (mArchState.mAUDCTL == value)
		return;

	mArchState.mAUDCTL = value;

	QueueChangeEvent(ChangeType::Audctl, value);
}

void ATPokeyRenderer::AddChannelEvent(int channel) {
	ChannelEdges& ce = mChannelEdges[channel];
	const uint32 t = ATSCHEDULER_GETTIME(mpScheduler);

	VDASSERT(ce.size() == mChannelEdgeBases[channel] || t - ce.back() < 0x80000000);
	ce.push_back(t);
}

void ATPokeyRenderer::SetChannelDeferredEvents(int channel, uint32 start, uint32 period) {
	VDASSERT((uint32)(start*2 - mLastOutputTime2) < 0x80000000);
	VDASSERT(period < 7500000);

	DeferredEvent& ev = mDeferredEvents[channel];
	ev.mbEnabled = true;
	ev.mbLinked = false;
	ev.mNextTime = start;
	ev.mPeriod = period;
}

void ATPokeyRenderer::SetChannelDeferredEventsLinked(int channel, uint32 loStart, uint32 loPeriod, uint32 hiStart, uint32 hiPeriod, uint32 loOffset) {
	VDASSERT((uint32)(loStart*2 - mLastOutputTime2) < 0x80000000);
	VDASSERT(hiStart - loStart - 1 < 0x7FFFFFFFU);		// wrapped(hiStart > loStart)
	VDASSERT(loPeriod < 30000);
	VDASSERT(hiPeriod < 7500000);

	DeferredEvent& ev = mDeferredEvents[channel];
	ev.mbEnabled = true;
	ev.mbLinked = true;
	ev.mNextTime = loStart;
	ev.mPeriod = loPeriod;
	ev.mNextHiTime = hiStart;
	ev.mHiPeriod = hiPeriod;
	ev.mHiLoOffset = loOffset;
}

void ATPokeyRenderer::ClearChannelDeferredEvents(int channel, uint32 t) {
	if (!mDeferredEvents[channel].mbEnabled)
		return;

	FlushDeferredEvents(channel, t);
	mDeferredEvents[channel].mbEnabled = false;
}

void ATPokeyRenderer::AddSerialNoisePulse(uint32 t) {
	if (mSerialPulseTimes.size() > 65536)
		return;

	if (!mSerialPulseTimes.empty() && mSerialPulseTimes.back() - t < 0x80000000)
		return;

	mSerialPulseTimes.push_back(t);
}

ATPokeyRenderer::EndBlockInfo ATPokeyRenderer::EndBlock(IATSyncAudioEdgePlayer *edgePlayer) {
	uint32 t = ATSCHEDULER_GETTIME(mpScheduler);

	Flush(t);

	// copy mixed samples to the audio log
	if (mpAudioLog) {
		uint32 samplesToCopy = mpAudioLog->mMaxMixedSamples - mpAudioLog->mNumMixedSamples;

		if (samplesToCopy > mOutputSampleCount)
			samplesToCopy = mOutputSampleCount;

		if (samplesToCopy) {
			std::copy_n(mRawOutputBuffer, samplesToCopy, mpAudioLog->mpMixedSamples + mpAudioLog->mNumMixedSamples);
			mpAudioLog->mNumMixedSamples += samplesToCopy;
		}
	}

	// merge noise samples
	const uint32 sampleCount = mOutputSampleCount;

	if (!mSerialPulseTimes.empty()) {
		const uint32 baseTime = t - sampleCount * 28;
		float pulse = mSerialPulse;

		auto it = mSerialPulseTimes.begin();
		auto itEnd = mSerialPulseTimes.end();

		while(it != itEnd) {
			const uint32 pulseTime = *it;
			const uint32 rawOffset = pulseTime - baseTime;

			if (rawOffset < 0x80000000) {
				const uint32 sampleOffset = rawOffset / 28;

				if (sampleOffset >= sampleCount)
					break;

				mRawOutputBuffer[sampleOffset] += pulse;
				pulse = -pulse;
			}
		
			++it;
		}
	
		mSerialPulse = pulse;
		mSerialPulseTimes.erase(mSerialPulseTimes.begin(), it);
	}

	mOutputSampleCount = 0;

	const uint32 t2 = t*2;
	VDASSERT(t2 - mLastOutputSampleTime2 <= 56);

	// prevent denormals
	if (fabsf(mHighPassAccum) < 1e-20)
		mHighPassAccum = 0;

	if (edgePlayer)
		edgePlayer->AddEdgeBuffer(mpEdgeBuffer);
	else
		mpEdgeBuffer->mEdges.clear();

	return EndBlockInfo {
		t - ((t2 - mLastOutputSampleTime2) >> 1) - 28 * sampleCount,
		sampleCount
	};
}

void ATPokeyRenderer::LoadState(ATSaveStateReader& reader) {
	const uint32 t = ATSCHEDULER_GETTIME(mpScheduler);

	// Careful -- we save the polynomial counters in simulation time, but we
	// have to roll them back to where sound generation currently is.

	mPolyState.mPoly4Counter  = (reader.ReadUint8()  +     15 - (t - mPolyState.mLastPoly4Time) % 15) % 15;
	mPolyState.mPoly5Counter  = (reader.ReadUint8()  +     31 - (t - mPolyState.mLastPoly5Time) % 31) % 31;
	mPolyState.mPoly9Counter  = (reader.ReadUint16() +    511 - (t - mPolyState.mLastPoly9Time) % 511) % 511;
	mPolyState.mPoly17Counter = (reader.ReadUint32() + 131071 - (t - mPolyState.mLastPoly17Time) % 131071) % 131071;

	mNoiseFlipFlops = 0;
	for(int i=0; i<4; ++i)
		mNoiseFlipFlops += (reader.ReadUint8() & 1) << i;

	mChannelOutputMask = mNoiseFlipFlops;
	for(int i=0; i<2; ++i)
		mChannelOutputMask += (reader.ReadUint8() & 1) << (i + 4);

	// discard outputs (no longer needed -- high-pass XOR is done dynamically now)
	for(int i=0; i<4; ++i)
		reader.ReadUint8();
}

void ATPokeyRenderer::ResetState() {
	const uint32 t = ATSCHEDULER_GETTIME(mpScheduler);

	mPolyState.mPoly4Counter  = (    15 - (t - mPolyState.mLastPoly4Time ) % 15) % 15;
	mPolyState.mPoly5Counter  = (    31 - (t - mPolyState.mLastPoly5Time ) % 31) % 31;
	mPolyState.mPoly9Counter  = (   511 - (t - mPolyState.mLastPoly9Time ) % 511) % 511;
	mPolyState.mPoly17Counter = (131071 - (t - mPolyState.mLastPoly17Time) % 131071) % 131071;
	
	mNoiseFlipFlops = 0xF;
	mChannelOutputMask = 0x3F;
}

ATPokeyRenderer::SavedState ATPokeyRenderer::SaveState() const {
	const uint32 t = ATSCHEDULER_GETTIME(mpScheduler);
	SavedState state {};

	// Careful -- we can't update polynomial counters here like we do in the
	// main POKEY module. That's because the polynomial counters have to be
	// advanced by sound rendering and not by the simulation.

	state.mPoly4Offset = (mPolyState.mPoly4Counter  + (mPolyState.mInitMask & (mPolyState.mLastPoly4Time  - t))) % 15;
	state.mPoly5Offset = (mPolyState.mPoly5Counter  + (mPolyState.mInitMask & (mPolyState.mLastPoly5Time  - t))) % 31;
	state.mPoly9Offset = (mPolyState.mPoly9Counter  + (mPolyState.mInitMask & (mPolyState.mLastPoly9Time  - t))) % 511;
	state.mPoly17Offset = (mPolyState.mPoly17Counter + (mPolyState.mInitMask & (mPolyState.mLastPoly17Time - t))) % 131071;
	state.mOutputFlipFlops = mNoiseFlipFlops + (mChannelOutputMask & 0x30);

	// mbInitMode is restored by the POKEY emulator.
	// AUDCTL is restored by the POKEY emulator.
	// AUDCx are restored by the POKEY emulator.

	return state;
}

void ATPokeyRenderer::QueueChangeEvent(ChangeType type, uint8 value) {
	const uint32 t = ATSCHEDULER_GETTIME(mpScheduler);
	mChangeQueue.push_back(ChangeEvent { t, type, value });

	if (mChangeQueue.size() >= 40000)
		Flush(t);
}

void ATPokeyRenderer::ProcessChangeEvents(uint32 t) {
	bool outputChanged = false;

	while(!mChangeQueue.empty()) {
		const ChangeEvent& ce = mChangeQueue.front();

		if ((uint32)(t - ce.mTime) >= UINT32_C(0x80000000))
			break;

		switch(ce.mType) {
			case ChangeType::Audc0:
			case ChangeType::Audc1:
			case ChangeType::Audc2:
			case ChangeType::Audc3:
				mRenderState.mAUDC[(int)ce.mType - (int)ChangeType::Audc0] = ce.mValue;
				UpdateVolume((int)ce.mType - (int)ChangeType::Audc0);

				outputChanged = true;
				break;

			case ChangeType::Audctl:
				{
					const uint8 delta = mRenderState.mAUDCTL ^ ce.mValue;

					mRenderState.mAUDCTL = ce.mValue;

					if ((delta & 0x04) && !(mRenderState.mAUDCTL & 0x04)) {
						if (!(mChannelOutputMask & 0x10)) {
							mChannelOutputMask |= 0x10;
							outputChanged = true;
						}
					}

					if ((delta & 0x02) && !(mRenderState.mAUDCTL & 0x02)) {
						if (!(mChannelOutputMask & 0x20)) {
							mChannelOutputMask |= 0x20;
							outputChanged = true;
						}
					}
				}
				break;

			case ChangeType::Init:
				mPolyState.mInitMask = ce.mValue ? 0 : UINT32_C(0xFFFFFFFF);

				// These offsets are specifically set so that the audio output patterns
				// are correctly timed.
				mPolyState.mPoly4Counter = 8 - kAudioDelay;
				mPolyState.mPoly5Counter = 22 - kAudioDelay;
				mPolyState.mPoly9Counter = 507 - kAudioDelay;
				mPolyState.mPoly17Counter = 131067 - kAudioDelay;
				mPolyState.mLastPoly17Time = t;
				mPolyState.mLastPoly9Time = t;
				mPolyState.mLastPoly5Time = t;
				mPolyState.mLastPoly4Time = t;
				break;

			case ChangeType::ResetOutputs:
				// preset all noise flip/flops
				mNoiseFlipFlops = 0xF;
				mChannelOutputMask |= 0xF;
				outputChanged = true;
				break;

			case ChangeType::Flush:
				break;
		}

		mChangeQueue.pop_front();
	}

	if (outputChanged) {
		UpdateOutput(t);

		if (mpAudioLog)
			LogOutputChange(t*2);
	}
}

void ATPokeyRenderer::FlushDeferredEvents(int channel, uint32 t) {
	DeferredEvent de = mDeferredEvents[channel];

	VDASSERT(de.mNextTime*2 - mLastOutputTime2 < 0x80000000);

	ChannelEdges& ce = mChannelEdges[channel];

	VDASSERT(ce.size() == mChannelEdgeBases[channel] || de.mNextTime - ce.back() < 0x80000000);		// wrap(nextTime >= back) -> nextTime - back >= 0

	if (de.mbLinked) {
		while((sint32)(de.mNextTime - t) < 0) {
			ce.push_back(de.mNextTime);
			de.mNextTime += de.mPeriod;

			if ((sint32)(de.mNextTime - de.mNextHiTime) >= 0) {
				de.mNextTime = de.mNextHiTime + de.mHiLoOffset;
				de.mNextHiTime += de.mHiPeriod;
			}
		}
	} else {
		while((sint32)(de.mNextTime - t) < 0) {
			ce.push_back(de.mNextTime);
			de.mNextTime += de.mPeriod;
		}
	}

	mDeferredEvents[channel] = de;
}

void ATPokeyRenderer::Flush(const uint32 t) {
	mChangeQueue.push_back(ChangeEvent { t, ChangeType::Flush });

	while(!mChangeQueue.empty()) {
		const uint32 changeTime = mChangeQueue.front().mTime;

		while(mLastFlushTime != changeTime) {
			uint32 dt = changeTime - mLastFlushTime;
			if (dt)
				Flush2(mLastFlushTime + std::min<uint32>(dt, 0xC000));
		}

		if (mpAudioLog)
			LogOutputChange(changeTime*2);

		ProcessChangeEvents(changeTime);
	}

	GenerateSamples(t * 2);
}

void ATPokeyRenderer::Flush2(const uint32 t) {
	// flush deferred events
	bool haveAnyEvents = false;

	for(int i=0; i<4; ++i) {
		if (mDeferredEvents[i].mbEnabled)
			FlushDeferredEvents(i, t);

		if (mChannelEdgeBases[i] != mChannelEdges[i].size())
			haveAnyEvents = true;
	}

	// Check if the noise flip-flops are different from the output channel
	// mask for any channels that are not volume-only and have non-zero
	// volume; if there are any, update the mask and the current output
	// level. Any other channels that differ won't matter since their volume
	// is either being overridden or zero; we can update them later when
	// that changes.
	uint8 dirtyOutputs = (mChannelOutputMask ^ mNoiseFlipFlops) & ~mVolumeOnlyMask & mNonZeroVolumeMask;

	if (dirtyOutputs) {
		mChannelOutputMask ^= dirtyOutputs;
		UpdateOutput2(mLastOutputTime2);

		if (mpAudioLog)
			LogOutputChange(mLastOutputTime2);
	}

	const uint32 baseTime = mLastFlushTime;
	mLastFlushTime = t;

	g_ATLCPokeyTEv("=== processing %08X:%08X ===\n", baseTime, t);

	// early out if we have no events to process
	if (!haveAnyEvents)
		return;

	// realign polynomial tables to start of frame
	if (mbInitMode) {
		mPolyState.mPoly4Offset =
			mPolyState.mPoly5Offset =
			mPolyState.mPoly9Offset =
			mPolyState.mPoly17Offset = (uintptr)mpTables->mInitModeBuffer;
	} else {
		mPolyState.UpdatePoly4Counter(baseTime);
		mPolyState.UpdatePoly5Counter(baseTime);
		mPolyState.UpdatePoly9Counter(baseTime);
		mPolyState.UpdatePoly17Counter(baseTime);

		mPolyState.mPoly4Offset  = (uintptr)mpTables->mPolyBuffer + mPolyState.mPoly4Counter;
		mPolyState.mPoly5Offset  = (uintptr)mpTables->mPolyBuffer + mPolyState.mPoly5Counter;
		mPolyState.mPoly9Offset  = (uintptr)mpTables->mPolyBuffer + mPolyState.mPoly9Counter;
		mPolyState.mPoly17Offset = (uintptr)mpTables->mPolyBuffer + mPolyState.mPoly17Counter;
	}

	for(int i=0; i<4; ++i) {
		auto& srcEdges = mChannelEdges[i];
		uint32 srcBegin = mChannelEdgeBases[i];
		uint32 srcEnd = srcEdges.size();

		// We should not have any edges in the future. We may have some edges slightly in the
		// past since we keep a couple of cycles back to delay the audio output.
		if (srcBegin != srcEnd) {
			VDASSERT(ATWrapTime{srcEdges[srcBegin]} >= baseTime - kAudioDelay);
		}

		auto& dstEdges = mSortedEdgesTemp[i];
		const uint32 numEdges = srcEnd - srcBegin;

		dstEdges.resize(numEdges + 1);

		uint32 *dst = dstEdges.data();

		srcEdges.push_back(baseTime ^ UINT32_C(0x80000000));

		const uint32 * const VDRESTRICT src = srcEdges.data() + srcBegin;
		auto [dst2, src2] = (this->*GetFireTimerRoutine(i))(dst, src, baseTime - kAudioDelay, t - baseTime);
		srcEdges.pop_back();

		uint32 numSrcEdgesInRange = (uint32)(src2 - src);
		uint32 numAudioEdges = numSrcEdgesInRange;
		
		while(numAudioEdges && ATWrapTime{src[numAudioEdges - 1]} >= t - kAudioDelay)
			--numAudioEdges;

		*dst2++ = UINT32_C(0xFFFFFFFF);

		dstEdges.resize((size_t)(dst2 - dst));

		if (i >= 2) {
			// Merge in events to update the high-pass filter. If high-pass audio mode is enabled for
			// ch1/2, we need to insert all of the events from ch3/4; otherwise, we only need the last
			// event, if any.
			uint32 hpClockStart = 0;
			uint32 hpClockEnd = numSrcEdgesInRange;

			// push ending bound forward to encompass events that may have been skipped by the main
			// audio update due to audio delay, but which are in scope for the high-pass since HP
			// processing runs two cycles ahead
			while(hpClockEnd != numEdges && ATWrapTime{src[hpClockEnd]} < t)
				++hpClockEnd;

			// push starting bound forward, for same reason
			while(hpClockStart != hpClockEnd && ATWrapTime{src[hpClockStart]} < baseTime)
				++hpClockStart;

			if (hpClockStart != hpClockEnd) {
				const bool hpEnabled = (mRenderState.mAUDCTL & (4 >> (i - 2))) != 0;

				if (hpEnabled) {
					auto& hpTargetEdges = mSortedEdgesTemp[i - 2];

					// high-pass is enabled -- offset events by audio delay, convert to HP update events, and merge
					// into ch1/2 list
					const uint32 numHpEvents = hpClockEnd - hpClockStart;

					mSortedEdgesHpTemp1.resize(numHpEvents + 1);

					// Compute update offset: +1 half cycle, clear high-pass bit, and rebias time.
					//
					// Add one half cycle to the high pass update so it's a half cycle earlier than
					// the output flip/flop. On real hardware, HP never updates at the same time;
					// it's either one half clock earlier or late, so there is no phase offset at
					// which high pass is fully effective or ineffective.

					const uint32 hpUpdateCoding = 0x4000 + 0x3F00 - (0x400 << i) + (4 << i) - (baseTime << 15);

					{
						uint32 *VDRESTRICT hpDest = mSortedEdgesHpTemp1.data();
						for(uint32 j=0; j<numHpEvents; ++j) {
							const uint32 evTime = src[hpClockStart + j];

							hpDest[j] = (evTime << 15) + hpUpdateCoding;
						}
					}

					mSortedEdgesHpTemp1.back() = 0xFFFFFFFF;
					mSortedEdgesHpTemp2.resize(hpTargetEdges.size() + numHpEvents);

					MergeOutputEvents(hpTargetEdges.data(), mSortedEdgesHpTemp1.data(), mSortedEdgesHpTemp2.data());

					hpTargetEdges.swap(mSortedEdgesHpTemp2);
					hpTargetEdges.back() = 0xFFFFFFFF;
				}
			}
		}

		mChannelEdgeBases[i] += numAudioEdges;

		if (mChannelEdgeBases[i] >= 16 && mChannelEdgeBases[i]*4 >= srcEnd) {
			srcEdges.erase(srcEdges.begin(), srcEdges.begin() + mChannelEdgeBases[i]);
			mChannelEdgeBases[i] = 0;
		}
	}

	const uint32 n0 = (uint32)mSortedEdgesTemp[0].size() - 1;
	const uint32 n1 = (uint32)mSortedEdgesTemp[1].size() - 1;
	const uint32 n2 = (uint32)mSortedEdgesTemp[2].size() - 1;
	const uint32 n3 = (uint32)mSortedEdgesTemp[3].size() - 1;
	const uint32 n01 = n0 + n1;
	const uint32 n23 = n2 + n3;
	const uint32 n = n01 + n23;

	mSortedEdgesTemp2[0].resize(n01 + 1);
	mSortedEdgesTemp2[1].resize(n23 + 1);

	if (n01) {
		MergeOutputEvents(mSortedEdgesTemp[0].data(),
			mSortedEdgesTemp[1].data(),
			mSortedEdgesTemp2[0].data());
	}

	mSortedEdgesTemp2[0].back() = 0xFFFFFFFF;

	if (n23) {
		MergeOutputEvents(mSortedEdgesTemp[2].data(),
			mSortedEdgesTemp[3].data(),
			mSortedEdgesTemp2[1].data());
	}

	mSortedEdgesTemp2[1].back() = 0xFFFFFFFF;

	// The merge order here is critical -- we need channels 3 and 4 to update before
	// 1 and 2 in case high pass mode is enabled, because if 1+3 or 2+4 fire at the
	// same time, the high pass is updated with the output state from the last cycle.
	mSortedEdges.resize(n + 1);

	MergeOutputEvents(mSortedEdgesTemp2[1].data(),
		mSortedEdgesTemp2[0].data(),
		mSortedEdges.data());

	if (g_ATLCPokeyTEv.IsEnabled()) {
		for(uint32 i=0; i<n; ++i) {
			const uint32 edge = mSortedEdges[i];

			g_ATLCPokeyTEv("%08X.%c:%u\n", (edge >> 15) + baseTime, edge & 0x4000 ? '5' : '0', VDFindLowestSetBitFast(~(edge >> 8) & 63));
		}
	}

	// if we have logging, we must log the edges before ProcessOutputEdges() updates the channel output state
	if (mpAudioLog)
		LogOutputEdges(baseTime*2, mSortedEdges.data(), n);

	ProcessOutputEdges(baseTime, mSortedEdges.data(), n);
	mSortedEdges.clear();
}

void ATPokeyRenderer::MergeOutputEvents(const uint32 *VDRESTRICT src1, const uint32 *VDRESTRICT src2, uint32 *VDRESTRICT dst) {
	uint32 a = *src1++;
	uint32 b = *src2++;

	for(;;) {
		if (b < a) {
			*dst++ = b;
			b = *src2++;
			continue;
		}

		if (a < b) {
			*dst++ = a;
			a = *src1++;
			continue;
		}

		if (a == 0xFFFFFFFF)
			break;

		*dst++ = a;
		a = *src1++;
	}
}

template<int activeChannel, bool T_UsePoly9>
const ATPokeyRenderer::FireTimerRoutine ATPokeyRenderer::kFireRoutines[2][16]={
	// What we are trying to do here is minimize the amount of work done
	// in the FireTimer() routine, in two ways: precompile code paths with
	// specific functions enabled, and identify when the resultant signal
	// does not change.
	//
	// AUDCx bit 7 controls clock selection and isn't tied to anything else,
	//             so it always needs to go through.
	//
	// AUDCx bit 5 enables pure tone mode. When set, it overrides bit 6.
	//             Therefore, we map [6:5] = 11 to 01.
	//
	// AUDCx bit 6 chooses the 4-bit LFSR or the 9/17-bit LFSR. It is
	//             overridden in the table as noted above.
	//
	// AUDCx bit 4 controls volume only mode. When it is set, we must still
	//             update the internal flip-flop states, but the volume
	//             level is locked and can't affect the output.
	//
	// We also check the volume on the channel. If it is zero, then the
	// output also doesn't affect the volume and therefore we can skip the
	// flush in that case as well.
	//
	// High-pass mode throws a wrench into the works. In that case, a pair
	// of channels are tied together and we have to be careful about what
	// optimizations are applied. We need to check volumes on both channels,
	// and the high channel can cause signal changes if the low channel is
	// un-muted even if the high channel is muted.
	//
	// The control value $00 is especially important as it is the init state
	// used by the OS to silence the audio channels, and thus it should run
	// quickly. It is annoying to us since it is an LFSR-based mode rather
	// than volume level.

	{
		&ATPokeyRenderer::FireTimer<activeChannel, 0x00, false, T_UsePoly9>,	// poly5 + poly9/17
		&ATPokeyRenderer::FireTimer<activeChannel, 0x00, false, T_UsePoly9>,	// poly5 + poly9/17
		&ATPokeyRenderer::FireTimer<activeChannel, 0x20, false, false>,			// poly5 + tone
		&ATPokeyRenderer::FireTimer<activeChannel, 0x20, false, false>,			// poly5 + tone
		&ATPokeyRenderer::FireTimer<activeChannel, 0x40, false, false>,			// poly5 + poly4
		&ATPokeyRenderer::FireTimer<activeChannel, 0x40, false, false>,			// poly5 + poly4
		&ATPokeyRenderer::FireTimer<activeChannel, 0x20, false, false>,			// poly5 + tone
		&ATPokeyRenderer::FireTimer<activeChannel, 0x20, false, false>,			// poly5 + tone
		&ATPokeyRenderer::FireTimer<activeChannel, 0x80, false, T_UsePoly9>,	// poly9/17
		&ATPokeyRenderer::FireTimer<activeChannel, 0x80, false, T_UsePoly9>,	// poly9/17
		&ATPokeyRenderer::FireTimer<activeChannel, 0xA0, false, false>,			// tone
		&ATPokeyRenderer::FireTimer<activeChannel, 0xA0, false, false>,			// tone
		&ATPokeyRenderer::FireTimer<activeChannel, 0xC0, false, false>,			// poly4
		&ATPokeyRenderer::FireTimer<activeChannel, 0xC0, false, false>,			// poly4
		&ATPokeyRenderer::FireTimer<activeChannel, 0xA0, false, false>,			// tone
		&ATPokeyRenderer::FireTimer<activeChannel, 0xA0, false, false>,			// tone
	},
	{
		&ATPokeyRenderer::FireTimer<activeChannel, 0x00, true , T_UsePoly9>,
		&ATPokeyRenderer::FireTimer<activeChannel, 0x00, false, T_UsePoly9>,
		&ATPokeyRenderer::FireTimer<activeChannel, 0x20, true , false>,
		&ATPokeyRenderer::FireTimer<activeChannel, 0x20, false, false>,
		&ATPokeyRenderer::FireTimer<activeChannel, 0x40, true , false>,
		&ATPokeyRenderer::FireTimer<activeChannel, 0x40, false, false>,
		&ATPokeyRenderer::FireTimer<activeChannel, 0x20, true , false>,
		&ATPokeyRenderer::FireTimer<activeChannel, 0x20, false, false>,
		&ATPokeyRenderer::FireTimer<activeChannel, 0x80, true , T_UsePoly9>,
		&ATPokeyRenderer::FireTimer<activeChannel, 0x80, false, T_UsePoly9>,
		&ATPokeyRenderer::FireTimer<activeChannel, 0xA0, true , false>,
		&ATPokeyRenderer::FireTimer<activeChannel, 0xA0, false, false>,
		&ATPokeyRenderer::FireTimer<activeChannel, 0xC0, true , false>,
		&ATPokeyRenderer::FireTimer<activeChannel, 0xC0, false, false>,
		&ATPokeyRenderer::FireTimer<activeChannel, 0xA0, true , false>,
		&ATPokeyRenderer::FireTimer<activeChannel, 0xA0, false, false>,
	},
};

ATPokeyRenderer::FireTimerRoutine ATPokeyRenderer::GetFireTimerRoutine(int ch) const {
	switch(ch) {
		case 0: return GetFireTimerRoutine<0>();
		case 1: return GetFireTimerRoutine<1>();
		case 2: return GetFireTimerRoutine<2>();
		case 3: return GetFireTimerRoutine<3>();

		default:
			return nullptr;
	}
}

template<int activeChannel>
ATPokeyRenderer::FireTimerRoutine ATPokeyRenderer::GetFireTimerRoutine() const {
	static_assert(activeChannel >= 0 && activeChannel < 4);

	// If high pass is enabled we must generate output events even if volume
	// is zero so we can latch the noise flip/flop into the high-pass flip/flop.
	// We need both the low channel and the high channel.
	const bool highPassEnabled = (mRenderState.mAUDCTL & (activeChannel & 1 ? 0x02 : 0x04)) != 0;
	const bool nonZeroVolume = mChannelVolume[activeChannel] || highPassEnabled;

	using FireTimerTable = FireTimerRoutine[2][16];
	const FireTimerTable *tab[2] = {
		&kFireRoutines<activeChannel, false>,
		&kFireRoutines<activeChannel, true>,
	};
	const bool usePoly9 = (mRenderState.mAUDCTL & 0x80) != 0;

	return (*tab[usePoly9])[nonZeroVolume][mRenderState.mAUDC[activeChannel] >> 4];
}

template<int activeChannel, uint8 audcn, bool outputAffectsSignal, bool T_UsePoly9>
std::pair<uint32 *, const uint32 *> ATPokeyRenderer::FireTimer(uint32 *VDRESTRICT dst, const uint32 *VDRESTRICT src, uint32 timeBase, uint32 timeLimit) {
	static constexpr bool noiseEnabled = !(audcn & 0x20);
	static constexpr bool poly5Enabled = !(audcn & 0x80);
	static constexpr int polyOffset = 3 - activeChannel;

	PolyState polyState = mPolyState;

	const uint8 channelBit = (1 << activeChannel);
	uint32 noiseFF = (mNoiseFlipFlops & channelBit) ? 1 : 0;

	const uint32 baseMaskCode = 0x3F00 - (0x100 << activeChannel);
	uint32 currentMaskCode = baseMaskCode + (noiseFF ? channelBit : 0);

	// These aren't pointers because they aren't guaranteed to be within the array until offset
	// by the time offset.
	const uintptr poly4Base = polyState.mPoly4Offset + polyOffset;
	const uintptr poly5Base = polyState.mPoly5Offset + polyOffset;
	const uintptr poly9Base = polyState.mPoly9Offset + polyOffset;
	const uintptr poly17Base = polyState.mPoly17Offset + polyOffset;

	const uint32 masks[2] = {
		baseMaskCode,
		baseMaskCode + channelBit
	};

	for(;;) {
		const uint32 timeOffset = (*src) - timeBase;

		if (timeOffset >= timeLimit)
			break;

		++src;

		if constexpr (poly5Enabled) {
			uint8 poly5 = *(const uint8 *)(poly5Base + timeOffset);

			if (!(poly5 & 4))
				continue;
		}
		
		if constexpr (noiseEnabled) {
			uint32 noiseFFInput;

			if constexpr ((audcn & 0x40) != 0) {
				const uint32 poly4 = *(const uint8 *)(poly4Base + timeOffset);
				noiseFFInput = (poly4 & 8) >> 3;
			} else if constexpr (T_UsePoly9) {
				const uint32 poly9 = *(const uint8 *)(poly9Base + timeOffset);
				noiseFFInput = (poly9 & 2) >> 1;
			} else {
				const uint32 poly17 = *(const uint8 *)(poly17Base + timeOffset);
				noiseFFInput = (poly17 & 1);
			}

			if constexpr (outputAffectsSignal) {
				// Because we are using noise in this path, using a branch would be fairly expensive
				// as it is highly likely to mispredict due to the random branch -- since we're
				// literally driving it from a psuedorandom noise generator. Use a branchless
				// algorithm instead.

				const uint32 outputChanged = noiseFF ^ noiseFFInput;
				noiseFF = noiseFFInput;

				*dst = (timeOffset << 15) + masks[noiseFF];
				dst += outputChanged;
			} else {
				noiseFF = noiseFFInput;
			}
		} else {
			// Noise isn't enabled -- hardcode some stuff since VC++ isn't able to
			// deduce that toggling a bit will always cause the changed check to pass.
			noiseFF ^= 1;

			if constexpr (outputAffectsSignal) {
				currentMaskCode ^= channelBit;

				// Update normal audio on full cycles; we reserve the half-cycle for high-pass update.
				*dst++ = (timeOffset << 15) + currentMaskCode;
			}
		}
	}

	if (noiseFF)
		mNoiseFlipFlops |= channelBit;
	else
		mNoiseFlipFlops &= ~channelBit;

	return { dst, src };
}

void ATPokeyRenderer::ProcessOutputEdges(uint32 timeBase, const uint32 *edges, uint32 n) {
#if VD_CPU_X86 || VD_CPU_X64
	union {
		uint8 b[64];
		uint32 d[16];
		__m128i v[4];
	} v;

	// VC++ auto-vectorizer does a poor job on the below, so we'll have to do it
	// ourselves.

	const uint8 vol0 = mChannelVolume[0];
	const uint8 vol1 = mChannelVolume[1];
	const uint8 vol2 = mChannelVolume[2];
	const uint8 vol3 = mChannelVolume[3];

	const uint32 d01
		= (uint32)vol0 * (mVolumeOnlyMask & 1 ? 0x01010101 : VDFromLE32(0x01000100))
		+ (uint32)vol1 * (mVolumeOnlyMask & 2 ? 0x01010101 : VDFromLE32(0x01010000));

	const uint32 d2 = (uint32)vol2 * 0x01010101;
	const uint32 d3 = (uint32)vol3 * 0x01010101;

	__m128i v01 = _mm_shuffle_epi32(_mm_cvtsi32_si128(d01), 0);
	__m128i v2 = _mm_shuffle_epi32(_mm_cvtsi32_si128(d2), 0x11);
	__m128i v3 = _mm_shuffle_epi32(_mm_cvtsi32_si128(d3), 0x05);

	if (mVolumeOnlyMask & 4)
		v2 = _mm_shuffle_epi32(v2, 0xFF);

	if (mVolumeOnlyMask & 8)
		v3 = _mm_shuffle_epi32(v3, 0xFF);

	__m128i v0123 = _mm_add_epi8(_mm_add_epi8(v01, v2), v3);

	v.v[0] = v0123;
	v.v[1] = _mm_or_si128(_mm_slli_epi16(v0123, 8), _mm_srli_epi16(v0123, 8));
	v.v[2] = _mm_shufflehi_epi16(_mm_shufflelo_epi16(v0123, 0xB1), 0xB1);
	v.v[3] = _mm_or_si128(_mm_slli_epi16(v.v[2], 8), _mm_srli_epi16(v.v[2], 8));

#else
	const uint8 v0 = mChannelVolume[0];
	const uint8 v1 = mChannelVolume[1];
	const uint8 v2 = mChannelVolume[2];
	const uint8 v3 = mChannelVolume[3];

	union {
		uint8 b[64];
		uint32 d[16];
	} v;

	v.d[0]  = (uint32)v0 * (mVolumeOnlyMask & 1 ? 0x01010101 : VDFromLE32(0x01000100));
	v.d[0] += (uint32)v1 * (mVolumeOnlyMask & 2 ? 0x01010101 : VDFromLE32(0x01010000));

	const uint32 d2 = (uint32)v2 * 0x01010101;
	const uint32 d3 = (uint32)v3 * 0x01010101;

	v.d[1] = v.d[0] + d2;
	if (mVolumeOnlyMask & 4)
		v.d[0] += d2;

	v.d[2] = v.d[0] + d3;
	v.d[3] = v.d[1] + d3;

	if (mVolumeOnlyMask & 8) {
		v.d[0] += d3;
		v.d[1] += d3;
	}

	// permute for high-pass XOR states
	{
		const uint32 * VDRESTRICT hp0 = &v.d[0];
		uint32 * VDRESTRICT hp1 = &v.d[4];
		uint32 * VDRESTRICT hp2 = &v.d[8];
		uint32 * VDRESTRICT hp3 = &v.d[12];

		for(int i=0; i<4; ++i)
			hp1[i] = ((hp0[i] & 0xFF00FF00) >> 8) + ((hp0[i] & 0x00FF00FF) << 8);

		for(int i=0; i<4; ++i)
			hp2[i] = ((hp0[i] & 0xFFFF0000) >> 16) + ((hp0[i] & 0x0000FFFF) << 16);

		for(int i=0; i<4; ++i)
			hp3[i] = ((hp1[i] & 0xFFFF0000) >> 16) + ((hp1[i] & 0x0000FFFF) << 16);
	}
#endif

	uint32 timeBase2 = timeBase * 2;
	uint8 outputMask = mChannelOutputMask;

	while(n--) {
		const uint32 code = *edges++;
		const uint32 t2 = timeBase2 + (code >> 14);

		// apply AND mask to clear the bits we're about to update
		outputMask &= (uint8)((code >> 8) & 0x3F); 

		// update channel and high-pass flip flops
		outputMask += (uint8)(code & ((outputMask << 4) + 15));

		UpdateOutput2(t2, v.b[outputMask]);
	}

	mChannelOutputMask = outputMask;
}

void ATPokeyRenderer::UpdateVolume(int index) {
	mChannelVolume[index] = mbChannelEnabled[index] ? mRenderState.mAUDC[index] & 15 : 0;

	if (mRenderState.mAUDC[index] & 0x10)
		mVolumeOnlyMask |= (1 << index);
	else
		mVolumeOnlyMask &= ~(1 << index);

	if (mRenderState.mAUDC[index] & 0x0F)
		mNonZeroVolumeMask |= (1 << index);
	else
		mNonZeroVolumeMask &= ~(1 << index);
}

void ATPokeyRenderer::UpdateOutput(uint32 t) {
	UpdateOutput2(t * 2);
}

void ATPokeyRenderer::UpdateOutput2(uint32 t2) {
	uint8 outputMask = (mChannelOutputMask ^ (mChannelOutputMask >> 4)) | mVolumeOnlyMask;

	int v0 = mChannelVolume[0];
	int v1 = mChannelVolume[1];
	int v2 = mChannelVolume[2];
	int v3 = mChannelVolume[3];
	int vpok	= ((outputMask & 0x01) ? v0 : 0)
				+ ((outputMask & 0x02) ? v1 : 0)
				+ ((outputMask & 0x04) ? v2 : 0)
				+ ((outputMask & 0x08) ? v3 : 0);

	UpdateOutput2(t2, vpok);
}

void ATPokeyRenderer::UpdateOutput2(uint32 t2, int vpok) {
	VDASSERT(t2 - mLastOutputTime2 < 0x80000000);

	GenerateSamples(t2);

	int oc = t2 - mLastOutputTime2;
	mAccum += mOutputLevel * (float)oc;

	mLastOutputTime2 = t2;

	VDASSERT(t2 - mLastOutputSampleTime2 <= 56);

	mOutputLevel	= mpTables->mMixTable[vpok];
}

void ATPokeyRenderer::GenerateSamples(uint32 t2) {
	sint32 delta = t2 - mLastOutputSampleTime2;

	if (!delta)
		return;

	const float vmin = mpTables->mReferenceClampLo;
	const float vmax = mpTables->mReferenceClampHi;

	if (delta >= 56) {
		mLastOutputSampleTime2 += 56;

		int oc = mLastOutputSampleTime2 - mLastOutputTime2;
		VDASSERT((unsigned)oc <= 56);

		mAccum += mOutputLevel * (float)oc;

		mLastOutputTime2 = mLastOutputSampleTime2;

		float delta = mAccum - mHighPassAccum;
		mHighPassAccum += delta * mpTables->mReferenceDecayPerSample;

		if (delta < vmin)
			delta = vmin;
		if (delta > vmax)
			delta = vmax;

		float v = delta;

		mAccum = 0;
		mRawOutputBuffer[mOutputSampleCount] = v;

		if (++mOutputSampleCount >= kBufferSize) {
			mOutputSampleCount = kBufferSize - 1;

			while((t2 - mLastOutputSampleTime2) >= 56)
				mLastOutputSampleTime2 += 56;

			mLastOutputTime2 = mLastOutputSampleTime2;
			return;
		}
	}

	if ((t2 - mLastOutputSampleTime2) < 56)
		return;

	const float coeff2 = mpTables->mReferenceDecayPerSample;
	const float v1 = mOutputLevel * 56.0f;
	mAccum = 0;

	auto hpAccum = mHighPassAccum;
	auto outputCount = mOutputSampleCount;
	auto lastTime2 = mLastOutputSampleTime2;

	while((t2 - lastTime2) >= 56) {
		lastTime2 += 56;

		float delta = v1 - hpAccum;
		hpAccum += delta * coeff2;

		if (delta < vmin)
			delta = vmin;
		if (delta > vmax)
			delta = vmax;

		mRawOutputBuffer[outputCount] = delta;

		if (++outputCount >= kBufferSize) {
			outputCount = kBufferSize - 1;

			VDASSERT(t2 - lastTime2 < 56000000);

			while((t2 - lastTime2) >= 56)
				lastTime2 += 56;
			break;
		}
	}

	mHighPassAccum = hpAccum;
	mOutputSampleCount = outputCount;
	mLastOutputSampleTime2 = lastTime2;

	mLastOutputTime2 = mLastOutputSampleTime2;

	VDASSERT(t2 - mLastOutputSampleTime2 <= 56);
}

void ATPokeyRenderer::LogOutputChange(uint32 t2) const {
	const uint32 nullEdge = 0x3F00;
	LogOutputEdges(t2, &nullEdge, 1);
}

void ATPokeyRenderer::LogOutputEdges(uint32 timeBase2, const uint32 *edges, uint32 n) const {
	if (mpAudioLog->mSampleIndex >= mpAudioLog->mMaxSamples)
		return;

	VDASSERT(ATWrapTime{timeBase2} >= mpAudioLog->mLastAudioTick);

	const uint8 v0 = mChannelVolume[0];
	const uint8 v1 = mChannelVolume[1];
	const uint8 v2 = mChannelVolume[2];
	const uint8 v3 = mChannelVolume[3];

	uint8 outputMask = mChannelOutputMask;
	uint8 lastOutputMask = mpAudioLog->mLastOutputMask;
	uint32 tickAccum = mpAudioLog->mAccumulatedAudioTicks;
	uint32 lastTick = mpAudioLog->mLastAudioTick;
	uint32 sampleIndex = mpAudioLog->mSampleIndex;
	const uint32 maxSamples = mpAudioLog->mMaxSamples;
	const uint32 ticksPerSample = mpAudioLog->mTicksPerSample;
	ATPokeyAudioState *VDRESTRICT samplePtr = &mpAudioLog->mpStates[sampleIndex];

	while(n--) {
		const uint32 code = *edges++;
		const uint32 t2 = timeBase2 + (code >> 14);

		// apply AND mask to clear the bits we're about to update
		outputMask &= (uint8)((code >> 8) & 0x3F); 
		outputMask += (uint8)(code & ((outputMask << 4) + 15));

		uint32 dt = t2 - lastTick;

		// we need to generate samples up to the current point with the _last_ output
		// mask, since the new value is for past this edge
		int ch0 = lastOutputMask & 1 ? v0 : 0;
		int ch1 = lastOutputMask & 2 ? v1 : 0;
		int ch2 = lastOutputMask & 4 ? v2 : 0;
		int ch3 = lastOutputMask & 8 ? v3 : 0;
		lastOutputMask = (outputMask ^ (outputMask >> 4)) | mVolumeOnlyMask;

		VDASSERT((lastTick - mpAudioLog->mStartingAudioTick) - (sampleIndex * mpAudioLog->mTicksPerSample) < mpAudioLog->mTicksPerSample);

		if ((uint32)(dt - 1) < UINT32_C(0x80000000)) {
			lastTick = t2;

			do {
				uint32 sampleTicks = ticksPerSample - tickAccum;

				if (sampleTicks > dt)
					sampleTicks = dt;

				if (tickAccum == 0)
					*samplePtr = {};

				tickAccum += sampleTicks;
				dt -= sampleTicks;

				samplePtr->mChannelOutputs[0] += ch0 * sampleTicks;
				samplePtr->mChannelOutputs[1] += ch1 * sampleTicks;
				samplePtr->mChannelOutputs[2] += ch2 * sampleTicks;
				samplePtr->mChannelOutputs[3] += ch3 * sampleTicks;

				if (tickAccum == ticksPerSample) {
					tickAccum = 0;

					++samplePtr;

					if (++sampleIndex >= maxSamples)
						goto log_full;
				}
			} while(dt);
		}
	}

log_full:
	mpAudioLog->mAccumulatedAudioTicks = tickAccum;
	mpAudioLog->mLastAudioTick = lastTick;
	mpAudioLog->mSampleIndex = sampleIndex;
	mpAudioLog->mLastOutputMask = lastOutputMask;
}

void ATPokeyRenderer::PolyState::UpdatePoly17Counter(uint32 t) {
	int polyDelta = t - mLastPoly17Time;
	mPoly17Counter += polyDelta & mInitMask;
	mLastPoly17Time = t;

	if (mPoly17Counter >= 131071)
		mPoly17Counter %= 131071;
}

void ATPokeyRenderer::PolyState::UpdatePoly9Counter(uint32 t) {
	int polyDelta = t - mLastPoly9Time;
	mPoly9Counter += polyDelta & mInitMask;
	mLastPoly9Time = t;

	if (mPoly9Counter >= 511)
		mPoly9Counter %= 511;
}

void ATPokeyRenderer::PolyState::UpdatePoly5Counter(uint32 t) {
	int polyDelta = t - mLastPoly5Time;
	mPoly5Counter += polyDelta & mInitMask;
	mLastPoly5Time = t;

	if (mPoly5Counter >= 31)
		mPoly5Counter %= 31;
}

void ATPokeyRenderer::PolyState::UpdatePoly4Counter(uint32 t) {
	int polyDelta = t - mLastPoly4Time;
	VDASSERT(polyDelta >= 0);
	mPoly4Counter += polyDelta & mInitMask;
	mLastPoly4Time = t;

	if (mPoly4Counter >= 15)
		mPoly4Counter %= 15;
}
