#ifndef UTILS_H
#define UTILS_H

#include "ELF.h"

namespace Utils {

    constexpr uint8_t HexCharValue(char character)
    {
        character = character | 32; // to lower

        if (character >= '0' && character <= '9')
            return character - '0';

        if (character >= 'a' && character <= 'f')
            return (character - 'a' ) + 10;

        return 0xFF;
    }

    template <typename T>
    struct ClassFnPtrCastT;

    template <typename R, typename C, typename... A>
    struct ClassFnPtrCastT< R( C::* )( A... ) >
    {
        constexpr auto operator()( C* self, OPD_t* address ) const 
        {
            using Fn = R(*)( C*, A... );
            return [address, self] ( A... args ) -> R {
                return __builtin_bit_cast( Fn, address )( self, args... );
            };
        }
    };

    template <typename T>
    constexpr ClassFnPtrCastT<T> ClassFnPtrCast;
}

#define CLASS_FN(T, instance, address) Utils::ClassFnPtrCast<decltype(T)>(instance, address)
#define STATIC_FN(T, address) ((decltype(T))address)

template <typename R, typename... TArgs>
inline __attribute__((always_inline)) R Lv2Call(uint64_t addr, TArgs... args)
{
    volatile OPD_t opd = { addr, g_LibLV2.kernelTOC, 0 };
    R(*func)(TArgs...) = (R(*)(TArgs...))&opd;
    return func(args...);
}

#endif // !UTILS_H