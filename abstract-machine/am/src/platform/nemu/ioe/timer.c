#include <am.h>
#include <nemu.h>

void __am_timer_init() {
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uint64_t us = 0;
  uint32_t high_us = inl(RTC_ADDR + 4); // 读取第二个时间寄存器
  uint32_t low_us = inl(RTC_ADDR);
  us = (uint64_t)low_us | (uint64_t)high_us << 32;
  uptime->us = us;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
