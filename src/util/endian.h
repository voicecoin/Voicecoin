#ifndef BCUS_ENDIAN_H
#define BCUS_ENDIAN_H

#include <stdint.h>

uint16_t endian_swap(uint16_t v);
uint32_t endian_swap(uint32_t v);
uint64_t endian_swap(uint64_t v);

#endif // BCUS_ENDIAN_H
