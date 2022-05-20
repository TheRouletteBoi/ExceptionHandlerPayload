extern "C" { 
    void SpinLockInit( volatile int* Lock );
    void SpinLockAcquire( volatile int* Lock );
    void SpinLockRelease( volatile int* Lock );
}