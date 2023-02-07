#pragma once
#include "stdafx.h"

__int64(__fastcall* original_function)(void*, void*);
__int64 hooked_function(void* a1, void* a2) {
	if (ExGetPreviousMode() != UserMode) {
		return original_function(a1, a2);
	}

	if (!a1) {
		return original_function(a1, a2);
	}

	communication* comms = (communication*)a1;

	if (comms->key != COMMUNICATION_KEY) {
		return original_function(a1, a2);
	}

	switch (comms->request) {

	case request::getbase: {
		PEPROCESS process = { 0 };
		PsLookupProcessByProcessId((HANDLE)comms->process_id, &process);
		comms->process_base = (ULONG64)PsGetProcessSectionBaseAddress(process);
		ObDereferenceObject(process);
	} break;
	case request::writing: {
		PVOID kernel_buff = ExAllocatePool(NonPagedPool, comms->size);

		if (!kernel_buff)
		{
			return STATUS_UNSUCCESSFUL;
		}

		if (!memcpy(kernel_buff, comms->buffer_address, comms->size))
		{
			return STATUS_UNSUCCESSFUL;
		}

		PEPROCESS process;
		PsLookupProcessByProcessId((HANDLE)comms->process_id, &process);
		core::write_kernel_memory((HANDLE)comms->process_id, comms->address, kernel_buff, comms->size);
		ExFreePool(kernel_buff);
	} break;
	case request::reading: {
		core::read_kernel_memory((HANDLE)comms->process_id, comms->address, comms->output, comms->size);
	} break;
	}
	
	return NULL;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING reg_path) {
	UNREFERENCED_PARAMETER(driver);
	UNREFERENCED_PARAMETER(reg_path);

	RTL_OSVERSIONINFOW info = { 0 };

	if (!info.dwBuildNumber) {
		RtlGetVersion(&info);
	}

	printf("[aceware] os version -> %d", info.dwBuildNumber);

	PVOID base = core::get_module_base("\\SystemRoot\\System32\\win32k.sys");
	if (!base) {
		return STATUS_FAILED_DRIVER_ENTRY;
	}

	printf("[aceware] win32k.sys -> 0x%x", base);

	PBYTE addr = util::find_pattern(base,
		"\x48\x8B\x05\x00\x00\x00\x00\x48\x85\xC0\x74\x1C\x4C\x8B\x54\x24\x00\x4C\x89\x54\x24\x00\x44\x8B\x54\x24\x00\x44\x89\x54\x24\x00\xFF\x15\x00\x00\x00\x00\xEB\x07",
		"xxx????xxxxxxxxx?xxxx?xxxx?xxxx?xx????xx");

	if (!addr) {
		return STATUS_FAILED_DRIVER_ENTRY;
	}

	addr = RVA(addr, 7);

	printf("[aceware] addr -> 0x%x", addr);

	*(void**)&original_function = _InterlockedExchangePointer((void**)addr, hooked_function);

	printf("[aceware] swapped pointer -> 0x%x to 0x%x", addr, &hooked_function);

	return STATUS_SUCCESS;
}