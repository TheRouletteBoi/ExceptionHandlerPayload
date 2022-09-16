#include "LV2/LV2.h"

#include "Hooks.h"
#include "PatternScanner.h"
#include "PPC.h"
#include "SpinLock.h"
#include "Utils.h"

#include "LV2/Scheduler.h"
#include "LV2/Process.h"
#include "LV2/Thread.h"

/***
 * TODO
 * hook syscall load module and check path for hdd0/your_game then force load the sprx using this hook method :)
 * hook into syscall 905 on CEX only consoles and use ps3mapi write process to make dex menus work on cex
 * 
 * 
 */ 

extern "C" uint64_t PayloadEntry(uint64_t TOC, uint64_t payloadTOC)
{	
    const uint64_t error = LibLV2Init(TOC, payloadTOC);
    if (error != LIB_LV2_INIT_SUCCESS)
        return LIB_LV2_INIT_SUCCESS;

    #ifdef INSTALL_HOOKS
        InstallHooks();
    #endif

    DEBUG_PRINT("Payload Loaded Firmware: %u.%u.%u\n", g_LibLV2.firmwareVersion.major, g_LibLV2.firmwareVersion.minor, g_LibLV2.firmwareVersion.revision);
    DEBUG_PRINT("RTOC: 0x%llx KRTOC: 0x%llx\n", g_LibLV2.payloadTOC, g_LibLV2.kernelTOC);
    DEBUG_PRINT("System Call Table: 0x%llx\n", g_LibLV2.systemCallTable);

    return 0;
}