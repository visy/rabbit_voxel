#include "Intellisense.h"
#include <stdint.h>
#include <stddef.h>

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


//# Stuff you may not have yet.
typedef unsigned int uint;
#define BIT_MASK(len)       ( (1<<(len))-1 )
static inline u32 quad8(u8 x)   {   x |= x<<8; return x | x<<16;    }


//# Declarations and inlines.

void tonccpy(void *dst, const void *src, uint size);

void __toncset(void *dst, u32 fill, uint size);
static inline void toncset(void *dst, u8 src, uint size);
static inline void toncset16(void *dst, u16 src, uint size);
static inline void toncset32(void *dst, u32 src, uint size);


//! VRAM-safe memset, byte version. Size in bytes.
static inline void toncset(void *dst, u8 src, uint size)
{   __toncset(dst, quad8(src), size);               }

//! VRAM-safe memset, halfword version. Size in hwords.
static inline void toncset16(void *dst, u16 src, uint size)
{   __toncset(dst, src|src<<16, size*2);            }

//! VRAM-safe memset, word version. Size in words.
static inline void toncset32(void *dst, u32 src, uint size)
{   __toncset(dst, src, size*4);                    }

//# tonccpy.c

//! VRAM-safe cpy.
/*! This version mimics memcpy in functionality, with
    the benefit of working for VRAM as well. It is also
    slightly faster than the original memcpy, but faster
    implementations can be made.
    \param dst  Destination pointer.
    \param src  Source pointer.
    \param size Fill-length in bytes.
    \note   The pointers and size need not be word-aligned.
*/
void tonccpy(void *dst, const void *src, uint size)
{
    if(size==0 || dst==NULL || src==NULL)
        return;

    uint count;
    u16 *dst16;     // hword destination
    u8  *src8;      // byte source

    // Ideal case: copy by 4x words. Leaves tail for later.
    if( ((u32)src|(u32)dst)%4==0 && size>=4)
    {
        u32 *src32= (u32*)src, *dst32= (u32*)dst;

        count= size/4;
        uint tmp= count&3;
        count /= 4;

        // Duff, bitch!
        switch(tmp) {
            do {    *dst32++ = *src32++;
        case 3:     *dst32++ = *src32++;
        case 2:     *dst32++ = *src32++;
        case 1:     *dst32++ = *src32++;
        case 0:     ; } while(count--);
        }

        // Check for tail
        size &= 3;
        if(size == 0)
            return;

        src8= (u8*)src32;
        dst16= (u16*)dst32;
    }
    else        // Unaligned.
    {
        uint dstOfs= (u32)dst&1;
        src8= (u8*)src;
        dst16= (u16*)(dst-dstOfs);

        // Head: 1 byte.
        if(dstOfs != 0)
        {
            *dst16= (*dst16 & 0xFF) | *src8++<<8;
            dst16++;
            if(--size==0)
                return;
        }
    }

    // Unaligned main: copy by 2x byte.
    count= size/2;
    while(count--)
    {
        *dst16++ = src8[0] | src8[1]<<8;
        src8 += 2;
    }

    // Tail: 1 byte.
    if(size&1)
        *dst16= (*dst16 &~ 0xFF) | *src8;
}

//# toncset.c

//! VRAM-safe memset, internal routine.
/*! This version mimics memset in functionality, with
    the benefit of working for VRAM as well. It is also
    slightly faster than the original memset.
    \param dst  Destination pointer.
    \param fill Word to fill with.
    \param size Fill-length in bytes.
    \note   The \a dst pointer and \a size need not be
        word-aligned. In the case of unaligned fills, \a fill
        will be masked off to match the situation.
*/
void __toncset(void *dst, u32 fill, uint size)
{
    if(size==0 || dst==NULL)
        return;

    uint left= (u32)dst&3;
    u32 *dst32= (u32*)(dst-left);
    u32 count, mask;

    // Unaligned head.
    if(left != 0)
    {
        // Adjust for very small stint.
        if(left+size<4)
        {
            mask= BIT_MASK(size*8)<<(left*8);  
            *dst32= (*dst32 &~ mask) | (fill & mask);
            return;
        }

        mask= BIT_MASK(left*8);
        *dst32= (*dst32 & mask) | (fill&~mask);
        dst32++;
        size -= 4-left;
    }

    // Main stint.
    count= size/4;
    uint tmp= count&3;
    count /= 4;

    switch(tmp) {
        do {    *dst32++ = fill;
    case 3:     *dst32++ = fill;
    case 2:     *dst32++ = fill;
    case 1:     *dst32++ = fill;
    case 0:     ; } while(count--);
    }

    // Tail
    size &= 3;
    if(size)
    {
        mask= BIT_MASK(size*8);
        *dst32= (*dst32 &~ mask) | (fill & mask);
    }
}

unsigned char lfsrnoise(int x, int y, int z) {
	int n = x + y + z;
	n = (n >> 16) ^ n;
	n = (n >> 8) ^ n;
	return (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
}


void generate_noisemap() {
	for (int y = 0; y < 64; y++) {
		for (int x = 0; x < 64; x++) {
			noisemap[y*64+x] = 256-lfsrnoise(x, y, 0)%128;
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

INLINE void draw_horizontal_line(int y, int width, int prev, int color) {
	if (y < 0) return;
	if (y > 160) return;
    if (width > 128) width = 128;
	if (dblbuf == -1) {
		toncset16(SCREEN+ypos[y]+(128-width), color, width-prev);
	} else {
		toncset16(SCREEN2+ypos[y]+(128-width), color, width-prev);
	}
}

int voxheights[128] = {0};

void voxel_render(Point p, int horizon, 
				  int distance, int screen_width) {

    // clear voxheights array
    for (int i = 0; i < 128; i++) {
        voxheights[i] = 0;
    }

	for (int z = 4; z < 16; z+=2) {
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
		for (int i = 0; i < 128; i+=1) {
			u8 vox = noisemap[(((s8)pleft.x&63)<<6)+((s8)pleft.y&63)];

			int height_on_screen = vox;
			height_on_screen = height_on_screen/(16-z);
			height_on_screen += horizon;
			
            if (height_on_screen > voxheights[i]) {
                int c = 0xFFFF-(height_on_screen<<4);
                draw_horizontal_line(i, height_on_screen, voxheights[i], c);
                voxheights[i] = height_on_screen;
            }

			pleft.x += dx;
		}
	}
}

#define REG_VCOUNT *(vu16*)0x04000006

void vid_vsync()
{
    while(REG_VCOUNT >= 160);   // wait till VDraw
    while(REG_VCOUNT < 160);    // wait till VBlank
}

void rotscale(int sx, int sy, u16 alpha)
{
	int ss= SIN[alpha%360], cc= COS[alpha%360];

    REG_BG2PA = cc*sx>>12;
    REG_BG2PB = -ss*sx>>12;
    REG_BG2PC = ss*sy>>12;
    REG_BG2PD = cc*sy>>12;
}

void scroll(int x, int y) {
    REG_BG2X = x;
    REG_BG2Y = y;
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

    rotscale(-0xFF*12,0x8a8,90);

	while(1) {

        vid_vsync();
        //scroll(SIN[t*4]<<2,COS[t*4]<<2);

		dblbuf = -dblbuf;
		if (dblbuf == 1) {
			DISPCONTROL = 0x0405;
            memset(SCREEN2, 0x0, 160*128*2);
		} else {
			DISPCONTROL = 0x0415;
            memset(SCREEN, 0x0, 160*128*2);
		}

		p.x=t*0.6;
		voxel_render(p, 10, 40, 128);
		t++;
	}
	return 0;
}