//	Altirra - Atari 800/800XL/5200 emulator
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

#include <stdafx.h>
#include <numeric>
#include <vd2/system/binary.h>
#include <vd2/system/bitmath.h>
#include <vd2/Kasumi/pixmap.h>
#include <vd2/Kasumi/pixmapops.h>
#include <vd2/Kasumi/pixmaputils.h>
#include <vd2/Tessa/Context.h>
#include <vd2/VDDisplay/internal/screenfx.h>
#include "bicubic.h"
#include "displaynode3d.h"
#include "image_shader.inl"

extern const VDTDataView g_VDDispVPView_RenderFillLinear(g_VDDispVP_RenderFillLinear);
extern const VDTDataView g_VDDispVPView_RenderFillGamma(g_VDDispVP_RenderFillGamma);
extern const VDTDataView g_VDDispVPView_RenderBlitLinear(g_VDDispVP_RenderBlitLinear);
extern const VDTDataView g_VDDispVPView_RenderBlitLinearColor(g_VDDispVP_RenderBlitLinearColor);
extern const VDTDataView g_VDDispVPView_RenderBlitLinearColor2(g_VDDispVP_RenderBlitLinearColor2);
extern const VDTDataView g_VDDispVPView_RenderBlitGamma(g_VDDispVP_RenderBlitGamma);
extern const VDTDataView g_VDDispFPView_RenderFill(g_VDDispFP_RenderFill);
extern const VDTDataView g_VDDispFPView_RenderFillLinearToGamma(g_VDDispFP_RenderFillLinearToGamma);
extern const VDTDataView g_VDDispFPView_RenderBlit(g_VDDispFP_RenderBlit);
extern const VDTDataView g_VDDispFPView_RenderBlitLinear(g_VDDispFP_RenderBlitLinear);
extern const VDTDataView g_VDDispFPView_RenderBlitLinearToGamma(g_VDDispFP_RenderBlitLinearToGamma);
extern const VDTDataView g_VDDispFPView_RenderBlitDirect(g_VDDispFP_RenderBlitDirect);
extern const VDTDataView g_VDDispFPView_RenderBlitStencil(g_VDDispFP_RenderBlitStencil);
extern const VDTDataView g_VDDispFPView_RenderBlitColor(g_VDDispFP_RenderBlitColor);
extern const VDTDataView g_VDDispFPView_RenderBlitColor2(g_VDDispFP_RenderBlitColor2);
extern const VDTDataView g_VDDispFPView_RenderBlitColorLinear(g_VDDispFP_RenderBlitColorLinear);

///////////////////////////////////////////////////////////////////////////

void VDDisplayMeshBuilder3D::SetVertexFormat(IVDTVertexFormat *vf) {
	mpCmd->mpVF = vf;
}

void VDDisplayMeshBuilder3D::SetVertexProgram(IVDTVertexProgram *vp) {
	mpCmd->mpVP = vp;
	if (!mpCmd->mpVP)
		mpPool->mbError = true;
}

void VDDisplayMeshBuilder3D::SetVertexProgram(VDTData vpdata) {
	SetVertexProgram(mpDctx->InitVP(vpdata));
}

void VDDisplayMeshBuilder3D::SetFragmentProgram(IVDTFragmentProgram *fp) {
	mpCmd->mpFP = fp;
	if (!mpCmd->mpFP)
		mpPool->mbError = true;
}

void VDDisplayMeshBuilder3D::SetFragmentProgram(VDTData fpdata) {
	SetFragmentProgram(mpDctx->InitFP(fpdata));
}

void VDDisplayMeshBuilder3D::SetVPConstData(const void *src, size_t len) {
	VDASSERT(!mpCmd->mVPConstCount);
	VDASSERT(!(len & 15));

	mpCmd->mVPConstCount = len >> 4;
	mpCmd->mVPConstOffset = (uint32)mpPool->mConstData.size();

	if (len) {
		mpPool->mConstData.resize(mpCmd->mVPConstOffset + len, 0);
		memcpy(&*(mpPool->mConstData.end() - len), src, len);
	}
}

void VDDisplayMeshBuilder3D::SetVPConstDataReuse() {
	mpCmd->mVPConstCount = 0;
	mpCmd->mVPConstOffset = 1;
}

void VDDisplayMeshBuilder3D::SetFPConstData(const void *src, size_t len) {
	VDASSERT(!mpCmd->mFPConstCount);
	VDASSERT(!(len & 15));

	mpCmd->mFPConstCount = len >> 4;
	mpCmd->mFPConstOffset = (uint32)mpPool->mConstData.size();

	if (len) {
		mpPool->mConstData.resize(mpCmd->mFPConstOffset + len, 0);
		memcpy(&*(mpPool->mConstData.end() - len), src, len);
	}
}

void VDDisplayMeshBuilder3D::SetFPConstDataReuse() {
	mpCmd->mFPConstCount = 0;
	mpCmd->mFPConstOffset = 1;
}

void *VDDisplayMeshBuilder3D::InitVertices(size_t vertexSize, uint32 vertexCount, VDDVertexTransformer vertexTransformer) {
	VDASSERT(!mpCmd->mVertexSourceOffset);
	VDASSERT(!mpCmd->mVertexCount);
	VDASSERT(vertexCount);
	VDASSERT(!(vertexSize & 3));

	mpCmd->mVertexSourceOffset = (uint32)mpPool->mVertexData.size();
	mpCmd->mVertexSourceLen = vertexCount * vertexSize;
	mpCmd->mVertexCount = vertexCount;
	mpCmd->mVertexSize = vertexSize;
	mpCmd->mVertexTransformer = vertexTransformer;

	mpPool->mVertexData.resize(mpCmd->mVertexSourceOffset + mpCmd->mVertexSourceLen);
	return &*(mpPool->mVertexData.end() - mpCmd->mVertexSourceLen);
}

void VDDisplayMeshBuilder3D::SetVertices(const void *data, size_t vertexSize, size_t vertexCount, VDDVertexTransformer vertexTransformer) {
	memcpy(InitVertices(vertexSize, vertexCount, vertexTransformer), data, vertexSize * vertexCount);
}

void VDDisplayMeshBuilder3D::SetTopology(const uint16 *indices, uint32 numTriangles) {
	VDASSERT(numTriangles);

	mpCmd->mIndexSourceCount = numTriangles * 3;
	mpCmd->mIndexSourceOffset = (uint32)mpPool->mIndexData.size();

	mpPool->mIndexData.resize(mpCmd->mIndexSourceOffset + mpCmd->mIndexSourceCount);
	memcpy(&*(mpPool->mIndexData.end() - mpCmd->mIndexSourceCount), indices, sizeof(uint16) * mpCmd->mIndexSourceCount);
}

void VDDisplayMeshBuilder3D::SetTopologyImm(std::initializer_list<uint16> indices) {
	VDASSERT(indices.size() % 3 == 0);
	SetTopology(&*indices.begin(), (uint32)(indices.size() / 3));
}

void VDDisplayMeshBuilder3D::SetTopologyQuad() {
	SetTopologyImm({0, 1, 2, 2, 1, 3});
}

void VDDisplayMeshBuilder3D::InitTextures(uint32 numTextures) {
	VDASSERT(!mpCmd->mTextureCount);

	mpCmd->mTextureStart = 0;
	mpCmd->mTextureCount = numTextures;

	if (numTextures) {
		mpCmd->mTextureStart = (uint32)mpPool->mTextureIndices.size();
		mpPool->mTextureIndices.resize(mpCmd->mTextureStart + numTextures, VDDPoolTextureIndex(0));
	}
}

void VDDisplayMeshBuilder3D::InitTextures(std::initializer_list<VDDPoolTextureIndex> textures) {
	VDASSERT(!mpCmd->mTextureCount);

	mpCmd->mTextureStart = 0;
	mpCmd->mTextureCount = (uint32)textures.size();

	if (mpCmd->mTextureCount) {
		mpCmd->mTextureStart = (uint32)mpPool->mTextureIndices.size();
		mpPool->mTextureIndices.insert(mpPool->mTextureIndices.end(), textures.begin(), textures.end());
	}
}

void VDDisplayMeshBuilder3D::SetTexture(uint32 textureSlot, VDDPoolTextureIndex poolTextureIndex) {
	VDASSERT(textureSlot < mpCmd->mTextureCount);
	mpPool->mTextureIndices[mpCmd->mTextureStart + textureSlot] = poolTextureIndex;
}

void VDDisplayMeshBuilder3D::InitSamplers(uint32 numSamplers) {
	VDASSERT(!mpCmd->mSamplerCount);
	mpCmd->mSamplerStart = (uint32)mpPool->mpSamplers.size();
	mpCmd->mSamplerCount = numSamplers;

	mpPool->mpSamplers.resize(mpCmd->mSamplerStart + numSamplers, nullptr);
}

void VDDisplayMeshBuilder3D::InitSamplers(std::initializer_list<IVDTSamplerState *> samplers) {
	VDASSERT(!mpCmd->mSamplerCount);
	mpCmd->mSamplerStart = (uint32)mpPool->mpSamplers.size();
	mpCmd->mSamplerCount = (uint32)samplers.size();

	mpPool->mpSamplers.insert(mpPool->mpSamplers.end(), samplers.begin(), samplers.end());
}

void VDDisplayMeshBuilder3D::SetSampler(uint32 samplerSlot, IVDTSamplerState *ss) {
	VDASSERT(samplerSlot < mpCmd->mSamplerCount);
	mpPool->mpSamplers[mpCmd->mSamplerStart + samplerSlot] = ss;
}

void VDDisplayMeshBuilder3D::SetClear(uint32 clearColor) {
	mpCmd->mbRenderClear = true;
	mpCmd->mRenderClearColor = clearColor;
}

void VDDisplayMeshBuilder3D::SetRenderView(VDDPoolRenderViewId id) {
	mpCmd->mRenderView = id;
}

VDDPoolRenderViewId VDDisplayMeshBuilder3D::SetRenderView(const VDDRenderView& renderView) {
	VDASSERT(renderView.mViewport.mX >= 0);
	VDASSERT(renderView.mViewport.mY >= 0);

	mpCmd->mRenderView = mpPool->RegisterRenderView(renderView);
	return mpCmd->mRenderView;
}

VDDPoolRenderViewId VDDisplayMeshBuilder3D::SetRenderView(VDDPoolTextureIndex texture, uint32 mipLevel, bool bypassConversion) {
	IVDTTexture *tex = mpPool->mpTextures[(uint32)texture - 1];
	VDDPoolRenderViewId rvi {};

	// we may get a null texture here if a texture failed to create, silently ignore it
	if (tex) {
		IVDTTexture2D *tex2d = vdpoly_cast<IVDTTexture2D *>(tex);

		if (tex2d) {
			VDTTextureDesc desc;
			tex2d->GetDesc(desc);
			rvi = SetRenderView(
				VDDRenderView {
					tex2d->GetLevelSurface(mipLevel),
					bypassConversion,
					VDTViewport { 0, 0, desc.mWidth, desc.mHeight, 0.0f, 1.0f },
					VDDisplaySoftViewport {
						{ (float)desc.mWidth, (float)desc.mHeight },
						{ 0, 0 } 
					}
				}
			);
		}
	}

	return rvi;
}

VDDPoolRenderViewId VDDisplayMeshBuilder3D::SetRenderView(VDDPoolTextureIndex texture, uint32 mipLevel, bool bypassConversion, const VDTViewport& viewport) {
	IVDTTexture *tex = mpPool->mpTextures[(uint32)texture - 1];
	VDDPoolRenderViewId rvi {};

	// we may get a null texture here if a texture failed to create, silently ignore it
	if (tex) {
		IVDTTexture2D *tex2d = vdpoly_cast<IVDTTexture2D *>(tex);

		if (tex2d)
			rvi = SetRenderView(VDDRenderView { tex2d->GetLevelSurface(mipLevel), bypassConversion, viewport, VDDisplaySoftViewport { { (float)viewport.mWidth, (float)viewport.mHeight }, { 0, 0 } } });
	}

	return rvi;
}

VDDisplayMeshBuilder3D::VDDisplayMeshBuilder3D(VDDisplayNodeContext3D& dctx, VDDisplayMeshPool3D& pool, VDDisplayMeshCommand3D& cmd, VDDPoolMeshIndex meshIndex)
	: mpDctx(&dctx)
	, mpPool(&pool)
	, mpCmd(&cmd)
	, mMeshIndex(meshIndex)
{
}

///////////////////////////////////////////////////////////////////////////

VDDisplayMeshPool3D::~VDDisplayMeshPool3D() {
	Clear();
}

void VDDisplayMeshPool3D::Clear() {
	mVertexData.clear();
	mIndexData.clear();
	mpSamplers.clear();
	mpTextures.clear();
	mTextureIndices.clear();
	mConstData.clear();
	mMeshes.clear();
	mRenderViews.clear();
	
	while(!mpAllocatedTextures.empty()) {
		mpAllocatedTextures.back()->Release();
		mpAllocatedTextures.pop_back();
	}

	mbError = false;
}

void VDDisplayMeshPool3D::SetTexture(VDDPoolTextureIndex index, IVDTTexture *texture) {
	VDASSERT(index != VDDPoolTextureIndex(0));
	mpTextures[(uint32)index - 1] = texture;
}

VDDPoolTextureIndex VDDisplayMeshPool3D::RegisterTexture(IVDTTexture *texture) {
	mpTextures.push_back(texture);

	return VDDPoolTextureIndex(mpTextures.size());
}

VDDPoolTextureIndex VDDisplayMeshPool3D::AddTempTexture(IVDTContext& ctx, uint32 width, uint32 height, VDTFormat format, uint32 mipcount) {
	VDDPoolTextureIndex pti = RegisterTexture(nullptr);

	vdrefptr<IVDTTexture2D> tex;
	if (ctx.CreateTexture2D(width, height, format, mipcount, kVDTUsage_Render, nullptr, ~tex)) {
		mpTextures.back() = tex;

		VDTTextureDesc desc;
		tex->GetDesc(desc);

		uint32 actualMips = desc.mMipCount;
		for(uint32 i=0; i<actualMips; ++i) {
			ctx.SetRenderTarget(0, tex->GetLevelSurface(i), false);
			ctx.Clear(kVDTClear_All, 0, 0, 0);
		}

		ctx.SetRenderTarget(0, nullptr, false);

		mpAllocatedTextures.push_back(tex);
		tex.release();
	}

	return pti;
}

VDDPoolRenderViewId VDDisplayMeshPool3D::RegisterRenderView(const VDDRenderView& renderView) {
	mRenderViews.emplace_back(renderView);

	return VDDPoolRenderViewId(mRenderViews.size());	// note that we start at 1
}

void VDDisplayMeshPool3D::SetRenderView(VDDPoolRenderViewId id, const VDDRenderView& renderView) {
	VDASSERT(id != VDDPoolRenderViewId(0));

	mRenderViews[(uint32)id - 1] = renderView;
}

void VDDisplayMeshPool3D::SetRenderViewWithSubrect(VDDPoolRenderViewId id, const VDDRenderView& renderView, float x, float y, float w, float h) {
	VDDRenderView newRenderView(renderView);
	newRenderView.mSoftViewport.mOffset += vdfloat2 { x, y };
	newRenderView.mSoftViewport.mSize = vdfloat2 { w, h };

	SetRenderView(id, newRenderView);
}

void VDDisplayMeshPool3D::SetRenderViewFromCurrent(VDDPoolRenderViewId id, IVDTContext& ctx) {
	SetRenderView(id, VDDRenderView { ctx.GetRenderTarget(0), ctx.GetRenderTargetBypass(0), ctx.GetViewport() });
}

VDDisplayMeshBuilder3D VDDisplayMeshPool3D::AddMesh(VDDisplayNodeContext3D& dctx) {
	mMeshes.emplace_back();

	return VDDisplayMeshBuilder3D(dctx, *this, mMeshes.back(), VDDPoolMeshIndex(mMeshes.size() - 1));
}

void VDDisplayMeshPool3D::DrawAllMeshes(IVDTContext& ctx, VDDisplayNodeContext3D& dctx) {
	DrawMeshes(ctx, dctx, VDDPoolMeshIndex(0), (uint32)mMeshes.size());
}

void VDDisplayMeshPool3D::DrawMeshes(IVDTContext& ctx, VDDisplayNodeContext3D& dctx, VDDPoolMeshIndex start, uint32 count) {
	VDDisplayMeshCommand3D *VDRESTRICT meshes = &mMeshes[(uint32)start];

	if (mVertexData.size() != mVertexTransformedData.size())
		mVertexTransformedData.resize(mVertexData.size());

	ctx.SetBlendState(NULL);
	ctx.SetRasterizerState(NULL);
	ctx.SetIndexStream(dctx.mpIndexCache);

	IVDTTexture *tex[16];

	while(count--) {
		if (meshes->mRenderView != VDDPoolRenderViewId(0)) {
			const auto& rv = mRenderViews[(uint32)meshes->mRenderView - 1];

			dctx.ApplyRenderView(rv);
		}

		if (meshes->mbRenderClear)
			ctx.Clear(kVDTClear_All, meshes->mRenderClearColor, 0, 0);

		ctx.SetVertexFormat(meshes->mpVF);
		ctx.SetVertexProgram(meshes->mpVP);
		ctx.SetFragmentProgram(meshes->mpFP);

		if (meshes->mVPConstCount) {
			ctx.SetVertexProgramConstCount(meshes->mVPConstCount);

			if (meshes->mVPConstCount)
				ctx.SetVertexProgramConstF(1, meshes->mVPConstCount, (const float *)&mConstData[meshes->mVPConstOffset]);
		}

		if (meshes->mFPConstCount) {
			ctx.SetFragmentProgramConstCount(meshes->mFPConstCount);

			if (meshes->mFPConstCount)
				ctx.SetFragmentProgramConstF(0, meshes->mFPConstCount, (const float *)&mConstData[meshes->mFPConstOffset]);
		}

		if (meshes->mSamplerCount)
			ctx.SetSamplerStates(0, meshes->mSamplerCount, &mpSamplers[meshes->mSamplerStart]);

		if (meshes->mTextureCount) {
			const VDDPoolTextureIndex *ptis = &mTextureIndices[meshes->mTextureStart];

			for(uint32 i=0; i<meshes->mTextureCount; ++i) {
				const uint32 pti = (uint32)ptis[i];

				tex[i] = pti ? mpTextures[pti - 1] : nullptr;
			}

			ctx.SetTextures(0, meshes->mTextureCount, tex);
			ctx.ClearTexturesStartingAt(meshes->mTextureCount);
		}

		const VDDisplaySoftViewport& vxt = dctx.GetCurrentSoftViewport();
		if (meshes->mLastSoftViewport != vxt) {
			meshes->mLastSoftViewport = vxt;

			const VDTViewport& vp = ctx.GetViewport();

			vdfloat2 invVp = vdfloat2{ 1, 1 } / vdfloat2{(float)vp.mWidth, (float)vp.mHeight};
			vdfloat2 scale = vxt.mSize * invVp;
			vdfloat2 offset = vdfloat2{-1, 1} - vdfloat2{-scale.x, scale.y} + invVp*vdfloat2{2,-2}*vxt.mOffset;

			meshes->mVertexTransformer(
				&mVertexTransformedData[meshes->mVertexSourceOffset],
				&mVertexData[meshes->mVertexSourceOffset],
				meshes->mVertexCount,
				scale,
				offset
			);

			meshes->mVertexCachedGeneration = 0;
		}

		if (!dctx.CacheVB(&mVertexTransformedData[meshes->mVertexSourceOffset], meshes->mVertexSourceLen, meshes->mVertexCachedOffset, meshes->mVertexCachedGeneration))
			return;

		ctx.SetVertexStream(0, dctx.mpVertexCache, meshes->mVertexCachedOffset, meshes->mVertexSize);
		
		if (!dctx.CacheIB(&mIndexData[meshes->mIndexSourceOffset], meshes->mIndexSourceCount, meshes->mIndexCachedOffset, meshes->mIndexCachedGeneration))
			return;

		ctx.DrawIndexedPrimitive(kVDTPT_Triangles, 0, 0, meshes->mVertexCount, meshes->mIndexCachedOffset, meshes->mIndexSourceCount / 3);

		++meshes;
	}

	ctx.ClearTexturesStartingAt(0);
}

///////////////////////////////////////////////////////////////////////////

size_t VDDisplayNodeContext3D::VDTDataHashPred::operator()(const VDTData& x) const {
	size_t hash = x.mLength;

	hash += (size_t)(uintptr_t)x.mpData;

	return hash;
}

bool VDDisplayNodeContext3D::VDTDataHashPred::operator()(const VDTData& x, const VDTData& y) const {
	return x.mLength == y.mLength && x.mpData == y.mpData;
}

VDDisplayNodeContext3D::VDDisplayNodeContext3D() {
}

VDDisplayNodeContext3D::~VDDisplayNodeContext3D() {
}

bool VDDisplayNodeContext3D::Init(IVDTContext& ctx, bool preferLinear) {
	mpContext = &ctx;

	if (ctx.IsFormatSupportedTexture2D(kVDTF_B8G8R8A8))
		mBGRAFormat = kVDTF_B8G8R8A8;
	else
		return false;

	if (ctx.IsFormatSupportedTexture2D(kVDTF_B8G8R8A8_sRGB)) {
		mBGRASRGBFormat = kVDTF_B8G8R8A8_sRGB;
		mbRenderLinear = preferLinear;
	} else
		mBGRASRGBFormat = mBGRAFormat;

	if (ctx.IsFormatSupportedTexture2D(kVDTF_R16G16B16A16F))
		mHDRFormat = kVDTF_R16G16B16A16F;
	else
		mHDRFormat = kVDTF_Unknown;

	if (!ctx.CreateVertexProgram(kVDTPF_MultiTarget, VDTDataView(g_VDDispVP_Texture), &mpVPTexture)) {
		Shutdown();
		return false;
	}

	if (!ctx.CreateVertexProgram(kVDTPF_MultiTarget, VDTDataView(g_VDDispVP_Texture2T), &mpVPTexture2T)) {
		Shutdown();
		return false;
	}

	if (!ctx.CreateVertexProgram(kVDTPF_MultiTarget, VDTDataView(g_VDDispVP_Texture3T), &mpVPTexture3T)) {
		Shutdown();
		return false;
	}

	static const VDTVertexElement kVertexFormat[]={
		{ offsetof(VDDisplayVertex3D, x), kVDTET_Float3, kVDTEU_Position, 0 },
		{ offsetof(VDDisplayVertex3D, u), kVDTET_Float2, kVDTEU_TexCoord, 0 },
	};

	if (!ctx.CreateVertexFormat(kVertexFormat, 2, mpVPTexture, &mpVFTexture)) {
		Shutdown();
		return false;
	}

	static const VDTVertexElement kVertexFormat2T[]={
		{ offsetof(VDDisplayVertex2T3D, x), kVDTET_Float3, kVDTEU_Position, 0 },
		{ offsetof(VDDisplayVertex2T3D, u0), kVDTET_Float2, kVDTEU_TexCoord, 0 },
		{ offsetof(VDDisplayVertex2T3D, u1), kVDTET_Float2, kVDTEU_TexCoord, 1 },
	};

	if (!ctx.CreateVertexFormat(kVertexFormat2T, 3, mpVPTexture2T, &mpVFTexture2T)) {
		Shutdown();
		return false;
	}

	static const VDTVertexElement kVertexFormat3T[]={
		{ offsetof(VDDisplayVertex3T3D, x), kVDTET_Float3, kVDTEU_Position, 0 },
		{ offsetof(VDDisplayVertex3T3D, u0), kVDTET_Float2, kVDTEU_TexCoord, 0 },
		{ offsetof(VDDisplayVertex3T3D, u1), kVDTET_Float2, kVDTEU_TexCoord, 1 },
		{ offsetof(VDDisplayVertex3T3D, u2), kVDTET_Float2, kVDTEU_TexCoord, 2 },
	};

	if (!ctx.CreateVertexFormat(kVertexFormat3T, 4, mpVPTexture3T, &mpVFTexture3T)) {
		Shutdown();
		return false;
	}

	if (!ctx.CreateFragmentProgram(kVDTPF_MultiTarget, VDTDataView(g_VDDispFP_Blit), &mpFPBlit)) {
		Shutdown();
		return false;
	}

	VDTSamplerStateDesc ssdesc = {};
	ssdesc.mFilterMode = kVDTFilt_Point;
	ssdesc.mAddressU = kVDTAddr_Clamp;
	ssdesc.mAddressV = kVDTAddr_Clamp;
	ssdesc.mAddressW = kVDTAddr_Clamp;

	if (!ctx.CreateSamplerState(ssdesc, &mpSSPoint)) {
		Shutdown();
		return false;
	}

	ssdesc.mFilterMode = kVDTFilt_Bilinear;

	if (!ctx.CreateSamplerState(ssdesc, &mpSSBilinear)) {
		Shutdown();
		return false;
	}

	ssdesc.mFilterMode = kVDTFilt_BilinearMip;
	ssdesc.mAddressU = kVDTAddr_Wrap;
	ssdesc.mAddressV = kVDTAddr_Wrap;

	if (!ctx.CreateSamplerState(ssdesc, &mpSSBilinearRepeatMip)) {
		Shutdown();
		return false;
	}

	if (!ctx.CreateVertexBuffer(kCacheVBBytes, true, nullptr, &mpVertexCache)) {
		Shutdown();
		return false;
	}

	if (!ctx.CreateIndexBuffer(kCacheIBIndices, false, true, nullptr, &mpIndexCache)) {
		Shutdown();
		return false;
	}

	mVBCacheBytePos = 0;
	mVBCacheGeneration = 1;
	mIBCacheIdxPos = 0;
	mIBCacheGeneration = 1;

	return true;
}

void VDDisplayNodeContext3D::Shutdown() {
	vdsaferelease <<=
		mpVertexCache,
		mpIndexCache,
		mpFPBlit,
		mpVFTexture,
		mpVFTexture2T,
		mpVFTexture3T,
		mpVPTexture,
		mpVPTexture2T,
		mpVPTexture3T,
		mpSSBilinearRepeatMip,
		mpSSBilinear,
		mpSSPoint;

	for(const auto& vpe : mVPCache)
		vpe.second->Release();

	mVPCache.clear();

	for(const auto& fpe : mFPCache)
		fpe.second->Release();

	mFPCache.clear();
}

void VDDisplayNodeContext3D::BeginScene(bool traceRTs) {
	mbTraceRTs = traceRTs;
	mTracedRTs.clear();
}

void VDDisplayNodeContext3D::ClearTrace() {
	mTracedRTs.clear();
}

VDDRenderView VDDisplayNodeContext3D::CaptureRenderView() const {
	return VDDRenderView { mpContext->GetRenderTarget(0), false, mpContext->GetViewport() };
}

void VDDisplayNodeContext3D::ApplyRenderView(const VDDRenderView& targetView) {
	mTracedRTs.emplace_back(targetView.mpTarget);

	mpContext->SetRenderTarget(0, targetView.mpTarget, targetView.mbBypassSrgb);
	mpContext->SetViewport(targetView.mViewport);

	mSoftViewport = targetView.mSoftViewport;
}

void VDDisplayNodeContext3D::ApplyRenderViewWithSubrect(const VDDRenderView& targetView, float x, float y, float w, float h) {
	VDDRenderView newTargetView(targetView);

	vdfloat2 invVp = vdfloat2{1,1} / vdfloat2{ (float)targetView.mViewport.mWidth, (float)targetView.mViewport.mHeight };
	newTargetView.mSoftViewport.mOffset += vdfloat2{ x, y };
	newTargetView.mSoftViewport.mSize = vdfloat2{ w, h };

	ApplyRenderView(newTargetView);
}

IVDTVertexProgram *VDDisplayNodeContext3D::InitVP(VDTData vpdata) {
	auto r = mVPCache.insert(vpdata);

	if (r.second) {
		if (!mpContext->CreateVertexProgram(kVDTPF_MultiTarget, vpdata, &r.first->second)) {
			mVPCache.erase(r.first);
			return nullptr;
		}
	}

	return r.first->second;
}

IVDTFragmentProgram *VDDisplayNodeContext3D::InitFP(VDTData fpdata) {
	auto r = mFPCache.insert(fpdata);

	if (r.second) {
		if (!mpContext->CreateFragmentProgram(kVDTPF_MultiTarget, fpdata, &r.first->second)) {
			mFPCache.erase(r.first);
			return nullptr;
		}
	}

	return r.first->second;
}

bool VDDisplayNodeContext3D::CacheVB(const void *data, uint32 len, uint32& offset, uint32& generation) {
	if (generation == mVBCacheGeneration)
		return true;

	if (kCacheVBBytes - mVBCacheBytePos < len) {
		if (len > kCacheVBBytes) {
			VDFAIL("Vertex data exceeds size of vertex cache.");
			return false;
		}

		mVBCacheBytePos = 0;
		mVBCacheGeneration += 2;
	}

	if (!mpVertexCache->Load(mVBCacheBytePos, len, data))
		return false;

	offset = mVBCacheBytePos;
	generation = mVBCacheGeneration;

	mVBCacheBytePos += len;
	return true;
}

bool VDDisplayNodeContext3D::CacheIB(const uint16 *data, uint32 count, uint32& offset, uint32& generation) {
	if (generation == mIBCacheGeneration)
		return true;

	if (kCacheIBIndices - mIBCacheIdxPos < count) {
		if (count > kCacheIBIndices) {
			VDFAIL("Index data exceeds size of index cache.");
			return false;
		}

		mIBCacheIdxPos = 0;
		mIBCacheGeneration += 2;
	}

	if (!mpIndexCache->Load(mIBCacheIdxPos * 2, count * 2, data))
		return false;

	offset = mIBCacheIdxPos;
	generation = mIBCacheGeneration;

	mIBCacheIdxPos += count;
	return true;
}

///////////////////////////////////////////////////////////////////////////

VDDisplaySourceNode3D::~VDDisplaySourceNode3D() {
}

///////////////////////////////////////////////////////////////////////////

VDDisplayTextureSourceNode3D::VDDisplayTextureSourceNode3D()
	: mpImageTex(NULL)
{
}

VDDisplayTextureSourceNode3D::~VDDisplayTextureSourceNode3D() {
	Shutdown();
}

bool VDDisplayTextureSourceNode3D::Init(IVDTTexture2D *tex, const VDDisplaySourceTexMapping& mapping) {
	mpImageTex = tex;
	mpImageTex->AddRef();
	mMapping = mapping;
	return true;
}

void VDDisplayTextureSourceNode3D::Shutdown() {
	vdsaferelease <<= mpImageTex;
}

VDDisplaySourceTexMapping VDDisplayTextureSourceNode3D::GetTextureMapping() const {
	return mMapping;
}

IVDTTexture2D *VDDisplayTextureSourceNode3D::Draw(IVDTContext& ctx, VDDisplayNodeContext3D& dctx) {
	return mpImageTex;
}

///////////////////////////////////////////////////////////////////////////

VDDisplayImageSourceNode3D::VDDisplayImageSourceNode3D() {
}

VDDisplayImageSourceNode3D::~VDDisplayImageSourceNode3D() {
	Shutdown();
}

bool VDDisplayImageSourceNode3D::Init(IVDTContext& ctx, VDDisplayNodeContext3D& dctx, uint32 w, uint32 h, uint32 format) {
	const VDTDeviceCaps& caps = ctx.GetDeviceCaps();

	uint32 texWidth = w;
	uint32 texHeight = h;

	if (!caps.mbNonPow2Conditional) {
		texWidth = VDCeilToPow2(w);
		texHeight = VDCeilToPow2(h);
	}

	if (texWidth > caps.mMaxTextureWidth || texHeight > caps.mMaxTextureHeight)
		return false;

	mbTextureInited = false;

	switch(format) {
		case nsVDPixmap::kPixFormat_RGB565:
		case nsVDPixmap::kPixFormat_XRGB1555:{
			VDTFormat rgb16Format = (format == nsVDPixmap::kPixFormat_RGB565) ? kVDTF_B5G6R5 : kVDTF_B5G5R5A1;

			if (ctx.IsFormatSupportedTexture2D(rgb16Format)) {
				if (ctx.CreateTexture2D(texWidth, texHeight, rgb16Format, 1, kVDTUsage_Default, NULL, &mpImageTex)) {
					mMapping.Init(w, h, texWidth, texHeight);
					return true;
				}
			}
			break;
		}

		case nsVDPixmap::kPixFormat_XRGB8888:
			if (ctx.CreateTexture2D(texWidth, texHeight, dctx.mBGRAFormat, 1, kVDTUsage_Default, NULL, &mpImageTex)) {
				mMapping.Init(w, h, texWidth, texHeight);
				return true;
			}
			break;

		case nsVDPixmap::kPixFormat_Y8_FR:
			if (ctx.IsFormatSupportedTexture2D(kVDTF_L8)) {
				if (ctx.CreateTexture2D(texWidth, texHeight, kVDTF_L8, 1, kVDTUsage_Default, NULL, &mpImageTex)) {
					mMapping.Init(w, h, texWidth, texHeight);
					return true;
				}
			}
			break;
		
		default:
			return false;
	}

	Shutdown();
	return false;
}

void VDDisplayImageSourceNode3D::Shutdown() {
	vdsaferelease <<= mpImageTex;
}

void VDDisplayImageSourceNode3D::Load(const VDPixmap& px) {
	VDTLockData2D lockData;
	
	vdrect32 r(0, 0, std::min<sint32>(px.w, mMapping.mTexWidth), std::min<sint32>(px.h, mMapping.mTexHeight));
	if (!mpImageTex->Lock(0, mbTextureInited ? &r : nullptr, !mbTextureInited || ((uint32)r.right == mMapping.mTexWidth && (uint32)r.bottom == mMapping.mTexHeight), lockData))
		return;

	if (!mbTextureInited) {
		mbTextureInited = true;
		VDMemset32Rect(lockData.mpData, lockData.mPitch, 0, mMapping.mTexWidth, mMapping.mTexHeight);
	}

	VDPixmap dstpx = {};
	dstpx.data = lockData.mpData;
	dstpx.pitch = lockData.mPitch;
	dstpx.format = nsVDPixmap::kPixFormat_XRGB8888;
	dstpx.w = mMapping.mTexWidth;
	dstpx.h = mMapping.mTexHeight;

	VDPixmapBlt(dstpx, px);

	mpImageTex->Unlock(0);
}

VDDisplaySourceTexMapping VDDisplayImageSourceNode3D::GetTextureMapping() const {
	return mMapping;
}

IVDTTexture2D *VDDisplayImageSourceNode3D::Draw(IVDTContext& ctx, VDDisplayNodeContext3D& dctx) {
	return mpImageTex;
}

///////////////////////////////////////////////////////////////////////////

VDDisplayBufferSourceNode3D::VDDisplayBufferSourceNode3D()
	: mpRTT(NULL)
	, mpChildNode(NULL)
{
}

VDDisplayBufferSourceNode3D::~VDDisplayBufferSourceNode3D() {
	Shutdown();
}

bool VDDisplayBufferSourceNode3D::Init(IVDTContext& ctx, VDDisplayNodeContext3D& dctx, float outx, float outy, float outw, float outh, uint32 w, uint32 h, bool hdr, VDDisplayNode3D *child) {
	if (mpChildNode != child) {
		if (mpChildNode)
			mpChildNode->Release();

		mpChildNode = child;
		child->AddRef();
	}

	const VDTDeviceCaps& caps = ctx.GetDeviceCaps();

	uint32 texWidth = w;
	uint32 texHeight = h;

	if (!caps.mbNonPow2) {
		texWidth = VDCeilToPow2(texWidth);
		texHeight = VDCeilToPow2(texHeight);
	}

	if (texWidth > caps.mMaxTextureWidth || texHeight > caps.mMaxTextureHeight) {
		Shutdown();
		return false;
	}

	if (mpRTT) {
		VDTTextureDesc desc;

		mpRTT->GetDesc(desc);

		if (desc.mWidth != texWidth || desc.mHeight != texHeight) {
			mpRTT->Release();
			mpRTT = NULL;
		}
	}

	if (!mpRTT) {
		if (!ctx.CreateTexture2D(texWidth, texHeight, hdr ? dctx.mHDRFormat : dctx.mBGRAFormat, 1, kVDTUsage_Render, NULL, &mpRTT)) {
			Shutdown();
			return false;
		}
	}

	// If the output rect is at or left/above the origin, we don't need to translate; otherwise,
	// shift left/up to optimize texture space. Always translate by integral pixels.
	mDestX = std::min<float>(0.0f, -ceilf(outx - 0.5f));
	mDestY = std::min<float>(0.0f, -ceilf(outy - 0.5f));

	outx += mDestX;
	outy += mDestY;

	mMapping.mTexelOffset.set(outx, outy);
	mMapping.mUVOffset.set(outx / (float)texWidth, outy / (float)texHeight);
	mMapping.mTexelSize.set(outw, outh);
	mMapping.mUVSize.set((float)outw / (float)texWidth, (float)outh / (float)texHeight);
	mMapping.mTexWidth = texWidth;
	mMapping.mTexHeight = texHeight;
	return true;
}

void VDDisplayBufferSourceNode3D::Shutdown() {
	vdsaferelease <<= mpRTT, mpChildNode;
}

VDDisplaySourceTexMapping VDDisplayBufferSourceNode3D::GetTextureMapping() const {
	return mMapping;
}

IVDTTexture2D *VDDisplayBufferSourceNode3D::Draw(IVDTContext& ctx, VDDisplayNodeContext3D& dctx) {
	if (!mpRTT)
		return NULL;

	IVDTSurface *rttsurf = mpRTT->GetLevelSurface(0);

	VDTSurfaceDesc rttsurfdesc;
	rttsurf->GetDesc(rttsurfdesc);

	VDTViewport vp;
	vp.mX = 0;
	vp.mY = 0;
	vp.mWidth = rttsurfdesc.mWidth;
	vp.mHeight = rttsurfdesc.mHeight;
	vp.mMinZ = 0.0f;
	vp.mMaxZ = 1.0f;

	const VDDRenderView renderView {
		rttsurf, false, vp,
		{
			mMapping.mTexelSize,
			vdfloat2 { (float)mDestX, (float)mDestY },
		}
	};

	mpChildNode->Draw(ctx, dctx, renderView);
	return mpRTT;
}

///////////////////////////////////////////////////////////////////////////

VDDisplayNode3D::~VDDisplayNode3D() {
}

///////////////////////////////////////////////////////////////////////////

VDDisplaySequenceNode3D::VDDisplaySequenceNode3D() {
}

VDDisplaySequenceNode3D::~VDDisplaySequenceNode3D() {
	Shutdown();
}

void VDDisplaySequenceNode3D::Shutdown() {
	while(!mNodes.empty()) {
		VDDisplayNode3D *node = mNodes.back();
		mNodes.pop_back();

		node->Release();
	}
}

void VDDisplaySequenceNode3D::AddNode(VDDisplayNode3D *node) {
	mNodes.push_back(node);
	node->AddRef();
}

void VDDisplaySequenceNode3D::Draw(IVDTContext& ctx, VDDisplayNodeContext3D& dctx, const VDDRenderView& renderView) {
	for(VDDisplayNode3D *node : mNodes)
		node->Draw(ctx, dctx, renderView);
}

///////////////////////////////////////////////////////////////////////////

VDDisplayClearNode3D::VDDisplayClearNode3D()
	: mColor(0)
{
}

VDDisplayClearNode3D::~VDDisplayClearNode3D() {
}

void VDDisplayClearNode3D::SetClearColor(uint32 c) {
	mColor = c;
}

void VDDisplayClearNode3D::Draw(IVDTContext& ctx, VDDisplayNodeContext3D& dctx, const VDDRenderView& renderView) {
	dctx.ApplyRenderView(renderView);
	ctx.Clear(kVDTClear_Color, mColor, 0, 0);
}

///////////////////////////////////////////////////////////////////////////

VDDisplayImageNode3D::VDDisplayImageNode3D()
	: mpPaletteTex(NULL)
	, mpVF(NULL)
	, mDstX(0)
	, mDstY(0)
	, mDstW(0)
	, mDstH(0)
	, mTexWidth(0)
	, mTexHeight(0)
	, mTex2Width(0)
	, mTex2Height(0)
	, mbBilinear(true)
{
	std::fill(mpImageTex, mpImageTex + 3, (IVDTTexture2D *)NULL);
}

VDDisplayImageNode3D::~VDDisplayImageNode3D() {
	Shutdown();
}

bool VDDisplayImageNode3D::CanStretch() const {
	switch(mRenderMode) {
		case kRenderMode_Blit:
		case kRenderMode_BlitY:
		case kRenderMode_BlitYCbCr:
		case kRenderMode_BlitRGB16Direct:
			return true;

		default:
			return false;
	}
}

bool VDDisplayImageNode3D::Init(IVDTContext& ctx, VDDisplayNodeContext3D& dctx, uint32 w, uint32 h, uint32 format) {
	const VDTDeviceCaps& caps = ctx.GetDeviceCaps();

	if (w > caps.mMaxTextureWidth || h > caps.mMaxTextureHeight)
		return false;

	mTexWidth = w;
	mTexHeight = h;

	constexpr VDTFormat bgraFormat = kVDTF_B8G8R8A8;

	float chromaOffsetU = 0.0f;
	
	mMeshPool.Clear();
	auto mb = mMeshPool.AddMesh(dctx);

	switch(format) {
		case nsVDPixmap::kPixFormat_RGB565:
		case nsVDPixmap::kPixFormat_XRGB1555:{
			VDTFormat rgb16Format = (format == nsVDPixmap::kPixFormat_RGB565) ? kVDTF_B5G6R5 : kVDTF_B5G5R5A1;

			if (ctx.IsFormatSupportedTexture2D(rgb16Format)) {
				if (!ctx.CreateTexture2D(w, h, rgb16Format, 1, kVDTUsage_Default, NULL, &mpImageTex[0])) {
					Shutdown();
					return false;
				}

				mb.SetVertexProgram(dctx.mpVPTexture);
				mb.SetFragmentProgram(VDTDataView(g_VDDispFP_Blit));

				mRenderMode = kRenderMode_BlitRGB16Direct;
				mpVF = dctx.mpVFTexture;
				mpVF->AddRef();
			} else {
				bool l8a8 = ctx.IsFormatSupportedTexture2D(kVDTF_L8A8);
				bool r8g8 = ctx.IsFormatSupportedTexture2D(kVDTF_R8G8);

				if (l8a8 || r8g8) {
					if (!ctx.CreateTexture2D(w, h, l8a8 ? kVDTF_L8A8 : kVDTF_R8G8, 1, kVDTUsage_Default, NULL, &mpImageTex[0])) {
						Shutdown();
						return false;
					}

					mb.SetVertexProgram(dctx.mpVPTexture);
					mb.SetFragmentProgram(l8a8 ? VDTDataView(g_VDDispFP_BlitRGB16_L8A8) : VDTDataView(g_VDDispFP_BlitRGB16_R8G8));

					uint8 palette[2][256][4];

					for(uint32 i=0; i<256; ++i) {
						uint8 r0, g0, g1, b1;

						if (rgb16Format == kVDTF_B5G6R5) {
							r0 = (i >> 3) & 31;
							g0 = (i & 7);
							g1 = i >> 5;

							g0 = (g0 << 5) + (g0 >> 1);
							g1 = (g1 << 2);
						} else {
							r0 = (i >> 2) & 31;
							g0 = (i & 3);
							g1 = i >> 5;

							g0 = (g0 << 6) + (g0 << 1);
							g1 = (g1 << 3) + (g1 >> 2);
						}

						r0 = (r0 << 3) + (r0 >> 2);

						b1 = i & 31;
						b1 = (b1 << 3) + (b1 >> 2);

						palette[1][i][0] = 0;
						palette[1][i][1] = g0;
						palette[1][i][2] = r0;
						palette[1][i][3] = 255;
						palette[0][i][0] = b1;
						palette[0][i][1] = g1;
						palette[0][i][2] = 0;
						palette[0][i][3] = 0;
					}

					VDTInitData2D palInitData = { palette, sizeof palette[0] };

					if (!ctx.CreateTexture2D(256, 2, bgraFormat, 1, kVDTUsage_Default, &palInitData, &mpPaletteTex)) {
						Shutdown();
						return false;
					}

					mRenderMode = kRenderMode_BlitRGB16;
					mpVF = dctx.mpVFTexture;
					mpVF->AddRef();
				}
			}
			break;
		}

		case nsVDPixmap::kPixFormat_RGB888:
			if (w * 3 <= caps.mMaxTextureWidth && ctx.IsFormatSupportedTexture2D(kVDTF_R8)) {
				mTexWidth *= 3;

				if (!ctx.CreateTexture2D(3*w, h, kVDTF_R8, 1, kVDTUsage_Default, NULL, &mpImageTex[0])) {
					Shutdown();
					return false;
				}

				mb.SetVertexProgram(dctx.mpVPTexture3T);
				mb.SetFragmentProgram(VDTDataView(g_VDDispFP_BlitRGB24));

				mRenderMode = kRenderMode_BlitRGB24;
				mpVF = dctx.mpVFTexture3T;
				mpVF->AddRef();
			}
			break;

		case nsVDPixmap::kPixFormat_YUV422_UYVY:
		case nsVDPixmap::kPixFormat_YUV422_UYVY_FR:
		case nsVDPixmap::kPixFormat_YUV422_UYVY_709:
		case nsVDPixmap::kPixFormat_YUV422_UYVY_709_FR:
		case nsVDPixmap::kPixFormat_YUV422_YUYV:
		case nsVDPixmap::kPixFormat_YUV422_YUYV_FR:
		case nsVDPixmap::kPixFormat_YUV422_YUYV_709:
		case nsVDPixmap::kPixFormat_YUV422_YUYV_709_FR:
			mTexWidth >>= 1;

			if (!ctx.CreateTexture2D(w >> 1, h, bgraFormat, 1, kVDTUsage_Default, NULL, &mpImageTex[0])) {
				Shutdown();
				return false;
			}

			VDTData program;

			switch(format) {
				case nsVDPixmap::kPixFormat_YUV422_UYVY:
					program = VDTDataView(g_VDDispFP_BlitUYVY_601_LR);
					break;

				case nsVDPixmap::kPixFormat_YUV422_UYVY_FR:
					program = VDTDataView(g_VDDispFP_BlitUYVY_601_FR);
					break;

				case nsVDPixmap::kPixFormat_YUV422_UYVY_709:
					program = VDTDataView(g_VDDispFP_BlitUYVY_709_LR);
					break;

				case nsVDPixmap::kPixFormat_YUV422_UYVY_709_FR:
					program = VDTDataView(g_VDDispFP_BlitUYVY_709_FR);
					break;

				case nsVDPixmap::kPixFormat_YUV422_YUYV:
					program = VDTDataView(g_VDDispFP_BlitYUYV_601_LR);
					break;

				case nsVDPixmap::kPixFormat_YUV422_YUYV_FR:
					program = VDTDataView(g_VDDispFP_BlitYUYV_601_FR);
					break;

				case nsVDPixmap::kPixFormat_YUV422_YUYV_709:
					program = VDTDataView(g_VDDispFP_BlitYUYV_709_LR);
					break;

				case nsVDPixmap::kPixFormat_YUV422_YUYV_709_FR:
					program = VDTDataView(g_VDDispFP_BlitYUYV_709_FR);
					break;
			}

			mb.SetVertexProgram(dctx.mpVPTexture3T);
			mb.SetFragmentProgram(program);

			mRenderMode = kRenderMode_BlitUYVY;

			mpVF = dctx.mpVFTexture3T;
			mpVF->AddRef();
			break;

		case nsVDPixmap::kPixFormat_Y8:
		case nsVDPixmap::kPixFormat_Y8_FR:
			if (ctx.IsFormatSupportedTexture2D(kVDTF_R8)) {
				if (!ctx.CreateTexture2D(w, h, kVDTF_R8, 1, kVDTUsage_Default, NULL, &mpImageTex[0])) {
					Shutdown();
					return false;
				}

				mb.SetVertexProgram(dctx.mpVPTexture);
				mb.SetFragmentProgram(
					format == nsVDPixmap::kPixFormat_Y8_FR
						? VDTDataView(g_VDDispFP_BlitY_FR)
						: VDTDataView(g_VDDispFP_BlitY_LR)
				);

				mRenderMode = kRenderMode_BlitY;

				mpVF = dctx.mpVFTexture;
				mpVF->AddRef();
			}
			break;

		case nsVDPixmap::kPixFormat_YUV444_Planar:
		case nsVDPixmap::kPixFormat_YUV422_Planar:
		case nsVDPixmap::kPixFormat_YUV420_Planar:
		case nsVDPixmap::kPixFormat_YUV444_Planar_709:
		case nsVDPixmap::kPixFormat_YUV422_Planar_709:
		case nsVDPixmap::kPixFormat_YUV420_Planar_709:
		case nsVDPixmap::kPixFormat_YUV444_Planar_FR:
		case nsVDPixmap::kPixFormat_YUV422_Planar_FR:
		case nsVDPixmap::kPixFormat_YUV420_Planar_FR:
		case nsVDPixmap::kPixFormat_YUV444_Planar_709_FR:
		case nsVDPixmap::kPixFormat_YUV422_Planar_709_FR:
		case nsVDPixmap::kPixFormat_YUV420_Planar_709_FR:
			if (ctx.IsFormatSupportedTexture2D(kVDTF_R8)) {
				const VDPixmapFormatInfo& formatInfo = VDPixmapGetInfo(format);

				if (!ctx.CreateTexture2D(w, h, kVDTF_R8, 1, kVDTUsage_Default, NULL, &mpImageTex[0])) {
					Shutdown();
					return false;
				}

				uint32 w2 = ((w - 1) >> formatInfo.auxwbits) + 1;
				uint32 h2 = ((h - 1) >> formatInfo.auxhbits) + 1;

				if (!ctx.CreateTexture2D(w2, h2, kVDTF_R8, 1, kVDTUsage_Default, NULL, &mpImageTex[1])) {
					Shutdown();
					return false;
				}

				if (!ctx.CreateTexture2D(w2, h2, kVDTF_R8, 1, kVDTUsage_Default, NULL, &mpImageTex[2])) {
					Shutdown();
					return false;
				}

				// 1:1 -> offset 0
				// 2:1 -> offset +1/4
				// 4:1 -> offset +3/8
				float chromaScaleH = (float)(1 << formatInfo.auxwbits);
				chromaOffsetU = (chromaScaleH - 1) / (chromaScaleH * 2.0f * (float)w2);

				mb.SetVertexProgram(dctx.mpVPTexture2T);

				switch(format) {
					case nsVDPixmap::kPixFormat_YUV444_Planar:
					case nsVDPixmap::kPixFormat_YUV422_Planar:
					case nsVDPixmap::kPixFormat_YUV420_Planar:
						mb.SetFragmentProgram(VDTDataView(g_VDDispFP_BlitYCbCr_601_LR));
						break;

					case nsVDPixmap::kPixFormat_YUV444_Planar_709:
					case nsVDPixmap::kPixFormat_YUV422_Planar_709:
					case nsVDPixmap::kPixFormat_YUV420_Planar_709:
						mb.SetFragmentProgram(VDTDataView(g_VDDispFP_BlitYCbCr_709_LR));
						break;

					case nsVDPixmap::kPixFormat_YUV444_Planar_FR:
					case nsVDPixmap::kPixFormat_YUV422_Planar_FR:
					case nsVDPixmap::kPixFormat_YUV420_Planar_FR:
						mb.SetFragmentProgram(VDTDataView(g_VDDispFP_BlitYCbCr_601_FR));
						break;

					case nsVDPixmap::kPixFormat_YUV444_Planar_709_FR:
					case nsVDPixmap::kPixFormat_YUV422_Planar_709_FR:
					case nsVDPixmap::kPixFormat_YUV420_Planar_709_FR:
						mb.SetFragmentProgram(VDTDataView(g_VDDispFP_BlitYCbCr_709_FR));
						break;
				}

				mTex2Width = w2;
				mTex2Height = h2;
				mRenderMode = kRenderMode_BlitYCbCr;

				mpVF = dctx.mpVFTexture2T;
				mpVF->AddRef();
			}
			break;

		case nsVDPixmap::kPixFormat_Pal8:
			if (ctx.IsFormatSupportedTexture2D(kVDTF_R8)) {
				if (!ctx.CreateTexture2D(w, h, kVDTF_R8, 1, kVDTUsage_Default, NULL, &mpImageTex[0])) {
					Shutdown();
					return false;
				}

				if (!ctx.CreateTexture2D(256, 1, dctx.mBGRAFormat, 1, kVDTUsage_Default, NULL, &mpPaletteTex)) {
					Shutdown();
					return false;
				}

				const VDTInitData2D initData = { mLastPalette, 0 };
				mpPaletteTex->Load(0, 0, 0, initData, 256, 1);

				mb.SetVertexProgram(dctx.mpVPTexture);
				mb.SetFragmentProgram(VDTDataView(g_VDDispFP_BlitPal8));

				mRenderMode = kRenderMode_BlitPal8;
				mpVF = dctx.mpVFTexture;
				mpVF->AddRef();
			}
			break;
		
		default:
			break;
	}

	if (!mpVF) {
		if (!ctx.CreateTexture2D(w, h, bgraFormat, 1, kVDTUsage_Default, NULL, &mpImageTex[0])) {
			Shutdown();
			return false;
		}

		mb.SetVertexProgram(dctx.mpVPTexture);
		mb.SetFragmentProgram(VDTDataView(g_VDDispFP_Blit));

		mRenderMode = kRenderMode_Blit;
		mpVF = dctx.mpVFTexture;
		mpVF->AddRef();
	}

	switch(mRenderMode) {
		case kRenderMode_BlitUYVY: {
			const float u0 = 0.25f / (float)mTexWidth;
			const float u1 = u0 + 1.0f;
			const float w0 = 0.0f;
			const float w1 = (float)mTexWidth;

			const VDDisplayVertex3T3D vx[4]={
				{ -1.0f, +1.0f, 0.0f, 0.0f, 0.0f, u0, 0.0f, w0 },
				{ -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, u0, 1.0f, w0 },
				{ +1.0f, +1.0f, 0.0f, 1.0f, 0.0f, u1, 0.0f, w1 },
				{ +1.0f, -1.0f, 0.0f, 1.0f, 1.0f, u1, 1.0f, w1 },
			};

			mb.SetVertices(vx);
			break;
		}

		case kRenderMode_BlitYCbCr: {
			const float u0 = chromaOffsetU;
			const float u1 = chromaOffsetU + 1.0f;

			const VDDisplayVertex2T3D vx[4]={
				{ -1.0f, +1.0f, 0.0f, 0.0f, 0.0f, u0, 0.0f },
				{ -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, u0, 1.0f },
				{ +1.0f, +1.0f, 0.0f, 1.0f, 0.0f, u1, 0.0f },
				{ +1.0f, -1.0f, 0.0f, 1.0f, 1.0f, u1, 1.0f },
			};

			mb.SetVertices(vx);
			break;
		}

		case kRenderMode_BlitRGB24: {
			const float u0 = -1.0f / (float)mTexWidth;
			const float u1 = u0 + 1.0f;
			const float u2 = 0.0f;
			const float u3 = 1.0f;
			const float u4 = +1.0f / (float)mTexWidth;
			const float u5 = u4 + 1.0f;

			const VDDisplayVertex3T3D vx[4]={
				{ -1.0f, +1.0f, 0.0f, u0, 0.0f, u2, 0.0f, u4, 0.0f },
				{ -1.0f, -1.0f, 0.0f, u0, 1.0f, u2, 1.0f, u4, 1.0f },
				{ +1.0f, +1.0f, 0.0f, u1, 0.0f, u3, 0.0f, u5, 0.0f },
				{ +1.0f, -1.0f, 0.0f, u1, 1.0f, u3, 1.0f, u5, 1.0f },
			};

			mb.SetVertices(vx);
			break;
		}

		default: {
			static const VDDisplayVertex3D vx[4]={
				{ -1.0f, +1.0f, 0.0f, 0.0f, 0.0f },
				{ -1.0f, -1.0f, 0.0f, 0.0f, 1.0f },
				{ +1.0f, +1.0f, 0.0f, 1.0f, 0.0f },
				{ +1.0f, -1.0f, 0.0f, 1.0f, 1.0f },
			};

			mb.SetVertices(vx);
			break;
		}
	}

	switch(mRenderMode) {
	case kRenderMode_Blit:
	case kRenderMode_BlitY:
	case kRenderMode_BlitRGB16Direct:
		{
			mb.InitSamplers(1);
			mb.SetSampler(0, mbBilinear ? dctx.mpSSBilinear : dctx.mpSSPoint);

			mb.InitTextures(1);
			mb.SetTexture(0, mMeshPool.RegisterTexture(mpImageTex[0]));
		}
		break;

	case kRenderMode_BlitPal8:
	case kRenderMode_BlitRGB16:
		{
			mb.InitSamplers(2);
			mb.SetSampler(0, dctx.mpSSBilinear);
			mb.SetSampler(1, dctx.mpSSPoint);

			IVDTTexture *const tex[2] = {
				mpImageTex[0],
				mpPaletteTex
			};

			mb.InitTextures(2);
			mb.SetTexture(0, mMeshPool.RegisterTexture(mpImageTex[0]));
			mb.SetTexture(1, mMeshPool.RegisterTexture(mpPaletteTex));
		}
		break;

	case kRenderMode_BlitYCbCr:
		{
			IVDTSamplerState *ss0 = mbBilinear ? dctx.mpSSBilinear : dctx.mpSSPoint;

			mb.InitSamplers(3);
			mb.SetSampler(0, ss0);
			mb.SetSampler(1, ss0);
			mb.SetSampler(2, ss0);

			mb.InitTextures(3);
			mb.SetTexture(0, mMeshPool.RegisterTexture(mpImageTex[0]));
			mb.SetTexture(1, mMeshPool.RegisterTexture(mpImageTex[1]));
			mb.SetTexture(2, mMeshPool.RegisterTexture(mpImageTex[2]));
		}
		break;

	case kRenderMode_BlitUYVY:
		{
			mb.InitSamplers(2);
			mb.SetSampler(0, dctx.mpSSPoint);
			mb.SetSampler(1, dctx.mpSSBilinear);

			auto tex = mMeshPool.RegisterTexture(mpImageTex[0]);
			mb.InitTextures(2);
			mb.SetTexture(0, tex);
			mb.SetTexture(1, tex);
		}
		break;

	case kRenderMode_BlitRGB24:
		{
			mb.InitSamplers(1);
			mb.SetSampler(0, dctx.mpSSPoint);

			mb.InitTextures(1);
			mb.SetTexture(0, mMeshPool.RegisterTexture(mpImageTex[0]));
		}
		break;
	}

	mb.SetVertexFormat(mpVF);
	mb.SetTopologyQuad();
	return true;
}

void VDDisplayImageNode3D::Shutdown() {
	for(int i=0; i<3; ++i)
		vdsaferelease <<= mpImageTex[i];

	vdsaferelease <<= mpPaletteTex, mpVF;
}

void VDDisplayImageNode3D::Load(const VDPixmap& px) {
	VDTLockData2D lockData;

	if (mRenderMode == kRenderMode_BlitPal8) {
		// palettes don't change often, so let's try to optimize it
		if (memcmp(mLastPalette, px.palette, sizeof(mLastPalette))) {
			memcpy(mLastPalette, px.palette, sizeof(mLastPalette));

			const VDTInitData2D initData = { mLastPalette, 0 };

			mpPaletteTex->Load(0, 0, 0, initData, 256, 1);
		}

		const VDTInitData2D plane0 = { px.data, px.pitch };

		mpImageTex[0]->Load(0, 0, 0, plane0, mTexWidth, mTexHeight);
	} else if (mRenderMode == kRenderMode_BlitY
		|| mRenderMode == kRenderMode_BlitUYVY
		|| mRenderMode == kRenderMode_BlitRGB16
		|| mRenderMode == kRenderMode_BlitRGB16Direct
		|| mRenderMode == kRenderMode_BlitRGB24
		) {
		const VDTInitData2D plane0 = { px.data, px.pitch };

		mpImageTex[0]->Load(0, 0, 0, plane0, mTexWidth, mTexHeight);
	} else if (mRenderMode == kRenderMode_BlitYCbCr) {
		const VDTInitData2D plane0 = { px.data, px.pitch };
		const VDTInitData2D plane1 = { px.data2, px.pitch2 };
		const VDTInitData2D plane2 = { px.data3, px.pitch3 };

		mpImageTex[0]->Load(0, 0, 0, plane0, mTexWidth, mTexHeight);
		mpImageTex[1]->Load(0, 0, 0, plane1, mTex2Width, mTex2Height);
		mpImageTex[2]->Load(0, 0, 0, plane2, mTex2Width, mTex2Height);
	} else {
		if (!mpImageTex[0]->Lock(0, NULL, false, lockData))
			return;

		VDPixmap dstpx = {};
		dstpx.data = lockData.mpData;
		dstpx.pitch = lockData.mPitch;
		dstpx.format = nsVDPixmap::kPixFormat_XRGB8888;
		dstpx.w = mTexWidth;
		dstpx.h = mTexHeight;

		VDPixmapBlt(dstpx, px);

		mpImageTex[0]->Unlock(0);
	}
}

void VDDisplayImageNode3D::Draw(IVDTContext& ctx, VDDisplayNodeContext3D& dctx, const VDDRenderView& renderView) {
	VDTAutoScope scope(ctx, "Image");
	
	dctx.ApplyRenderViewWithSubrect(renderView, mDstX, mDstY, mDstW, mDstH);

	mMeshPool.DrawAllMeshes(ctx, dctx);
}

///////////////////////////////////////////////////////////////////////////

VDDisplayBlitNode3D::VDDisplayBlitNode3D() {
}

VDDisplayBlitNode3D::~VDDisplayBlitNode3D() {
	Shutdown();
}

bool VDDisplayBlitNode3D::Init(IVDTContext& ctx, VDDisplayNodeContext3D& dctx, uint32 w, uint32 h, bool linear, float sharpnessX, float sharpnessY, VDDisplaySourceNode3D *source) {
	if (mpSourceNode != source) {
		if (mpSourceNode)
			mpSourceNode->Release();

		mpSourceNode = source;
		source->AddRef();
	}

	mMapping = source->GetTextureMapping();

	mMeshPool.Clear();
	auto mb = mMeshPool.AddMesh(dctx);

	const bool sharpBilinear = linear && (sharpnessX != 1.0f || sharpnessY != 1.0f);

	const float u0 = sharpBilinear ? mMapping.mTexelOffset.x : mMapping.mUVOffset.x;
	const float v0 = sharpBilinear ? mMapping.mTexelOffset.y : mMapping.mUVOffset.y;
	const float u1 = u0 + (sharpBilinear ? (float)w : mMapping.mUVSize.x);
	const float v1 = v0 + (sharpBilinear ? (float)h : mMapping.mUVSize.y);

	const VDDisplayVertex3D vx[4]={
		{ -1.0f, +1.0f, 0.0f, u0, v0 },
		{ -1.0f, -1.0f, 0.0f, u0, v1 },
		{ +1.0f, +1.0f, 0.0f, u1, v0 },
		{ +1.0f, -1.0f, 0.0f, u1, v1 },
	};

	mb.SetVertices(vx);

	if (sharpBilinear) {
		mb.SetFragmentProgram(VDTDataView(g_VDDispFP_BlitSharp));

		float params[4]={
			sharpnessX,
			sharpnessY,
			1.0f / (float)mMapping.mTexHeight,
			1.0f / (float)mMapping.mTexWidth,
		};

		mb.SetFPConstData(params);
	} else {
		mb.SetFragmentProgram(dctx.mpFPBlit);
	}

	mSourceTextureIndex = mMeshPool.RegisterTexture(nullptr);
	mb.InitTextures(1);
	mb.SetTexture(0, mSourceTextureIndex);
	mb.SetTopologyQuad();

	mb.InitSamplers(1);
	mb.SetSampler(0, linear ? dctx.mpSSBilinear : dctx.mpSSPoint);

	mb.SetVertexProgram(dctx.mpVPTexture);
	mb.SetVertexFormat(dctx.mpVFTexture);

	return true;
}

void VDDisplayBlitNode3D::Shutdown() {
	vdsaferelease <<= mpSourceNode;
}

void VDDisplayBlitNode3D::Draw(IVDTContext& ctx, VDDisplayNodeContext3D& dctx, const VDDRenderView& renderView) {
	IVDTTexture2D *src = mpSourceNode->Draw(ctx, dctx);

	if (!src)
		return;

	VDTAutoScope scope(ctx, "Blit");

	dctx.ApplyRenderViewWithSubrect(renderView, mDstX, mDstY, mDstW, mDstH);

	mMeshPool.SetTexture(mSourceTextureIndex, src);
	mMeshPool.DrawAllMeshes(ctx, dctx);
}

///////////////////////////////////////////////////////////////////////////

struct VDDisplayStretchBicubicNode3D::Vertex {
	float x, y, z;
	float u0, v0;
	float u1, v1;
	float u2, v2;
	float uf, vf;
};

VDDisplayStretchBicubicNode3D::VDDisplayStretchBicubicNode3D() {
}

VDDisplayStretchBicubicNode3D::~VDDisplayStretchBicubicNode3D() {
	Shutdown();
}

bool VDDisplayStretchBicubicNode3D::Init(IVDTContext& ctx, VDDisplayNodeContext3D& dctx, uint32 srcw, uint32 srch, uint32 dstw, uint32 dsth, float outx, float outy, float outw, float outh, VDDisplaySourceNode3D *src) {
	uint32 srctexh = srch;
	uint32 dsttexw = dstw;

	const VDTDeviceCaps& caps = ctx.GetDeviceCaps();
	if (!caps.mbNonPow2) {
		srctexh = VDCeilToPow2(srctexh);
		dsttexw = VDCeilToPow2(dsttexw);
	}

	if (dsttexw > caps.mMaxTextureWidth ||
		srctexh > caps.mMaxTextureHeight)
	{
		Shutdown();
		return false;
	}

	mpSourceNode = src;

	mMeshPool.Clear();

	if (!mpVF) {
		IVDTVertexProgram *vp = dctx.InitVP(VDTDataView(g_VDDispVP_StretchBltCubic));

		static const VDTVertexElement kVertexFormat[]={
			{ offsetof(Vertex, x), kVDTET_Float3, kVDTEU_Position, 0 },
			{ offsetof(Vertex, u0), kVDTET_Float2, kVDTEU_TexCoord, 0 },
			{ offsetof(Vertex, u1), kVDTET_Float2, kVDTEU_TexCoord, 1 },
			{ offsetof(Vertex, u2), kVDTET_Float2, kVDTEU_TexCoord, 2 },
			{ offsetof(Vertex, uf), kVDTET_Float2, kVDTEU_TexCoord, 3 },
		};

		if (!ctx.CreateVertexFormat(kVertexFormat, 5, vp, &mpVF)) {
			Shutdown();
			return false;
		}
	}

	mSrcW = srcw;
	mSrcH = srch;
	mDstW = dstw;
	mDstH = dsth;

	VDDPoolTextureIndex tempTex = mMeshPool.AddTempTexture(ctx, dsttexw, srctexh, dctx.mBGRAFormat, 1);

	mOutX = outx;
	mOutY = outy;
	mOutW = outw;
	mOutH = outh;

	const VDDisplaySourceTexMapping mapping = src->GetTextureMapping();

	uint32 filterTexWidth = std::max<uint32>(dstw, dsth);

	if (!ctx.GetDeviceCaps().mbNonPow2) {
		filterTexWidth = VDCeilToPow2(filterTexWidth);
	}

	const float offsetx = std::max<float>(0, -outx);
	const float offsety = std::max<float>(0, -outy);

	const float srcustep = 1.0f / (float)mapping.mTexWidth;
	const float srcvstep = 1.0f / (float)mapping.mTexHeight;

	const float hdu = srcw * srcustep;
	const float hdv = srch * srcvstep;

	const float u2 = hdu * offsetx / outw;
	const float u0 = u2 - 1.5f * srcustep;
	const float u4 = u2 + 1.5f * srcustep;

	const float u1 = u0 + hdu * std::min<float>(1.0f, (float)dstw / outw);
	const float u3 = u2 + hdu * std::min<float>(1.0f, (float)dstw / outw);
	const float u5 = u4 + hdu * std::min<float>(1.0f, (float)dstw / outw);

	const float vdu = (float)dstw / (float)dsttexw;
	const float vdv = (float)srch / (float)srctexh;

	const float v2 = vdv * offsety / outh;
	const float v0 = v2 - 1.5f / (float)srctexh;
	const float v4 = v2 + 1.5f / (float)srctexh;

	const float v1 = v0 + vdv * std::min<float>(1.0f, dsth / outh);
	const float v3 = v2 + vdv * std::min<float>(1.0f, dsth / outh);
	const float v5 = v4 + vdv * std::min<float>(1.0f, dsth / outh);

	const float filterTexStep = 1.0f / (float)filterTexWidth;
	const float hf0 = 0;
	const float hf1 = dstw * filterTexStep;
	const float vf0 = 0;
	const float vf1 = dsth * filterTexStep;

	const Vertex vxhoriz[4] = {
		// horizontal pass
		{ -1.0f, +1.0f, 0.0f, u0, 0.0f, u2, 0.0f, u4, 0.0f, hf0, 0.0f },
		{ -1.0f, -1.0f, 0.0f, u0, hdv,  u2, hdv,  u4, hdv,  hf0, 0.0f },
		{ +1.0f, +1.0f, 0.0f, u1, 0.0f, u3, 0.0f, u5, 0.0f, hf1, 0.0f },
		{ +1.0f, -1.0f, 0.0f, u1, hdv,  u3, hdv,  u5, hdv,  hf1, 0.0f },
	};
	
	const Vertex vxvert[4] = {
		// vertical pass
		{ -1.0f, +1.0f, 0.0f, 0.0f, v0, 0.0f, v2, 0.0f, v4, vf0, 1.0f },
		{ -1.0f, -1.0f, 0.0f, 0.0f, v1, 0.0f, v3, 0.0f, v5, vf1, 1.0f },
		{ +1.0f, +1.0f, 0.0f, vdu,  v0, vdu,  v2, vdu,  v4, vf0, 1.0f },
		{ +1.0f, -1.0f, 0.0f, vdu,  v1, vdu,  v3, vdu,  v5, vf1, 1.0f },
	};

	vdfastvector<uint32> texData(filterTexWidth * 2, 0);

	VDDisplayCreateBicubicTexture(texData.data(), dstw, srcw, offsetx, outw);
	VDDisplayCreateBicubicTexture(texData.data() + filterTexWidth, dsth, srch, offsety, outh);

	VDTInitData2D texFiltInitData = { texData.data(), (ptrdiff_t)(filterTexWidth * sizeof(uint32)) };
	if (!ctx.CreateTexture2D(filterTexWidth, 2, dctx.mBGRAFormat, 1, kVDTUsage_Default, &texFiltInitData, &mpFilterTex)) {
		Shutdown();
		return false;
	}

	VDDPoolTextureIndex filterTex = mMeshPool.RegisterTexture(mpFilterTex);
	mSourceTex = mMeshPool.RegisterTexture(nullptr);

	// do horizontal blit
	auto mb = mMeshPool.AddMesh(dctx);
	mb.InitSamplers({ dctx.mpSSBilinear, dctx.mpSSPoint, dctx.mpSSPoint });
	mb.InitTextures({ mSourceTex, mSourceTex, filterTex });
	mb.SetVertexProgram(VDTDataView(g_VDDispVP_StretchBltCubic));
	mb.SetFragmentProgram(VDTDataView(g_VDDispFP_StretchBltCubic));
	mb.SetVertexFormat(mpVF);
	mb.SetVertices(vxhoriz);
	mb.SetTopologyQuad();

	VDTViewport vp;
	vp.mX = 0;
	vp.mY = 0;
	vp.mWidth = mDstW;
	vp.mHeight = mSrcH;
	vp.mMinZ = 0.0f;
	vp.mMaxZ = 1.0f;
	mb.SetRenderView(tempTex, 0, false, vp);

	// do vertical blit
	auto mb2 = mMeshPool.AddMesh(dctx);
	mb2.InitTextures({ tempTex, tempTex, filterTex });
	mb2.InitSamplers({ dctx.mpSSBilinear, dctx.mpSSPoint, dctx.mpSSPoint });
	mb2.SetVertexProgram(VDTDataView(g_VDDispVP_StretchBltCubic));
	mb2.SetFragmentProgram(VDTDataView(g_VDDispFP_StretchBltCubic));
	mb2.SetVertexFormat(mpVF);
	mb2.SetVertices(vxvert);
	mb2.SetTopologyQuad();

	mOutputView = mMeshPool.RegisterRenderView({});
	mb2.SetRenderView(mOutputView);

	return true;
}

void VDDisplayStretchBicubicNode3D::Shutdown() {
	vdsaferelease <<= mpRTTHoriz, mpFilterTex, mpVF;
}

void VDDisplayStretchBicubicNode3D::Draw(IVDTContext& ctx, VDDisplayNodeContext3D& dctx, const VDDRenderView& renderView) {
	if (!mpFilterTex)
		return;

	IVDTTexture2D *srctex = mpSourceNode->Draw(ctx, dctx);

	if (!srctex)
		return;
	
	mMeshPool.SetRenderViewWithSubrect(mOutputView, renderView, std::max<float>(0.0f, mOutX), std::max<float>(0.0f, mOutY), mDstW, mDstH);
	mMeshPool.SetTexture(mSourceTex, srctex);
	mMeshPool.DrawAllMeshes(ctx, dctx);
}

///////////////////////////////////////////////////////////////////////////

constexpr uint32 VDDisplayScreenFXNode3D::kTessellation = 24;

VDDisplayScreenFXNode3D::VDDisplayScreenFXNode3D() {
}

VDDisplayScreenFXNode3D::~VDDisplayScreenFXNode3D() {
	Shutdown();
}

bool VDDisplayScreenFXNode3D::Init(IVDTContext& ctx, VDDisplayNodeContext3D& dctx, const Params& initParams, VDDisplaySourceNode3D *source) {
	mParams = initParams;

	mpSourceNode = source;

	mMapping = source->GetTextureMapping();

	const bool sharpBilinear = mParams.mbLinear && (mParams.mSharpnessX != 1.0f || mParams.mSharpnessY != 1.0f);
	const bool scanlines = (mParams.mScanlineIntensity > 0);

	mMeshPool.Clear();
	auto mb = mMeshPool.AddMesh(dctx);

	float u0 = mMapping.mTexelOffset.x;
	float v0 = mMapping.mTexelOffset.y;
	float u1 = u0 + mMapping.mTexelSize.x;
	float v1 = v0 + mMapping.mTexelSize.y;
	float u2 = 0.0f;
	float v2 = 0.0f;
	float u3 = 1.0f;
	float v3 = 1.0f;

	if (scanlines) {
		// When scanlines are active, the image is shifted up by a quarter of a scanline so that
		// the vertical space formerly occupied by a scanline is split into upper and lower halves,
		// with the upper half containing the bright part of each scanline and the lower half having
		// the dark region between scanlines. Besides maintaining consistency with the CPU,
		// this avoids an unwanted case with exact 2:1 sizing where the standard mapping would result
		// in both half scanlines mapping 25%/75% between scanlines, giving a blurry image. Applying
		// a 1/4 scan offset gives crisp half-scanlines on and off of each scanline instead.

		v0 += 0.25f;
		v1 += 0.25f;
	}

	if (!sharpBilinear) {
		u0 /= (float)mMapping.mTexWidth;
		u1 /= (float)mMapping.mTexWidth;
		v0 /= (float)mMapping.mTexHeight;
		v1 /= (float)mMapping.mTexHeight;
	}

	static constexpr VDTDataView kFPSources[] = {
		VDTDataView(g_VDDispFP_ScreenFX_PtLinear_NoScanlines_Linear),
		VDTDataView(g_VDDispFP_ScreenFX_PtLinear_NoScanlines_Gamma),
		VDTDataView(g_VDDispFP_ScreenFX_PtLinear_NoScanlines_CC),
		VDTDataView(g_VDDispFP_ScreenFX_PtLinear_NoScanlines_GammaCC),
		VDTDataView(g_VDDispFP_ScreenFX_PtLinear_Scanlines_Linear),
		VDTDataView(g_VDDispFP_ScreenFX_PtLinear_Scanlines_Gamma),
		VDTDataView(g_VDDispFP_ScreenFX_PtLinear_Scanlines_CC),
		VDTDataView(g_VDDispFP_ScreenFX_PtLinear_Scanlines_GammaCC),
		VDTDataView(g_VDDispFP_ScreenFX_Sharp_NoScanlines_Linear),
		VDTDataView(g_VDDispFP_ScreenFX_Sharp_NoScanlines_Gamma),
		VDTDataView(g_VDDispFP_ScreenFX_Sharp_NoScanlines_CC),
		VDTDataView(g_VDDispFP_ScreenFX_Sharp_NoScanlines_GammaCC),
		VDTDataView(g_VDDispFP_ScreenFX_Sharp_Scanlines_Linear),
		VDTDataView(g_VDDispFP_ScreenFX_Sharp_Scanlines_Gamma),
		VDTDataView(g_VDDispFP_ScreenFX_Sharp_Scanlines_CC),
		VDTDataView(g_VDDispFP_ScreenFX_Sharp_Scanlines_GammaCC),
	};

	uint32 fpMode
		= (sharpBilinear ? 8 : 0)
		+ (mParams.mScanlineIntensity > 0 ? 4 : 0)
		+ (mParams.mbRenderLinear ? 2 : mParams.mColorCorrectionMatrix[0][0] != 0 ? 3 : mParams.mGamma != 1.0f ? 1 : 0);

	mb.SetVertexProgram(VDTDataView(g_VDDispVP_ScreenFX));
	mb.SetFragmentProgram(kFPSources[fpMode]);

	float gamma = mParams.mGamma;
	bool gammaHasSrgb = mParams.mColorCorrectionMatrix[0][0] != 0;

	vdsaferelease <<= mpGammaRampTex;

	uint32 gammaTex[256];
	VDDisplayCreateGammaRamp(gammaTex, 256, gammaHasSrgb, mParams.mbUseAdobeRGB, gamma);

	VDTInitData2D initData { gammaTex, sizeof(gammaTex) };

	if (!ctx.CreateTexture2D(256, 1, dctx.mBGRAFormat, 1, kVDTUsage_Default, &initData, &mpGammaRampTex)) {
		Shutdown();
		return false;
	}

	// Compute the scanline mask texture.
	//
	// The scanline mask texture is a texture mapped vertically to the destination
	// rect, supplying the scanline intensity for each pixel row. It is used
	// instead of a repeating texture to reduce aliasing effects, since we can more
	// accurately pre-interpolate on the CPU rather than in the pixel shader.
	//
	// Scanlines are modeled as a raised cosine wave centered on each beam scan,
	// multiplied onto the video output image. The multiplication is conceptually
	// done in linear space, but the math is transformed to push the gamma correction
	// into the texture where we can precompute it.

	uint32 scanlineMaskH = std::max<uint32>(1, mParams.mClipDstH);
	uint32 scanlineMaskTexH = scanlineMaskH;

	// If the device doesn't support pow2, round up the texture size. We'll only
	// use the top part.
	if (!ctx.GetDeviceCaps().mbNonPow2) {
		scanlineMaskTexH = VDCeilToPow2(scanlineMaskTexH);
	}

	vdblock<uint32> imageMask(scanlineMaskTexH);
	VDDisplayCreateScanlineMaskTexture(imageMask.data(), sizeof(imageMask[0]), mParams.mSrcH, scanlineMaskH, mParams.mDstY - mParams.mClipDstY, mParams.mDstH, scanlineMaskTexH, mParams.mScanlineIntensity, mParams.mbRenderLinear);

	mScanlineMaskVScale = (float)mParams.mDstH / (float)scanlineMaskTexH;
	mScanlineMaskVBase = (mParams.mDstY - mParams.mClipDstY) / (float)scanlineMaskTexH;

	VDTInitData2D initDataScanlineMask { imageMask.data(), sizeof(imageMask[0]) };
	if (!ctx.CreateTexture2D(1, scanlineMaskTexH, dctx.mBGRAFormat, 1, kVDTUsage_Default, &initDataScanlineMask, &mpScanlineMaskTex)) {
		Shutdown();
		return false;
	}

	const bool useDistortion = mParams.mDistortionX > 0;

	mb.InitSamplers({
		// source
		mParams.mbLinear ? dctx.mpSSBilinear : dctx.mpSSPoint,
		mParams.mDistortionX > 0 ? dctx.mpSSBilinear : dctx.mpSSPoint,
		dctx.mpSSBilinear
	});

	const float ccOverride = mParams.mColorCorrectionMatrix[0][0] == 0 && mParams.mbRenderLinear ? 1.0f : 0.0f;

	VDDisplayDistortionMapping distortionMapping;
	distortionMapping.Init(mParams.mDistortionX, mParams.mDistortionYRatio, (float)mParams.mDstW / (float)mParams.mDstH);

	const float ccPreScale = mParams.mbSignedInput ? 255.0f / 127.0f : 1.0f;
	const float ccPreOffset = mParams.mbSignedInput ? -64.0f / 127.0f : 0.0f;
	const float ccScale = mParams.mbRenderLinear ? mParams.mHDRScale : 1.0f;

	float params[4*6]={
		// regular blit parameters
		mParams.mSharpnessX,
		mParams.mSharpnessY,
		1.0f / (float)mMapping.mTexHeight,
		1.0f / (float)mMapping.mTexWidth,

		// distortion info
		distortionMapping.mScaleX,
		distortionMapping.mScaleY,
		distortionMapping.mSqRadius,
		mScanlineMaskVBase,

		// sharp bilinear texel addressing info 
		sharpBilinear ? mMapping.mTexelSize.x : mMapping.mUVSize.x,
		sharpBilinear ? mMapping.mTexelSize.y : mMapping.mUVSize.y,
		mParams.mScanlineIntensity > 0 ? sharpBilinear ? 0.25f : 0.25f / (float)mMapping.mTexHeight : 0.0f,
		mScanlineMaskVScale,

		// color correction matrix
		(mParams.mColorCorrectionMatrix[0][0] + ccOverride) * ccScale,
		mParams.mColorCorrectionMatrix[1][0] * ccScale,
		mParams.mColorCorrectionMatrix[2][0] * ccScale,
		ccPreOffset,

		mParams.mColorCorrectionMatrix[0][1] * ccScale,
		(mParams.mColorCorrectionMatrix[1][1] + ccOverride) * ccScale,
		mParams.mColorCorrectionMatrix[2][1] * ccScale,
		ccPreScale,

		mParams.mColorCorrectionMatrix[0][2] * ccScale,
		mParams.mColorCorrectionMatrix[1][2] * ccScale,
		(mParams.mColorCorrectionMatrix[2][2] + ccOverride) * ccScale,
		0,
	};

	mb.SetFPConstData(params);
	
	float vsParams[16] = {
		mScanlineMaskVScale,
		mScanlineMaskVBase,
		0,
		0,

		1.0f,
		useDistortion ? -0.5f : 0.0f,
		0.0f,
		0.0f,
	
		// distortion info
		distortionMapping.mScaleX,
		distortionMapping.mScaleY,
		distortionMapping.mSqRadius,
		0,

		// output transform
		2.0f,
		-2.0f,
		1.0f,
		-1.0f
	};

	mb.SetVPConstData(vsParams);
	mb.SetVertexFormat(dctx.mpVFTexture2T);

	mOutputViewId = mMeshPool.RegisterRenderView({});
	mb.SetRenderView(mOutputViewId);

	mSourceTextureIndex = mMeshPool.RegisterTexture(nullptr);
	mb.InitTextures({ mSourceTextureIndex, mMeshPool.RegisterTexture(mpScanlineMaskTex), mMeshPool.RegisterTexture(mpGammaRampTex) });

	if (useDistortion) {
		vdblock<VDDisplayVertex2T3D> vdat((kTessellation+1)*(kTessellation+1));

		int vxidx = 0;
		for(int y=0; y<=kTessellation; ++y) {
			const float yf1 = (float)y / (float)kTessellation;
			const float yf0 = 1.0f - yf1;

			for(int x=0; x<=kTessellation; ++x) {
				const float xf1 = (float)x / (float)kTessellation;
				const float xf0 = 1.0f - xf1;
				const vdfloat2 uv1 { 
					u2 * xf0 + u3 * xf1,
					v2 * yf0 + v3 * yf1
				};

				vdfloat2 v = uv1 - vdfloat2 { 0.5f, 0.5f };
				vdfloat2 v2 = v * vdfloat2 { distortionMapping.mScaleX, distortionMapping.mScaleY };
				vdfloat2 pos = v * sqrtf(distortionMapping.mSqRadius / (1.0f + nsVDMath::dot(v2, v2))) + vdfloat2 { 0.5f, 0.5f };

				vdat[vxidx++] = {
					pos.x * 2.0f - 1.0f,
					pos.y * -2.0f + 1.0f,
					0.0f,
					u0 * xf0 + u1 * xf1,
					v0 * yf0 + v1 * yf1,
					uv1.x,
					uv1.y
				};
			}
		}

		mb.SetVertices(vdat.data(), vdat.size());

		vdblock<uint16> idat(kTessellation*kTessellation*6);

		int i = 0;
		for(int y=0; y<(int)kTessellation; ++y) {
			int idx0 = y*(int)(kTessellation + 1);
			int idx1 = idx0 + (int)(kTessellation + 1);

			for(int x=0; x<(int)kTessellation; ++x, ++idx0, ++idx1) {
				idat[i++] = idx0;
				idat[i++] = idx1;
				idat[i++] = idx0 + 1;
				idat[i++] = idx0 + 1;
				idat[i++] = idx1;
				idat[i++] = idx1 + 1;
			}
		}

		mb.SetClear(0);
		mb.SetTopology(idat.data(), idat.size() / 3);
	} else {
		const VDDisplayVertex2T3D verts[] = {
			{ -1.0f, +1.0f, 0.0f, u0, v0, u2, v2 },
			{ -1.0f, -1.0f, 0.0f, u0, v1, u2, v3 },
			{ +1.0f, +1.0f, 0.0f, u1, v0, u3, v2 },
			{ +1.0f, -1.0f, 0.0f, u1, v1, u3, v3 },
		};

		mb.SetVertices(verts);
		mb.SetTopologyQuad();
	}

	return true;
}

void VDDisplayScreenFXNode3D::Shutdown() {
	vdsaferelease <<= mpSourceNode, mpGammaRampTex, mpScanlineMaskTex;
}

void VDDisplayScreenFXNode3D::Draw(IVDTContext& ctx, VDDisplayNodeContext3D& dctx, const VDDRenderView& renderView) {
	IVDTTexture2D *src = mpSourceNode->Draw(ctx, dctx);

	if (!src)
		return;

	VDTAutoScope scope(ctx, "ScreenFX");
	
	mMeshPool.SetTexture(mSourceTextureIndex, src);

	VDDRenderView newRenderView(renderView);
	newRenderView.mSoftViewport.mOffset += vdfloat2 { mParams.mDstX, mParams.mDstY };
	newRenderView.mSoftViewport.mSize = vdfloat2 { mParams.mDstW, mParams.mDstH };
	newRenderView.mViewport.mWidth = mParams.mClipDstX + mParams.mClipDstW;
	newRenderView.mViewport.mHeight = mParams.mClipDstY + mParams.mClipDstH;

	mMeshPool.SetRenderView(mOutputViewId, newRenderView);

	mMeshPool.DrawAllMeshes(ctx, dctx);
}

///////////////////////////////////////////////////////////////////////////

VDDisplayArtifactingNode3D::VDDisplayArtifactingNode3D() {
}

VDDisplayArtifactingNode3D::~VDDisplayArtifactingNode3D() {
	Shutdown();
}

bool VDDisplayArtifactingNode3D::Init(IVDTContext& ctx, VDDisplayNodeContext3D& dctx, float dy, bool extendedOutput, VDDisplaySourceNode3D *source) {
	mpSourceNode = source;

	mMeshPool.Clear();
	auto mb = mMeshPool.AddMesh(dctx);

	mMapping = source->GetTextureMapping();

	const float u0 = mMapping.mUVOffset.x;
	const float v0 = mMapping.mUVOffset.y;
	const float u1 = u0 + mMapping.mUVSize.x;
	const float v1 = v0 + mMapping.mUVSize.y;
	const float u2 = u0;
	const float v2 = v0 + dy / mMapping.mTexHeight;
	const float u3 = u1;
	const float v3 = v1 + dy / mMapping.mTexHeight;

	const VDDisplayVertex2T3D vx[4]={
		{ -1.0f, +1.0f, 0.0f, u0, v0, u2, v2 },
		{ -1.0f, -1.0f, 0.0f, u0, v1, u2, v3 },
		{ +1.0f, +1.0f, 0.0f, u1, v0, u3, v2 },
		{ +1.0f, -1.0f, 0.0f, u1, v1, u3, v3 },
	};

	mb.SetVertices(vx);
	mb.SetTopologyQuad();

	mb.SetVertexFormat(dctx.mpVFTexture2T);
	mb.SetVertexProgram(dctx.mpVPTexture2T);
	mb.SetFragmentProgram(extendedOutput ? VDTDataView(g_VDDispFP_PALArtifacting_ExtendedOutput) : VDTDataView(g_VDDispFP_PALArtifacting_NormalOutput));

	mb.InitSamplers({dctx.mpSSPoint});

	mSourceTextureIndex = mMeshPool.RegisterTexture(nullptr);
	mb.InitTextures({mSourceTextureIndex});

	return true;
}

void VDDisplayArtifactingNode3D::Shutdown() {
}

void VDDisplayArtifactingNode3D::Draw(IVDTContext& ctx, VDDisplayNodeContext3D& dctx, const VDDRenderView& renderView) {
	IVDTTexture2D *src = mpSourceNode->Draw(ctx, dctx);

	if (!src)
		return;

	const auto& texMapping = mpSourceNode->GetTextureMapping();
	
	VDTAutoScope scope(ctx, "Artifacting");

	dctx.ApplyRenderViewWithSubrect(renderView, 0, 0, texMapping.mTexelSize.x, texMapping.mTexelSize.y);

	mMeshPool.SetTexture(mSourceTextureIndex, src);
	mMeshPool.DrawAllMeshes(ctx, dctx);
}

///////////////////////////////////////////////////////////////////////////

VDDisplayBloomNode3D::VDDisplayBloomNode3D() {
}

VDDisplayBloomNode3D::~VDDisplayBloomNode3D() {
	Shutdown();
}

bool VDDisplayBloomNode3D::Init(IVDTContext& ctx, VDDisplayNodeContext3D& dctx, const Params& params, VDDisplaySourceNode3D *source) {
	mMeshPool.Clear();

	mParams = params;

	if (mpSourceNode != source) {
		if (mpSourceNode)
			mpSourceNode->Release();

		mpSourceNode = source;
		source->AddRef();
	}

	mMapping = source->GetTextureMapping();

	// Compute source UV for clipped source rect.
	const uint32 w = params.mClipW;
	const uint32 h = params.mClipH;

	// Clip source rect.
	const float srcu  = (params.mClipX - params.mDstX) / params.mDstW * mMapping.mUVSize.x + mMapping.mUVOffset.x;
	const float srcv  = (params.mClipY - params.mDstY) / params.mDstH * mMapping.mUVSize.y + mMapping.mUVOffset.y;
	const float srcdu =  params.mClipW                 / params.mDstW * mMapping.mUVSize.x;
	const float srcdv =  params.mClipH                 / params.mDstH * mMapping.mUVSize.y;

	// Compute blur prescaler.
	//
	// We can do a max radius-7 blur in the main passes, so above that we must use
	// the prescaler. The prescaler itself uses 4 bilinear samples with some overlap,
	// so reasonably it can accommodate up to about 4x before we start to get
	// aliasing, and we can sort of get away with 8x. From a quality standpoint, it
	// is better to keep the prescaler low and the blur filter high; performance is
	// the opposite.

	uint32 factor1 = 1;
	uint32 factor2 = 1;
	float prescaleOffset = 0.0f;

	mbPrescale2x = false;

	if (mParams.mBlurRadius > 56) {
		prescaleOffset = 1.0f;
		factor1 = 16;
		factor2 = 4;
		mbPrescale2x = true;
	} else if (mParams.mBlurRadius > 28) {
		prescaleOffset = 1.0f;
		factor1 = 8;
		factor2 = 4;
		mbPrescale2x = true;
	} else if (mParams.mBlurRadius > 14) {
		factor1 = 4;
		factor2 = 4;
		prescaleOffset = 1.0f;
	} else if (mParams.mBlurRadius > 7) {
		factor1 = 2;
		factor2 = 2;
		prescaleOffset = 0.5f;
	} else {
		factor1 = 1;
		factor2 = 1;
		prescaleOffset = 0.0f;
	}

	const uint32 blurW1 = std::max<uint32>((w + factor1 - 1)/factor1, 1);
	const uint32 blurH1 = std::max<uint32>((h + factor1 - 1)/factor1, 1);
	const uint32 blurW2 = mbPrescale2x ? blurW1 * (factor1/factor2) : blurW1;
	const uint32 blurH2 = mbPrescale2x ? blurH1 * (factor1/factor2) : blurH1;

	mBlurW = blurW1;
	mBlurH = blurH1;
	mBlurW2 = blurW2;
	mBlurH2 = blurH2;

	const auto& caps = ctx.GetDeviceCaps();
	uint32 texw1 = caps.mbNonPow2 ? blurW1 : VDCeilToPow2(blurW1);
	uint32 texh1 = caps.mbNonPow2 ? blurH1 : VDCeilToPow2(blurH1);
	uint32 texw2 = caps.mbNonPow2 ? blurW2 : VDCeilToPow2(blurW2);
	uint32 texh2 = caps.mbNonPow2 ? blurH2 : VDCeilToPow2(blurH2);

	struct Blit {
		constexpr static Blit From2UVOffsetSize(const vdfloat2& uvoffset1, const vdfloat2& uvsize1, const vdfloat2& uvoffset2, const vdfloat2& uvsize2) {
			const float u0 = uvoffset1.x;
			const float v0 = uvoffset1.y;
			const float u1 = u0 + uvsize1.x;
			const float v1 = v0 + uvsize1.y;
			const float u2 = uvoffset2.x;
			const float v2 = uvoffset2.y;
			const float u3 = u2 + uvsize2.x;
			const float v3 = v2 + uvsize2.y;

			return Blit {{
				{ -1.0f, +1.0f, 0.0f, u0, v0, u2, v2 },
				{ -1.0f, -1.0f, 0.0f, u0, v1, u2, v3 },
				{ +1.0f, +1.0f, 0.0f, u1, v0, u3, v2 },
				{ +1.0f, -1.0f, 0.0f, u1, v1, u3, v3 },
			}};
		}

		constexpr static Blit From2UVRects(float du1, float dv1, float du2 = 0, float dv2 = 0) {
			return Blit {{
				{ -1.0f, +1.0f, 0.0f, 0,   0,   0,   0   },
				{ -1.0f, -1.0f, 0.0f, 0,   dv1, 0,   dv2 },
				{ +1.0f, +1.0f, 0.0f, du1, 0,   du2, 0   },
				{ +1.0f, -1.0f, 0.0f, du1, dv1, du2, dv2 },
			}};
		}

		VDDisplayVertex2T3D vx[4];
	};

	// Compute blur filter kernel.
	//
	// Note that this is half of a symmetric kernel. The seventh tap (w6) is the center
	// tap. The other six taps are grouped as bilinear tap pairs, so they come in pairs;
	// an even half-kernel is the same cost as the next odd half-kernel, so we always
	// create an odd length half-kernel.

	const float mainBlurRadius = mParams.mBlurRadius / (float)factor1;
	const float blurHeightScale = 1.5f / std::min<float>(7.0f, mainBlurRadius);

	const float wf = expf(-7.5f * (1.5f / 7.0f));
	const float w0 = std::max<float>(0, expf(-7.0f * blurHeightScale) - wf);
	const float w1 = std::max<float>(0, expf(-6.0f * blurHeightScale) - wf);
	const float w2 = std::max<float>(0, expf(-5.0f * blurHeightScale) - wf);
	const float w3 = std::max<float>(0, expf(-4.0f * blurHeightScale) - wf);
	const float w4 = std::max<float>(0, expf(-3.0f * blurHeightScale) - wf);
	const float w5 = std::max<float>(0, expf(-2.0f * blurHeightScale) - wf);
	const float w6 = std::max<float>(0, expf(-1.0f * blurHeightScale) - wf);
	const float wscale = 1.0f / (w6 + 2*(w5+w4+w3+w2+w1));

	// fill out constant buffers
	float vpConstants1[12] {};
	float vpConstants2[12] {};
	float vpConstants3[12] {};
	float fpConstants[8] {};

	vpConstants1[0] = prescaleOffset / (float)mMapping.mTexWidth;
	vpConstants1[1] = prescaleOffset / (float)mMapping.mTexHeight;

	vpConstants2[0] = prescaleOffset / (float)texw2;
	vpConstants2[1] = prescaleOffset / (float)texh2;

	const float blurUStep = 1.0f / (float)texw1;
	const float blurVStep = 1.0f / (float)texh1;
	const float filterOffset0 = 1.0f + w4 / std::max<float>(w4 + w5, 1e-10f);
	const float filterOffset1 = 3.0f + w2 / std::max<float>(w2 + w3, 1e-10f);
	const float filterOffset2 = 5.0f + w0 / std::max<float>(w0 + w1, 1e-10f);

	vpConstants2[2] = filterOffset0 * blurUStep;
	vpConstants2[4] = filterOffset1 * blurUStep;
	vpConstants2[6] = filterOffset2 * blurUStep;

	vpConstants3[3] = filterOffset0 * blurVStep;
	vpConstants3[5] = filterOffset1 * blurVStep;
	vpConstants3[7] = filterOffset2 * blurVStep;

	fpConstants[0] = w6 * wscale;
	fpConstants[1] = (w5+w4) * wscale;
	fpConstants[2] = (w3+w2) * wscale;
	fpConstants[3] = (w1+w0) * wscale;

	fpConstants[4] = 1.0f + mParams.mThreshold;
	fpConstants[5] = -mParams.mThreshold;
	fpConstants[6] = mParams.mDirectIntensity;
	fpConstants[7] = mParams.mIndirectIntensity;

	////////////////////////////////

	mSourceTextureIndex = mMeshPool.RegisterTexture(nullptr);

	const auto rttFormat = mParams.mbRenderLinear ? dctx.mHDRFormat : dctx.mBGRASRGBFormat;
	VDDPoolTextureIndex rtt1 = mMeshPool.AddTempTexture(ctx, texw1, texh1, rttFormat, 1);
	VDDPoolTextureIndex rtt2 = mMeshPool.AddTempTexture(ctx, texw1, texh1, rttFormat, 1);

	// prescale pass: source -> prescale RTT or RTT 1
	VDDPoolTextureIndex curTexture = mSourceTextureIndex;
	auto mb = mMeshPool.AddMesh(dctx);
	mb.SetVertexProgram(VDTDataView(g_VDDispVP_Bloom1));
	mb.SetFragmentProgram(mParams.mbRenderLinear ? VDTDataView(g_VDDispFP_Bloom1_Linear) : VDTDataView(g_VDDispFP_Bloom1_Gamma));
	mb.SetVPConstData(vpConstants1);
	mb.SetFPConstData(fpConstants);

	mb.InitSamplers({dctx.mpSSBilinear});
	mb.InitTextures({curTexture});

	if (mbPrescale2x)
		curTexture = mMeshPool.AddTempTexture(ctx, texw2, texh2, rttFormat, 1);
	else
		curTexture = rtt1;

	mb.SetRenderView(curTexture, 0, false, VDTViewport { 0, 0, mBlurW2, mBlurH2, 0.0f, 1.0f });
	mb.SetVertexFormat(dctx.mpVFTexture2T);
	mb.SetVertices(
		Blit::From2UVOffsetSize(
			vdfloat2 { srcu, srcv },
			vdfloat2 {
				(float)(blurW2 * factor2) / (float)mMapping.mTexWidth,
				(float)(blurH2 * factor2) / (float)mMapping.mTexHeight,
			},
			vdfloat2 { 0, 0 },
			vdfloat2 { 0, 0 }
		).vx
	);
	mb.SetTopologyQuad();

	// prescale 2 pass: prescale RTT -> RTT 1
	if (mbPrescale2x) {
		mb = mMeshPool.AddMesh(dctx);
		mb.SetVPConstData(vpConstants2);
		mb.SetFPConstDataReuse();
		mb.SetVertexProgram(VDTDataView(g_VDDispVP_Bloom1));
		mb.SetFragmentProgram(VDTDataView(g_VDDispFP_Bloom1A));
		mb.SetVertexFormat(dctx.mpVFTexture2T);
		mb.SetVertices(
			Blit::From2UVRects((float)blurW2 / (float)texw2, (float)blurH2 / (float)texh2).vx
		);
		mb.InitTextures({curTexture});
		mb.SetRenderView(rtt1, 0, false, VDTViewport { 0, 0, mBlurW, mBlurH, 0.0f, 1.0f });
		mb.SetTopologyQuad();
		curTexture = rtt1;
	}

	// horizontal blur pass: RTT 1 -> RTT 2
	mb = mMeshPool.AddMesh(dctx);
	mb.InitSamplers({dctx.mpSSBilinear});
	mb.SetVertexProgram(VDTDataView(g_VDDispVP_Bloom2));
	mb.SetFragmentProgram(VDTDataView(g_VDDispFP_Bloom2));
	mb.SetVPConstData(vpConstants2);
	mb.SetFPConstDataReuse();
	mb.SetVertexFormat(dctx.mpVFTexture2T);
	mb.SetVertices(
		Blit::From2UVRects((float)blurW1 / (float)texw1, (float)blurH1 / (float)texh1).vx
	);
	mb.InitTextures({rtt1});
	mb.SetRenderView(rtt2, 0, false, VDTViewport { 0, 0, mBlurW, mBlurH, 0.0f, 1.0f });
	mb.SetTopologyQuad();
	
	// vertical blur pass: RTT 2 -> RTT 1
	mb = mMeshPool.AddMesh(dctx);
	mb.SetVertexProgram(VDTDataView(g_VDDispVP_Bloom2));
	mb.SetFragmentProgram(VDTDataView(g_VDDispFP_Bloom2));
	mb.SetVPConstData(vpConstants3);
	mb.SetFPConstDataReuse();
	mb.InitTextures({rtt2});
	mb.SetRenderView(rtt1, 0, false, VDTViewport { 0, 0, mBlurW, mBlurH, 0.0f, 1.0f });
	mb.SetVertexFormat(dctx.mpVFTexture2T);
	mb.SetVertices(
		Blit::From2UVRects((float)blurW1 / (float)texw1, (float)blurH1 / (float)texh1).vx
	);
	mb.SetTopologyQuad();

	// final pass: RTT 1 -> dest
	mb = mMeshPool.AddMesh(dctx);
		
	mb.InitSamplers({dctx.mpSSBilinear, dctx.mpSSPoint});
	mb.SetVertexProgram(VDTDataView(g_VDDispVP_Bloom3));
	mb.SetFragmentProgram(params.mbRenderLinear ? VDTDataView(g_VDDispFP_Bloom3_Linear) : VDTDataView(g_VDDispFP_Bloom3_Gamma));
	mb.SetVPConstDataReuse();
	mb.SetFPConstDataReuse();

	mb.SetVertexFormat(dctx.mpVFTexture2T);
	mb.SetVertices(
		Blit::From2UVOffsetSize(
			vdfloat2 { srcu, srcv },
			vdfloat2 { srcdu, srcdv },
			vdfloat2 { 0, 0 },
			vdfloat2 {
				((float)w / (float)factor1) / (float)texw1,
				((float)h / (float)factor1) / (float)texh1
			}
		).vx
	);

	mOutputViewId = mMeshPool.RegisterRenderView(VDDRenderView{});
	mb.SetRenderView(mOutputViewId);
	mb.InitTextures({ rtt1, mSourceTextureIndex });
	mb.SetTopologyQuad();

	return mMeshPool;
}

void VDDisplayBloomNode3D::Shutdown() {
	mMeshPool.Clear();
	vdsaferelease <<= mpSourceNode;
}

void VDDisplayBloomNode3D::Draw(IVDTContext& ctx, VDDisplayNodeContext3D& dctx, const VDDRenderView& renderView) {
	mMeshPool.SetRenderViewWithSubrect(mOutputViewId, renderView, mParams.mClipX, mParams.mClipY, mParams.mClipW, mParams.mClipH);

	IVDTTexture2D *src = mpSourceNode->Draw(ctx, dctx);

	if (!src)
		return;

	VDTAutoScope scope(ctx, "Bloom");

	mMeshPool.SetTexture(mSourceTextureIndex, src);

	mMeshPool.DrawAllMeshes(ctx, dctx);
}
