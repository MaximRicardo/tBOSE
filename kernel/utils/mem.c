#include <stdint.h>
#include <string.h>

void* memcpy(void *restrict dest, const void *restrict src, size_t n) {
    uint8_t *restrict dest_u8 = (uint8_t*)dest;
    uint8_t *restrict src_u8 = (uint8_t*)src;

    for (size_t i = 0; i < n; i++) {
        *(dest_u8++) = *(src_u8++);
    }

    return dest;
}

void* memmove(void *dest, const void *src, size_t n) {
    unsigned char *pd = dest;
    const unsigned char *ps = src;
    if (ps < pd)
        for (pd += n, ps += n; n--;)
            *--pd = *--ps;
    else
        while(n--)
            *pd++ = *ps++;
    return dest;
}

void* memset(void *s, int c, size_t n) {
    uint8_t *s_u8 = (uint8_t*)s;

    for (size_t i = 0; i < n; i++) {
        *(s_u8++) = c;
    }

    return s;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    size_t i;  
    const unsigned char * cs = (const unsigned char*) s1;
    const unsigned char * ct = (const unsigned char*) s2;

    for (i = 0; i < n; i++, cs++, ct++) {
        if (*cs < *ct) {
            return -1;
        }
        else if (*cs > *ct)
        {
            return 1;
        }
    }

    return 0;
}
