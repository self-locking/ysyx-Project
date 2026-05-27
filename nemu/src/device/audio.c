/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>
#include <device/map.h>
#include <SDL2/SDL.h>

enum {
  reg_freq,
  reg_channels,
  reg_samples,
  reg_sbuf_size,
  reg_init,
  reg_count,
  nr_reg
};

static uint8_t *sbuf = NULL;
static uint32_t *audio_base = NULL;

static uint32_t sbuf_head = 0;  // SDL 回调读取位置
static uint32_t sbuf_tail = 0;  // AM 写入位置

// SDL 音频回调：NEMU 定期被 SDL 调用，从 sbuf 取数据送给声卡
static void sdl_audio_callback(void *userdata, uint8_t *stream, int len) {
  uint32_t count = audio_base[reg_count];
  
  // 实际可提供的数据量
  int available = (len <= count) ? len : count;
  int silence   = len - available;
  
  // 从环形缓冲区读出 available 字节
  for (int i = 0; i < available; i++) {
    stream[i] = sbuf[sbuf_head];
    sbuf_head = (sbuf_head + 1) % CONFIG_SB_SIZE;
  }
  
  // 数据不足时填静音，避免噪音
  if (silence > 0) {
    memset(stream + available, 0, silence);
  }
  
  // 更新 count：原子地减少已消耗字节数
  audio_base[reg_count] -= available;
}

static void audio_io_handler(uint32_t offset, int len, bool is_write) {
  // 监听 reg_init 寄存器的写入，触发 SDL 音频初始化
  if (is_write) {
    if (offset == reg_init * sizeof(uint32_t)) {
      // 只有 init 被置1时才初始化
      if (audio_base[reg_init]) {
        SDL_AudioSpec wanted = {
          .freq     = audio_base[reg_freq],
          .format   = AUDIO_S16SYS,          // 16位有符号，本机字节序
          .channels = audio_base[reg_channels],
          .samples  = audio_base[reg_samples], // 每次回调的采样数
          .callback = sdl_audio_callback,
          .userdata = NULL,
        };
        SDL_InitSubSystem(SDL_INIT_AUDIO);
        SDL_OpenAudio(&wanted, NULL);  // 按 wanted 严格初始化
        SDL_PauseAudio(0);             // 开始播放（解除暂停）
        
        // 初始化缓冲区状态
        audio_base[reg_sbuf_size] = CONFIG_SB_SIZE;
        audio_base[reg_count]     = 0;
        sbuf_head = 0;
        sbuf_tail = 0;
      }
    }
  }
}

void init_audio() {
  uint32_t space_size = sizeof(uint32_t) * nr_reg;
  audio_base = (uint32_t *)new_space(space_size);
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
  add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif

  sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE);
  add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL);
}
