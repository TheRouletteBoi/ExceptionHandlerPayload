#ifndef LV1_CALLS_H
#define LV1_CALLS_H

#include "../Types.h"

#define HYPERCALL_0(hyperCallIndex)                                                                 \
register uint64_t R3  __asm__ ("3");                                                                \
register uint64_t R4  __asm__ ("4");                                                                \
register uint64_t R5  __asm__ ("5");                                                                \
register uint64_t R6  __asm__ ("6");                                                                \
register uint64_t R7  __asm__ ("7");                                                                \
register uint64_t R8  __asm__ ("8");                                                                \
register uint64_t R9  __asm__ ("9");                                                                \
register uint64_t R10 __asm__ ("10");                                                               \
register uint64_t R11 __asm__ ("11") = hyperCallIndex;                                              \
                                                                                                    \
__asm__ volatile ( "sc 1"                                                                           \
                 : "=r" ( R3 ), "=r" ( R4 ), "=r" ( R5 ), "=r" ( R6 ),                              \
                   "=r" ( R7 ), "=r" ( R8 ), "=r" ( R9 ), "=r" ( R10 ), "=r" ( R11 )                \
                 : "r"  ( R11 )                                                                     \
                 : "lr", "0", "12", "ctr", "xer", "cr0", "cr1", "cr5", "cr6", "cr7", "memory" );    \

#define HYPERCALL_1(hyperCallIndex, arg1)                                                           \
register uint64_t R3  __asm__ ("3") = (uint64_t)arg1;                                               \
register uint64_t R4  __asm__ ("4");                                                                \
register uint64_t R5  __asm__ ("5");                                                                \
register uint64_t R6  __asm__ ("6");                                                                \
register uint64_t R7  __asm__ ("7");                                                                \
register uint64_t R8  __asm__ ("8");                                                                \
register uint64_t R9  __asm__ ("9");                                                                \
register uint64_t R10 __asm__ ("10");                                                               \
register uint64_t R11 __asm__ ("11") = hyperCallIndex;                                              \
                                                                                                    \
__asm__ volatile ( "sc 1"                                                                           \
                 : "=r" ( R3 ), "=r" ( R4 ), "=r" ( R5 ), "=r" ( R6 ),                              \
                   "=r" ( R7 ), "=r" ( R8 ), "=r" ( R9 ), "=r" ( R10 ), "=r" ( R11 )                \
                 : "r"  ( R11 )                                                                     \
                 : "lr", "0", "12", "ctr", "xer", "cr0", "cr1", "cr5", "cr6", "cr7", "memory" );    \

#define HYPERCALL_2(hyperCallIndex, arg1, arg2)                                                     \
register uint64_t R3  __asm__ ("3") = (uint64_t)arg1;                                               \
register uint64_t R4  __asm__ ("4") = (uint64_t)arg2;                                               \
register uint64_t R5  __asm__ ("5");                                                                \
register uint64_t R6  __asm__ ("6");                                                                \
register uint64_t R7  __asm__ ("7");                                                                \
register uint64_t R8  __asm__ ("8");                                                                \
register uint64_t R9  __asm__ ("9");                                                                \
register uint64_t R10 __asm__ ("10");                                                               \
register uint64_t R11 __asm__ ("11") = hyperCallIndex;                                              \
                                                                                                    \
__asm__ volatile ( "sc 1"                                                                           \
                 : "=r" ( R3 ), "=r" ( R4 ), "=r" ( R5 ), "=r" ( R6 ),                              \
                   "=r" ( R7 ), "=r" ( R8 ), "=r" ( R9 ), "=r" ( R10 ), "=r" ( R11 )                \
                 : "r"  ( R11 )                                                                     \
                 : "lr", "0", "12", "ctr", "xer", "cr0", "cr1", "cr5", "cr6", "cr7", "memory" );    \

#define HYPERCALL_3(hyperCallIndex, arg1, arg2, arg3)                                               \
register uint64_t R3  __asm__ ("3") = (uint64_t)arg1;                                               \
register uint64_t R4  __asm__ ("4") = (uint64_t)arg2;                                               \
register uint64_t R5  __asm__ ("5") = (uint64_t)arg3;                                               \
register uint64_t R6  __asm__ ("6");                                                                \
register uint64_t R7  __asm__ ("7");                                                                \
register uint64_t R8  __asm__ ("8");                                                                \
register uint64_t R9  __asm__ ("9");                                                                \
register uint64_t R10 __asm__ ("10");                                                               \
register uint64_t R11 __asm__ ("11") = hyperCallIndex;                                              \
                                                                                                    \
__asm__ volatile ( "sc 1"                                                                           \
                 : "=r" ( R3 ), "=r" ( R4 ), "=r" ( R5 ), "=r" ( R6 ),                              \
                   "=r" ( R7 ), "=r" ( R8 ), "=r" ( R9 ), "=r" ( R10 ), "=r" ( R11 )                \
                 : "r"  ( R11 )                                                                     \
                 : "lr", "0", "12", "ctr", "xer", "cr0", "cr1", "cr5", "cr6", "cr7", "memory" );    \

#define HYPERCALL_4(hyperCallIndex, arg1, arg2, arg3, arg4)                                         \
register uint64_t R3  __asm__ ("3") = (uint64_t)arg1;                                               \
register uint64_t R4  __asm__ ("4") = (uint64_t)arg2;                                               \
register uint64_t R5  __asm__ ("5") = (uint64_t)arg3;                                               \
register uint64_t R6  __asm__ ("6") = (uint64_t)arg4;                                               \
register uint64_t R7  __asm__ ("7");                                                                \
register uint64_t R8  __asm__ ("8");                                                                \
register uint64_t R9  __asm__ ("9");                                                                \
register uint64_t R10 __asm__ ("10");                                                               \
register uint64_t R11 __asm__ ("11") = hyperCallIndex;                                              \
                                                                                                    \
__asm__ volatile ( "sc 1"                                                                           \
                 : "=r" ( R3 ), "=r" ( R4 ), "=r" ( R5 ), "=r" ( R6 ),                              \
                   "=r" ( R7 ), "=r" ( R8 ), "=r" ( R9 ), "=r" ( R10 ), "=r" ( R11 )                \
                 : "r"  ( R11 )                                                                     \
                 : "lr", "0", "12", "ctr", "xer", "cr0", "cr1", "cr5", "cr6", "cr7", "memory" );    \

#define HYPERCALL_5(hyperCallIndex, arg1, arg2, arg3, arg4, arg5)                                   \
register uint64_t R3  __asm__ ("3") = (uint64_t)arg1;                                               \
register uint64_t R4  __asm__ ("4") = (uint64_t)arg2;                                               \
register uint64_t R5  __asm__ ("5") = (uint64_t)arg3;                                               \
register uint64_t R6  __asm__ ("6") = (uint64_t)arg4;                                               \
register uint64_t R7  __asm__ ("7") = (uint64_t)arg5;                                               \
register uint64_t R8  __asm__ ("8");                                                                \
register uint64_t R9  __asm__ ("9");                                                                \
register uint64_t R10 __asm__ ("10");                                                               \
register uint64_t R11 __asm__ ("11") = hyperCallIndex;                                              \
                                                                                                    \
__asm__ volatile ( "sc 1"                                                                           \
                 : "=r" ( R3 ), "=r" ( R4 ), "=r" ( R5 ), "=r" ( R6 ),                              \
                   "=r" ( R7 ), "=r" ( R8 ), "=r" ( R9 ), "=r" ( R10 ), "=r" ( R11 )                \
                 : "r"  ( R11 )                                                                     \
                 : "lr", "0", "12", "ctr", "xer", "cr0", "cr1", "cr5", "cr6", "cr7", "memory" );    \

#define HYPERCALL_6(hyperCallIndex, arg1, arg2, arg3, arg4, arg5, arg6)                             \
register uint64_t R3  __asm__ ("3") = (uint64_t)arg1;                                               \
register uint64_t R4  __asm__ ("4") = (uint64_t)arg2;                                               \
register uint64_t R5  __asm__ ("5") = (uint64_t)arg3;                                               \
register uint64_t R6  __asm__ ("6") = (uint64_t)arg4;                                               \
register uint64_t R7  __asm__ ("7") = (uint64_t)arg5;                                               \
register uint64_t R8  __asm__ ("8") = (uint64_t)arg6;                                               \
register uint64_t R9  __asm__ ("9");                                                                \
register uint64_t R10 __asm__ ("10");                                                               \
register uint64_t R11 __asm__ ("11") = hyperCallIndex;                                              \
                                                                                                    \
__asm__ volatile ( "sc 1"                                                                           \
                 : "=r" ( R3 ), "=r" ( R4 ), "=r" ( R5 ), "=r" ( R6 ),                              \
                   "=r" ( R7 ), "=r" ( R8 ), "=r" ( R9 ), "=r" ( R10 ), "=r" ( R11 )                \
                 : "r"  ( R11 )                                                                     \
                 : "lr", "0", "12", "ctr", "xer", "cr0", "cr1", "cr5", "cr6", "cr7", "memory" );    \

#define HYPERCALL_7(hyperCallIndex, arg1, arg2, arg3, arg4, arg5, arg6, arg7)                       \
register uint64_t R3  __asm__ ("3") = (uint64_t)arg1;                                               \
register uint64_t R4  __asm__ ("4") = (uint64_t)arg2;                                               \
register uint64_t R5  __asm__ ("5") = (uint64_t)arg3;                                               \
register uint64_t R6  __asm__ ("6") = (uint64_t)arg4;                                               \
register uint64_t R7  __asm__ ("7") = (uint64_t)arg5;                                               \
register uint64_t R8  __asm__ ("8") = (uint64_t)arg6;                                               \
register uint64_t R9  __asm__ ("9") = (uint64_t)arg7;                                               \
register uint64_t R10 __asm__ ("10");                                                               \
register uint64_t R11 __asm__ ("11") = hyperCallIndex;                                              \
                                                                                                    \
__asm__ volatile ( "sc 1"                                                                           \
                 : "=r" ( R3 ), "=r" ( R4 ), "=r" ( R5 ), "=r" ( R6 ),                              \
                   "=r" ( R7 ), "=r" ( R8 ), "=r" ( R9 ), "=r" ( R10 ), "=r" ( R11 )                \
                 : "r"  ( R11 )                                                                     \
                 : "lr", "0", "12", "ctr", "xer", "cr0", "cr1", "cr5", "cr6", "cr7", "memory" );    \

#define HYPERCALL_8(hyperCallIndex, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)                 \
register uint64_t R3  __asm__ ("3")  = (uint64_t)arg1;                                              \
register uint64_t R4  __asm__ ("4")  = (uint64_t)arg2;                                              \
register uint64_t R5  __asm__ ("5")  = (uint64_t)arg3;                                              \
register uint64_t R6  __asm__ ("6")  = (uint64_t)arg4;                                              \
register uint64_t R7  __asm__ ("7")  = (uint64_t)arg5;                                              \
register uint64_t R8  __asm__ ("8")  = (uint64_t)arg6;                                              \
register uint64_t R9  __asm__ ("9")  = (uint64_t)arg7;                                              \
register uint64_t R10 __asm__ ("10") = (uint64_t)arg8;                                              \
register uint64_t R11 __asm__ ("11") = hyperCallIndex;                                              \
                                                                                                    \
__asm__ volatile ( "sc 1"                                                                           \
                 : "=r" ( R3 ), "=r" ( R4 ), "=r" ( R5 ), "=r" ( R6 ),                              \
                   "=r" ( R7 ), "=r" ( R8 ), "=r" ( R9 ), "=r" ( R10 ), "=r" ( R11 )                \
                 : "r"  ( R11 )                                                                     \
                 : "lr", "0", "12", "ctr", "xer", "cr0", "cr1", "cr5", "cr6", "cr7", "memory" );    \


#define HYPERCALL_RETURN(numOut) \
HyperCallResult<numOut> result; \
result.statusCode = status_code( R3 & 0xFFFFFFFF ); \
switch( numOut )                             \
{                                            \
    case 7: result.SetReturnValue( 6, R10 ); \
    case 6: result.SetReturnValue( 5, R9 );  \
    case 5: result.SetReturnValue( 4, R8 );  \
    case 4: result.SetReturnValue( 3, R7 );  \
    case 3: result.SetReturnValue( 2, R6 );  \
    case 2: result.SetReturnValue( 1, R5 );  \
    case 1: result.SetReturnValue( 0, R4 );  \
    case 0:                                  \
        break;                               \
}                                            \
return result;\

namespace lv1 {

enum class status_code;

template <int NumOut>
struct HyperCallResult
{
    status_code  statusCode;
    uint64_t     returnRegisters[NumOut];

    inline uint64_t GetReturnValue(int index) const
    {
        return returnRegisters[index];
    }

    inline void SetReturnValue(int index, uint64_t value)
    {
        returnRegisters[index] = value;
    }
};

template <int HyperCallIndex, int NumIn, int NumOut>
struct HyperCall { };

template <int HyperCallIndex, int NumOut>
struct HyperCall<HyperCallIndex, 0, NumOut>
{
    __attribute__((noinline)) 
    HyperCallResult<NumOut> operator()() const 
    {
        HYPERCALL_0( HyperCallIndex );
        HYPERCALL_RETURN( NumOut );
    }
};

template <int HyperCallIndex, int NumOut>
struct HyperCall<HyperCallIndex, 1, NumOut>
{
    template<typename T1>
    __attribute__((noinline)) 
    HyperCallResult<NumOut> operator()( T1 arg1 ) const 
    {
        HYPERCALL_1( HyperCallIndex, arg1 );
        HYPERCALL_RETURN( NumOut );
    }
};

template <int HyperCallIndex, int NumOut>
struct HyperCall<HyperCallIndex, 2, NumOut>
{
    template<typename T1, typename T2>
    __attribute__((noinline)) 
    HyperCallResult<NumOut> operator()( T1 arg1, T2 arg2 ) const 
    {
        HYPERCALL_2( HyperCallIndex, arg1, arg2 );
        HYPERCALL_RETURN( NumOut );
    }
};

template<int HyperCallIndex, int NumOut>
struct HyperCall<HyperCallIndex, 3, NumOut>
{
    template<typename T1, typename T2, typename T3>
    __attribute__((noinline)) 
    HyperCallResult<NumOut> operator()( T1 arg1, T2 arg2, T3 arg3 ) const 
    {
        HYPERCALL_3( HyperCallIndex, arg1, arg2, arg3 );
        HYPERCALL_RETURN( NumOut );
    }
};

template<int HyperCallIndex, int NumOut>
struct HyperCall<HyperCallIndex, 4, NumOut>
{
    template<typename T1, typename T2, typename T3, typename T4>
    __attribute__((noinline)) 
    HyperCallResult<NumOut> operator()( T1 arg1, T2 arg2, T3 arg3, T4 arg4 ) const 
    {
        HYPERCALL_4( HyperCallIndex, arg1, arg2, arg3, arg4 );
        HYPERCALL_RETURN( NumOut );
    }
};

template<int HyperCallIndex, int NumOut>
struct HyperCall<HyperCallIndex, 5, NumOut>
{
    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    __attribute__((noinline)) 
    HyperCallResult<NumOut> operator()( T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5 ) const 
    {
        HYPERCALL_5( HyperCallIndex, arg1, arg2, arg3, arg4, arg5 );
        HYPERCALL_RETURN( NumOut );
    }
};

template<int HyperCallIndex, int NumOut>
struct HyperCall<HyperCallIndex, 6, NumOut>
{
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    __attribute__((noinline)) 
    HyperCallResult<NumOut> operator()( T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6 ) const 
    {
        HYPERCALL_6( HyperCallIndex, arg1, arg2, arg3, arg4, arg5, arg6 );
        HYPERCALL_RETURN( NumOut );
    }
};

template<int HyperCallIndex, int NumOut>
struct HyperCall<HyperCallIndex, 7, NumOut>
{
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    __attribute__((noinline)) 
    HyperCallResult<NumOut> operator()( T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7 ) const 
    {
        HYPERCALL_7( HyperCallIndex, arg1, arg2, arg3, arg4, arg5, arg6, arg7 );
        HYPERCALL_RETURN( NumOut );
    }
};

template<int HyperCallIndex, int NumOut>
struct HyperCall<HyperCallIndex, 8, NumOut>
{
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    __attribute__((noinline)) 
    HyperCallResult<NumOut> operator()( T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8 ) const 
    {
        HYPERCALL_8( HyperCallIndex, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8 );
        HYPERCALL_RETURN( NumOut );
    }
};


}

#endif // !LV1_CALLS_H