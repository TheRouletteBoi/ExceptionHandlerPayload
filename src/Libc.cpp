#include "Libc.h"

void* memcpy(void* destination, void const* source, size_t size)
{
    void *const result = destination;

    while (size--)
    {
        *(char*)destination = *(char*)source;
        destination = (char*)destination + 1;
        source = (char*)source + 1;
    }

    return result;
}

void* memset(void* destination, int val, size_t size)
{
    void *const result = destination;

    while (size--) 
    {
        *(char *)destination = (char)val;
        destination = (char *)destination + 1;
    }

    return result;
}

void* memmove(void* destination, void const* source, size_t size)
{
    void * const result = destination;

    if (destination <= source || (char *)destination >= ((char *)source + size)) 
    {
        // Non-Overlapping Buffers, copy from lower to higher addresses.
        while (size--) 
        {
            *(char *)destination = *(char *)source;
            destination = (char *)destination + 1;
            source = (char *)source + 1;
        }

    } 
    else 
    {
        // Overlapping Buffers, copy from higher to lower addresses.
        destination = (char *)destination + size - 1;
        source = (char *)source + size - 1;

        while (size--) 
        {
            *(char *)destination = *(char *)source;
            destination = (char *)destination - 1;
            source = (char *)source - 1;
        }
    }

    return result;
}

int strcmp(char const* string1, char const* string2)
{
    int result = 0;

    while ((result = *(unsigned char *)string1 - *(unsigned char *)string2) == 0 && *string2) 
    {
        ++string1;
        ++string2;
    }

    //  (if positive) - (if negative) makes branchless code
    return ((-result) < 0) - (result < 0);
}

size_t strlen(const char* string)
{
    const char* endPtr;

    for (endPtr = string; *endPtr; ++endPtr)
            ;
    return (endPtr - string);
}

char* strcat(char* destination, char const* source)
{
    char* copyPtr = destination;

    // Find end of Destination.
    while (*copyPtr) copyPtr++;

    // Copy Source to end of Destination.
    while ((*copyPtr++ = *source++));

    return destination;                 
}

char* strcpy(char* destination, const char* source)
{
    char* start = destination;

    while ((*destination++ = *source++));

    return start;
}

char* strncat(char* destination, const char* source, size_t count)
{
    char* start = destination;

    while (*destination++)
        ;
    destination--;

    while (count--)
    {
        if (!(*destination++ = *source++))
            return start;
    }

    *destination = '\0';

    return start;
}