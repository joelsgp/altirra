//	Altirra - Atari 800/800XL/5200 emulator
//	Copyright (C) 2008-2012 Avery Lee
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
#include "audiosyncmixer.h"
#include "oshelper.h"
#include "resource.h"

struct ATAudioSyncMixer::SoundPred {
	SoundPred(uint32 t) : mTimeBase(t + 0x80000000U) {}

	bool operator()(const Sound& x, const Sound& y) const {
		return (x.mStartTime - mTimeBase) < (y.mStartTime - mTimeBase);
	}

	uint32 mTimeBase;
};

ATAudioSyncMixer::ATAudioSyncMixer()
	: mNextSoundId(1)
{
}

ATAudioSyncMixer::~ATAudioSyncMixer() {
}

void ATAudioSyncMixer::Init(ATScheduler *sch) {
	mpScheduler = sch;

	static const uint32 kResIds[]={
		IDR_DISK_SPIN,
		IDR_TRACK_STEP,
		IDR_TRACK_STEP_2,
		IDR_TRACK_STEP_2,
		IDR_TRACK_STEP_3
	};

	static const float kBaseVolumes[]={
		0.05f,
		0.4f,
		0.8f,
		0.8f,
		0.4f
	};

	static_assert(vdcountof(kResIds) == vdcountof(mSamples), "Sample array mismatch");
	static_assert(vdcountof(kBaseVolumes) == vdcountof(mSamples), "Sample array mismatch");

	vdfastvector<uint8> data;
	for(size_t i=0; i<vdcountof(kResIds); ++i) {
		if (i + 1 == kATAudioSampleId_DiskStep2H) {
			// special case
			auto samp = mSamples[kATAudioSampleId_DiskStep2 - 1];
			samp.mLength >>= 1;
			mSamples[i] = samp;
		} else {
			ATLoadMiscResource(kResIds[i], data);

			const size_t len = data.size() * sizeof(data[0]);
			sint16 *p = (sint16 *)mAllocator.Allocate(len);
			memcpy(p, data.data(), len);

			mSamples[i] = { p, (uint32)(len / sizeof(sint16)), kBaseVolumes[i] };
		}
	}
}

void ATAudioSyncMixer::Shutdown() {
	mpScheduler = NULL;
}

uint32 ATAudioSyncMixer::AddSound(ATAudioMix mix, uint32 delay, ATAudioSampleId sampleId, float volume) {
	const uint32 index = (uint32)((uint32)sampleId - 1);
	
	if (index >= vdcountof(mSamples))
		return 0;

	const auto& sample = mSamples[index];
	return AddSound(mix, delay, sample.mpData, sample.mLength, sample.mBaseVolume * volume);
}

uint32 ATAudioSyncMixer::AddLoopingSound(ATAudioMix mix, uint32 delay, ATAudioSampleId sampleId, float volume) {
	const uint32 index = (uint32)((uint32)sampleId - 1);
	
	if (index >= vdcountof(mSamples))
		return 0;

	const auto& sample = mSamples[index];
	return AddLoopingSound(mix, delay, sample.mpData, sample.mLength, sample.mBaseVolume * volume);
}

uint32 ATAudioSyncMixer::AddSound(ATAudioMix mix, uint32 delay, const sint16 *sample, uint32 len, float volume) {
	const uint32 t = ATSCHEDULER_GETTIME(mpScheduler) + delay;

	if (mFreeSounds.empty())
		mFreeSounds.push_back(mAllocator.Allocate<Sound>());

	Sound *s = mFreeSounds.back();
	mFreeSounds.pop_back();
	
	s->mId = mNextSoundId;
	mNextSoundId += 2;

	s->mStartTime = t;
	s->mLoopPeriod = kATCyclesPerSyncSample * len;
	s->mEndTime = t + s->mLoopPeriod;
	s->mLength = len;
	s->mVolume = volume * (60.0f * 28.0f / 32767.0f);
	s->mMix = mix;
	s->mpSample = sample;
	s->mbEndValid = true;

	mSounds.push_back(s);
	return s->mId;
}

uint32 ATAudioSyncMixer::AddLoopingSound(ATAudioMix mix, uint32 delay, const sint16 *sample, uint32 len, float volume) {
	const uint32 t = ATSCHEDULER_GETTIME(mpScheduler) + delay;

	if (mFreeSounds.empty())
		mFreeSounds.push_back(mAllocator.Allocate<Sound>());

	Sound *s = mFreeSounds.back();
	mFreeSounds.pop_back();

	s->mId = mNextSoundId;
	mNextSoundId += 2;

	s->mStartTime = t;
	s->mLoopPeriod = kATCyclesPerSyncSample * len;
	s->mEndTime = t + s->mLoopPeriod;
	s->mLength = len;
	s->mVolume = volume * (60.0f * 28.0f / 32767.0f);
	s->mMix = mix;
	s->mpSample = sample;
	s->mbEndValid = false;

	mSounds.push_back(s);
	return s->mId;
}

void ATAudioSyncMixer::StopSound(uint32 id) {
	auto it = mSounds.begin(), itEnd = mSounds.end();

	for(; it != itEnd; ++it) {
		Sound *s = *it;
		if (s->mId != id)
			continue;

		mFreeSounds.push_back(s);

		*it = mSounds.back();
		mSounds.pop_back();
		return;
	}
}

void ATAudioSyncMixer::StopSound(uint32 id, uint32 time) {
	auto it = mSounds.begin(), itEnd = mSounds.end();

	for(; it != itEnd; ++it) {
		Sound *s = *it;

		if (s->mId != id)
			continue;

		// check if we're killing the sound before it starts
		if ((sint32)(time - s->mStartTime) <= 0) {
			mFreeSounds.push_back(s);
			*it = mSounds.back();
			mSounds.pop_back();
			return;
		}

		// check if we're trying to kill a one-shot after it would already end
		if (s->mbEndValid && (sint32)(time - s->mEndTime) >= 0)
			return;

		// mark new end time and exit
		s->mEndTime = time;
		s->mbEndValid = true;
		return;
	}
}

void ATAudioSyncMixer::WriteAudio(const ATSyncAudioMixInfo& mixInfo) {
	uint32 startTime = mixInfo.mStartTime;
	float *dstLeft = mixInfo.mpLeft;
	float *dstRightOpt = mixInfo.mpRight;
	uint32 n = mixInfo.mCount;

	const uint32 endTime = startTime + n*kATCyclesPerSyncSample;

	// process one-shot sounds
	auto it = mSounds.begin(), itEnd = mSounds.end();
	while(it != itEnd) {
		Sound *s = *it;

		// drop sounds that we've already passed
		if ((sint32)(s->mEndTime - startTime) <= 0 && s->mbEndValid) {
			mFreeSounds.push_back(s);
			*it = mSounds.back();
			mSounds.pop_back();
			itEnd = mSounds.end();
			continue;
		}
		
		++it;

		// skip if sound hasn't happened yet
		if ((sint32)(s->mStartTime - endTime) >= 0)
			continue;

		// check if we're being asked to truncate the sample
		uint32 len = s->mLength;

		if (s->mbEndValid && s->mEndTime - s->mStartTime < s->mLoopPeriod)
			len = (s->mEndTime - s->mStartTime) / (sint32)kATCyclesPerSyncSample;

		// check for the sample starting behind the current window
		sint32 dstoffset = (sint32)(s->mStartTime - startTime) / (sint32)kATCyclesPerSyncSample;
		const sint16 *VDRESTRICT src = s->mpSample;

		if (dstoffset < 0) {
			uint32 shift = (uint32)-dstoffset;

			if (shift >= len)
				continue;

			src += shift;
			len -= shift;
			dstoffset = 0;
		}

		// check if the sample extends beyond the current window
		bool completed = true;

		if (len + dstoffset > n) {
			len = n - dstoffset;
			completed = false;
		}

		// add samples
		const float vol = s->mVolume * mixInfo.mpMixLevels[s->mMix];
		float *VDRESTRICT dl = dstLeft + dstoffset;
		if (dstRightOpt) {
			float *VDRESTRICT dr = dstRightOpt + dstoffset;

			while(len--) {
				const float sample = (float)*src++ * vol;
				*dl++ += sample;
				*dr++ += sample;
			}
		} else {
			while(len--) {
				const float sample = (float)*src++ * vol;
				*dl++ += sample;
			}
		}

		if (completed)
			s->mStartTime += s->mLoopPeriod;
	}
}
