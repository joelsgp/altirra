//	Altirra - Atari 800/800XL emulator
//	Copyright (C) 2009 Avery Lee
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
#include <vd2/system/error.h>
#include <vd2/system/filesys.h>
#include "harddisk.h"
#include "kerneldb.h"
#include "oshelper.h"

namespace {
	enum {
		CIOStatBreak		= 0x80,	// break key abort
		CIOStatIOCBInUse	= 0x81,	// IOCB in use
		CIOStatUnkDevice	= 0x82,	// unknown device
		CIOStatWriteOnly	= 0x83,	// opened for write only
		CIOStatInvalidCmd	= 0x84,	// invalid command
		CIOStatNotOpen		= 0x85,	// device or file not open
		CIOStatInvalidIOCB	= 0x86,	// invalid IOCB number
		CIOStatReadOnly		= 0x87,	// opened for read only
		CIOStatEndOfFile	= 0x88,	// end of file reached
		CIOStatTruncRecord	= 0x89,	// record truncated
		CIOStatTimeout		= 0x8A,	// device timeout
		CIOStatNAK			= 0x8B,	// device NAK
		CIOStatSerFrameErr	= 0x8C,	// serial bus framing error
		CIOStatCursorRange	= 0x8D,	// cursor out of range
		CIOStatSerOverrun	= 0x8E,	// serial frame overrun error
		CIOStatSerChecksum	= 0x8F,	// serial checksum error
		CIOStatDeviceDone	= 0x90,	// device done error
		CIOStatBadScrnMode	= 0x91,	// bad screen mode
		CIOStatNotSupported	= 0x92,	// function not supported by handler
		CIOStatOutOfMemory	= 0x93,	// not enough memory
		CIOStatDriveNumErr	= 0xA0,	// disk drive # error
		CIOStatTooManyFiles	= 0xA1,	// too many open disk files
		CIOStatDiskFull		= 0xA2,	// disk full
		CIOStatFatalDiskIO	= 0xA3,	// fatal disk I/O error
		CIOStatFileNumDiff	= 0xA4,	// internal file # mismatch
		CIOStatFileNameErr	= 0xA5,	// filename error
		CIOStatPointDLen	= 0xA6,	// point data length error
		CIOStatFileLocked	= 0xA7,	// file locked
		CIOStatInvDiskCmd	= 0xA8,	// invalid command for disk
		CIOStatDirFull		= 0xA9,	// directory full (64 files)
		CIOStatFileNotFound	= 0xAA,	// file not found
		CIOStatInvPoint		= 0xAB,	// invalid point

		CIOCmdOpen			= 0x03,
		CIOCmdGetRecord		= 0x05,
		CIOCmdGetChars		= 0x07,
		CIOCmdPutRecord		= 0x09,
		CIOCmdPutChars		= 0x0B,
		CIOCmdClose			= 0x0C,
		CIOCmdGetStatus		= 0x0D,
		CIOCmdSpecial		= 0x0E	// $0E and up is escape
	};
}

class ATHardDiskEmulator : public IATHardDiskEmulator {
	ATHardDiskEmulator(const ATHardDiskEmulator&);
	ATHardDiskEmulator& operator=(const ATHardDiskEmulator&);
public:
	ATHardDiskEmulator();
	~ATHardDiskEmulator();

	bool IsEnabled() const;
	void SetEnabled(bool enabled);

	bool IsReadOnly() const;
	void SetReadOnly(bool enabled);

	const wchar_t *GetBasePath() const;
	void SetBasePath(const wchar_t *s);

	void WarmReset();
	void ColdReset();

	void OnCIOVector(ATCPUEmulator *cpu, ATCPUEmulatorMemory *mem, int offset);

protected:
	void DoOpen(ATCPUEmulator *cpu, ATCPUEmulatorMemory *mem);
	void DoClose(ATCPUEmulator *cpu, ATCPUEmulatorMemory *mem);
	void DoGetByte(ATCPUEmulator *cpu, ATCPUEmulatorMemory *mem);
	void DoPutByte(ATCPUEmulator *cpu, ATCPUEmulatorMemory *mem);
	void DoGetRecord(ATCPUEmulator *cpu, ATCPUEmulatorMemory *mem);
	void DoPutRecord(ATCPUEmulator *cpu, ATCPUEmulatorMemory *mem);
	void DoGetStatus(ATCPUEmulator *cpu, ATCPUEmulatorMemory *mem);
	void DoSpecial(ATCPUEmulator *cpu, ATCPUEmulatorMemory *mem);

	bool ReadFilename(ATCPUEmulator *cpu, ATCPUEmulatorMemory *mem);
	bool GetNextMatch(VDDirectoryIterator& it);

	static bool IsValidFilename(const char *s);

	struct Channel {
		VDFile	mFile;
		vdfastvector<uint8>	mData;
		uint32	mOffset;
		bool	mbUsingRawData;
		bool	mbOpen;
		bool	mbReadEnabled;
		bool	mbWriteEnabled;
	};

	Channel		mChannels[8];
	VDStringW	mBasePath;
	VDStringW	mSearchPattern;
	VDStringW	mFilter;
	bool		mbEnabled;
	bool		mbReadOnly;

	char		mFilename[128];
};

IATHardDiskEmulator *ATCreateHardDiskEmulator() {
	return new ATHardDiskEmulator;
}

ATHardDiskEmulator::ATHardDiskEmulator()
	: mbEnabled(false)
	, mbReadOnly(false)
{
	ColdReset();
}

ATHardDiskEmulator::~ATHardDiskEmulator() {
	ColdReset();
}

bool ATHardDiskEmulator::IsEnabled() const {
	return mbEnabled;
}

void ATHardDiskEmulator::SetEnabled(bool enable) {
	if (mbEnabled == enable)
		return;

	mbEnabled = enable;

	ColdReset();
}

bool ATHardDiskEmulator::IsReadOnly() const {
	return mbReadOnly;
}

void ATHardDiskEmulator::SetReadOnly(bool enabled) {
	mbReadOnly = enabled;
}

const wchar_t *ATHardDiskEmulator::GetBasePath() const {
	return mBasePath.c_str();
}

void ATHardDiskEmulator::SetBasePath(const wchar_t *basePath) {
	mBasePath = basePath;
	mSearchPattern = VDMakePath(basePath, L"*.*");
}

void ATHardDiskEmulator::WarmReset() {
	ColdReset();
}

void ATHardDiskEmulator::ColdReset() {
	for(int i=0; i<8; ++i) {
		Channel& ch = mChannels[i];

		ch.mbOpen = false;
		ch.mFile.closeNT();
	}
}

void ATHardDiskEmulator::OnCIOVector(ATCPUEmulator *cpu, ATCPUEmulatorMemory *mem, int offset) {
	if (!mbEnabled)
		return;

	switch(offset) {
		case 0:
			DoOpen(cpu, mem);
			break;

		case 2:
			DoClose(cpu, mem);
			break;

		case 4:
			DoGetByte(cpu, mem);
			break;

		case 6:
			DoPutByte(cpu, mem);
			break;

		case 8:
			DoGetStatus(cpu, mem);
			break;

		case 10:
			DoSpecial(cpu, mem);
			break;
	}
}

void ATHardDiskEmulator::DoOpen(ATCPUEmulator *cpu, ATCPUEmulatorMemory *mem) {
	ATKernelDatabase kdb(mem);
	const int idx = (cpu->GetX() >> 4) & 7;

	Channel& ch = mChannels[idx];
	if (ch.mbOpen) {
		cpu->SetY(CIOStatIOCBInUse);
		return;
	}

	uint8 mode = kdb.ICAX1Z;
	bool append = false;
	bool create = false;
	bool write = false;
	uint32 flags;

	switch(mode) {
		case 0x04:
			flags = nsVDFile::kRead | nsVDFile::kDenyWrite | nsVDFile::kOpenExisting;
			break;

		case 0x08:
			flags = nsVDFile::kWrite | nsVDFile::kDenyAll | nsVDFile::kCreateAlways;
			create = true;
			write = true;
			break;

		case 0x09:
			flags = nsVDFile::kWrite | nsVDFile::kDenyAll | nsVDFile::kOpenAlways;
			append = true;
			write = true;
			break;

		case 0x0C:
			flags = nsVDFile::kReadWrite | nsVDFile::kDenyAll | nsVDFile::kOpenExisting;
			write = true;
			break;

		case 0x06:
			break;

		default:
			cpu->SetY(CIOStatInvalidCmd);
			return;
	}

	if (write && mbReadOnly) {
		cpu->SetY(CIOStatReadOnly);
		return;
	}

	ch.mbReadEnabled = (mode & 0x04) != 0;
	ch.mbWriteEnabled = (mode & 0x08) != 0;

	if (!ReadFilename(cpu, mem))
		return;

	if (mode == 0x06) {
		ch.mbOpen = true;
		ch.mbUsingRawData = true;
		ch.mData.clear();
		ch.mOffset = 0;

		try {
			VDDirectoryIterator it(mSearchPattern.c_str());

			while(GetNextMatch(it)) {
				uint8 *s = ch.mData.alloc(18);

				memset(s, ' ', 18);
				
				const wchar_t *fn = it.GetName();
				const wchar_t *ext = VDFileSplitExt(fn);

				int flen = ext - fn;
				if (flen > 8)
					flen = 8;

				for(int i=0; i<flen; ++i)
					s[i+2] = toupper((char)fn[i]);

				if (*ext == L'.')
					++ext;

				int elen = wcslen(ext);
				if (elen > 3)
					elen = 3;

				for(int i=0; i<elen; ++i)
					s[i+10] = toupper((char)ext[i]);

				sint64 byteSize = it.GetSize();

				if (byteSize > 999 * 125)
					byteSize = 999 * 125;

				int sectors = ((int)byteSize + 124) / 125;

				s[14] = '0' + (sectors / 100);
				s[15] = '0' + ((sectors / 10) % 10);
				s[16] = '0' + (sectors % 10);
				s[17] = 0x9B;
			}
		} catch(const MyError&) {
		}

		uint8 *t = ch.mData.alloc(17);
		t[ 0] = '9';
		t[ 1] = '9';
		t[ 2] = '9';
		t[ 3] = ' ';
		t[ 4] = 'F';
		t[ 5] = 'R';
		t[ 6] = 'E';
		t[ 7] = 'E';
		t[ 8] = ' ';
		t[ 9] = 'S';
		t[10] = 'E';
		t[11] = 'C';
		t[12] = 'T';
		t[13] = 'O';
		t[14] = 'R';
		t[15] = 'S';
		t[16] = 0x9B;

		cpu->SetY(1);
	} else {
		// attempt to open file
		try {
			VDDirectoryIterator it(mSearchPattern.c_str());

			if (!GetNextMatch(it)) {
				if (create)
					ch.mFile.open(VDMakePath(mBasePath.c_str(), mFilter.c_str()).c_str(), flags);
				else {
					cpu->SetY(CIOStatFileNotFound);
					return;
				}
			} else {
				ch.mFile.open(it.GetFullPath().c_str(), flags);
			}
			if (append)
				ch.mFile.seek(0, nsVDFile::kSeekEnd);
		} catch(const MyError&) {
			ch.mFile.closeNT();
			cpu->SetY(CIOStatFileNotFound);
			return;
		}

		// all good
		ch.mbOpen = true;
		ch.mbUsingRawData = false;
		cpu->SetY(1);
	}
}

void ATHardDiskEmulator::DoClose(ATCPUEmulator *cpu, ATCPUEmulatorMemory *mem) {
	const int idx = (cpu->GetX() >> 4) & 7;
	Channel& ch = mChannels[idx];

	if (ch.mbOpen) {
		ch.mbOpen = false;
		ch.mFile.closeNT();
	}

	cpu->SetY(1);
}

void ATHardDiskEmulator::DoGetByte(ATCPUEmulator *cpu, ATCPUEmulatorMemory *mem) {
	const int idx = (cpu->GetX() >> 4) & 7;
	Channel& ch = mChannels[idx];

	if (!ch.mbOpen) {
		cpu->SetY(CIOStatNotOpen);
		return;
	}

	if (!ch.mbReadEnabled) {
		cpu->SetY(CIOStatWriteOnly);
		return;
	}

	if (ch.mbUsingRawData) {
		if (ch.mOffset >= ch.mData.size()) {
			cpu->SetY(CIOStatEndOfFile);
		} else {
			cpu->SetA(ch.mData[ch.mOffset++]);
			cpu->SetY(1);
		}
		return;
	}

	try {
		uint8 buf;
		int actual = ch.mFile.readData(&buf, 1);

		if (!actual) {
			cpu->SetY(CIOStatEndOfFile);
		} else {
			cpu->SetA(buf);
			cpu->SetY(1);
		}
	} catch(const MyError&) {
		cpu->SetY(CIOStatFatalDiskIO);
	}
}

void ATHardDiskEmulator::DoPutByte(ATCPUEmulator *cpu, ATCPUEmulatorMemory *mem) {
	const int idx = (cpu->GetX() >> 4) & 7;
	Channel& ch = mChannels[idx];

	if (!ch.mbOpen) {
		cpu->SetY(CIOStatNotOpen);
		return;
	}

	if (!ch.mbWriteEnabled) {
		cpu->SetY(CIOStatWriteOnly);
		return;
	}

	try {
		uint8 buf = cpu->GetA();
		int actual = ch.mFile.writeData(&buf, 1);

		if (!actual)
			cpu->SetY(CIOStatFatalDiskIO);
		else
			cpu->SetY(1);

	} catch(const MyError&) {
		cpu->SetY(CIOStatFatalDiskIO);
	}
}

void ATHardDiskEmulator::DoGetStatus(ATCPUEmulator *cpu, ATCPUEmulatorMemory *mem) {
	ATKernelDatabase kdb(mem);

	cpu->SetY(1);
}

void ATHardDiskEmulator::DoSpecial(ATCPUEmulator *cpu, ATCPUEmulatorMemory *mem) {
	ATKernelDatabase kdb(mem);
	const uint8 command = kdb.ICDNOZ;

	try {
		if (command == 0x25) {			// note
			const int idx = (cpu->GetX() >> 4) & 7;
			Channel& ch = mChannels[idx];

			if (!ch.mbOpen) {
				cpu->SetY(CIOStatNotOpen);
				return;
			}

			int offset = ch.mbUsingRawData ? ch.mOffset : (int)ch.mFile.tell();
			int sector = offset / 125;

			kdb.ICAX3Z = (uint8)sector;
			kdb.ICAX4Z = (uint8)(sector >> 8);
			kdb.ICAX5Z = (uint8)(offset % 125);

			cpu->SetY(1);		
		} else if (command == 0x26) {	// point
			const int idx = (cpu->GetX() >> 4) & 7;
			Channel& ch = mChannels[idx];

			if (!ch.mbOpen) {
				cpu->SetY(CIOStatNotOpen);
				return;
			}

			if (kdb.ICAX5Z >= 125) {
				cpu->SetY(CIOStatInvPoint);
				return;
			}

			int pos = 125*(kdb.ICAX3Z + 256*(int)kdb.ICAX4Z) + kdb.ICAX5Z;

			if (ch.mbUsingRawData) {
				if (pos > (int)ch.mData.size()) {
					cpu->SetY(CIOStatInvPoint);
					return;
				}

				ch.mOffset = pos;
			} else {
				if (pos > ch.mFile.tell()) {
					cpu->SetY(CIOStatInvPoint);
					return;
				}

				ch.mFile.seek(pos);
			}

		} else if (command == 0x23) {	// lock
			if (!ReadFilename(cpu, mem))
				return;

			VDDirectoryIterator it(mSearchPattern.c_str());

			while(GetNextMatch(it))
				ATFileSetReadOnlyAttribute(it.GetFullPath().c_str(), true);
		} else if (command == 0x24) {	// unlock
			if (!ReadFilename(cpu, mem))
				return;

			VDDirectoryIterator it(mSearchPattern.c_str());

			while(GetNextMatch(it))
				ATFileSetReadOnlyAttribute(it.GetFullPath().c_str(), false);
		} else if (command == 0x21) {	// delete
			if (mbReadOnly) {
				cpu->SetY(CIOStatReadOnly);
				return;
			}

			if (!ReadFilename(cpu, mem))
				return;

			VDDirectoryIterator it(mSearchPattern.c_str());

			while(GetNextMatch(it))
				VDRemoveFile(it.GetFullPath().c_str());
		}
	} catch(const MyError&) {
		cpu->SetY(CIOStatFatalDiskIO);
	}
}

bool ATHardDiskEmulator::ReadFilename(ATCPUEmulator *cpu, ATCPUEmulatorMemory *mem) {
	ATKernelDatabase kdb(mem);
	uint16 bufadr = kdb.ICBAZ;

	for(int i=0; i<128; ++i) {
		uint8 c = mem->ReadByte(bufadr + i);

		if (c == 0x9B || c == 0x20 || c == ',') {
			mFilename[i] = 0;
			break;
		}

		// check for excessively long or unterminated filename
		if (i == 127)  {
			cpu->SetY(CIOStatFileNameErr);
			return false;
		}

		// reject non-ASCII characters
		if (c < 0x20 || c > 0x7f) {
			cpu->SetY(CIOStatFileNameErr);
			return false;
		}

		// convert to lowercase
		if (c >= 0x61 && c <= 0x7A)
			c -= 0x20;

		mFilename[i] = (char)c;
	}

	// validate filename format
	if (!IsValidFilename(mFilename)) {
		cpu->SetY(CIOStatFileNameErr);
		return false;
	}

	const char *fn = strchr(mFilename, ':') + 1;
	mFilter = VDTextAToW(fn);

	return true;
}

bool ATHardDiskEmulator::GetNextMatch(VDDirectoryIterator& it) {
	for(;;) {
		if (!it.Next())
			return false;

		if (!it.IsDirectory() && VDFileWildMatch(mFilter.c_str(), it.GetName()))
			return true;
	}
}

bool ATHardDiskEmulator::IsValidFilename(const char *s) {
	char c;

	c = *s++;
	if (c != 'H')
		return false;

	c = *s++;
	if (c != ':') {
		if (c != '1')
			return false;

		c = *s++;
		if (c != ':')
			return false;
	}

	c = *s++;
	if ((c < 'A' || c > 'Z') && c != '*' && c != '?')
		return false;

	for(int i=0; i<7; ++i) {
		c = *s;
		if ((c < 'A' || c > 'Z') && (c < '0' || c > '9') && c != '*' && c != '?')
			break;
		++s;
	}

	c = *s;
	if (c == '.') {
		++s;

		for(int i=0; i<3; ++i) {
			c = *s;
			if ((c < 'A' || c > 'Z') && (c < '0' || c > '9') && c != '*' && c != '?')
				break;
			++s;
		}
	}

	return !*s;
}