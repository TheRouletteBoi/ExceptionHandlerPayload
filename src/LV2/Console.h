#include "../Utils.h"

namespace lv2 {

class console 
{
public:
    static console* get_instance()
    { 
        return STATIC_FN(&get_instance, g_LibLV2.lv2ConsoleGetInstance_opd)();
    }

    int putc(int character)
    { 
        return CLASS_FN(&console::putc, this, g_LibLV2.lv2ConsolePutc_opd)(character);
    }

    void write(const char* text, int length)
    {
        CLASS_FN(&console::write, this, g_LibLV2.lv2ConsoleWrite_opd)(text, length);
    }

    void write_async(const char* text, int length)
    { 
        CLASS_FN(&console::write_async, this, g_LibLV2.lv2ConsoleWriteAsync_opd)(text, length);
    }

    int flush()
    {
        return CLASS_FN(&console::flush, this, g_LibLV2.lv2ConsoleFlush_opd)();
    }
};


}