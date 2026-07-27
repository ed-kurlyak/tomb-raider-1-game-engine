#include "3d_gen.h"

#include "matrix.h"
#include "phd_math.h"
//#include "vars.h"
//#include "winmain.h"
//#include "screen.h"
//#include "config.h"
//#include "game/output.h"
//#include "game/screen.h"
//#include "global/vars.h"

#include <math.h>

void phd_LookAt(int32_t xsrc, int32_t ysrc, int32_t zsrc, int32_t xtar,
				int32_t ytar, int32_t ztar, int16_t roll)
{
	PHD_ANGLE angles[2];
	phd_GetVectorAngles(xtar - xsrc, ytar - ysrc, ztar - zsrc, angles);

	PHD_3DPOS viewer;
	viewer.x = xsrc;
	viewer.y = ysrc;
	viewer.z = zsrc;
	viewer.x_rot = angles[1];
	viewer.y_rot = angles[0];
	viewer.z_rot = roll;
	phd_GenerateW2V(&viewer);
}

void phd_GetVectorAngles(int32_t x, int32_t y, int32_t z, int16_t *dest)
{
	dest[0] = phd_atan(z, x);

	while ((int16_t)x != x || (int16_t)y != y || (int16_t)z != z)
	{
		x >>= 2;
		y >>= 2;
		z >>= 2;
	}

	PHD_ANGLE pitch = phd_atan(phd_sqrt(SQUARE(x) + SQUARE(z)), y);

	if ((y > 0 && pitch > 0) || (y < 0 && pitch < 0))
	{
		pitch = -pitch;
	}

	dest[1] = pitch;
}

