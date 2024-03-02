#include "stdafx.h"
#include <vd2/system/binary.h>
#include <vd2/system/file.h>
#include <vd2/system/strutil.h>
#include "diskfs.h"
#include "diskimage.h"

class ATARCCRC {
public:
	ATARCCRC();

	uint16 Compute(const uint8 *data, uint32 len) const;

	uint8	mTableLo[256];
	uint8	mTableHi[256];
};

ATARCCRC::ATARCCRC() {
	for(int i=0; i<256; ++i) {
		uint32 v = i;
		for(int j=0; j<8; ++j) {
			v = (v >> 1) ^ (0xa001 & -(sint32)(v & 1));
		}

		mTableLo[i] = (uint8)v;
		mTableHi[i] = (uint8)(v >> 8);
	}
}

uint16 ATARCCRC::Compute(const uint8 *data, uint32 len) const {
	uint8 hi = 0;
	uint8 lo = 0;

	while(len--) {
		uint32 index = lo ^ *data++;

		lo = hi ^ mTableLo[index];
		hi = mTableHi[index];
	}

	return ((uint16)hi << 8) + lo;
}

class ATDiskFSARC : public IATDiskFS {
public:
	ATDiskFSARC();
	~ATDiskFSARC();

public:
	void Init(const wchar_t *path);
	void GetInfo(ATDiskFSInfo& info);

	bool IsReadOnly() { return true; }
	void SetReadOnly(bool readOnly) {}

	bool Validate(ATDiskFSValidationReport& report);
	void Flush();

	uintptr FindFirst(uintptr key, ATDiskFSEntryInfo& info);
	bool FindNext(uintptr searchKey, ATDiskFSEntryInfo& info);
	void FindEnd(uintptr searchKey);

	void GetFileInfo(uintptr key, ATDiskFSEntryInfo& info);
	uintptr GetParentDirectory(uintptr dirKey);

	uintptr LookupFile(uintptr parentKey, const char *filename);

	void DeleteFile(uintptr key);
	void ReadFile(uintptr key, vdfastvector<uint8>& dst);
	void WriteFile(uintptr parentKey, const char *filename, const void *src, uint32 len);
	void RenameFile(uintptr key, const char *newFileName);

protected:
	struct DirEnt;

	bool IsSectorAllocated(uint32 sector) const;
	void AllocateSector(uint32 sector);
	void FreeSector(uint32 sector);

	static void WriteFileName(DirEnt& de, const char *filename);
	static bool IsValidFileName(const char *filename);

	static bool DecompressMode3(uint8 *dst, uint32 dstlen, const uint8 *src, uint32 srclen);
	static bool DecompressMode4(uint8 *dst, uint32 dstlen, const uint8 *src, uint32 srclen);
	static bool DecompressMode8(uint8 *dst, uint32 dstlen, const uint8 *src, uint32 srclen);

	VDFile	mFile;

	struct RawDirEnt {
		uint8	mId;					// must be 0x1A
		uint8	mCompressionMethod;
		uint8	mName[13];
		uint8	mCompressedSize[4];
		uint8	mDate[2];
		uint8	mTime[2];
		uint8	mCRC[2];
		uint8	mOriginalSize[4];
	};

	struct FindHandle {
		uint32	mIndex;
	};

	struct DirEnt {
		sint64	mPos;
		uint32	mCompressedSize;
		uint32	mOriginalSize;
		uint16	mDate;
		uint16	mTime;
		uint16	mCRC;
		uint8	mCompressionMethod;
		char	mName[13];
	};

	typedef vdfastvector<DirEnt> Directory;
	Directory mDirectory;

	ATARCCRC mCRC;
};

ATDiskFSARC::ATDiskFSARC() {
}

ATDiskFSARC::~ATDiskFSARC() {
}

void ATDiskFSARC::Init(const wchar_t *path) {
	mFile.open(path);

	sint64 fileSize = mFile.size();
	for(;;) {
		RawDirEnt rde;

		if (sizeof rde != mFile.readData(&rde, sizeof rde))
			break;

		if (rde.mId != 0x1A)
			break;

		// check for end of archive marker
		if (rde.mCompressionMethod == 0)
			break;

		DirEnt de;
		de.mPos = mFile.tell();
		de.mCompressedSize = VDReadUnalignedLEU32(rde.mCompressedSize);
		de.mOriginalSize = VDReadUnalignedLEU32(rde.mOriginalSize);
		de.mDate = VDReadUnalignedLEU16(rde.mDate);
		de.mTime = VDReadUnalignedLEU16(rde.mTime);
		de.mCRC = VDReadUnalignedLEU16(rde.mCRC);
		de.mCompressionMethod = rde.mCompressionMethod;
		memcpy(de.mName, rde.mName, 12);
		de.mName[12] = 0;

		if ((sint64)de.mPos + de.mCompressedSize > fileSize)
			break;

		mDirectory.push_back(de);

		mFile.skip(de.mCompressedSize);
	}
}

void ATDiskFSARC::GetInfo(ATDiskFSInfo& info) {
	info.mFSType = "Compressed archive";
	info.mFreeBlocks = 0;
	info.mBlockSize = 1;
}

bool ATDiskFSARC::Validate(ATDiskFSValidationReport& report) {
	report.mbBrokenFiles = false;
	report.mbBitmapIncorrect = false;

	return true;
}

void ATDiskFSARC::Flush() {
}

uintptr ATDiskFSARC::FindFirst(uintptr key, ATDiskFSEntryInfo& info) {
	if (key)
		return 0;

	FindHandle *h = new FindHandle;
	h->mIndex = 0;

	if (!FindNext((uintptr)h, info)) {
		delete h;
		return 0;
	}

	return (uintptr)h;
}

bool ATDiskFSARC::FindNext(uintptr searchKey, ATDiskFSEntryInfo& info) {
	FindHandle *h = (FindHandle *)searchKey;

	if (h->mIndex >= mDirectory.size())
		return false;

	GetFileInfo(++h->mIndex, info);
	return true;
}

void ATDiskFSARC::FindEnd(uintptr searchKey) {
	delete (FindHandle *)searchKey;
}

void ATDiskFSARC::GetFileInfo(uintptr key, ATDiskFSEntryInfo& info) {
	const DirEnt& de = mDirectory[key - 1];

	info.mFileName	= VDTextAToW(de.mName);
	info.mSectors	= de.mCompressedSize;
	info.mBytes		= de.mOriginalSize;
	info.mKey		= key;
	info.mbIsDirectory = false;
	info.mbDateValid = false;
	if (de.mDate | de.mTime) {
		info.mbDateValid = true;
		info.mDate.mYear = (de.mDate >> 9) + 1980;
		info.mDate.mMonth = (de.mDate >> 5) & 15;
		info.mDate.mDayOfWeek = 0;
		info.mDate.mDay = de.mDate & 31;
		info.mDate.mHour = (de.mTime >> 11) & 31;
		info.mDate.mMinute = (de.mTime >> 5) & 63;
		info.mDate.mSecond = (de.mTime & 31) * 2;
		info.mDate.mMilliseconds = 0;
	}
}

uintptr ATDiskFSARC::GetParentDirectory(uintptr dirKey) {
	return 0;
}

uintptr ATDiskFSARC::LookupFile(uintptr parentKey, const char *filename) {
	if (parentKey)
		return 0;

	uint32 n = mDirectory.size();
	for(uint32 i=0; i<n; ++i) {
		const DirEnt& de = mDirectory[i];

		if (!strcmp(de.mName, filename))
			return (uintptr)(i + 1);
	}

	return 0;
}

void ATDiskFSARC::DeleteFile(uintptr key) {
	throw ATDiskFSException(kATDiskFSError_ReadOnly);
}

void ATDiskFSARC::ReadFile(uintptr key, vdfastvector<uint8>& dst) {
	VDASSERT(key >= 1 && key <= mDirectory.size());

	const uint8 fileId = (uint8)key - 1;
	const DirEnt& de = mDirectory[fileId];

	if (de.mOriginalSize > 16777216)
		throw ATDiskFSException(kATDiskFSError_FileTooLarge);

	dst.resize(de.mOriginalSize);

	mFile.seek(de.mPos);

	// modes 1 and 2 are uncompressed
	if (de.mCompressionMethod == 1 || de.mCompressionMethod == 2) {
		if (de.mCompressedSize != de.mOriginalSize)
			throw ATDiskFSException(kATDiskFSError_CorruptedFileSystem);

		mFile.read(dst.data(), de.mCompressedSize);
	} else {
		vdfastvector<uint8> tmp;
		tmp.resize(de.mCompressedSize);
		mFile.read(tmp.data(), de.mCompressedSize);

		if (de.mCompressionMethod == 3) {
			// mode 3 is packed (RLE)
			if (!DecompressMode3(dst.data(), de.mOriginalSize, tmp.data(), de.mCompressedSize))
				throw ATDiskFSException(kATDiskFSError_DecompressionError);
		} else if (de.mCompressionMethod == 4) {
			// mode 4 is squeezed (RLE + Huffman)
			if (!DecompressMode4(dst.data(), de.mOriginalSize, tmp.data(), de.mCompressedSize))
				throw ATDiskFSException(kATDiskFSError_DecompressionError);
		} else if (de.mCompressionMethod == 8) {
			// mode 8 is crunched (RLE + variable length LZW)
			if (!DecompressMode8(dst.data(), de.mOriginalSize, tmp.data(), de.mCompressedSize))
				throw ATDiskFSException(kATDiskFSError_DecompressionError);
		} else {
			throw ATDiskFSException(kATDiskFSError_UnsupportedCompressionMode);
		}
	}

	uint16 crc = mCRC.Compute(dst.data(), dst.size());

	if (crc != de.mCRC)
		throw ATDiskFSException(kATDiskFSError_CRCError);
}

void ATDiskFSARC::WriteFile(uintptr parentKey, const char *filename, const void *src, uint32 len) {
	throw ATDiskFSException(kATDiskFSError_ReadOnly);
}

void ATDiskFSARC::RenameFile(uintptr key, const char *filename) {
	throw ATDiskFSException(kATDiskFSError_ReadOnly);
}

// Packed (mode 3) encoding is a simple RLE encoding. All bytes are literal bytes except
// for the 90 hex code. The second byte indicates the number of times to repeat the last
// byte minus one, except for a 90 90 sequence which is an escaped 90.
//
bool ATDiskFSARC::DecompressMode3(uint8 *dst, uint32 dstlen, const uint8 *src0, uint32 srclen) {
	const uint8 *src = src0;

	uint8 prevoutput = 0;
	while(dstlen) {
		if (!srclen--)
			return false;

		const uint8 c = *src++;

		if (c == 0x90) {
			if (!srclen--)
				return false;

			const uint8 d = *src++;
			if (d == 0) {
				prevoutput = 0x90;

				if (!dstlen--)
					return false;

				*dst++ = 0x90;
			} else {
				uint32 count = d - 1;

				if (dstlen < count)
					return false;

				dstlen -= count;

				while(count--)
					*dst++ = prevoutput;
			}
		} else {
			prevoutput = c;

			if (!dstlen--)
				return false;

			*dst++ = c;
		}
	}

	return true;
}

namespace {
	struct TreeNode {
		sint16	mNext[2];
	};
}

// Mode 4 is Squeezing, which consists of RLE + Huffman encoding.
//
//	uint16		codeCount;
//	TreeNode[n]	tree;
//		sint16		zero;
//		sint16		one;
//	byte[n]		rawData;
//
// Huffman tree nodes have non-negative values for branches and negative values for
// leaves. The leaf values are (-1)-c for a value of c. The RLE encoding underneath
// is the same as the Packed mode (mode 3).
bool ATDiskFSARC::DecompressMode4(uint8 *dst, uint32 dstlen, const uint8 *src0, uint32 srclen) {
	// decompress tree
	const uint8 *src = src0;

	if (srclen < 2)
		return false;

	uint32 numNodes = VDReadUnalignedLEU16(src);

	srclen -= 2;
	src += 2;

	if (srclen < numNodes * 4)
		return false;

	srclen -= numNodes * 4;

	vdfastvector<TreeNode> treeNodes(numNodes);
	for(uint32 i = 0; i < numNodes; ++i) {
		TreeNode& node = treeNodes[i];
		node.mNext[0] = VDReadUnalignedLES16(src);
		node.mNext[1] = VDReadUnalignedLES16(src+2);
		src += 4;
	}

	// validate tree
	{
		vdfastvector<int> height(treeNodes.size(), -1);
		vdfastvector<int> traversalStack;
		height[0] = 0;

		int pos = 0;
		for(;;) {
			const TreeNode& node = treeNodes[pos];
			int nodeHeight = height[pos];

			if (node.mNext[0] >= 0) {
				if (height[node.mNext[0]] >= 0)
					return false;

				height[node.mNext[0]] = nodeHeight + 1;
				traversalStack.push_back(node.mNext[0]);
			}

			if (node.mNext[1] >= 0) {
				if (height[node.mNext[1]] >= 0)
					return false;

				height[node.mNext[1]] = nodeHeight + 1;
				traversalStack.push_back(node.mNext[1]);
			}

			if (traversalStack.empty())
				break;

			pos = traversalStack.back();
			traversalStack.pop_back();
		}
	}

	// decode tokens
	uint8 prevoutput = 0;
	size_t accum = 0;
	sint8 bitsleft = 0;
	bool byte2 = false;
	while(dstlen) {
		int pos = 0;
		while(pos >= 0) {
			if (bitsleft >= 0) {
				if (!srclen--)
					return false;

				bitsleft = (sint8)0xff;
				accum = *src++;
			}

			bitsleft += bitsleft;

			pos = treeNodes[pos].mNext[accum & 1];
			accum >>= 1;
		}

		const int c = (uint8)~pos;

		if (byte2) {
			if (c == 0) {
				prevoutput = 0x90;

				if (!dstlen--)
					return false;

				*dst++ = 0x90;
			} else {
				uint32 count = c - 1;

				if (dstlen < count)
					return false;

				dstlen -= count;

				while(count--)
					*dst++ = prevoutput;
			}

			byte2 = false;
		} else if (c == 0x90) {
			byte2 = true;
		} else {
			prevoutput = c;

			if (!dstlen--)
				return false;

			*dst++ = c;
		}
	}

	return true;
}

// Mode 8 is Crunched, which is variable length LZW on top of RLE (Packed) encoding. The
// first byte is the maximum code length in bits, followed by the bitstream. The bitstream
// is processed LSB first with the codes being stored in natural bit order. The dictionary
// starts with 257 codes, with the first 256 codes being literal bytes and 100 being the
// flush code. The dictionary becomes full at 12 bits (4096 entries), after which codes
// are discarded until the next flush.
//
// The first code in the bitstream is special in that it doesn't add a dictionary entry,
// which we optimize here by adding the never used 100 entry instead. It still takes 9 bits
// to encode even though there's no point in encoding a flush code.
//
// The flush operation is a bit weird due to the way that the bitstream is read, which dates
// all the way back to the original Unix compress.c. The stream is read in blocks of 8 codes
// each, which means that each block is always byte aligned. Ordinarily this doesn't matter
// except for the flush, where the extra bytes in the block are discarded. After the flush,
// decoding is restarted with the special first code at 9 bits again.
//
// Underneath the LZW encoding is the same RLE encoding used by Packed (mode 3).
//
bool ATDiskFSARC::DecompressMode8(uint8 *dst, uint32 dstlen, const uint8 *src0, uint32 srclen) {
	struct Entry {
		sint16 mPrevLink;
		uint8 mFirstChar;
		uint8 mLastChar;
	} dict[4096];
	uint8 tmp[4096];
	uint8 *tmpend = tmp + sizeof(tmp)/sizeof(tmp[0]);

	int prevoutput = 0;
	bool byte2 = false;

	for(int i=0; i<256; ++i) {
		Entry& e = dict[i];
		e.mPrevLink = -1;
		e.mFirstChar = i;
		e.mLastChar = i;
	}

	int bits = 9;
	int accum = 0;
	int accumbits = 0;
	int n = 256;

	// First byte nominally contains $0C (max codelen?).
	const uint8 *src = src0;
	if (!srclen--)
		return false;

	if (*src++ != 0x0C)
		return false;

	int lastCode = 0;
	int codesRead = 0;
	while(dstlen) {
		int code = 0;

		for(int i = 0; i < bits; ++i) {
			if (!accumbits) {
				if (!srclen--)
					return false;

				accum = *src++;
				accumbits = 8;
			}

			if (accum & 1)
				code += (1 << bits);

			accum >>= 1;
			--accumbits;
			code >>= 1;
		}

		++codesRead;

		if (code > n)
			return false;

		if (code == 256) {
			// align to next dword boundary within compressed stream
			uint32 align = ((-codesRead & 7)*bits) >> 3;

			if (srclen < align)
				return false;

			srclen -= align;
			src += align;

			// reset code table
			n = 256;
			bits = 9;
			lastCode = 0;
			accumbits = 0;
			accum = 0;
			codesRead = 0;
			continue;
		}

		if (n < 4096) {
			Entry& e = dict[n];

			e.mFirstChar = dict[lastCode].mFirstChar;
			if (code == n)
				e.mLastChar = e.mFirstChar;
			else
				e.mLastChar = dict[code].mFirstChar;

			e.mPrevLink = lastCode;

			++n;
			if (n >= (1 << bits) && bits < 12)
				++bits;
		}

		// unravel RLE bytes for desired code
		uint8 *tmpstart = tmpend;

		for(int link = code; link >= 0; link = dict[link].mPrevLink)
			*--tmpstart = dict[link].mLastChar;

		// decompress RLE bytes
		while(tmpstart != tmpend) {
			const uint8 c = *tmpstart++;

			if (byte2) {
				if (c == 0) {
					prevoutput = 0x90;

					if (!dstlen--)
						return false;

					*dst++ = 0x90;
				} else {
					uint32 count = c - 1;

					if (dstlen < count)
						return false;

					dstlen -= count;

					while(count--)
						*dst++ = prevoutput;
				}

				byte2 = false;
			} else {
				if (c == 0x90)
					byte2 = true;
				else {
					prevoutput = c;

					if (!dstlen--)
						return false;
					*dst++ = c;
				}
			}
		}
		
		lastCode = code;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////

IATDiskFS *ATDiskMountImageARC(const wchar_t *path) {
	ATDiskFSARC *fs = new ATDiskFSARC;

	fs->Init(path);

	return fs;
}
