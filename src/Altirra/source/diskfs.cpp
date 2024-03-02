#include "stdafx.h"
#include "diskfs.h"
#include "diskimage.h"

IATDiskFS *ATDiskMountImageDOS2(IATDiskImage *image, bool readOnly);
IATDiskFS *ATDiskMountImageSDX2(IATDiskImage *image, bool readOnly);

ATDiskFSException::ATDiskFSException(ATDiskFSError error)
	: mErrorCode(error)
{
	switch(error) {
		case kATDiskFSError_InvalidFileName:
			assign("The file name is not allowed by this file system.");
			break;

		case kATDiskFSError_DiskFull:
			assign("There is not enough space on the disk.");
			break;

		case kATDiskFSError_DirectoryFull:
			assign("The directory is full and cannot hold any more file entries.");
			break;

		case kATDiskFSError_CorruptedFileSystem:
			assign("The file system is damaged.");
			break;

		case kATDiskFSError_FileExists:
			assign("A file or directory already exists with the same name.");
			break;

		case kATDiskFSError_ReadOnly:
			assign("The file system has been mounted read-only.");
			break;

		case kATDiskFSError_FileTooLarge:
			assign("The file is too large for this file system.");
			break;

		case kATDiskFSError_ReadError:
			assign("An I/O error was encountered while reading from the disk.");
			break;

		case kATDiskFSError_WriteError:
			assign("An I/O error was encountered while writing to the disk.");
			break;

		case kATDiskFSError_CannotReadSparseFile:
			assign("The file cannot be read as it is sparsely allocated.");
			break;

		case kATDiskFSError_DirectoryNotEmpty:
			assign("The directory is not empty.");
			break;

		case kATDiskFSError_UnsupportedCompressionMode:
			assign("The file uses an unsupported compression mode.");
			break;

		case kATDiskFSError_DecompressionError:
			assign("An error was encountered while decompressing the file.");
			break;

		case kATDiskFSError_CRCError:
			assign("A CRC error was encountered while decompressing the file.");
			break;
	}
}

IATDiskFS *ATDiskMountImage(IATDiskImage *image, bool readOnly) {
	uint8 secbuf[128];

	if (image->ReadVirtualSector(0, secbuf, 128) < 128)
		return NULL;

	// $80 is the signature byte for SDX SD/DD disks; $40 is for DD 512.
	if (secbuf[7] == 0x80 || secbuf[7] == 0x40)
		return ATDiskMountImageSDX2(image, readOnly);
	else 
		return ATDiskMountImageDOS2(image, readOnly);
}
