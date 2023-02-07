#pragma once
#include "core.h"

namespace core {
	bool read_kernel_memory(HANDLE pid, uintptr_t address, void* buffer, SIZE_T size) {
		if (!address || !buffer || !size)
			return false;

		SIZE_T bytes = 0;
		NTSTATUS status = STATUS_SUCCESS;
		PEPROCESS process;
		PsLookupProcessByProcessId((HANDLE)pid, &process);

		status = MmCopyVirtualMemory(process, (void*)address, (PEPROCESS)PsGetCurrentProcess(), (void*)buffer, size, KernelMode, &bytes);

		if (!NT_SUCCESS(status))
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	bool write_kernel_memory(HANDLE pid, uintptr_t address, void* buffer, SIZE_T size)
	{
		if (!address || !buffer || !size)
			return false;

		NTSTATUS status = STATUS_SUCCESS;
		PEPROCESS process;
		PsLookupProcessByProcessId((HANDLE)pid, &process);

		KAPC_STATE state;
		KeStackAttachProcess((PEPROCESS)process, &state);

		MEMORY_BASIC_INFORMATION info;

		status = ZwQueryVirtualMemory(ZwCurrentProcess(), (PVOID)address, MemoryBasicInformation, &info, sizeof(info), NULL);
		if (!NT_SUCCESS(status))
		{
			KeUnstackDetachProcess(&state);
			return false;
		}

		if (((uintptr_t)info.BaseAddress + info.RegionSize) < (address + size))
		{
			KeUnstackDetachProcess(&state);
			return false;
		}

		if (!(info.State & MEM_COMMIT) || (info.Protect & (PAGE_GUARD | PAGE_NOACCESS)))
		{
			KeUnstackDetachProcess(&state);
			return false;
		}

		if ((info.Protect & PAGE_EXECUTE_READWRITE) || (info.Protect & PAGE_EXECUTE_WRITECOPY)
			|| (info.Protect & PAGE_READWRITE) || (info.Protect & PAGE_WRITECOPY))
		{
			RtlCopyMemory((void*)address, buffer, size);
		}
		KeUnstackDetachProcess(&state);
		return true;
	}

	PVOID get_module_base(const char* module_name)
	{
		ULONG bytes = 0;
		NTSTATUS status = ZwQuerySystemInformation(SystemModuleInformation, NULL, bytes, &bytes);

		if (!bytes)
			return NULL;

		PRTL_PROCESS_MODULES modules = (PRTL_PROCESS_MODULES)ExAllocatePoolWithTag(NonPagedPool, bytes, 0x4e554c4c);

		status = ZwQuerySystemInformation(SystemModuleInformation, modules, bytes, &bytes);

		if (!NT_SUCCESS(status))
			return NULL;

		PRTL_PROCESS_MODULE_INFORMATION module = modules->Modules;
		PVOID module_base = 0, module_size = 0;

		for (ULONG i = 0; i < modules->NumberOfModules; i++)
		{
			if (strcmp((char*)module[i].FullPathName, module_name) == NULL)
			{
				module_base = module[i].ImageBase;
				module_size = (PVOID)module[i].ImageSize;
				break;
			}
		}

		if (modules)
			ExFreePoolWithTag(modules, 0x4e554c4c);

		if (module_base <= NULL)
			return NULL;

		return module_base;
	}
}
