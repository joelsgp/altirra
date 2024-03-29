//	Altirra - Atari 800/800XL/5200 emulator
//	UI library
//	Copyright (C) 2009-2012 Avery Lee
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

#ifndef f_AT_ATUI_DIALOG_H
#define f_AT_ATUI_DIALOG_H

#ifdef _MSC_VER
#pragma once
#endif

#include <vd2/system/function.h>
#include <vd2/system/vdstl.h>
#include <vd2/system/vectors.h>
#include <vd2/system/win32/miniwindows.h>
#include <at/atui/uiproxies.h>
#include <list>

class IVDUIDropFileList {
public:
	virtual bool GetFileName(int index, VDStringW& fileName) = 0;
};

#define VDWM_APP_POSTEDCALL (WM_APP + 0x400)

class VDDialogFrameW32 {
public:
	bool IsCreated() const { return mhdlg != NULL; }
	VDZHWND GetWindowHandle() const { return mhdlg; }

	bool	Create(VDGUIHandle hwndParent);
	bool	Create(VDDialogFrameW32 *parent);
	void	Destroy();
	void	Close();

	void	Show();
	void	Hide();

	void Sync(bool writeToDataStore);

	void BringToFront();

	vdsize32 GetSize() const;
	void SetSize(const vdsize32& sz, bool repositionSafe = false);

	vdrect32 GetArea() const;
	void SetArea(const vdrect32& r, bool repositionSafe);
	void SetPosition(const vdpoint32& pt);

	vdrect32 GetClientArea() const;

	void AdjustPosition();
	void CenterOnParent();

	sintptr ShowDialog(VDGUIHandle hwndParent);
	sintptr ShowDialog(VDDialogFrameW32 *parent);

	static void ShowInfo(VDGUIHandle hParent, const wchar_t *message, const wchar_t *caption);
	static void SetDefaultCaption(const wchar_t *caption);

protected:
	VDDialogFrameW32(uint32 dlgid);

	void End(sintptr result);

	void AddProxy(VDUIProxyControl *proxy, uint32 id);

	void SetCurrentSizeAsMinSize();

	VDZHWND GetControl(uint32 id);

	void SetFocusToControl(uint32 id);
	void EnableControl(uint32 id, bool enabled);
	void ShowControl(uint32 id, bool visible);

	vdrect32 GetControlPos(uint32 id);
	vdrect32 GetControlScreenPos(uint32 id);

	void SetCaption(uint32 id, const wchar_t *format);

	bool GetControlText(uint32 id, VDStringW& s);
	void SetControlText(uint32 id, const wchar_t *s);
	void SetControlTextF(uint32 id, const wchar_t *format, ...);

	sint32 GetControlValueSint32(uint32 id);
	uint32 GetControlValueUint32(uint32 id);
	double GetControlValueDouble(uint32 id);
	VDStringW GetControlValueString(uint32 id);

	void ExchangeControlValueBoolCheckbox(bool write, uint32 id, bool& val);
	void ExchangeControlValueSint32(bool write, uint32 id, sint32& val, sint32 minVal, sint32 maxVal);
	void ExchangeControlValueUint32(bool write, uint32 id, uint32& val, uint32 minVal, uint32 maxVal);
	void ExchangeControlValueDouble(bool write, uint32 id, const wchar_t *format, double& val, double minVal, double maxVal);
	void ExchangeControlValueString(bool write, uint32 id, VDStringW& s);

	void CheckButton(uint32 id, bool checked);
	bool IsButtonChecked(uint32 id);

	int GetButtonTriState(uint32 id);
	void SetButtonTriState(uint32 id, int state);

	void BeginValidation();
	bool EndValidation();

	void FailValidation(uint32 id);
	void FailValidation(uint32 id, const wchar_t *msg);
	void SignalFailedValidation(uint32 id);

	void SetPeriodicTimer(uint32 id, uint32 msperiod);

	void ShowInfo(const wchar_t *message, const wchar_t *caption = nullptr);
	void ShowWarning(const wchar_t *message, const wchar_t *caption = nullptr);
	void ShowError(const wchar_t *message, const wchar_t *caption = nullptr);
	bool Confirm(const wchar_t *message, const wchar_t *caption = nullptr);

	int ActivateMenuButton(uint32 id, const wchar_t *const *items);
	int ActivatePopupMenu(int x, int y, const wchar_t *const *items);

	// listbox
	void LBClear(uint32 id);
	sint32 LBGetSelectedIndex(uint32 id);
	void LBSetSelectedIndex(uint32 id, sint32 idx);
	void LBAddString(uint32 id, const wchar_t *s);
	void LBAddStringF(uint32 id, const wchar_t *format, ...);

	// combobox
	void CBClear(uint32 id);
	sint32 CBGetSelectedIndex(uint32 id);
	void CBSetSelectedIndex(uint32 id, sint32 idx);
	void CBAddString(uint32 id, const wchar_t *s);

	// trackbar
	sint32 TBGetValue(uint32 id);
	void TBSetValue(uint32 id, sint32 value);
	void TBSetRange(uint32 id, sint32 minval, sint32 maxval);
	void TBSetPageStep(uint32 id, sint32 pageStep);

	// up/down controls
	void UDSetRange(uint32 id, sint32 minval, sint32 maxval);

	void PostCall(const vdfunction<void()>& call);

protected:
	virtual VDZINT_PTR DlgProc(VDZUINT msg, VDZWPARAM wParam, VDZLPARAM lParam);
	virtual void OnDataExchange(bool write);
	virtual bool OnLoaded();
	virtual bool OnOK();
	virtual bool OnCancel();
	virtual void OnSize();
	virtual bool OnClose();
	virtual void OnDestroy();
	virtual bool OnTimer(uint32 id);
	virtual bool OnErase(VDZHDC hdc);
	virtual bool OnCommand(uint32 id, uint32 extcode);
	virtual void OnDropFiles(VDZHDROP hDrop);
	virtual void OnDropFiles(IVDUIDropFileList *dropFileList);
	virtual void OnHScroll(uint32 id, int code);
	virtual void OnVScroll(uint32 id, int code);
	virtual void OnHelp();
	virtual void OnContextMenu(uint32 id, int x, int y);
	virtual bool PreNCDestroy();

	bool	mbValidationFailed;
	bool	mbIsModal;
	VDZHWND	mhdlg;
	int		mMinWidth;
	int		mMinHeight;

private:
	void ExecutePostedCalls();
	void SetDialogIcon();

	static VDZINT_PTR VDZCALLBACK StaticDlgProc(VDZHWND hwnd, VDZUINT msg, VDZWPARAM wParam, VDZLPARAM lParam);

	const char *mpDialogResourceName;
	uint32	mFailedId;
	VDStringW mFailedMsg;

	std::list<vdfunction<void()>> mPostedCalls;

	static const wchar_t *spDefaultCaption;

protected:
	VDUIProxyMessageDispatcherW32 mMsgDispatcher;
};

class VDDialogResizerW32 {
public:
	VDDialogResizerW32();
	~VDDialogResizerW32();

	enum {
		kAnchorX1_C	= 0x01,
		kAnchorX1_R	= 0x02,
		kAnchorX2_C	= 0x04,
		kAnchorX2_R	= 0x08,
		kAnchorY1_M	= 0x10,
		kAnchorY1_B	= 0x20,
		kAnchorY2_M	= 0x40,
		kAnchorY2_B	= 0x80,

		kL		= 0,
		kC		= kAnchorX2_R,
		kR		= kAnchorX2_R | kAnchorX1_R,
		kHMask	= 0x0F,

		kT		= 0,
		kM		= kAnchorY2_B,
		kB		= kAnchorY2_B | kAnchorY1_B,
		kVMask	= 0xF0,

		kX1Y1Mask = 0x33,
		kX2Y2Mask = 0xCC,

		kTL		= kT | kL,
		kTR		= kT | kR,
		kTC		= kT | kC,
		kML		= kM | kL,
		kMR		= kM | kR,
		kMC		= kM | kC,
		kBL		= kB | kL,
		kBR		= kB | kR,
		kBC		= kB | kC,

		kAvoidFlicker = 0x100
	};

	void Init(VDZHWND hwnd);
	void Relayout();
	void Relayout(int width, int height);
	void Add(uint32 id, int alignment);

	void Erase(const VDZHDC *phdc);

protected:
	struct ControlEntry {
		VDZHWND	mhwnd;
		int		mAlignment;
		sint32	mX1;
		sint32	mY1;
		sint32	mX2;
		sint32	mY2;
	};

	VDZHWND	mhwndBase;
	int		mWidth;
	int		mHeight;

	typedef vdfastvector<ControlEntry> Controls;
	Controls mControls;
};

#endif
