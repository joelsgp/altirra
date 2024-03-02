//	Altirra - Atari 800/800XL/5200 emulator
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

#include "stdafx.h"
#include <vd2/system/binary.h>
#include <vd2/system/vdalloc.h>
#include <vd2/system/vdstl.h>
#include "cpu.h"
#include "cpumemory.h"
#include "kerneldb.h"
#include "cio.h"
#include "virtualscreen.h"

class ATVirtualScreenHandler : public IATVirtualScreenHandler {
public:
	ATVirtualScreenHandler();
	~ATVirtualScreenHandler();

	void GetScreen(uint32& width, uint32& height, const uint8 *&screen) const;
	bool GetCursorInfo(uint32& x, uint32& y) const;

	void Resize(uint32 w, uint32 h);
	void PushLine(const char *line);

	bool IsRawInputActive() const { return !mbWaitingForInput; }
	void SetShiftControlLockState(bool shift, bool ctrl);
	bool GetShiftLockState() const;
	bool GetControlLockState() const;

	bool CheckForBell() {
		bool pending = mbBellPending;
		mbBellPending = false;
		return pending;
	}

	void ColdReset();
	void WarmReset();

	void SetHookPage(uint8 hookPage);
	void SetGetCharAddress(uint16 addr);
	void OnCIOVector(ATCPUEmulator *cpu, ATCPUEmulatorMemory *mem, int offset);

protected:
	void PutChar(uint8 c);
	void PutRawChar(uint8 c);
	void ClearScreen();

	uint16	mGetCharAddress;
	uint8	mHookPage;
	bool	mbWaitingForInput;

	uint8	mShiftCtrlLockState;
	bool	mbWriteShiftCtrlLockState;
	bool	mbEscapeNextChar;
	bool	mbBellPending;
	bool	mbForcedInputMode;

	uint32	mWidth;
	uint32	mHeight;
	uint32	mX;
	uint32	mY;
	uint32	mInputIndex;

	vdfastvector<uint8> mScreen;
	vdfastvector<uint8> mActiveInputLine;
};

ATVirtualScreenHandler::ATVirtualScreenHandler()
	: mGetCharAddress(0)
	, mHookPage(0)
	, mbWaitingForInput(false)
	, mShiftCtrlLockState(0x40)
	, mbWriteShiftCtrlLockState(false)
	, mbEscapeNextChar(false)
	, mbBellPending(false)
	, mWidth(40)
	, mHeight(24)
	, mX(0)
	, mY(0)
	, mInputIndex(0)
{
	mScreen.resize(mWidth * mHeight, 0x20);
}

ATVirtualScreenHandler::~ATVirtualScreenHandler() {
}

void ATVirtualScreenHandler::GetScreen(uint32& width, uint32& height, const uint8 *&screen) const {
	width = mWidth;
	height = mHeight;
	screen = mScreen.data();
}

bool ATVirtualScreenHandler::GetCursorInfo(uint32& x, uint32& y) const {
	x = mX;
	y = mY;
	return mbWaitingForInput;
}

void ATVirtualScreenHandler::Resize(uint32 w, uint32 h) {
	if (mWidth == w && mHeight == h)
		return;

	vdfastvector<uint8> newScreen(w * h, 0x20);

	uint32 commonW = std::min<uint32>(w, mWidth);
	uint32 commonH = std::min<uint32>(h, mHeight);

	for(uint32 y=0; y<commonH; ++y)
		memcpy(&newScreen[y * w], &mScreen[y * mWidth], commonW);
	
	mScreen.swap(newScreen);
	mWidth = w;
	mHeight = h;

	if (mX >= w)
		mX = w - 1;

	if (mY >= h)
		mY = h - 1;
}

void ATVirtualScreenHandler::PushLine(const char *line) {
	size_t len = strlen(line);
	mActiveInputLine.resize(len + 1);

	for(size_t i=0; i<len; ++i)
		mActiveInputLine[i] = (uint8)line[i];

	mActiveInputLine.back() = 0x9B;

	for(size_t i=0; i<len; ++i)
		PutRawChar(mActiveInputLine[i]);

	PutChar(0x9B);
}

void ATVirtualScreenHandler::SetShiftControlLockState(bool shift, bool ctrl) {
	mShiftCtrlLockState = (ctrl ? 0x80 : 0x00) + (shift ? 0x40 : 0x00);
	mbWriteShiftCtrlLockState = true;
}

bool ATVirtualScreenHandler::GetShiftLockState() const {
	return (mShiftCtrlLockState & 0x40) != 0;
}

bool ATVirtualScreenHandler::GetControlLockState() const {
	return (mShiftCtrlLockState & 0x80) != 0;
}

void ATVirtualScreenHandler::WarmReset() {
	ClearScreen();

	mbEscapeNextChar = false;
}

void ATVirtualScreenHandler::ColdReset() {
	ClearScreen();

	mbEscapeNextChar = false;
}

void ATVirtualScreenHandler::SetHookPage(uint8 hookPage) {
	mHookPage = hookPage;
}

void ATVirtualScreenHandler::SetGetCharAddress(uint16 addr) {
	mGetCharAddress = addr;
}

void ATVirtualScreenHandler::OnCIOVector(ATCPUEmulator *cpu, ATCPUEmulatorMemory *mem, int offset) {
	const uint8 iocb = cpu->GetX();
	ATKernelDatabase kdb(mem);

	switch(offset) {
		case 0:		// open
			mbForcedInputMode = (mem->ReadByte(ATKernelSymbols::ICAX1 + iocb) & 1) != 0;
			mbWaitingForInput = false;
			break;

		case 2:		// close
			break;

		case 4:		// get byte
			if (mbWriteShiftCtrlLockState) {
				mbWriteShiftCtrlLockState = false;
				kdb.SHFLOK = mShiftCtrlLockState;
			} else
				mShiftCtrlLockState = kdb.SHFLOK;

			while(mInputIndex >= mActiveInputLine.size()) {
				mInputIndex = 0;

				mActiveInputLine.clear();

				mbWaitingForInput = true;
				cpu->PushWord(mGetCharAddress - 1);
				cpu->PushWord(0xE4C0 - 1);
				cpu->PushWord(0xE4C0 - 1);
				return;
			}

			mbWaitingForInput = false;
			cpu->SetA(mActiveInputLine[mInputIndex++]);
			cpu->Ldy(ATCIOSymbols::CIOStatSuccess);
			break;

		case 6:		// put byte
			{
				uint8 c = cpu->GetA();

				if (mbEscapeNextChar) {
					mbEscapeNextChar = false;
					PutRawChar(c);
				} else if (c != 0x9B && kdb.DSPFLG)
					PutRawChar(c);
				else
					PutChar(c);

				cpu->Ldy(ATCIOSymbols::CIOStatSuccess);
				mbWaitingForInput = false;
			}
			break;

		case 8:		// get status
			cpu->Ldy(ATCIOSymbols::CIOStatSuccess);
			break;

		case 10:	// special
			cpu->Ldy(ATCIOSymbols::CIOStatSuccess);
			break;
	}
}

void ATVirtualScreenHandler::PutChar(uint8 c) {
	switch(c) {
		case 0x1B:		// Escape
			mbEscapeNextChar = true;
			break;

		case 0x1C:		// Cursor up
			if (mY)
				--mY;
			else
				mY = mHeight - 1;
			break;

		case 0x1D:		// Cursor down
			if (++mY >= mHeight)
				mY = 0;
			break;

		case 0x1E:		// Cursor left
			if (mX)
				--mX;
			else
				mX = mWidth - 1;
			break;

		case 0x1F:		// Cursor right
			if (++mX >= mWidth)
				mX = 0;
			break;

		case 0x7D:		// Clear
			ClearScreen();
			break;

		case 0x7E:		// Backspace
			mScreen[mX + mY*mWidth] = ' ';
			if (mX)
				--mX;
			break;

		case 0x9B:		// EOL
			mX = 0;
			if (++mY >= mHeight) {
				memmove(mScreen.data(), &mScreen[mWidth], mWidth * (mHeight - 1));
				memset(&mScreen[mWidth * (mHeight - 1)], ' ', mWidth);
				mY = mHeight - 1;
			}
			break;

		case 0x9C:		// Delete line
			if (mY + 1 < mHeight)
				memmove(&mScreen[mY * mWidth], &mScreen[mY * mWidth + mWidth], mWidth * (mHeight - mY - 1));

			memset(&mScreen[(mHeight - 1) * mWidth], ' ', mWidth);
			break;

		case 0x9D:		// Insert line
			if (mY + 1 < mHeight)
				memmove(&mScreen[mY * mWidth + mWidth], &mScreen[mY * mWidth], mWidth * (mHeight - mY - 1));

			memset(&mScreen[mY * mWidth], ' ', mWidth);
			break;

		case 0xFD:		// Bell
			mbBellPending = true;
			break;

		case 0xFE:		// Delete character
			if (mX + 1 < mWidth)
				memmove(&mScreen[mX + mY * mWidth], &mScreen[mX + mY * mWidth + 1], mWidth - mX - 1);

			mScreen[mWidth - 1 + mY * mWidth] = ' ';
			break;

		case 0xFF:		// Insert character
			if (mX + 1 < mWidth)
				memmove(&mScreen[mX + mY * mWidth + 1], &mScreen[mX + mY * mWidth], mWidth - mX - 1);

			mScreen[mX + mY * mWidth] = ' ';
			break;

		default:
			PutRawChar(c);
			break;
	}
}

void ATVirtualScreenHandler::PutRawChar(uint8 c) {
	mScreen[mX + mY*mWidth] = c;

	if (++mX >= mWidth) {
		mX = 0;

		if (++mY >= mHeight) {
			memmove(mScreen.data(), &mScreen[mWidth], mWidth * (mHeight - 1));
			memset(&mScreen[mWidth * (mHeight - 1)], ' ', mWidth);
			mY = mHeight - 1;
		}
	}
}

void ATVirtualScreenHandler::ClearScreen() {
	memset(mScreen.data(), 0, mScreen.size() * sizeof(mScreen[0]));
	mX = 0;
	mY = 0;
}

///////////////////////////////////////////////////////////////////////////

IATVirtualScreenHandler *ATCreateVirtualScreenHandler() {
	vdautoptr<ATVirtualScreenHandler> hook(new ATVirtualScreenHandler);

	return hook.release();
}