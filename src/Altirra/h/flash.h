//	Altirra - Atari 800/800XL/5200 emulator
//	Copyright (C) 2008-2011 Avery Lee
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

#ifndef f_AT_FLASH_H
#define f_AT_FLASH_H

#include "scheduler.h"

enum ATFlashType {
	kATFlashType_Am29F010,	// AMD 128K x 8-bit
	kATFlashType_Am29F040B,	// AMD 512K x 8-bit
	kATFlashType_AT29C010A,	// Atmel 128K x 8-bit
	kATFlashType_AT29C040	// Atmel 512K x 8-bit
};

class ATFlashEmulator : public IATSchedulerCallback {
public:
	ATFlashEmulator();
	~ATFlashEmulator();

	void Init(void *mem, ATFlashType type, ATScheduler *scheduler);
	void Shutdown();

	void ColdReset();

	bool IsDirty() const { return mbDirty; }
	void SetDirty(bool dirty) { mbDirty = dirty; }

	bool CheckForWriteActivity() {
		if (!mbWriteActivity)
			return false;

		mbWriteActivity = false;
		return true;
	}

	bool IsControlReadEnabled() const { return mReadMode != kReadMode_Normal; }

	bool ReadByte(uint32 address, uint8& value);
	bool WriteByte(uint32 address, uint8 value);

protected:
	virtual void OnScheduledEvent(uint32 id);

	uint8 *mpMemory;
	ATScheduler *mpScheduler;
	ATFlashType mFlashType;

	enum ReadMode {
		kReadMode_Normal,
		kReadMode_Autoselect,
		kReadMode_WriteStatusPending
	};

	ReadMode mReadMode;
	int mCommandPhase;
	bool mbDirty;
	bool mbWriteActivity;
	bool mbAtmelSDP;
	uint32	mWriteSector;
	ATEvent *mpWriteEvent;
};

#endif	// f_AT_FLASH_H