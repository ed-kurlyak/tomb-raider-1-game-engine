
#ifndef _TYPES_
#define _TYPES_

//#include <stdint.h>
#define uint32_t unsigned int
#define int32_t int
#define uint16_t unsigned short int
#define int16_t short int
#define uint8_t unsigned char
#define int8_t char


#define MAX_MATRICES 40
#define MAX_NESTED_MATRICES 32

#define PHD_ONE 0x10000
#define PHD_DEGREE (PHD_ONE / 360) // = 182
#define PHD_360 (PHD_ONE)		   // = 65536 = 0x10000
#define PHD_180 (PHD_ONE / 2)	  // = 32768 = 0x8000
#define PHD_90 (PHD_ONE / 4)	   // = 16384 = 0x4000
#define PHD_45 (PHD_ONE / 8)	   // = 8192 = 0x2000
#define PHD_135 (PHD_45 * 3)	   // = 24576 = 0x6000


typedef struct PHD_3DPOS
{
	int32_t x;
	int32_t y;
	int32_t z;
	int16_t x_rot;
	int16_t y_rot;
	int16_t z_rot;
} PHD_3DPOS;


typedef struct PHD_MATRIX
{
	int _00;
	int _01;
	int _02;
	int _03;
	int _10;
	int _11;
	int _12;
	int _13;
	int _20;
	int _21;
	int _22;
	int _23;
} PHD_MATRIX;


typedef int16_t PHD_ANGLE;

#define SQUARE(A) ((A) * (A))

#define W2V_SHIFT 14
#define W2V_SCALE (1 << W2V_SHIFT)


#endif