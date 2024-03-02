#ifndef f_AT_UIBUTTON_H
#define f_AT_UIBUTTON_H

#include <vd2/system/VDString.h>
#include <vd2/VDDisplay/font.h>
#include "uiwidget.h"
#include "callback.h"

class ATUIButton : public ATUIWidget {
public:
	enum {
		kActionActivate = kActionCustom
	};

	ATUIButton();
	~ATUIButton();

	void SetStockImage(sint32 id);
	void SetText(const wchar_t *s);
	void SetDepressed(bool depressed);

	ATCallbackHandler1<void, ATUIButton *>& OnPressedEvent() { return mPressedEvent; }
	ATCallbackHandler1<void, ATUIButton *>& OnActivatedEvent() { return mActivatedEvent; }

public:
	virtual void OnMouseDownL(sint32 x, sint32 y);
	virtual void OnMouseUpL(sint32 x, sint32 y);

	virtual void OnActionStart(uint32 id);
	virtual void OnActionStop(uint32 id);

	virtual void OnCreate();
	virtual void OnSize();

	virtual void OnSetFocus();
	virtual void OnKillFocus();

protected:
	void Paint(IVDDisplayRenderer& rdr, sint32 w, sint32 h);
	void Relayout();

	sint32 mStockImageIdx;
	bool mbDepressed;
	sint32 mTextX;
	sint32 mTextY;
	VDStringW mText;
	vdrefptr<IVDDisplayFont> mpFont;

	ATCallbackHandler1<void, ATUIButton *> mActivatedEvent;
	ATCallbackHandler1<void, ATUIButton *> mPressedEvent;
};

#endif
