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

#ifndef f_AT_ATUI_UIPROXIES_H
#define f_AT_ATUI_UIPROXIES_H

#include <vd2/system/event.h>
#include <vd2/system/refcount.h>
#include <vd2/system/unknown.h>
#include <vd2/system/vdstl.h>
#include <vd2/system/vectors.h>
#include <vd2/system/VDString.h>
#include <vd2/system/win32/miniwindows.h>

#include <vd2/system/function.h>

struct VDUIAccelerator;
class VDFunctionThunk;

class VDUIProxyControl : public vdlist_node {
public:
	VDUIProxyControl();

	VDZHWND GetHandle() const { return mhwnd; }

	virtual void Attach(VDZHWND hwnd);
	virtual void Detach();

	void SetArea(const vdrect32& r);

	void SetEnabled(bool);
	void SetRedraw(bool);

	virtual VDZLRESULT On_WM_COMMAND(VDZWPARAM wParam, VDZLPARAM lParam);
	virtual VDZLRESULT On_WM_NOTIFY(VDZWPARAM wParam, VDZLPARAM lParam);

protected:
	VDZHWND	mhwnd;
	int mRedrawInhibitCount;
};

class VDUIProxyMessageDispatcherW32 {
public:
	void AddControl(VDUIProxyControl *control);
	void RemoveControl(VDZHWND hwnd);
	void RemoveAllControls(bool detach);

	VDZLRESULT Dispatch_WM_COMMAND(VDZWPARAM wParam, VDZLPARAM lParam);
	VDZLRESULT Dispatch_WM_NOTIFY(VDZWPARAM wParam, VDZLPARAM lParam);

protected:
	size_t Hash(VDZHWND hwnd) const;
	VDUIProxyControl *GetControl(VDZHWND hwnd);

	enum { kHashTableSize = 31 };

	typedef vdlist<VDUIProxyControl> HashChain;
	HashChain mHashTable[kHashTableSize];
};

/////////////////////////////////////////////////////////////////////////////

class IVDUIListViewVirtualItem : public IVDRefCount {
public:
	virtual void GetText(int subItem, VDStringW& s) const = 0;
};

class IVDUIListViewVirtualComparer {
public:
	virtual int Compare(IVDUIListViewVirtualItem *x, IVDUIListViewVirtualItem *y) = 0;
};

class VDUIProxyListView : public VDUIProxyControl {
public:
	VDUIProxyListView();

	void AutoSizeColumns(bool expandlast = false);
	void Clear();
	void ClearExtraColumns();
	void DeleteItem(int index);
	int GetColumnCount() const;
	int GetItemCount() const;
	int GetSelectedIndex() const;
	void SetSelectedIndex(int index);
	IVDUIListViewVirtualItem *GetSelectedItem() const;
	void GetSelectedIndices(vdfastvector<int>& indices) const;
	void SetFullRowSelectEnabled(bool enabled);
	void SetGridLinesEnabled(bool enabled);
	void SetItemCheckboxesEnabled(bool enabled);
	void EnsureItemVisible(int index);
	int GetVisibleTopIndex();
	void SetVisibleTopIndex(int index);
	IVDUIListViewVirtualItem *GetSelectedVirtualItem() const;
	IVDUIListViewVirtualItem *GetVirtualItem(int index) const;
	void InsertColumn(int index, const wchar_t *label, int width, bool rightAligned = false);
	int InsertItem(int item, const wchar_t *text);
	int InsertVirtualItem(int item, IVDUIListViewVirtualItem *lvvi);
	void RefreshItem(int item);
	void RefreshAllItems();
	void EditItemLabel(int item);
	void GetItemText(int item, VDStringW& s) const;
	void SetItemText(int item, int subitem, const wchar_t *text);

	bool IsItemChecked(int item);
	void SetItemChecked(int item, bool checked);
	void SetItemCheckedVisible(int item, bool visible);

	void SetItemImage(int item, uint32 imageIndex);

	bool GetItemScreenRect(int item, vdrect32& r) const;

	void Sort(IVDUIListViewVirtualComparer& comparer);

	VDEvent<VDUIProxyListView, int>& OnColumnClicked() {
		return mEventColumnClicked;
	}

	VDEvent<VDUIProxyListView, int>& OnItemSelectionChanged() {
		return mEventItemSelectionChanged;
	}

	VDEvent<VDUIProxyListView, int>& OnItemDoubleClicked() {
		return mEventItemDoubleClicked;
	}

	struct ContextMenuEvent {
		int mIndex;
		int mX;
		int mY;
	};

	VDEvent<VDUIProxyListView, ContextMenuEvent>& OnItemContextMenu() {
		return mEventItemContextMenu;
	}

	struct CheckedChangingEvent {
		int mIndex;
		bool mbNewVisible;
		bool mbNewChecked;
		bool mbAllowChange;
	};

	VDEvent<VDUIProxyListView, CheckedChangingEvent *>& OnItemCheckedChanging() {
		return mEventItemCheckedChanging;
	}

	VDEvent<VDUIProxyListView, int>& OnItemCheckedChanged() {
		return mEventItemCheckedChanged;
	}

	struct LabelChangedEvent {
		bool mbAllowEdit;
		int mIndex;
		const wchar_t *mpNewLabel;
	};

	VDEvent<VDUIProxyListView, LabelChangedEvent *>& OnItemLabelChanged() {
		return mEventItemLabelEdited;
	}

	VDEvent<VDUIProxyListView, int>& OnItemBeginDrag() {
		return mEventItemBeginDrag;
	}

	VDEvent<VDUIProxyListView, int>& OnItemBeginRDrag() {
		return mEventItemBeginRDrag;
	}

protected:
	void Detach();

	static int VDZCALLBACK SortAdapter(VDZLPARAM x, VDZLPARAM y, VDZLPARAM cookie);
	VDZLRESULT On_WM_NOTIFY(VDZWPARAM wParam, VDZLPARAM lParam);

	int			mChangeNotificationLocks;
	int			mNextTextIndex;
	VDStringW	mTextW[3];
	VDStringA	mTextA[3];

	vdfastvector<int>	mColumnWidthCache;

	VDEvent<VDUIProxyListView, int> mEventColumnClicked;
	VDEvent<VDUIProxyListView, int> mEventItemSelectionChanged;
	VDEvent<VDUIProxyListView, int> mEventItemDoubleClicked;
	VDEvent<VDUIProxyListView, int> mEventItemCheckedChanged;
	VDEvent<VDUIProxyListView, CheckedChangingEvent *> mEventItemCheckedChanging;
	VDEvent<VDUIProxyListView, ContextMenuEvent> mEventItemContextMenu;
	VDEvent<VDUIProxyListView, LabelChangedEvent *> mEventItemLabelEdited;
	VDEvent<VDUIProxyListView, int> mEventItemBeginDrag;
	VDEvent<VDUIProxyListView, int> mEventItemBeginRDrag;
};

/////////////////////////////////////////////////////////////////////////////

class VDUIProxyHotKeyControl : public VDUIProxyControl {
public:
	VDUIProxyHotKeyControl();
	~VDUIProxyHotKeyControl();

	bool GetAccelerator(VDUIAccelerator& accel) const;
	void SetAccelerator(const VDUIAccelerator& accel);

	VDEvent<VDUIProxyHotKeyControl, VDUIAccelerator>& OnEventHotKeyChanged() {
		return mEventHotKeyChanged;
	}

protected:
	VDZLRESULT On_WM_COMMAND(VDZWPARAM wParam, VDZLPARAM lParam);

	VDEvent<VDUIProxyHotKeyControl, VDUIAccelerator> mEventHotKeyChanged;
};

/////////////////////////////////////////////////////////////////////////////

class VDUIProxyTabControl : public VDUIProxyControl {
public:
	VDUIProxyTabControl();
	~VDUIProxyTabControl();

	void AddItem(const wchar_t *s);
	void DeleteItem(int index);

	vdsize32 GetControlSizeForContent(const vdsize32&) const;
	vdrect32 GetContentArea() const;

	int GetSelection() const;
	void SetSelection(int index);

	VDEvent<VDUIProxyTabControl, int>& OnSelectionChanged() {
		return mSelectionChanged;
	}

protected:
	VDZLRESULT On_WM_NOTIFY(VDZWPARAM wParam, VDZLPARAM lParam);

	VDEvent<VDUIProxyTabControl, int> mSelectionChanged;
};

/////////////////////////////////////////////////////////////////////////////

class VDUIProxyListBoxControl : public VDUIProxyControl {
public:
	VDUIProxyListBoxControl();
	~VDUIProxyListBoxControl();

	void Clear();
	int AddItem(const wchar_t *s, uintptr data = 0);
	uintptr GetItemData(int index) const;

	int GetSelection() const;
	void SetSelection(int index);

	void MakeSelectionVisible();

	void SetTabStops(const int *units, uint32 n);

	VDEvent<VDUIProxyListBoxControl, int>& OnSelectionChanged() {
		return mSelectionChanged;
	}

	VDEvent<VDUIProxyListBoxControl, int>& OnItemDoubleClicked() {
		return mEventItemDoubleClicked;
	}

protected:
	VDZLRESULT On_WM_COMMAND(VDZWPARAM wParam, VDZLPARAM lParam);

	VDEvent<VDUIProxyListBoxControl, int> mSelectionChanged;
	VDEvent<VDUIProxyListBoxControl, int> mEventItemDoubleClicked;
};

/////////////////////////////////////////////////////////////////////////////

class VDUIProxyComboBoxControl : public VDUIProxyControl {
public:
	VDUIProxyComboBoxControl();
	~VDUIProxyComboBoxControl();

	void AddItem(const wchar_t *s);

	int GetSelection() const;
	void SetSelection(int index);

	VDEvent<VDUIProxyComboBoxControl, int>& OnSelectionChanged() {
		return mSelectionChanged;
	}

protected:
	VDZLRESULT On_WM_COMMAND(VDZWPARAM wParam, VDZLPARAM lParam);

	VDEvent<VDUIProxyComboBoxControl, int> mSelectionChanged;
};

/////////////////////////////////////////////////////////////////////////////

class IVDUITreeViewVirtualItem : public IVDRefUnknown {
public:
	virtual void GetText(VDStringW& s) const = 0;
};

class IVDUITreeViewVirtualItemComparer {
public:
	virtual int Compare(IVDUITreeViewVirtualItem& x, IVDUITreeViewVirtualItem& y) const = 0;
};

class VDUIProxyTreeViewControl : public VDUIProxyControl {
public:
	typedef uintptr NodeRef;

	static const NodeRef kNodeRoot;
	static const NodeRef kNodeFirst;
	static const NodeRef kNodeLast;

	VDUIProxyTreeViewControl();
	~VDUIProxyTreeViewControl();

	virtual void Attach(VDZHWND hwnd);
	virtual void Detach();

	IVDUITreeViewVirtualItem *GetSelectedVirtualItem() const;

	void Clear();
	void DeleteItem(NodeRef ref);
	NodeRef AddItem(NodeRef parent, NodeRef insertAfter, const wchar_t *label);
	NodeRef AddVirtualItem(NodeRef parent, NodeRef insertAfter, IVDUITreeViewVirtualItem *item);

	void MakeNodeVisible(NodeRef node);
	void SelectNode(NodeRef node);
	void RefreshNode(NodeRef node);
	void ExpandNode(NodeRef node, bool expanded);
	void EditNodeLabel(NodeRef node);
	void EnumChildren(NodeRef parent, const vdfunction<void(IVDUITreeViewVirtualItem *)>& callback);
	void SortChildren(NodeRef parent, IVDUITreeViewVirtualItemComparer& comparer);

	VDEvent<VDUIProxyTreeViewControl, int>& OnItemSelectionChanged() {
		return mEventItemSelectionChanged;
	}

	VDEvent<VDUIProxyTreeViewControl, bool *>& OnItemDoubleClicked() {
		return mEventItemDoubleClicked;
	}

	struct BeginEditEvent {
		NodeRef mNode;
		IVDUITreeViewVirtualItem *mpItem;
		bool mbAllowEdit;
		bool mbOverrideText;
		VDStringW mOverrideText;
	};

	VDEvent<VDUIProxyTreeViewControl, BeginEditEvent *>& OnItemBeginEdit() {
		return mEventItemBeginEdit;
	}

	struct EndEditEvent {
		NodeRef mNode;
		IVDUITreeViewVirtualItem *mpItem;
		const wchar_t *mpNewText;
	};

	VDEvent<VDUIProxyTreeViewControl, EndEditEvent *>& OnItemEndEdit() {
		return mEventItemEndEdit;
	}

	struct GetDispAttrEvent {
		IVDUITreeViewVirtualItem *mpItem;
		bool mbIsBold;
	};

	VDEvent<VDUIProxyTreeViewControl, GetDispAttrEvent *>& OnItemGetDisplayAttributes() {
		return mEventItemGetDisplayAttributes;
	}

protected:
	VDZLRESULT On_WM_NOTIFY(VDZWPARAM wParam, VDZLPARAM lParam);
	VDZLRESULT FixLabelEditWndProcA(VDZHWND hwnd, VDZUINT msg, VDZWPARAM wParam, VDZLPARAM lParam);
	VDZLRESULT FixLabelEditWndProcW(VDZHWND hwnd, VDZUINT msg, VDZWPARAM wParam, VDZLPARAM lParam);

	int			mNextTextIndex;
	VDStringW mTextW[3];
	VDStringA mTextA[3];
	VDZHFONT	mhfontBold;
	bool		mbCreatedBoldFont;

	void *mPrevEditWndProc;
	VDFunctionThunk *mpEditWndProcThunk;

	VDEvent<VDUIProxyTreeViewControl, int> mEventItemSelectionChanged;
	VDEvent<VDUIProxyTreeViewControl, bool *> mEventItemDoubleClicked;
	VDEvent<VDUIProxyTreeViewControl, BeginEditEvent *> mEventItemBeginEdit;
	VDEvent<VDUIProxyTreeViewControl, EndEditEvent *> mEventItemEndEdit;
	VDEvent<VDUIProxyTreeViewControl, GetDispAttrEvent *> mEventItemGetDisplayAttributes;
};

#endif
