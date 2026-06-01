#include <am.h>

#define RTC_ADDR_LOW  (*(volatile uint32_t *)0x02004000)
#define RTC_ADDR_HIGH (*(volatile uint32_t *)0x02004004)

void __am_timer_init() {
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uint64_t us = 0;

  uint32_t high = RTC_ADDR_HIGH;
  uint32_t low = RTC_ADDR_LOW;

  us = (uint64_t)low | (uint64_t)high << 32;
  
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
