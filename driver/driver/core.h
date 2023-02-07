#pragma once
#include "stdafx.h"

namespace core {
	bool read_kernel_memory(HANDLE pid, uintptr_t address, void* buffer, SIZE_T size);
	bool write_kernel_memory(HANDLE pid, uintptr_t address, void* buffer, SIZE_T size);
	PVOID get_module_base(const char* module_name);
}