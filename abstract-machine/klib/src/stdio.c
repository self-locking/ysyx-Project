#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
#define MAX_LENGTH 1024

int handle_fmt(char *buf, const char *fmt, va_list ap) {
    char c;
    int count = 0;

    while ((c = *fmt++)) {
        if (count == MAX_LENGTH - 1) panic("Out of string buff");
        switch (c) {
            case '%': {
                c = *fmt++;
                int width = 0;        // 宽度
                char pad_char = ' ';  // 填充
                int base = 10;
                int is_long = 0;      // 是否是long

                if (c == '0') {
                    pad_char = '0';
                    c = *fmt++;
                }

                while (c >= '0' && c <= '9') {
                    width = width * 10 + c - '0';
                    c = *fmt++;
                }

                if (c == 'l') {
                    is_long = 1;
                    c = *fmt++;
                }

                switch (c) {
                    case 'x':
                    case 'X':
                        base = 16;
                    case 'd': {
                        unsigned long u;
                        int neg = 0;   // 是否为负

                        if (base == 10) {
                            // 统一使用 long 接收，int 会被提升
                            long d = is_long ? va_arg(ap, long) : va_arg(ap, int);
                            if (d < 0) {
                                neg = 1;
                                u = -(unsigned long)d;
                            } else {
                                u = d;
                            }
                        } else if (base == 16) {
                            u = is_long ? va_arg(ap, unsigned long)
                                         : va_arg(ap, unsigned int);
                        }

                        char numbuf[32];
                        int len = 0;

                        if (u == 0) {
                            numbuf[len++] = '0';
                        }

                        while (u > 0) {
                            int digit = u % base;
                            if (digit < 10) {
                                numbuf[len++] = '0' + digit;
                            } else {
                                numbuf[len++] = (c == 'x') ? 'a' + digit - 10
                                                           : 'A' + digit - 10;
                            }
                            u /= base;
                        }

                        int total_len = len + neg;

                        // 0 填充时先输出负号
                        if (neg && pad_char == '0') {
                            *buf++ = '-';
                            count++;
                            neg = 0;
                        }

                        while (total_len < width) {
                            *buf++ = pad_char;
                            count++;
                            total_len++;
                        }

                        // 普通情况输出负号
                        if (neg) {
                            *buf++ = '-';
                            count++;
                        }

                        for (int i = len - 1; i >= 0; i--) {
                            *buf++ = numbuf[i];
                            count++;
                        }
                        break;
                    }

                    case 'c': {
                        char ch = va_arg(ap, int);
                        *buf++ = ch;
                        count++;
                        break;
                    }

                    case 's': {
                        char *p = va_arg(ap, char *);
                        while (*p) {
                            *buf++ = *p++;
                            count++;
                        }
                        break;
                    }

                    case '%': {
                        *buf++ = '%';
                        count++;
                        break;
                    }

                    default: {
                        *buf = '\0';
                        putstr("May Bad\n");
                        return -1;
                    }
                }
                break;
            }

            default: {
                *buf++ = c;
                count++;
                break;
            }
        }
    }
    *buf = '\0';
    return count;
}

int printf(const char *fmt, ...) {
  va_list ap;
  int count = 0;
  char buf[MAX_LENGTH];

  va_start(ap, fmt); // 指向最后一个固定参数fmt
  
  count =  handle_fmt(buf, fmt, ap);
  for(int i = 0; i < count; i++) {
    putch(buf[i]);
  }

  va_end(ap); // C标准要求调用va_end
  
  return count;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap; // 参数列表栈针
  int count = 0;
  va_start(ap, fmt);

  count = handle_fmt(out, fmt, ap);

  va_end(ap);

  return count;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
