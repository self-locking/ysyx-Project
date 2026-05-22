#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
#define MAX_LENGTH 1024

int handle_fmt(char *buf, const char *fmt, va_list ap) {
  char c;
  int count = 0;

  /* 双括号告诉编译器是赋值不是比较 */
  while ((c = *fmt++)) {
    if (count == MAX_LENGTH - 1) panic("Out of string buff");
    switch(c) {
      case '%': {
        c = *fmt++;
        int width = 0;
        char pad_char = ' ';
        int base = 10;

        if (c == '0') {
        pad_char = '0';
        c = *fmt++;
        }

        while (c >= '0' && c <= '9') {
          width = width * 10 + c - '0';
          c = *fmt++;
        }

        switch(c) {
          case 'x':
          case 'X':
            base = 16;

          case 'd': {
            unsigned int u;
            int neg = 0;

            if (base == 10) {
              int d = va_arg(ap, int);
              if (d < 0) {
                neg = 1;
                u = -d;
              } 
              else u = d;
            } 
            else if (base == 16) u = va_arg(ap, unsigned int);

            // 方便填充采用倒序和临时数组
            char numbuf[32];
            int len = 0;

            if (u == 0) {
                numbuf[len++] = '0';
            }

            while (u > 0) {
              int digit = u % base;

              if (digit < 10) {
                  numbuf[len++] = '0' + digit;
              } 
              else {
                if (c == 'x') {
                    numbuf[len++] = 'a' + digit - 10;
                } 
                else numbuf[len++] = 'A' + digit - 10;
              }
              u /= base;
            }

            int total_len = len + neg;

            // 如果是0填充，先输出负号
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
            /* va_arg(ap, type) 将参数以...类型提取并指向下一个参数 */
            char ch = va_arg(ap, int); // char被提升为int
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
            return -1; // 未实现或错误类型
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
