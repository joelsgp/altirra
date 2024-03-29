#ifndef f_AT_DISKIMAGE_H
#define f_AT_DISKIMAGE_H

class IVDRandomAccessStream;

enum ATDiskTimingMode {
	kATDiskTimingMode_Any,
	kATDiskTimingMode_UsePrecise,
	kATDiskTimingMode_UseOrdered
};

struct ATDiskVirtualSectorInfo {
	uint32	mStartPhysSector;
	uint32	mNumPhysSectors;
};

struct ATDiskPhysicalSectorInfo {
	uint32	mOffset;			// offset within memory image
	sint32	mDiskOffset;		// offset within disk image (for rewriting)
	uint16	mSize;
	bool	mbDirty;
	float	mRotPos;
	uint8	mFDCStatus;
	sint16	mWeakDataOffset;
};

struct ATDiskGeometryInfo {
	uint16	mSectorSize;
	uint8	mBootSectorCount;
	uint32	mTotalSectorCount;
	uint8	mTrackCount;
	uint32	mSectorsPerTrack;
	uint8	mSideCount;
	bool	mbMFM;
};

enum ATDiskImageFormat {
	kATDiskImageFormat_None,
	kATDiskImageFormat_ATR,
	kATDiskImageFormat_XFD,
	kATDiskImageFormat_P2,
	kATDiskImageFormat_P3,
	kATDiskImageFormat_ATX,
	kATDiskImageFormat_DCM
};

class VDINTERFACE IATDiskImage {
public:
	virtual ~IATDiskImage() {}

	virtual ATDiskTimingMode GetTimingMode() const = 0;

	virtual bool IsDirty() const = 0;
	virtual bool IsUpdatable() const = 0;
	virtual bool IsDynamic() const = 0;
	virtual ATDiskImageFormat GetImageFormat() const = 0;

	virtual bool Flush() = 0;

	virtual void SetPath(const wchar_t *path) = 0;
	virtual void Save(const wchar_t *path, ATDiskImageFormat format) = 0;

	virtual ATDiskGeometryInfo GetGeometry() const = 0;
	virtual uint32 GetSectorSize() const = 0;
	virtual uint32 GetSectorSize(uint32 virtIndex) const = 0;
	virtual uint32 GetBootSectorCount() const = 0;

	virtual uint32 GetPhysicalSectorCount() const = 0;
	virtual void GetPhysicalSectorInfo(uint32 index, ATDiskPhysicalSectorInfo& info) const = 0;

	virtual void ReadPhysicalSector(uint32 index, void *data, uint32 len) = 0;
	virtual void WritePhysicalSector(uint32 index, const void *data, uint32 len) = 0;

	virtual uint32 GetVirtualSectorCount() const = 0;
	virtual void GetVirtualSectorInfo(uint32 index, ATDiskVirtualSectorInfo& info) const = 0;

	virtual uint32 ReadVirtualSector(uint32 index, void *data, uint32 len) = 0;
	virtual bool WriteVirtualSector(uint32 index, const void *data, uint32 len) = 0;
};

IATDiskImage *ATLoadDiskImage(const wchar_t *path);
IATDiskImage *ATLoadDiskImage(const wchar_t *origPath, const wchar_t *imagePath, IVDRandomAccessStream& stream);
IATDiskImage *ATMountDiskImageVirtualFolder(const wchar_t *path, uint32 sectorCount);
IATDiskImage *ATMountDiskImageVirtualFolderSDFS(const wchar_t *path, uint32 sectorCount, uint64 uniquenessValue);
IATDiskImage *ATCreateDiskImage(uint32 sectorCount, uint32 bootSectorCount, uint32 sectorSize);

#endif	// f_AT_DISKIMAGE_H
