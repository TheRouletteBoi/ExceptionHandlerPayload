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
    if (g_LibLV2.lv2_kern_tty_write && g_LibLV2.lv2_clprintf_opd)
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

    MAKE_PATTERN( "7D A8 03 A6 4E 80 00 21" ).FindCodeMatch(
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
                    cxt.lv2_clprintf_opd = Signature::FindProcedureDescriptor(clPrintfBranchAddress, cxt.kernelLastOPDEntry);
                }

                cxt.printf = STATIC_FN(cxt.printf, Signature::FindProcedureDescriptor(printfBranchAddress, cxt.kernelLastOPDEntry));
            }

            return Signature::CallbackStatus::SCANNER_STOP;
        }
    );

    MAKE_PATTERN( "?? ?? ?? ?? 7F 84 E3 78 7F A5 EB 78 ?? ?? ?? ?? 7F 83 E3 78 7F A4 07 B4 ?? ?? ?? ??" ).FindCodeMatches(
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

    MAKE_PATTERN( "E9 22 ?? ?? 88 09 00 00 2F 80 00 00 41 9E ?? ?? E8 62" ).FindCodeMatch(
        LV2_BASE, 0x800000, g_LibLV2, +[](LibLV2Context& cxt, uint64_t result) -> Signature::CallbackStatus 
        {
            cxt.lv2_console_get_instance_opd = Signature::FindProcedureDescriptor(result, cxt.kernelLastOPDEntry);
            return Signature::CallbackStatus::SCANNER_STOP;
        }
    );

    MAKE_PATTERN( "7C 1B E8 AE 3B ?? 00 01 3B ?? ?? ?? 2F 80 00 0A" ).FindCodeMatches(
        LV2_BASE, 0x800000, g_LibLV2, +[](LibLV2Context& cxt, uint64_t result) -> Signature::CallbackStatus 
        {
            auto offset = 0x68UL;

            if (cxt.firmwareVersion < lv1::ps3_firmware_version(0, 9, 1))
                offset = 0x60UL;
            
            if (cxt.lv2_console_write_opd == 0)
            {
                cxt.lv2_console_write_opd = Signature::FindProcedureDescriptor(result - offset, cxt.kernelLastOPDEntry);

                return Signature::CallbackStatus::SCANNER_CONTINUE;
            }

            cxt.lv2_console_write_async_opd = Signature::FindProcedureDescriptor(result - offset, cxt.kernelLastOPDEntry);

            return Signature::CallbackStatus::SCANNER_STOP;
        }
    );

    MAKE_PATTERN( "7C 80 23 78 7C A9 2B 78 7C CB 33 78 7C E8 3B 78 7C 64 1B 78 7C 05 03 78 7D 26 4B 78 7D 67 5B 78" ).FindCodeMatch(
        LV2_BASE, 0x80000, g_LibLV2, +[](LibLV2Context& cxt, uint64_t result) -> Signature::CallbackStatus 
        {
            cxt.page_allocate_opd = Signature::FindProcedureDescriptor(result, cxt.kernelLastOPDEntry);
            return Signature::CallbackStatus::SCANNER_STOP;
        }
    );

    MAKE_PATTERN( "7C 64 1B 78 38 A0 00 09 38 60 00 00" ).FindCodeMatch(
        LV2_BASE, 0x800000, g_LibLV2, +[] ( LibLV2Context& cxt, uint64_t result) -> Signature::CallbackStatus 
        {
            const auto branchAddress = PPCGetBranchAddress(result + 0xC);

            if (branchAddress)
            {
                cxt.page_free_opd = Signature::FindProcedureDescriptor(branchAddress, cxt.kernelLastOPDEntry);
            }

            return Signature::CallbackStatus::SCANNER_STOP;
        }
    );

    MAKE_PATTERN( "7F A0 48 00 3D 20 80 01 7C DF 33 78 2F 25 00 00" ).FindCodeMatch(
        LV2_BASE, 0x800000, g_LibLV2, +[](LibLV2Context& cxt, uint64_t result) -> Signature::CallbackStatus 
        {
            cxt.kmem_export_to_proc_opd = Signature::FindProcedureDescriptor(result - 0x18, cxt.kernelLastOPDEntry);
            return Signature::CallbackStatus::SCANNER_STOP;
        }
    );

    MAKE_PATTERN( "E8 83 00 58 E8 63 00 68 4B ?? ?? ??" ).FindCodeMatch(
        LV2_BASE, 0x800000, g_LibLV2, +[](LibLV2Context& cxt, uint64_t result) -> Signature::CallbackStatus 
        {
            const auto branchAddress = PPCGetBranchAddress(result + 0x8);

            if (branchAddress)
            {
                cxt.kmem_unexport_from_proc_opd = Signature::FindProcedureDescriptor(branchAddress, cxt.kernelLastOPDEntry);
            }

            return Signature::CallbackStatus::SCANNER_STOP;
        }
    );

    MAKE_PATTERN( "7C 7C 1B 78 7C 9D 23 78 7F F2 02 A6 7F D3 02 A6 91 81 ?? ?? 7C 08 02 A6" ).FindCodeMatch(
        LV2_BASE, 0x800000, g_LibLV2, +[](LibLV2Context& cxt, uint64_t result) -> Signature::CallbackStatus 
        {
            cxt.ppu_thread_msg_interrupt_exception_opd = Signature::FindProcedureDescriptor(result - 0x28, cxt.kernelLastOPDEntry);   
            return Signature::CallbackStatus::SCANNER_STOP;
        }
    );
        

    if (g_LibLV2.lv2_console_get_instance_opd           == 0 ||
        g_LibLV2.lv2_console_write_opd                  == 0 || 
        g_LibLV2.lv2_console_write_async_opd            == 0 || 
        g_LibLV2.printf                                 == 0 ||
        g_LibLV2.ppu_thread_msg_interrupt_exception_opd == 0)
    {
        return LIB_LV2_ADDRESS_NOT_FOUND;
    }

    DEBUG_PRINT("lv2_console_get_instance_opd 0x%llx\n",                g_LibLV2.lv2_console_get_instance_opd);
    DEBUG_PRINT("lv2_console_write_opd 0x%llx\n",                       g_LibLV2.lv2_console_write_opd);
    DEBUG_PRINT("lv2_console_write_async_opd 0x%llx\n",                 g_LibLV2.lv2_console_write_async_opd);
    DEBUG_PRINT("lv2_console_putc_opd 0x%llx\n",                        g_LibLV2.lv2_console_putc_opd);
    DEBUG_PRINT("lv2_console_flush_opd 0x%llx\n",                       g_LibLV2.lv2_console_flush_opd);
    DEBUG_PRINT("lv2_clprintf_opd 0x%llx\n",                            g_LibLV2.lv2_clprintf_opd);
    DEBUG_PRINT("lv2_kern_tty_write 0x%llx\n",                          g_LibLV2.lv2_kern_tty_write);
    DEBUG_PRINT("page_allocate_opd 0x%llx\n",                           g_LibLV2.page_allocate_opd);
    DEBUG_PRINT("page_free_opd 0x%llx\n",                               g_LibLV2.page_free_opd);
    DEBUG_PRINT("kmem_export_to_proc_opd 0x%llx\n",                     g_LibLV2.kmem_export_to_proc_opd);
    DEBUG_PRINT("kmem_unexport_to_proc_opd 0x%llx\n",                   g_LibLV2.kmem_unexport_from_proc_opd);
    DEBUG_PRINT("ppu_thread_msg_interrupt_exception_opd 0x%llx\n",      g_LibLV2.ppu_thread_msg_interrupt_exception_opd);

    return LIB_LV2_INIT_SUCCESS;
}