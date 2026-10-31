/* Tiny libc subset so the portable core builds for wasm32 without a C library. */
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

void *memcpy(void *d, const void *s, size_t n)
{
    unsigned char *a = d; const unsigned char *b = s;
    while (n--) *a++ = *b++;
    return d;
}

void *memmove(void *d, const void *s, size_t n)
{
    unsigned char *a = d; const unsigned char *b = s;
    if (a == b || n == 0) return d;
    if (a < b) { while (n--) *a++ = *b++; }
    else { a += n; b += n; while (n--) *--a = *--b; }
    return d;
}

void *memset(void *d, int c, size_t n)
{
    unsigned char *a = d;
    while (n--) *a++ = (unsigned char)c;
    return d;
}

int memcmp(const void *x, const void *y, size_t n)
{
    const unsigned char *a = x, *b = y;
    while (n--) { if (*a != *b) return *a < *b ? -1 : 1; a++; b++; }
    return 0;
}

size_t strlen(const char *s) { size_t n = 0; while (s[n]) n++; return n; }
char *strcpy(char *d, const char *s) { char *r = d; while ((*d++ = *s++) != 0) {} return r; }
int strcmp(const char *a, const char *b)
{
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}
int strncmp(const char *a, const char *b, size_t n)
{
    while (n && *a && *a == *b) { a++; b++; n--; }
    return n ? (unsigned char)*a - (unsigned char)*b : 0;
}
char *strchr(const char *s, int c)
{
    for (;; s++) {
        if (*s == (char)c) return (char *)s;
        if (!*s) return 0;
    }
}

int snprintf(char *buf, size_t cap, const char *fmt, ...)
{
    va_list ap;
    size_t o = 0;
    va_start(ap, fmt);
#define OUT(ch) do { if (o + 1 < cap) buf[o] = (char)(ch); o++; } while (0)
    while (*fmt) {
        if (*fmt != '%') { OUT(*fmt++); continue; }
        {
            int left = 0, width = 0;
            fmt++;
            if (*fmt == '-') { left = 1; fmt++; }
            while (*fmt >= '0' && *fmt <= '9') width = width * 10 + (*fmt++ - '0');
            if (*fmt == 'd') {
                int v = va_arg(ap, int);
                char t[16];
                int n = 0, i;
                unsigned u = v < 0 ? 0u - (unsigned)v : (unsigned)v;
                if (u == 0) t[n++] = '0';
                while (u) { t[n++] = (char)('0' + u % 10u); u /= 10u; }
                if (v < 0) t[n++] = '-';
                for (i = n; i < width && !left; i++) OUT(' ');
                for (i = n - 1; i >= 0; i--) OUT(t[i]);
                for (i = n; i < width && left; i++) OUT(' ');
            } else if (*fmt == 's') {
                const char *s = va_arg(ap, const char *);
                int n = (int)strlen(s), i;
                for (i = n; i < width && !left; i++) OUT(' ');
                while (*s) OUT(*s++);
                for (i = n; i < width && left; i++) OUT(' ');
            } else if (*fmt == '%') {
                OUT('%');
            }
            if (*fmt) fmt++;
        }
    }
#undef OUT
    if (cap) buf[o < cap ? o : cap - 1] = 0;
    va_end(ap);
    return (int)o;
}
