#include "stdafx.h"
#include <time.h>
#include <vd2/system/binary.h>
#include <vd2/system/bitmath.h>
#include <vd2/system/hash.h>
#include <vd2/system/strutil.h>
#include <vd2/system/vdalloc.h>
#include "diskfs.h"
#include "diskimage.h"
#include "diskfssdx2util.h"

class ATDiskFSSDX2 final : public IATDiskFS {
public:
	ATDiskFSSDX2();
	~ATDiskFSSDX2();

	void Init(IATDiskImage *image, bool readOnly);
	void InitNew(IATDiskImage *image, const char *volNameHint);

public:
	void GetInfo(ATDiskFSInfo& info);

	bool IsReadOnly() { return mbReadOnly; }
	void SetReadOnly(bool readOnly);

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
	uintptr WriteFile(uintptr parentKey, const char *filename, const void *src, uint32 len);
	void RenameFile(uintptr key, const char *newFileName);
	void SetFileTimestamp(uintptr key, const VDExpandedDate& date);

	void CreateDir(uintptr parentKey, const char *filename);

protected:
	struct DirEnt;
	struct FileHandle;

	void	GetFileInfo(const uint8 *dirEnt, uintptr key, ATDiskFSEntryInfo& info);
	uintptr LookupFileByDirHandle(uint32 sectorMapStart, FileHandle& dh, const char *filename, int *freeOffset);

	uintptr	WriteEntry(uintptr parentKey, const char *filename, const void *src, uint32 len, bool isDir);

	void	OpenFile(FileHandle& fh, uint32 sectorMapStart, uintptr fileKey = 0);
	void	SeekFile(FileHandle& fh, uint32 offset, bool allowExtend);
	void	ReadFile(FileHandle& fh, void *dst, uint32 len);
	void	WriteFile(FileHandle& fh, const void *dst, uint32 len);
	void	FlushFile(FileHandle& fh);

	uint32	GetDirectorySectorMap(uintptr sec);

	void	ReadSector(uint32 sector, void *buf);
	void	WriteSector(uint32 sector, void *buf);

	bool	IsSectorAllocated(uint32 sector);
	uint32	AllocateSector();
	void	AllocateSector(uint32 sector);
	void	FreeSector(uint32 sector);
	void	LoadBitmapSector(uint32 sector);
	void	FlushBitmapSector();

	void	MarkVolumeChanged();

	static void WriteFileName(uint8 fn[11], const char *filename);
	static bool IsValidFileName(const char *filename);

	IATDiskImage *mpImage;
	uint32	mBitmapStartSector;
	int		mBitmapSectorShift;
	uint32	mTotalSectors;
	uint32	mSectorSize;
	int		mSectorShift;
	uint32	mSectorsPerMapPage;
	bool	mbDirty;
	bool	mbSuperBlockDirty;
	bool	mbReadOnly;
	uint32	mFreeSectors;
	uint32	mLastAllocSector;

	struct FileHandle {
		uint32	mCurrentDataSector;
		uint32	mCurrentMapSector;
		uint32	mDataOffset;
		uint32	mSectorOffset;
		uint32	mDataSector;
		uint32	mMapSectorOffset;
		uint32	mSectorMapStart;
		uint32	mFileSize;
		bool	mbDataSectorValid;
		bool	mbDataBufferDirty;
		bool	mbMapBufferDirty;
		bool	mbFileSizeDirty;

		uint8	mDataBuffer[512];
		uint8	mMapBuffer[512];
	};

	struct FindHandle {
		FileHandle	mDirectory;

		uintptr	mBaseKey;
		uint32	mPos;
		uint32	mSize;
	};

	uint8	mSuperBlock[512];
	uint8	mSectorBuffer[512];

	mutable uint32	mBitmapSector;
	mutable bool	mbBitmapSectorDirty;
	mutable uint8	mBitmapBuffer[512];

	vdfastvector<uint8> mTempSectorMap;
};

ATDiskFSSDX2::ATDiskFSSDX2() {
}

ATDiskFSSDX2::~ATDiskFSSDX2() {
}

void ATDiskFSSDX2::Init(IATDiskImage *image, bool readOnly) {
	mpImage = image;
	mbDirty = false;
	mbReadOnly = readOnly;
	mSectorSize = image->GetSectorSize();
	mSectorShift = VDFindHighestSetBit(mSectorSize);
	mSectorsPerMapPage = (mSectorSize - 4) >> 1;
	mbBitmapSectorDirty = false;
	mBitmapSector = 0;
	mLastAllocSector = 1;

	// read superblock
	mpImage->ReadVirtualSector(0, mSuperBlock, mpImage->GetSectorSize(0));
	mbSuperBlockDirty = false;

	mTotalSectors = VDReadUnalignedLEU16(mSuperBlock + 11);
	mFreeSectors = VDReadUnalignedLEU16(mSuperBlock + 13);
	mBitmapStartSector = VDReadUnalignedLEU16(mSuperBlock + 16);
	mBitmapSectorShift = mSectorShift + 3;
}

void ATDiskFSSDX2::InitNew(IATDiskImage *image, const char *volNameHint) {
	mpImage = image;
	mbDirty = true;
	mbReadOnly = false;
	mbBitmapSectorDirty = false;
	mBitmapSector = 0;
	mLastAllocSector = 1;

	// init disk geometry
	const ATDiskGeometryInfo geo = image->GetGeometry();

	mSectorSize = geo.mSectorSize;
	mSectorShift = VDFindHighestSetBit(mSectorSize);
	mSectorsPerMapPage = (mSectorSize - 4) >> 1;

	mTotalSectors = geo.mTotalSectorCount;
	mFreeSectors = 0;	// will be adjusted later when we 'free' sectors in the bitmap
	mBitmapSectorShift = mSectorShift + 3;

	// init disk layout
	const uint32 bootSectorCount = 3;
	const uint32 bitmapSectorCount = (mTotalSectors >> mBitmapSectorShift) + 1;	// (!!) no -1 because sector 0 bit is unused
	const uint32 specialSectorCount = bootSectorCount + bitmapSectorCount + 2;
	const uint32 rootDirMapSector = specialSectorCount - 1;
	const uint32 rootDirDataSector = specialSectorCount;

	mBitmapStartSector = bootSectorCount + 1;

	// init superblock / first boot sector
	memcpy(mSuperBlock, kATSDFSBootSector0, sizeof kATSDFSBootSector0);
	mbSuperBlockDirty = true;

	VDWriteUnalignedLEU16(mSuperBlock + 9, rootDirMapSector);
	VDWriteUnalignedLEU16(mSuperBlock + 11, mTotalSectors);
	VDWriteUnalignedLEU16(mSuperBlock + 13, mFreeSectors);
	mSuperBlock[15] = bitmapSectorCount;
	VDWriteUnalignedLEU16(mSuperBlock + 16, mBitmapStartSector);
	mSuperBlock[30] = geo.mTrackCount;
	mSuperBlock[31] = mSectorSize;
	VDWriteUnalignedLEU16(mSuperBlock + 33, mSectorSize);
	VDWriteUnalignedLEU16(mSuperBlock + 35, mSectorsPerMapPage);

	// set volume name, VSN, and random ID
	uint64 volHash64 = VDGetCurrentDate().mTicks;
	memset(&mSuperBlock[22], ' ', 8);

	if (volNameHint) {
		volHash64 += VDHashString32(volNameHint);

		int volNameLen = 0;
		const char *s = volNameHint;

		while(volNameLen < 8) {
			char c = (uint8)*s++;

			if (!c)
				break;

			// convert lowercase to uppercase
			if ((unsigned)(c - 'a') < 26)
				c &= 0xdf;

			// stop if we hit a period
			if (c == '.')
				break;

			// skip char if not alphanumeric
			if ((unsigned)(c - 'A') >= 26 && (unsigned)(c - '0') >= 10)
				continue;

			mSuperBlock[22 + volNameLen++] = (uint8)c;
		}
	}

	if (mSuperBlock[22] == ' ') {
		// put in default if name is empty
		memcpy(&mSuperBlock[22], "NEWDISK ", 8);
	}

	const uint16 volHash16 = (uint16)((volHash64 * 0x0001000100010001ull) >> 56);

	VDWriteUnalignedLEU16(&mSuperBlock[38], volHash16);	// VSN + volume random ID

	// init second boot sector
	memset(mSectorBuffer, 0, sizeof mSectorBuffer);
	memcpy(mSectorBuffer, kATSDFSBootSector1, sizeof kATSDFSBootSector1);
	mpImage->WriteVirtualSector(1, mSectorBuffer, mpImage->GetSectorSize(1));

	// clear bitmap sectors
	memset(mSectorBuffer, 0, sizeof mSectorBuffer);

	for(uint32 i=0; i<bitmapSectorCount; ++i)
		WriteSector(mBitmapStartSector + i, mSectorBuffer);

	// free non boot/bitmap/directory sectors
	for(uint32 i=specialSectorCount+1; i<=mTotalSectors; ++i)
		FreeSector(i);

	// initialize root directory sector map
	memset(mSectorBuffer, 0, sizeof mSectorBuffer);
	VDWriteUnalignedLEU16(&mSectorBuffer[4], rootDirDataSector);
	WriteSector(rootDirMapSector, mSectorBuffer);

	// initialize root directory
	memset(mSectorBuffer, 0, sizeof mSectorBuffer);
	mSectorBuffer[3] = 23;		// directory length = one entry (23 bytes)
	memcpy(&mSectorBuffer[6], "MAIN       ", 11);

	WriteSector(rootDirDataSector, mSectorBuffer);
}

void ATDiskFSSDX2::GetInfo(ATDiskFSInfo& info) {
	info.mFSType = "SpartaDOS X";
	info.mFreeBlocks = mFreeSectors;
	info.mBlockSize = mSectorSize;
}

void ATDiskFSSDX2::SetReadOnly(bool readOnly) {
	mbReadOnly = readOnly;
}

bool ATDiskFSSDX2::Validate(ATDiskFSValidationReport& report) {
	bool errorsFound = false;

	report.mbBrokenFiles = false;
	report.mbBitmapIncorrect = false;
	report.mbOpenWriteFiles = false;

	uint32 freeSectors = 0;

	for(uint32 i = 1; i <= mTotalSectors; ++i)
		if (!IsSectorAllocated(i))
			++freeSectors;

	if (freeSectors != mFreeSectors) {
		report.mbBitmapIncorrect = true;
		errorsFound = true;
	}

	return !errorsFound;
}

void ATDiskFSSDX2::Flush() {
	if (!mbDirty || mbReadOnly)
		return;

	FlushBitmapSector();

	if (mbSuperBlockDirty) {
		VDWriteUnalignedLEU16(mSuperBlock + 13, mFreeSectors);
		mpImage->WriteVirtualSector(0, mSuperBlock, mpImage->GetSectorSize(0));
		mbSuperBlockDirty = false;
	}

	mbDirty = false;
}

uintptr ATDiskFSSDX2::FindFirst(uintptr key, ATDiskFSEntryInfo& info) {
	uint32 sectorMapStart = GetDirectorySectorMap(key);

	FindHandle *h = new FindHandle;

	OpenFile(h->mDirectory, sectorMapStart);

	uint8 dirHdr[23];
	ReadFile(h->mDirectory, dirHdr, 23);

	h->mPos = 23;
	h->mSize = dirHdr[3] + ((uint32)dirHdr[4] << 8) + ((uint32)dirHdr[5] << 16);
	h->mBaseKey = sectorMapStart << 16;

	if (!FindNext((uintptr)h, info)) {
		delete h;
		return 0;
	}

	return (uintptr)h;
}

bool ATDiskFSSDX2::FindNext(uintptr searchKey, ATDiskFSEntryInfo& info) {
	FindHandle *h = (FindHandle *)searchKey;

	while(h->mPos < h->mSize) {
		uint8 rawde[23];

		ReadFile(h->mDirectory, rawde, 23);

		if (!(rawde[0] & 0x08)) {
			h->mPos += 23;
			continue;
		}

		GetFileInfo(rawde, h->mBaseKey + h->mPos / 23, info);

		h->mPos += 23;
		return true;
	}

	return false;
}

void ATDiskFSSDX2::FindEnd(uintptr searchKey) {
	delete (FindHandle *)searchKey;
}

void ATDiskFSSDX2::GetFileInfo(uintptr key, ATDiskFSEntryInfo& info) {
	uint32 dirSectorMap = key >> 16;
	uint32 fileIndex = key & 0xffff;

	FileHandle fh;
	OpenFile(fh, dirSectorMap);
	SeekFile(fh, 23 * fileIndex, false);

	uint8 dirEnt[23];
	ReadFile(fh, dirEnt, 23);

	GetFileInfo(dirEnt, key, info);
}

uintptr ATDiskFSSDX2::GetParentDirectory(uintptr dirKey) {
	const uint32 dirSectorMapStart = dirKey >> 16;

	FileHandle fh;
	OpenFile(fh, dirSectorMapStart);

	uint8 dirEnt[23];
	ReadFile(fh, dirEnt, 23);

	const uint32 parentSectorMapStart = VDReadUnalignedLEU16(dirEnt + 1);

	// check for root
	if (!parentSectorMapStart)
		return 0;

	// search parent directory
	OpenFile(fh, parentSectorMapStart);
	ReadFile(fh, dirEnt, 23);

	uint32 dirSize = dirEnt[3] + ((uint32)dirEnt[4] << 8) + ((uint32)dirEnt[5] << 16);

	uint32 pos = 23;
	uint32 index = 1;

	while(pos < dirSize) {
		ReadFile(fh, dirEnt, 23);

		if (VDReadUnalignedLEU16(dirEnt + 1) == dirSectorMapStart)
			return (parentSectorMapStart << 16) + index;

		++index;
		pos += 23;
	}

	return 0;
}

void ATDiskFSSDX2::GetFileInfo(const uint8 *rawde, uintptr key, ATDiskFSEntryInfo& info) {
	const uint8 *fnstart = rawde + 6;
	const uint8 *fnend = fnstart + 8;

	while(fnend != fnstart && fnend[-1] == 0x20)
		--fnend;

	const uint8 *extstart = rawde + 14;
	const uint8 *extend = extstart + 3;

	while(extend != extstart && extend[-1] == 0x20)
		--extend;

	info.mFileName.clear();
	while(fnstart != fnend)
		info.mFileName += *fnstart++;

	if (extstart != extend) {
		info.mFileName += '.';

		while(extstart != extend)
			info.mFileName += *extstart++;
	}

	info.mBytes		= rawde[3] + ((uint32)rawde[4] << 8) + ((uint32)rawde[5] << 16);
	info.mSectors	= info.mBytes ? ((info.mBytes - 1) >> mSectorShift) + 1 : 0;
	info.mKey		= key;
	info.mbIsDirectory = (rawde[0] & 0x20) != 0;
	info.mbDateValid = false;

	if (rawde[17] |
		rawde[18] |
		rawde[19] |
		rawde[20] |
		rawde[21] |
		rawde[22])
	{
		info.mbDateValid = true;
		info.mDate.mDay = rawde[17];
		info.mDate.mMonth = rawde[18];
		info.mDate.mYear = rawde[19] >= 50 ? rawde[19] + 1900 : rawde[19] + 2000;
		info.mDate.mHour = rawde[20];
		info.mDate.mMinute = rawde[21];
		info.mDate.mSecond = rawde[22];
		info.mDate.mDayOfWeek = 0;
		info.mDate.mMilliseconds = 0;
	}
}

uintptr ATDiskFSSDX2::LookupFile(uintptr parentKey, const char *filename) {
	if (!IsValidFileName(filename))
		return 0;

	uint32 sectorMapStart = GetDirectorySectorMap(parentKey);

	FileHandle fh;
	OpenFile(fh, sectorMapStart);

	return LookupFileByDirHandle(sectorMapStart, fh, filename, NULL);
}

uintptr ATDiskFSSDX2::LookupFileByDirHandle(uint32 sectorMapStart, FileHandle& fh, const char *filename, int *freeOffset) {
	uint8 dirEnt[23];
	SeekFile(fh, 0, false);
	ReadFile(fh, dirEnt, 23);

	uint32 dirSize = dirEnt[3] + ((uint32)dirEnt[4] << 8) + ((uint32)dirEnt[5] << 16);

	uint32 pos = 23;
	uint32 index = 1;

	uint8 fn[11];
	WriteFileName(fn, filename);

	if (freeOffset)
		*freeOffset = 0;

	while(pos < dirSize) {
		ReadFile(fh, dirEnt, 23);

		if (dirEnt[0] & 0x08) {
			if (!memcmp(dirEnt + 6, fn, 11))
				return (sectorMapStart << 16) + index;
		} else if (freeOffset && !*freeOffset)
			*freeOffset = pos;

		// zero indicates end of directory
		if (!dirEnt[0])
			break;

		++index;
		pos += 23;
	}

	return 0;
}

void ATDiskFSSDX2::DeleteFile(uintptr key) {
	if (mbReadOnly)
		throw ATDiskFSException(kATDiskFSError_ReadOnly);

	FileHandle fh;
	OpenFile(fh, key >> 16);

	uint32 dirOffset = (key & 0xffff)*23;
	SeekFile(fh, dirOffset, false);

	uint8 dirEnt[23];
	ReadFile(fh, dirEnt, 23);

	uint32 fileSectorMap = VDReadUnalignedLEU16(dirEnt + 1);
	uint32 len = dirEnt[3] + ((uint32)dirEnt[4] << 8) + ((uint32)dirEnt[5] << 16);
	uint32 sectors = (len + (mSectorSize - 1)) >> mSectorShift;

	vdfastvector<uint32> sectorsToFree;

	while(sectors) {
		uint32 tc = mSectorsPerMapPage;

		if (tc > sectors)
			tc = sectors;

		sectorsToFree.push_back(fileSectorMap);
		ReadSector(fileSectorMap, mSectorBuffer);

		for(uint32 i=0; i<tc; ++i) {
			uint16 sector = VDReadUnalignedLEU16(mSectorBuffer + 4 + i*2);

			if (sector)
				sectorsToFree.push_back(sector);
		}

		sectors -= tc;
		
		fileSectorMap = VDReadUnalignedLEU16(mSectorBuffer);
	}

	std::sort(sectorsToFree.begin(), sectorsToFree.end());
	if (!sectorsToFree.empty() && !sectorsToFree.front())
		throw ATDiskFSException(kATDiskFSError_CorruptedFileSystem);

	// free directory entry
	dirEnt[0] |= 0x10;		// set deleted flag
	dirEnt[0] &= ~0x08;		// clear in use flag
	dirEnt[1] = 0;			// clear sector chain
	dirEnt[2] = 0;

	SeekFile(fh, dirOffset, false);
	WriteFile(fh, dirEnt, 23);
	FlushFile(fh);

	// begin freeing sectors
	for(vdfastvector<uint32>::const_iterator it(sectorsToFree.begin()), itEnd(sectorsToFree.end());
		it != itEnd;
		++it)
	{
		const uint32 sectorToFree = *it;

		if (!IsSectorAllocated(sectorToFree))
			throw ATDiskFSException(kATDiskFSError_CorruptedFileSystem);

		FreeSector(sectorToFree);
	}

	MarkVolumeChanged();
}

void ATDiskFSSDX2::ReadFile(uintptr key, vdfastvector<uint8>& dst) {
	FileHandle fh;
	OpenFile(fh, key >> 16);
	SeekFile(fh, (key & 0xffff)*23, false);

	uint8 dirEnt[23];
	ReadFile(fh, dirEnt, 23);

	uint32 fileSectorMap = VDReadUnalignedLEU16(dirEnt + 1);
	uint32 len = dirEnt[3] + ((uint32)dirEnt[4] << 8) + ((uint32)dirEnt[5] << 16);

	dst.resize(len);
	OpenFile(fh, fileSectorMap);
	ReadFile(fh, dst.data(), len);
}

uintptr ATDiskFSSDX2::WriteFile(uintptr parentKey, const char *filename, const void *src, uint32 len) {
	return WriteEntry(parentKey, filename, src, len, false);
}

void ATDiskFSSDX2::CreateDir(uintptr parentKey, const char *filename) {
	if (mbReadOnly)
		throw ATDiskFSException(kATDiskFSError_ReadOnly);

	if (!IsValidFileName(filename))
		throw ATDiskFSException(kATDiskFSError_InvalidFileName);

	uint8 dirEnt[23] = {0};

	dirEnt[3] = 23;

	WriteFileName(dirEnt + 6, filename);

	WriteEntry(parentKey, filename, dirEnt, 23, true);
}

uintptr ATDiskFSSDX2::WriteEntry(uintptr parentKey, const char *filename, const void *src, uint32 len, bool isDir) {
	if (mbReadOnly)
		throw ATDiskFSException(kATDiskFSError_ReadOnly);

	if (!IsValidFileName(filename))
		throw ATDiskFSException(kATDiskFSError_InvalidFileName);

	if (len >= 0x1000000)
		throw ATDiskFSException(kATDiskFSError_FileTooLarge);

	uint32 dirSectorMap = GetDirectorySectorMap(parentKey);

	// if this is a subdirectory, copy the entry and fill in the parent directory sector map
	uint8 subDirData[23];
	if (isDir) {
		VDASSERT(len == 23);

		memcpy(subDirData, src, 23);
		src = subDirData;
		len = 23;

		VDWriteUnalignedLEU16(subDirData + 1, dirSectorMap);
	}

	FileHandle fh;
	OpenFile(fh, dirSectorMap);

	uint8 dirEnt[23];
	ReadFile(fh, dirEnt, 23);

	uint32 dirLen = dirEnt[3] + ((uint32)dirEnt[4] << 8) + ((uint32)dirEnt[5] << 16);

	int dirEntOffset = 0;
	if (LookupFileByDirHandle(dirSectorMap, fh, filename, &dirEntOffset))
		throw ATDiskFSException(kATDiskFSError_FileExists);

	uint32 dataSectorCount = (len + (mSectorSize - 1)) >> mSectorShift;
	uint32 mapSectorCount = (dataSectorCount + mSectorsPerMapPage - 1) / mSectorsPerMapPage;
	
	const bool extendDir = (dirEntOffset == 0);
	uint32 dirSectorCount = 0;
	
	if (extendDir) {
		// check if we need to extend the directory by a sector
		if ((dirLen & (mSectorSize - 1)) + 23 > mSectorSize) {
			dirSectorCount = 1;

			// check if we need to add a sector for the sector map
			if (((dirLen >> mSectorShift) + 1) % mSectorsPerMapPage == 0)
				++dirSectorCount;
		}

		dirEntOffset = dirLen;
	}

	uint32 totalAllocCount = dataSectorCount + mapSectorCount + dirSectorCount;

	if (mFreeSectors < totalAllocCount)
		throw ATDiskFSException(kATDiskFSError_DiskFull);

	uint8 dirEnt2[23] = {0x00};
	const uintptr fileKey = (dirSectorMap << 16) + (dirEntOffset / 23);

	if (extendDir) {
		// precreate the new directory entry
		SeekFile(fh, dirEntOffset, true);
		WriteFile(fh, dirEnt2, 23);

		// rewrite directory length (safe write)
		dirLen += 23;
		dirEnt[3] = (uint8)dirLen;
		dirEnt[4] = (uint8)(dirLen >> 8);
		dirEnt[5] = (uint8)(dirLen >> 16);
		SeekFile(fh, 0, false);
		WriteFile(fh, dirEnt, 23);
		FlushFile(fh);
	}

	// write file contents
	FileHandle fh2;
	OpenFile(fh2, 0, fileKey);
	WriteFile(fh2, src, len);
	FlushFile(fh2);

	// update directory entry
	dirEnt2[0] = isDir ? 0x28 : 0x08;
	dirEnt2[1] = (uint8)(fh2.mSectorMapStart     );			// sector map lo
	dirEnt2[2] = (uint8)(fh2.mSectorMapStart >> 8);			// sector map hi
	dirEnt2[3] = (uint8)(fh2.mFileSize >>  0);	// length lo
	dirEnt2[4] = (uint8)(fh2.mFileSize >>  8);	// length med
	dirEnt2[5] = (uint8)(fh2.mFileSize >> 16);	// length hi
	WriteFileName(dirEnt2 + 6, filename);

	time_t t;
	time(&t);

	const tm *tmvp = localtime(&t);

	if (tmvp) {
		dirEnt2[17] = tmvp->tm_mday;
		dirEnt2[18] = tmvp->tm_mon + 1;
		dirEnt2[19] = tmvp->tm_year % 100;
		dirEnt2[20] = tmvp->tm_hour;
		dirEnt2[21] = tmvp->tm_min;
		dirEnt2[22] = tmvp->tm_sec;
	} else {
		memcpy(dirEnt2 + 17, 0, 6);
	}

	SeekFile(fh, dirEntOffset, true);
	WriteFile(fh, dirEnt2, 23);
	FlushFile(fh);

	MarkVolumeChanged();

	return fileKey;
}

void ATDiskFSSDX2::RenameFile(uintptr key, const char *filename) {
	if (mbReadOnly)
		throw ATDiskFSException(kATDiskFSError_ReadOnly);

	if (!IsValidFileName(filename))
		throw ATDiskFSException(kATDiskFSError_InvalidFileName);

	FileHandle dh;
	OpenFile(dh, key >> 16);
	uintptr conflictingKey = LookupFileByDirHandle(key >> 16, dh, filename, NULL);

	if (conflictingKey == key)
		return;

	if (conflictingKey)
		throw ATDiskFSException(kATDiskFSError_FileExists);

	uint32 dirOffset = 23*(key & 0xffff);
	SeekFile(dh, dirOffset, false);

	uint8 dirEnt[23];
	ReadFile(dh, dirEnt, 23);
	WriteFileName(&dirEnt[6], filename);

	SeekFile(dh, dirOffset, false);
	WriteFile(dh, dirEnt, 23);
	FlushFile(dh);

	// if this is a subdirectory, we need to also update its entry
	if (dirEnt[0] & 0x20) {
		FileHandle sdh;

		const uint32 subDirSectorMap = VDReadUnalignedLEU16(dirEnt + 1);

		OpenFile(sdh, subDirSectorMap);

		uint8 subDirEnt[23];
		ReadFile(sdh, subDirEnt, 23);
		SeekFile(sdh, 0, false);
		memcpy(subDirEnt + 6, dirEnt + 6, 11);
		WriteFile(sdh, subDirEnt, 23);

		FlushFile(sdh);
	}

	mbDirty = true;
}

void ATDiskFSSDX2::SetFileTimestamp(uintptr key, const VDExpandedDate& date) {
	FileHandle dh;

	const uint32 dirOffset = 23*(key & 0xffff);
	OpenFile(dh, key >> 16);
	SeekFile(dh, dirOffset, false);

	uint8 newDate[6];
	newDate[0] = date.mDay;
	newDate[1] = date.mMonth;
	newDate[2] = date.mYear % 100;
	newDate[3] = date.mHour;
	newDate[4] = date.mMinute;
	newDate[5] = date.mSecond;

	uint8 dirEnt[23];
	ReadFile(dh, dirEnt, 23);

	// check if timestamp is actually changing
	if (memcmp(dirEnt + 17, newDate, 6)) {
		// patch in the new timestamp
		memcpy(dirEnt + 17, newDate, 6);

		// update directory entry
		SeekFile(dh, dirOffset, false);
		WriteFile(dh, dirEnt, 23);
		FlushFile(dh);

		// dirty the disk
		MarkVolumeChanged();
	}
}

void ATDiskFSSDX2::OpenFile(FileHandle& fh, uint32 sectorMapStart, uintptr fileKey) {
	if (!sectorMapStart && fileKey) {
		sectorMapStart = AllocateSector();
		memset(fh.mMapBuffer, 0, sizeof fh.mMapBuffer);
		fh.mbMapBufferDirty = true;

		FileHandle dh;
		OpenFile(dh, fileKey >> 16);
		SeekFile(dh, 23 * (fileKey & 0xffff) + 1, false);

		uint8 secMapAddr[2];
		VDWriteUnalignedLEU16(secMapAddr, sectorMapStart);
		WriteFile(dh, secMapAddr, 2);
		FlushFile(dh);
	} else {
		ReadSector(sectorMapStart, fh.mMapBuffer);
		fh.mbMapBufferDirty = false;
	}

	fh.mSectorMapStart = sectorMapStart;
	fh.mCurrentDataSector = 0;
	fh.mCurrentMapSector = sectorMapStart;
	fh.mSectorOffset = 0;
	fh.mDataOffset = 0;
	fh.mMapSectorOffset = 0;
	fh.mDataSector = VDReadUnalignedLEU16(fh.mMapBuffer + 4);
	fh.mFileSize = 0;
	fh.mbDataSectorValid = false;
	fh.mbDataBufferDirty = false;
	fh.mbFileSizeDirty = false;
}

void ATDiskFSSDX2::SeekFile(FileHandle& fh, uint32 offset, bool allowExtend) {
	uint32 sectorOffset = offset >> mSectorShift;

	if (sectorOffset != fh.mSectorOffset || !fh.mbDataSectorValid || (allowExtend && !fh.mDataSector)) {
		if (sectorOffset < fh.mMapSectorOffset) {
			do {
				uint32 prevMapSector = VDReadUnalignedLEU16(fh.mMapBuffer + 2);

				if (fh.mbMapBufferDirty) {
					const_cast<ATDiskFSSDX2 *>(this)->WriteSector(fh.mCurrentMapSector, fh.mMapBuffer);
					fh.mbMapBufferDirty = false;
				}

				ReadSector(prevMapSector, fh.mMapBuffer);
				fh.mCurrentMapSector = prevMapSector;
				fh.mMapSectorOffset -= mSectorsPerMapPage;
			} while(sectorOffset < fh.mMapSectorOffset);
		} else if (sectorOffset > fh.mMapSectorOffset) {
			while(sectorOffset - fh.mMapSectorOffset >= mSectorsPerMapPage) {
				uint32 nextMapSector = VDReadUnalignedLEU16(fh.mMapBuffer);

				if (!nextMapSector) {
					if (!allowExtend)
						throw ATDiskFSException(kATDiskFSError_CorruptedFileSystem);

					uint32 newSector = const_cast<ATDiskFSSDX2 *>(this)->AllocateSector();

					if (fh.mCurrentMapSector) {
						VDWriteUnalignedLEU16(fh.mMapBuffer, newSector);
						const_cast<ATDiskFSSDX2 *>(this)->WriteSector(fh.mCurrentMapSector, fh.mMapBuffer);
						fh.mbMapBufferDirty = false;
					}

					memset(fh.mMapBuffer, 0, sizeof fh.mMapBuffer);
					VDWriteUnalignedLEU16(fh.mMapBuffer + 2, fh.mCurrentMapSector);

					fh.mCurrentMapSector = newSector;
					fh.mbMapBufferDirty = true;
				} else {
					ReadSector(nextMapSector, fh.mMapBuffer);
					fh.mCurrentMapSector = nextMapSector;
				}

				fh.mMapSectorOffset += mSectorsPerMapPage;
			}
		}

		fh.mSectorOffset = sectorOffset;

		VDASSERT((uint32)(sectorOffset - fh.mMapSectorOffset) < mSectorsPerMapPage);
		uint8 *sectorPtr = fh.mMapBuffer + 4 + (sectorOffset - fh.mMapSectorOffset)*2; 
		fh.mDataSector = VDReadUnalignedLEU16(sectorPtr);
		fh.mbDataSectorValid = true;

		if (!fh.mDataSector && allowExtend) {
			if (fh.mbDataBufferDirty) {
				WriteSector(fh.mCurrentDataSector, fh.mDataBuffer);
				fh.mbDataBufferDirty = false;
			}

			fh.mDataSector = AllocateSector();
			fh.mCurrentDataSector = fh.mDataSector;
			VDWriteUnalignedLEU16(sectorPtr, fh.mDataSector);
			fh.mbMapBufferDirty = true;

			memset(fh.mDataBuffer, 0, sizeof fh.mDataBuffer);
			fh.mbDataBufferDirty = true;
		}
	}

	fh.mDataOffset = offset & (mSectorSize - 1);
}

void ATDiskFSSDX2::ReadFile(FileHandle& fh, void *dst, uint32 len) {
	while(len) {
		uint32 tc = 0;

		if (fh.mbDataSectorValid) {
			if (fh.mCurrentDataSector != fh.mDataSector) {
				VDASSERT(!fh.mbDataBufferDirty);

				if (fh.mDataSector)
					ReadSector(fh.mDataSector, fh.mDataBuffer);
				else
					memset(fh.mDataBuffer, 0, sizeof fh.mDataBuffer);

				fh.mCurrentDataSector = fh.mDataSector;
			}

			tc = mSectorSize - fh.mDataOffset;

			if (tc > len)
				tc = len;
		}

		if (tc) {
			memcpy(dst, &fh.mDataBuffer[fh.mDataOffset], tc);
			fh.mDataOffset += tc;
			len -= tc;
			dst = (char *)dst + tc;
		} else {
			SeekFile(fh, (fh.mSectorOffset << mSectorShift) + fh.mDataOffset, false);
		}
	}
}

void ATDiskFSSDX2::WriteFile(FileHandle& fh, const void *src, uint32 len) {
	while(len) {
		uint32 tc = 0;

		if (fh.mDataSector) {
			if (fh.mCurrentDataSector != fh.mDataSector) {
				if (fh.mbDataBufferDirty) {
					WriteSector(fh.mCurrentDataSector, fh.mDataBuffer);
					fh.mbDataBufferDirty = false;
				}

				ReadSector(fh.mDataSector, fh.mDataBuffer);
				fh.mCurrentDataSector = fh.mDataSector;
			}

			tc = mSectorSize - fh.mDataOffset;

			if (tc > len)
				tc = len;
		}

		if (tc) {
			VDASSERT(fh.mCurrentDataSector);
			memcpy(&fh.mDataBuffer[fh.mDataOffset], src, tc);
			fh.mbDataBufferDirty = true;
			fh.mDataOffset += tc;
			len -= tc;
			src = (const char *)src + tc;

			uint32 pos = (fh.mSectorOffset << mSectorShift) + fh.mDataOffset;

			if (pos > fh.mFileSize) {
				fh.mFileSize = pos;
				fh.mbFileSizeDirty = true;
			}
		} else {
			SeekFile(fh, (fh.mSectorOffset << mSectorShift) + fh.mDataOffset, true);
		}
	}
}

void ATDiskFSSDX2::FlushFile(FileHandle& fh) {
	if (fh.mbDataBufferDirty) {
		WriteSector(fh.mCurrentDataSector, fh.mDataBuffer);
		fh.mbDataBufferDirty = false;
	}

	if (fh.mbMapBufferDirty) {
		WriteSector(fh.mCurrentMapSector, fh.mMapBuffer);
		fh.mbMapBufferDirty = false;
	}
}

uint32 ATDiskFSSDX2::GetDirectorySectorMap(uintptr key) {
	if (!key)
		return VDReadUnalignedLEU16(mSuperBlock + 9);

	FileHandle fh;
	OpenFile(fh, key >> 16);
	SeekFile(fh, (key & 0xffff) * 23, false);

	uint8 dirEnt[23];
	ReadFile(fh, dirEnt, 23);

	return VDReadUnalignedLEU16(dirEnt + 1);
}

void ATDiskFSSDX2::ReadSector(uint32 sector, void *buf) {
	if (mSectorSize != mpImage->ReadVirtualSector(sector - 1, buf, mSectorSize))
		throw ATDiskFSException(kATDiskFSError_ReadError);
}

void ATDiskFSSDX2::WriteSector(uint32 sector, void *buf) {
	if (!mpImage->WriteVirtualSector(sector - 1, buf, mSectorSize))
		throw ATDiskFSException(kATDiskFSError_WriteError);
}

bool ATDiskFSSDX2::IsSectorAllocated(uint32 sector) {
	uint32 bitmapSectorOffset = sector >> mBitmapSectorShift;

	LoadBitmapSector(mBitmapStartSector + bitmapSectorOffset);

	return !(mBitmapBuffer[(sector >> 3) & (mSectorSize - 1)] & (0x80 >> (sector & 7)));
}

uint32 ATDiskFSSDX2::AllocateSector() {
	if (!mFreeSectors)
		throw ATDiskFSException(kATDiskFSError_DiskFull);

	uint32 sec = mLastAllocSector;
	for(uint32 i=0; i<mTotalSectors; ++i) {
		if (++sec > mTotalSectors)
			sec = 1;

		if (!IsSectorAllocated(sec)) {
			AllocateSector(sec);
			return sec;
		}
	}

	throw ATDiskFSException(kATDiskFSError_CorruptedFileSystem);
}

void ATDiskFSSDX2::AllocateSector(uint32 sector) {
	uint32 bitmapSectorOffset = sector >> mBitmapSectorShift;

	LoadBitmapSector(mBitmapStartSector + bitmapSectorOffset);

	uint8& maskref = mBitmapBuffer[(sector >> 3) & (mSectorSize - 1)];
	const uint8 maskbit = (0x80 >> (sector & 7));
	VDASSERT((maskref & maskbit) != 0);
	maskref &= ~maskbit;
	mbBitmapSectorDirty = true;
	mbDirty = true;

	mLastAllocSector = sector;
	mbSuperBlockDirty = true;
	--mFreeSectors;
}

void ATDiskFSSDX2::FreeSector(uint32 sector) {
	uint32 bitmapSectorOffset = sector >> mBitmapSectorShift;

	LoadBitmapSector(mBitmapStartSector + bitmapSectorOffset);

	mBitmapBuffer[(sector >> 3) & (mSectorSize - 1)] |= (0x80 >> (sector & 7));
	mbBitmapSectorDirty = true;
	mbDirty = true;

	++mFreeSectors;
	mbSuperBlockDirty = true;
}

void ATDiskFSSDX2::LoadBitmapSector(uint32 sector) {
	if (mBitmapSector == sector)
		return;

	FlushBitmapSector();

	ReadSector(sector, mBitmapBuffer);
	mBitmapSector = sector;
}

void ATDiskFSSDX2::FlushBitmapSector() {
	if (mbBitmapSectorDirty) {
		const_cast<ATDiskFSSDX2 *>(this)->WriteSector(mBitmapSector, mBitmapBuffer);
		mbBitmapSectorDirty = false;
	}
}

void ATDiskFSSDX2::MarkVolumeChanged() {
	// Increment volume sequence counter.
	++mSuperBlock[38];
	mbSuperBlockDirty = true;
	mbDirty = true;
}

void ATDiskFSSDX2::WriteFileName(uint8 fn[11], const char *filename) {
	int offset = 0;
	bool ext = false;

	for(;;) {
		uint8 c = *filename;
		if (!c)
			break;

		++filename;

		if (c == '.') {
			while(offset < 8)
				fn[offset++] = 0x20;

			ext = true;
			continue;
		}

		// convert to uppercase
		if ((uint8)(c - 0x61) < 26)
			c -= 0x20;

		if (ext) {
			fn[offset++] = c;

			if (offset == 11)
				break;
		} else {
			if (offset < 8)
				fn[offset++] = c;
		}
	}

	while(offset < 11)
		fn[offset++] = 0x20;
}

bool ATDiskFSSDX2::IsValidFileName(const char *filename) {

	// all 8 characters may be from [a-zA-Z0-9_]
	int count = 0;

	for(;;) {
		uint8 c = *filename;

		if ((uint8)(c - 0x30) >= 10 && (uint8)((c & 0xdf) - 0x41) >= 26 && c != '_')
			break;

		++filename;

		if (++count > 8)
			return false;
	}

	// next needs to be EOS or a period
	if (!*filename)
		return true;

	if (*filename++ != '.')
		return false;

	// up to 3 alphanumeric characters may follow
	count = 0;

	for(;;) {
		uint8 c = *filename++;

		if ((uint8)(c - 0x30) >= 10 && (uint8)((c & 0xdf) - 0x41) >= 26)
			break;

		if (++count > 3)
			return false;
	}

	// looks OK
	return true;
}

///////////////////////////////////////////////////////////////////////////

IATDiskFS *ATDiskMountImageSDX2(IATDiskImage *image, bool readOnly) {
	vdautoptr<ATDiskFSSDX2> fs(new ATDiskFSSDX2);

	fs->Init(image, readOnly);

	return fs.release();
}

IATDiskFS *ATDiskFormatImageSDX2(IATDiskImage *image, const char *volNameHint) {
	vdautoptr<ATDiskFSSDX2> fs(new ATDiskFSSDX2);

	fs->InitNew(image, volNameHint);

	return fs.release();
}
