#ifndef f_DITA_STDAFX_H
#define f_DITA_STDAFX_H

#if defined(_MSC_VER) && _MSC_VER < 1300
#pragma warning(disable: 4786)
static const struct VD_MSVC_C4786Workaround { VD_MSVC_C4786Workaround() {} } g_VD_MSVC_C4786Workaround;
#endif

#include <vd2/system/vdtypes.h>
#include <vd2/Dita/interface.h>

#include <windows.h>

#include <vd2/system/VDString.h>
#include <vd2/Dita/basetypes.h>
#include <vd2/Dita/controls.h>
#include <vd2/Dita/services.h>
#include <vd2/Dita/w32control.h>

#endif
