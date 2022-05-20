#ifndef LV2_PROCESS_H
#define LV2_PROCESS_H

#include "../Types.h"

namespace lv2 {

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


}

#endif // !LV2_PROCESS_H