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

#include <stdafx.h>
#include "pokey.h"
#include "pokeyrenderer.h"
#include "pokeytables.h"
#include "scheduler.h"

ATPokeyRenderer::ATPokeyRenderer()
	: mpScheduler(NULL)
	, mpTables(NULL)
	, mAccum(0)
	, mOutputLevel(0)
	, mLastOutputTime(0)
	, mLastOutputSampleTime(0)
	, mAudioInput2(0)
	, mExternalInput(0)
	, mbSpeakerState(false)
	, mAUDCTL(0)
	, mLastPoly17Time(0)
	, mLastPoly9Time(0)
	, mLastPoly5Time(0)
	, mLastPoly4Time(0)
	, mPoly17Counter(0)
	, mPoly9Counter(0)
	, mPoly5Counter(0)
	, mPoly4Counter(0)
	, mOutputSampleCount(0)
{
	for(int i=0; i<4; ++i) {
		mbChannelEnabled[i] = true;
		mChannelVolume[i] = 0;
		mAUDC[i] = 0;
	}
}

ATPokeyRenderer::~ATPokeyRenderer() {
}

void ATPokeyRenderer::Init(ATScheduler *sch, ATPokeyTables *tables) {
	mpScheduler = sch;
	mpTables = tables;

	mLastOutputTime = ATSCHEDULER_GETTIME(mpScheduler);
	mLastOutputSampleTime = mLastOutputTime;

	ColdReset();
}

void ATPokeyRenderer::ColdReset() {
	mbInitMode = true;

	for(int i=0; i<4; ++i) {
		mOutputs[i] = 1;
		mNoiseFF[i] = 0;

		mDeferredEvents[i].mbEnabled = false;
	}

	mOutputLevel = 0;

	const uint32 t = ATSCHEDULER_GETTIME(mpScheduler);
	mLastPoly17Time = t;
	mLastPoly9Time = t;
	mLastPoly5Time = t;
	mLastPoly4Time = t;
	mPoly17Counter = 0;
	mPoly9Counter = 0;
	mPoly5Counter = 0;
	mPoly4Counter = 0;

	// This must be done after everything else is inited, as it will start recomputing
	// derived values.
	SetAUDCTL(0);

	for(int i=0; i<4; ++i)
		SetAUDCx(i, 0);
}

void ATPokeyRenderer::GetAudioState(ATPokeyAudioState& state) {
	Flush(ATSCHEDULER_GETTIME(mpScheduler));

	for(int ch=0; ch<4; ++ch) {
		int level = mChannelVolume[ch];

		if (!(mAUDC[ch] & 0x10))
			level *= mOutputs[ch];

		state.mChannelOutputs[ch] = level;
	}
}

void ATPokeyRenderer::SetChannelEnabled(int channel, bool enabled) {
	VDASSERT(channel < 4);
	if (mbChannelEnabled[channel] != enabled) {
		mbChannelEnabled[channel] = enabled;

		UpdateVolume(channel);
		UpdateOutput();
	}
}

void ATPokeyRenderer::SetInitMode(bool init) {
	if (init == mbInitMode)
		return;

	const uint32 t = ATSCHEDULER_GETTIME(mpScheduler);
	Flush(t);

	mbInitMode = init;

	mPoly4Counter = 0;
	mPoly5Counter = 0;
	mPoly9Counter = 0;
	mPoly17Counter = 0;
	mLastPoly17Time = t + 1;
	mLastPoly9Time = t + 1;
	mLastPoly5Time = t + 1;
	mLastPoly4Time = t + 1;
}

bool ATPokeyRenderer::SetSpeaker(bool newState) {
	if (mbSpeakerState == newState)
		return false;

	mbSpeakerState = newState;
	UpdateOutput();
	return true;
}

void ATPokeyRenderer::SetAudioLine2(int v) {
	mAudioInput2 = v;
	mExternalInput = mAudioInput2;
	UpdateOutput();
}

void ATPokeyRenderer::ResetTimers() {
	const uint32 t = ATSCHEDULER_GETTIME(mpScheduler);
	Flush(t);
	GenerateSamples(t);
	mOutputs[0] = 0;
	mOutputs[1] = 0;
	mOutputs[2] = 1;
	mOutputs[3] = 1;

	UpdateOutput(t);
	mNoiseFF[0] = 1;
	mNoiseFF[1] = 1;
	mNoiseFF[2] = 1;
	mNoiseFF[3] = 1;
}

void ATPokeyRenderer::SetAUDCx(int index, uint8 value) {
	const uint32 t = ATSCHEDULER_GETTIME(mpScheduler);
	Flush(t);

	mAUDC[index] = value;

	UpdateVolume(index);
	UpdateOutput();
}

void ATPokeyRenderer::SetAUDCTL(uint8 value) {
	mAUDCTL = value;

	bool outputsChanged = false;
	if (!(mAUDCTL & 0x04) && !mHighPassFF[0]) {
		mHighPassFF[0] = true;
		mOutputs[0] = mNoiseFF[0] ^ 0x01;
		if (mChannelVolume[0])
			outputsChanged = true;
	}

	if (!(mAUDCTL & 0x02) && !mHighPassFF[1]) {
		mHighPassFF[1] = true;
		mOutputs[1] = mNoiseFF[1] ^ 0x01;
		if (mChannelVolume[1])
			outputsChanged = true;
	}

	if (outputsChanged)
		UpdateOutput();
}

void ATPokeyRenderer::AddChannelEvent(int channel) {
	ChannelEdges& ce = mChannelEdges[channel];
	const uint32 t = ATSCHEDULER_GETTIME(mpScheduler);

	VDASSERT(ce.empty() || t >= ce.back());
	ce.push_back(t);
}

void ATPokeyRenderer::SetChannelDeferredEvents(int channel, uint32 start, uint32 period) {
	VDASSERT(start >= mLastOutputTime);
	VDASSERT(period < 7500000);

	DeferredEvent& ev = mDeferredEvents[channel];
	ev.mbEnabled = true;
	ev.mbLinked = false;
	ev.mNextTime = start;
	ev.mPeriod = period;
}

void ATPokeyRenderer::SetChannelDeferredEventsLinked(int channel, uint32 loStart, uint32 loPeriod, uint32 hiStart, uint32 hiPeriod, uint32 loOffset) {
	VDASSERT(loStart >= mLastOutputTime);
	VDASSERT(hiStart > loStart);
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

uint32 ATPokeyRenderer::EndBlock() {
	static uint32 lastFlush = 0;
	uint32 startingSamples = mOutputSampleCount;
	uint32 startingSampleTime = mLastOutputSampleTime;

	uint32 t = ATSCHEDULER_GETTIME(mpScheduler);

	Flush(t);
	GenerateSamples(t);

	uint32 sampleCount = mOutputSampleCount;

	mOutputSampleCount = 0;

	lastFlush = t;

	VDASSERT(t - mLastOutputSampleTime <= 28);

	return sampleCount;
}

void ATPokeyRenderer::FlushDeferredEvents(int channel, uint32 t) {
	DeferredEvent de = mDeferredEvents[channel];

	VDASSERT(de.mNextTime >= mLastOutputTime);

	ChannelEdges& ce = mChannelEdges[channel];

	VDASSERT(ce.empty() || de.mNextTime >= ce.back());

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
	for(int i=0; i<4; ++i) {
		if (mDeferredEvents[i].mbEnabled)
			FlushDeferredEvents(i, t);
	}

	const uint32 n0 = mChannelEdges[0].size();
	const uint32 n1 = mChannelEdges[1].size();
	const uint32 n2 = mChannelEdges[2].size();
	const uint32 n3 = mChannelEdges[3].size();
	const uint32 n01 = n0 + n1;
	const uint32 n23 = n2 + n3;

	if (!(n01 + n23))
		return;

	// We apply a bias to all of the timestamps to ensure that the wraparound isn't in the way.
	const uint32 timeSortOffset = t + 2;

	mSortedEdgesTemp2[0].resize(n01 + 1);
	mSortedEdgesTemp2[1].resize(n23 + 1);
	mSortedEdges.resize(n01 + n23);

	for(int i=0; i<4; ++i) {
		mSortedEdgesTemp[i].resize(mChannelEdges[i].size() + 1);
		Edge *dst = mSortedEdgesTemp[i].data();

		for(ChannelEdges::const_iterator it(mChannelEdges[i].begin()), itEnd(mChannelEdges[i].end());
			it != itEnd;
			++it)
		{
			const uint32 t = *it;

			dst->channel = i;
			dst->t = t - timeSortOffset;
			++dst;
		}

		// add sentinel
		dst->t = 0xFFFFFFFF;

		mChannelEdges[i].clear();
	}

	// The VC++ STL has a bug where it will fire bogus assertions on a null output pointer
	// even if the source ranges are empty.
	if (n01) {
		MergeEvents(mSortedEdgesTemp[0].data(),
			mSortedEdgesTemp[1].data(),
			mSortedEdgesTemp2[0].data());
	}

	mSortedEdgesTemp2[0].back().t = 0xFFFFFFFF;

	if (n23) {
		MergeEvents(mSortedEdgesTemp[2].data(),
			mSortedEdgesTemp[3].data(),
			mSortedEdgesTemp2[1].data());
	}

	mSortedEdgesTemp2[1].back().t = 0xFFFFFFFF;

	MergeEvents(mSortedEdgesTemp2[0].data(),
		mSortedEdgesTemp2[1].data(),
		mSortedEdges.data());

	VDASSERT((sint32)(mSortedEdges.back().t + timeSortOffset - t) <= 0);
	VDASSERT((sint32)(mSortedEdges.front().t + timeSortOffset - mLastOutputTime) >= 0);

	SortedEdges::iterator it(mSortedEdges.begin()), itEnd(mSortedEdges.end());

	FireTimerRoutine routines[4];
	routines[0] = GetFireTimerRoutine<0>();
	routines[1] = GetFireTimerRoutine<1>();
	routines[2] = GetFireTimerRoutine<2>();
	routines[3] = GetFireTimerRoutine<3>();

	for(; it != itEnd; ++it)
	{
		const Edge& edge = *it;

		(this->*routines[edge.channel])(edge.t + timeSortOffset);
	}

	mSortedEdges.clear();
}

void ATPokeyRenderer::MergeEvents(const Edge *src1, const Edge *src2, Edge *dst) {
	Edge a = *src1++;
	Edge b = *src2++;

	for(;;) {
		if (b.t < a.t) {
			*dst++ = b;
			b = *src2++;
			continue;
		}

		if (a.t < b.t) {
			*dst++ = a;
			a = *src1++;
			continue;
		}

		if (a.t == 0xFFFFFFFF)
			break;

		*dst++ = a;
		a = *src1++;
	}
}

template<int activeChannel>
ATPokeyRenderer::FireTimerRoutine ATPokeyRenderer::GetFireTimerRoutine() const {
	static const FireTimerRoutine kRoutines[2][2][16]={
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
			{
				&ATPokeyRenderer::FireTimer<activeChannel, 0x00, false, false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x00, false, false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x20, false, false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x20, false, false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x40, false, false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x40, false, false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x20, false, false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x20, false, false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x80, false, false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x80, false, false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0xA0, false, false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0xA0, false, false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0xC0, false, false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0xC0, false, false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0xA0, false, false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0xA0, false, false>,
			},
			{
				&ATPokeyRenderer::FireTimer<activeChannel, 0x00, true , false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x00, false, false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x20, true , false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x20, false, false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x40, true , false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x40, false, false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x20, true , false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x20, false, false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x80, true , false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x80, false, false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0xA0, true , false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0xA0, false, false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0xC0, true , false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0xC0, false, false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0xA0, true , false>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0xA0, false, false>,
			},
		},
		{
			{
				&ATPokeyRenderer::FireTimer<activeChannel, 0x00, false, true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x00, false, true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x20, false, true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x20, false, true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x40, false, true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x40, false, true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x20, false, true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x20, false, true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x80, false, true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x80, false, true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0xA0, false, true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0xA0, false, true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0xC0, false, true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0xC0, false, true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0xA0, false, true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0xA0, false, true>,
			},
			{
				&ATPokeyRenderer::FireTimer<activeChannel, 0x00, true , true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x00, true , true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x20, true , true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x20, true , true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x40, true , true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x40, true , true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x20, true , true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x20, true , true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x80, true , true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0x80, true , true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0xA0, true , true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0xA0, true , true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0xC0, true , true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0xC0, true , true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0xA0, true , true>,
				&ATPokeyRenderer::FireTimer<activeChannel, 0xA0, true , true>,
			}
		},
	};

	const bool highPassEnabled = activeChannel & 1 ? (mAUDCTL & 0x02) != 0 : (mAUDCTL & 0x04) != 0;
	const bool nonZeroVolume = mChannelVolume[activeChannel] || (highPassEnabled && mChannelVolume[activeChannel & 1]);

	return kRoutines[highPassEnabled][nonZeroVolume][mAUDC[activeChannel] >> 4];
}

template<int activeChannel, uint8 audcn, bool outputAffectsSignal, bool highPassEnabled>
void ATPokeyRenderer::FireTimer(uint32 t) {
	bool outputsChanged = false;

	if ((audcn & 0xa0) != 0xa0) {
		const uint8 polyBit = 8 >> activeChannel;

		if (!(audcn & 0x80)) {
			UpdatePoly5Counter(t);
			uint8 poly5 = mpTables->mPoly5Buffer[mPoly5Counter];
			if (!(poly5 & polyBit)) {
				// This is a bit troublesome. If the high pass filter is enabled, ch1/2
				// update their high pass flip-flops on every ch3/4 tick regardless of whether
				// the 5-bit filter is enabled.
				if (highPassEnabled)
					goto update_high_pass;

				return;
			}
		}

		uint8 noiseFFInput = mNoiseFF[activeChannel] ^ 1;

		if (!(audcn & 0x20)) {
			if (audcn & 0x40) {
				UpdatePoly4Counter(t);
				uint8 poly4 = mpTables->mPoly4Buffer[mPoly4Counter];
				noiseFFInput = (poly4 & polyBit) != 0;
			} else if (mAUDCTL & 0x80) {
				UpdatePoly9Counter(t);
				uint8 poly9 = mpTables->mPoly9Buffer[mPoly9Counter];
				noiseFFInput = (poly9 & polyBit) != 0;
			} else {
				UpdatePoly17Counter(t);
				uint8 poly17 = mpTables->mPoly17Buffer[mPoly17Counter];
				noiseFFInput = (poly17 & polyBit) != 0;
			}
		}

		mNoiseFF[activeChannel] = noiseFFInput;
	} else {
		mNoiseFF[activeChannel] ^= 1;
	}

	mOutputs[activeChannel] = mNoiseFF[activeChannel];

	if (activeChannel == 0) {
		if (highPassEnabled)
			mOutputs[0] ^= mHighPassFF[0];
		else
			mOutputs[0] ^= 0x01;
	}

	// count timer 2
	if (activeChannel == 1) {
		if (highPassEnabled)
			mOutputs[1] ^= mHighPassFF[1];
		else
			mOutputs[1] ^= 0x01;
	}

	if (!highPassEnabled || mChannelVolume[activeChannel])
		outputsChanged = true;

update_high_pass:

	// count timer 3
	if (activeChannel == 2 && highPassEnabled) {
		mHighPassFF[0] = mNoiseFF[0];

		if (mOutputs[0]) {
			mOutputs[0] = 0;

			if (mChannelVolume[0])
				outputsChanged = true;
		}
	}

	// count timer 4
	if (activeChannel == 3 && highPassEnabled) {
		mHighPassFF[1] = mNoiseFF[1];

		if (mOutputs[1]) {
			mOutputs[1] = 0;

			if (mChannelVolume[1])
				outputsChanged = true;
		}
	}

	if (outputAffectsSignal && outputsChanged)
		UpdateOutput(t);
}

void ATPokeyRenderer::UpdateVolume(int index) {
	mChannelVolume[index] = mbChannelEnabled[index] ? mAUDC[index] & 15 : 0;
}

void ATPokeyRenderer::UpdateOutput() {
	const uint32 t = ATSCHEDULER_GETTIME(mpScheduler);
	Flush(t);
	UpdateOutput(t);
}

void ATPokeyRenderer::UpdateOutput(uint32 t) {
	VDASSERT(t >= mLastOutputTime);

	GenerateSamples(t);

	int oc = t - mLastOutputTime;
	mAccum += mOutputLevel * oc;
	mLastOutputTime = t;

	VDASSERT(t - mLastOutputSampleTime <= 28);

	int out0 = mOutputs[0];
	int out1 = mOutputs[1];
	int out2 = mOutputs[2];
	int out3 = mOutputs[3];

	int v0 = mChannelVolume[0];
	int v1 = mChannelVolume[1];
	int v2 = mChannelVolume[2];
	int v3 = mChannelVolume[3];
	int vpok	= ((mAUDC[0] & 0x10) ? v0 : out0 * v0)
				+ ((mAUDC[1] & 0x10) ? v1 : out1 * v1)
				+ ((mAUDC[2] & 0x10) ? v2 : out2 * v2)
				+ ((mAUDC[3] & 0x10) ? v3 : out3 * v3);

	mOutputLevel	= mpTables->mMixTable[vpok] 
					+ mExternalInput
					+ (mbSpeakerState ? +24 : 0);		// The XL speaker is approximately 80% as loud as a full volume channel.
}

void ATPokeyRenderer::GenerateSamples(uint32 t) {
	sint32 delta = t - mLastOutputSampleTime;

	if (!delta)
		return;

	if (delta >= 28) {
		mLastOutputSampleTime += 28;

		int oc = mLastOutputSampleTime - mLastOutputTime;
		VDASSERT((unsigned)oc <= 28);
		mAccum += mOutputLevel * oc;
		mLastOutputTime = mLastOutputSampleTime;

		float v = mAccum;

		mAccum = 0;
		mRawOutputBuffer[mOutputSampleCount] = (float)v;

		if (++mOutputSampleCount >= kBufferSize) {
			mOutputSampleCount = kBufferSize - 1;

			while((t - mLastOutputSampleTime) >= 28)
				mLastOutputSampleTime += 28;
			return;
		}
	}

	if ((t - mLastOutputSampleTime) < 28)
		return;

	const float v = mOutputLevel * 28;
	mAccum = 0;

	while((t - mLastOutputSampleTime) >= 28) {
		mLastOutputSampleTime += 28;

		mRawOutputBuffer[mOutputSampleCount] = v;

		if (++mOutputSampleCount >= kBufferSize) {
			mOutputSampleCount = kBufferSize - 1;

			VDASSERT(t - mLastOutputSampleTime < 28000000);

			while((t - mLastOutputSampleTime) >= 28)
				mLastOutputSampleTime += 28;
			break;
		}
	}

	mLastOutputTime = mLastOutputSampleTime;

	VDASSERT(t - mLastOutputSampleTime <= 28);
}

void ATPokeyRenderer::UpdatePoly17Counter(uint32 t) {
	if (mbInitMode)
		return;

	int polyDelta = t - mLastPoly17Time;
	mPoly17Counter += polyDelta;
	mLastPoly17Time = t;

	if (mPoly17Counter >= 131071)
		mPoly17Counter %= 131071;
}

void ATPokeyRenderer::UpdatePoly9Counter(uint32 t) {
	if (mbInitMode)
		return;

	int polyDelta = t - mLastPoly9Time;
	mPoly9Counter += polyDelta;
	mLastPoly9Time = t;

	if (mPoly9Counter >= 511)
		mPoly9Counter %= 511;
}

void ATPokeyRenderer::UpdatePoly5Counter(uint32 t) {
	if (mbInitMode)
		return;

	int polyDelta = t - mLastPoly5Time;
	mPoly5Counter += polyDelta;
	mLastPoly5Time = t;

	if (mPoly5Counter >= 31)
		mPoly5Counter %= 31;
}

void ATPokeyRenderer::UpdatePoly4Counter(uint32 t) {
	if (mbInitMode)
		return;

	int polyDelta = t - mLastPoly4Time;
	mPoly4Counter += polyDelta;
	mLastPoly4Time = t;

	if (mPoly4Counter >= 15)
		mPoly4Counter %= 15;
}
