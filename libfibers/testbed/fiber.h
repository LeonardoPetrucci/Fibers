#pragma once

#define KERNELSPACE
#ifdef USERSPACE

#include "src/ult.h"

#define ConvertThreadToFiber() ult_convert()
#define CreateFiber(dwStackSize, lpStartAddress, lpParameter) ult_creat(dwStackSize, lpStartAddress, lpParameter)
#define SwitchToFiber(lpFiber) ult_switch_to(lpFiber)
#define FlsAlloc(lpCallback) fls_alloc()
#define FlsFree(dwFlsIndex)	fls_free(dwFlsIndex)
#define FlsGetValue(dwFlsIndex) fls_get(dwFlsIndex)
#define FlsSetValue(dwFlsIndex, lpFlsData) fls_set((dwFlsIndex), (long long)(lpFlsData))

#else

// TODO:
// Here you should point to the invocation of your code!
// See README.md for further details.

#include "../include/fiberlib.h"

#define ConvertThreadToFiber() Convert_thread_to_fiber()
#define CreateFiber(dwStackSize, lpStartAddress, lpParameter) Create_fiber(dwStackSize, lpStartAddress, lpParameter)
#define SwitchToFiber(lpFiber) Switch_to_fiber(lpFiber)
#define FlsAlloc(lpCallback) Fls_alloc()
#define FlsFree(dwFlsIndex) Fls_free(dwFlsIndex)
#define FlsGetValue(dwFlsIndex) Fls_get_value(dwFlsIndex)
#define FlsSetValue(dwFlsIndex, lpFlsData) Fls_set_value((dwFlsIndex), (long long)(lpFlsData))

#endif /* USERSPACE */
