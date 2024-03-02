//	Altirra - Atari 800/800XL/5200 emulator
//	Copyright (C) 2009-2011 Avery Lee
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

#ifndef f_AT_RS232_H
#define f_AT_RS232_H

class ATCPUEmulator;
class ATCPUEmulatorMemory;
class ATScheduler;
class IATUIRenderer;

struct ATRS232Config {
	bool	mbTelnetEmulation;
	bool	mbTelnetLFConversion;
	bool	mbAllowOutbound;
	bool	mbRequireMatchedDTERate;
	bool	mbExtendedBaudRates;
	bool	mbListenForIPv6;
	bool	mbDisableThrottling;
	uint32	mListenPort;
	uint32	mConnectionSpeed;

	ATRS232Config()
		: mbTelnetEmulation(true)
		, mbTelnetLFConversion(true)
		, mbAllowOutbound(true)
		, mbRequireMatchedDTERate(false)
		, mbExtendedBaudRates(false)
		, mbListenForIPv6(true)
		, mbDisableThrottling(false)
		, mListenPort(0)
		, mConnectionSpeed(9600)
	{
	}
};

class IATRS232Emulator {
public:
	virtual ~IATRS232Emulator() {}

	virtual void Init(ATCPUEmulatorMemory *mem, ATScheduler *sched, ATScheduler *slowsched, IATUIRenderer *uir) = 0;
	virtual void Shutdown() = 0;

	virtual void ColdReset() = 0;

	virtual void GetConfig(ATRS232Config& config) = 0;
	virtual void SetConfig(const ATRS232Config& config) = 0;

	virtual void OnCIOVector(ATCPUEmulator *cpu, ATCPUEmulatorMemory *mem, int offset) = 0;
};


struct ATRS232TerminalState {
	bool mbDataTerminalReady;
	bool mbRequestToSend;
};

struct ATRS232ControlState {
	bool mbCarrierDetect;
	bool mbClearToSend;
	bool mbDataSetReady;
};

class IATRS232DeviceCallback {
public:
	virtual void OnControlStateChanged(const ATRS232ControlState& status) = 0;
};

class IATRS232Device {
public:
	virtual ~IATRS232Device() {}

	virtual void SetCallback(IATRS232DeviceCallback *cb) = 0;

	virtual void ColdReset() = 0;

	virtual void SetTerminalState(const ATRS232TerminalState&) = 0;
	virtual ATRS232ControlState GetControlState() = 0;
	virtual bool Read(uint32 baudRate, uint8& c, bool& framingError) = 0;
	virtual void Write(uint32 baudRate, uint8 c) = 0;
	virtual void SetConfig(const ATRS232Config&) = 0;

	virtual void FlushOutputBuffer() = 0;
};

IATRS232Emulator *ATCreateRS232Emulator();

#endif