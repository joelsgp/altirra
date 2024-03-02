#ifndef f_VD2_VDDISPLAY_DISPLAYDRV3D_H
#define f_VD2_VDDISPLAY_DISPLAYDRV3D_H

#include <vd2/system/refcount.h>
#include <vd2/system/vdstl.h>
#include <vd2/VDDisplay/displaydrv.h>
#include <vd2/VDDisplay/renderer.h>
#include <vd2/Tessa/Context.h>
#include "displaynode3d.h"
#include "renderer3d.h"

struct VDPixmap;
class IVDTContext;
class IVDTTexture2D;

///////////////////////////////////////////////////////////////////////////

class VDDisplayDriver3D : public VDVideoDisplayMinidriver, public IVDTAsyncPresent {
	VDDisplayDriver3D(const VDDisplayDriver3D&);
	VDDisplayDriver3D& operator=(const VDDisplayDriver3D&);
public:
	VDDisplayDriver3D();
	~VDDisplayDriver3D();

	virtual bool Init(HWND hwnd, HMONITOR hmonitor, const VDVideoDisplaySourceInfo& info);
	virtual void Shutdown();

	virtual bool ModifySource(const VDVideoDisplaySourceInfo& info);

	virtual void SetFilterMode(FilterMode mode);
	virtual void SetFullScreen(bool fullscreen, uint32 w, uint32 h, uint32 refresh);
	virtual void SetDestRect(const vdrect32 *r, uint32 color);
	virtual void SetPixelSharpness(float xfactor, float yfactor);

	virtual bool IsValid();
	virtual bool IsFramePending();
	virtual bool Resize(int w, int h);
	virtual bool Update(UpdateMode);
	virtual void Refresh(UpdateMode);
	virtual bool Paint(HDC hdc, const RECT& rClient, UpdateMode lastUpdateMode);
	virtual void PresentQueued();

public:
	virtual void QueuePresent();

private:
	bool CreateSwapChain();
	bool CreateImageNode();
	void DestroyImageNode();
	bool RebuildTree();

	HWND mhwnd;
	HMONITOR mhMonitor;
	IVDTContext *mpContext;
	IVDTSwapChain *mpSwapChain;
	VDDisplayImageNode3D *mpImageNode;
	VDDisplayImageSourceNode3D *mpImageSourceNode;
	VDDisplayNode3D *mpRootNode;

	FilterMode mFilterMode;
	bool mbCompositionTreeDirty;
	bool mbFramePending;

	bool mbFullScreen;
	uint32 mFullScreenWidth;
	uint32 mFullScreenHeight;
	uint32 mFullScreenRefreshRate;

	VDVideoDisplaySourceInfo mSource;

	VDDisplayNodeContext3D mDisplayNodeContext;
	VDDisplayRenderer3D mRenderer;
};

#endif
