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



.text
.p2align 2
.globl  HookOpCodePrepareDispatch
.type   HookOpCodePrepareDispatch,@function
.section .opd,"aw",@progbits
HookOpCodePrepareDispatch: 
.p2align 3
.quad  .HookOpCodePrepareDispatch
.quad  .TOC.@tocbase
.quad  0
.section .text,"ax",@progbits


//
// Handles passing execution to our hook.
//
.HookOpCodePrepareDispatch:
    .set STACK_SIZE,   0x300
    .set CALL_CONTEXT, 0x70
	
	stdu      %r1, -STACK_SIZE(%r1)
	
	//
	// Save all general purpose registers.
	//
	std       %r0, CALL_CONTEXT(%r1)
	addi      %r0, %r1, STACK_SIZE
	std       %r0, CALL_CONTEXT+8(%r1)
	std       %r2, CALL_CONTEXT+0x10(%r1)
	std       %r3, CALL_CONTEXT+0x18(%r1)
	std       %r4, CALL_CONTEXT+0x20(%r1)
	std       %r5, CALL_CONTEXT+0x28(%r1)
	std       %r6, CALL_CONTEXT+0x30(%r1)
	std       %r7, CALL_CONTEXT+0x38(%r1)
	std       %r8, CALL_CONTEXT+0x40(%r1)
	std       %r9, CALL_CONTEXT+0x48(%r1)
	std       %r10, CALL_CONTEXT+0x50(%r1)
	std       %r11, CALL_CONTEXT+0x58(%r1)
	std       %r12, CALL_CONTEXT+0x60(%r1)
	std       %r13, CALL_CONTEXT+0x68(%r1)
	std       %r14, CALL_CONTEXT+0x70(%r1)
	std       %r15, CALL_CONTEXT+0x78(%r1)
	std       %r16, CALL_CONTEXT+0x80(%r1)
	std       %r17, CALL_CONTEXT+0x88(%r1)
	std       %r18, CALL_CONTEXT+0x90(%r1)
	std       %r19, CALL_CONTEXT+0x98(%r1)
	std       %r20, CALL_CONTEXT+0xA0(%r1)
	std       %r21, CALL_CONTEXT+0xA8(%r1)
	std       %r22, CALL_CONTEXT+0xB0(%r1)
	std       %r23, CALL_CONTEXT+0xB8(%r1)
	std       %r24, CALL_CONTEXT+0xC0(%r1)
	std       %r25, CALL_CONTEXT+0xC8(%r1)
	std       %r26, CALL_CONTEXT+0xD0(%r1)
	std       %r27, CALL_CONTEXT+0xD8(%r1)
	std       %r28, CALL_CONTEXT+0xE0(%r1)
	std       %r29, CALL_CONTEXT+0xE8(%r1)
	std       %r30, CALL_CONTEXT+0xF0(%r1)
	std       %r31, CALL_CONTEXT+0xF8(%r1)
	
	//
	// Save all special purpose registers.
	//
	mflr      %r0
	std       %r0, CALL_CONTEXT+0x100(%r1)
	mfctr     %r0
	std       %r0, CALL_CONTEXT+0x108(%r1)
	mfcr      %r0
	std       %r0, CALL_CONTEXT+0x110(%r1)
	mfxer     %r0
	std       %r0, CALL_CONTEXT+0x118(%r1)
	
	//
	// Save all floating point registers.
	//
	stfd      %f0, CALL_CONTEXT+0x120(%r1)
	stfd      %f1, CALL_CONTEXT+0x128(%r1)
	stfd      %f2, CALL_CONTEXT+0x130(%r1)
	stfd      %f3, CALL_CONTEXT+0x138(%r1)
	stfd      %f4, CALL_CONTEXT+0x140(%r1)
	stfd      %f5, CALL_CONTEXT+0x148(%r1)
	stfd      %f6, CALL_CONTEXT+0x150(%r1)
	stfd      %f7, CALL_CONTEXT+0x158(%r1)
	stfd      %f8, CALL_CONTEXT+0x160(%r1)
	stfd      %f9, CALL_CONTEXT+0x168(%r1)
	stfd      %f10, CALL_CONTEXT+0x170(%r1)
	stfd      %f11, CALL_CONTEXT+0x178(%r1)
	stfd      %f12, CALL_CONTEXT+0x180(%r1)
	stfd      %f13, CALL_CONTEXT+0x188(%r1)
	stfd      %f14, CALL_CONTEXT+0x190(%r1)
	stfd      %f15, CALL_CONTEXT+0x198(%r1)
	stfd      %f16, CALL_CONTEXT+0x1A0(%r1)
	stfd      %f17, CALL_CONTEXT+0x1A8(%r1)
	stfd      %f18, CALL_CONTEXT+0x1B0(%r1)
	stfd      %f19, CALL_CONTEXT+0x1B8(%r1)
	stfd      %f20, CALL_CONTEXT+0x1C0(%r1)
	stfd      %f21, CALL_CONTEXT+0x1C8(%r1)
	stfd      %f22, CALL_CONTEXT+0x1D0(%r1)
	stfd      %f23, CALL_CONTEXT+0x1D8(%r1)
	stfd      %f24, CALL_CONTEXT+0x1E0(%r1)
	stfd      %f25, CALL_CONTEXT+0x1E8(%r1)
	stfd      %f26, CALL_CONTEXT+0x1F0(%r1)
	stfd      %f27, CALL_CONTEXT+0x1F8(%r1)
	stfd      %f28, CALL_CONTEXT+0x200(%r1)
	stfd      %f29, CALL_CONTEXT+0x208(%r1)
	stfd      %f30, CALL_CONTEXT+0x210(%r1)
	stfd      %f31, CALL_CONTEXT+0x218(%r1)
	
	//
	// Load the TOC of our payload.
	//
	lis       %r2, __toc@highest
	sldi      %r2, %r2, 32
	oris      %r2, %r2, __toc@h
	ori       %r2, %r2, __toc@l
	
	//
	// HookOpCode::Handler( CallContext )
	//
	addi      %r3, %r1, CALL_CONTEXT
	bl        Handler
	
	//
	// Restore all special purpose registers.
	//
	ld        %r0, CALL_CONTEXT+0x100(%r1)
	mtlr      %r0
	ld        %r0, CALL_CONTEXT+0x108(%r1)
	mtctr     %r0
	ld        %r0, CALL_CONTEXT+0x110(%r1)
	mtcr      %r0
	ld        %r0, CALL_CONTEXT+0x118(%r1)
	mtxer     %r0
	
	// 
	// Restore all general purpose registers.
	//
	ld        %r0, CALL_CONTEXT(%r1)
	ld        %r2, CALL_CONTEXT+0x10(%r1)
	ld        %r3, CALL_CONTEXT+0x18(%r1)
	ld        %r4, CALL_CONTEXT+0x20(%r1)
	ld        %r5, CALL_CONTEXT+0x28(%r1)
	ld        %r6, CALL_CONTEXT+0x30(%r1)
	ld        %r7, CALL_CONTEXT+0x38(%r1)
	ld        %r8, CALL_CONTEXT+0x40(%r1)
	ld        %r9, CALL_CONTEXT+0x48(%r1)
	ld        %r10, CALL_CONTEXT+0x50(%r1)
	ld        %r11, CALL_CONTEXT+0x58(%r1)
	ld        %r12, CALL_CONTEXT+0x60(%r1)
	ld        %r13, CALL_CONTEXT+0x68(%r1)
	ld        %r14, CALL_CONTEXT+0x70(%r1)
	ld        %r15, CALL_CONTEXT+0x78(%r1)
	ld        %r16, CALL_CONTEXT+0x80(%r1)
	ld        %r17, CALL_CONTEXT+0x88(%r1)
	ld        %r18, CALL_CONTEXT+0x90(%r1)
	ld        %r19, CALL_CONTEXT+0x98(%r1)
	ld        %r20, CALL_CONTEXT+0xA0(%r1)
	ld        %r21, CALL_CONTEXT+0xA8(%r1)
	ld        %r22, CALL_CONTEXT+0xB0(%r1)
	ld        %r23, CALL_CONTEXT+0xB8(%r1)
	ld        %r24, CALL_CONTEXT+0xC0(%r1)
	ld        %r25, CALL_CONTEXT+0xC8(%r1)
	ld        %r26, CALL_CONTEXT+0xD0(%r1)
	ld        %r27, CALL_CONTEXT+0xD8(%r1)
	ld        %r28, CALL_CONTEXT+0xE0(%r1)
	ld        %r29, CALL_CONTEXT+0xE8(%r1)
	ld        %r30, CALL_CONTEXT+0xF0(%r1)
	ld        %r31, CALL_CONTEXT+0xF8(%r1)
	
	//
	// Restore all floating point registers.
	//
	lfd       %f0, CALL_CONTEXT+0x120(%r1)
	lfd       %f1, CALL_CONTEXT+0x128(%r1)
	lfd       %f2, CALL_CONTEXT+0x130(%r1)
	lfd       %f3, CALL_CONTEXT+0x138(%r1)
	lfd       %f4, CALL_CONTEXT+0x140(%r1)
	lfd       %f5, CALL_CONTEXT+0x148(%r1)
	lfd       %f6, CALL_CONTEXT+0x150(%r1)
	lfd       %f7, CALL_CONTEXT+0x158(%r1)
	lfd       %f8, CALL_CONTEXT+0x160(%r1)
	lfd       %f9, CALL_CONTEXT+0x168(%r1)
	lfd       %f10, CALL_CONTEXT+0x170(%r1)
	lfd       %f11, CALL_CONTEXT+0x178(%r1)
	lfd       %f12, CALL_CONTEXT+0x180(%r1)
	lfd       %f13, CALL_CONTEXT+0x188(%r1)
	lfd       %f14, CALL_CONTEXT+0x190(%r1)
	lfd       %f15, CALL_CONTEXT+0x198(%r1)
	lfd       %f16, CALL_CONTEXT+0x1A0(%r1)
	lfd       %f17, CALL_CONTEXT+0x1A8(%r1)
	lfd       %f18, CALL_CONTEXT+0x1B0(%r1)
	lfd       %f19, CALL_CONTEXT+0x1B8(%r1)
	lfd       %f20, CALL_CONTEXT+0x1C0(%r1)
	lfd       %f21, CALL_CONTEXT+0x1C8(%r1)
	lfd       %f22, CALL_CONTEXT+0x1D0(%r1)
	lfd       %f23, CALL_CONTEXT+0x1D8(%r1)
	lfd       %f24, CALL_CONTEXT+0x1E0(%r1)
	lfd       %f25, CALL_CONTEXT+0x1E8(%r1)
	lfd       %f26, CALL_CONTEXT+0x1F0(%r1)
	lfd       %f27, CALL_CONTEXT+0x1F8(%r1)
	lfd       %f28, CALL_CONTEXT+0x200(%r1)
	lfd       %f29, CALL_CONTEXT+0x208(%r1)
	lfd       %f30, CALL_CONTEXT+0x210(%r1)
	lfd       %f31, CALL_CONTEXT+0x218(%r1)
	
	//
	// Restore stack address and return.
	//
	ld        %r1, CALL_CONTEXT+8(%r1)
	blr
	