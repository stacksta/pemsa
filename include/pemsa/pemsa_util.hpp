#ifndef PEMSA_UTIL_HPP
#define PEMSA_UTIL_HPP

#define HEX_TO_INT(c) (c >= 'a') ? (c - 'a' + 10) : (c - '0')
#define SET_HALF(b, v, r) (r ? ((b & 0xf0) | (v & 0x0f)) : ((b & 0x0f) | (v << 4)))
#define GET_HALF(b, r) (r ? (b & 0x0f) : ((b & 0xf0) >> 4))

#endif