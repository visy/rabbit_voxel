#include "Intellisense.h"
#include <stdint.h>

#define SCREEN ((unsigned short*)0x06000000)
#define SCREEN2 ((unsigned short*)0x0600A000)
#define DISPCONTROL *(unsigned int*)0x04000000

struct Point {
	float x;
	float y;
};

typedef struct Point Point;

typedef unsigned int   uint, eint;
typedef unsigned short ushort, eshort;

typedef uint8_t  u8,  byte, uchar, echar;
typedef uint16_t u16, hword;
typedef uint32_t u32, word;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef volatile u8  vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;
typedef volatile u64 vu64;

typedef volatile s8  vs8;
typedef volatile s16 vs16;
typedef volatile s32 vs32;
typedef volatile s64 vs64;

typedef const u8  cu8;
typedef const u16 cu16;
typedef const u32 cu32;
typedef const u64 cu64;

typedef const s8  cs8;
typedef const s16 cs16;
typedef const s32 cs32;
typedef const s64 cs64;

#define INLINE static inline

typedef s32 FIXED;

#define FIX_SHIFT       8
#define FIX_SCALE       ( 1<<FIX_SHIFT		)
#define FIX_MASK        ( FIX_SCALE-1		)
#define FIX_SCALEF      ( (float)FIX_SCALE	)
#define FIX_SCALEF_INV	( 1.0/FIX_SCALEF	)

INLINE FIXED int2fx(int d)
{   return d<<FIX_SHIFT;    }

INLINE FIXED float2fx(float f)
{   return (FIXED)(f*FIX_SCALEF);   }

INLINE u32 fx2uint(FIXED fx)    
{   return fx>>FIX_SHIFT;   }

INLINE u32 fx2ufrac(FIXED fx)
{   return fx&FIX_MASK; }

INLINE int fx2int(FIXED fx)
{   return fx/FIX_SCALE;    }

INLINE float fx2float(FIXED fx)
{   return fx/FIX_SCALEF;   }

INLINE FIXED fxadd(FIXED fa, FIXED fb)
{   return fa + fb;         }

INLINE FIXED fxsub(FIXED fa, FIXED fb)
{   return fa - fb;         }

INLINE FIXED fxmul(FIXED fa, FIXED fb)
{   return (fa*fb)>>FIX_SHIFT;              }

INLINE FIXED fxdiv(FIXED fa, FIXED fb)
{   return ((fa)*FIX_SCALE)/(fb);           }

#define FX_RECIPROCAL(a, fp)	( ((1<<(fp))+(a)-1)/(a) )
#define FX_RECIMUL(x, a, fp)	( ((x)*((1<<(fp))+(a)-1)/(a))>>(fp) )

unsigned char noisemap[64*64] = {0};
int dblbuf = -1;

unsigned char lfsrnoise(int x, int y, int z) {
	int n = x + y + z;
	n = (n >> 16) ^ n;
	n = (n >> 8) ^ n;
	return (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
}


void generate_noisemap() {
	for (int y = 0; y < 64; y++) {
		for (int x = 0; x < 64; x++) {
			noisemap[y*64+x] = 64-lfsrnoise(x, y, 0)%32;
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

INLINE void draw_vertical_line(int x, int height, int color) {
	if (x < 0) return;
	if (x > 160) return;
	if (height >= 128) height = 128;
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

void voxel_render(Point p, int horizon, 
				  int distance, int screen_width) {

	// draw from back to front (high z coordinate to low z coordinate)
	for (int z = distance; z > 1; z-=4) {
		// find line on map
		Point pleft;
		pleft.x = -z+p.x;
		pleft.y = -z+p.y;

		Point pright;
		pright.x = z+p.x;
		pright.y = -z+p.y;

		// line segmentation
		float dx = fx2float(fxdiv(float2fx(pright.x - pleft.x), float2fx(screen_width)));

		// raster the line and draw a vertical line for each segment
		for (int i = 0; i < screen_width; i+=2) {
			u8 vox = noisemap[(((s8)pleft.x&63)<<6)+((s8)pleft.y&63)];

			int height_on_screen = vox;
			height_on_screen=height_on_screen<<4;
			height_on_screen = fx2float(fxdiv(int2fx(height_on_screen),float2fx(z)));
			height_on_screen += horizon;
			
			int c = 0xFFFF-(height_on_screen<<4);
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
		p.x=t*0.5;
		voxel_render(p, 10, 40, 160);
		t++;
	}
	return 0;
}