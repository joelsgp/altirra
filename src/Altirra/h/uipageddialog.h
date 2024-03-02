//	Altirra - Atari 800/800XL/5200 emulator
//	Copyright (C) 2009-2018 Avery Lee
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

#ifndef f_AT_UIPAGEDDIALOG_H
#define f_AT_UIPAGEDDIALOG_H

#include <vd2/system/vdtypes.h>
#include <vd2/system/vdalloc.h>
#include <vd2/system/vdstl.h>
#include <vd2/system/vdstl_vectorview.h>
#include <vd2/system/win32/miniwindows.h>
#include <at/atnativeui/dialog.h>
#include <at/atnativeui/uiproxies.h>

class VDFunctionThunk;

class ATUIDialogPage : public VDDialogFrameW32 {
public:
	using VDDialogFrameW32::VDDialogFrameW32;

	virtual ~ATUIDialogPage() = default;

	struct HelpEntry {
		uint32 mId;
		uint32 mLinkedId;
		vdrect32 mArea;
		VDStringW mLabel;
		VDStringW mText;
	};

	const HelpEntry *GetHelpEntry(const vdpoint32& pt) const;

	virtual void ExchangeOtherSettings(bool write);

protected:
	void AddHelpEntry(uint32 id, const wchar_t *label, const wchar_t *s);
	void LinkHelpEntry(uint32 id, uint32 linkedId);
	void ClearHelpEntries();

	typedef vdvector<HelpEntry> HelpEntries;
	HelpEntries mHelpEntries;
};

class ATUIPagedDialog : public VDDialogFrameW32 {
public:
	ATUIPagedDialog(uint32 id);
	~ATUIPagedDialog();

	void SetInitialPage(int index);
	int GetSelectedPage() const { return mSelectedPage; }

protected:
	enum {
		kTimerID_Help = 10
	};

	bool OnLoaded() override;
	void OnDataExchange(bool write) override;
	void OnDestroy() override;
	bool OnTimer(uint32 id) override;
	void OnDpiChanged() override;

	void CheckFocus(const vdpoint32& pt);

	virtual void OnPopulatePages() = 0;

	void PushCategory(const wchar_t *name);
	void PopCategory();
	void AddPage(const wchar_t *name, vdautoptr<ATUIDialogPage> page);
	void SelectPage(int index);

	void AppendRTF(VDStringA& rtf, const wchar_t *text);

	void InstallMouseHook();
	void UninstallMouseHook();
	VDZLRESULT OnMouseEvent(int code, VDZWPARAM wParam, VDZLPARAM lParam);

	void OnTreeSelectedItemChanged();

	struct PageTreeItem;

	int mSelectedPage {};
	int mInitialPage {};
	uint32 mLastHelpId {};
	vdrect32 mLastHelpRect { 0, 0, 0, 0 };

	vdfastvector<ATUIDialogPage *> mPages;
	vdfastvector<PageTreeItem *> mPageTreeItems;
	vdfastvector<uintptr> mPageTreeCategories;

	VDFunctionThunkInfo *mpMouseFuncThunk = nullptr;
	void *mpMouseHook = nullptr;

	VDUIProxyRichEditControl mHelpView;
	VDUIProxyListBoxControl mPageListView;
	VDUIProxyTreeViewControl mPageTreeView;
	ATUINativeWindowProxy mPageAreaView;
};

#endif