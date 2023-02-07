#pragma once
#include <ntifs.h>
#include <ntddk.h>
#include <windef.h>
#include <ntimage.h>

#include "structs.h"

#define RVA(addr, size)			((PBYTE)(addr + *(DWORD*)(addr + ((size) - 4)) + size))
#define COMMUNICATION_KEY		(0xEBDFEADF)
#define printf(text, ...)		(DbgPrintEx(0, 0, text, ##__VA_ARGS__))

enum request {
	getbase = 0,
	reading = 1,
	writing = 2
};

struct communication {
	request request;
	DWORD process_id;
	DWORD key;
	ULONG64 process_base;

	void* output;
	void* buffer_address;
	uintptr_t address;
	size_t size;
};

extern "C" NTSTATUS ZwQuerySystemInformation(SYSTEM_INFORMATION_CLASS systemInformationClass, PVOID systemInformation, ULONG systemInformationLength, PULONG returnLength);
extern "C" NTSTATUS NTAPI MmCopyVirtualMemory(PEPROCESS SourceProcess, PVOID SourceAddress, PEPROCESS TargetProcess, PVOID TargetAddress, SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode, PSIZE_T ReturnSize);
extern "C" NTKERNELAPI PVOID PsGetProcessSectionBaseAddress(PEPROCESS Process);

#include "memory.h"
#include "util.h"
#include "core.h"