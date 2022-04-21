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

#define REG_BG2HOFS		*(u16*)0x04000018 //BG2 X-Offset
#define REG_BG2VOFS		*(u16*)0x0400001A //BG2 Y-Offset

#define REG_BG2PA		*(u16*)0x04000020 //BG2 Rotation/Scaling Paramater A
#define REG_BG2PB		*(u16*)0x04000022 //BG2 Rotation/Scaling Paramater B
#define REG_BG2PC		*(u16*)0x04000024 //BG2 Rotation/Scaling Paramater C
#define REG_BG2PD		*(u16*)0x04000026 //BG2 Rotation/Scaling Paramater D
#define REG_BG2X		*(u32*)0x04000028 //BG2 Reference Point X-Coordinate
#define REG_BG2Y		*(u32*)0x0400002C //BG2 Reference Point Y-Coordinate

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

const s16 SIN[360] = {    0,    4,    8,   13,   17,   22,   26,   31,   35,   40,
  44,   48,   53,   57,   61,   66,   70,   74,   79,   83,
  87,   91,   95,  100,  104,  108,  112,  116,  120,  124,
 127,  131,  135,  139,  143,  146,  150,  154,  157,  161,
 164,  167,  171,  174,  177,  181,  184,  187,  190,  193,
 196,  198,  201,  204,  207,  209,  212,  214,  217,  219,
 221,  223,  226,  228,  230,  232,  233,  235,  237,  238,
 240,  242,  243,  244,  246,  247,  248,  249,  250,  251,
 252,  252,  253,  254,  254,  255,  255,  255,  255,  255,
 255,  255,  255,  255,  255,  255,  254,  254,  253,  252,
 252,  251,  250,  249,  248,  247,  246,  244,  243,  242,
 240,  238,  237,  235,  233,  232,  230,  228,  226,  223,
 221,  219,  217,  214,  212,  209,  207,  204,  201,  198,
 196,  193,  190,  187,  184,  181,  177,  174,  171,  167,
 164,  161,  157,  154,  150,  146,  143,  139,  135,  131,
 128,  124,  120,  116,  112,  108,  104,  100,   95,   91,
  87,   83,   79,   74,   70,   66,   61,   57,   53,   48,
  44,   40,   35,   31,   26,   22,   17,   13,    8,    4,
   0,   -4,   -8,  -13,  -17,  -22,  -26,  -31,  -35,  -40,
 -44,  -48,  -53,  -57,  -61,  -66,  -70,  -74,  -79,  -83,
 -87,  -91,  -95, -100, -104, -108, -112, -116, -120, -124,
-127, -131, -135, -139, -143, -146, -150, -154, -157, -161,
-164, -167, -171, -174, -177, -181, -184, -187, -190, -193,
-196, -198, -201, -204, -207, -209, -212, -214, -217, -219,
-221, -223, -226, -228, -230, -232, -233, -235, -237, -238,
-240, -242, -243, -244, -246, -247, -248, -249, -250, -251,
-252, -252, -253, -254, -254, -255, -255, -255, -255, -255,
-255, -255, -255, -255, -255, -255, -254, -254, -253, -252,
-252, -251, -250, -249, -248, -247, -246, -244, -243, -242,
-240, -238, -237, -235, -233, -232, -230, -228, -226, -223,
-221, -219, -217, -214, -212, -209, -207, -204, -201, -198,
-196, -193, -190, -187, -184, -181, -177, -174, -171, -167,
-164, -161, -157, -154, -150, -146, -143, -139, -135, -131,
-128, -124, -120, -116, -112, -108, -104, -100,  -95,  -91,
 -87,  -83,  -79,  -74,  -70,  -66,  -61,  -57,  -53,  -48,
 -44,  -40,  -35,  -31,  -26,  -22,  -17,  -13,   -8,   -4 };

const s16 COS[360] = {  256,  255,  255,  255,  255,  255,  254,  254,  253,  252,
 252,  251,  250,  249,  248,  247,  246,  244,  243,  242,
 240,  238,  237,  235,  233,  232,  230,  228,  226,  223,
 221,  219,  217,  214,  212,  209,  207,  204,  201,  198,
 196,  193,  190,  187,  184,  181,  177,  174,  171,  167,
 164,  161,  157,  154,  150,  146,  143,  139,  135,  131,
 128,  124,  120,  116,  112,  108,  104,  100,   95,   91,
  87,   83,   79,   74,   70,   66,   61,   57,   53,   48,
  44,   40,   35,   31,   26,   22,   17,   13,    8,    4,
   0,   -4,   -8,  -13,  -17,  -22,  -26,  -31,  -35,  -40,
 -44,  -48,  -53,  -57,  -61,  -66,  -70,  -74,  -79,  -83,
 -87,  -91,  -95, -100, -104, -108, -112, -116, -120, -124,
-127, -131, -135, -139, -143, -146, -150, -154, -157, -161,
-164, -167, -171, -174, -177, -181, -184, -187, -190, -193,
-196, -198, -201, -204, -207, -209, -212, -214, -217, -219,
-221, -223, -226, -228, -230, -232, -233, -235, -237, -238,
-240, -242, -243, -244, -246, -247, -248, -249, -250, -251,
-252, -252, -253, -254, -254, -255, -255, -255, -255, -255,
-255, -255, -255, -255, -255, -255, -254, -254, -253, -252,
-252, -251, -250, -249, -248, -247, -246, -244, -243, -242,
-240, -238, -237, -235, -233, -232, -230, -228, -226, -223,
-221, -219, -217, -214, -212, -209, -207, -204, -201, -198,
-196, -193, -190, -187, -184, -181, -177, -174, -171, -167,
-164, -161, -157, -154, -150, -146, -143, -139, -135, -131,
-128, -124, -120, -116, -112, -108, -104, -100,  -95,  -91,
 -87,  -83,  -79,  -74,  -70,  -66,  -61,  -57,  -53,  -48,
 -44,  -40,  -35,  -31,  -26,  -22,  -17,  -13,   -8,   -4,
   0,    4,    8,   13,   17,   22,   26,   31,   35,   40,
  44,   48,   53,   57,   61,   66,   70,   74,   79,   83,
  87,   91,   95,  100,  104,  108,  112,  116,  120,  124,
 127,  131,  135,  139,  143,  146,  150,  154,  157,  161,
 164,  167,  171,  174,  177,  181,  184,  187,  190,  193,
 196,  198,  201,  204,  207,  209,  212,  214,  217,  219,
 221,  223,  226,  228,  230,  232,  233,  235,  237,  238,
 240,  242,  243,  244,  246,  247,  248,  249,  250,  251,
 252,  252,  253,  254,  254,  255,  255,  255,  255,  255 };

unsigned char noisemap[64*64] = {0};
int dblbuf = -1;

int ypos[128] = {0};

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

INLINE void draw_horizontal_line(int y, int width, int color) {
	if (y < 0) return;
	if (y > 160) return;
	if (width >= 128) width = 128;
	if (width <= 0) return;
	if (dblbuf == -1) {
		memset(SCREEN+ypos[y]+width, color, (128-width)*sizeof(u16));
	} else {
		memset(SCREEN2+ypos[y]+width, color, (128-width)*sizeof(u16));
	}
}

void voxel_render(Point p, int horizon, 
				  int distance, int screen_width) {

	// draw from back to front (high z coordinate to low z coordinate)
	for (int z = distance; z > 1; z-=8) {
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
		for (int i = 0; i < 128; i+=1) {
			u8 vox = noisemap[(((s8)pleft.x&63)<<6)+((s8)pleft.y&63)];

			int height_on_screen = vox;
			height_on_screen=height_on_screen<<4;
			height_on_screen = fx2float(fxdiv(int2fx(height_on_screen),float2fx(z)));
			height_on_screen += horizon;
			
			int c = 0xFFFF-(height_on_screen<<4);
			draw_horizontal_line(i, height_on_screen, c);
			pleft.x += dx;
		}
	}
}

int main()
{
	generate_noisemap();

	for(int i = 0; i < 128; i++) {
		ypos[i] = i*160;
	}

	Point p;
	p.x = 0;
	p.y = 0;
	int t = 0;

		u32 cx = 120;
		u32 cy = 120;

		u32 zoomx = 140;
		u32 zoomy = 160;

		u32 center_y = (cy * zoomy) >> 8;
		u32 center_x = (cx * zoomx) >> 8;
		u32 angle = 90;
		REG_BG2X = (REG_BG2HOFS - center_y * SIN[angle] - center_x * COS[angle]);
		REG_BG2Y = (REG_BG2VOFS - center_y * COS[angle] + center_x * SIN[angle]);
		REG_BG2PA = (COS[angle] * zoomx) >> 8;
		REG_BG2PB = (SIN[angle] * zoomx) >> 8;
		REG_BG2PC = (-SIN[angle] * zoomy) >> 8;
		REG_BG2PD = (COS[angle] * zoomy) >> 8;

	while(1){


		dblbuf = -dblbuf;
		if (dblbuf == 1) {
			DISPCONTROL = 0x0405;
		} else {
			DISPCONTROL = 0x0415;
		}




		p.x=t*0.4;
		voxel_render(p, 10, 80, 160);
		t++;
	}
	return 0;
}