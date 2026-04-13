#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t len = 0;
  while (s[len] != '\0') {len++;}
  return len;
}

char *strcpy(char *dst, const char *src) {
  size_t i = 0;
  while (src[i] != '\0') {
    dst[i] = src[i];
    i++;
  }
  dst[i] = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  size_t i = 0;
  for (; i < n && src[i] != '\0'; i++) {
    dst[i] = src[i];
  }
  for (; i < n; i++) {dst[i] = '\0';}
  return dst;
}

char *strcat(char *dst, const char *src) {
  char *p = dst;
  while (*p != '\0') p++;
  while (*src != '\0') {
    *p = *src;
    p++;
    src++;
  }
  *p = '\0';
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  size_t i = 0;
  for (; s1[i] != '\0' && s2[i] != '\0'; i++) {
    if (s1[i] != s2[i]) {return s1[i] - s2[i];}
  }
  return s1[i] - s2[i];
}

int strncmp(const char *s1, const char *s2, size_t n) {
  size_t i = 0;
  for (; i < n && s1[i] != '\0' && s2[i] != '\0'; i++) {
    if (s1[i] != s2[i]) return s1[i] - s2[i];
  }
  if (i < n) return s1[i] - s2[i];
  return 0;
}

void *memset(void *s, int c, size_t n) {
  uint8_t *p = (uint8_t *)s;
  for (size_t i = 0; i < n; i++) {
    p[i] = (uint8_t)c;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  uint8_t *p = (uint8_t *)dst;
  const uint8_t *q = (const uint8_t *)src;
  if (p < q) {
    // 用位置判断拷贝方向
    for (size_t i = 0; i < n; i++) p[i] = q[i];
  }
  else {
    for (size_t i = n; i > 0; i--) p[i - 1] = q[i - 1];
  }
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  uint8_t *p = (uint8_t *)out;
  const uint8_t *q = (const uint8_t *)in;
  for (size_t i = 0; i < n; i++) {
    p[i] = q[i];
  }
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const uint8_t *p = (const uint8_t *)s1;
  const uint8_t *q = (const uint8_t *)s2;
  for (size_t i = 0; i < n; i++) {
    if (p[i] != q[i]) return p[i] - q[i];
  }
  return 0;
}

#endif
