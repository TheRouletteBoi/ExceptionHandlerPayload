.text
.p2align 2

.globl   SpinLockInit
.type    SpinLockInit,@function
.section .opd,"aw",@progbits
SpinLockInit: 
.p2align 3
.quad    .SpinLockInit
.quad    .TOC.@tocbase
.quad    0

.globl   SpinLockAcquire
.type    SpinLockAcquire,@function
.section .opd,"aw",@progbits
SpinLockAcquire: 
.p2align 3
.quad    .SpinLockAcquire
.quad    .TOC.@tocbase
.quad    0

.globl   SpinLockRelease
.type    SpinLockRelease,@function
.section .opd,"aw",@progbits
SpinLockRelease: 
.p2align 3
.quad    .SpinLockRelease
.quad    .TOC.@tocbase
.quad    0

.section .text,"ax",@progbits

//
// SpinLockInit(volatile int* Lock)
//
.SpinLockInit:
    li   %r4, 0
    stw  %r4, 0(%r3)
    lwsync
    blr


.set LOCK_TAG, 0xDEADBEEF

//
// SpinLockAcquire(volatile int* Lock)
//
.SpinLockAcquire:
    lis   %r4, LOCK_TAG@h
    ori   %r4, %r4, LOCK_TAG@l

_TryAcquire:
    //
    // Check lock for tag.
    //
    lwarx  %r5, 0, %r3
    cmpw   %r5, %r4
    beq-   _Spin_Loop
    
    //
    // Attempt to write to the lock.
    //
    stwcx. %r4, 0, %r3
    bne-   _TryAcquire
    isync
    b      _LockAcquired

    //
    // Spin untill we can attempt to acquire the lock again.
    //
_Spin_Loop:
    lwz   %r5, 0(%r3)
    cmpw  %r5, %r4
    beq-  _Spin_Loop
    b     _TryAcquire

_LockAcquired:
    blr

//
// SpinLockRelease(volatile int* Lock)
//
.SpinLockRelease:
    lwsync
    li  %r4, 0
    stw %r4, 0(%r3)
    blr