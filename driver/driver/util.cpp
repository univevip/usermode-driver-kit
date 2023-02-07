#pragma once
#include "util.h"

namespace util {
    PIMAGE_NT_HEADERS get_header(PVOID module) {
        return (PIMAGE_NT_HEADERS)((PBYTE)module + PIMAGE_DOS_HEADER(module)->e_lfanew);
    }

    PBYTE find_pattern(PVOID module, DWORD size, LPCSTR pattern, LPCSTR mask) {

        auto checkMask = [](PBYTE buffer, LPCSTR pattern, LPCSTR mask) -> BOOL
        {
            for (auto x = buffer; *mask; pattern++, mask++, x++) {
                auto addr = *(BYTE*)(pattern);
                if (addr != *x && *mask != '?')
                    return FALSE;
            }

            return TRUE;
        };

        for (auto x = 0; x < size - strlen(mask); x++) {

            auto addr = (PBYTE)module + x;
            if (checkMask(addr, pattern, mask))
                return addr;
        }

        return NULL;
    }

    PBYTE find_pattern(PVOID base, LPCSTR pattern, LPCSTR mask) {

        auto header = get_header(base);
        auto section = IMAGE_FIRST_SECTION(header);

        for (auto x = 0; x < header->FileHeader.NumberOfSections; x++, section++) {

            /*
            * Avoids non paged memory,
            * As well as greatly speeds up the process of scanning 30+ sections.
            */
            if (!memcmp(section->Name, ".text", 5) || !memcmp(section->Name, "PAGE", 4)) {
                auto addr = find_pattern((PBYTE)base + section->VirtualAddress, section->Misc.VirtualSize, pattern, mask);
                if (addr) {
                    return addr;
                }
            }
        }

        return NULL;
    }
}