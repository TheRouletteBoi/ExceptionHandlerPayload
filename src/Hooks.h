#include "Types.h"
#include "ELF.h"

struct SystemCallContext 
{
    uint64_t args[ 8 ];
    uint64_t index;
    OPD_t    handler;
};

void InstallHooks();