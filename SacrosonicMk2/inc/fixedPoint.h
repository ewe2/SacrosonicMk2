#ifndef FIXEDPOINT_H_INCLUDED
#define FIXEDPOINT_H_INCLUDED

#include <stdint.h>

typedef union {
    struct {
        uint16_t f;
        uint16_t i;
    } p;
    uint32_t c;
} FixedPoint;

#endif /* FIXEDPOINT_H_INCLUDED */
