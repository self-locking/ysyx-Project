#include <am.h>
#include <klib-macros.h>

#define WIDTH 400
#define HEIGHT 300

static uint32_t color_buf[WIDTH * HEIGHT];

void draw(uint32_t color) {
	// add code here
	for(int i = 0; i < WIDTH * HEIGHT; i++){
		color_buf[i] = color;
	}
	io_write(AM_GPU_FBDRAW, 0, 0, color_buf, WIDTH, HEIGHT, true);
}

static inline int abs(int x) { return x < 0 ? -x : x; }
static inline int max(int a, int b) { return a > b ? a : b; }
static inline int max3(int a, int b, int c) { return max(a, max(b, c)); }

int main() {
	uint32_t color[8] = {0x000000, 0xff0000, 0x00ff00, 0x0000ff,
						 0xff00ff, 0x00ffff, 0xffff00, 0xffffff};	

	ioe_init(); // initialization for GUI
	int current = 0;
	int run = 1;
	int n = 33000;
	while (run) {
        int next = (current + 1) % 8; // loop from 0 to 7

        int R1 = (color[current] >> 16) & 0xff;
        int G1 = (color[current] >> 8)  & 0xff;
        int B1 = (color[current] >> 0)  & 0xff;

        int R2 = (color[next] >> 16) & 0xff;
        int G2 = (color[next] >> 8)  & 0xff;
        int B2 = (color[next] >> 0)  & 0xff;

        int dR = R2 - R1;
        int dG = G2 - G1;
        int dB = B2 - B1;

		//compute the bigest steps
        int steps = max3(abs(dR), abs(dG), abs(dB));
		if (steps == 0) steps = 1;

        for (int i = 0; i <= steps; i++) {
            int r = R1 + dR * i / steps;
            int g = G1 + dG * i / steps;
            int b = B1 + dB * i / steps;
            uint32_t c = (r << 16) | (g << 8) | b;
            draw(c);

			//monitor the keyboard
			AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
			if(ev.keycode != AM_KEY_NONE && ev.keydown){
				if(ev.keycode == AM_KEY_ESCAPE) {
					run = 0;
					break;
				}
				else { n = 3300; }	
			}
			else { n = 33000; }

            // delay n ms
            uint64_t next_time = io_read(AM_TIMER_UPTIME).us + n;
            while (io_read(AM_TIMER_UPTIME).us < next_time && run);
        }

        current = next;
    }
	return 0;
}
