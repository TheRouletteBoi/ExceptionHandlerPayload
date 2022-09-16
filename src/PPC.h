#ifndef POWER_PC_H
#define POWER_PC_H

#include "Types.h"
#include "Libc.h"
#include "LV2/LV2.h"

#define MASK_N_BITS(N) ( ( 1 << ( N ) ) - 1 )

#define POWERPC_HI(X) ( ( X >> 16 ) & 0xFFFF )
#define POWERPC_LO(X) ( X & 0xFFFF )

// PowerPC most significant bit is addressed as bit 0 in documentation.
#define POWERPC_BIT32(N) ( 31 - N )

// Opcode is bits 0-5. 
// Allowing for op codes ranging from 0-63.
#define POWERPC_OPCODE(OP)       ( OP << 26 )
#define POWERPC_OPCODE_ADDI      POWERPC_OPCODE( 14 )
#define POWERPC_OPCODE_ADDIS     POWERPC_OPCODE( 15 )
#define POWERPC_OPCODE_BC        POWERPC_OPCODE( 16 )
#define POWERPC_OPCODE_B         POWERPC_OPCODE( 18 )
#define POWERPC_OPCODE_BCCTR     POWERPC_OPCODE( 19 )
#define POWERPC_OPCODE_ORI       POWERPC_OPCODE( 24 )
#define POWERPC_OPCODE_EXTENDED  POWERPC_OPCODE( 31 ) // Use extended opcodes.
#define POWERPC_OPCODE_STW       POWERPC_OPCODE( 36 )
#define POWERPC_OPCODE_LWZ       POWERPC_OPCODE( 32 )
#define POWERPC_OPCODE_LD        POWERPC_OPCODE( 58 )
#define POWERPC_OPCODE_STD       POWERPC_OPCODE( 62 )
#define POWERPC_OPCODE_MASK      POWERPC_OPCODE( 63 )

#define POWERPC_EXOPCODE(OP)     ( OP << 1 )
#define POWERPC_EXOPCODE_BCCTR   POWERPC_EXOPCODE( 528 )
#define POWERPC_EXOPCODE_MTSPR   POWERPC_EXOPCODE( 467 )

#define POWERPC_ADDI(rD, rA, SIMM)  (uint32_t)( POWERPC_OPCODE_ADDI | ( rD << POWERPC_BIT32( 10 ) ) | ( rA << POWERPC_BIT32( 15 ) ) | SIMM )
#define POWERPC_ADDIS(rD, rA, SIMM) (uint32_t)( POWERPC_OPCODE_ADDIS | ( rD << POWERPC_BIT32( 10 ) ) | ( rA << POWERPC_BIT32( 15 ) ) | SIMM )
#define POWERPC_B(LI, AA, LK)       (uint32_t)( POWERPC_OPCODE_B | ( LI & ( MASK_N_BITS( 24 ) << 2 ) ) | ( ( AA & 1 ) << 1 ) | ( LK & 1 ) )
#define POWERPC_BCCTR(BO, BI, LK)   (uint32_t)( POWERPC_OPCODE_BCCTR | ( BO << POWERPC_BIT32( 10 ) ) | ( BI << POWERPC_BIT32( 15 ) ) | ( LK & 1 ) | POWERPC_EXOPCODE_BCCTR )
#define POWERPC_LIS(rD, SIMM)       POWERPC_ADDIS( rD, 0, SIMM ) // Mnemonic for addis %rD, 0, SIMM
#define POWERPC_LI(rD, SIMM)        POWERPC_ADDI( rD, 0, SIMM )  // Mnemonic for addi %rD, 0, SIMM
#define POWERPC_MTSPR(SPR, rS)      (uint32_t)( POWERPC_OPCODE_EXTENDED | ( rS << POWERPC_BIT32( 10 ) ) | ( POWERPC_SPR( SPR ) << POWERPC_BIT32( 20 ) ) | POWERPC_EXOPCODE_MTSPR )
#define POWERPC_MTCTR(rS)           POWERPC_MTSPR( 9, rS ) // Mnemonic for mtspr 9, rS
#define POWERPC_ORI(rS, rA, UIMM)   (uint32_t)( POWERPC_OPCODE_ORI | ( rS << POWERPC_BIT32( 10 ) ) | ( rA << POWERPC_BIT32( 15 ) ) | UIMM )
#define POWERPC_STD(rS, DS, rA)     (uint32_t)( POWERPC_OPCODE_STD | ( rS << POWERPC_BIT32( 10 ) ) | ( rA << POWERPC_BIT32( 15 ) ) | ( (INT16)DS & 0xFFFF ) )
#define POWERPC_LD(rS, DS, rA)      (uint32_t)( POWERPC_OPCODE_LD | ( rS << POWERPC_BIT32( 10 ) ) | ( rA << POWERPC_BIT32( 15 ) ) | ( (INT16)DS & 0xFFFF ) )

// Branch related fields.
#define POWERPC_BRANCH_LINKED    1
#define POWERPC_BRANCH_ABSOLUTE  2
#define POWERPC_BRANCH_TYPE_MASK ( POWERPC_BRANCH_LINKED | POWERPC_BRANCH_ABSOLUTE )

#define POWERPC_BRANCH_CONDITION_MASK ( 0x001F0000 )
#define POWERPC_BRANCH_CONDITION_MASK ( 0x001F0000 )

NO_INLINE static uint64_t PPCGetConditionalBranchAddress(uint64_t instructionAddress, uint32_t instruction) 
{
    //  BC - Branch Conditional (BEQ BNE BLT BGE)
    //  [Opcode]   [Branch Options]     [Condition Register]         [Address]      [Absolute] [Linked]
    //   0-5            6-10                    11-15                   16-29            30        31
    //  
    //  Example
    //  010000         00100                   00001              0000 0000 0000 01      0         0

    if ((instruction & 0xFC000000) != POWERPC_OPCODE_BC)
        return 0;

    if (instruction & POWERPC_BRANCH_ABSOLUTE)
        return instruction & ~(0x0000FFFC);

    intptr_t   branchOffset = instruction & 0x0000FFFC;

    // If The MSB for branch offset is set make it negative.
    if (branchOffset & (1 << 15 ))
        branchOffset |= ~0x0000FFFF;

    return uint64_t(intptr_t(instructionAddress + branchOffset));
}

NO_INLINE static uint64_t PPCGetBranchAddress(uint64_t instructionAddress, uint32_t instruction)
{
    if ((instruction & 0xFC000000) != POWERPC_OPCODE_B)
        return 0;

    //  B - Branch (B BL BA BLA)
    //  [Opcode]       [Address]              [Absolute] [Linked]
    //    0-5            6-29                     30        31
    //
    //  Example
    //  010010    0000 0000 0000 0000 0000 0001   0         0

    if (instruction & POWERPC_BRANCH_ABSOLUTE) 
        return instruction & ~(POWERPC_OPCODE_B | POWERPC_BRANCH_TYPE_MASK);

    intptr_t branchOffset = instruction & 0x03FFFFFC;

    // If The MSB for branch offset is set make it negative.
    if (branchOffset & (1 << 25)) 
        branchOffset |= ~0x03FFFFFF;

    return uint64_t(intptr_t(instructionAddress + branchOffset));
}

static inline uint64_t PPCGetBranchAddress(uint64_t instructionAddress)
{
    return PPCGetBranchAddress(instructionAddress, *(uint32_t*)instructionAddress);
}

static inline uint64_t PPCGetConditionalBranchAddress(uint64_t instructionAddress
)
{
    return PPCGetBranchAddress(instructionAddress, *(uint32_t*)instructionAddress);
}

static inline uint64_t PPCGetAnyBranchAddress(uint64_t instructionAddress)
{
    const auto address = PPCGetBranchAddress(instructionAddress);

    if (address == 0)
        return PPCGetConditionalBranchAddress(instructionAddress);

    return address;
}

#define __mtspr(SPR, value) \
  __asm__ volatile ("mtspr %0,%1" : : "n" (SPR), "r" (value))
  
#define __mfspr(SPR) __extension__				\
  ({ unsigned long long __macro_result;					\
  __asm__ volatile ("mfspr %0,%1" : "=r" (__macro_result) : "n" (SPR)); \
  __macro_result; })

#define POWERPC_SPR_SRR0 26
#define POWERPC_SPR_SRR1 27

#define POWERPC_SPR_SPRG0 272
#define POWERPC_SPR_SPRG1 273
#define POWERPC_SPR_SPRG2 274
#define POWERPC_SPR_SPRG3 275

#define __mfsprg0() __mfspr( POWERPC_SPR_SPRG0 )
#define __mfsprg1() __mfspr( POWERPC_SPR_SPRG1 )
#define __mfsprg2() __mfspr( POWERPC_SPR_SPRG2 )
#define __mfsprg3() __mfspr( POWERPC_SPR_SPRG3 )

#define __mtsprg0(value) __mtspr( POWERPC_SPR_SPRG0, value )
#define __mtsprg1(value) __mtspr( POWERPC_SPR_SPRG1, value )
#define __mtsprg2(value) __mtspr( POWERPC_SPR_SPRG2, value )
#define __mtsprg3(value) __mtspr( POWERPC_SPR_SPRG3, value )

#define __mfmsrd(L) __extension__				\
  ({ unsigned long long __macro_result;					\
  __asm__ volatile ("mfmsrd %0,%1" : "=r" (__macro_result) : "n" (L)); \
  __macro_result; })

#define __mtmsrd(value, L) \
  __asm__ volatile ("mtmsrd %0,%1" : : "r" (value), "n" (L))
  
#define __icbi(base) \
  __asm__ volatile ("icbi %y0" : "=m" (base) : : "memory")

static inline void PPCFlushInstructionCache(void* address, size_t length)
{
    auto flushEnd = (char *)address + length;

    // Allign address by 128
    auto flushStart = uint64_t(address) & ~127uLL;

    for (char* cacheLine = (char*)flushStart; cacheLine < flushEnd; cacheLine += 128LL)
    {
        // Flush data cache.
        __builtin_ppc_dcbst(cacheLine);
        __builtin_ppc_sync();

        // Flush instruction cache.
        __icbi(cacheLine);
        __builtin_ppc_isync();
    }
}

static inline void PPCWriteBranchRelative(uint32_t* instruction, uint64_t branchDestination, bool linked)
{
    intptr_t offset = (branchDestination - (uint64_t)instruction);

    *instruction = POWERPC_B(offset, 0, linked);

    PPCFlushInstructionCache(instruction, 4);
}

static void PatchInJump(uint64_t address, uint64_t destination, bool linked)
{
    uint32_t stubInstructions[7] = { 0 };

    stubInstructions[0] = 0x3D600000 + ((destination >> 48) & 0xFFFF); // Load Destination First 2 Bytes 0x1234

    stubInstructions[1] = 0x616B0000 + ((destination >> 32) & 0xFFFF); // Load Destination Next 2 Bytes 0x5678

    stubInstructions[2] = 0x796B07C6; // Shift That To The Upper Part (IE 0x0000 0000 1234 5678 Becomes 0x1234567800000000)

    stubInstructions[3] = 0x656B0000 + ((destination >> 16) & 0xFFFF); // Load Destination Next 2 Bytes 0x8765

    stubInstructions[4] = 0x616B0000 + (destination & 0xFFFF); // Load Destination Next 2 Bytes 0x4321

    stubInstructions[5] = 0x7D6903A6; // mtctr %r11

    stubInstructions[6] = 0x4E800420;

    memcpy((void*)address, stubInstructions, sizeof(stubInstructions));

    PPCFlushInstructionCache((void*)address, 4 * 7);
}

static void HookFunctionStart(uint64_t address, uint64_t destination, uint64_t stub)
{
    uint32_t stubInstructions[14] = { 0 };
    uint64_t branchAddress = (address + (4 * 7)); // Address + 7 Instructions

    stubInstructions[0] = 0x3D600000 + ((branchAddress >> 48) & 0xFFFF); // Load Destination First 2 Bytes 0x1234

    stubInstructions[1] = 0x616B0000 + ((branchAddress >> 32) & 0xFFFF); // Load Destination Next 2 Bytes 0x5678

    stubInstructions[2] = 0x796B07C6; // Shift That To The Upper Part (IE 0x0000 0000 1234 5678 Becomes 0x1234567800000000)

    stubInstructions[3] = 0x656B0000 + ((branchAddress >> 16) & 0xFFFF); // Load Destination Next 2 Bytes 0x8765

    stubInstructions[4] = 0x616B0000 + (branchAddress & 0xFFFF); // Load Destination Next 2 Bytes 0x4321

    stubInstructions[5] = 0x7D6903A6; // mtctr %r11

    for (int i = 0; i < 7; ++i)
    {
        stubInstructions[i + 6] = *(uint32_t*)(address + (4 * i));  // Store Original Instructions
    }

    stubInstructions[13] = 0x4E800420;

    memcpy((void*)stub, stubInstructions, sizeof(stubInstructions));
    PPCFlushInstructionCache((void*)stub, 4 * 13);

    PatchInJump(address, destination, false);
}




#define LV2_HOOKED_FUNCTION(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 2, 40(1)\n" \
"	bl +4\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -16\n" \
"	add  2, 2, 0\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	nop\n" \
"	nop\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_PRECALL_0(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	nop\n" \
"	nop\n" \
"	std 2, 40(1)\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -16\n" \
"	add  2, 2, 0\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_PRECALL_1(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 3, 48(1)\n" \
"	nop\n" \
"	nop\n" \
"	std 2, 40(1)\n" \
"	ld 3, 48(1)\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -20\n" \
"	add  2, 2, 0\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_PRECALL_2(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	nop\n" \
"	nop\n" \
"	std 2, 40(1)\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -24\n" \
"	add  2, 2, 0\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_PRECALL_3(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	std 5, 64(1)\n" \
"	nop\n" \
"	nop\n" \
"	std 2, 40(1)\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	ld 5, 64(1)\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -28\n" \
"	add  2, 2, 0\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_PRECALL_4(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	std 5, 64(1)\n" \
"	std 6, 72(1)\n" \
"	nop\n" \
"	nop\n" \
"	std 2, 40(1)\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	ld 5, 64(1)\n" \
"	ld 6, 72(1)\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -32\n" \
"	add  2, 2, 0\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_PRECALL_5(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	std 5, 64(1)\n" \
"	std 6, 72(1)\n" \
"	std 7, 80(1)\n" \
"	nop\n" \
"	nop\n" \
"	std 2, 40(1)\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	ld 5, 64(1)\n" \
"	ld 6, 72(1)\n" \
"	ld 7, 80(1)\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -36\n" \
"	add  2, 2, 0\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_PRECALL_6(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	std 5, 64(1)\n" \
"	std 6, 72(1)\n" \
"	std 7, 80(1)\n" \
"	std 8, 88(1)\n" \
"	nop\n" \
"	nop\n" \
"	std 2, 40(1)\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	ld 5, 64(1)\n" \
"	ld 6, 72(1)\n" \
"	ld 7, 80(1)\n" \
"	ld 8, 88(1)\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -40\n" \
"	add  2, 2, 0\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_PRECALL_7(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	std 5, 64(1)\n" \
"	std 6, 72(1)\n" \
"	std 7, 80(1)\n" \
"	std 8, 88(1)\n" \
"	std 9, 96(1)\n" \
"	nop\n" \
"	nop\n" \
"	std 2, 40(1)\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	ld 5, 64(1)\n" \
"	ld 6, 72(1)\n" \
"	ld 7, 80(1)\n" \
"	ld 8, 88(1)\n" \
"	ld 9, 96(1)\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -44\n" \
"	add  2, 2, 0\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_PRECALL_8(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	std 5, 64(1)\n" \
"	std 6, 72(1)\n" \
"	std 7, 80(1)\n" \
"	std 8, 88(1)\n" \
"	std 9, 96(1)\n" \
"	std 10, 104(1)\n" \
"	nop\n" \
"	nop\n" \
"	std 2, 40(1)\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	ld 5, 64(1)\n" \
"	ld 6, 72(1)\n" \
"	ld 7, 80(1)\n" \
"	ld 8, 88(1)\n" \
"	ld 9, 96(1)\n" \
"	ld 10, 104(1)\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -48\n" \
"	add  2, 2, 0\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_PRECALL_SUCCESS_0(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	nop\n" \
"	nop\n" \
"	cmpwi 3, 0\n" \
"	bne 1f\n" \
"	std 2, 40(1)\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -16\n" \
"	add  2, 2, 0\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"1:\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_PRECALL_SUCCESS_1(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 3, 48(1)\n" \
"	nop\n" \
"	nop\n" \
"	cmpwi 3, 0\n" \
"	bne 1f\n" \
"	std 2, 40(1)\n" \
"	ld 3, 48(1)\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -20\n" \
"	add  2, 2, 0\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"1:\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_PRECALL_SUCCESS_2(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	nop\n" \
"	nop\n" \
"	cmpwi 3, 0\n" \
"	bne 1f\n" \
"	std 2, 40(1)\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -24\n" \
"	add  2, 2, 0\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"1:\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_PRECALL_SUCCESS_3(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	std 5, 64(1)\n" \
"	nop\n" \
"	nop\n" \
"	cmpwi 3, 0\n" \
"	bne 1f\n" \
"	std 2, 40(1)\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	ld 5, 64(1)\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -28\n" \
"	add  2, 2, 0\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"1:\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_PRECALL_SUCCESS_4(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	std 5, 64(1)\n" \
"	std 6, 72(1)\n" \
"	nop\n" \
"	nop\n" \
"	cmpwi 3, 0\n" \
"	bne 1f\n" \
"	std 2, 40(1)\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	ld 5, 64(1)\n" \
"	ld 6, 72(1)\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -32\n" \
"	add  2, 2, 0\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"1:\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_PRECALL_SUCCESS_5(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	std 5, 64(1)\n" \
"	std 6, 72(1)\n" \
"	std 7, 80(1)\n" \
"	nop\n" \
"	nop\n" \
"	cmpwi 3, 0\n" \
"	bne 1f\n" \
"	std 2, 40(1)\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	ld 5, 64(1)\n" \
"	ld 6, 72(1)\n" \
"	ld 7, 80(1)\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -36\n" \
"	add  2, 2, 0\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"1:\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_PRECALL_SUCCESS_6(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	std 5, 64(1)\n" \
"	std 6, 72(1)\n" \
"	std 7, 80(1)\n" \
"	std 8, 88(1)\n" \
"	nop\n" \
"	nop\n" \
"	cmpwi 3, 0\n" \
"	bne 1f\n" \
"	std 2, 40(1)\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	ld 5, 64(1)\n" \
"	ld 6, 72(1)\n" \
"	ld 7, 80(1)\n" \
"	ld 8, 88(1)\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -40\n" \
"	add  2, 2, 0\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"1:\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_PRECALL_SUCCESS_7(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	std 5, 64(1)\n" \
"	std 6, 72(1)\n" \
"	std 7, 80(1)\n" \
"	std 8, 88(1)\n" \
"	std 9, 96(1)\n" \
"	nop\n" \
"	nop\n" \
"	cmpwi 3, 0\n" \
"	bne 1f\n" \
"	std 2, 40(1)\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	ld 5, 64(1)\n" \
"	ld 6, 72(1)\n" \
"	ld 7, 80(1)\n" \
"	ld 8, 88(1)\n" \
"	ld 9, 96(1)\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -44\n" \
"	add  2, 2, 0\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"1:\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_PRECALL_SUCCESS_8(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	std 5, 64(1)\n" \
"	std 6, 72(1)\n" \
"	std 7, 80(1)\n" \
"	std 8, 88(1)\n" \
"	std 9, 96(1)\n" \
"	std 10, 104(1)\n" \
"	nop\n" \
"	nop\n" \
"	cmpwi 3, 0\n" \
"	bne 1f\n" \
"	std 2, 40(1)\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	ld 5, 64(1)\n" \
"	ld 6, 72(1)\n" \
"	ld 7, 80(1)\n" \
"	ld 8, 88(1)\n" \
"	ld 9, 96(1)\n" \
"	ld 10, 104(1)\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -48\n" \
"	add  2, 2, 0\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"1:\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_POSTCALL_0(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 2, 40(1)\n" \
"	bl +4\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -16\n" \
"	add  2, 2, 0\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	nop\n" \
"	nop\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_POSTCALL_1(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 2, 40(1)\n" \
"	bl +4\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -16\n" \
"	add  2, 2, 0\n" \
"	std 3, 48(1)\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	ld 3, 48(1)\n" \
"	nop\n" \
"	nop\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_POSTCALL_2(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 2, 40(1)\n" \
"	bl +4\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -16\n" \
"	add  2, 2, 0\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	nop\n" \
"	nop\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_POSTCALL_3(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 2, 40(1)\n" \
"	bl +4\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -16\n" \
"	add  2, 2, 0\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	std 5, 64(1)\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	ld 5, 64(1)\n" \
"	nop\n" \
"	nop\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_POSTCALL_4(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 2, 40(1)\n" \
"	bl +4\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -16\n" \
"	add  2, 2, 0\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	std 5, 64(1)\n" \
"	std 6, 72(1)\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	ld 5, 64(1)\n" \
"	ld 6, 72(1)\n" \
"	nop\n" \
"	nop\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_POSTCALL_5(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 2, 40(1)\n" \
"	bl +4\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -16\n" \
"	add  2, 2, 0\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	std 5, 64(1)\n" \
"	std 6, 72(1)\n" \
"	std 7, 80(1)\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	ld 5, 64(1)\n" \
"	ld 6, 72(1)\n" \
"	ld 7, 80(1)\n" \
"	nop\n" \
"	nop\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_POSTCALL_6(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 2, 40(1)\n" \
"	bl +4\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -16\n" \
"	add  2, 2, 0\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	std 5, 64(1)\n" \
"	std 6, 72(1)\n" \
"	std 7, 80(1)\n" \
"	std 8, 88(1)\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	ld 5, 64(1)\n" \
"	ld 6, 72(1)\n" \
"	ld 7, 80(1)\n" \
"	ld 8, 88(1)\n" \
"	nop\n" \
"	nop\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_POSTCALL_7(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 2, 40(1)\n" \
"	bl +4\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -16\n" \
"	add  2, 2, 0\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	std 5, 64(1)\n" \
"	std 6, 72(1)\n" \
"	std 7, 80(1)\n" \
"	std 8, 88(1)\n" \
"	std 9, 96(1)\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	ld 5, 64(1)\n" \
"	ld 6, 72(1)\n" \
"	ld 7, 80(1)\n" \
"	ld 8, 88(1)\n" \
"	ld 9, 96(1)\n" \
"	nop\n" \
"	nop\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_POSTCALL_8(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 2, 40(1)\n" \
"	bl +4\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -16\n" \
"	add  2, 2, 0\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	std 5, 64(1)\n" \
"	std 6, 72(1)\n" \
"	std 7, 80(1)\n" \
"	std 8, 88(1)\n" \
"	std 9, 96(1)\n" \
"	std 10, 104(1)\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	ld 5, 64(1)\n" \
"	ld 6, 72(1)\n" \
"	ld 7, 80(1)\n" \
"	ld 8, 88(1)\n" \
"	ld 9, 96(1)\n" \
"	ld 10, 104(1)\n" \
"	nop\n" \
"	nop\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_COND_POSTCALL_0(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 2, 40(1)\n" \
"	bl +4\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -16\n" \
"	add  2, 2, 0\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	extsw 4, 3\n" \
"	cmpwi 4, -15007\n" \
"	bne +12\n" \
"	nop\n" \
"	nop\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_COND_POSTCALL_1(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 2, 40(1)\n" \
"	bl +4\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -16\n" \
"	add  2, 2, 0\n" \
"	std 3, 48(1)\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	extsw 4, 3\n" \
"	cmpwi 4, -15007\n" \
"	bne +16\n" \
"	ld 3, 48(1)\n" \
"	nop\n" \
"	nop\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_COND_POSTCALL_2(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 2, 40(1)\n" \
"	bl +4\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -16\n" \
"	add  2, 2, 0\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	extsw 4, 3\n" \
"	cmpwi 4, -15007\n" \
"	bne +20\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	nop\n" \
"	nop\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_COND_POSTCALL_3(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 2, 40(1)\n" \
"	bl +4\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -16\n" \
"	add  2, 2, 0\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	std 5, 64(1)\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	extsw 4, 3\n" \
"	cmpwi 4, -15007\n" \
"	bne +24\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	ld 5, 64(1)\n" \
"	nop\n" \
"	nop\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_COND_POSTCALL_4(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 2, 40(1)\n" \
"	bl +4\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -16\n" \
"	add  2, 2, 0\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	std 5, 64(1)\n" \
"	std 6, 72(1)\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	extsw 4, 3\n" \
"	cmpwi 4, -15007\n" \
"	bne +28\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	ld 5, 64(1)\n" \
"	ld 6, 72(1)\n" \
"	nop\n" \
"	nop\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_COND_POSTCALL_5(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 2, 40(1)\n" \
"	bl +4\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -16\n" \
"	add  2, 2, 0\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	std 5, 64(1)\n" \
"	std 6, 72(1)\n" \
"	std 7, 80(1)\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	extsw 4, 3\n" \
"	cmpwi 4, -15007\n" \
"	bne +32\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	ld 5, 64(1)\n" \
"	ld 6, 72(1)\n" \
"	ld 7, 80(1)\n" \
"	nop\n" \
"	nop\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_COND_POSTCALL_6(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 2, 40(1)\n" \
"	bl +4\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -16\n" \
"	add  2, 2, 0\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	std 5, 64(1)\n" \
"	std 6, 72(1)\n" \
"	std 7, 80(1)\n" \
"	std 8, 88(1)\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	extsw 4, 3\n" \
"	cmpwi 4, -15007\n" \
"	bne +36\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	ld 5, 64(1)\n" \
"	ld 6, 72(1)\n" \
"	ld 7, 80(1)\n" \
"	ld 8, 88(1)\n" \
"	nop\n" \
"	nop\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_COND_POSTCALL_7(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 2, 40(1)\n" \
"	bl +4\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -16\n" \
"	add  2, 2, 0\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	std 5, 64(1)\n" \
"	std 6, 72(1)\n" \
"	std 7, 80(1)\n" \
"	std 8, 88(1)\n" \
"	std 9, 96(1)\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	extsw 4, 3\n" \
"	cmpwi 4, -15007\n" \
"	bne +40\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	ld 5, 64(1)\n" \
"	ld 6, 72(1)\n" \
"	ld 7, 80(1)\n" \
"	ld 8, 88(1)\n" \
"	ld 9, 96(1)\n" \
"	nop\n" \
"	nop\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_HOOKED_FUNCTION_COND_POSTCALL_8(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 2, 40(1)\n" \
"	bl +4\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -16\n" \
"	add  2, 2, 0\n" \
"	std 3, 48(1)\n" \
"	std 4, 56(1)\n" \
"	std 5, 64(1)\n" \
"	std 6, 72(1)\n" \
"	std 7, 80(1)\n" \
"	std 8, 88(1)\n" \
"	std 9, 96(1)\n" \
"	std 10, 104(1)\n" \
"	bl .L._"#name"\n" \
"	ld 2, 40(1)\n" \
"	extsw 4, 3\n" \
"	cmpwi 4, -15007\n" \
"	bne +44\n" \
"	ld 3, 48(1)\n" \
"	ld 4, 56(1)\n" \
"	ld 5, 64(1)\n" \
"	ld 6, 72(1)\n" \
"	ld 7, 80(1)\n" \
"	ld 8, 88(1)\n" \
"	ld 9, 96(1)\n" \
"	ld 10, 104(1)\n" \
"	nop\n" \
"	nop\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_MULTIPLE_RETURN_2(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 2, 40(1)\n" \
"	bl +4\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -16\n" \
"	add  2, 2, 0\n" \
"	bl .L._"#name"\n" \
"	ld 4,  8(3)\n" \
"	ld 3, 0(3)\n" \
"	ld 2, 40(1)\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args

#define LV2_MULTIPLE_RETURN_7(ret, name, args) asm("" \
"	.section \".text\"\n" \
"	.align 2\n" \
"	.p2align 3,,7\n" \
"	.globl "#name"\n" \
"	.section \".opd\",\"aw\"\n" \
"	.align 3\n" \
#name": \n" \
"	.quad .L."#name",.TOC.@tocbase\n" \
"	.previous\n" \
"	.type   "#name", @function\n" \
".L."#name":\n" \
"	mflr 0\n" \
"	std 0, 32(1)\n" \
"	std 2, 40(1)\n" \
"	bl +4\n" \
"	li 0, 0\n" \
"	li 2, 0\n" \
"	oris 2, 2, __toc@h\n" \
"	ori 2, 2, __toc@l\n" \
"	oris 0, 0, .L."#name"@h\n" \
"	ori 0, 0, .L."#name"@l\n" \
"	subf 0, 0, 2\n" \
"	mflr 2\n" \
"	addi 2, 2, -16\n" \
"	add  2, 2, 0\n" \
"	bl .L._"#name"\n" \
"	ld 4,  8(3)\n" \
"	ld 5, 16(3)\n" \
"	ld 6, 24(3)\n" \
"	ld 7, 32(3)\n" \
"	ld 8, 40(3)\n" \
"	ld 9, 48(3)\n" \
"	ld 3, 0(3)\n" \
"	ld 2, 40(1)\n" \
"	ld 0, 32(1)\n" \
"	mtlr 0\n" \
"	blr\n" \
"	.size "#name",.-.L."#name"\n"); \
ret name args; \
ret _##name args


#define MAKE_JUMP(addr, to) *(uint32_t *)(addr) = (0x12 << 26) | ((((to-(uint64_t)(addr))>>2)&0xFFFFFF) << 2)
#define MAKE_CALL(addr, to) *(uint32_t *)(addr) = (0x12 << 26) | ((((to-(uint64_t)(addr))>>2)&0xffffff) << 2) | 1

typedef struct function_descriptor 
{
	void	*addr;
	void    *toc;	
} f_desc_t;

static INLINE void change_function(uint64_t func_addr, void *newfunc)
{
	f_desc_t *f = (f_desc_t *)newfunc;
	MAKE_JUMP(func_addr, (uint64_t)f->addr);
	PPCFlushInstructionCache((void*)func_addr, 4);
}

static INLINE void hook_function_with_precall(uint64_t func_addr, void *newfunc, unsigned int nparams)
{
	if (nparams > 8)
		nparams = 8;
	
	f_desc_t *f = (f_desc_t *)newfunc;
	uint32_t inst = *(uint32_t *)func_addr;
	uint32_t *orig_call = &((uint32_t *)f->addr)[2+nparams]; // WARNING: relies on HOOKED_FUNCTION_PRECALL_N not being changed	
	
	orig_call[0] = inst;
	MAKE_CALL(&orig_call[1], (func_addr+4));
    PPCFlushInstructionCache((void*)orig_call, 8);
	change_function(func_addr, newfunc);
}

static INLINE void hook_function_on_precall_success(uint64_t func_addr, void *newfunc, unsigned int nparams)
{
	hook_function_with_precall(func_addr, newfunc, nparams);
}

#endif // !POWER_PC_H