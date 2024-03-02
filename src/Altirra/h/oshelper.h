#ifndef AT_OSHELPER_H
#define AT_OSHELPER_H

#include <vd2/system/vdtypes.h>

struct VDPixmap;

bool ATLoadKernelResource(int id, void *dst, uint32 size);
void ATFileSetReadOnlyAttribute(const wchar_t *path, bool readOnly);

void ATCopyFrameToClipboard(void *hwnd, const VDPixmap& px);

#endif