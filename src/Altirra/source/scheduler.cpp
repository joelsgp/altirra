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
#include "scheduler.h"

class ATEvent : public ATEventLink {
public:
	IATSchedulerCallback *mpCB;
	uint32 mId;
	sint32 mNextTime;
};

ATScheduler::ATScheduler()
	: mNextEventCounter(-1000)
	, mTimeBase(1000)
	, mpFreeEvents(NULL)
{
	mActiveEvents.mpNext = mActiveEvents.mpPrev = &mActiveEvents;
}

ATScheduler::~ATScheduler() {
	ATEventLink *ev = mpFreeEvents;
	while(ev) {
		ATEventLink *next = ev->mpNext;
		delete static_cast<ATEvent *>(ev);
		ev = next;
	}
	mpFreeEvents = NULL;

	ev = mActiveEvents.mpNext;
	while(ev != &mActiveEvents) {
		ATEventLink *next = ev->mpNext;
		delete static_cast<ATEvent *>(ev);
		ev = next;
	}

	mActiveEvents.mpPrev = mActiveEvents.mpNext = &mActiveEvents;
}

void ATScheduler::ProcessNextEvent() {

	sint32 timeToNext = 100000;
	while(mActiveEvents.mpNext != &mActiveEvents) {
		ATEvent *ev = static_cast<ATEvent *>(mActiveEvents.mpNext);
		sint32 timeToNextEvent = ev->mNextTime - (mTimeBase + mNextEventCounter);

		VDASSERT(timeToNextEvent<10000000);

		if (timeToNextEvent) {
			if (timeToNext > timeToNextEvent)
				timeToNext = timeToNextEvent;
			break;
		}

		IATSchedulerCallback *cb = ev->mpCB;
		uint32 id = ev->mId;
		ev->mId = 0;

		VDASSERT(id);

		mActiveEvents.mpNext = ev->mpNext;
		mActiveEvents.mpNext->mpPrev = &mActiveEvents;

		ev->mpNext = mpFreeEvents;
		mpFreeEvents = ev;

		cb->OnScheduledEvent(id);
	}

	VDASSERT((uint32)(timeToNext - 1) < 100000);
	mTimeBase += mNextEventCounter;
	mNextEventCounter = -timeToNext;
	mTimeBase -= mNextEventCounter;
}

void ATScheduler::SetEvent(uint32 ticks, IATSchedulerCallback *cb, uint32 id, ATEvent *&ptr) {
	if (ptr)
		RemoveEvent(ptr);

	ptr = AddEvent(ticks, cb, id);
}

ATEvent *ATScheduler::AddEvent(uint32 ticks, IATSchedulerCallback *cb, uint32 id) {
	VDASSERT(ticks > 0 && ticks < 100000000);
	VDASSERT(id);

	ATEvent *ev;
	if (mpFreeEvents) {
		ev = static_cast<ATEvent *>(mpFreeEvents);
		mpFreeEvents = ev->mpNext;
	} else {
		ev = new ATEvent;
		ev->mId = 0;
	}

	VDASSERT(!ev->mId);

	ev->mpCB = cb;
	ev->mId = id;
	ev->mNextTime = mTimeBase + mNextEventCounter + ticks;

	ATEventLink *it = mActiveEvents.mpNext;
	for(; it != &mActiveEvents; it = it->mpNext) {
		ATEvent *ev2 = static_cast<ATEvent *>(it);

		if ((ev->mNextTime - mTimeBase) < (ev2->mNextTime - mTimeBase))
			break;
	}

	if (it == mActiveEvents.mpNext) {
		mTimeBase += mNextEventCounter;
		mNextEventCounter = -(sint32)ticks;
		mTimeBase -= mNextEventCounter;
		VDASSERT((uint32)-mNextEventCounter < 100000000);
	}

	ATEventLink *t = it->mpPrev;
	t->mpNext = ev;
	ev->mpPrev = t;
	it->mpPrev = ev;
	ev->mpNext = it;

	return ev;
}

void ATScheduler::RemoveEvent(ATEvent *p) {
	bool wasFront = (mActiveEvents.mpNext == p);

	VDASSERT(p->mId);

	// unlink from active events
	ATEventLink *prev = p->mpPrev;
	ATEventLink *next = p->mpNext;
	prev->mpNext = next;
	next->mpPrev = prev;

	p->mId = 0;

	// free event
	p->mpNext = mpFreeEvents;
	mpFreeEvents = p;

	if (wasFront)
		ProcessNextEvent();
}

int ATScheduler::GetTicksToEvent(ATEvent *ev) const {
	return ev->mNextTime - (mTimeBase + mNextEventCounter);
}
