#ifndef HOOKOPCODE_H
#define HOOKOPCODE_H

#include "Types.h"
#include "ELF.h"
#include "Libc.h"
#include "PPC.h"

#define MAX_HOOKS 5

namespace HookOpCode
{
    struct HookContext
    {
        // General purpose registers (gprs)
        uint64_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19, r20, r21, r22, r23, r24, r25, r26, r27, r28, r29, r30, r31;

        // Special purpose registers (sprs)
        uint64_t lr, ctr, cr, xer;

        // Floating point registers (fprs)
        double f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25, f26, f27, f28, f29, f30, f31;

        template<typename T>
        inline T& GetGpr(int index)
        {
            union reg_t
            {
                uint64_t packed;
                struct
                {
                    char ___padding[sizeof(uint64_t) - sizeof(T)];
                    T value;
                };
            };

            reg_t& reg = *(reg_t*)(&r0 + index);
            return reg.value;
        }

        inline double& GetFpr(int index)
        {
            return (&f0)[index];
        }
    };

    using HookCallback = void(*)(HookContext*);
    struct HookEntry
    {
        uintptr_t address;
        uint32_t instruction;
        HookCallback callback;
    };

    extern "C" void Handler(HookContext* ctx);
    void AttachDetour(uintptr_t address, HookCallback callback);
}

#endif // !HOOKOPCODE_H