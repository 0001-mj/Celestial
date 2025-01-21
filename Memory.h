#pragma once

#include <SDK.hpp>
using namespace SDK;

#include <minhook.h>

uintptr_t ModuleBase;

class Memory {
public:
    static bool PatchRET(void* Address, bool bCleanupStack = false) {
        if (!Address) return false;

        DWORD OldProtection;
        if (!VirtualProtect(Address, sizeof(char), PAGE_EXECUTE_READWRITE, &OldProtection)) return false;

        *reinterpret_cast<unsigned char*>(Address) = bCleanupStack ? 0xC2 : 0xC3;

        DWORD NewProtection;
        if (!VirtualProtect(Address, sizeof(char), NewProtection, &NewProtection)) return false;

        return true;
    }

    static bool PatchCall(void* Start, void* End) {
        if (!Start || !End) return false;

        DWORD OldProtection;
        if (!VirtualProtect(Start, 5, PAGE_EXECUTE_READWRITE, &OldProtection)) return false;

        unsigned char Instruction = 0xE8;
        memcpy(Start, &Instruction, sizeof(Instruction));

        int32_t RelativeAddress = static_cast<int32_t>(
            reinterpret_cast<uintptr_t>(End) - reinterpret_cast<uintptr_t>(Start) - 5
            );

        memcpy(reinterpret_cast<unsigned char*>(Start) + 1, &RelativeAddress, sizeof(RelativeAddress));

        DWORD NewProtection;
        if (!VirtualProtect(Start, 5, OldProtection, &NewProtection)) return false;

        return true;
    }

    static bool SwapVirtualFunction(void* Class, int Index, void* Impl, void** Internal = nullptr)
    {
        void** VFTable = *static_cast<void***>(Class);

        if (Internal) *Internal = VFTable[Index];

        DWORD OldProtection;
        if (!VirtualProtect(&VFTable[Index], sizeof(void*), PAGE_EXECUTE_READWRITE, &OldProtection)) return false;

        VFTable[Index] = Impl;

        DWORD NewProtection;
        if (!VirtualProtect(&VFTable[Index], sizeof(void*), OldProtection, &NewProtection)) return false;

        return true;
    }
private:
    Memory() = delete;
};