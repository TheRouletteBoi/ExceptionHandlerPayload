#ifndef LV2_PROCESS_H
#define LV2_PROCESS_H

#include "../Types.h"
#include "../Utils.h"

namespace lv2 {

typedef uint32_t sys_pid_t;

class process 
{
public:
    void*       systemCallTable; // 0x00
    char        _0x08[0x28 - 0x8];
    uint32_t    processID; // 0x28
    int         status;    // 0x2C
    char        _0x30[0xB0 - 0x30];
    process*    parentProcess; // 0xB0
    process*    childProcess; // 0xB8
    process*    nextSiblingProcess; // 0xC0
    uint64_t    numChildren; // 0xC8
    char        _0xD0[0x160 - 0xD0];
    const char* imageName;
};

static inline int get_process_object_by_id(sys_pid_t pid, process** outProcessObject, void** outProcessHandle)
{
    return STATIC_FN(&get_process_object_by_id, g_LibLV2.getProcessObjectById_opd)(pid, outProcessObject, outProcessHandle);
}

static inline int id_table_unreserve_id(void* objectList, void* processHandle)
{
    return STATIC_FN(&id_table_unreserve_id, g_LibLV2.idTableUnreserveId_opd)(objectList, processHandle);
}

static inline int unreserve_process_handle(void* processHandle)
{
    void** processList = (void**)(*(uint64_t*)(g_LibLV2.kernelTOC + 0x20E0)); // 0x20E0 offset varies per firmware. TODO: find offset using patter scanner
    return id_table_unreserve_id(*processList, processHandle);
}

static inline process* get_process_by_pid(sys_pid_t pid)
{
    process* processObject = NULL;
    void*    processHandle = NULL;

    auto error = get_process_object_by_id(pid, &processObject, &processHandle);

    if (error)
    {
        DEBUG_PRINT( "lv2::get_process_object_entry_and_address_by_id error: %x\n", error);
        return NULL;
    }

    return processObject;
}

static inline uint32_t process_write_memory(lv2::process* process, void* destination, void* source, size_t size, int flag)
{
    return STATIC_FN(&process_write_memory, g_LibLV2.processWriteMemory_opd)(process, destination, source, size, flag);
}

static inline uint32_t process_read_memory(lv2::process* process, void* destination, void* source, size_t size)
{
    return STATIC_FN(&process_read_memory, g_LibLV2.processReadMemory_opd)(process, destination, source, size);
}

/* 
* Extends the kernel stack of the syscall in 4096 bytes. Can only be called once in a syscall
**/
static inline void extend_kstack(uint64_t a1)
{
    return STATIC_FN(&extend_kstack, g_LibLV2.extendKstack_opd)(a1);
}

}

#endif // !LV2_PROCESS_H