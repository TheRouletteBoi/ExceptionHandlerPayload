#include "Types.h"

void* memcpy(void* destination, void const* source, size_t size);
void* memset(void* destination, int val, size_t size);
void* memmove(void* destination, void const* source, size_t size);
int memcmp(const void* destination, const void* source, size_t size);
int strcmp(char const* string1, char const* string2);
size_t strlen(const char* string);
char* strcpy(char* destination, const char* source);
char* strcat(char* destination, char const* source);
char* strncat(char* destination, const char* source, size_t count);
char* strstr(const char *haystack, const char *needle);