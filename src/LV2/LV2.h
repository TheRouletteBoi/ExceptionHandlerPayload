
#ifndef LV2_H
#define LV2_H

#include "../Types.h"
#include "../ELF.h"
#include "../Config.h"
#include "../LV1/LV1.h"
#include "SystemCalls.h"

#define LV2_BASE      ( 0x8000000000000000ULL )
#define LV2_HTAB_BASE ( 0x800000000F000000ULL )
#define LV2_HTAB_SIZE ( 0x40000 )
#define LV2_HV_BASE   ( LV2_BASE + 0x14000000ULL )

#define LIB_LV2_INIT_SUCCESS                ( 0x00001337UL )
#define LIB_LV2_OPD_SEG_NOT_FOUND           ( 0x0000DEADUL )
#define LIB_LV2_SYSCALL_TABLE_NOT_FOUND     ( 0x0000DEAEUL )
#define LIB_LV2_SYSCALL_DISPATCH_NOT_FOUND  ( 0x0000DEAFUL )
#define LIB_LV2_ADDRESS_NOT_FOUND           ( 0x0000DEAFUL )

struct LibLV2Context 
{
    // Table of contents.
    uint64_t kernelTOC;
    uint64_t payloadTOC;

    lv1::ps3_firmware_version firmwareVersion;

    // Last proceedure descriptor inside the .OPD segment.
    OPD_t* kernelLastOPDEntry;
    OPD_t** systemCallTable;

    // TOC function pointers.
    OPD_t* lv2ConsoleGetInstance_opd;
    OPD_t* lv2ConsoleWrite_opd;
    OPD_t* lv2ConsoleWriteAsync_opd;
    OPD_t* lv2ConsolePutc_opd;
    OPD_t* lv2ConsoleFlush_opd;
    OPD_t* lv2Clprintf_opd;
    OPD_t* pageAllocate_opd;
    OPD_t* pageFree_opd;
    OPD_t* kmemExportToProc_opd;
    OPD_t* kmemUnexportFromProc_opd;
    OPD_t* ppuThreadMsgInterruptException_opd;
    OPD_t* processWriteMemory_opd;
    OPD_t* processReadMemory_opd;
    OPD_t* getProcessObjectById_opd;
    OPD_t* idTableUnreserveId_opd;
    OPD_t* extendKstack_opd;
    OPD_t* ppuLoaderLoadProgram_opd;

    uint32_t* systemCallDispatchBranch;
    uint32_t* ppuThreadMsgInterruptExceptionBranch;
    uint64_t ppuThreadMsgInterruptException3rdInstructionAddress;
    uint64_t ppuLoaderLoadProgramBranch;

    int(*lv2_kern_tty_write)(const char* text, size_t length);
    void(*printf)(const char* fmt, ... );
};

extern LibLV2Context g_LibLV2;

uint64_t LibLV2Init(uint64_t TOC, uint64_t payloadTOC);
void LibLV2DebugPrint(const char* format, ...);

#ifdef HAS_DEBUG_PRINTS
    #define DEBUG_PRINT(format, ...) LibLV2DebugPrint(format, ##__VA_ARGS__)
#else
    #define DEBUG_PRINT(format, ...)
#endif

#endif // !LV2_H