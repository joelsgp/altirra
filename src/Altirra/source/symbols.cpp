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
#include <vd2/system/vdstl.h>
#include <vd2/system/error.h>
#include <vd2/system/file.h>
#include <vd2/system/filesys.h>
#include <vd2/system/strutil.h>
#include <vd2/system/VDString.h>
#include <algorithm>
#include <ctype.h>
#include <map>
#include "symbols.h"
#include "ksyms.h"

class ATSymbolFileParsingException : public MyError {
public:
	ATSymbolFileParsingException(int line) : MyError("Symbol file parsing failed at line %d.", line) {}
};

class ATSymbolStore : public vdrefcounted<IATCustomSymbolStore> {
public:
	ATSymbolStore();
	~ATSymbolStore();

	void Load(const wchar_t *filename);
	void Save(const wchar_t *filename);

	void Init(uint32 moduleBase, uint32 moduleSize);
	void RemoveSymbol(uint32 offset);
	void AddSymbol(uint32 offset, const char *name, uint32 size = 1, uint32 flags = kATSymbol_Read | kATSymbol_Write | kATSymbol_Execute, uint16 fileid = 0, uint16 lineno = 0);
	void AddReadWriteRegisterSymbol(uint32 offset, const char *writename, const char *readname = NULL);
	uint16 AddFileName(const wchar_t *filename);
	void AddSourceLine(uint16 fileId, uint16 line, uint32 moduleOffset);

public:
	uint32	GetDefaultBase() const { return mModuleBase; }
	uint32	GetDefaultSize() const { return mModuleSize; }
	bool	LookupSymbol(uint32 moduleOffset, uint32 flags, ATSymbol& symbol);
	sint32	LookupSymbol(const char *s);
	const wchar_t *GetFileName(uint16 fileid);
	uint16	GetFileId(const wchar_t *fileName);
	void	GetLines(uint16 fileId, vdfastvector<ATSourceLineInfo>& lines);
	bool	GetLineForOffset(uint32 moduleOffset, ATSourceLineInfo& lineInfo);
	bool	GetOffsetForLine(const ATSourceLineInfo& lineInfo, uint32& moduleOffset);
	uint32	GetSymbolCount() const;
	void	GetSymbol(uint32 index, ATSymbolInfo& symbol);

protected:
	void LoadSymbols(VDTextStream& ifile);
	void LoadCC65Labels(VDTextStream& ifile);
	void LoadLabels(VDTextStream& ifile);
	void LoadMADSListing(VDTextStream& ifile);
	void LoadKernelListing(VDTextStream& ifile);

	struct Symbol {
		uint32	mNameOffset;
		uint16	mOffset;
		uint8	mFlags;
		uint8	mSize;
		uint16	mFileId;
		uint16	mLine;
	};

	struct SymEqPred {
		bool operator()(const Symbol& sym, uint32 offset) const {
			return sym.mOffset == offset;
		}

		bool operator()(uint32 offset, const Symbol& sym) const {
			return offset == sym.mOffset;
		}

		bool operator()(const Symbol& sym1, const Symbol& sym2) const {
			return sym1.mOffset == sym2.mOffset;
		}
	};

	struct SymSort {
		bool operator()(const Symbol& sym, uint32 offset) const {
			return sym.mOffset < offset;
		}

		bool operator()(uint32 offset, const Symbol& sym) const {
			return offset < sym.mOffset;
		}

		bool operator()(const Symbol& sym1, const Symbol& sym2) const {
			return sym1.mOffset < sym2.mOffset;
		}
	};

	uint32	mModuleBase;
	uint32	mModuleSize;
	bool	mbSymbolsNeedSorting;

	typedef vdfastvector<Symbol> Symbols;
	Symbols					mSymbols;
	vdfastvector<char>		mNameBytes;
	vdfastvector<const wchar_t *>	mFileNames;

	typedef std::map<uint32, uint32> OffsetToLine;
	typedef std::map<uint32, uint32> LineToOffset;

	OffsetToLine	mOffsetToLine;
	LineToOffset	mLineToOffset;
};

ATSymbolStore::ATSymbolStore()
	: mModuleBase(0)
	, mbSymbolsNeedSorting(false)
{
}

ATSymbolStore::~ATSymbolStore() {
	while(!mFileNames.empty()) {
		delete mFileNames.back();
		mFileNames.pop_back();
	}
}

void ATSymbolStore::Load(const wchar_t *filename) {
	VDFileStream fs(filename);

	{
		VDTextStream ts(&fs);

		const char *line = ts.GetNextLine();

		if (!strncmp(line, "mads ", 5) || !strncmp(line, "xasm ", 5)) {
			LoadMADSListing(ts);
			return;
		}

		if (!strncmp(line, "Altirra symbol file", 19)) {
			LoadSymbols(ts);
			return;
		}

		if (!strncmp(line, "ca65 ", 5))
			throw MyError("CA65 listings are not supported.");
	}

	fs.Seek(0);

	VDTextStream ts2(&fs);

	const wchar_t *ext = VDFileSplitExt(filename);
	if (!vdwcsicmp(ext, L".lbl")) {
		LoadCC65Labels(ts2);
		return;
	}

	if (!vdwcsicmp(ext, L".lab")) {
		LoadLabels(ts2);
		return;
	}

	LoadKernelListing(ts2);
}

void ATSymbolStore::Init(uint32 moduleBase, uint32 moduleSize) {
	mModuleBase = moduleBase;
	mModuleSize = moduleSize;
}

void ATSymbolStore::RemoveSymbol(uint32 offset) {
	if (mbSymbolsNeedSorting) {
		std::sort(mSymbols.begin(), mSymbols.end(), SymSort());
		mbSymbolsNeedSorting = false;
	}

	Symbols::iterator it(std::lower_bound(mSymbols.begin(), mSymbols.end(), offset, SymSort()));

	if (it != mSymbols.end() && it->mOffset == offset)
		mSymbols.erase(it);
}

void ATSymbolStore::AddSymbol(uint32 offset, const char *name, uint32 size, uint32 flags, uint16 fileid, uint16 lineno) {
	Symbol sym;

	sym.mNameOffset = (uint32)mNameBytes.size();
	sym.mOffset		= (uint16)(offset - mModuleBase);
	sym.mFlags		= (uint8)flags;
	sym.mSize		= size > 255 ? 0 : size;
	sym.mFileId		= fileid;
	sym.mLine		= lineno;

	mSymbols.push_back(sym);
	mNameBytes.insert(mNameBytes.end(), name, name + strlen(name) + 1);

	mbSymbolsNeedSorting = true;
}

void ATSymbolStore::AddReadWriteRegisterSymbol(uint32 offset, const char *writename, const char *readname) {
	if (readname)
		AddSymbol(offset, readname, 1, kATSymbol_Read);

	if (writename)
		AddSymbol(offset, writename, 1, kATSymbol_Write);
}

uint16 ATSymbolStore::AddFileName(const wchar_t *filename) {
	VDStringW tempName(filename);
	VDStringW::size_type pos = 0;
	while((pos = tempName.find(L'/', pos)) != VDStringW::npos) {
		tempName[pos] = L'\\';
		++pos;
	}

	size_t n = mFileNames.size();
	for(size_t i=0; i<n; ++i)
		if (!_wcsicmp(mFileNames[i], tempName.c_str()))
			return i+1;

	mFileNames.push_back(_wcsdup(tempName.c_str()));
	return (uint16)mFileNames.size();
}

void ATSymbolStore::AddSourceLine(uint16 fileId, uint16 line, uint32 moduleOffset) {
	uint32 key = (fileId << 16) + line;
	mLineToOffset.insert(LineToOffset::value_type(key, moduleOffset));
	mOffsetToLine.insert(OffsetToLine::value_type(moduleOffset, key));
}

bool ATSymbolStore::LookupSymbol(uint32 moduleOffset, uint32 flags, ATSymbol& symout) {
	if (mbSymbolsNeedSorting) {
		std::sort(mSymbols.begin(), mSymbols.end(), SymSort());
		mbSymbolsNeedSorting = false;
	}

	Symbols::const_iterator itBegin(mSymbols.begin());
	Symbols::const_iterator it(std::upper_bound(mSymbols.begin(), mSymbols.end(), moduleOffset, SymSort()));

	while(it != itBegin) {
		--it;
		const Symbol& sym = *it;

		if (sym.mFlags & flags) {
			if (sym.mOffset && (moduleOffset - sym.mOffset) >= sym.mSize)
				return false;

			symout.mpName	= mNameBytes.data() + sym.mNameOffset;
			symout.mFlags	= sym.mFlags;
			symout.mOffset	= sym.mOffset;
			symout.mFileId	= sym.mFileId;
			symout.mLine	= sym.mLine;
			return true;
		}
	}

	return false;
}

sint32 ATSymbolStore::LookupSymbol(const char *s) {
	Symbols::const_iterator it(mSymbols.begin()), itEnd(mSymbols.end());
	for(; it != itEnd; ++it) {
		const Symbol& sym = *it;

		if (!_stricmp(s, mNameBytes.data() + sym.mNameOffset))
			return sym.mOffset;
	}

	return -1;
}

const wchar_t *ATSymbolStore::GetFileName(uint16 fileid) {
	if (!fileid)
		return NULL;

	--fileid;
	if (fileid >= mFileNames.size())
		return NULL;

	return mFileNames[fileid];
}

uint16 ATSymbolStore::GetFileId(const wchar_t *fileName) {
	VDStringW tempName(fileName);
	VDStringW::size_type pos = 0;
	while((pos = tempName.find(L'/', pos)) != VDStringW::npos) {
		tempName[pos] = L'\\';
		++pos;
	}

	bool fullyQualified = (tempName.find(L'\\') != VDStringW::npos || tempName.find(L':') != VDStringW::npos);

	const wchar_t *fullPath = tempName.c_str();
	size_t l1 = wcslen(fullPath);

	size_t n = mFileNames.size();
	for(size_t i=0; i<n; ++i) {
		const wchar_t *fn = mFileNames[i];

		if (fullyQualified && wcschr(fn, L':')) {
			if (!_wcsicmp(fn, fullPath))
				return (uint16)(i+1);
		} else {
			size_t l2 = wcslen(fn);
			size_t lm = l1 > l2 ? l2 : l1;

			if (!_wcsnicmp(fn+l2-lm, fullPath+l1-lm, lm)) {
				if ((l1 <= lm || wcschr(L"\\/:", fullPath[l1-lm-1])) && (l2 <= lm || wcschr(L"\\/:", fn[l2-lm-1])))
					return (uint16)(i+1);
			}
		}
	}

	return 0;
}

void ATSymbolStore::GetLines(uint16 matchFileId, vdfastvector<ATSourceLineInfo>& lines) {
	OffsetToLine::const_iterator it(mOffsetToLine.begin()), itEnd(mOffsetToLine.end());
	for(; it!=itEnd; ++it) {
		uint32 offset = it->first;
		uint32 key = it->second;
		uint16 fileId = key >> 16;

		if (fileId == matchFileId) {
			ATSourceLineInfo& linfo = lines.push_back();
			linfo.mOffset = offset;
			linfo.mFileId = matchFileId;
			linfo.mLine = key & 0xffff;
		}
	}
}

bool ATSymbolStore::GetLineForOffset(uint32 moduleOffset, ATSourceLineInfo& lineInfo) {
	OffsetToLine::const_iterator it(mOffsetToLine.upper_bound(moduleOffset));

	if (it == mOffsetToLine.begin())
		return false;

	--it;

	uint32 key = it->second;
	lineInfo.mOffset = moduleOffset;
	lineInfo.mFileId = key >> 16;
	lineInfo.mLine = key & 0xffff;
	return true;
}

bool ATSymbolStore::GetOffsetForLine(const ATSourceLineInfo& lineInfo, uint32& moduleOffset) {
	uint32 key = ((uint32)lineInfo.mFileId << 16) + lineInfo.mLine;

	OffsetToLine::const_iterator it(mOffsetToLine.find(key));

	if (it == mOffsetToLine.end())
		return false;

	moduleOffset = it->second;
	return true;
}

uint32 ATSymbolStore::GetSymbolCount() const {
	return mSymbols.size();
}

void ATSymbolStore::GetSymbol(uint32 index, ATSymbolInfo& symbol) {
	const Symbol& sym = mSymbols[index];

	symbol.mpName	= mNameBytes.data() + sym.mNameOffset;
	symbol.mFlags	= sym.mFlags;
	symbol.mOffset	= sym.mOffset;
	symbol.mLength	= sym.mSize;
}

void ATSymbolStore::LoadSymbols(VDTextStream& ifile) {
	enum {
		kStateNone,
		kStateSymbols
	} state = kStateNone;

	mModuleBase = 0;
	mModuleSize = 0x10000;

	int lineno = 0;
	while(const char *line = ifile.GetNextLine()) {
		++lineno;

		while(*line == ' ' || *line == '\t')
			++line;

		// skip comments
		if (*line == ';')
			continue;

		// skip blank lines
		if (!*line)
			continue;

		// check for group
		if (*line == '[') {
			const char *groupStart = ++line;

			while(*line != ']') {
				if (!*line)
					throw ATSymbolFileParsingException(lineno);
				++line;
			}

			VDStringSpanA groupName(groupStart, line);

			if (groupName == "symbols")
				state = kStateSymbols;
			else
				state = kStateNone;

			continue;
		}

		if (state == kStateSymbols) {
			// rwx address,length name
			uint32 rwxflags = 0;
			for(;;) {
				char c = *line++;

				if (!c)
					throw ATSymbolFileParsingException(lineno);

				if (c == ' ' || c == '\t')
					break;

				if (c == 'r')
					rwxflags |= kATSymbol_Read;
				else if (c == 'w')
					rwxflags |= kATSymbol_Write;
				else if (c == 'x')
					rwxflags |= kATSymbol_Execute;
			}

			if (!rwxflags)
				throw ATSymbolFileParsingException(lineno);

			while(*line == ' ' || *line == '\t')
				++line;

			char *end;
			unsigned long address = strtoul(line, &end, 16);

			if (line == end)
				throw ATSymbolFileParsingException(lineno);

			line = end;

			if (*line++ != ',')
				throw ATSymbolFileParsingException(lineno);

			unsigned long length = strtoul(line, &end, 16);
			if (line == end)
				throw ATSymbolFileParsingException(lineno);

			line = end;

			while(*line == ' ' || *line == '\t')
				++line;

			const char *nameStart = line;

			while(*line != ' ' && *line != '\t' && *line != ';' && *line)
				++line;

			if (line == nameStart)
				throw ATSymbolFileParsingException(lineno);

			const char *nameEnd = line;

			while(*line == ' ' || *line == '\t')
				++line;

			if (*line && *line != ';')
				throw ATSymbolFileParsingException(lineno);

			AddSymbol(address, VDStringA(nameStart, nameEnd).c_str(), length);
		}
	}
}

void ATSymbolStore::LoadCC65Labels(VDTextStream& ifile) {
	VDStringA label;

	while(const char *line = ifile.GetNextLine()) {
		unsigned long addr;
		int nameoffset;
		char namecheck;

		if (2 != sscanf(line, "al %6lx %n%c", &addr, &nameoffset, &namecheck))
			continue;

		const char *labelStart = line + nameoffset;
		const char *labelEnd = labelStart;

		for(;;) {
			char c = *labelEnd;

			if (!c || c == ' ' || c == '\t' || c == '\n' || c== '\r')
				break;

			++labelEnd;
		}

		label.assign(labelStart, labelEnd);
		AddSymbol(addr, label.c_str());
	}

	mModuleBase = 0;
	mModuleSize = 0x10000;
}

void ATSymbolStore::LoadLabels(VDTextStream& ifile) {
	VDStringA label;

	while(const char *line = ifile.GetNextLine()) {
		unsigned long addr;
		int nameoffset;
		char namecheck;

		if (2 != sscanf(line, "%6lx %n%c", &addr, &nameoffset, &namecheck))
			continue;

		const char *labelStart = line + nameoffset;
		const char *labelEnd = labelStart;

		for(;;) {
			char c = *labelEnd;

			if (!c || c == ' ' || c == '\t' || c == '\n' || c== '\r')
				break;

			++labelEnd;
		}

		label.assign(labelStart, labelEnd);
		AddSymbol(addr, label.c_str());
	}

	mModuleBase = 0;
	mModuleSize = 0x10000;	
}

void ATSymbolStore::LoadMADSListing(VDTextStream& ifile) {
	uint16 fileid = 0;

	enum {
		kModeNone,
		kModeSource,
		kModeLabels
	} mode = kModeNone;

	VDStringA label;

	typedef vdfastvector<std::pair<int, int> > LastLines;
	LastLines lastLines;
	int nextline = 1;
	bool macroMode = false;

	while(const char *line = ifile.GetNextLine()) {
		char space0;
		int origline;
		int address;
		int address2;
		char dummy;
		char space1;
		char space2;
		char space3;
		char space4;
		int op;

		if (!strncmp(line, "Macro: ", 7)) {
			macroMode = true;
		} else if (!strncmp(line, "Source: ", 8)) {
			if (macroMode)
				macroMode = false;
			else {
				if (fileid)
					lastLines.push_back(LastLines::value_type(nextline, fileid));

				fileid = AddFileName(VDTextAToW(line+8).c_str());
				mode = kModeSource;
				nextline = 1;
			}

			continue;
		} else if (!strncmp(line, "Label table:", 12)) {
			fileid = 0;
			mode = kModeLabels;
		}

		if (mode == kModeSource) {
			if (macroMode)
				continue;

			bool valid = false;

			if (2 == sscanf(line, "%c%5d", &space0, &origline)
				&& space0 == ' ')
			{
				if (fileid && origline > 0) {
					// check for discontinuous line (mads doesn't re-emit the parent line)
					if (origline != nextline) {
						LastLines::const_reverse_iterator it(lastLines.rbegin()), itEnd(lastLines.rend());

						for(; it != itEnd; ++it) {
							if (it->first == origline && it->second != fileid) {
								fileid = it->second;
								break;
							}
						}
					}

					nextline = origline + 1;
				}

				// 105 1088 8D ...
				// 131 2000-201F> 00 ...
				if (7 == sscanf(line, "%c%5d%c%4x%c%2x%c", &space0, &origline, &space1, &address, &space2, &op, &space3)
					&& space0 == ' '
					&& space1 == ' '
					&& space2 == ' '
					&& (space3 == ' ' || space3 == '\t'))
				{
					if (fileid && origline > 0)
						AddSourceLine(fileid, origline, address);

					valid = true;
				} else if (8 == sscanf(line, "%c%5d%c%4x-%4x>%c%2x%c", &space0, &origline, &space1, &address, &address2, &space2, &op, &space3)
					&& space0 == ' '
					&& space1 == ' '
					&& space2 == ' '
					&& (space3 == ' ' || space3 == '\t'))
				{
					if (fileid && origline > 0)
						AddSourceLine(fileid, origline, address);

					valid = true;
				}
			} else if (8 == sscanf(line, "%6x%c%c%c%c%c%2x%c", &address, &space0, &space1, &dummy, &space2, &space3, &op, &space4)
				&& space0 == ' '
				&& space1 == ' '
				&& space2 == ' '
				&& space3 == ' '
				&& space4 == ' '
				&& isdigit((unsigned char)dummy))
			{
				valid = true;
			} else if (6 == sscanf(line, "%6d%c%4x%c%2x%c", &origline, &space0, &address, &space1, &op, &space2)
				&& space0 == ' '
				&& space1 == ' '
				&& (space2 == ' ' || space2 == '\t'))
			{
				valid = true;
			}

			if (valid) { 
				uint32 key = ((uint32)fileid << 16) + origline;
				mOffsetToLine.insert(OffsetToLine::value_type(address, key));
				mLineToOffset.insert(LineToOffset::value_type(key, address));
			}
		} else if (mode == kModeLabels) {
			// MADS:
			// 00      11A3    DLI
			//
			// xasm:
			//         2000 MAIN

			if (isdigit((unsigned char)line[0])) {
				int pos1;
				int pos2;
				if (3 == sscanf(line, "%2x %4x %n%c%*s%n", &op, &address, &pos1, &dummy, &pos2)) {
					label.assign(line + pos1, line + pos2);

					AddSymbol(address, label.c_str());
				}
			} else {
				int pos1;
				int pos2;
				if (2 == sscanf(line, "%4x %n%c%*s%n", &address, &pos1, &dummy, &pos2)) {
					label.assign(line + pos1, line + pos2);

					AddSymbol(address, label.c_str());
				}
			}
		}
	}

	mModuleBase = 0;
	mModuleSize = 0x10000;
}

void ATSymbolStore::LoadKernelListing(VDTextStream& ifile) {
	// hardcoded for now for the kernel
	Init(0xD800, 0x2800);

	while(const char *line = ifile.GetNextLine()) {
		int len = strlen(line);
		if (len < 33)
			continue;

		// What we're looking for:
		//    3587  F138  A9 00            ZERORM

		const char *s = line;
		if (*s++ != ' ') continue;
		if (*s++ != ' ') continue;
		if (*s++ != ' ') continue;

		// skip line number
		while(*s == ' ')
			++s;
		if (!isdigit((unsigned char)*s++)) continue;
		while(isdigit((unsigned char)*s))
			++s;

		if (*s++ != ' ') continue;
		if (*s++ != ' ') continue;

		// read address
		uint32 address = 0;
		for(int i=0; i<4; ++i) {
			char c = *s;
			if (!isxdigit((unsigned char)c))
				goto fail;

			++s;
			c = toupper(c);
			if (c >= 'A')
				c -= 7;

			address = (address << 4) + (c - '0');
		}

		// skip two more spaces
		if (*s++ != ' ') continue;
		if (*s++ != ' ') continue;

		// check for first opcode byte
		if (!isxdigit((unsigned char)*s++)) continue;
		if (!isxdigit((unsigned char)*s++)) continue;

		// skip all the way to label
		s = line + 33;
		const char *t = s;
		while(isalpha((unsigned char)*t))
			++t;

		if (t != s) {
			AddSymbol(address, VDStringA(s, t-s).c_str());
		}

fail:
		;
	}
}

///////////////////////////////////////////////////////////////////////////////

bool ATCreateDefaultVariableSymbolStore(IATSymbolStore **ppStore) {
	vdrefptr<ATSymbolStore> symstore(new ATSymbolStore);

	symstore->Init(0x0000, 0x0400);

	using namespace ATKernelSymbols;
	symstore->AddSymbol(WARMST, "WARMST", 1);
	symstore->AddSymbol(DOSVEC, "DOSVEC", 1);
	symstore->AddSymbol(DOSINI, "DOSINI", 1);
	symstore->AddSymbol(POKMSK, "POKMSK", 1);
	symstore->AddSymbol(BRKKEY, "BRKKEY", 1);
	symstore->AddSymbol(RTCLOK, "RTCLOK", 1);
	symstore->AddSymbol(ICDNOZ, "ICDNOZ", 1);
	symstore->AddSymbol(ICBALZ, "ICBALZ", 1);
	symstore->AddSymbol(ICBAHZ, "ICBAHZ", 1);
	symstore->AddSymbol(ICAX1Z, "ICAX1Z", 1);
	symstore->AddSymbol(ICAX2Z, "ICAX2Z", 1);
	symstore->AddSymbol(STATUS, "STATUS", 1);
	symstore->AddSymbol(CHKSUM, "CHKSUM", 1);
	symstore->AddSymbol(BUFRLO, "BUFRLO", 1);
	symstore->AddSymbol(BUFRHI, "BUFRHI", 1);
	symstore->AddSymbol(BFENLO, "BFENLO", 1);
	symstore->AddSymbol(BFENHI, "BFENHI", 1);
	symstore->AddSymbol(BUFRFL, "BUFRFL", 1);
	symstore->AddSymbol(RECVDN, "RECVDN", 1);
	symstore->AddSymbol(CHKSNT, "CHKSNT", 1);
	symstore->AddSymbol(CRITIC, "CRITIC", 1);
	symstore->AddSymbol(ATRACT, "ATRACT", 1);
	symstore->AddSymbol(DRKMSK, "DRKMSK", 1);
	symstore->AddSymbol(COLRSH, "COLRSH", 1);
	symstore->AddSymbol(LMARGN, "LMARGN", 1);
	symstore->AddSymbol(RMARGN, "RMARGN", 1);
	symstore->AddSymbol(ROWCRS, "ROWCRS", 1);
	symstore->AddSymbol(COLCRS, "COLCRS", 2);
	symstore->AddSymbol(DINDEX, "DINDEX", 1);
	symstore->AddSymbol(SAVMSC, "SAVMSC", 2);
	symstore->AddSymbol(LOGCOL, "LOGCOL", 1);
	symstore->AddSymbol(RAMTOP, "RAMTOP", 1);
	symstore->AddSymbol(BUFCNT, "BUFCNT", 1);
	symstore->AddSymbol(BUFADR, "BUFADR", 2);
	symstore->AddSymbol(RAMLO, "RAMLO", 1);
	symstore->AddSymbol(FR0, "FR0", 1);
	symstore->AddSymbol(FR1, "FR1", 1);
	symstore->AddSymbol(CIX, "CIX", 1);
	symstore->AddSymbol(INBUFF, "INBUFF", 1);
	symstore->AddSymbol(FLPTR, "FLPTR", 1);
	symstore->AddSymbol(VDSLST, "VDSLST", 1);
	symstore->AddSymbol(VPRCED, "VPRCED", 1);
	symstore->AddSymbol(VINTER, "VINTER", 1);
	symstore->AddSymbol(VBREAK, "VBREAK", 1);
	symstore->AddSymbol(VKEYBD, "VKEYBD", 1);
	symstore->AddSymbol(VSERIN, "VSERIN", 1);
	symstore->AddSymbol(VSEROR, "VSEROR", 1);
	symstore->AddSymbol(VSEROC, "VSEROC", 1);
	symstore->AddSymbol(VTIMR1, "VTIMR1", 1);
	symstore->AddSymbol(VTIMR2, "VTIMR2", 1);
	symstore->AddSymbol(VTIMR4, "VTIMR4", 1);
	symstore->AddSymbol(VIMIRQ, "VIMIRQ", 1);
	symstore->AddSymbol(CDTMV1, "CDTMV1", 1);
	symstore->AddSymbol(CDTMV2, "CDTMV2", 1);
	symstore->AddSymbol(CDTMV3, "CDTMV3", 1);
	symstore->AddSymbol(CDTMV4, "CDTMV4", 1);
	symstore->AddSymbol(CDTMV5, "CDTMV5", 1);
	symstore->AddSymbol(VVBLKI, "VVBLKI", 1);
	symstore->AddSymbol(VVBLKD, "VVBLKD", 1);
	symstore->AddSymbol(CDTMA1, "CDTMA1", 1);
	symstore->AddSymbol(CDTMA2, "CDTMA2", 1);
	symstore->AddSymbol(CDTMF3, "CDTMF3", 1);
	symstore->AddSymbol(CDTMF4, "CDTMF4", 1);
	symstore->AddSymbol(CDTMF5, "CDTMF5", 1);
	symstore->AddSymbol(SDMCTL, "SDMCTL", 1);
	symstore->AddSymbol(SDLSTL, "SDLSTL", 1);
	symstore->AddSymbol(SDLSTH, "SDLSTH", 1);
	symstore->AddSymbol(COLDST, "COLDST", 1);
	symstore->AddSymbol(GPRIOR, "GPRIOR", 1);
	symstore->AddSymbol(JVECK, "JVECK", 1);
	symstore->AddSymbol(PCOLR0, "PCOLR0", 1);
	symstore->AddSymbol(PCOLR1, "PCOLR1", 1);
	symstore->AddSymbol(PCOLR2, "PCOLR2", 1);
	symstore->AddSymbol(PCOLR3, "PCOLR3", 1);
	symstore->AddSymbol(COLOR0, "COLOR0", 1);
	symstore->AddSymbol(COLOR1, "COLOR1", 1);
	symstore->AddSymbol(COLOR2, "COLOR2", 1);
	symstore->AddSymbol(COLOR3, "COLOR3", 1);
	symstore->AddSymbol(COLOR4, "COLOR4", 1);
	symstore->AddSymbol(MEMTOP, "MEMTOP", 1);
	symstore->AddSymbol(MEMLO, "MEMLO", 1);
	symstore->AddSymbol(CHACT, "CHACT", 1);
	symstore->AddSymbol(KEYDEL, "KEYDEL", 1);
	symstore->AddSymbol(CH1, "CH1", 1);
	symstore->AddSymbol(CHBAS, "CHBAS", 1);
	symstore->AddSymbol(CH, "CH", 1);
	symstore->AddSymbol(DDEVIC, "DDEVIC", 1);
	symstore->AddSymbol(DUNIT, "DUNIT", 1);
	symstore->AddSymbol(TIMFLG, "TIMFLG", 1);
	symstore->AddSymbol(HATABS, "HATABS", 1);
	symstore->AddSymbol(LBUFF, "LBUFF", 1);

	*ppStore = symstore.release();
	return true;
}

bool ATCreateDefaultVariableSymbolStore5200(IATSymbolStore **ppStore) {
	vdrefptr<ATSymbolStore> symstore(new ATSymbolStore);

	symstore->Init(0x0000, 0x0400);

	using namespace ATKernelSymbols5200;
	symstore->AddSymbol(POKMSK, "POKMSK", 1);
	symstore->AddSymbol(RTCLOK, "RTCLOK", 1);
	symstore->AddSymbol(CRITIC, "CRITIC", 1);
	symstore->AddSymbol(ATRACT, "ATRACT", 1);
	symstore->AddSymbol(SDMCTL, "SDMCTL", 1);
	symstore->AddSymbol(SDLSTL, "SDLSTL", 1);
	symstore->AddSymbol(SDLSTH, "SDLSTH", 1);
	symstore->AddSymbol(PCOLR0, "PCOLR0", 1);
	symstore->AddSymbol(PCOLR1, "PCOLR1", 1);
	symstore->AddSymbol(PCOLR2, "PCOLR2", 1);
	symstore->AddSymbol(PCOLR3, "PCOLR3", 1);
	symstore->AddSymbol(COLOR0, "COLOR0", 1);
	symstore->AddSymbol(COLOR1, "COLOR1", 1);
	symstore->AddSymbol(COLOR2, "COLOR2", 1);
	symstore->AddSymbol(COLOR3, "COLOR3", 1);
	symstore->AddSymbol(COLOR4, "COLOR4", 1);

	symstore->AddSymbol(VIMIRQ, "VIMIRQ", 1);
	symstore->AddSymbol(VVBLKI, "VVBLKI", 1);
	symstore->AddSymbol(VVBLKD, "VVBLKD", 1);
	symstore->AddSymbol(VDSLST, "VDSLST", 1);
	symstore->AddSymbol(VTRIGR, "VTRIGR", 1);
	symstore->AddSymbol(VBRKOP, "VBRKOP", 1);
	symstore->AddSymbol(VKYBDI, "VKYBDI", 1);
	symstore->AddSymbol(VKYBDF, "VKYBDF", 1);
	symstore->AddSymbol(VSERIN, "VSERIN", 1);
	symstore->AddSymbol(VSEROR, "VSEROR", 1);
	symstore->AddSymbol(VSEROC, "VSEROC", 1);
	symstore->AddSymbol(VTIMR1, "VTIMR1", 1);
	symstore->AddSymbol(VTIMR2, "VTIMR2", 1);
	symstore->AddSymbol(VTIMR4, "VTIMR4", 1);

	*ppStore = symstore.release();
	return true;
}

bool ATCreateDefaultKernelSymbolStore(IATSymbolStore **ppStore) {
	using namespace ATKernelSymbols;

	vdrefptr<ATSymbolStore> symstore(new ATSymbolStore);

	symstore->Init(0xD800, 0x0D00);
	symstore->AddSymbol(AFP, "AFP", 1);
	symstore->AddSymbol(FASC, "FASC", 1);
	symstore->AddSymbol(IPF, "IPF", 1);
	symstore->AddSymbol(FPI, "FPI", 1);
	symstore->AddSymbol(ZFR0, "ZFR0", 1);
	symstore->AddSymbol(ZF1, "ZF1", 1);
	symstore->AddSymbol(FADD, "FADD", 1);
	symstore->AddSymbol(FSUB, "FSUB", 1);
	symstore->AddSymbol(FMUL, "FMUL", 1);
	symstore->AddSymbol(FDIV, "FDIV", 1);
	symstore->AddSymbol(PLYEVL, "PLYEVL", 1);
	symstore->AddSymbol(FLD0R, "FLD0R", 1);
	symstore->AddSymbol(FLD0P, "FLD0P", 1);
	symstore->AddSymbol(FLD1R, "FLD1R", 1);
	symstore->AddSymbol(FLD1P, "FLD1P", 1);
	symstore->AddSymbol(FST0R, "FST0R", 1);
	symstore->AddSymbol(FST0P, "FST0P", 1);
	symstore->AddSymbol(FMOVE, "FMOVE", 1);
	symstore->AddSymbol(EXP, "EXP", 1);
	symstore->AddSymbol(EXP10, "EXP10", 1);
	symstore->AddSymbol(LOG, "LOG", 1);
	symstore->AddSymbol(LOG10, "LOG10", 1);
	symstore->AddSymbol(0xE400, "EDITRV", 3);
	symstore->AddSymbol(0xE410, "SCRENV", 3);
	symstore->AddSymbol(0xE420, "KEYBDV", 3);
	symstore->AddSymbol(0xE430, "PRINTV", 3);
	symstore->AddSymbol(0xE440, "CASETV", 3);
	symstore->AddSymbol(0xE450, "DISKIV", 3);
	symstore->AddSymbol(0xE453, "DSKINV", 3);
	symstore->AddSymbol(0xE456, "CIOV", 3);
	symstore->AddSymbol(0xE459, "SIOV", 3);
	symstore->AddSymbol(0xE45C, "SETVBV", 3);
	symstore->AddSymbol(0xE45F, "SYSVBV", 3);
	symstore->AddSymbol(0xE462, "XITVBV", 3);
	symstore->AddSymbol(0xE465, "SIOINV", 3);
	symstore->AddSymbol(0xE468, "SENDEV", 3);
	symstore->AddSymbol(0xE46B, "INTINV", 3);
	symstore->AddSymbol(0xE46E, "CIOINV", 3);
	symstore->AddSymbol(0xE471, "BLKBDV", 3);
	symstore->AddSymbol(0xE474, "WARMSV", 3);
	symstore->AddSymbol(0xE477, "COLDSV", 3);
	symstore->AddSymbol(0xE47A, "RBLOKV", 3);
	symstore->AddSymbol(0xE47D, "CSOPIV", 3);
	symstore->AddSymbol(0xE480, "VCTABL", 3);

	*ppStore = symstore.release();
	return true;
}

namespace {
	struct HardwareSymbol {
		uint32 mOffset;
		const char *mpWriteName;
		const char *mpReadName;
	};

	static const HardwareSymbol kGTIASymbols[]={
		{ 0x00, "HPOSP0", "M0PF" },
		{ 0x01, "HPOSP1", "M1PF" },
		{ 0x02, "HPOSP2", "M2PF" },
		{ 0x03, "HPOSP3", "M3PF" },
		{ 0x04, "HPOSM0", "P0PF" },
		{ 0x05, "HPOSM1", "P1PF" },
		{ 0x06, "HPOSM2", "P2PF" },
		{ 0x07, "HPOSM3", "P3PF" },
		{ 0x08, "SIZEP0", "M0PL" },
		{ 0x09, "SIZEP1", "M1PL" },
		{ 0x0A, "SIZEP2", "M2PL" },
		{ 0x0B, "SIZEP3", "M3PL" },
		{ 0x0C, "SIZEM", "P0PL" },
		{ 0x0D, "GRAFP0", "P1PL" },
		{ 0x0E, "GRAFP1", "P2PL" },
		{ 0x0F, "GRAFP2", "P3PL" },
		{ 0x10, "GRAFP3", "TRIG0" },
		{ 0x11, "GRAFM", "TRIG1" },
		{ 0x12, "COLPM0", "TRIG2" },
		{ 0x13, "COLPM1", "TRIG3" },
		{ 0x14, "COLPM2", "PAL" },
		{ 0x15, "COLPM3", NULL },
		{ 0x16, "COLPF0" },
		{ 0x17, "COLPF1" },
		{ 0x18, "COLPF2" },
		{ 0x19, "COLPF3" },
		{ 0x1A, "COLBK" },
		{ 0x1B, "PRIOR" },
		{ 0x1C, "VDELAY" },
		{ 0x1D, "GRACTL" },
		{ 0x1E, "HITCLR" },
		{ 0x1F, "CONSOL", "CONSOL" },
	};

	static const HardwareSymbol kPOKEYSymbols[]={
		{ 0x00, "AUDF1", "POT0" },
		{ 0x01, "AUDC1", "POT1" },
		{ 0x02, "AUDF2", "POT2" },
		{ 0x03, "AUDC2", "POT3" },
		{ 0x04, "AUDF3", "POT4" },
		{ 0x05, "AUDC3", "POT5" },
		{ 0x06, "AUDF4", "POT6" },
		{ 0x07, "AUDC4", "POT7" },
		{ 0x08, "AUDCTL", "ALLPOT" },
		{ 0x09, "STIMER", "KBCODE" },
		{ 0x0A, "SKRES", "RANDOM" },
		{ 0x0B, "POTGO" },
		{ 0x0D, "SEROUT", "SERIN" },
		{ 0x0E, "IRQEN", "IRQST" },
		{ 0x0F, "SKCTL", "SKSTAT" },
	};

	static const HardwareSymbol kPIASymbols[]={
		{ 0x00, "PORTA" },
		{ 0x01, "PORTB" },
		{ 0x02, "PACTL" },
		{ 0x03, "PBCTL" },
	};

	static const HardwareSymbol kANTICSymbols[]={
		{ 0x00, "DMACTL" },
		{ 0x01, "CHACTL" },
		{ 0x02, "DLISTL" },
		{ 0x03, "DLISTH" },
		{ 0x04, "HSCROL" },
		{ 0x05, "VSCROL" },
		{ 0x07, "PMBASE" },
		{ 0x09, "CHBASE" },
		{ 0x0A, "WSYNC" },
		{ 0x0B, NULL, "VCOUNT" },
		{ 0x0C, NULL, "PENH" },
		{ 0x0D, NULL, "PENV" },
		{ 0x0E, "NMIEN" },
		{ 0x0F, "NMIRES", "NMIST" },
	};

	void AddHardwareSymbols(ATSymbolStore *store, uint32 base, const HardwareSymbol *sym, uint32 n) {
		while(n--) {
			store->AddReadWriteRegisterSymbol(base + sym->mOffset, sym->mpWriteName, sym->mpReadName);
			++sym;
		}
	}

	template<size_t N>
	inline void AddHardwareSymbols(ATSymbolStore *store, uint32 base, const HardwareSymbol (&syms)[N]) {
		AddHardwareSymbols(store, base, syms, N);
	}
}

bool ATCreateDefaultHardwareSymbolStore(IATSymbolStore **ppStore) {
	vdrefptr<ATSymbolStore> symstore(new ATSymbolStore);

	symstore->Init(0xD000, 0x0500);
	AddHardwareSymbols(symstore, 0xD000, kGTIASymbols);
	AddHardwareSymbols(symstore, 0xD200, kPOKEYSymbols);
	AddHardwareSymbols(symstore, 0xD300, kPIASymbols);
	AddHardwareSymbols(symstore, 0xD400, kANTICSymbols);

	*ppStore = symstore.release();
	return true;
}

bool ATCreateDefault5200HardwareSymbolStore(IATSymbolStore **ppStore) {
	vdrefptr<ATSymbolStore> symstore(new ATSymbolStore);

	symstore->Init(0xC000, 0x3000);
	AddHardwareSymbols(symstore, 0xC000, kGTIASymbols);
	AddHardwareSymbols(symstore, 0xE800, kPOKEYSymbols);
	AddHardwareSymbols(symstore, 0xD400, kANTICSymbols);

	*ppStore = symstore.release();
	return true;
}

bool ATCreateCustomSymbolStore(IATCustomSymbolStore **ppStore) {
	vdrefptr<ATSymbolStore> symstore(new ATSymbolStore);
	*ppStore = symstore.release();
	return true;
}

bool ATLoadSymbols(const wchar_t *sym, IATSymbolStore **outsymbols) {
	vdrefptr<IATCustomSymbolStore> symbols;
	if (!ATCreateCustomSymbolStore(~symbols))
		return false;

	try {
		symbols->Load(sym);
	} catch(const MyError&) {
		return false;
	}

	*outsymbols = symbols.release();
	return true;
}

void ATSaveSymbols(const wchar_t *filename, IATSymbolStore *syms) {
	VDFileStream fs(filename, nsVDFile::kWrite | nsVDFile::kDenyAll | nsVDFile::kCreateAlways);
	VDTextOutputStream tos(&fs);

	// write header
	tos.PutLine("Altirra symbol file");

	// write out symbols
	tos.PutLine();
	tos.PutLine("[symbols]");

	uint32 base = syms->GetDefaultBase();

	const uint32 n = syms->GetSymbolCount();
	for(uint32 i=0; i<n; ++i) {
		ATSymbolInfo sym;

		syms->GetSymbol(i, sym);

		tos.FormatLine("%s%s%s %04x,%x %s"
			, sym.mFlags & kATSymbol_Read ? "r" : ""
			, sym.mFlags & kATSymbol_Write ? "w" : ""
			, sym.mFlags & kATSymbol_Execute ? "x" : ""
			, base + sym.mOffset
			, sym.mLength
			, sym.mpName);
	}
}
