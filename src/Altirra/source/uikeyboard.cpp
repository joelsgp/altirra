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

#include "stdafx.h"
#include <windows.h>

struct ATUICookedKeyMap {
	uint8	mScanCode[256];
};

struct ATUIDefaultCookedKeyMap : public ATUICookedKeyMap {
public:
	ATUIDefaultCookedKeyMap();
};

ATUIDefaultCookedKeyMap::ATUIDefaultCookedKeyMap() {
	mScanCode[(uint8)'l'] = 0x80;	// really 0x00
	mScanCode[(uint8)'L'] = 0x40;

	mScanCode[(uint8)'j'] = 0x01;
	mScanCode[(uint8)'J'] = 0x41;

	mScanCode[(uint8)';'] = 0x02;
	mScanCode[(uint8)':'] = 0x42;

	mScanCode[(uint8)'k'] = 0x05;
	mScanCode[(uint8)'K'] = 0x45;

	mScanCode[(uint8)'+'] = 0x06;
	mScanCode[(uint8)'\\']= 0x46;

	mScanCode[(uint8)'*'] = 0x07;
	mScanCode[(uint8)'^'] = 0x47;

	mScanCode[(uint8)'o'] = 0x08;
	mScanCode[(uint8)'O'] = 0x48;

	mScanCode[(uint8)'p'] = 0x0A;
	mScanCode[(uint8)'P'] = 0x4A;

	mScanCode[(uint8)'u'] = 0x0B;
	mScanCode[(uint8)'U'] = 0x4B;

	mScanCode[(uint8)'i'] = 0x0D;
	mScanCode[(uint8)'I'] = 0x4D;

	mScanCode[(uint8)'-'] = 0x0E;
	mScanCode[(uint8)'_'] = 0x4E;

	mScanCode[(uint8)'='] = 0x0F;
	mScanCode[(uint8)'|'] = 0x4F;

	mScanCode[(uint8)'v'] = 0x10;
	mScanCode[(uint8)'V'] = 0x50;

	mScanCode[(uint8)'c'] = 0x12;
	mScanCode[(uint8)'C'] = 0x52;

	mScanCode[(uint8)'b'] = 0x15;
	mScanCode[(uint8)'B'] = 0x55;

	mScanCode[(uint8)'x'] = 0x16;
	mScanCode[(uint8)'X'] = 0x56;

	mScanCode[(uint8)'z'] = 0x17;
	mScanCode[(uint8)'Z'] = 0x57;

	mScanCode[(uint8)'4'] = 0x18;
	mScanCode[(uint8)'$'] = 0x58;

	mScanCode[(uint8)'3'] = 0x1A;
	mScanCode[(uint8)'#'] = 0x5A;

	mScanCode[(uint8)'6'] = 0x1B;
	mScanCode[(uint8)'&'] = 0x5B;

	mScanCode[(uint8)'5'] = 0x1D;
	mScanCode[(uint8)'%'] = 0x5D;

	mScanCode[(uint8)'2'] = 0x1E;
	mScanCode[(uint8)'"'] = 0x5E;

	mScanCode[(uint8)'1'] = 0x1F;
	mScanCode[(uint8)'!'] = 0x5F;

	mScanCode[(uint8)','] = 0x20;
	mScanCode[(uint8)'['] = 0x60;

	mScanCode[(uint8)' '] = 0x21;

	mScanCode[(uint8)'.'] = 0x22;
	mScanCode[(uint8)']'] = 0x62;
	mScanCode[(uint8)'`'] = 0xa2;

	mScanCode[(uint8)'n'] = 0x23;
	mScanCode[(uint8)'N'] = 0x63;

	mScanCode[(uint8)'m'] = 0x25;
	mScanCode[(uint8)'M'] = 0x65;

	mScanCode[(uint8)'/'] = 0x26;
	mScanCode[(uint8)'?'] = 0x66;

	mScanCode[(uint8)'r'] = 0x28;
	mScanCode[(uint8)'R'] = 0x68;

	mScanCode[(uint8)'e'] = 0x2A;
	mScanCode[(uint8)'E'] = 0x6A;

	mScanCode[(uint8)'y'] = 0x2B;
	mScanCode[(uint8)'Y'] = 0x6B;

	mScanCode[(uint8)'t'] = 0x2D;
	mScanCode[(uint8)'T'] = 0x6D;

	mScanCode[(uint8)'w'] = 0x2E;
	mScanCode[(uint8)'W'] = 0x6E;

	mScanCode[(uint8)'q'] = 0x2F;
	mScanCode[(uint8)'Q'] = 0x6F;

	mScanCode[(uint8)'9'] = 0x30;
	mScanCode[(uint8)'('] = 0x70;

	mScanCode[(uint8)'0'] = 0x32;
	mScanCode[(uint8)')'] = 0x72;

	mScanCode[(uint8)'7'] = 0x33;
	mScanCode[(uint8)'\'']= 0x73;

	mScanCode[(uint8)'8'] = 0x35;
	mScanCode[(uint8)'@'] = 0x75;

	mScanCode[(uint8)'<'] = 0x36;
	mScanCode[(uint8)'>'] = 0x37;

	mScanCode[(uint8)'f'] = 0x38;
	mScanCode[(uint8)'F'] = 0x78;

	mScanCode[(uint8)'h'] = 0x39;
	mScanCode[(uint8)'H'] = 0x79;

	mScanCode[(uint8)'d'] = 0x3A;
	mScanCode[(uint8)'D'] = 0x7A;

	mScanCode[(uint8)'g'] = 0x3D;
	mScanCode[(uint8)'G'] = 0x7D;

	mScanCode[(uint8)'s'] = 0x3E;
	mScanCode[(uint8)'S'] = 0x7E;

	mScanCode[(uint8)'a'] = 0x3F;
	mScanCode[(uint8)'A'] = 0x7F;
}

ATUIDefaultCookedKeyMap g_ATCookedKeyMap;

bool ATUIGetScanCodeForCharacter(char c, uint8& ch) {
	uint8 scanCode = g_ATCookedKeyMap.mScanCode[(uint8)c];

	if (!scanCode)
		return false;

	ch = scanCode & 0x7F;
	return true;
}

///////////////////////////////////////////////////////////////////////////////

namespace {
	enum {
		kShift = 0x10000,
		kCtrl = 0x20000,
		kAlt = 0x40000
	};
}

static uint32 g_ATVKeyMap[]={

#define VKEYMAP(vkey, mods, sc) { ((vkey) << 8) + (mods) + (sc) }
#define VKEYMAP_CSALL(vkey, sc) \
	VKEYMAP((vkey), 0, (sc)),	\
	VKEYMAP((vkey), kShift, (sc) + 0x40),	\
	VKEYMAP((vkey), kCtrl, (sc) + 0x80),	\
	VKEYMAP((vkey), kCtrl + kShift, (sc) + 0xC0)

#define VKEYMAP_C_SALL(vkey, sc) \
	VKEYMAP((vkey), kCtrl, (sc) + 0x80),	\
	VKEYMAP((vkey), kCtrl + kShift, (sc) + 0xC0)

	VKEYMAP(VK_UP, 0,						0x8E),
	VKEYMAP(VK_UP, kCtrl,					0x0E),
	VKEYMAP(VK_UP, kAlt,					0x8E),
	VKEYMAP(VK_UP, kAlt + kCtrl,			0x0E),
	VKEYMAP(VK_UP, kShift,					0xCE),
	VKEYMAP(VK_UP, kCtrl + kShift,			0x4E),
	VKEYMAP(VK_UP, kAlt + kShift,			0xCE),
	VKEYMAP(VK_UP, kAlt + kCtrl + kShift,	0x4E),

	VKEYMAP(VK_DOWN, 0,						0x8F),
	VKEYMAP(VK_DOWN, kCtrl,					0x0F),
	VKEYMAP(VK_DOWN, kAlt,					0x8F),
	VKEYMAP(VK_DOWN, kAlt + kCtrl,			0x0F),
	VKEYMAP(VK_DOWN, kShift,				0xCF),
	VKEYMAP(VK_DOWN, kCtrl + kShift,		0x4F),
	VKEYMAP(VK_DOWN, kAlt + kShift,			0xCF),
	VKEYMAP(VK_DOWN, kAlt + kCtrl + kShift,	0x4F),

	VKEYMAP(VK_LEFT, 0,						0x86),
	VKEYMAP(VK_LEFT, kCtrl,					0x06),
	VKEYMAP(VK_LEFT, kAlt,					0x86),
	VKEYMAP(VK_LEFT, kAlt + kCtrl,			0x06),
	VKEYMAP(VK_LEFT, kShift,				0xC6),
	VKEYMAP(VK_LEFT, kCtrl + kShift,		0x46),
	VKEYMAP(VK_LEFT, kAlt + kShift,			0xC6),
	VKEYMAP(VK_LEFT, kAlt + kCtrl + kShift,	0x46),

	VKEYMAP(VK_RIGHT, 0,					0x87),
	VKEYMAP(VK_RIGHT, kCtrl,				0x07),
	VKEYMAP(VK_RIGHT, kAlt,					0x87),
	VKEYMAP(VK_RIGHT, kAlt + kCtrl,			0x07),
	VKEYMAP(VK_RIGHT, kShift,				0xC7),
	VKEYMAP(VK_RIGHT, kCtrl + kShift,		0x47),
	VKEYMAP(VK_RIGHT, kAlt + kShift,		0xC7),
	VKEYMAP(VK_RIGHT, kAlt + kCtrl + kShift,0x47),

	VKEYMAP_CSALL(VK_TAB,		0x2C),	// Tab
	VKEYMAP_CSALL(VK_BACK,		0x34),	// Backspace
	VKEYMAP_CSALL(VK_RETURN,	0x0C),	// Enter
	VKEYMAP_CSALL(VK_ESCAPE,	0x1C),	// Esc
	VKEYMAP_CSALL(VK_END,		0x27),	// Fuji
	VKEYMAP(VK_OEM_1, kCtrl,	0x82),	// ;:
	VKEYMAP(VK_OEM_1, kCtrl + kShift, 0xC2),	// ;:
	VKEYMAP(VK_OEM_PLUS, kCtrl,			0x86),	// +
	VKEYMAP(VK_OEM_PLUS, kCtrl + kShift,0x86),	// +
	VKEYMAP(VK_OEM_4, kCtrl,			0xE0),	// [{
	VKEYMAP(VK_OEM_4, kCtrl + kShift,	0xE0),	// [{
	VKEYMAP(VK_OEM_5, kCtrl,			0x9C),	// Ctrl+\| -> Ctrl+Esc
	VKEYMAP(VK_OEM_5, kCtrl + kShift,	0xDC),	// Ctrl+Shift+\| -> Ctrl+Shift+Esc
	VKEYMAP(VK_OEM_6, kCtrl,			0xE2),	// ]}
	VKEYMAP(VK_OEM_6, kCtrl + kShift,	0xE2),	// ]}
	VKEYMAP(VK_OEM_COMMA, kCtrl,		0xA0),	// Ctrl+,
	VKEYMAP(VK_OEM_PERIOD, kCtrl,		0xA2),	// Ctrl+,
	VKEYMAP(VK_OEM_2, kCtrl,			0xA6),	// Ctrl+/
	VKEYMAP(VK_OEM_2, kCtrl + kShift,	0xE6),	// Ctrl+?
	VKEYMAP(VK_HOME,	0,				0x76),	// Home -> Shift+< (Clear)
	VKEYMAP(VK_HOME,	kShift,			0x76),	// Shift+Home -> Shift+< (Clear)
	VKEYMAP(VK_HOME,	kCtrl,			0xB6),	// Ctrl+Home -> Shift+< (Clear)
	VKEYMAP(VK_HOME,	kCtrl + kShift,	0xF6),	// Ctrl+Shift+Home -> Shift+< (Clear)
	VKEYMAP(VK_DELETE,	0,				0xB4),	// Delete -> Ctrl+Backspace
	VKEYMAP(VK_DELETE,	kShift,			0x74),	// Shift+Delete -> Shift+Backspace
	VKEYMAP(VK_DELETE,	kCtrl,			0xF4),	// Shift+Delete -> Ctrl+Shift+Backspace
	VKEYMAP(VK_DELETE,	kCtrl + kShift,	0xF4),	// Ctrl+Shift+Delete -> Ctrl+Shift+Backspace
	VKEYMAP(VK_INSERT,	0,				0xB7),	// Insert -> Ctrl+>
	VKEYMAP(VK_INSERT,	kShift,			0x77),	// Shift+Insert -> Shift+> (Insert)
	VKEYMAP(VK_INSERT,	kCtrl,			0xF7),	// Shift+Insert -> Ctrl+Shift+>
	VKEYMAP(VK_INSERT,	kCtrl + kShift,	0xF7),	// Ctrl+Shift+Insert -> Ctrl+Shift+>
	VKEYMAP(VK_SPACE,	kShift,			0x61),	// Shift+Space
	VKEYMAP(VK_SPACE,	kCtrl,			0xA1),	// Ctrl+Space
	VKEYMAP(VK_SPACE,	kCtrl + kShift,	0xE1),	// Ctrl+Shift+Space

	VKEYMAP_C_SALL('A', 0x3F),
	VKEYMAP_C_SALL('B', 0x15),
	VKEYMAP_C_SALL('C', 0x12),
	VKEYMAP_C_SALL('D', 0x3A),
	VKEYMAP_C_SALL('E', 0x2A),
	VKEYMAP_C_SALL('F', 0x38),
	VKEYMAP_C_SALL('G', 0x3D),
	VKEYMAP_C_SALL('H', 0x39),
	VKEYMAP_C_SALL('I', 0x0D),
	VKEYMAP_C_SALL('J', 0x01),
	VKEYMAP_C_SALL('K', 0x05),
	VKEYMAP_C_SALL('L', 0x00),
	VKEYMAP_C_SALL('M', 0x25),
	VKEYMAP_C_SALL('N', 0x23),
	VKEYMAP_C_SALL('O', 0x08),
	VKEYMAP_C_SALL('P', 0x0A),
	VKEYMAP_C_SALL('Q', 0x2F),
	VKEYMAP_C_SALL('R', 0x28),
	VKEYMAP_C_SALL('S', 0x3E),
	VKEYMAP_C_SALL('T', 0x2D),
	VKEYMAP_C_SALL('U', 0x0B),	
	VKEYMAP_C_SALL('V', 0x10),
	VKEYMAP_C_SALL('W', 0x2E),
	VKEYMAP_C_SALL('X', 0x16),
	VKEYMAP_C_SALL('Y', 0x2B),
	VKEYMAP_C_SALL('Z', 0x17),
	VKEYMAP_C_SALL('0', 0x32),
	VKEYMAP_C_SALL('1', 0x1F),
	VKEYMAP_C_SALL('2', 0x1E),
	VKEYMAP_C_SALL('3', 0x1A),
	VKEYMAP_C_SALL('4', 0x18),
	VKEYMAP_C_SALL('5', 0x1D),
	VKEYMAP_C_SALL('6', 0x1B),
	VKEYMAP_C_SALL('7', 0x33),
	VKEYMAP_C_SALL('8', 0x35),
	VKEYMAP_C_SALL('9', 0x30),
};

void ATUIInitVirtualKeyMap() {
	std::sort(g_ATVKeyMap, g_ATVKeyMap + sizeof(g_ATVKeyMap)/sizeof(g_ATVKeyMap[0]));
}

bool ATUIGetScanCodeForVirtualKey(uint32 virtKey, bool alt, bool ctrl, bool shift, uint8& scanCode) {
	const uint32 *begin = g_ATVKeyMap;
	const uint32 *end = g_ATVKeyMap + sizeof(g_ATVKeyMap)/sizeof(g_ATVKeyMap[0]);

	uint32 vkcode = virtKey << 8;

	if (alt)
		vkcode += kAlt;

	if (ctrl)
		vkcode += kCtrl;

	if (shift)
		vkcode += kShift;

	const uint32 *it = std::lower_bound(begin, end, vkcode);

	if (it == end || ((*it) & 0xffffff00) != vkcode)
		return false;

	scanCode = (uint8)*it;
	return true;
}
