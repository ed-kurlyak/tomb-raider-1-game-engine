#ifndef _3D_GEN_
#define _3D_GEN_

//#include "global/types.h"

//#include <stdint.h>

#include "types.h"

void phd_LookAt(int32_t xsrc, int32_t ysrc, int32_t zsrc, int32_t xtar,
				int32_t ytar, int32_t ztar, int16_t roll);
void phd_GetVectorAngles(int32_t x, int32_t y, int32_t z, int16_t *dest);

#endif