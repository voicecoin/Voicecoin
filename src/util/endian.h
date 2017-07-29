#ifndef BCUS_ENDIAN_H
#define BCUS_ENDIAN_H

#include <stdint.h>

uint16_t endian_swap(uint16_t v);
uint32_t endian_swap(uint32_t v);
uint64_t endian_swap(uint64_t v);

template<typename IntType>
void write_to_buff(unsigned char* ptr, IntType x)
{
    IntType v = endian_swap(x);
    memcpy(ptr, (char*)&v, sizeof(IntType));
}

template<typename IntType>
IntType read_from_buff(const unsigned char* ptr)
{
    IntType x;
    memcpy((char*)&x, ptr, sizeof(IntType));
    return endian_swap(x);
}

#endif // BCUS_ENDIAN_H
