#include "LV2/Process.h"

// Utils for managing user mapped pages.
template <typename T>
class UserMappedPointer 
{
public:
    UserMappedPointer(uint8_t* kernelPtr, uint32_t userPtr, size_t size = 1) 
        : kernelPtr(kernelPtr), userPtr(userPtr), size(size)
    {

    }

    template <typename R>
    R UserAs()       const { return (R)this->userPtr; }
    T* User()        const { return (T*)this->userPtr;  }
    T* Kernel()      const { return (T*)this->kernelPtr; }
    T& operator * () const { return *this->Kernel(); }
    T& operator * ()       { return *this->Kernel(); }

    T* operator-> () const { return this->Kernel(); }
    T* operator-> ()       { return this->Kernel(); }

public:
    uint8_t* const kernelPtr;
    const uint32_t userPtr;
    size_t         size;
};

class UserMappedMemory 
{
public:
    UserMappedMemory(void* kernelPage, uint32_t userPage, size_t maxSize) 
        : kernelPage((uint8_t*)kernelPage), userPage(userPage), 
            maxSize(maxSize), currentSize(0)
    {

    }

    template <typename T>
    UserMappedPointer<T> Get(size_t count)
    {
        const auto requestedSize = sizeof(T) * count;
        
        if ((requestedSize + this->currentSize) < this->maxSize)
        {
            auto newAlloc = UserMappedPointer<T>(&this->kernelPage[this->currentSize], userPage + this->currentSize, requestedSize);

            this->currentSize += requestedSize;

            DEBUG_PRINT("UserMappedPointer: NewAlloc: %llX RequestedSize: %X CurrentSize: %X\n", newAlloc.kernelPtr, requestedSize, this->currentSize);

            return newAlloc;
        }

        return UserMappedPointer<T>(NULL, NULL, 0);
    }

public:
    uint8_t* const kernelPage;
    const uint32_t userPage;
    size_t         currentSize;
    const size_t   maxSize;
};

UserMappedMemory AllocateUserPage(lv2::process* process, size_t size, uint64_t exportFlags = 0x40000);
void FreeUserModePages(lv2::process* process, UserMappedMemory& memoryManager);

// Allocates kernel pages and maps them to a usermode address.
void AllocateUserModePagesWithCallback(lv2::process* process, size_t size, void(*callback)(UserMappedMemory& memory));