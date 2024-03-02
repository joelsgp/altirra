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

#include "stdafx.h"
#include <windows.h>
#include <windowsx.h>
#include <hash_map>
#include <vd2/system/strutil.h>
#include <vd2/system/vdstl.h>
#include <vd2/system/vectors.h>
#include <vd2/system/w32assist.h>
#include <vd2/system/math.h>
#include "ui.h"
#include "uiframe.h"

// Requires Windows XP
#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED 0x031A
#endif


#pragma comment(lib, "msimg32")

///////////////////////////////////////////////////////////////////////////////

extern ATContainerWindow *g_pMainWindow;

///////////////////////////////////////////////////////////////////////////////

namespace ATUIFrame {
	int g_splitterDistH;
	int g_splitterDistV;
}

using namespace ATUIFrame;

void ATInitUIFrameSystem() {
	g_splitterDistH = GetSystemMetrics(SM_CXEDGE);
	g_splitterDistV = GetSystemMetrics(SM_CYEDGE);
}

void ATShutdownUIFrameSystem() {
}

///////////////////////////////////////////////////////////////////////////////

ATContainerResizer::ATContainerResizer()
	: mhdwp(NULL)
{
}

void ATContainerResizer::LayoutWindow(HWND hwnd, int x, int y, int width, int height) {
	if (!mhdwp)
		mhdwp = BeginDeferWindowPos(4);

	if (mhdwp) {
		HDWP hdwp = DeferWindowPos(mhdwp, hwnd, NULL, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE);

		if (hdwp) {
			mhdwp = hdwp;
			return;
		}
	}

	SetWindowPos(hwnd, NULL, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
}

void ATContainerResizer::ResizeWindow(HWND hwnd, int width, int height) {
	if (!mhdwp)
		mhdwp = BeginDeferWindowPos(4);

	if (mhdwp) {
		HDWP hdwp = DeferWindowPos(mhdwp, hwnd, NULL, 0, 0, width, height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

		if (hdwp) {
			mhdwp = hdwp;
			return;
		}
	}

	SetWindowPos(hwnd, NULL, 0, 0, width, height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
}

void ATContainerResizer::Flush() {
	if (mhdwp) {
		EndDeferWindowPos(mhdwp);
		mhdwp = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////

ATContainerSplitterBar::ATContainerSplitterBar()
	: mpControlledPane(NULL)
	, mbVertical(false)
	, mDistanceOffset(0)
{
}

ATContainerSplitterBar::~ATContainerSplitterBar() {
}

bool ATContainerSplitterBar::Init(HWND hwndParent, ATContainerDockingPane *pane, bool vertical) {
	mbVertical = vertical;
	mpControlledPane = pane;

	if (!mhwnd) {
		if (!CreateWindow(MAKEINTATOM(sWndClass), _T(""), WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS, 0, 0, 0, 0, hwndParent, (UINT)0, VDGetLocalModuleHandleW32(), static_cast<VDShaderEditorBaseWindow *>(this)))
			return false;
	}

	return true;
}

void ATContainerSplitterBar::Shutdown() {
	if (mhwnd)
		DestroyWindow(mhwnd);
}

LRESULT ATContainerSplitterBar::WndProc(UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
	case WM_SIZE:
		OnSize();
		break;

	case WM_PAINT:
		OnPaint();
		break;

	case WM_LBUTTONDOWN:
		OnLButtonDown(wParam, (SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam));
		return 0;

	case WM_LBUTTONUP:
		OnLButtonUp(wParam, (SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam));
		return 0;

	case WM_MOUSEMOVE:
		OnMouseMove(wParam, (SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam));
		return 0;

	case WM_CAPTURECHANGED:
		OnCaptureChanged((HWND)lParam);
		return 0;

	case WM_SETCURSOR:
		SetCursor(LoadCursor(NULL, mbVertical ? IDC_SIZEWE : IDC_SIZENS));
		return TRUE;
	}

	return DefWindowProc(mhwnd, msg, wParam, lParam);
}

void ATContainerSplitterBar::OnPaint() {
	PAINTSTRUCT ps;

	if (HDC hdc = BeginPaint(mhwnd, &ps)) {
		RECT r;
		GetClientRect(mhwnd, &r);
//		DrawEdge(hdc, &r, EDGE_RAISED, mbVertical ? BF_LEFT|BF_RIGHT|BF_ADJUST : BF_TOP|BF_BOTTOM|BF_ADJUST);
		FillRect(hdc, &r, (HBRUSH)(COLOR_3DFACE+1));

		EndPaint(mhwnd, &ps);
	}
}

void ATContainerSplitterBar::OnSize() {
	InvalidateRect(mhwnd, NULL, TRUE);
}

void ATContainerSplitterBar::OnLButtonDown(WPARAM wParam, int x, int y) {
	POINT pt = {x, y};
	MapWindowPoints(mhwnd, GetParent(mhwnd), &pt, 1);

	RECT r;
	if (!GetClientRect(mhwnd, &r))
		return;

	const vdrect32& rPane = mpControlledPane->GetArea();

	switch(mpControlledPane->GetDockCode()) {
	case kATContainerDockLeft:
		mDistanceOffset = rPane.width() - pt.x;
		break;
	case kATContainerDockRight:
		mDistanceOffset = rPane.width() + pt.x;
		break;
	case kATContainerDockTop:
		mDistanceOffset = rPane.height() - pt.y;
		break;
	case kATContainerDockBottom:
		mDistanceOffset = rPane.height() + pt.y;
		break;
	}

	SetCapture(mhwnd);
}

void ATContainerSplitterBar::OnLButtonUp(WPARAM wParam, int x, int y) {
	if (GetCapture() == mhwnd)
		ReleaseCapture();
}

void ATContainerSplitterBar::OnMouseMove(WPARAM wParam, int x, int y) {
	if (GetCapture() != mhwnd)
		return;

	POINT pt = {x, y};
	MapWindowPoints(mhwnd, GetParent(mhwnd), &pt, 1);

	const vdrect32& rParentPane = mpControlledPane->GetParentPane()->GetArea();
	int parentW = rParentPane.width();
	int parentH = rParentPane.height();

	switch(mpControlledPane->GetDockCode()) {
	case kATContainerDockLeft:
		mpControlledPane->SetDockFraction((float)(mDistanceOffset + pt.x) / (float)parentW);
		break;
	case kATContainerDockRight:
		mpControlledPane->SetDockFraction((float)(mDistanceOffset - pt.x) / (float)parentW);
		break;
	case kATContainerDockTop:
		mpControlledPane->SetDockFraction((float)(mDistanceOffset + pt.y) / (float)parentH);
		break;
	case kATContainerDockBottom:
		mpControlledPane->SetDockFraction((float)(mDistanceOffset - pt.y) / (float)parentH);
		break;
	}
}

void ATContainerSplitterBar::OnCaptureChanged(HWND hwndNewCapture) {
}

///////////////////////////////////////////////////////////////////////////////

ATDragHandleWindow::ATDragHandleWindow()
	: mX(0)
	, mY(0)
{
}

ATDragHandleWindow::~ATDragHandleWindow() {
}

VDGUIHandle ATDragHandleWindow::Create(int x, int y, int cx, int cy, VDGUIHandle parent, int id) {
	HWND hwnd = CreateWindowEx(WS_EX_TOPMOST|WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE, (LPCTSTR)sWndClass, _T(""), WS_POPUP, x, y, cx, cy, (HWND)parent, (HMENU)id, VDGetLocalModuleHandleW32(), static_cast<VDShaderEditorBaseWindow *>(this));

	if (hwnd)
		ShowWindow(hwnd, SW_SHOWNOACTIVATE);

	return (VDGUIHandle)hwnd;
}

void ATDragHandleWindow::Destroy() {
	if (mhwnd)
		DestroyWindow(mhwnd);
}

int ATDragHandleWindow::HitTest(int screenX, int screenY) {
	int xdist = screenX - mX;
	int ydist = screenY - mY;
	int dist = abs(xdist) + abs(ydist);

	if (dist >= 37)
		return -1;

	if (xdist < -18)
		return kATContainerDockLeft;

	if (xdist > +18)
		return kATContainerDockRight;

	if (ydist < -18)
		return kATContainerDockTop;

	if (ydist > +18)
		return kATContainerDockBottom;

	return kATContainerDockCenter;
}

LRESULT ATDragHandleWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
		case WM_CREATE:
			OnCreate();
			break;

		case WM_MOVE:
			OnMove();
			break;

		case WM_PAINT:
			OnPaint();
			return 0;

		case WM_ERASEBKGND:
			return FALSE;
	}

	return VDShaderEditorBaseWindow::WndProc(msg, wParam, lParam);
}

void ATDragHandleWindow::OnCreate() {
	POINT pt[8]={
		{  0<<0, 37<<0 },
		{  0<<0, 38<<0 },
		{ 37<<0, 75<<0 },
		{ 38<<0, 75<<0 },
		{ 75<<0, 38<<0 },
		{ 75<<0, 37<<0 },
		{ 38<<0,  0<<0 },
		{ 37<<0,  0<<0 },
	};

	HRGN rgn = CreatePolygonRgn(pt, 8, ALTERNATE);
	if (rgn) {
		if (!SetWindowRgn(mhwnd, rgn, TRUE))
			DeleteObject(rgn);
	}

	OnMove();
}

void ATDragHandleWindow::OnMove() {
	RECT r;
	GetWindowRect(mhwnd, &r);
	mX = r.left + 37;
	mY = r.top + 37;
}

void ATDragHandleWindow::OnPaint() {
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(mhwnd, &ps);
	if (hdc) {
		RECT r;
		GetClientRect(mhwnd, &r);
		FillRect(hdc, &r, (HBRUSH)(COLOR_3DFACE + 1));

		int saveIndex = SaveDC(hdc);
		if (saveIndex) {
			SelectObject(hdc, GetStockObject(DC_PEN));

			uint32 a0 = GetSysColor(COLOR_3DSHADOW);
			uint32 a2 = GetSysColor(COLOR_3DFACE);
			uint32 a4 = GetSysColor(COLOR_3DHIGHLIGHT);
			uint32 a1 = (a0|a2) - (((a0^a2) & 0xfefefe)>>1);
			uint32 a3 = (a2|a4) - (((a2^a4) & 0xfefefe)>>1);
			uint32 b0 = GetSysColor(COLOR_3DDKSHADOW);

			MoveToEx(hdc, 0, 37, NULL);
			SetDCPenColor(hdc, a4);
			LineTo(hdc, 37, 0);
			SetDCPenColor(hdc, a3);
			LineTo(hdc, 74, 37);
			SetDCPenColor(hdc, b0);
			LineTo(hdc, 37, 74);
			SetDCPenColor(hdc, a1);
			LineTo(hdc, 0, 37);

			MoveToEx(hdc, 1, 37, NULL);
			SetDCPenColor(hdc, a4);
			LineTo(hdc, 37, 1);
			SetDCPenColor(hdc, a3);
			LineTo(hdc, 73, 37);
			SetDCPenColor(hdc, a0);
			LineTo(hdc, 37, 73);
			SetDCPenColor(hdc, a1);
			LineTo(hdc, 1, 37);

			MoveToEx(hdc, 19, 55, NULL);
			SetDCPenColor(hdc, a1);
			LineTo(hdc, 19, 19);
			LineTo(hdc, 55, 19);
			SetDCPenColor(hdc, a3);
			LineTo(hdc, 55, 55);
			LineTo(hdc, 19, 55);

			MoveToEx(hdc, 20, 54, NULL);
			SetDCPenColor(hdc, a3);
			LineTo(hdc, 20, 20);
			LineTo(hdc, 54, 20);
			SetDCPenColor(hdc, a1);
			LineTo(hdc, 54, 54);
			LineTo(hdc, 20, 54);

			RestoreDC(hdc, saveIndex);
		}

		EndPaint(mhwnd, &ps);
	}
}

///////////////////////////////////////////////////////////////////////////////

ATContainerDockingPane::ATContainerDockingPane(ATContainerWindow *parent)
	: mpParent(parent)
	, mpDockParent(NULL)
	, mDockCode(-1)
	, mDockFraction(0)
	, mCenterCount(0)
	, mbFullScreen(false)
	, mbFullScreenLayout(false)
	, mbPinned(false)
{
}

ATContainerDockingPane::~ATContainerDockingPane() {
	while(!mChildren.empty()) {
		ATContainerDockingPane *child = mChildren.back();
		mChildren.pop_back();

		VDASSERT(child->mpDockParent == this);
		child->mpDockParent = NULL;
		child->mDockCode = -1;
		child->Release();
	}

	DestroyDragHandles();
	DestroySplitter();
}

void ATContainerDockingPane::SetArea(ATContainerResizer& resizer, const vdrect32& area, bool parentContainsFullScreen) {
	bool fullScreenLayout = mbFullScreen || parentContainsFullScreen;

	if (mArea == area && mbFullScreenLayout == fullScreenLayout)
		return;

	mArea = area;
	mbFullScreenLayout = fullScreenLayout;

	Relayout(resizer);
}

void ATContainerDockingPane::Clear() {
	AddRef();

	while(!mChildren.empty())
		mChildren.back()->Clear();

	if (mpContent) {
		HWND hwnd = mpContent->GetHandleW32();

		if (hwnd)
			DestroyWindow(hwnd);

		// This should happen automatically if we had a window, but just in case, we flush it.
		mpContent.clear();
	}

	RemoveEmptyNode();

	Release();
}

void ATContainerDockingPane::Relayout(ATContainerResizer& resizer) {
	mCenterArea = mArea;

	if (mpSplitter) {
		HWND hwndSplitter = mpSplitter->GetHandleW32();

		::ShowWindow(hwndSplitter, mbFullScreenLayout ? SW_HIDE : SW_SHOWNOACTIVATE);

		switch(mDockCode) {
			case kATContainerDockLeft:
				resizer.LayoutWindow(hwndSplitter, mArea.right, mArea.top, g_splitterDistH, mArea.height());
				break;

			case kATContainerDockRight:
				resizer.LayoutWindow(hwndSplitter, mArea.left - g_splitterDistH, mArea.top, g_splitterDistH, mArea.height());
				break;

			case kATContainerDockTop:
				resizer.LayoutWindow(hwndSplitter, mArea.left, mArea.bottom, mArea.width(), g_splitterDistV);
				break;

			case kATContainerDockBottom:
				resizer.LayoutWindow(hwndSplitter, mArea.left, mArea.top - g_splitterDistV, mArea.width(), g_splitterDistV);
				break;
		}
	}

	Children::const_iterator it(mChildren.begin()), itEnd(mChildren.end());
	for(; it != itEnd; ++it) {
		ATContainerDockingPane *pane = *it;

		vdrect32 rPane(mCenterArea);
		if (!mbFullScreenLayout) {
			int padX = 0;
			int padY = 0;

			if (pane->mpContent) {
				HWND hwndContent = pane->mpContent->GetHandleW32();

				if (hwndContent) {
					RECT rPad = {0,0,0,0};
					AdjustWindowRect(&rPad, GetWindowLong(hwndContent, GWL_STYLE), FALSE);

					padX += rPad.right - rPad.left;
					padY += rPad.bottom - rPad.top;
				}
			}

			int w = std::max<int>(VDRoundToInt(mArea.width() * pane->mDockFraction), padX);
			int h = std::max<int>(VDRoundToInt(mArea.height() * pane->mDockFraction), padY); 

			switch(pane->mDockCode) {
				case kATContainerDockLeft:
					rPane.right = rPane.left + w;
					mCenterArea.left = rPane.right + g_splitterDistH;
					break;

				case kATContainerDockRight:
					rPane.left = rPane.right - w;
					mCenterArea.right = rPane.left - g_splitterDistH;
					break;

				case kATContainerDockTop:
					rPane.bottom = rPane.top + h;
					mCenterArea.top = rPane.bottom + g_splitterDistV;
					break;

				case kATContainerDockBottom:
					rPane.top = rPane.bottom - h;
					mCenterArea.bottom = rPane.top - g_splitterDistV;
					break;

				case kATContainerDockCenter:
					break;
			}
		}

		pane->SetArea(resizer, rPane, mbFullScreenLayout);
	}

	RepositionContent();
}

bool ATContainerDockingPane::GetFrameSizeForContent(vdsize32& sz) {
	double horizFraction = 1.0f;
	double vertFraction = 1.0f;
	int horizExtra = 0;
	int vertExtra = 0;

	Children::const_iterator it(mChildren.begin()), itEnd(mChildren.end());
	for(; it != itEnd; ++it) {
		ATContainerDockingPane *pane = *it;

		switch(pane->mDockCode) {
			case kATContainerDockLeft:
			case kATContainerDockRight:
				horizFraction -= pane->mDockFraction;
				horizExtra += g_splitterDistH + 1;		// +1 for rounding bias
				break;

			case kATContainerDockTop:
			case kATContainerDockBottom:
				vertFraction -= pane->mDockFraction;
				vertExtra += g_splitterDistV + 1;
				break;

			case kATContainerDockCenter:
				break;
		}
	}

	if (horizFraction < 1e-5f || vertFraction < 1e-5f)
		return false;

	sz.w = VDRoundToInt32((double)sz.w / horizFraction) + horizExtra;
	sz.h = VDRoundToInt32((double)sz.h / vertFraction) + vertExtra;
	return true;
}

int ATContainerDockingPane::GetDockCode() const {
	return mDockCode;
}

float ATContainerDockingPane::GetDockFraction() const {
	return mDockFraction;
}

void ATContainerDockingPane::SetDockFraction(float frac) {
	if (frac < 0.0f)
		frac = 0.0f;

	if (frac > 1.0f)
		frac = 1.0f;

	mDockFraction = frac;

	if (mpDockParent) {
		ATContainerResizer resizer;
		mpDockParent->Relayout(resizer);
		resizer.Flush();
	}
}

ATFrameWindow *ATContainerDockingPane::GetAnyContent() const {
	if (mpContent)
		return mpContent;

	Children::const_iterator it(mChildren.begin()), itEnd(mChildren.end());
	for(; it!=itEnd; ++it) {
		ATContainerDockingPane *child = *it;
		ATFrameWindow *content = child->GetAnyContent();

		if (content)
			return content;
	}

	return NULL;
}

ATContainerDockingPane *ATContainerDockingPane::GetCenterPane() const {
	Children::const_iterator it(mChildren.begin()), itEnd(mChildren.end());
	for(; it!=itEnd; ++it) {
		ATContainerDockingPane *child = *it;

		if (child->GetDockCode() == kATContainerDockCenter)
			return child;
	}

	return NULL;
}

uint32 ATContainerDockingPane::GetChildCount() const {
	return mChildren.size();
}

ATContainerDockingPane *ATContainerDockingPane::GetChildPane(uint32 index) const {
	if (index >= mChildren.size())
		return NULL;

	return mChildren[index];
}

void ATContainerDockingPane::SetContent(ATFrameWindow *frame) {
	VDASSERT(!mpContent);
	mpContent = frame;
	frame->SetPane(this);

	RepositionContent();
}

void ATContainerDockingPane::Dock(ATContainerDockingPane *pane, int code) {
	size_t pos = mCenterCount;

	if (code == kATContainerDockCenter)
		++mCenterCount;
	else {
		switch(code) {
			case kATContainerDockLeft:
			case kATContainerDockRight:
				pane->mDockFraction = 1.0f;
				for(Children::const_iterator it(mChildren.begin()), itEnd(mChildren.end()); it != itEnd; ++it) {
					ATContainerDockingPane *child = *it;

					if (child->mDockCode == kATContainerDockLeft || child->mDockCode == kATContainerDockRight)
						pane->mDockFraction -= child->mDockFraction;
				}

				if (pane->mDockFraction < 0.1f)
					pane->mDockFraction = 0.1f;
				else
					pane->mDockFraction *= 0.5f;
				break;

			case kATContainerDockTop:
			case kATContainerDockBottom:
				pane->mDockFraction = 1.0f;
				for(Children::const_iterator it(mChildren.begin()), itEnd(mChildren.end()); it != itEnd; ++it) {
					ATContainerDockingPane *child = *it;

					if (child->mDockCode == kATContainerDockTop || child->mDockCode == kATContainerDockBottom)
						pane->mDockFraction -= child->mDockFraction;
				}

				if (pane->mDockFraction < 0.1f)
					pane->mDockFraction = 0.1f;
				else
					pane->mDockFraction *= 0.5f;
				break;
		}

		if (!mpDockParent)
			pane->mDockFraction *= 0.5f;
	}

	mChildren.insert(mChildren.end() - pos, pane);
	pane->AddRef();
	pane->mpDockParent = this;
	pane->mDockCode = code;

	ATContainerResizer resizer;
	Relayout(resizer);
	resizer.Flush();

	pane->CreateSplitter();
}

bool ATContainerDockingPane::Undock(ATFrameWindow *pane) {
	if (mpContent == pane) {
		pane->SetPane(NULL);
		mpContent = NULL;

		if (mpDockParent) {
			RemoveEmptyNode();
			return true;
		}

		return true;
	}

	Children::const_iterator it(mChildren.begin()), itEnd(mChildren.end());
	for(; it!=itEnd; ++it) {
		ATContainerDockingPane *child = *it;

		if (child->Undock(pane))
			return true;
	}

	return false;
}

void ATContainerDockingPane::NotifyFontsUpdated() {
	if (mpContent)
		mpContent->NotifyFontsUpdated();

	Children::const_iterator it(mChildren.begin()), itEnd(mChildren.end());
	for(; it!=itEnd; ++it) {
		ATContainerDockingPane *child = *it;

		child->NotifyFontsUpdated();
	}
}

void ATContainerDockingPane::RecalcFrame() {
	Children::const_iterator it(mChildren.begin()), itEnd(mChildren.end());
	for(; it!=itEnd; ++it) {
		ATContainerDockingPane *child = *it;

		child->RecalcFrame();
	}

	if (mpContent)
		mpContent->RecalcFrame();
}

void ATContainerDockingPane::UpdateActivationState(ATFrameWindow *frame) {
	if (mpContent) {
		HWND hwndContent = mpContent->GetHandleW32();

		if (hwndContent)
			SendMessage(hwndContent, WM_NCACTIVATE, frame == mpContent, 0);
	}

	Children::const_iterator it(mChildren.begin()), itEnd(mChildren.end());
	for(; it!=itEnd; ++it) {
		ATContainerDockingPane *pane = *it;

		pane->UpdateActivationState(frame);
	}
}

void ATContainerDockingPane::CreateDragHandles() {
	if (!mpDragHandle) {
		mpDragHandle = new ATDragHandleWindow;
		POINT pt = { (mCenterArea.left + mCenterArea.right - 75)/2, (mCenterArea.top + mCenterArea.bottom - 75)/2 };

		HWND hwndParent = mpParent->GetHandleW32();
		ClientToScreen(hwndParent, &pt);

		mpDragHandle->Create(pt.x, pt.y, 75, 75, NULL, 0);

		Children::const_iterator it(mChildren.begin()), itEnd(mChildren.end());
		for(; it!=itEnd; ++it) {
			ATContainerDockingPane *pane = *it;
			pane->CreateDragHandles();
		}
	}
}

void ATContainerDockingPane::DestroyDragHandles() {
	Children::const_iterator it(mChildren.begin()), itEnd(mChildren.end());
	for(; it!=itEnd; ++it) {
		ATContainerDockingPane *pane = *it;
		pane->DestroyDragHandles();
	}

	if (mpDragHandle) {
		mpDragHandle->Destroy();
		mpDragHandle = NULL;
	}
}

void ATContainerDockingPane::CreateSplitter() {
	if (mpSplitter)
		return;

	if (mDockCode == kATContainerDockCenter)
		return;

	mpSplitter = new ATContainerSplitterBar;
	if (!mpSplitter->Init(mpParent->GetHandleW32(), this, mDockCode == kATContainerDockLeft || mDockCode == kATContainerDockRight)) {
		mpSplitter = NULL;
		return;
	}

	HWND hwndSplitter = mpSplitter->GetHandleW32();

	vdrect32 rSplit(mArea);

	switch(mDockCode) {
		case kATContainerDockLeft:
			rSplit.left = rSplit.right;
			rSplit.right += g_splitterDistH;
			break;
		case kATContainerDockRight:
			rSplit.right = rSplit.left;
			rSplit.left -= g_splitterDistH;
			break;
		case kATContainerDockTop:
			rSplit.top = rSplit.bottom;
			rSplit.bottom += g_splitterDistV;
			break;
		case kATContainerDockBottom:
			rSplit.bottom = rSplit.top;
			rSplit.top -= g_splitterDistV;
			break;
	}

	SetWindowPos(hwndSplitter, NULL, rSplit.left, rSplit.top, rSplit.width(), rSplit.height(), SWP_NOZORDER|SWP_NOACTIVATE);
}

void ATContainerDockingPane::DestroySplitter() {
	if (!mpSplitter)
		return;

	mpSplitter->Shutdown();
	mpSplitter = NULL;
}

bool ATContainerDockingPane::HitTestDragHandles(int screenX, int screenY, int& code, ATContainerDockingPane **ppPane) {
	if (mpDragHandle) {
		int localCode = mpDragHandle->HitTest(screenX, screenY);

		if (localCode >= 0) {
			code = localCode;
			*ppPane = this;
			AddRef();
			return true;
		}
	}

	Children::const_iterator it(mChildren.begin()), itEnd(mChildren.end());
	for(; it!=itEnd; ++it) {
		ATContainerDockingPane *pane = *it;
		if (pane && pane->HitTestDragHandles(screenX, screenY, code, ppPane))
			return true;
	}

	return false;
}

void ATContainerDockingPane::UpdateFullScreenState() {
	if (mpContent)
		mbFullScreen = mpContent->IsFullScreen();
	else
		mbFullScreen = false;

	Children::const_iterator it(mChildren.begin()), itEnd(mChildren.end());
	for(; it!=itEnd; ++it) {
		ATContainerDockingPane *pane = *it;
		if (pane && pane->IsFullScreen()) {
			mbFullScreen = true;
			break;
		}
	}

	if (mpDockParent)
		mpDockParent->UpdateFullScreenState();
}

bool ATContainerDockingPane::IsFullScreen() const {
	return mbFullScreen;
}

void ATContainerDockingPane::RepositionContent() {
	if (!mpContent)
		return;

	HWND hwndContent = mpContent->GetHandleW32();

	if (mbFullScreenLayout && !mpContent->IsFullScreen())
		ShowWindow(hwndContent, SW_HIDE);
	else {
		ShowWindow(hwndContent, SW_SHOWNOACTIVATE);
		SetWindowPos(hwndContent, NULL, mCenterArea.left, mCenterArea.top, mCenterArea.width(), mCenterArea.height(), SWP_NOZORDER|SWP_NOACTIVATE);
	}
}

void ATContainerDockingPane::RemoveAnyEmptyNodes() {
	AddRef();

	RemoveEmptyNode();

	if ((mpDockParent || mbPinned) && !mChildren.empty()) {
		vdfastvector<ATContainerDockingPane *> children;

		for(Children::const_iterator it(mChildren.begin()), itEnd(mChildren.end()); it != itEnd; ++it) {
			ATContainerDockingPane *child = *it;

			child->AddRef();
			children.push_back(child);
		}

		while(!children.empty()) {
			ATContainerDockingPane *child = children.back();
			children.pop_back();

			child->RemoveEmptyNode();
			child->Release();
		}
	}

	Release();
}

void ATContainerDockingPane::RemoveEmptyNode() {
	ATContainerDockingPane *parent = mpDockParent;
	if (!parent || mpContent || mbPinned)
		return;

	if (!mChildren.empty()) {
		ATContainerDockingPane *child = mChildren.back();

		mpContent = child->mpContent;
		child->mpContent = NULL;

		if (mpContent)
			mpContent->SetPane(this);

		for(Children::const_iterator it(child->mChildren.begin()), itEnd(child->mChildren.end()); it != itEnd; ++it) {
			ATContainerDockingPane *child2 = *it;

			mChildren.push_back(child2);
			child2->mpDockParent = this;
		}

		child->mChildren.clear();

		AddRef();
		child->RemoveEmptyNode();
		ATContainerResizer resizer;
		Relayout(resizer);
		resizer.Flush();
		Release();
		return;
	}

	DestroySplitter();

	Children::iterator itDel(std::find(parent->mChildren.begin(), parent->mChildren.end(), this));
	VDASSERT(itDel != parent->mChildren.end());
	parent->mChildren.erase(itDel);

	if (mDockCode == kATContainerDockCenter)
		--parent->mCenterCount;

	mpDockParent = NULL;
	mDockCode = -1;
	mDockFraction = 0;
	Release();

	// NOTE: We're dead at this point!

	ATContainerResizer resizer;
	parent->Relayout(resizer);
	resizer.Flush();

	if (parent->mChildren.empty())
		parent->RemoveEmptyNode();
}

///////////////////////////////////////////////////////////////////////////////

ATContainerWindow::ATContainerWindow()
	: mpDockingPane(new ATContainerDockingPane(this))
	, mpDragPaneTarget(NULL)
	, mpActiveFrame(NULL)
	, mpFullScreenFrame(NULL)
	, mbBlockActiveUpdates(false)
	, mhfontCaption(NULL)
	, mhfontCaptionSymbol(NULL)
{
	if (mpDockingPane) {
		mpDockingPane->AddRef();
		mpDockingPane->SetPinned(true);
	}
}

ATContainerWindow::~ATContainerWindow() {
	if (mpDragPaneTarget) {
		mpDragPaneTarget->Release();
		mpDragPaneTarget = NULL;
	}
	if (mpDockingPane) {
		mpDockingPane->Release();
		mpDockingPane = NULL;
	}
}

void *ATContainerWindow::AsInterface(uint32 id) {
	if (id == ATContainerWindow::kTypeID)
		return static_cast<ATContainerWindow *>(this);

	return VDShaderEditorBaseWindow::AsInterface(id);
}

VDGUIHandle ATContainerWindow::Create(int x, int y, int cx, int cy, VDGUIHandle parent, bool visible) {
	return Create(sWndClass, x, y, cx, cy, parent, visible);
}

VDGUIHandle ATContainerWindow::Create(ATOM wc, int x, int y, int cx, int cy, VDGUIHandle parent, bool visible) {
	return (VDGUIHandle)CreateWindowEx(0, (LPCTSTR)wc, _T(""), WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|(visible ? WS_VISIBLE : 0), x, y, cx, cy, (HWND)parent, NULL, VDGetLocalModuleHandleW32(), static_cast<VDShaderEditorBaseWindow *>(this));
}

void ATContainerWindow::Destroy() {
	if (mhwnd) {
		DestroyWindow(mhwnd);
		mhwnd = NULL;
	}
}

void ATContainerWindow::Clear() {
	if (mpDockingPane)
		mpDockingPane->Clear();
}

void ATContainerWindow::AutoSize() {
	if (!mpDockingPane || !mhwnd || mpFullScreenFrame)
		return;

	WINDOWPLACEMENT wp = {sizeof(WINDOWPLACEMENT)};
	if (!GetWindowPlacement(mhwnd, &wp))
		return;

	if (wp.showCmd != SW_SHOWNORMAL)
		return;

	ATContainerDockingPane *centerPane = mpDockingPane->GetCenterPane();
	if (!centerPane)
		return;

	ATFrameWindow *frame = centerPane->GetContent();
	if (!frame)
		return;

	vdsize32 sz;
	if (!frame->GetIdealSize(sz))
		return;

	if (!mpDockingPane->GetFrameSizeForContent(sz))
		return;

	RECT r = {0, 0, sz.w, sz.h};
	if (!AdjustWindowRect(&r, GetWindowLong(mhwnd, GWL_STYLE), GetMenu(mhwnd) != NULL))
		return;

	const int desiredWidth = (r.right - r.left);
	const int desiredHeight = (r.bottom - r.top);
	wp.rcNormalPosition.right = wp.rcNormalPosition.left + desiredWidth;
	wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + desiredHeight;

	SetWindowPlacement(mhwnd, &wp);

	// Check for the case where the menu has forced the client rectangle to shrink
	// vertically due to wrapping -- in that case, measure the delta and attempt to
	// apply a one time correction.
	RECT r2;
	if (GetWindowRect(mhwnd, &r2) && r2.right - r2.left == desiredWidth && r2.bottom - r2.top == desiredHeight) {
		RECT rc;

		if (GetClientRect(mhwnd, &rc) && rc.right == sz.w && rc.bottom < sz.h) {
			wp.rcNormalPosition.bottom += (sz.h - rc.bottom);

			SetWindowPlacement(mhwnd, &wp);
		}
	}
}

void ATContainerWindow::Relayout() {
	OnSize();
}

ATContainerWindow *ATContainerWindow::GetContainerWindow(HWND hwnd) {
	if (hwnd) {
		ATOM a = (ATOM)GetClassLong(hwnd, GCW_ATOM);

		if (a == sWndClass || a == sWndClassMain) {
			VDShaderEditorBaseWindow *w = (VDShaderEditorBaseWindow *)GetWindowLongPtr(hwnd, 0);
			return vdpoly_cast<ATContainerWindow *>(w);
		}
	}

	return NULL;
}

uint32 ATContainerWindow::GetUndockedPaneCount() const {
	return mUndockedFrames.size();
}

ATFrameWindow *ATContainerWindow::GetUndockedPane(uint32 index) const {
	if (index >= mUndockedFrames.size())
		return NULL;

	return mUndockedFrames[index];
}

void ATContainerWindow::NotifyFontsUpdated() {
	if (mpDockingPane)
		mpDockingPane->NotifyFontsUpdated();
}

bool ATContainerWindow::InitDragHandles() {
	mpDockingPane->CreateDragHandles();
	return true;
}

void ATContainerWindow::ShutdownDragHandles() {
	mpDockingPane->DestroyDragHandles();
}

void ATContainerWindow::UpdateDragHandles(int screenX, int screenY) {
	if (mpDragPaneTarget) {
		mpDragPaneTarget->Release();
		mpDragPaneTarget = NULL;
		mDragPaneTargetCode = -1;
	}

	mpDockingPane->HitTestDragHandles(screenX, screenY, mDragPaneTargetCode, &mpDragPaneTarget);
}

ATContainerDockingPane *ATContainerWindow::DockFrame(ATFrameWindow *frame, int code) {
	return DockFrame(frame, mpDockingPane, code);
}

ATContainerDockingPane *ATContainerWindow::DockFrame(ATFrameWindow *frame, ATContainerDockingPane *parent, int code) {
	parent->AddRef();
	if (mpDragPaneTarget)
		mpDragPaneTarget->Release();
	mpDragPaneTarget = parent;
	mDragPaneTargetCode = code;

	return DockFrame(frame);
}

ATContainerDockingPane *ATContainerWindow::DockFrame(ATFrameWindow *frame) {
	if (!mpDragPaneTarget)
		return NULL;

	if (frame) {
		UndockedFrames::iterator it = std::find(mUndockedFrames.begin(), mUndockedFrames.end(), frame);
		if (it != mUndockedFrames.end())
			mUndockedFrames.erase(it);

		HWND hwndFrame = frame->GetHandleW32();

		if (hwndFrame) {
			HWND hwndActive = ::GetFocus();
			if (hwndActive && ::GetAncestor(hwndActive, GA_ROOT) != hwndFrame)
				hwndActive = NULL;

			if (::GetForegroundWindow() == hwndFrame) {
				::SetForegroundWindow(mhwnd);
				mpActiveFrame = frame;
			}

			UINT style = GetWindowLong(hwndFrame, GWL_STYLE);
			style |= WS_CHILD | WS_SYSMENU;
			style &= ~(WS_POPUP | WS_THICKFRAME);		// must remove WS_SYSMENU for top level menus to work
			SetWindowLong(hwndFrame, GWL_STYLE, style);

			GUITHREADINFO gti = {sizeof(GUITHREADINFO)};
			::GetGUIThreadInfo(GetCurrentThreadId(), &gti);

			// Prevent WM_CHILDACTIVATE from changing the active window.
			mbBlockActiveUpdates = true;
			SetParent(hwndFrame, mhwnd);
			mbBlockActiveUpdates = false;

			SetWindowPos(hwndFrame, NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED|SWP_NOACTIVATE);

			UINT exstyle = GetWindowLong(hwndFrame, GWL_EXSTYLE);
			exstyle |= WS_EX_TOOLWINDOW;
			exstyle &= ~WS_EX_WINDOWEDGE;
			SetWindowLong(hwndFrame, GWL_EXSTYLE, exstyle);

			SendMessage(mhwnd, WM_CHANGEUISTATE, MAKELONG(UIS_INITIALIZE, UISF_HIDEACCEL|UISF_HIDEFOCUS), 0);

			if (hwndActive)
				::SetFocus(hwndActive);
		}
	}

	vdrefptr<ATContainerDockingPane> newPane(new ATContainerDockingPane(this));

	if (frame) {
		frame->SetContainer(this);
		newPane->SetContent(frame);
		frame->RecalcFrame();
	}

	mpDragPaneTarget->Dock(newPane, mDragPaneTargetCode);

	if (frame)
		NotifyFrameActivated(mpActiveFrame);

	return newPane;
}

void ATContainerWindow::AddUndockedFrame(ATFrameWindow *frame) {
	VDASSERT(std::find(mUndockedFrames.begin(), mUndockedFrames.end(), frame) == mUndockedFrames.end());

	mUndockedFrames.push_back(frame);
	frame->SetContainer(this);
}

void ATContainerWindow::UndockFrame(ATFrameWindow *frame, bool visible) {
	HWND hwndFrame = frame->GetHandleW32();
	UINT style = GetWindowLong(hwndFrame, GWL_STYLE);

	if (mpActiveFrame == frame) {
		mpActiveFrame = NULL;

		if (!visible)
			::SetFocus(mhwnd);
	}

	if (mpFullScreenFrame == frame) {
		mpFullScreenFrame = frame;
		frame->SetFullScreen(false);
	}

	if (style & WS_CHILD) {
		ShowWindow(hwndFrame, SW_HIDE);
		mpDockingPane->Undock(frame);

		RECT r;
		GetWindowRect(hwndFrame, &r);

		HWND hwndOwner = GetWindow(mhwnd, GW_OWNER);
		SetParent(hwndFrame, hwndOwner);

		style &= ~WS_CHILD;
		style |= WS_OVERLAPPEDWINDOW;
		SetWindowLong(hwndFrame, GWL_STYLE, style);

		UINT exstyle = GetWindowLong(hwndFrame, GWL_EXSTYLE);
		exstyle |= WS_EX_TOOLWINDOW;
		SetWindowLong(hwndFrame, GWL_EXSTYLE, exstyle);

		SetWindowPos(hwndFrame, NULL, r.left, r.top, 0, 0, SWP_NOSIZE|SWP_FRAMECHANGED|SWP_NOACTIVATE|SWP_NOZORDER|SWP_HIDEWINDOW);
		SendMessage(hwndFrame, WM_CHANGEUISTATE, MAKELONG(UIS_INITIALIZE, UISF_HIDEACCEL|UISF_HIDEFOCUS), 0);

		if (visible)
			ShowWindow(hwndFrame, SW_SHOWNA);

		VDASSERT(std::find(mUndockedFrames.begin(), mUndockedFrames.end(), frame) == mUndockedFrames.end());
		mUndockedFrames.push_back(frame);

		::SetActiveWindow(hwndFrame);
	}
}

void ATContainerWindow::SetFullScreenFrame(ATFrameWindow *frame) {
	if (mpFullScreenFrame == frame)
		return;

	mpFullScreenFrame = frame;

	LONG exStyle = GetWindowLong(mhwnd, GWL_EXSTYLE);

//	if (frame)
//		exStyle &= ~WS_EX_CLIENTEDGE;
//	else
//		exStyle |= WS_EX_CLIENTEDGE;

	SetWindowLong(mhwnd, GWL_EXSTYLE, exStyle);
	SetWindowPos(mhwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

	ATContainerResizer resizer;
	mpDockingPane->Relayout(resizer);
	resizer.Flush();
}

void ATContainerWindow::ActivateFrame(ATFrameWindow *frame) {
	if (mpActiveFrame == frame)
		return;

	NotifyFrameActivated(frame);
}

void ATContainerWindow::RemoveAnyEmptyNodes() {
	if (mpDockingPane)
		mpDockingPane->RemoveAnyEmptyNodes();
}

void ATContainerWindow::NotifyFrameActivated(ATFrameWindow *frame) {
	if (mbBlockActiveUpdates)
		return;

	HWND hwndFrame = NULL;
	
	if (frame)
		hwndFrame = frame->GetHandleW32();

	VDASSERT(!hwndFrame || !(GetWindowLong(hwndFrame, GWL_STYLE) & WS_CHILD) || GetAncestor(hwndFrame, GA_ROOT) == mhwnd);
	mpActiveFrame = frame;

	if (mpDockingPane)
		mpDockingPane->UpdateActivationState(frame);
}

void ATContainerWindow::NotifyUndockedFrameDestroyed(ATFrameWindow *frame) {
	UndockedFrames::iterator it = std::find(mUndockedFrames.begin(), mUndockedFrames.end(), frame);
	if (it != mUndockedFrames.end())
		mUndockedFrames.erase(it);
}

LRESULT ATContainerWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
		case WM_CREATE:
			if (!OnCreate())
				return -1;
			break;

		case WM_DESTROY:
			OnDestroy();
			break;

		case WM_SIZE:
			OnSize();
			break;

		case WM_PARENTNOTIFY:
			if (LOWORD(wParam) == WM_CREATE)
				OnSize();
			else if (LOWORD(wParam) == WM_DESTROY)
				OnChildDestroy((HWND)lParam);
			break;

		case WM_NCACTIVATE:
			if (wParam != 0)
				mpDockingPane->UpdateActivationState(mpActiveFrame);
			else
				mpDockingPane->UpdateActivationState(NULL);
			break;

		case WM_SETFOCUS:
			OnSetFocus((HWND)wParam);
			break;

		case WM_KILLFOCUS:
			OnKillFocus((HWND)wParam);
			break;

		case WM_ACTIVATE:
			if (OnActivate(LOWORD(wParam), HIWORD(wParam) != 0, (HWND)lParam))
				return 0;
			break;

		case WM_SYSCOLORCHANGE:
		case WM_THEMECHANGED:
			if (mpDockingPane)
				mpDockingPane->RecalcFrame();
			break;
	}

	return VDShaderEditorBaseWindow::WndProc(msg, wParam, lParam);
}

bool ATContainerWindow::OnCreate() {
	RecreateSystemObjects();
	OnSize();
	return true;
}

void ATContainerWindow::OnDestroy() {
	DestroySystemObjects();
}

void ATContainerWindow::OnSize() {
	RECT r;
	GetClientRect(mhwnd, &r);

	ATContainerResizer resizer;
	mpDockingPane->SetArea(resizer, vdrect32(0, 0, r.right, r.bottom), false);
	resizer.Flush();
}

void ATContainerWindow::OnChildDestroy(HWND hwndChild) {
	ATFrameWindow *frame = ATFrameWindow::GetFrameWindow(hwndChild);

	if (frame) {
		if (mpActiveFrame == frame) {
			mpActiveFrame = NULL;

			ATFrameWindow *frameToActivate = NULL;

			for(ATContainerDockingPane *pane = frame->GetPane(); pane; pane = pane->GetParentPane()) {
				ATContainerDockingPane *pane2 = pane->GetCenterPane();
				
				if (!pane2)
					continue;

				ATFrameWindow *frame2 = pane2->GetContent();

				if (frame2 && frame2 != frame) {
					frameToActivate = frame2;
					break;
				}
			}

			if (!frameToActivate)
				frameToActivate = mpDockingPane->GetAnyContent();

			if (frameToActivate) {
				::SetFocus(frameToActivate->GetHandleW32());
				NotifyFrameActivated(frameToActivate);
			} else {
				::SetFocus(mhwnd);
			}
		}

		ShowWindow(hwndChild, SW_HIDE);
		UndockFrame(frame, false);
	}
}

void ATContainerWindow::OnSetFocus(HWND hwndOldFocus) {
	if (mpActiveFrame) {
		VDASSERT(mpActiveFrame->GetContainer() == this);

		NotifyFrameActivated(mpActiveFrame);

		HWND hwndActiveFrame = mpActiveFrame->GetHandleW32();
		SetFocus(hwndActiveFrame);
	}
}

void ATContainerWindow::OnKillFocus(HWND hwndNewFocus) {
}

bool ATContainerWindow::OnActivate(UINT code, bool minimized, HWND hwnd) {
	if (code != WA_INACTIVE && !minimized) {
		if (mpActiveFrame) {
			VDASSERT(mpActiveFrame->GetContainer() == this);

			NotifyFrameActivated(mpActiveFrame);

			HWND hwndActiveFrame = mpActiveFrame->GetHandleW32();
			if (hwndActiveFrame)
				SetFocus(hwndActiveFrame);
		}
	}

	return true;
}

void ATContainerWindow::RecreateSystemObjects() {
	DestroySystemObjects();

	NONCLIENTMETRICS ncm = {sizeof(NONCLIENTMETRICS)};

	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, FALSE);

	mhfontCaption = CreateFontIndirect(&ncm.lfSmCaptionFont);

	LOGFONT lf = ncm.lfSmCaptionFont;
	lf.lfEscapement = 0;
	lf.lfItalic = FALSE;
	lf.lfUnderline = FALSE;
	lf.lfWeight = 0;
	vdwcslcpy(lf.lfFaceName, L"Marlett", sizeof(lf.lfFaceName)/sizeof(lf.lfFaceName[0]));

	mhfontCaptionSymbol = CreateFontIndirect(&lf);
}

void ATContainerWindow::DestroySystemObjects() {
	if (mhfontCaption) {
		DeleteFont(mhfontCaption);
		mhfontCaption = NULL;
	}

	if (mhfontCaptionSymbol) {
		DeleteFont(mhfontCaptionSymbol);
		mhfontCaptionSymbol = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////

ATFrameWindow::ATFrameWindow()
	: mbDragging(false)
	, mbFullScreen(false)
	, mbActiveCaption(false)
	, mbCloseDown(false)
	, mbCloseTracking(false)
	, mpDockingPane(NULL)
	, mpContainer(NULL)
{
}

ATFrameWindow::~ATFrameWindow() {
}

ATFrameWindow *ATFrameWindow::GetFrameWindow(HWND hwnd) {
	if (hwnd) {
		VDShaderEditorBaseWindow *w = (VDShaderEditorBaseWindow *)GetWindowLongPtr(hwnd, 0);
		return vdpoly_cast<ATFrameWindow *>(w);
	}

	return NULL;
}

void *ATFrameWindow::AsInterface(uint32 iid) {
	if (iid == ATFrameWindow::kTypeID)
		return static_cast<ATFrameWindow *>(this);

	return VDShaderEditorBaseWindow::AsInterface(iid);
}

bool ATFrameWindow::IsFullScreen() const {
	return mbFullScreen;
}

void ATFrameWindow::SetFullScreen(bool fs) {
	if (mbFullScreen == fs)
		return;

	mbFullScreen = fs;

	if (mpContainer)
		mpContainer->SetFullScreenFrame(fs ? this : NULL);

	if (mpDockingPane)
		mpDockingPane->UpdateFullScreenState();

	if (mhwnd) {
		LONG style = GetWindowLong(mhwnd, GWL_STYLE);
		LONG exStyle = GetWindowLong(mhwnd, GWL_EXSTYLE);

		if (fs) {
			style &= ~(WS_CAPTION | WS_THICKFRAME | WS_POPUP);
			if (!(style & WS_CHILD))
				style |= WS_POPUP;
			exStyle &= ~WS_EX_TOOLWINDOW;
		} else {
			style &= ~WS_POPUP;
			style |= WS_CAPTION;

			if (!(style & WS_CHILD))
				style |= WS_THICKFRAME;

			exStyle |= WS_EX_TOOLWINDOW;
		}

		SetWindowLong(mhwnd, GWL_STYLE, style);
		SetWindowLong(mhwnd, GWL_EXSTYLE, exStyle);
		SetWindowPos(mhwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
}

void ATFrameWindow::NotifyFontsUpdated() {
	if (mhwnd) {
		HWND hwndChild = GetWindow(mhwnd, GW_CHILD);

		if (hwndChild)
			SendMessage(hwndChild, ATWM_FONTSUPDATED, 0, 0);
	}
}

bool ATFrameWindow::GetIdealSize(vdsize32& sz) {
	if (!mhwnd)
		return false;

	sz.w = 0;
	sz.h = 0;

	HWND hwndChild = GetWindow(mhwnd, GW_CHILD);
	if (!hwndChild)
		return false;

	if (!SendMessage(hwndChild, ATWM_GETAUTOSIZE, 0, (LPARAM)&sz))
		return false;

	NONCLIENTMETRICS ncm = {sizeof(NONCLIENTMETRICS)};
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, FALSE);

	sz.h += ncm.iSmCaptionHeight;
	sz.w += 2*GetSystemMetrics(SM_CXEDGE);
	sz.h += 2*GetSystemMetrics(SM_CYEDGE);

	return true;
}

void ATFrameWindow::RecalcFrame() {
	if (mhwnd) {
		SetWindowPos(mhwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
		OnSize();
	}
}

VDGUIHandle ATFrameWindow::Create(const wchar_t *title, int x, int y, int cx, int cy, VDGUIHandle parent) {
	return (VDGUIHandle)CreateWindowExW(WS_EX_TOOLWINDOW, (LPCWSTR)sWndClass, title, WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, x, y, cx, cy, (HWND)parent, NULL, VDGetLocalModuleHandleW32(), static_cast<VDShaderEditorBaseWindow *>(this));
}

VDGUIHandle ATFrameWindow::CreateChild(const wchar_t *title, int x, int y, int cx, int cy, VDGUIHandle parent) {
	return (VDGUIHandle)CreateWindowExW(WS_EX_TOOLWINDOW, (LPCWSTR)sWndClass, title, WS_CHILD|WS_CAPTION|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, x, y, cx, cy, (HWND)parent, NULL, VDGetLocalModuleHandleW32(), static_cast<VDShaderEditorBaseWindow *>(this));
}

LRESULT ATFrameWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
		case WM_CREATE:
			mTitle = (const TCHAR *)((LPCREATESTRUCT)lParam)->lpszName;
			if (!OnCreate())
				return -1;
			break;

		case WM_DESTROY:
			OnDestroy();
			break;

		case WM_SIZE:
			OnSize();
			break;

		case WM_PARENTNOTIFY:
			if (LOWORD(wParam) == WM_CREATE)
				OnSize();
			break;

		case WM_NCLBUTTONDOWN:
			if (mpDockingPane && wParam == HTCLOSE) {
				mbCloseDown = true;
				mbCloseTracking = true;
				::SetCapture(mhwnd);
				PaintCaption(NULL);
				return 0;
			}

			if (OnNCLButtonDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
				return 0;
			break;

		case WM_LBUTTONUP:
			if (mbCloseTracking) {
				mbCloseTracking = false;
				::ReleaseCapture();

				int x = GET_X_LPARAM(lParam);
				int y = GET_Y_LPARAM(lParam);

				POINT pt = {x, y};
				ClientToScreen(mhwnd, &pt);

				x = pt.x;
				y = pt.y;

				RECT r = {};
				GetWindowRect(mhwnd, &r);

				x -= r.left;
				y -= r.top;

				mbCloseDown = false;

				PaintCaption(NULL);

				if (mCloseRect.contains(vdpoint32(x, y)))
					SendMessage(mhwnd, WM_SYSCOMMAND, SC_CLOSE, lParam);
			}

			if (mbDragging) {
				EndDrag(true);
			}
			break;

		case WM_MOUSEMOVE:
			if (mbCloseTracking) {
				int x = GET_X_LPARAM(lParam);
				int y = GET_Y_LPARAM(lParam);

				POINT pt = {x, y};
				ClientToScreen(mhwnd, &pt);

				x = pt.x;
				y = pt.y;

				RECT r = {};
				GetWindowRect(mhwnd, &r);

				x -= r.left;
				y -= r.top;

				bool closeDown = mCloseRect.contains(vdpoint32(x, y));

				if (mbCloseDown != closeDown) {
					mbCloseDown = closeDown;

					PaintCaption(NULL);
				}

				return 0;
			}

			if (OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
				return 0;
			break;

		case WM_CAPTURECHANGED:
			if (mbCloseTracking) {
				mbCloseTracking = false;
				return 0;
			}

			if ((HWND)lParam != mhwnd) {
				EndDrag(false);
			}
			break;

		case WM_KEYDOWN:
			if (mbDragging) {
				if (wParam == VK_ESCAPE) {
					EndDrag(false);
				}
			}
			break;

		case WM_CHILDACTIVATE:
		case WM_MOUSEACTIVATE:
			if (ATContainerWindow *cont = ATContainerWindow::GetContainerWindow(GetAncestor(mhwnd, GA_ROOTOWNER))) {
				cont->NotifyFrameActivated(this);

				if (msg == WM_MOUSEACTIVATE) {
					HWND focus = ::GetFocus();
					HWND hwndTest;

					for(hwndTest = focus; hwndTest && hwndTest != mhwnd; hwndTest = GetAncestor(hwndTest, GA_PARENT))
						;

					if (hwndTest != mhwnd)
						::SetFocus(mhwnd);
				}
			}
			break;

		case WM_SETFOCUS:
			{
				HWND hwndChild = GetWindow(mhwnd, GW_CHILD);

				if (hwndChild)
					SetFocus(hwndChild);
			}
			return 0;

		case WM_ERASEBKGND:
			return TRUE;

		case WM_NCACTIVATE:
			mbActiveCaption = (wParam != 0);

			if (mpDockingPane) {
				PaintCaption(NULL);

				// Toggle visible flag to prevent DefWindowProc() from painting the caption.
				DWORD prevFlags = GetWindowLong(mhwnd, GWL_STYLE);
				SetWindowLong(mhwnd, GWL_STYLE, prevFlags & ~WS_VISIBLE);
				LRESULT r = VDShaderEditorBaseWindow::WndProc(msg, wParam, lParam);
				SetWindowLong(mhwnd, GWL_STYLE, prevFlags);
				return r;
			}
			break;

		case WM_NCCALCSIZE:
			if (mpDockingPane) {
				RECT& r = *(RECT *)lParam;
				const int x = r.left;
				const int y = r.top;

				if (mbFullScreen) {
					mCaptionRect.set(0, 0, 0, 0);
				} else {
					NONCLIENTMETRICS ncm = {sizeof(NONCLIENTMETRICS)};

					SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, FALSE);

					const int h = ncm.iSmCaptionHeight;

					mCaptionRect.set(0, 0, r.right, h);

					int bsize = std::min<int>(GetSystemMetrics(SM_CXSMSIZE), GetSystemMetrics(SM_CYSMSIZE));

					mCloseRect.set(r.right - r.left - bsize, 0, r.right - r.left, h);

					r.top += h;
					if (r.top > r.bottom)
						r.top = r.bottom;

					int xe = GetSystemMetrics(SM_CXEDGE);
					int ye = GetSystemMetrics(SM_CYEDGE);
					r.left += xe;
					r.top += ye;
					r.right -= xe;
					r.bottom -= ye;
				}

				mClientRect.set(r.left, r.top, r.right, r.bottom);
				mClientRect.translate(-x, -y);
				return 0;
			}
			break;

		case WM_NCPAINT:
			if (mpDockingPane) {
				PaintCaption((HRGN)wParam);
				return 0;
			}
			break;

		case WM_NCHITTEST:
			if (mpDockingPane) {
				if (mbFullScreen)
					return HTCLIENT;

				int x = GET_X_LPARAM(lParam);
				int y = GET_Y_LPARAM(lParam);

				RECT r = {};
				GetWindowRect(mhwnd, &r);

				x -= r.left;
				y -= r.top;

				const vdpoint32 pt(x, y);

				if (mCloseRect.contains(pt))
					return HTCLOSE;

				if (mCaptionRect.contains(pt))
					return HTCAPTION;

				if (mClientRect.contains(pt))
					return HTCLIENT;

				return HTBORDER;
			}
			break;

		case WM_SETTEXT:
			mTitle = (const TCHAR *)lParam;
			if (mpDockingPane) {
				DWORD prevFlags = GetWindowLong(mhwnd, GWL_STYLE);
				SetWindowLong(mhwnd, GWL_STYLE, prevFlags & ~WS_VISIBLE);
				LRESULT r = VDShaderEditorBaseWindow::WndProc(msg, wParam, lParam);
				SetWindowLong(mhwnd, GWL_STYLE, prevFlags);
			}
			break;
	}

	return VDShaderEditorBaseWindow::WndProc(msg, wParam, lParam);
}

void ATFrameWindow::PaintCaption(HRGN clipRegion) {
	if (mbFullScreen)
		return;

	HDC hdc;
	
	if (clipRegion && clipRegion != (HRGN)1)
		hdc = GetDCEx(mhwnd, clipRegion, DCX_WINDOW | DCX_INTERSECTRGN | 0x10000);
	else
		hdc = GetDCEx(mhwnd, NULL, DCX_WINDOW | 0x10000);

	if (!hdc)
		return;

	NONCLIENTMETRICS ncm = {sizeof(NONCLIENTMETRICS)};

	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, FALSE);
	RECT r;
	GetWindowRect(mhwnd, &r);
	int x = r.left;
	int y = r.top;
	r.right -= r.left;
	r.bottom = ncm.iSmCaptionHeight;
	r.top = 0;
	r.left = 0;

	int xe = GetSystemMetrics(SM_CXEDGE);
	int ye = GetSystemMetrics(SM_CYEDGE);

	RECT rc;
	rc.left = mClientRect.left - xe;
	rc.top = mClientRect.top - ye;
	rc.right = mClientRect.right + xe;
	rc.bottom = mClientRect.bottom + ye;
	DrawEdge(hdc, &rc, EDGE_SUNKEN, BF_RECT);

	RECT r2 = r;
	if (r2.right < 0)
		r2.right = 0;

	BOOL gradientsEnabled = FALSE;
	SystemParametersInfo(SPI_GETGRADIENTCAPTIONS, 0, &gradientsEnabled, FALSE);

	if (gradientsEnabled) {
		const uint32 c0 = GetSysColor(mbActiveCaption ? COLOR_ACTIVECAPTION : COLOR_INACTIVECAPTION);
		const uint32 c1 = GetSysColor(mbActiveCaption ? COLOR_GRADIENTACTIVECAPTION : COLOR_GRADIENTINACTIVECAPTION);
		TRIVERTEX v[2];
		v[0].x = r.left;
		v[0].y = r.top;
		v[0].Red = (c0 & 0xff) << 8;
		v[0].Green = c0 & 0xff00;
		v[0].Blue = (c0 & 0xff0000) >> 8;
		v[1].x = r.right;
		v[1].y = r.bottom;
		v[1].Red = (c1 & 0xff) << 8;
		v[1].Green = c1 & 0xff00;
		v[1].Blue = (c1 & 0xff0000) >> 8;

		GRADIENT_RECT gr;
		gr.UpperLeft = 0;
		gr.LowerRight = 1;
		GradientFill(hdc, v, 2, &gr, 1, GRADIENT_FILL_RECT_H);
	} else {
		FillRect(hdc, &r2, mbActiveCaption ? (HBRUSH)(COLOR_ACTIVECAPTION + 1) : (HBRUSH)(COLOR_INACTIVECAPTION + 1));
	}
	
	if (mpContainer) {
		HFONT hfont = mpContainer->GetCaptionFont();
		if (hfont) {
			HGDIOBJ holdfont = SelectObject(hdc, hfont);

			if (holdfont) {
				//SetBkMode(hdc, OPAQUE);
				SetBkMode(hdc, TRANSPARENT);
				SetBkColor(hdc, GetSysColor(mbActiveCaption ? COLOR_ACTIVECAPTION : COLOR_INACTIVECAPTION));
				SetTextColor(hdc, GetSysColor(mbActiveCaption ? COLOR_CAPTIONTEXT : COLOR_INACTIVECAPTIONTEXT));
				SetTextAlign(hdc, TA_LEFT | TA_TOP);

				RECT rc = { mCaptionRect.left + xe*2, mCaptionRect.top, mCaptionRect.right, mCaptionRect.bottom };
				DrawText(hdc, mTitle.data(), mTitle.size(), &rc, DT_NOPREFIX | DT_SINGLELINE | DT_LEFT | DT_VCENTER);
				SelectObject(hdc, holdfont);
			}
		}

		HFONT hfont2 = mpContainer->GetCaptionSymbolFont();
		if (hfont2) {
			HGDIOBJ holdfont = SelectObject(hdc, hfont2);

			if (holdfont) {
				RECT r3;
				r3.left = mCloseRect.left;
				r3.top = mCloseRect.top;
				r3.right = mCloseRect.right;
				r3.bottom = mCloseRect.bottom;

				SetTextColor(hdc, RGB(0, 0, 0));
				if (mbCloseDown)
					DrawText(hdc, _T("r"), 1, &r3, DT_NOPREFIX | DT_LEFT | DT_BOTTOM | DT_SINGLELINE);
				else
					DrawText(hdc, _T("r"), 1, &r3, DT_NOPREFIX | DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				SelectObject(hdc, holdfont);
			}
		}
	}

	ReleaseDC(mhwnd, hdc);
}

bool ATFrameWindow::OnCreate() {
	OnSize();
	return true;
}

void ATFrameWindow::OnDestroy() {
	if (!mpDockingPane && mpContainer)
		mpContainer->NotifyUndockedFrameDestroyed(this);
}

void ATFrameWindow::OnSize() {
	RECT r;
	if (GetClientRect(mhwnd, &r)) {
		HWND hwndChild = GetWindow(mhwnd, GW_CHILD);

		if (hwndChild)
			SetWindowPos(hwndChild, NULL, 0, 0, r.right, r.bottom, SWP_NOZORDER|SWP_NOACTIVATE);
	}

	if (mpDockingPane)
		PaintCaption(NULL);
}

bool ATFrameWindow::OnNCLButtonDown(int code, int x, int y) {
	if (code != HTCAPTION)
		return false;

	RECT r;

	mbDragging = true;
	mbDragVerified = false;
	GetWindowRect(mhwnd, &r);

	mDragOriginX = x;
	mDragOriginY = y;
	mDragOffsetX = r.left - x;
	mDragOffsetY = r.top - y;

	mpDragContainer = ATContainerWindow::GetContainerWindow(GetWindow(mhwnd, GW_OWNER));

	SetForegroundWindow(mhwnd);
	SetActiveWindow(mhwnd);
	SetFocus(mhwnd);
	SetCapture(mhwnd);
	return true;
}

bool ATFrameWindow::OnMouseMove(int x, int y) {
	if (!mbDragging)
		return false;

	POINT pt = {x, y};
	ClientToScreen(mhwnd, &pt);

	if (!mbDragVerified) {
		int dx = abs(GetSystemMetrics(SM_CXDRAG));
		int dy = abs(GetSystemMetrics(SM_CYDRAG));

		if (abs(mDragOriginX - pt.x) <= dx && abs(mDragOriginY - pt.y) <= dy)
			return true;

		mbDragVerified = true;

		UINT style = GetWindowLong(mhwnd, GWL_STYLE);
		if (style & WS_CHILD) {
			mpDragContainer->UndockFrame(this);
		}

		if (mpDragContainer) {
			mpDragContainer->InitDragHandles();
		}
	}

	SetWindowPos(mhwnd, NULL, pt.x + mDragOffsetX, pt.y + mDragOffsetY, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);

	if (mpDragContainer)
		mpDragContainer->UpdateDragHandles(pt.x, pt.y);

	return true;
}

void ATFrameWindow::EndDrag(bool success) {
	if (mbDragging) {
		mbDragging = false;		// also prevents recursion
		if (GetCapture() == mhwnd)
			ReleaseCapture();

		if (mpDragContainer) {
			if (mbDragVerified) {
				if (success)
					mpDragContainer->DockFrame(this);

				mpDragContainer->ShutdownDragHandles();
			}

			mpDragContainer = NULL;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

namespace {
	typedef stdext::hash_map<uint32, ATPaneCreator> PaneCreators;
	PaneCreators g_paneCreatorMap;

	typedef stdext::hash_map<uint32, ATPaneClassCreator> PaneClassCreators;
	PaneClassCreators g_paneClassCreatorMap;

	typedef stdext::hash_map<uint32, ATUIPane *> ActivePanes;
	ActivePanes g_activePanes;

	HFONT	g_monoFont;
}

void ATRegisterUIPaneType(uint32 id, ATPaneCreator creator) {
	g_paneCreatorMap[id] = creator;
}

void ATRegisterUIPaneClass(uint32 id, ATPaneClassCreator creator) {
	g_paneClassCreatorMap[id] = creator;
}

void ATRegisterActiveUIPane(uint32 id, ATUIPane *w) {
	g_activePanes[id] = w;
}

void ATUnregisterActiveUIPane(uint32 id, ATUIPane *w) {
	g_activePanes.erase(id);
}

void ATGetUIPanes(vdfastvector<ATUIPane *>& panes) {
	for(ActivePanes::const_iterator it(g_activePanes.begin()), itEnd(g_activePanes.end());
		it != itEnd;
		++it)
	{
		panes.push_back(it->second);
	}
}

ATUIPane *ATGetUIPane(uint32 id) {
	ActivePanes::const_iterator it(g_activePanes.find(id));

	return it != g_activePanes.end() ? it->second : NULL;
}

ATUIPane *ATGetUIPaneByFrame(ATFrameWindow *frame) {
	if (!frame)
		return NULL;

	HWND hwndParent = frame->GetHandleW32();

	ActivePanes::const_iterator it(g_activePanes.begin()), itEnd(g_activePanes.end());
	for(; it != itEnd; ++it) {
		ATUIPane *pane = it->second;
		HWND hwndPane = pane->GetHandleW32();

		if (!hwndPane)
			continue;

		if (GetParent(hwndPane) == hwndParent)
			return pane;
	}

	return NULL;
}

void ATActivateUIPane(uint32 id, bool giveFocus, bool visible) {
	vdrefptr<ATUIPane> pane(ATGetUIPane(id));

	if (!pane) {
		if (id >= 0x100) {
			PaneClassCreators::const_iterator it(g_paneClassCreatorMap.find(id & 0xff00));
			if (it == g_paneClassCreatorMap.end())
				return;
		
			if (!it->second(id, ~pane))
				return;
		} else {
			PaneCreators::const_iterator it(g_paneCreatorMap.find(id));
			if (it == g_paneCreatorMap.end())
				return;
		
			if (!it->second(~pane))
				return;
		}

		vdrefptr<ATFrameWindow> frame(new ATFrameWindow);
		frame->Create(pane->GetUIPaneName(), CW_USEDEFAULT, CW_USEDEFAULT, 300, 200, (VDGUIHandle)g_pMainWindow->GetHandleW32());
		pane->Create(frame);

		int preferredCode = pane->GetPreferredDockCode();
		if (preferredCode >= 0 && visible)
			g_pMainWindow->DockFrame(frame, preferredCode);
		else
			g_pMainWindow->AddUndockedFrame(frame);

		if (visible)
			ShowWindow(frame->GetHandleW32(), SW_SHOWNOACTIVATE);
	}

	if (giveFocus) {
		HWND hwndPane = pane->GetHandleW32();
		HWND hwndPaneParent = GetParent(hwndPane);
		SetFocus(hwndPane);

		if (hwndPaneParent) {
			ATFrameWindow *frame = ATFrameWindow::GetFrameWindow(hwndPaneParent);

			// We must not set an undocked pane as activated, as it leads to focus badness
			// (the container window keeps giving activation away).
			if (frame && frame->GetPane())
				g_pMainWindow->NotifyFrameActivated(frame);
		}
	}
}

ATUIPane::ATUIPane(uint32 paneId, const wchar_t *name)
	: mPaneId(paneId)
	, mpName(name)
	, mDefaultWindowStyles(WS_CHILD|WS_CLIPCHILDREN)
	, mPreferredDockCode(-1)
{
}

ATUIPane::~ATUIPane() {
}

bool ATUIPane::Create(ATFrameWindow *frame) {
	HWND hwnd = CreateWindow((LPCTSTR)sWndClass, _T(""), mDefaultWindowStyles & ~WS_VISIBLE, 0, 0, 0, 0, frame->GetHandleW32(), (HMENU)100, VDGetLocalModuleHandleW32(), static_cast<VDShaderEditorBaseWindow *>(this));

	if (!hwnd)
		return false;

	::ShowWindow(hwnd, SW_SHOWNOACTIVATE);
	return true;
}

void ATUIPane::SetName(const wchar_t *name) {
	mpName = name;
}

LRESULT ATUIPane::WndProc(UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
		case WM_CREATE:
			if (!OnCreate())
				return -1;
			break;

		case WM_DESTROY:
			OnDestroy();
			break;

		case WM_SIZE:
			OnSize();
			break;

		case WM_SETFOCUS:
			OnSetFocus();
			return 0;

		case WM_COMMAND:
			if (OnCommand(LOWORD(wParam), HIWORD(wParam)))
				return 0;
			break;

		case ATWM_FONTSUPDATED:
			OnFontsUpdated();
			break;
	}

	return VDShaderEditorBaseWindow::WndProc(msg, wParam, lParam);
}

bool ATUIPane::OnCreate() {
	RegisterUIPane();
	OnSize();
	return true;
}

void ATUIPane::OnDestroy() {
	UnregisterUIPane();
}

void ATUIPane::OnSize() {
}

void ATUIPane::OnSetFocus() {
}

void ATUIPane::OnFontsUpdated() {
}

bool ATUIPane::OnCommand(uint32 id, uint32 extcode) {
	return false;
}

void ATUIPane::RegisterUIPane() {
	ATRegisterActiveUIPane(mPaneId, this);
}

void ATUIPane::UnregisterUIPane() {
	ATUnregisterActiveUIPane(mPaneId, this);
}
