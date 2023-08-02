#include "../Types.h"

namespace lv2 {

typedef int32_t  sys_prx_id_t;
typedef uint64_t sys_prx_flags_t;

typedef uint32_t sys_prx_user_pchar_t;
typedef uint32_t sys_prx_user_segment_vector_t;
typedef uint32_t sys_prx_user_libent_addr_t;
typedef uint32_t sys_prx_user_libstub_addr_t;
typedef uint32_t sys_prx_user_p_prx_id_t;
typedef uint32_t sys_prx_user_p_const_void_t;
typedef uint32_t sys_prx_user_p_stop_level_t;

typedef struct 
{
    uint64_t base;				/* segment base */
    uint64_t fileSize;			/* segment size */
    uint64_t memorySize;		/* segment size */
    uint64_t index;				/* segment index */
    uint64_t type;				/* segment type == ELF type */
} sys_prx_segment_info_t;

typedef struct sys_prx_module_info_t 
{
    uint64_t                      size;
    char                          name[30];
    char                          version[2];
    uint32_t                      modAttribute;
    uint32_t                      startEntry;
    uint32_t                      stopEntry;
    uint32_t                      numberOfAllSegments;
    sys_prx_user_pchar_t          fileName;
    uint32_t                      fileNameSize;
    sys_prx_user_segment_vector_t segments;
    uint32_t                      numberOfSegments;
} sys_prx_module_info_t;

typedef struct sys_prx_module_info_option_t 
{
    uint64_t size;
    union 
    {
        uint32_t moduleInfo32;
        uint64_t moduleInfo64;
    };
} sys_prx_module_info_option_t;

typedef struct sys_prx_unload_module_option_t 
{
    uint64_t size;				/* sizeof(this) */
} sys_prx_unload_module_option_t;

typedef struct sys_prx_load_module_option_t 
{
    uint64_t size;				/* sizeof(this) */
} sys_prx_load_module_option_t;



}