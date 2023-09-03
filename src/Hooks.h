#include "Types.h"
#include "ELF.h"

struct SystemCallContext 
{
    uint64_t args[ 8 ];
    uint64_t index;
    OPD_t    handler;


    template <typename T>
    T GetArg(uint8_t parameterIndex)
    {
        return (T)args[parameterIndex];
    }

    template <typename T>
    void SetArg(uint8_t parameterIndex, T value)
    {
        *reinterpret_cast<T*>(&args[parameterIndex]) = value;
    }

};

void InstallHooks();