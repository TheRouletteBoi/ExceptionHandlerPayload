#ifndef LV2_STRING_H
#define LV2_STRING_H

#include "../Types.h"
#include "../Utils.h"

namespace lv2 {

using PrintfCallback_t = int(*)(const char* string, size_t length, void* arg);

static uint64_t clprintf(const char* format, va_list arguments, PrintfCallback_t printfCallback, void* argument) 
{
    return STATIC_FN(&clprintf, g_LibLV2.lv2Clprintf_opd)(format, arguments, printfCallback, argument);
}


}

#endif // !LV2_STRING_H