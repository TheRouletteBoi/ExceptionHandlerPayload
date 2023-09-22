#include "LV2.h"
#include "String.h"
#include "SystemCalls.h"
#include "../PatternScanner.h"
#include "../PPC.h"
#include "../Utils.h"

#include "../LV1/LV1Calls.h"

LibLV2Context g_LibLV2;

OPD_t* FindLastOPDEntry(uint64_t TOC)
{
    uint64_t  tocSegmentStart    = TOC - 0x8000;
    uint64_t  lastEntryUnaligned = (tocSegmentStart - sizeof(OPD_t));
    uint64_t* lastEntryTOCPtr    = (uint64_t*)(lastEntryUnaligned + __builtin_offsetof(OPD_t, TOC));

    // OPD segment is aligned by a weird value, search backwards to account for alignment.
    for (uint64_t i = 0; i < 10; ++i)
    {
        if (*lastEntryTOCPtr == TOC)
        {
            // We found TOC in a OPD entry, now subtract the offset of that member.
            return (OPD_t*)(uint64_t(lastEntryTOCPtr) - __builtin_offsetof(OPD_t, TOC));
        }

        lastEntryTOCPtr--;
    }

    return 0;
}

#ifdef HAS_DEBUG_PRINTS
void LibLV2DebugPrint(const char* format, ...)
{
#ifdef HAS_DEBUG_PRINTS_WITH_DEBUG_COBRA
    g_LibLV2.lv2_kern_tty_write = (decltype(g_LibLV2.lv2_kern_tty_write))0x8000000000362288;
    g_LibLV2.lv2Clprintf_opd = (OPD_t*)0x800000000032e268;
#endif

    if (g_LibLV2.lv2_kern_tty_write && g_LibLV2.lv2Clprintf_opd)
    {
        va_list argList;
        va_start(argList, format);

        lv2::clprintf(format, argList, (lv2::PrintfCallback_t)g_LibLV2.lv2_kern_tty_write, NULL);

        va_end(argList);
    }
}
#endif

bool LibLV2IsSystemCallTable(OPD_t** address, const OPD_t* invalidSysCallEntry)
{
    return ( address[  0 ] == invalidSysCallEntry &&
             address[  1 ] != invalidSysCallEntry &&
             address[  2 ] != invalidSysCallEntry &&
             address[ 16 ] == invalidSysCallEntry &&
             address[ 17 ] == invalidSysCallEntry &&
             address[ 20 ] == invalidSysCallEntry &&
             address[ 32 ] == invalidSysCallEntry &&
             address[ 33 ] == invalidSysCallEntry &&
             address[ 34 ] == invalidSysCallEntry &&
             address[ 36 ] == invalidSysCallEntry &&
             address[ 39 ] == invalidSysCallEntry &&
             address[ 40 ] == invalidSysCallEntry &&
             address[ 42 ] == invalidSysCallEntry );
}

uint64_t LibLV2Init(uint64_t TOC, uint64_t payloadTOC)
{
    lv1::hypercall::get_firmware_version(&g_LibLV2.firmwareVersion);

    g_LibLV2.kernelTOC  = TOC;
    g_LibLV2.payloadTOC = payloadTOC;
    g_LibLV2.kernelLastOPDEntry = FindLastOPDEntry(TOC);
    g_LibLV2.systemCallTable = (OPD_t**)0x800000000038a4e8;

    if (g_LibLV2.kernelLastOPDEntry == NULL)
        return LIB_LV2_OPD_SEG_NOT_FOUND;

    MAKE_PATTERN("3C 60 80 01 60 63 00 03 4E 80 00 20").FindCodeMatches(
        LV2_BASE, 0x800000, g_LibLV2, +[](LibLV2Context& cxt, uint64_t result) -> Signature::CallbackStatus 
        {
            const auto procDescriptor = Signature::FindProcedureDescriptor(result, cxt.kernelLastOPDEntry);

            uint64_t tocSegmentEnd = cxt.kernelTOC + 0x8000;

            // Search for XREF to this systemcall not implmented proc descriptor.
            for (uint64_t i = 0; i < 0x100000; i += 0x8)
            {
                auto searchXref = (OPD_t**)(tocSegmentEnd + i);

                if (*searchXref == procDescriptor)
                {
                    if (LibLV2IsSystemCallTable(searchXref, procDescriptor))
                    {
                        g_LibLV2.systemCallTable = searchXref;
                        return Signature::CallbackStatus::SCANNER_STOP;
                    }
                }
            }

            return Signature::CallbackStatus::SCANNER_CONTINUE;
        }
    );	

    if (g_LibLV2.systemCallTable == NULL)
        return LIB_LV2_SYSCALL_TABLE_NOT_FOUND;

    MAKE_PATTERN("7D A8 03 A6 4E 80 00 21").FindCodeMatch(
        LV2_BASE, 0x800000, g_LibLV2, +[](LibLV2Context& cxt, uint64_t result) -> Signature::CallbackStatus 
        {
            cxt.systemCallDispatchBranch = (uint32_t*)(result + 4);

            return Signature::CallbackStatus::SCANNER_STOP;
        }
    );

    if (g_LibLV2.systemCallDispatchBranch == NULL)
        return LIB_LV2_SYSCALL_DISPATCH_NOT_FOUND;

    MAKE_PATTERN("E8 ?? 00 10 ?? ?? ?? ?? 38 84 FF FC").FindCodeMatch(
        LV2_BASE, 0x800000, g_LibLV2, +[](LibLV2Context& cxt, uint64_t result) -> Signature::CallbackStatus 
        {
            const auto printfBranchAddress = PPCGetBranchAddress(result + 0xC);

            if (printfBranchAddress)
            {
                const auto clPrintfBranchAddress = PPCGetBranchAddress(printfBranchAddress + 0x60);

                if (clPrintfBranchAddress)
                {
                    cxt.lv2Clprintf_opd = Signature::FindProcedureDescriptor(clPrintfBranchAddress, cxt.kernelLastOPDEntry);
                }

                cxt.printf = STATIC_FN(cxt.printf, Signature::FindProcedureDescriptor(printfBranchAddress, cxt.kernelLastOPDEntry));
            }

            return Signature::CallbackStatus::SCANNER_STOP;
        }
    );

    MAKE_PATTERN("?? ?? ?? ?? 7F 84 E3 78 7F A5 EB 78 ?? ?? ?? ?? 7F 83 E3 78 7F A4 07 B4 ?? ?? ?? ??").FindCodeMatches(
        g_LibLV2.systemCallTable[SYS_CONSOLE_WRITE]->Function, 0x200, g_LibLV2, +[](LibLV2Context& cxt, uint64_t result) -> Signature::CallbackStatus 
        {
            const auto branchAddress = PPCGetBranchAddress(result + 0x18);

            if (branchAddress)
            {
                cxt.lv2_kern_tty_write = (decltype(cxt.lv2_kern_tty_write))Signature::FindProcedureDescriptor(branchAddress, cxt.kernelLastOPDEntry);
            }

            return Signature::CallbackStatus::SCANNER_STOP;
        }
    );

    MAKE_PATTERN("E9 22 ?? ?? 88 09 00 00 2F 80 00 00 41 9E ?? ?? E8 62").FindCodeMatch(
        LV2_BASE, 0x800000, g_LibLV2, +[](LibLV2Context& cxt, uint64_t result) -> Signature::CallbackStatus 
        {
            cxt.lv2ConsoleGetInstance_opd = Signature::FindProcedureDescriptor(result, cxt.kernelLastOPDEntry);
            return Signature::CallbackStatus::SCANNER_STOP;
        }
    );

    MAKE_PATTERN("7C 1B E8 AE 3B ?? 00 01 3B ?? ?? ?? 2F 80 00 0A").FindCodeMatches(
        LV2_BASE, 0x800000, g_LibLV2, +[](LibLV2Context& cxt, uint64_t result) -> Signature::CallbackStatus 
        {
            auto offset = 0x68UL;

            if (cxt.firmwareVersion < lv1::ps3_firmware_version(0, 9, 1))
                offset = 0x60UL;
            
            if (cxt.lv2ConsoleWrite_opd == 0)
            {
                cxt.lv2ConsoleWrite_opd = Signature::FindProcedureDescriptor(result - offset, cxt.kernelLastOPDEntry);

                return Signature::CallbackStatus::SCANNER_CONTINUE;
            }

            cxt.lv2ConsoleWriteAsync_opd = Signature::FindProcedureDescriptor(result - offset, cxt.kernelLastOPDEntry);

            return Signature::CallbackStatus::SCANNER_STOP;
        }
    );

    MAKE_PATTERN("7C 80 23 78 7C A9 2B 78 7C CB 33 78 7C E8 3B 78 7C 64 1B 78 7C 05 03 78 7D 26 4B 78 7D 67 5B 78").FindCodeMatch(
        LV2_BASE, 0x80000, g_LibLV2, +[](LibLV2Context& cxt, uint64_t result) -> Signature::CallbackStatus 
        {
            cxt.pageAllocate_opd = Signature::FindProcedureDescriptor(result, cxt.kernelLastOPDEntry);
            return Signature::CallbackStatus::SCANNER_STOP;
        }
    );

    MAKE_PATTERN("7C 64 1B 78 38 A0 00 09 38 60 00 00").FindCodeMatch(
        LV2_BASE, 0x800000, g_LibLV2, +[] ( LibLV2Context& cxt, uint64_t result) -> Signature::CallbackStatus 
        {
            const auto branchAddress = PPCGetBranchAddress(result + 0xC);

            if (branchAddress)
            {
                cxt.pageFree_opd = Signature::FindProcedureDescriptor(branchAddress, cxt.kernelLastOPDEntry);
            }

            return Signature::CallbackStatus::SCANNER_STOP;
        }
    );

    MAKE_PATTERN("7F A0 48 00 3D 20 80 01 7C DF 33 78 2F 25 00 00").FindCodeMatch(
        LV2_BASE, 0x800000, g_LibLV2, +[](LibLV2Context& cxt, uint64_t result) -> Signature::CallbackStatus 
        {
            cxt.kmemExportToProc_opd = Signature::FindProcedureDescriptor(result - 0x18, cxt.kernelLastOPDEntry);
            return Signature::CallbackStatus::SCANNER_STOP;
        }
    );

    MAKE_PATTERN("E8 83 00 58 E8 63 00 68 4B ?? ?? ??").FindCodeMatch(
        LV2_BASE, 0x800000, g_LibLV2, +[](LibLV2Context& cxt, uint64_t result) -> Signature::CallbackStatus 
        {
            const auto branchAddress = PPCGetBranchAddress(result + 0x8);

            if (branchAddress)
            {
                cxt.kmemUnexportFromProc_opd = Signature::FindProcedureDescriptor(branchAddress, cxt.kernelLastOPDEntry);
            }

            return Signature::CallbackStatus::SCANNER_STOP;
        }
    );

    MAKE_PATTERN("7C 7C 1B 78 7C 9D 23 78 7F F2 02 A6 7F D3 02 A6 91 81 ?? ?? 7C 08 02 A6").FindCodeMatch(
        LV2_BASE, 0x800000, g_LibLV2, +[](LibLV2Context& cxt, uint64_t result) -> Signature::CallbackStatus 
        {
            cxt.ppuThreadMsgInterruptException3rdInstructionAddress = (result - 0x20);
            cxt.ppuThreadMsgInterruptException_opd = Signature::FindProcedureDescriptor(result - 0x28, cxt.kernelLastOPDEntry);   
            return Signature::CallbackStatus::SCANNER_STOP;
        }
    );

    MAKE_PATTERN("38 A0 ?? ?? 4B ?? ?? ?? E8 82 ?? ?? E8 62 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 4B ?? ?? ??").FindCodeMatch(
        LV2_BASE, 0x800000, g_LibLV2, +[](LibLV2Context& cxt, uint64_t result) -> Signature::CallbackStatus 
        {
            cxt.ppuThreadMsgInterruptExceptionBranch = (uint32_t*)(result + 0x1C);
            return Signature::CallbackStatus::SCANNER_STOP;
        }
    );

    MAKE_PATTERN("7C BE ?? ?? 7C 9D ?? ?? 7C DC ?? ?? 7C FB ?? ?? 63 FF ?? ?? 7C 7A ?? ?? 40 82 ?? ??").FindCodeMatch(
        LV2_BASE, 0x800000, g_LibLV2, +[](LibLV2Context& cxt, uint64_t result) -> Signature::CallbackStatus 
        {
            cxt.processWriteMemory_opd = Signature::FindProcedureDescriptor(result - 0x2C, cxt.kernelLastOPDEntry);
            return Signature::CallbackStatus::SCANNER_STOP;
        }
    );

    MAKE_PATTERN("F8 21 ?? ?? 7C 08 ?? ?? 2C 25 ?? ?? FB E1 ?? ?? 3F E0 ?? ?? FB 61 ?? ??").FindCodeMatch(
        LV2_BASE, 0x800000, g_LibLV2, +[](LibLV2Context& cxt, uint64_t result) -> Signature::CallbackStatus 
        {
            cxt.processReadMemory_opd = Signature::FindProcedureDescriptor(result, cxt.kernelLastOPDEntry);
            return Signature::CallbackStatus::SCANNER_STOP;
        }
    );

    MAKE_PATTERN("4B ?? ?? ?? 2F 83 ?? ?? 7F A4 ?? ?? 7F 65 ?? ?? 7F 86 ?? ?? 38 E0 ?? ?? 41 ?? ?? ??").FindCodeMatch(
        LV2_BASE, 0x800000, g_LibLV2, +[](LibLV2Context& cxt, uint64_t result) -> Signature::CallbackStatus 
        {
            cxt.getProcessObjectById_opd = Signature::FindProcedureDescriptor(result - 0x24, cxt.kernelLastOPDEntry);
            return Signature::CallbackStatus::SCANNER_STOP;
        }
    );

    MAKE_PATTERN("7C 68 ?? ?? 7C 20 04 AC 38 00 ?? ?? 7D 40 ?? ??").FindCodeMatch(
        LV2_BASE, 0x800000, g_LibLV2, +[](LibLV2Context& cxt, uint64_t result) -> Signature::CallbackStatus 
        {
            cxt.idTableUnreserveId_opd = Signature::FindProcedureDescriptor(result - 0x1C, cxt.kernelLastOPDEntry);
            return Signature::CallbackStatus::SCANNER_STOP;
        }
    );

    MAKE_PATTERN("F8 21 ?? ?? 7C 08 ?? ?? FB 81 ?? ?? F8 01 ?? ?? FB A1 ?? ?? 7C 7D ?? ?? 4B ?? ?? ??").FindCodeMatch(
        LV2_BASE, 0x800000, g_LibLV2, +[](LibLV2Context& cxt, uint64_t result) -> Signature::CallbackStatus 
        {
            cxt.extendKstack_opd = Signature::FindProcedureDescriptor(result, cxt.kernelLastOPDEntry);
            return Signature::CallbackStatus::SCANNER_STOP;
        }
    );

    MAKE_PATTERN("39 1F ?? ?? 39 3F ?? ?? 39 5F ?? ?? FB A1 ?? ??").FindCodeMatch(
        LV2_BASE, 0x800000, g_LibLV2, +[](LibLV2Context& cxt, uint64_t result) -> Signature::CallbackStatus 
        {
            const auto branchLocation = result + 0x10;
            const auto functionAddress = PPCGetBranchAddress(branchLocation);

            cxt.ppuLoaderLoadProgramBranch = branchLocation;
            cxt.ppuLoaderLoadProgram_opd = Signature::FindProcedureDescriptor(functionAddress, cxt.kernelLastOPDEntry);

            return Signature::CallbackStatus::SCANNER_STOP;
        }
    );

    
        
#ifndef HAS_DEBUG_PRINTS_WITH_DEBUG_COBRA
    if (g_LibLV2.lv2ConsoleGetInstance_opd              == 0 ||
        g_LibLV2.lv2ConsoleWrite_opd                    == 0 || 
        g_LibLV2.lv2ConsoleWriteAsync_opd               == 0 || 
        g_LibLV2.printf                                 == 0 ||
        g_LibLV2.ppuThreadMsgInterruptException_opd     == 0)
    {
        return LIB_LV2_ADDRESS_NOT_FOUND;
    }
#endif

    DEBUG_PRINT("lv2ConsoleGetInstance_opd 0x%llx\n",                   g_LibLV2.lv2ConsoleGetInstance_opd);
    DEBUG_PRINT("lv2ConsoleWrite_opd 0x%llx\n",                         g_LibLV2.lv2ConsoleWrite_opd);
    DEBUG_PRINT("lv2ConsoleWriteAsync_opd 0x%llx\n",                    g_LibLV2.lv2ConsoleWriteAsync_opd);
    DEBUG_PRINT("lv2ConsolePutc_opd 0x%llx\n",                          g_LibLV2.lv2ConsolePutc_opd);
    DEBUG_PRINT("lv2ConsoleFlush_opd 0x%llx\n",                         g_LibLV2.lv2ConsoleFlush_opd);
    DEBUG_PRINT("lv2Clprintf_opd 0x%llx\n",                             g_LibLV2.lv2Clprintf_opd);
    DEBUG_PRINT("lv2_kern_tty_write 0x%llx\n",                          g_LibLV2.lv2_kern_tty_write);
    DEBUG_PRINT("pageAllocate_opd 0x%llx\n",                            g_LibLV2.pageAllocate_opd);
    DEBUG_PRINT("pageFree_opd 0x%llx\n",                                g_LibLV2.pageFree_opd);
    DEBUG_PRINT("kmemExportToProc_opd 0x%llx\n",                        g_LibLV2.kmemExportToProc_opd);
    DEBUG_PRINT("kmem_unexport_to_proc_opd 0x%llx\n",                   g_LibLV2.kmemUnexportFromProc_opd);

    DEBUG_PRINT("ppuThreadMsgInterruptException_opd 0x%llx\n",          g_LibLV2.ppuThreadMsgInterruptException_opd);
    DEBUG_PRINT("ppuThreadMsgInterruptException3rdInstructionAddress 0x%llx\n",                 g_LibLV2.ppuThreadMsgInterruptException3rdInstructionAddress);
    
    DEBUG_PRINT("ppuLoaderLoadProgram_opd 0x%llx\n",                    g_LibLV2.ppuLoaderLoadProgram_opd);
    DEBUG_PRINT("ppuLoaderLoadProgramBranch 0x%llx\n",                  g_LibLV2.ppuLoaderLoadProgramBranch);
    

    return LIB_LV2_INIT_SUCCESS;
}