#pragma once

#include ".\scitech\INCLUDE\MGRAPH.H"
//#pragma comment (lib, "./scitech/LIB/WIN32/VC/MGLLT.LIB")
#pragma comment(lib, ".\\scitech\\LIB\\WIN32\\VC\\MGLLT.LIB")
#pragma comment(lib, "winmm.lib")

#pragma comment(lib, "legacy_stdio_definitions.lib")


extern HINSTANCE g_hInst;
extern HWND g_hWnd;

extern MGLDC* windc;
extern MGLDC* dibdc;

extern char * phd_winptr;

struct pal_rgb
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};

extern pal_rgb palette[256];

void Create_Normal_Palette();
void Create_BackBuffer();
void Clear_BackBuffer();
void Present_BackBuffer();
void Delete_BackBuffer();
