
#include "Hex.h"
#include "Libc.h"
#include "LV2/LV2.h"

void DumpHex(const void* buffer, const size_t size, const size_t columns, 
    const bool showAbsoluteAddress, const bool showASCII, const size_t byteGrouping, 
    const bool byteSwapGroup)
{

    char string[DUMP_HEX_CHUNK_SIZE];
    size_t stringIndex = 0;

    // Example Line:
    // 0x12345678: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 |................|
    // 
    // 0x12345678: 00000000 00000000 00000000 00000000 00 00 00 00 |................|

    const auto addressLength    = sizeof( uintptr_t ) * 2;
    const auto columnDataWidth  = byteGrouping * 2;
    const auto columnCharWidths = ( columns * columnDataWidth ) + columns;
    const auto asciiDumpWidth   = ( showASCII ? ( 2 + columns ) : 0 );
    const auto maxLineWidth     = addressLength + columnCharWidths + asciiDumpWidth;

    for (size_t i = 0; i < size; ++i)
    {
        if ((i % columns) == 0)
        {
            // Ensure we dont print a newline or ascii dump for first byte.
            if (i != 0)
            {
                if (showASCII)
                {
                    // We are currently after the last printed column so
                    // finsh the line with the ASCII dump.
                    string[stringIndex++] = '|';

                    stringIndex += columns;

                    string[stringIndex++] = '|';
                }

                string[stringIndex++] = '\n';
            }

            // Flush the buffer if we have no space left.
            // 
            // This has to be done after completing the current line as
            // the ASCII dump will be written ahed of the current string index.
            if (stringIndex >= DUMP_HEX_CHUNK_SIZE - (maxLineWidth + 1))
            {
                string[stringIndex] = NULL;

                g_LibLV2.lv2_kern_tty_write(string, stringIndex);

                stringIndex = 0;
            }

            // Write offset or absolute address.
            const auto address = (showAbsoluteAddress ? (uintptr_t)buffer : 0) + i;

            string[stringIndex++] = '0';
            string[stringIndex++] = 'x';

            #ifdef	__LP64__
                stringIndex += EmitQwordToHex<false, 16>(&string[stringIndex], address);
            #else
                stringIndex += EmitDwordToHex<false, 8>(&string[stringIndex], address);
            #endif

            
            string[stringIndex++] = ':';
            string[stringIndex++] = ' ';
        }

        const auto currentByte    = ((uint8_t*)buffer)[i];
        //const auto columnGrouping = std::max(Size - i, ByteGrouping);

        // Write column hex value.
        stringIndex += EmitByteToHex<false, 2>(&string[stringIndex], currentByte);

        /*switch (columnGrouping)
        {
            case 1:
                stringIndex += EmitByteToHex<false, 2>(&string[stringIndex], currentByte);
                break;
            case 2:
                stringIndex += EmitWordToHex<false, 2>(&string[stringIndex ], byteSwapGroup ? _byteswap_ushort(currentByte) : currentByte);
                break;
            case 4:
                stringIndex += EmitDwordToHex<false, 2>(&string[stringIndex ], byteSwapGroup ? _byteswap_ulong(currentByte) : currentByte);
                break;
            case 8:
                stringIndex += EmitQwordToHex<false, 2>(&string[stringIndex ], byteSwapGroup ? _byteswap_uint64(currentByte) : currentByte);
                break;
        }*/

        string[stringIndex++] = ' ';

        if (showASCII)
        {
            // Write the ASCII dump ahed of the current string position by calculating the
            // index relative to the just written value.
            //
            // 0x12345678: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 |................|
            //                                             ^
            //	                                          Example postion in this context.
            //
            // Therefore to calculate the position of where the ASCII dump will start is
            // simple as taking the number of columns we have to print and size of each
            // column and spaces between each column aslwell as the dump seperator '|'.
             
            const auto currentByteColumnIndex = (i % columns);
            const auto columnsLeftToPrint     = ((columns - 1) - currentByteColumnIndex);
            const auto numSpaceCharsLeft      =  columnsLeftToPrint;
            const auto numByteCharsLeft       = (columnsLeftToPrint * columnDataWidth) + numSpaceCharsLeft;
            const auto asciiDumpIndex         = (stringIndex + numByteCharsLeft + currentByteColumnIndex) + 1;
                                              
            string[asciiDumpIndex] = ByteToASCIIPrintable(currentByte);
        }
    }

    if (showASCII)
    {
        // If the size of the dump was not a multiple of the number of columns
        // we may still have residual columns to print.
        const auto residualData    = size % columns;
        const auto residualColumns = columns - residualData;
        const auto pad             = (residualColumns * columnDataWidth) + residualColumns;

        // Pad with spaces from the last printed byte to the start of the ASCII dump.
        // 
        // 0x12345678: 00 00 00 00 00 00 00 00 00 00 00                |...........     |
        //                                              ^
        //										     Padding this with spaces
        if (residualData != 0)
        {
            memset(&string[stringIndex], ' ', pad);

            stringIndex += pad;
        }

        string[stringIndex++] = '|';

        // Pad with spaces for the missing ASCII chars in the ASCII dump.
        // 
        // 0x12345678: 00 00 00 00 00 00 00 00 00 00 00                |...........     |
        //                                                                          ^
        //		                                                                  Padding this with spaces
        if (residualData != 0)
        {
            memset(&string[stringIndex + residualData], ' ', residualColumns);
        }

        stringIndex += columns;

        string[stringIndex++] = '|';
    }

    // Write any remaining lines.
    if (stringIndex != 0)
    {
        string[stringIndex++ ] = '\n';
        string[stringIndex ] = NULL;

        g_LibLV2.lv2_kern_tty_write(string, stringIndex);
    }
}
