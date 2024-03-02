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
#include "Dialog.h"
#include "resource.h"
#include "cartridge.h"

class ATUIDialogCartridgeMapper : public VDDialogFrameW32 {
public:
	ATUIDialogCartridgeMapper(uint32 cartSize, bool show2600Warning);

	int GetMapper() const { return mMapper; }

protected:
	bool OnLoaded();
	void OnDataExchange(bool write);

	static uint32 GetModeSize(int mode);
	static const wchar_t *GetModeName(int mode);

	int mMapper;
	uint32 mCartSize;
	bool mbShow2600Warning;
	typedef vdfastvector<int> Mappers;
	Mappers mMappers;

	struct MapSorter {
		bool operator()(int x, int y) const {
			// The terminology is a bit screwed here -- need to clean up mapper vs. mode.
			int xm = ATGetCartridgeMapperForMode(x);
			int ym = ATGetCartridgeMapperForMode(y);

			if (!xm)
				xm = 1000;

			if (!ym)
				ym = 1000;

			if (xm < ym)
				return true;

			if (xm == ym && xm == 1000 && wcscmp(GetModeName(x), GetModeName(y)) < 0)
				return true;

			return false;
		}
	};
};

ATUIDialogCartridgeMapper::ATUIDialogCartridgeMapper(uint32 cartSize, bool show2600Warning)
	: VDDialogFrameW32(IDD_CARTRIDGE_MAPPER)
	, mMapper(0)
	, mCartSize(cartSize)
	, mbShow2600Warning(show2600Warning)
{
}

bool ATUIDialogCartridgeMapper::OnLoaded() {
	for(int i=1; i<kATCartridgeModeCount; ++i) {
		bool valid = false;

		if (GetModeSize(i) == mCartSize) {
			// size matches
			valid = true;
		} else if (i == kATCartridgeMode_TelelinkII && mCartSize == 8192 + 256) {
			// We allow either 8K or 8K + 256 for the Telelink II cart.
			valid = true;
		} else if (i == kATCartridgeMode_8K && (mCartSize == 2048 || mCartSize == 4096)) {
			// We allow 2K and 4K for the regular 8K option.
			valid = true;
		} else if (i == kATCartridgeMode_SIC && (mCartSize == 0x20000 || mCartSize == 0x40000)) {
			// We allow 128K and 256K for SIC!.
			valid = true;
		} else if (i == kATCartridgeMode_MaxFlash_1024K_Bank0 && (mCartSize == 0x40000 || mCartSize == 0x80000)) {
			// We allow 256K and 512K for the new 8Mbit MaxFlash carts.
			valid = true;
		} else if (i == kATCartridgeMode_Megacart_1M_2 && (mCartSize == 0x40000 || mCartSize == 0x80000)) {
			// We allow 256K and 512K for the 1M Megacart.
			valid = true;
		}

		if (valid)
			mMappers.push_back(i);
	}

	if (mMappers.empty()) {
		EnableControl(IDC_LIST, false);
		LBAddString(IDC_LIST, L"No compatible mappers found.");
		EnableControl(1 /* IDOK */, false);
	} else {
		std::sort(mMappers.begin(), mMappers.end(), MapSorter());

		for(Mappers::const_iterator it(mMappers.begin()), itEnd(mMappers.end()); it != itEnd; ++it) {
			LBAddString(IDC_LIST, GetModeName(*it));
		}

		LBSetSelectedIndex(IDC_LIST, 0);
	}

	OnDataExchange(false);
	OnDataExchange(true);

	if (!mMappers.empty())
		SetFocusToControl(IDC_LIST);
	return true;
}

void ATUIDialogCartridgeMapper::OnDataExchange(bool write) {
	if (write) {
		int idx = LBGetSelectedIndex(IDC_LIST);

		if (idx < 0 || (uint32)idx >= mMappers.size()) {
			FailValidation(IDC_LIST);
			return;
		}

		mMapper = mMappers[idx];
	} else {
		ShowControl(IDC_STATIC_2600WARNING, mbShow2600Warning);
	}
}

uint32 ATUIDialogCartridgeMapper::GetModeSize(int mode) {
	switch(mode) {
		case kATCartridgeMode_8K:					return 8192;
		case kATCartridgeMode_16K:					return 16384;
		case kATCartridgeMode_OSS_034M:				return 16384;
		case kATCartridgeMode_XEGS_32K:				return 32768;
		case kATCartridgeMode_XEGS_64K:				return 65536;
		case kATCartridgeMode_XEGS_128K:			return 131072;
		case kATCartridgeMode_XEGS_256K:			return 262144;
		case kATCartridgeMode_XEGS_512K:			return 524288;
		case kATCartridgeMode_XEGS_1M:				return 1048576;
		case kATCartridgeMode_OSS_M091:				return 16384;
		case kATCartridgeMode_BountyBob800:			return 40960;
		case kATCartridgeMode_BountyBob5200:		return 40960;
		case kATCartridgeMode_MegaCart_16K:			return 16384;
		case kATCartridgeMode_MegaCart_32K:			return 32768;
		case kATCartridgeMode_MegaCart_64K:			return 65536;
		case kATCartridgeMode_MegaCart_128K:		return 131072;
		case kATCartridgeMode_MegaCart_256K:		return 262144;
		case kATCartridgeMode_MegaCart_512K:		return 524288;
		case kATCartridgeMode_MegaCart_1M:			return 1048576;
		case kATCartridgeMode_Switchable_XEGS_32K:	return 32768;
		case kATCartridgeMode_Switchable_XEGS_64K:	return 65536;
		case kATCartridgeMode_Switchable_XEGS_128K:	return 131072;
		case kATCartridgeMode_Switchable_XEGS_256K:	return 262144;
		case kATCartridgeMode_Switchable_XEGS_512K:	return 524288;
		case kATCartridgeMode_Switchable_XEGS_1M:	return 1048576;
		case kATCartridgeMode_MaxFlash_128K:		return 131072;
		case kATCartridgeMode_MaxFlash_128K_MyIDE:	return 131072;
		case kATCartridgeMode_MaxFlash_1024K:		return 1048576;
		case kATCartridgeMode_MaxFlash_1024K_Bank0:	return 1048576;
		case kATCartridgeMode_5200_32K:				return 32768;
		case kATCartridgeMode_5200_16K_TwoChip:		return 16384;
		case kATCartridgeMode_5200_16K_OneChip:		return 16384;
		case kATCartridgeMode_5200_8K:				return 8192;
		case kATCartridgeMode_5200_4K:				return 4096;
		case kATCartridgeMode_Corina_1M_EEPROM:		return 1048576 + 8192;
		case kATCartridgeMode_Corina_512K_SRAM_EEPROM:	return 524288 + 8192;
		case kATCartridgeMode_SpartaDosX_128K:		return 131072;
		case kATCartridgeMode_TelelinkII:			return 8192;
		case kATCartridgeMode_Williams_64K:			return 65536;
		case kATCartridgeMode_Diamond_64K:			return 65536;
		case kATCartridgeMode_Express_64K:			return 65536;
		case kATCartridgeMode_SpartaDosX_64K:		return 65536;
		case kATCartridgeMode_RightSlot_8K:			return 8192;
		case kATCartridgeMode_DB_32K:				return 32768;
		case kATCartridgeMode_Atrax_128K:			return 131072;
		case kATCartridgeMode_Williams_32K:			return 32768;
		case kATCartridgeMode_Phoenix_8K:			return 8192;
		case kATCartridgeMode_Blizzard_4K:			return 4096;
		case kATCartridgeMode_Blizzard_16K:			return 16384;
		case kATCartridgeMode_SIC:					return 524288;
		case kATCartridgeMode_Atrax_SDX_64K:		return 65536;
		case kATCartridgeMode_Atrax_SDX_128K:		return 131072;
		case kATCartridgeMode_OSS_043M:				return 16384;
		case kATCartridgeMode_OSS_8K:				return 8192;
		case kATCartridgeMode_AST_32K:				return 32768;
		case kATCartridgeMode_Turbosoft_64K:		return 65536;
		case kATCartridgeMode_Turbosoft_128K:		return 131072;
		case kATCartridgeMode_Megacart_1M_2:		return 1048576;
		case kATCartridgeMode_5200_64K_32KBanks:	return 65536;
		default:
			return 0;
	}
}

const wchar_t *ATUIDialogCartridgeMapper::GetModeName(int mode) {
	switch(mode) {
		case kATCartridgeMode_8K:					return L"1: 8K";
		case kATCartridgeMode_16K:					return L"2: 16K";
		case kATCartridgeMode_OSS_034M:				return L"3: OSS '034M'";
		case kATCartridgeMode_5200_32K:				return L"4: 5200 32K";
		case kATCartridgeMode_DB_32K:				return L"5: DB 32K";
		case kATCartridgeMode_5200_16K_TwoChip:		return L"6: 5200 16K (two chip)";
		case kATCartridgeMode_BountyBob5200:		return L"7: Bounty Bob (5200)";
		case kATCartridgeMode_Williams_64K:			return L"8: Williams 64K";
		case kATCartridgeMode_Express_64K:			return L"9: Express 64K";
		case kATCartridgeMode_Diamond_64K:			return L"10: Diamond 64K";
		case kATCartridgeMode_SpartaDosX_64K:		return L"11: SpartaDOS X 64K";
		case kATCartridgeMode_XEGS_32K:				return L"12: 32K XEGS";
		case kATCartridgeMode_XEGS_64K:				return L"13: 64K XEGS";
		case kATCartridgeMode_XEGS_128K:			return L"14: 128K XEGS";
		case kATCartridgeMode_OSS_M091:				return L"15: OSS 'M091'";
		case kATCartridgeMode_5200_16K_OneChip:		return L"16: 5200 16K (one chip)";
		case kATCartridgeMode_Atrax_128K:			return L"17: Atrax 128K";
		case kATCartridgeMode_BountyBob800:			return L"18: Bounty Bob (800)";
		case kATCartridgeMode_5200_8K:				return L"19: 5200 8K";
		case kATCartridgeMode_5200_4K:				return L"20: 5200 4K";
		case kATCartridgeMode_RightSlot_8K:			return L"21: Right slot 8K";
		case kATCartridgeMode_Williams_32K:			return L"22: Williams 32K";
		case kATCartridgeMode_XEGS_256K:			return L"23: 256K XEGS";
		case kATCartridgeMode_XEGS_512K:			return L"24: 512K XEGS";
		case kATCartridgeMode_XEGS_1M:				return L"25: 1M XEGS";
		case kATCartridgeMode_MegaCart_16K:			return L"26: 16K MegaCart";
		case kATCartridgeMode_MegaCart_32K:			return L"27: 32K MegaCart";
		case kATCartridgeMode_MegaCart_64K:			return L"28: 64K MegaCart";
		case kATCartridgeMode_MegaCart_128K:		return L"29: 128K MegaCart";
		case kATCartridgeMode_MegaCart_256K:		return L"30: 256K MegaCart";
		case kATCartridgeMode_MegaCart_512K:		return L"31: 512K MegaCart";
		case kATCartridgeMode_MegaCart_1M:			return L"32: 1M MegaCart";
		case kATCartridgeMode_Switchable_XEGS_32K:	return L"33: 32K Switchable XEGS";
		case kATCartridgeMode_Switchable_XEGS_64K:	return L"34: 64K Switchable XEGS";
		case kATCartridgeMode_Switchable_XEGS_128K:	return L"35: 128K Switchable XEGS";
		case kATCartridgeMode_Switchable_XEGS_256K:	return L"36: 256K Switchable XEGS";
		case kATCartridgeMode_Switchable_XEGS_512K:	return L"37: 512K Switchable XEGS";
		case kATCartridgeMode_Switchable_XEGS_1M:	return L"38: 1M Switchable XEGS";
		case kATCartridgeMode_Phoenix_8K:			return L"39: Phoenix 8K";
		case kATCartridgeMode_Blizzard_16K:			return L"40: Blizzard 16K";
		case kATCartridgeMode_MaxFlash_128K:		return L"41: MaxFlash 128K / 1Mbit";
		case kATCartridgeMode_MaxFlash_1024K:		return L"42: MaxFlash 1M / 8Mbit - older (bank 127)";
		case kATCartridgeMode_SpartaDosX_128K:		return L"43: SpartaDOS X 128K";
		case kATCartridgeMode_OSS_8K:				return L"44: OSS 8K";
		case kATCartridgeMode_OSS_043M:				return L"45: OSS '043M'";
		case kATCartridgeMode_Blizzard_4K:			return L"46: Blizzard 4K";
		case kATCartridgeMode_AST_32K:				return L"47: AST 32K";
		case kATCartridgeMode_Atrax_SDX_64K:		return L"48: Atrax SDX 64K";
		case kATCartridgeMode_Atrax_SDX_128K:		return L"49: Atrax SDX 128K";
		case kATCartridgeMode_Turbosoft_64K:		return L"50: Turbosoft 64K";
		case kATCartridgeMode_Turbosoft_128K:		return L"51: Turbosoft 128K";
		case kATCartridgeMode_MaxFlash_128K_MyIDE:	return L"MaxFlash 128K + MyIDE";
		case kATCartridgeMode_Corina_1M_EEPROM:		return L"Corina 1M + 8K EEPROM";
		case kATCartridgeMode_Corina_512K_SRAM_EEPROM:	return L"Corina 512K + 512K SRAM + 8K EEPROM";
		case kATCartridgeMode_TelelinkII:			return L"8K Telelink II";
		case kATCartridgeMode_SIC:					return L"SIC!";
		case kATCartridgeMode_MaxFlash_1024K_Bank0:	return L"MaxFlash 1M / 8Mbit - newer (bank 0)";
		case kATCartridgeMode_Megacart_1M_2:		return L"Megacart 1M (2)";
		case kATCartridgeMode_5200_64K_32KBanks:	return L"5200 64K cartridge (32K banks)";
		default:
			return L"";
	}
}

int ATUIShowDialogCartridgeMapper(VDGUIHandle h, uint32 cartSize, bool show2600Warning) {
	ATUIDialogCartridgeMapper mapperdlg(cartSize, show2600Warning);

	return mapperdlg.ShowDialog(h) ? mapperdlg.GetMapper() : -1;
}
