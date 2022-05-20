#ifndef PATTERN_SCANNER_H
#define PATTERN_SCANNER_H

#include "Types.h"
#include "FNV.h"
#include "Utils.h"

constexpr bool IsSkipChar(char character)
{
    return character == '?' || character == '*';
}

class Signature 
{
public:
    enum CallbackStatus
    {
        SCANNER_CONTINUE,
        SCANNER_STOP
    };

    #define ALIGN_ADDR(address, alignAmount) uintptr_t(address & ~(alignAmount - 1))

    inline bool Matches(uint64_t memory) const
    {
        return NonCryptoHashWithMaskFNV1A64((const char*)memory, this->mask ) == this->hash;
    }
    
    // Converts IDA style string to mask/hash combo.
    constexpr static inline Signature FromString(const char* pattern)
    {
        Signature result        = { FNV_OFFSET_BASIS_64, 0 };
        char      lastCharacter = NULL;
        int       maskBitIndex  = 0;

        while (*pattern != 0)
        {
            const char currentCharacter = pattern[0];
            const bool hasNextCharacter = pattern[1] != 0;

            if (currentCharacter == ' ')
            {
                lastCharacter = currentCharacter;
            }

            // If we hit a wildcard character and the last character is not also a wildcard,
            // this must be the start of a wildcard block.
            else if (IsSkipChar(currentCharacter))
            {
                if (!IsSkipChar(lastCharacter))
                {
                    maskBitIndex++;
                }
            }

            // Not a wild card check if we have a valid hex identifier.
            else
            {
                if (!hasNextCharacter)
                    return Signature{};

                const uint8_t currentHexChar = Utils::HexCharValue(currentCharacter);
                const uint8_t nextHexChar    = Utils::HexCharValue(pattern[ 1]);

                if (currentHexChar == 0xFF || nextHexChar == 0xFF)
                    return Signature{};

                uint8_t hexByte = (currentHexChar << 4) | nextHexChar;

                result.hash =  FNV1A_ITERATION(hexByte, result.hash, FNV_PRIME_64);
                result.mask |= (1ULL << maskBitIndex++);

                pattern++;
            }

            pattern++;

            lastCharacter = currentCharacter;
        }

        return result;
    }

    template <typename T, bool IsCodeAligned, bool StopOnFirstResult>
    uint64_t FindMatchesEx(uint64_t start, size_t length, T& userData, 
        CallbackStatus(*callbackFn)(T& userData, uint64_t result)) const
    {
        const auto alignSize    = IsCodeAligned ? 4 : 1;
        const auto startAddress = ALIGN_ADDR(start, alignSize);
        const auto stepSize     = alignSize;

        for (auto i = 0ULL; i < (length - alignSize); i += stepSize)
        {
            const auto searchAddress = startAddress + i;

            if (Matches(searchAddress))
            {   
                if (callbackFn != NULL)
                {
                    auto status = callbackFn(userData, searchAddress);

                    if (status == CallbackStatus::SCANNER_STOP)
                    {
                        return searchAddress;
                    }
                }

                if (StopOnFirstResult)
                    return searchAddress;
            }
        }

        return 0;
    }

    template <typename T>
    uint64_t FindMatch(uint64_t start, size_t length, T& userData, 
        CallbackStatus(*callbackFn)(T& userData, uint64_t result)) const
    {
        return FindMatchesEx<T, false, true>(start, length, userData, callbackFn);
    }

    template<typename T>
    void FindMatches(uint64_t start, size_t length, T& userData, 
        CallbackStatus(*callbackFn)(T& userData, uint64_t result)) const
    {
        FindMatchesEx<T, false, false>(start, length, userData, callbackFn);
    }

    template<typename T>
    uint64_t FindCodeMatch(uint64_t start, size_t length, T& userData,
        CallbackStatus(*callbackFn)(T& userData, uint64_t result)) const
    {
        return FindMatchesEx<T, true, true>(start, length, userData, callbackFn);
    }

    template<typename T>
    void FindCodeMatches(uint64_t start, size_t length, T& userData, 
        CallbackStatus(*callbackFn)(T& userData, uint64_t result)) const
    {
        FindMatchesEx<T, true, false>(start, length, userData, callbackFn);
    }

    #include "ELF.h"

    // Search for a proccedure descriptor (OPD Entry) for a given function address.
    NO_INLINE static OPD_t* FindProcedureDescriptor(uint64_t function, OPD_t* lastProcedureDescriptor)
    {
        auto currentProcedureDescriptor = lastProcedureDescriptor;

        while (currentProcedureDescriptor->TOC == lastProcedureDescriptor->TOC)
        {
            if (currentProcedureDescriptor->Function == function)
                return currentProcedureDescriptor;

            currentProcedureDescriptor--;
        }

        return 0;
    }

public:
    uint64_t hash;
    uint64_t mask;

private:
    constexpr Signature() : hash(0), mask(0) { }
    constexpr Signature(uint64_t hash, uint64_t mask) : hash(hash), mask(mask) { }
};

#define MAKE_PATTERN(X) \
    []( ) -> auto { \
        constexpr auto Sig = Signature::FromString( X ); \
        \
        return Sig; \
    }( )

#endif // !PATTERN_SCANNER_H