#include "Types.h"



#define GET_NIBBLE(V, N)      ( ( V >> ( N * 4ULL ) ) & 0xFULL )
#define IS_NIBBLE_SET( V, N ) ( V & ( 0xFULL << ( N * 4ULL ) ) )

#define GET_BYTE(V, N)        ( ( V >> ( N *  8U ) ) &  UINT8_MAX )
#define GET_WORD(V, N)        ( ( V >> ( N *  8U ) ) & UINT16_MAX )
#define GET_DWORD(V, N)       ( ( V >> ( N * 32U ) ) & UINT32_MAX )
#define GET_QWORD(V, N)       ( V )

// Size of the buffer used in DumpHex()
#define DUMP_HEX_CHUNK_SIZE  0x100


static const char* s_HexChars      = "0123456789ABCDEF";
static const char* s_HexCharsLower = "0123456789abcdef";

static const char* s_HexCharsByte[] = {
    "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "0A", "0B", "0C", "0D", "0E", "0F",
    "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "1A", "1B", "1C", "1D", "1E", "1F",
    "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "2A", "2B", "2C", "2D", "2E", "2F",
    "30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "3A", "3B", "3C", "3D", "3E", "3F",
    "40", "41", "42", "43", "44", "45", "46", "47", "48", "49", "4A", "4B", "4C", "4D", "4E", "4F",
    "50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "5A", "5B", "5C", "5D", "5E", "5F",
    "60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "6A", "6B", "6C", "6D", "6E", "6F",
    "70", "71", "72", "73", "74", "75", "76", "77", "78", "79", "7A", "7B", "7C", "7D", "7E", "7F",
    "80", "81", "82", "83", "84", "85", "86", "87", "88", "89", "8A", "8B", "8C", "8D", "8E", "8F",
    "90", "91", "92", "93", "94", "95", "96", "97", "98", "99", "9A", "9B", "9C", "9D", "9E", "9F",
    "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "AA", "AB", "AC", "AD", "AE", "AF",
    "B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8", "B9", "BA", "BB", "BC", "BD", "BE", "BF",
    "C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7", "C8", "C9", "CA", "CB", "CC", "CD", "CE", "CF",
    "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "DA", "DB", "DC", "DD", "DE", "DF",
    "E0", "E1", "E2", "E3", "E4", "E5", "E6", "E7", "E8", "E9", "EA", "EB", "EC", "ED", "EE", "EF",
    "F0", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "FA", "FB", "FC", "FD", "FE", "FF",
};

static const char* s_HexCharsByteLower[] = {
    "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "0A", "0B", "0C", "0D", "0E", "0F",
    "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "1A", "1B", "1C", "1D", "1E", "1F",
    "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "2A", "2B", "2C", "2D", "2E", "2F",
    "30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "3A", "3B", "3C", "3D", "3E", "3F",
    "40", "41", "42", "43", "44", "45", "46", "47", "48", "49", "4A", "4B", "4C", "4D", "4E", "4F",
    "50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "5A", "5B", "5C", "5D", "5E", "5F",
    "60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "6A", "6B", "6C", "6D", "6E", "6F",
    "70", "71", "72", "73", "74", "75", "76", "77", "78", "79", "7A", "7B", "7C", "7D", "7E", "7F",
    "80", "81", "82", "83", "84", "85", "86", "87", "88", "89", "8A", "8B", "8C", "8D", "8E", "8F",
    "90", "91", "92", "93", "94", "95", "96", "97", "98", "99", "9A", "9B", "9C", "9D", "9E", "9F",
    "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "AA", "AB", "AC", "AD", "AE", "AF",
    "B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8", "B9", "BA", "BB", "BC", "BD", "BE", "BF",
    "C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7", "C8", "C9", "CA", "CB", "CC", "CD", "CE", "CF",
    "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "DA", "DB", "DC", "DD", "DE", "DF",
    "E0", "E1", "E2", "E3", "E4", "E5", "E6", "E7", "E8", "E9", "EA", "EB", "EC", "ED", "EE", "EF",
    "F0", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "FA", "FB", "FC", "FD", "FE", "FF",
};

// Returns the hex digit for a 4 bit value.
template <const bool LOWERCASE = false>
static inline constexpr char NibbleToHex(uint8_t nibble)
{
    return LOWERCASE ? s_HexCharsLower[nibble] : s_HexChars[nibble];
}

constexpr static const char s_ASCIIPrintableCharacterMap[] = "................................"
                                                           " !\"#$%&\'()*+,-./0123456789:;<=>?"
                                                           "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
                                                           "`abcdefghijklmnopqrstuvwxyz{|}~."
                                                           "................................"
                                                           "................................"
                                                           "................................"
                                                           "................................";

// Returns only printable ASCII characters for a byte.
static inline constexpr char ByteToASCIIPrintable(const uint8_t character)
{
    return s_ASCIIPrintableCharacterMap[character];
}

// Returns the hex string for a byte.
template <bool LOWERCASE = false, const int WIDTH = 2>
static inline constexpr const char* ByteToHex(const uint8_t byteValue)
{
    static_assert(WIDTH > 0 && WIDTH <= 2, "Invalid width provided to ByteToHex()");

    if (IS_NIBBLE_SET(byteValue, 1) && WIDTH == 1)
    {
        return LOWERCASE ? s_HexCharsByteLower[byteValue] + 1 : s_HexCharsByte[byteValue] + 1;
    }

    return LOWERCASE ? s_HexCharsByteLower[byteValue] : s_HexCharsByte[byteValue];
}

// Places the hex string for a uint8_t into a buffer.
template <const bool LOWERCASE = false, const int WIDTH = 2>
inline size_t EmitByteToHex(char* buffer, const uint8_t byteValue)
{
    static_assert(WIDTH > 0 && WIDTH <= 2, "Invalid width provided to EmitByteToHex()");

    size_t stringSize = 0U;

    if (IS_NIBBLE_SET(byteValue, 1) || WIDTH > 1)
        buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(byteValue, 1));

    buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(byteValue, 0));

    return stringSize;
}

// Places the hex string for a word into a buffer.
template <const bool LOWERCASE = false, const int WIDTH = 4>
inline size_t EmitWordToHex(char* buffer, const uint16_t wordValue)
{
    static_assert(WIDTH > 0 && WIDTH <= 4, "Invalid width provided to EmitWordToHex()");

    size_t stringSize = 0U;

    if (IS_NIBBLE_SET(wordValue, 3) || WIDTH > 3)
            buffer[stringSize++ ] = NibbleToHex<LOWERCASE>(GET_NIBBLE(wordValue, 3));

    if (IS_NIBBLE_SET(wordValue, 2) || WIDTH > 2)
            buffer[stringSize++ ] = NibbleToHex<LOWERCASE>(GET_NIBBLE(wordValue, 2));

    if (IS_NIBBLE_SET(wordValue, 1) || WIDTH > 1)
            buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(wordValue, 1));

    buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(wordValue, 0));

    return stringSize;
}

// Places the hex string for a double word into a buffer.
template <const bool LOWERCASE = false, const int WIDTH = 8>
inline size_t EmitDwordToHex(char* buffer, const uint32_t doubleWordValue)
{
    static_assert(WIDTH > 0 && WIDTH <= 8, "Invalid width provided to EmitDwordToHex()");

    size_t stringSize = 0U;

    if (IS_NIBBLE_SET(doubleWordValue, 7 ) || WIDTH > 7)
        buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(doubleWordValue, 7));

    if (IS_NIBBLE_SET(doubleWordValue, 6) || WIDTH > 6)
        buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(doubleWordValue, 6));

    if (IS_NIBBLE_SET(doubleWordValue, 5) || WIDTH > 5)
        buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(doubleWordValue, 5));

    if (IS_NIBBLE_SET(doubleWordValue, 4) || WIDTH > 4)
        buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(doubleWordValue, 4));

    if (IS_NIBBLE_SET(doubleWordValue, 3) || WIDTH > 3)
        buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(doubleWordValue, 3));

    if (IS_NIBBLE_SET(doubleWordValue, 2) || WIDTH > 2)
        buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(doubleWordValue, 2));

    if (IS_NIBBLE_SET(doubleWordValue, 1) || WIDTH > 1)
        buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(doubleWordValue, 1));

    buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(doubleWordValue, 0));

    return stringSize;
}

// Places the hex string for a quad word into a buffer.
template <const bool LOWERCASE = false, const int WIDTH = 16>
inline size_t EmitQwordToHex(char* buffer, const uint64_t quadWordValue)
{
    static_assert(WIDTH > 0 && WIDTH <= 16, "Invalid width provided to EmitQwordToHex()");

    size_t stringSize = 0U;

    if (IS_NIBBLE_SET(quadWordValue, 15) || WIDTH > 15)
        buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(quadWordValue, 15));

    if (IS_NIBBLE_SET(quadWordValue, 14) || WIDTH > 14)
        buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(quadWordValue, 14));

    if (IS_NIBBLE_SET(quadWordValue, 13) || WIDTH > 13)
        buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(quadWordValue, 13));

    if (IS_NIBBLE_SET(quadWordValue, 12) || WIDTH > 12)
        buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(quadWordValue, 12));

    if (IS_NIBBLE_SET(quadWordValue, 11) || WIDTH > 11)
        buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(quadWordValue, 11));

    if (IS_NIBBLE_SET(quadWordValue, 10) || WIDTH > 10)
        buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(quadWordValue, 10));

    if (IS_NIBBLE_SET(quadWordValue, 9) || WIDTH > 9)
        buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(quadWordValue, 9));

    if (IS_NIBBLE_SET(quadWordValue, 8) || WIDTH > 8)
        buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(quadWordValue, 8));

    if (IS_NIBBLE_SET(quadWordValue, 7) || WIDTH > 7)
        buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(quadWordValue, 7));

    if (IS_NIBBLE_SET(quadWordValue, 6) || WIDTH > 6)
        buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(quadWordValue, 6));

    if (IS_NIBBLE_SET(quadWordValue, 5) || WIDTH > 5)
        buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(quadWordValue, 5));

    if (IS_NIBBLE_SET(quadWordValue, 4) || WIDTH > 4)
        buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(quadWordValue, 4));

    if (IS_NIBBLE_SET(quadWordValue, 3) || WIDTH > 3)
        buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(quadWordValue, 3));

    if (IS_NIBBLE_SET(quadWordValue, 2) || WIDTH > 2)
        buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(quadWordValue, 2));
        
    if (IS_NIBBLE_SET(quadWordValue, 1) || WIDTH > 1)
        buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(quadWordValue, 1));

    buffer[stringSize++] = NibbleToHex<LOWERCASE>(GET_NIBBLE(quadWordValue, 0));

    return stringSize;
}
 
// Utility function for dumping buffers or memory into a human readable format.
//
// TODO:
// Float support?
// Decimal support?
void DumpHex(const void* buffer, const size_t size, const size_t columns = 16, 
    const bool showAbsoluteAddress = false, const bool showASCII = false, 
    const size_t byteGrouping = 1, const bool byteSwapGroup = true);