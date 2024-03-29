#ifndef f_AT_AUDIOFILTERS_H
#define f_AT_AUDIOFILTERS_H

#include <math.h>

extern "C" VDALIGN(16) const sint16 gATAudioResamplingKernel63To44[65][64];

uint64 ATFilterResampleMono16(sint16 *d, const float *s, uint32 count, uint64 accum, sint64 inc);
uint64 ATFilterResampleMonoToStereo16(sint16 *d, const float *s, uint32 count, uint64 accum, sint64 inc);
uint64 ATFilterResampleStereo16(sint16 *d, const float *s1, const float *s2, uint32 count, uint64 accum, sint64 inc);

void ATFilterComputeSymmetricFIR_8_32F(float *dst, const float *src, size_t n, const float *kernel);
void ATFilterComputeSymmetricFIR_8_32F(float *dst, size_t n, const float *kernel);

class ATAudioFilter {
public:
	enum { kFilterOverlap = 8 };

	ATAudioFilter();

	void CopyState(const ATAudioFilter& src) {
		mHiPassAccum = src.mHiPassAccum;
	}

	bool CloseTo(const ATAudioFilter& src, float threshold) {
		return fabsf(src.mHiPassAccum - mHiPassAccum) < threshold && fabsf(src.mDiffHistory - mDiffHistory) < threshold;
	}

	float GetScale() const;
	void SetScale(float scale);

	void SetActiveMode(bool active);

	void PreFilter(float * VDRESTRICT dst, uint32 count, float dcLevel);

	// Run a high-pass filter on adjacent differences rather than normal samples. This takes advantage
	// of Direct Form I by cancelling the adjacent difference operation on the input with a running sum
	// operation of a previous stage; this avoids the need to expand edges to pulses. PreFilter1()
	// is the optional pre-differencing step, while PreFilter2() is the integration step.
	void PreFilterDiff(float * VDRESTRICT dst, uint32 count);
	void PreFilterEdges(float * VDRESTRICT dst, uint32 count, float dcLevel);

	void Filter(float *dst, const float *src, uint32 count);
	void Filter(float *dst, uint32 count);

protected:
	float	mHiPassAccum = 0;
	float	mHiCoeff = 0;
	float	mScale = 0;
	float	mDiffHistory = 0;

	float	mLoPassCoeffs[kFilterOverlap];
};

#endif
