#include "Intellisense.h"

#define SCREEN ((unsigned short*)0x06000000)
#define SCREEN2 ((unsigned short*)0x0600A000)
#define DISPCONTROL *(unsigned int*)0x04000000

struct Point {
	float x;
	float y;
};

typedef struct Point Point;

unsigned char noisemap[64*64] = {0};

int dblbuf = -1;

unsigned char lfsrnoise(int x, int y, int z) {
	int n = x + y + z;
	n = (n >> 16) ^ n;
	n = (n >> 8) ^ n;
	return (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
}

unsigned char voxel_get(int x, int y) {
	return noisemap[(y&63)*64+(x&63)];
}

void generate_noisemap() {
	for (int y = 0; y < 64; y++) {
		for (int x = 0; x < 64; x++) {
			noisemap[y*64+x] = lfsrnoise(x, y, 0)%128;
		}
	}
}

void clearscreen() {
	if (dblbuf == 1) {
		for (int i = 0; i < 160*128; i++) {
			SCREEN[i] = 0xFF;
		}
	} else {
		for (int i = 0; i < 160*128; i++) {
			SCREEN2[i] = 0xFF;
		}
	}
}

void draw_vertical_line(int x, int height, int color) {
	if (x < 0) return;
	if (x > 240) return;
	if (height >= 128) return;
	if (height <= 0) return;

	if (dblbuf == -1) {
		for (int i = height; i < 128; i++) {
			SCREEN[x + i * 160] = color;
		}
	} else {
		for (int i = height; i < 128; i++) {
			SCREEN2[x + i * 160] = color;
		}
	}
}

void voxel_render(Point p, int height, int horizon, int scale_height, 
				  int distance, int screen_width, int screen_height) {

	// draw from back to front (high z coordinate to low z coordinate)
	for (float z = distance; z > 1; z-=16) {
		// find line on map
		Point pleft;
		pleft.x = -z+p.x;
		pleft.y = -z+p.y;

		Point pright;
		pright.x = z+p.x;
		pright.y = -z+p.y;

		// line segmentation
		float dx = (pright.x - pleft.x) / screen_width;

		// raster the line and draw a vertical line for each segment
		for (int i = 0; i < screen_width; i+=1) {
			int height_on_screen = abs(height - voxel_get(pleft.x,pleft.y)) / z * scale_height + horizon;
			
			int c = 0xFFFF-height_on_screen*16;
			draw_vertical_line(i, height_on_screen, c);
			pleft.x += dx;
		}
	}
}

int main()
{
	generate_noisemap();

	Point p;
	p.x = 0;
	p.y = 0;
	int t = 0;
	while(1){
		dblbuf = -dblbuf;
		if (dblbuf == 1) {
			DISPCONTROL = 0x0405;
		} else {
			DISPCONTROL = 0x0415;
		}
		p.x=t;
		voxel_render(p, 128, 10, 20, 150, 160, 128);
		t++;
	}
	return 0;
}