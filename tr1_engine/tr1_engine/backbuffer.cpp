#include "backbuffer.h"

MGLDC* windc = NULL;
MGLDC* dibdc = NULL;

pal_rgb palette[256] = {0};

char * phd_winptr = NULL;

void Create_Normal_Palette()
{
	palette_t	pal[256];

	pal[0].red = 0;
	pal[0].green = 0;
	pal[0].blue = 0;

	for (int i = 1; i < 256; i++)
	{
		pal[i].red = palette[i].b;
		pal[i].green = palette[i].g;
		pal[i].blue = palette[i].r;
	}

	//цвет для очистки экрана
	pal[0].red = 0;
	pal[0].green = (unsigned char)(255 * 0.125f);
	pal[0].blue = (unsigned char)(255 * 0.3f);

	MGL_setPalette(dibdc, pal, 256, 0);
	MGL_realizePalette(dibdc, 256, 0, false);

}

void Create_BackBuffer()
{
	RECT Rc;
	GetClientRect(g_hWnd, &Rc);

	pixel_format_t	pf;

	MGL_setAppInstance(g_hInst);

	MGL_registerDriver(MGL_PACKED8NAME, PACKED8_driver);

	//для оконного режима
	MGL_initWindowed("");

	if (!MGL_changeDisplayMode(grWINDOWED))
		MGL_fatalError("Unable to use window mode!");

	if ((windc = MGL_createWindowedDC(g_hWnd)) == NULL)
		MGL_fatalError("Unable to create Windowed DC!");

	MGL_getPixelFormat(windc, &pf);

	if ((dibdc = MGL_createMemoryDC(Rc.right, Rc.bottom, 8, &pf)) == NULL)
		MGL_fatalError("Unable to create Memory DC!");

	Create_Normal_Palette();

}


void Clear_BackBuffer()
{
	RECT Rc;
	GetClientRect(g_hWnd, &Rc);


	MGL_beginDirectAccess();

	//char * phd_winptr_my = NULL;
	//phd_winptr_my = (char*)dibdc->surface;

	phd_winptr = (char*)dibdc->surface;

	//очищаем m_BackBuffer (экран)
	for (int x = 0; x < Rc.right; x++)
	{
		for (int y = 0; y < Rc.bottom; y++)
		{
			int Index = y * Rc.right + x;

			//phd_winptr_my[Index + 0] = 0;
			phd_winptr[Index + 0] = 0;
		}
	}

}

void Present_BackBuffer()
{
	RECT Rc;
	GetClientRect(g_hWnd, &Rc);

	MGL_endDirectAccess();

	//MGL present back buffer
	HDC hdcScreen = GetDC(g_hWnd);
	MGL_setWinDC(windc, hdcScreen);

	MGL_bitBltCoord(windc, dibdc, 0, 0, Rc.right, Rc.bottom, 0, 0, MGL_REPLACE_MODE);

	ReleaseDC(g_hWnd, hdcScreen);

}

void Delete_BackBuffer()
{
	if (windc)
		MGL_destroyDC(windc);
	if (dibdc)
		MGL_destroyDC(dibdc);

	windc = dibdc = NULL;
}

