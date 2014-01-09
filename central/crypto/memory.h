#include <stddef.h>
/*
 * Secure version of memset, memcpy, memmove.
 * Adapted from "Secure Programming Cookbook for C and C++ - VM (2003)"
 * 
 * Dead-code elimination passes might remove the original functions since some
 * compilers implement them as built-ins, and might remove them if the compiler
 * is sure that the data written is never read again.
 *
 * Memset before freeing critical data.
 */

/*
 * A secure version of memset.
 */
volatile void *secure_memset(volatile void *dst, int c, size_t len) 
{
    volatile char *buf;

    for (buf = (volatile char *)dst; len; buf[--len] = c);
        return dst;
}

/*
 * A secure version of memcpy.
 */
volatile void *secure_memcpy(volatile void *dst, volatile void *src, 
        size_t len) 
{
    volatile char *cdst, *csrc;

    cdst = (volatile char *)dst;
    csrc = (volatile char *)src;

    while (len--) cdst[len] = csrc[len];

    return dst;
}

/*
 * A secure version of memmove.
 */
volatile void *secure_memmove(volatile void *dst, volatile void *src,
        size_t len) 
{
    size_t i;
    volatile char *cdst, *csrc;

    cdst = (volatile char *)dst;
    csrc = (volatile char *)src;

    if (csrc > cdst && csrc < cdst + len)
        for (i = 0; i < len; i++) cdst[i] = csrc[i];
    else
        while (len--) cdst[len] = csrc[len];

    return dst;
}
