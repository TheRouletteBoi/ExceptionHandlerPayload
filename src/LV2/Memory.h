
#include "../Types.h"
#include "../Utils.h"

namespace lv2 {

class process;

enum page_size 
{
    PAGE_SIZE_4K = 0x100,
    PAGE_SIZE_64K = 0x200,
    PAGE_SIZE_1M = 0x300
};

static inline uint32_t page_allocate(lv2::process* process, uint64_t numBytes, uint64_t pageID, 
    page_size pageSize, void** pageAddressOut)
{
    return STATIC_FN(&page_allocate, g_LibLV2.pageAllocate_opd)(process, numBytes, pageID, pageSize, pageAddressOut);
}

static inline uint32_t page_free(lv2::process* process, void* pageAddress, uint64_t pageID)
{
    return STATIC_FN(&page_free, g_LibLV2.pageFree_opd)(process, pageAddress, pageID);
}

static inline uint32_t kmem_export_to_proc(lv2::process* process, void* pageAddress, uint64_t flags, uint64_t* pageVirtualAddressOut)
{
    return STATIC_FN(&kmem_export_to_proc, g_LibLV2.kmemExportToProc_opd)(process, pageAddress, flags, pageVirtualAddressOut);
}

static inline uint32_t kmem_unexport_from_proc(lv2::process* process, uint64_t pageVirtualAddress)
{
    return STATIC_FN(&kmem_unexport_from_proc, g_LibLV2.kmemUnexportFromProc_opd)(process, pageVirtualAddress);
}


}