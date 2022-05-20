#include "../Types.h"

//
// https://kib.kiev.ua/x86docs/POWER/PPC_Vers202_Book3_public.pdf
//
// https://www.cresco.enea.it/LA1/cresco_sp14_ylichron/CBE-docs/CellBE_PXCell_Handbook_v1.11_12May08_pub.pdf
//
union HashTableEntry {
    struct {
        //
        // First 64 bits here.
        //
        uint64_t Reserved : 15;	// 0:14  Software must set these bits to ‘0’ or results are undefined.
        uint64_t AVPN : 42;		// 15:56 Abbreviated Virtual Page Number (vsid | api == avpn)
        uint64_t SW : 4;	    // 57:60 Available for software use
        uint64_t L : 1;			// 61    Large Page Indicator ( large (L=1) or 4 KB (L=0) )
        uint64_t H : 1;			// 62    Hash Function Identifier
        uint64_t V : 1;			// 63    Valid (1=valid, 0=invalid)

        //
        // Second half of PTE starts here.
        //
        uint64_t Reserved2 : 2;	// 64:85   Software must set these bits to ‘0’ or results are undefined.
        uint64_t RPN : 50;		// 86:115  Real Page Number
        uint64_t Reserved3 : 2;	// 116:117 Software must set these bits to ‘0’ or results are undefined.
        uint64_t AC : 1;	    // 118     Address Compare bit
        uint64_t R : 1;			// 119     Reference bit
        uint64_t C : 1;			// 120     Change bit
        uint64_t W : 1;			// 121     Write-through. Hardware always treats this as '0'
        uint64_t I : 1;			// 122     Caching-inhibited bit
        uint64_t M : 1;			// 123     Memory Coherence. Hardware always treats this as '0'
        uint64_t G : 1;			// 124     Guarded bit
        uint64_t N : 1;			// 125     No Execute bit (1=no-execute, 0=allow execute)
        uint64_t PP : 2;		// 126:127 Page Protection bits
    };
    uint64_t Packed[ 2 ];
};
