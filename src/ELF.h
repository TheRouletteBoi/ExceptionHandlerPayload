#ifndef ELF_H
#define ELF_H

typedef struct OPD_s
{
    uint64_t Function;
    uint64_t TOC;
    uint64_t Enviroment;
} OPD_t;

#endif // !ELF_H