#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  va_list ap;
  char c;
  int count = 0;

  va_start(ap, fmt);
  while ((c = *fmt++)) {
    switch(c) {
      case '%': {
        c = *fmt++;
        switch(c) {
          case 'c': {
            /* char is promoted to int */
            char ch = va_arg(ap, int);
            putch(ch);
            count++;
            break;
          }

          case 's': {
            char *p = va_arg(ap, char *);
            while (*p) {
              putch(*p++);
              count++;
            }
            break;
          }

          case 'd': {
            int d = va_arg(ap, int);
            if (d < 0) {
              putch('-');
              count++;
              d = -d;
            }
            int tmp  = 1;
            while (d / tmp >= 10) tmp *= 10;
            while (tmp >= 1) {
              putch(d / tmp + '0');
              count++;
              d %= tmp;
              tmp /= 10;
            }
            break;
          }

          case 'f': {
            double f = va_arg(ap, double);
            if (f < 0) {
              putch('-');
              count++;
              f = -f;
            }
            int tmp = 1;
            int integer = f / 1;
            while (integer / tmp >= 10) tmp *= 10;
            while (tmp >= 1) {
              putch(integer / tmp + '0');
              count++;
              integer %= tmp;
              tmp /= 10;
            }

            f -= integer;
            putch('.');
            count++;
            integer = f * 1000000; // print 6 digits after the decimal point
            tmp  = 1;
            while (integer / tmp >= 10) tmp *= 10;
            while (tmp >= 1) {
              putch(integer / tmp + '0');
              integer %= tmp;
              tmp /= 10;
            }
            break;
          }

          default: {
            return -1;
          }
        }
        break;
      }

      default: {
        putch(c);
        count++;
        break;
      }
    }
  }

  va_end(ap);
  return count;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap; // 参数列表栈针
  char c;
  int count = 0;

  va_start(ap, fmt); // 指向第一个参数
  /* 双括号告诉编译器是赋值不是比较 */
  while ((c = *fmt++)) {
    switch(c) {
      case '%': {
        c = *fmt++;
        switch(c) {
          case 'c': {
            /* va_arg将参数以...类型提取并指向下一个参数 */
            char ch = va_arg(ap, int); // char被提升为int
            *out++ = ch;
            count++;
            break;
          }

          case 's': {
            char *p = va_arg(ap, char *);
            while (*p) {
              *out++ = *p++;
              count++;
            }
            break;
          }

          case 'd': {
            int d = va_arg(ap, int);
            if (d < 0) {
              *out++ = '-';
              count++;
              d = -d; // 转为正数方便处理
            }
            int tmp = 1;
            while (d / tmp >= 10) tmp *= 10; // 找最高位
            while (tmp >= 1) {
              *out++ = d / tmp + '0';
              count++;
              d %= tmp;
              tmp /= 10;
            }
            break;
          }

          case '%': {
            *out++ = '%';
            count++;
            break;
          }

          default: {
            *out = '\0';
            return -1; // 未实现或错误类型
          }
        }
        break;
      }

      default: {
        *out++ = c;
        count++;
        break;
      }
    }
  }

  *out = '\0';
  va_end(ap); // C标准要求调用va_end
  return count;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
