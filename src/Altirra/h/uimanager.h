#ifndef f_AT2_UIMANAGER_H
#define f_AT2_UIMANAGER_H

#include <vd2/VDDisplay/compositor.h>
#include <vd2/VDDisplay/renderer.h>
#include <vd2/Kasumi/pixmaputils.h>
#include <vd2/system/event.h>
#include "ui.h"

class ATUIWidget;
class ATUIContainer;
class IVDDisplayFont;
class VDDisplayImageView;
struct ATUIKeyEvent;
struct ATUICharEvent;
struct ATUITriggerBinding;

enum ATUIThemeFont {
	kATUIThemeFont_Default,
	kATUIThemeFont_Header,
	kATUIThemeFont_Mono,
	kATUIThemeFont_MonoSmall,
	kATUIThemeFont_Menu,
	kATUIThemeFont_Tooltip,
	kATUIThemeFont_TooltipBold,
	kATUIThemeFontCount
};

enum ATUIStockImageIdx {
	kATUIStockImageIdx_MenuCheck,
	kATUIStockImageIdx_MenuRadio,
	kATUIStockImageIdx_MenuArrow,
	kATUIStockImageIdx_ButtonLeft,
	kATUIStockImageIdx_ButtonRight,
	kATUIStockImageIdx_ButtonUp,
	kATUIStockImageIdx_ButtonDown,
	kATUIStockImageIdxCount
};

struct ATUIStockImage {
	VDDisplayImageView mImageView;
	VDPixmapBuffer mBuffer;
	int mOffsetX;
	int mOffsetY;
	int mWidth;
	int mHeight;
};

struct ATUISystemMetrics {
	sint32 mVertSliderWidth;
};

struct ATUITouchInput {
	uint32 mId;
	sint32 mX;
	sint32 mY;
	bool mbDown;
	bool mbUp;
	bool mbPrimary;
};

class IATUIClipboard {
public:
	virtual void CopyText(const char *s) = 0;
};

class IATUINativeDisplay {
public:
	virtual void Invalidate() = 0;
	virtual void ConstrainCursor(bool constrain) = 0;
	virtual void CaptureCursor(bool motionMode) = 0;
	virtual void ReleaseCursor() = 0;
	virtual vdpoint32 GetCursorPosition() = 0;
	virtual void SetCursorImage(uint32 id) = 0;
	virtual void *BeginModal() = 0;
	virtual void EndModal(void *cookie) = 0;
	virtual bool IsKeyDown(uint32 vk) = 0;
	virtual IATUIClipboard *GetClipboard() = 0;
};

class ATUIManager : public IVDDisplayCompositor {
	ATUIManager(const ATUIManager&);
	ATUIManager& operator=(const ATUIManager&);
public:
	ATUIManager();
	~ATUIManager();

	int AddRef() { return 2; }
	int Release() { return 1; }

	void Init(IATUINativeDisplay *natDisplay);
	void Shutdown();

	IATUIClipboard *GetClipboard();

	ATUIContainer *GetMainWindow() const;
	ATUIWidget *GetFocusWindow() const;
	uint32 GetCurrentCursorImageId() const { return mCursorImageId; }
	bool IsCursorCaptured() const { return mbCursorCaptured; }
	ATUIWidget *GetCursorWindow() const { return mpCursorWindow; }
	ATUIWidget *GetCursorCaptureWindow() const { return mbCursorCaptured ? mpCursorWindow : NULL; }
	bool IsInvalidated() const { return mbInvalidated; }

	ATUIWidget *GetWindowByInstance(uint32 id) const;

	void BeginAction(ATUIWidget *w, const ATUITriggerBinding& binding);
	void EndAction(uint32 vk);

	void Resize(sint32 w, sint32 h);
	void SetForeground(bool foreground);
	void SetThemeScaleFactor(float scale);

	void SetActiveWindow(ATUIWidget *w);
	void CaptureCursor(ATUIWidget *w, bool motionMode = false, bool constrainPosition = false);

	/// Adds or removes a tracking window, which receives cursor window change
	/// notifications for itself and all windows below it.
	void AddTrackingWindow(ATUIWidget *w);
	void RemoveTrackingWindow(ATUIWidget *w);

	ATUIWidget *GetModalWindow() const { return mpModalWindow; }
	void BeginModal(ATUIWidget *w);
	void EndModal();

	bool IsKeyDown(uint32 vk);
	vdpoint32 GetCursorPosition();

	ATUITouchMode GetTouchModeAtPoint(const vdpoint32& pt) const;

	void OnTouchInput(const ATUITouchInput *inputs, uint32 n);

	bool OnMouseRelativeMove(sint32 dx, sint32 dy);
	bool OnMouseMove(sint32 x, sint32 y);
	bool OnMouseDown(sint32 x, sint32 y, uint32 vk, bool dblclk);
	bool OnMouseUp(sint32 x, sint32 y, uint32 vk);
	bool OnMouseWheel(sint32 x, sint32 y, float delta);
	void OnMouseLeave();
	void OnMouseHover(sint32 x, sint32 y);

	bool OnContextMenu(const vdpoint32 *pt);

	bool OnKeyDown(const ATUIKeyEvent& event);
	bool OnKeyUp(const ATUIKeyEvent& event);
	bool OnChar(const ATUICharEvent& event);

	void OnCaptureLost();

	IVDDisplayFont *GetThemeFont(ATUIThemeFont themeFont) const { return mpThemeFonts[themeFont]; }
	ATUIStockImage& GetStockImage(ATUIStockImageIdx stockImage) const { return *mpStockImages[stockImage]; }

	const ATUISystemMetrics& GetSystemMetrics() const { return mSystemMetrics; }

public:
	void Attach(ATUIWidget *w);
	void Detach(ATUIWidget *w);
	void Invalidate(ATUIWidget *w);
	void UpdateCursorImage(ATUIWidget *w);

public:
	void Composite(IVDDisplayRenderer& r, const VDDisplayCompositeInfo& compInfo);

protected:
	class ActiveAction;

	void UpdateCursorImage();
	bool UpdateCursorWindow(sint32 x, sint32 y);
	void SetCursorWindow(ATUIWidget *w);

	void LockDestroy();
	void UnlockDestroy();

	void RepeatAction(ActiveAction& action);
	void ReinitTheme();

	IATUINativeDisplay *mpNativeDisplay;
	ATUIContainer *mpMainWindow;
	ATUIWidget *mpCursorWindow;
	ATUIWidget *mpActiveWindow;
	ATUIWidget *mpModalWindow;
	void *mpModalCookie;
	bool mbCursorCaptured;
	bool mbCursorMotionMode;
	uint32 mCursorImageId;

	bool mbForeground;
	bool mbInvalidated;

	float mThemeScale;

	struct ModalEntry {
		ATUIWidget *mpPreviousModal;
		void *mpPreviousModalCookie;
	};

	typedef vdfastvector<ModalEntry> ModalStack;
	ModalStack mModalStack;

	typedef vdfastvector<ATUIWidget *> DestroyList;
	DestroyList mDestroyList;
	int mDestroyLocks;

	uint32 mNextInstanceId;

	typedef vdhashmap<uint32, ATUIWidget *> InstanceMap;
	InstanceMap mInstanceMap;

	typedef vdhashmap<uint32, ActiveAction *> ActiveActionMap;
	ActiveActionMap mActiveActionMap;

	struct PointerInfo {
		ATUIWidget *mpTargetWindow;
		uint32 mId;
	};

	PointerInfo mPointers[7];

	vdfastvector<ATUIWidget *> mTrackingWindows;

	ATUISystemMetrics mSystemMetrics;

	IVDDisplayFont *mpThemeFonts[kATUIThemeFontCount];

	ATUIStockImage *mpStockImages[kATUIStockImageIdxCount];
};

#endif
