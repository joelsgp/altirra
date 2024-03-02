//	Altirra - Atari 800/800XL emulator
//	UI library
//	Copyright (C) 2008-2015 Avery Lee
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

#ifndef AT_UIFRAME_H
#define AT_UIFRAME_H

#ifdef _MSC_VER
	#pragma once
#endif

#include <vd2/system/refcount.h>
#include <vd2/system/vdstl.h>
#include <vd2/system/vectors.h>
#include <vd2/system/VDString.h>
#include "uinativewindow.h"

// Sent to the top-level window prior to processing a key event;
// LPARAM = &msg. Returns true if handled, false if not.
#define ATWM_PRETRANSLATE	(WM_APP + 200)

// Sent to child/owned windows when fonts are updated at container
// level.
#define ATWM_FONTSUPDATED	(WM_APP + 201)

// Sent to a window to request its ideal size; LPARAM = &SIZE.
// Returns true if handled, false if not.
#define ATWM_GETAUTOSIZE	(WM_APP + 202)

// Sent to a dockable pane to indicate that its full-screen state
// has changed. WPARAM = (bool)state.
#define ATWM_SETFULLSCREEN	(WM_APP + 203)

// Internal message used by dockable frame system to handle move
// tracking.
#define ATWM_ENDTRACKING	(WM_APP + 204)

// Tunneled from ancestors to focus. Return is true if handled,
// false if not.
#define ATWM_PRESYSKEYDOWN	(WM_APP + 205)
#define ATWM_PRESYSKEYUP	(WM_APP + 206)
#define ATWM_PREKEYDOWN		(WM_APP + 207)
#define ATWM_PREKEYUP		(WM_APP + 208)

// Sent to top-level window prior to processing WM_SYSCHAR. Returns
// true to re-route to top-level window directly, false to not.
#define ATWM_QUERYSYSCHAR	(WM_APP + 209)

// Invalidate layout of parent window.
#define ATWM_INVLAYOUT		(WM_APP + 210)

// Sent when a modal loop begins to warn that key messages may be
// missed.
#define ATWM_FORCEKEYSUP	(WM_APP + 211)

// Posted to indicate when a KEYUP message occurs relative to
// the CHAR message translated from the original KEYDOWN. wParam
// is unused; lParam is the same as the KEYUP message. This message
// is not created by the UI system; it must be generated by window
// procedures.
#define ATWM_CHARUP			(WM_APP + 212)

void ATInitUIFrameSystem();
void ATShutdownUIFrameSystem();

class ATContainerWindow;
class ATContainerDockingPane;
class ATFrameWindow;

enum {
	kATContainerDockCenter,
	kATContainerDockLeft,
	kATContainerDockRight,
	kATContainerDockTop,
	kATContainerDockBottom
};

class ATContainerResizer {
public:
	ATContainerResizer();

	void LayoutWindow(HWND hwnd, int x, int y, int width, int height, bool visible);
	void ResizeWindow(HWND hwnd, int width, int height);

	void Flush();

protected:
	HDWP mhdwp;
	vdfastvector<HWND> mWindowsToShow;
};

class ATContainerSplitterBar final : public ATUINativeWindow {
public:
	ATContainerSplitterBar();
	~ATContainerSplitterBar();

	bool Init(HWND hwndParent, ATContainerDockingPane *pane, bool vertical);
	void Shutdown();

protected:
	static LRESULT StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	void OnPaint();
	void OnSize();
	void OnLButtonDown(WPARAM wParam, int x, int y);
	void OnLButtonUp(WPARAM wParam, int x, int y);
	void OnMouseMove(WPARAM wParam, int x, int y);
	void OnCaptureChanged(HWND hwndNewCapture);

	virtual bool IsTouchHitTestCapable() const;

	ATContainerDockingPane *mpControlledPane;
	bool	mbVertical;
	int		mDistanceOffset;

};

class ATDragHandleWindow final : public ATUINativeWindow {
public:
	ATDragHandleWindow();
	~ATDragHandleWindow();

	VDGUIHandle Create(int x, int y, int cx, int cy, VDGUIHandle parent, int id);
	void Destroy();

	int HitTest(int screenX, int screenY);

protected:
	LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

	void OnCreate();
	void OnMove();
	void OnPaint();

	int mX;
	int mY;
};

class ATContainerDockingPane final : public vdrefcounted<IVDRefCount> {
public:
	ATContainerDockingPane(ATContainerWindow *parent);
	~ATContainerDockingPane();

	const vdrect32& GetArea() const { return mArea; }
	void	SetArea(ATContainerResizer& resizer, const vdrect32& area, bool parentContainsFullScreen);

	void	Clear();

	void	InvalidateLayoutAll();
	void	InvalidateLayout();
	void	UpdateLayout(ATContainerResizer& resizer);
	void	Relayout(ATContainerResizer& resizer);

	bool	GetFrameSizeForContent(vdsize32& sz);

	// Docking panes automatically disappear when they have no content unless they
	// are pinned.
	void	SetPinned(bool pinned) { mbPinned = pinned; }

	int		GetDockCode() const;
	float	GetDockFraction() const;
	void	SetDockFraction(float dist);

	ATContainerDockingPane *GetParentPane() const { return mpDockParent; }

	uint32	GetContentCount() const;
	ATFrameWindow *GetContent(uint32 idx) const;
	ATFrameWindow *GetAnyContent(bool requireVisible, ATFrameWindow *exclude) const;

	uint32	GetChildCount() const;
	ATContainerDockingPane *GetChildPane(uint32 index) const;

	void	AddContent(ATFrameWindow *frame, bool deferResize);
	ATContainerDockingPane *Dock(ATFrameWindow *pane, int code);
	bool	Undock(ATFrameWindow *pane);

	void	NotifyFontsUpdated();

	void	RecalcFrame();

	ATFrameWindow *GetVisibleFrame() const;
	void	SetVisibleFrame(ATFrameWindow *frame);

	void	UpdateModalState(ATFrameWindow *modalFrame);
	void	UpdateActivationState(ATFrameWindow *frame);

	void	CreateDragHandles();
	void	DestroyDragHandles();
	void	CreateSplitter();
	void	DestroySplitter();

	bool	HitTestDragHandles(int screenX, int screenY, int& code, ATContainerDockingPane **ppPane);

	void	UpdateFullScreenState();
	bool	IsFullScreen() const;

	void	RemoveAnyEmptyNodes();

	void	OnTabChange(HWND hwndSender);

protected:
	void	RecalcFrameInternal();
	void	RepositionContent(ATContainerResizer& resizer);
	void	RemoveEmptyNode();

	ATContainerWindow *const mpParent;
	vdrefptr<ATDragHandleWindow> mpDragHandle;
	vdrefptr<ATContainerSplitterBar> mpSplitter;

	typedef vdfastvector<ATFrameWindow *> FrameWindows;
	FrameWindows mContent;

	ATContainerDockingPane	*mpDockParent;

	typedef vdfastvector<ATContainerDockingPane *> Children;
	Children mChildren;

	vdrect32	mArea;
	vdrect32	mCenterArea;
	int			mDockCode;
	float		mDockFraction;
	bool		mbFullScreen;
	bool		mbFullScreenLayout;
	bool		mbPinned;
	bool		mbLayoutInvalid;
	bool		mbDescendantLayoutInvalid;
	sint32		mVisibleFrameIndex;

	HWND		mhwndTabControl;
};

class ATContainerWindow : public ATUINativeWindow {
public:
	enum { kTypeID = 'uicw' };

	ATContainerWindow();
	~ATContainerWindow();

	void *AsInterface(uint32 id);

	VDGUIHandle Create(int x, int y, int cx, int cy, VDGUIHandle parent, bool visible);
	VDGUIHandle Create(ATOM wndClass, int x, int y, int cx, int cy, VDGUIHandle parent, bool visible);
	void Destroy();

	void Clear();
	void AutoSize();
	void Relayout();

	bool CanModifyCurrentFrame() const { return mpActiveFrame && !mpFullScreenFrame && !mpModalFrame; }
	void UndockCurrentFrame();
	void CloseCurrentFrame();

	static ATContainerWindow *GetContainerWindow(HWND hwnd);

	ATContainerDockingPane *GetBasePane() const { return mpDockingPane; }
	ATFrameWindow *GetActiveFrame() const { return mpActiveFrame; }
	ATFrameWindow *GetModalFrame() const { return mpModalFrame; }

	int GetSplitterWidth() const { return mSplitterWidth; }
	int GetSplitterHeight() const { return mSplitterHeight; }
	int GetCaptionHeight() const { return mCaptionHeight; }
	HFONT GetCaptionFont() const { return mhfontCaption; }
	HFONT GetCaptionSymbolFont() const { return mhfontCaptionSymbol; }
	HFONT GetLabelFont() const { return mhfontLabel; }

	uint32	GetUndockedPaneCount() const;
	ATFrameWindow *GetUndockedPane(uint32 index) const;

	bool	IsActivelyMovingSizing() const { return mbActivelyMovingSizing; }

	bool	IsLayoutSuspended() const { return mLayoutSuspendCount > 0; }
	void	SuspendLayout();
	void	ResumeLayout();

	void	NotifyFontsUpdated();

	bool	InitDragHandles();
	void	ShutdownDragHandles();
	void	UpdateDragHandles(int screenX, int screenY);
	ATContainerDockingPane *DockFrame(ATFrameWindow *frame, ATContainerDockingPane *pane, int code);
	ATContainerDockingPane *DockFrame(ATFrameWindow *frame, int code);
	ATContainerDockingPane *DockFrame(ATFrameWindow *frame);
	void	AddUndockedFrame(ATFrameWindow *frame);
	void	UndockFrame(ATFrameWindow *frame, bool visible = true, bool destroy = false);
	void	CloseFrame(ATFrameWindow *frame);

	ATFrameWindow *GetFullScreenFrame() const { return mpFullScreenFrame; }
	void	SetFullScreenFrame(ATFrameWindow *frame);

	void	SetModalFrame(ATFrameWindow *frame);

	void	ActivateFrame(ATFrameWindow *frame);

	void	RemoveAnyEmptyNodes();

	void	NotifyFrameActivated(ATFrameWindow *frame);
	void	NotifyUndockedFrameDestroyed(ATFrameWindow *frame);

	void	NotifyDockedFrameDestroyed(ATFrameWindow *w);

	void	AddTrackingNotification(ATFrameWindow *w);

protected:
	LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

	virtual bool OnCreate();
	virtual void OnDestroy();
	void OnSize();

	void OnSetFocus(HWND hwndOldFocus);
	void OnKillFocus(HWND hwndNewFocus);
	bool OnActivate(UINT code, bool minimized, HWND hwnd);

	void RecreateSystemObjects();
	void DestroySystemObjects();

	void UpdateMonitorDpi();
	virtual void UpdateMonitorDpi(unsigned dpiY);

	ATContainerDockingPane *mpDockingPane = nullptr;
	ATContainerDockingPane *mpDragPaneTarget = nullptr;
	ATFrameWindow *mpActiveFrame = nullptr;
	ATFrameWindow *mpFullScreenFrame = nullptr;
	ATFrameWindow *mpModalFrame = nullptr;
	int mDragPaneTargetCode = 0;
	bool mbBlockActiveUpdates = false;
	bool mbActivelyMovingSizing = false;
	int mCaptionHeight = 0;
	int mSplitterWidth = 1;
	int mSplitterHeight = 1;
	HFONT mhfontCaption = nullptr;
	HFONT mhfontCaptionSymbol = nullptr;
	HFONT mhfontLabel = nullptr;

	int mMonitorDpi = 96;

	uint32 mLayoutSuspendCount = 0;

	typedef vdfastvector<ATFrameWindow *> UndockedFrames;
	UndockedFrames mUndockedFrames;

	vdfastvector<ATFrameWindow *> mTrackingNotifyFrames;
};

class ATFrameWindow final : public ATUINativeWindow {
public:
	enum { kTypeID = 'uifr' };

	enum FrameMode {
		kFrameModeUndocked,
		kFrameModeNone,
		kFrameModeEdge,
		kFrameModeFull
	};

	ATFrameWindow(ATContainerWindow *container);
	~ATFrameWindow();

	static ATFrameWindow *GetFrameWindow(HWND hwnd);

	void *AsInterface(uint32 iid);

	ATContainerWindow *GetContainer() const { return mpContainer; }

	void SetPane(ATContainerDockingPane *pane) { mpDockingPane = pane; }
	ATContainerDockingPane *GetPane() const { return mpDockingPane; }

	const wchar_t *GetTitle() const { return mTitle.c_str(); }

	bool IsDocked() const { return mFrameMode != kFrameModeUndocked; }
	bool IsActivelyMovingSizing() const;

	bool IsFullScreen() const;
	void SetFullScreen(bool fs);

	bool IsVisible() const;
	void SetVisible(bool vis);
	void SetFrameMode(FrameMode fm);

	void EnableEndTrackNotification();

	void NotifyFontsUpdated();
	void NotifyEndTracking();

	bool GetIdealSize(vdsize32& sz);
	void RecalcFrame();
	void Relayout(int w, int h);

	VDGUIHandle Create(const wchar_t *title, int x, int y, int cx, int cy, VDGUIHandle parent);
	VDGUIHandle CreateChild(const wchar_t *title, int x, int y, int cx, int cy, VDGUIHandle parent);

protected:
	LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	void PaintCaption(HRGN clipRegion);

	bool OnCreate();
	void OnDestroy();
	void OnSize();
	bool OnNCLButtonDown(int code, int x, int y);
	bool OnMouseMove(int x, int y);

	void EndDrag(bool success);

	int		mDragOriginX = 0;
	int		mDragOriginY = 0;
	int		mDragOffsetX = 0;
	int		mDragOffsetY = 0;
	bool	mbDragging = false;
	bool	mbDragVerified = false;
	bool	mbFullScreen = false;
	bool	mbActiveCaption = false;
	bool	mbCloseDown = false;
	bool	mbCloseTracking = false;
	bool	mbActivelyMovingSizing = false;
	bool	mbEnableEndTrackNotification = false;
	FrameMode	mFrameMode = kFrameModeUndocked;
	vdrect32	mCaptionRect = {};
	vdrect32	mClientRect = {};
	vdrect32	mCloseRect = {};
	ATContainerDockingPane *mpDockingPane = nullptr;
	ATContainerWindow *const mpContainer;
	vdrefptr<ATContainerWindow> mpDragContainer;

	VDStringW	mTitle;
};

class ATUIPane : public ATUINativeWindow {
public:
	enum { kTypeID = 'uipn' };

	ATUIPane(uint32 paneId, const wchar_t *name);
	~ATUIPane();

	void *AsInterface(uint32 iid);

	uint32 GetUIPaneId() const { return mPaneId; }
	const wchar_t *GetUIPaneName() const { return mpName; }
	int GetPreferredDockCode() const { return mPreferredDockCode; }

	bool Create(ATFrameWindow *w);

protected:
	void SetName(const wchar_t *name);

	LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

	virtual bool OnCreate();
	virtual void OnDestroy();
	virtual void OnSize();
	virtual void OnSetFocus();
	virtual void OnFontsUpdated();
	virtual bool OnCommand(uint32 id, uint32 extcode);

	void RegisterUIPane();
	void UnregisterUIPane();

	const wchar_t *		mpName;
	uint32 const		mPaneId;
	uint32				mDefaultWindowStyles;
	int					mPreferredDockCode;
};

typedef bool (*ATPaneCreator)(ATUIPane **);
typedef bool (*ATPaneClassCreator)(uint32 id, ATUIPane **);

void ATRegisterUIPaneType(uint32 id, ATPaneCreator creator);
void ATRegisterUIPaneClass(uint32 id, ATPaneClassCreator creator);

void ATActivateUIPane(uint32 id, bool giveFocus, bool visible = true, uint32 relid = 0, int reldock = 0);

uint32 ATUIGetGlobalDpiW32();
uint32 ATUIGetWindowDpiW32(HWND hwnd);
HFONT ATUICreateDefaultFontForDpiW32(uint32 dpi);

#endif
