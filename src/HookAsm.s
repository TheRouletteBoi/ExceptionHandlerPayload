.text
.p2align 2
.globl  SystemCallHookPrepareDispatch
.type   SystemCallHookPrepareDispatch,@function
.section .opd,"aw",@progbits
SystemCallHookPrepareDispatch: 
.p2align 3
.quad  .SystemCallHookPrepareDispatch
.quad  .TOC.@tocbase
.quad  0
.section .text,"ax",@progbits


//
// Handles passing execution to our hook.
//
//
// ROM:800000000027DECC 79 6B 1F 24                 sldi      r11, r11, 3
// ROM:800000000027DED0 7D AB 6A 14                 add       r13, r11, r13
// ROM:800000000027DED4 E9 AD 00 00                 ld        r13, 0(r13)
// ROM:800000000027DED8 E9 AD 00 00                 ld        r13, 0(r13)
// ROM:800000000027DEDC F8 21 FF 91                 stdu      r1, back_chain(%r1)
// ROM:800000000027DEE0 7D A8 03 A6                 mtlr      r13
// ROM:800000000027DEE4 4E 80 00 21                 blrl          <---- We come from here.
.SystemCallHookPrepareDispatch:
    .set STACK_SIZE,   0x200
    .set CALL_CONTEXT, 0x40
    //
    // Save our return address (the system call dispatcher)
    //
    mflr    %r0
    std     %r0, 0x10(%r1)
    stdu    %r1, -STACK_SIZE(%r1)
    std     %r2, CALL_CONTEXT+0x50(%r1)

    //
    // Load the TOC of our payload.
    //
    lis   %r2, __toc@highest
    sldi  %r2, %r2, 32
    oris  %r2, %r2, __toc@h
    ori   %r2, %r2, __toc@l


    //
    // Save system call arguments r3-r10
    //
    std   %r3,  CALL_CONTEXT+0x00(%r1)
    std   %r4,  CALL_CONTEXT+0x08(%r1)
    std   %r5,  CALL_CONTEXT+0x10(%r1)
    std   %r6,  CALL_CONTEXT+0x18(%r1)
    std   %r7,  CALL_CONTEXT+0x20(%r1)
    std   %r8,  CALL_CONTEXT+0x28(%r1)
    std   %r9,  CALL_CONTEXT+0x30(%r1)
    std   %r10, CALL_CONTEXT+0x38(%r1)

    //
    // Store original systemcall number and handler address.
    //
    srdi  %r11, %r11, 3 
    std   %r11, CALL_CONTEXT+0x40(%r1)
    std   %r13, CALL_CONTEXT+0x48(%r1)

    //
    // SystemCallHookProcess( CallContext )
    //
    addi  %r3, %r1, CALL_CONTEXT
    bl    SystemCallHookProcess

    //
    // Restore system call registers.
    //
    ld   %r3,  CALL_CONTEXT+0x00(%r1)
    ld   %r4,  CALL_CONTEXT+0x08(%r1)
    ld   %r5,  CALL_CONTEXT+0x10(%r1)
    ld   %r6,  CALL_CONTEXT+0x18(%r1)
    ld   %r7,  CALL_CONTEXT+0x20(%r1)
    ld   %r8,  CALL_CONTEXT+0x28(%r1)
    ld   %r9,  CALL_CONTEXT+0x30(%r1)
    ld   %r10, CALL_CONTEXT+0x38(%r1)
    ld   %r11, CALL_CONTEXT+0x40(%r1)
    ld   %r13, CALL_CONTEXT+0x48(%r1)

    //
    // Restore TOC
    //
    ld    %r2, CALL_CONTEXT+0x50(%r1)
    addi  %r1, %r1, STACK_SIZE

    //
    // Restore return address and branch to system call.
    //
    ld    %r0, 0x10(%r1)
    mtctr %r13
    mtlr  %r0
    bctr    

/* 
.text
.p2align 2
.globl  PpuThreadMsgInterruptExceptionHookPrepare
.type   PpuThreadMsgInterruptExceptionHookPrepare,@function
.section .opd,"aw",@progbits
PpuThreadMsgInterruptExceptionHookPrepare: 
.p2align 3
.quad  .PpuThreadMsgInterruptExceptionHookPrepare
.quad  .TOC.@tocbase
.quad  0
.section .text,"ax",@progbits


.PpuThreadMsgInterruptExceptionHookPrepare:
    .set STACK_SIZE,   0x200
    .set CALL_CONTEXT, 0x40
    //
    // Save our return address (the system call dispatcher)
    //
    mflr    %r0
    std     %r0, 0x10(%r1)
    stdu    %r1, -STACK_SIZE(%r1)
    std     %r2, CALL_CONTEXT+0x50(%r1)

    //
    // Load the TOC of our payload.
    //
    lis   %r2, __toc@highest
    sldi  %r2, %r2, 32
    oris  %r2, %r2, __toc@h
    ori   %r2, %r2, __toc@l

    bl    PpuThreadMsgInterruptExceptionHook


    // Restore TOC
    ld    %r2, CALL_CONTEXT+0x50(%r1)
    addi  %r1, %r1, STACK_SIZE


    ld    %r0, 0x10(%r1)
    mtlr  %r0
    bctr    */