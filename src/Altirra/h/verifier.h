//	Altirra - Atari 800/800XL emulator
//	Copyright (C) 2008-2010 Avery Lee
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

#ifndef f_AT_VERIFIER_H
#define f_AT_VERIFIER_H

class ATCPUEmulator;
class ATCPUEmulatorMemory;
class ATSimulator;

enum {
	kATVerifierFlag_UndocumentedKernelEntry = 0x01,
	kATVerifierFlag_RecursiveNMI = 0x02,
	kATVerifierFlag_InterruptRegs = 0x04
};

class ATCPUVerifier {
	ATCPUVerifier(const ATCPUVerifier&);
	ATCPUVerifier& operator=(const ATCPUVerifier&);
public:
	ATCPUVerifier();
	~ATCPUVerifier();

	void Init(ATCPUEmulator *cpu, ATCPUEmulatorMemory *mem, ATSimulator *sim);

	uint32 GetFlags() const { return mFlags; }
	void SetFlags(uint32 flags);

	void AddAllowedTarget(uint32 addr);
	void RemoveAllowedTarget(uint32 addr);
	void RemoveAllowedTargets();
	void ResetAllowedTargets();
	void GetAllowedTargets(vdfastvector<uint16>& exceptions);

	void OnReset();
	void OnIRQEntry();
	void OnNMIEntry();
	void OnReturn();
	void VerifyJump(uint16 target);

protected:
	ATCPUEmulator *mpCPU;
	ATCPUEmulatorMemory *mpMemory;
	ATSimulator *mpSimulator;

	uint32	mFlags;

	bool	mbInNMIRoutine;
	uint8	mNMIStackLevel;

	typedef vdfastvector<uint16> Addresses;
	Addresses	mAllowedTargets;

	struct StackRegState {
		uint8	mA;
		uint8	mX;
		uint8	mY;
		uint8	mbActive;
		uint16	mPC;
		uint16	mPad2;
	};

	StackRegState mStackRegState[256];
};

#endif	// f_AT_VERIFIER_H