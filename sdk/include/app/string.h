#ifndef __STRING_H__
#define __STRING_H__
#include <stddef.h>
#include <stdint.h>
void*
memcpy(void* dest, const void* src, size_t len);
void*
memset(void* dest, int byte, size_t len);
int
memcmp(const void* ptr1, const void* ptr2, size_t len);
void*
memmove(void* dest, const void* src, size_t count);
size_t
strlen(const char* str);
char
*strerror(int errnum);
char
*strerror_r(int errnum, char *buf, size_t buflen);
char *
strncpy(char *dst, const char *src, size_t len);
int
strncmp(const char *s1, const char *s2, size_t len);
char
*strstr(const char *haystack, const char *needle);
char
*strncat(size_t ssize;
	char *restrict dst, const char src[restrict ssize],
	size_t ssize);
int
strcmp(const char *s1, const char *s2);
char *strchr(const char *s, int c);

char *strrchr(const char *s, int c);
#endif
