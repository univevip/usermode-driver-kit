#pragma once
#include "stdafx.h"

namespace util {
	PIMAGE_NT_HEADERS get_header(PVOID module);
	PBYTE find_pattern(PVOID base, LPCSTR pattern, LPCSTR mask);
}