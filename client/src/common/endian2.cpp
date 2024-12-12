#include "endian2.h"

namespace bcus {

namespace
{
    static struct check_endian
    {
        check_endian()
        {
            int n = 0x01020304;
            char c = *(char *)(&n);
            is_big_endin = (c == 0x01);
        }
        bool is_big_endin;
    } check_endian;
}

uint16_t endian_swap(uint16_t v)
{
    if (check_endian.is_big_endin)
        return v;
    return ((v & 0xff00) >> 8) | ((v & 0x00ff) << 8);
}

uint32_t endian_swap(uint32_t v)
{
    if (check_endian.is_big_endin)
        return v;
    return ((v & 0xff000000) >> 24) | ((v & 0x00ff0000) >> 8) |
        ((v & 0x0000ff00) << 8) | ((v & 0x000000ff) << 24);
}

uint64_t endian_swap(uint64_t v)
{
    if (check_endian.is_big_endin)
        return v;
    return  ((v & 0xff00000000000000ull) >> 56) |
        ((v & 0x00ff000000000000ull) >> 40) |
        ((v & 0x0000ff0000000000ull) >> 24) |
        ((v & 0x000000ff00000000ull) >>  8) |
        ((v & 0x00000000ff000000ull) <<  8) |
        ((v & 0x0000000000ff0000ull) << 24) |
        ((v & 0x000000000000ff00ull) << 40) |
        ((v & 0x00000000000000ffull) << 56);
}

// #include <iostream>
// void test_endian()
// {
//     uint16_t w = 0x0102;
//     printf("%04x, %04x\n", w, endian_swap(w));
//     uint32_t n = 0x01020304;
//     printf("%08x, %08x\n", n, endian_swap(n));
//     uint64_t n2 = 0x0102030405060708ll;
//     printf("%016llx, %016llx\n", n2, endian_swap(n2));
// };

}