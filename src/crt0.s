
.text
    .section .start,"ax",@progbits
    .align 3
    .global _start
    .type   _start, @function
    .extern PayloadEntry
    
    
_start:
.set STACK_SIZE, 0x220
    //
    // Setup stack.
    //
    mflr  %r0
    std   %r0,  0x10(%r1)
    std   %r2,  0x28(%r1)
    stdu  %r1,  -STACK_SIZE(%r1)

    mr    %r3, %r2

    //
    // Load the TOC of our payload.
    //
    lis   %r2, __toc@highest
    sldi  %r2, %r2, 32
    oris  %r2, %r2, __toc@h
    ori   %r2, %r2, __toc@l
    
    //
    // PayloadEntry( KernelRTOC, PayloadRTOC )
    //
    mr    %r4, %r2
    bl    .PayloadEntry

    //
    // Cleanup stack and return.
    //
    addi  %r1, %r1, STACK_SIZE
    ld    %r2, 0x28(%r1)
    ld    %r0, 0x10(%r1)
    mtlr  %r0
    blr