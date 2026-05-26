#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  uint32_t data = inl(VGACTL_ADDR);
  uint32_t width = data >> 16;
  uint32_t height = data & 0xFFFF;

  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = width, .height = height,
    .vmemsz = width * height * sizeof(uint32_t)
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int x = ctl->x, y = ctl->y;
  int w = ctl->w, h = ctl->h;
  uint32_t *pixels = (uint32_t *)ctl->pixels;
  int screen_w = inl(VGACTL_ADDR) >> 16;          // 屏幕宽度
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;  // 显存地址

  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      // 计算在全屏显存中的一维绝对位置：
      int fb_index = (y + i) * screen_w + (x + j);  // 计算一维数组的屏幕像素下标
      int pixel_index = i * w + j;                  // 计算一维数组的绘制像素下标

      fb[fb_index] = pixels[pixel_index];
    }
  }

  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}

void __am_gpu_init() {
}
