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
        switch(c) {
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

          case 'd': {
            int d = va_arg(ap, int);
            if (d < 0) {
              *buf++ = '-';
              count++;
              d = -d; // 转为正数方便处理
            }
            int tmp = 1;
            while (d / tmp >= 10) tmp *= 10; // 找最高位
            while (tmp >= 1) {
              *buf++ = d / tmp + '0';
              count++;
              d %= tmp;
              tmp /= 10;
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
