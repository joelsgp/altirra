//	Altirra - Atari 800/800XL/5200 emulator
//	Copyright (C) 2009-2010 Avery Lee
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

#ifndef f_AT_UIENHANCEDTEXT_H
#define f_AT_UIENHANCEDTEXT_H

class ATSimulator;

class IATUIEnhancedTextEngine {
public:
	virtual ~IATUIEnhancedTextEngine() {}

	virtual void Init(HWND hwnd, ATSimulator *sim) = 0;
	virtual void Shutdown() = 0;

	virtual bool IsRawInputEnabled() const = 0;

	virtual void SetFont(const LOGFONTW *font) = 0;

	virtual void OnSize(uint32 w, uint32 h) = 0;
	virtual void OnChar(int ch) = 0;
	virtual bool OnKeyDown(uint32 keyCode, uint32 modifiers) = 0;
	virtual bool OnKeyUp(uint32 keyCode, uint32 modifiers) = 0;

	virtual void Update(bool forceInvalidate) = 0;
	virtual void Paint(HDC hdc) = 0;
};

IATUIEnhancedTextEngine *ATUICreateEnhancedTextEngine();

#endif	// f_AT_UIENHANCEDTEXT_H
