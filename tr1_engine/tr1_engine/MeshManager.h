//======================================================================================
//	Ed Kurlyak 2023 Software Rendering
//======================================================================================

#ifndef _MESHMANAGER_
#define _MESHMANAGER_

#include <windows.h>
#include <math.h>
#include <stdio.h>

#include <vector>

//#include <vfw.h> //для DrawDibDraw
//#pragma comment(lib, "Vfw32.lib") //для DrawDibDraw

#include "3d_gen.h"
#include "phd_math.h"
#include "matrix.h"
#include "vars.h"

#include "backbuffer.h"


#define PI 3.14159265358979f

#define SWAP(a,b,t) { t = a; a = b; b = t; }

#define BITS_PER_PIXEL	32

struct vector3
{
	float x,y,z;
	float tu,tv;

	
	vector3() {};
	vector3(float ix, float iy, float iz) : x(ix), y(iy), z(iz) {};
	
	
	/*
	int x, y, z;
	int tu, tv;


	vector3() {};
	vector3(int ix, int iy, int iz) : x(ix), y(iy), z(iz) {};
	*/
	vector3 operator - (const vector3 &VecIn)
	{
		vector3 VecOut;
		
		VecOut.x = x - VecIn.x;
		VecOut.y = y - VecIn.y;
		VecOut.z = z - VecIn.z;
		
		return VecOut;
	}

	vector3 &operator = (const vector3 &VecIn)
	{
		x = VecIn.x;
		y = VecIn.y;
		z = VecIn.z;

		tu = VecIn.tu;
		tv = VecIn.tv;

		return *this;
	}


	

	vector3 & operator += (const vector3 &VecIn)
	{
		x = x + VecIn.x;
		y = y + VecIn.y;
		z = z + VecIn.z;

		return *this;
	}
};



struct polygon
{
	polygon(){};
	~polygon(){};
	
	int  TexID;
	vector3 Vertex[3];
};

struct list 
{
	list()
	{
		PolygonCurr = 0; PolygonCount = 0; PolyList = NULL;
	};

	~list()
	{
		if(PolyList != NULL)
		{
			free(PolyList);
			PolyList = NULL;
		}
		PolygonCurr = 0;
		PolygonCount = 0;
	}

	void Reset()
	{
		if(PolyList != NULL)
		{
			free(PolyList);
			PolyList = NULL;
		}
		PolygonCurr = 0;
		PolygonCount = 0;
	}

	int Is_Empty_List ();
	polygon *Get_From_List();
	void Add_To_List(polygon *p);
	
	polygon* PolyList;
	int PolygonCount;
	int PolygonCurr;
};

struct matrix4x4
{
	union
	{
		int  Mat[4][4];
		struct
		{
			int r1c1; int r1c2; int r1c3; int r1c4;
			int r2c1; int r2c2; int r2c3; int r2c4;
			int r3c1; int r3c2; int r3c3; int r3c4;
			int r4c1; int r4c2; int r4c3; int r4c4;
		};
	};

	matrix4x4 (){};
	~matrix4x4 (){};

	matrix4x4 (int ir1c1, int ir1c2, int ir1c3, int ir1c4,
					int ir2c1, int ir2c2, int ir2c3, int ir2c4,
					int ir3c1, int ir3c2, int ir3c3, int ir3c4,
					int ir4c1, int ir4c2, int ir4c3, int ir4c4)
	{
		r1c1 = ir1c1; r1c2 = ir1c2; r1c3 = ir1c3; r1c4 = ir1c4;
		r2c1 = ir2c1; r2c2 = ir2c2; r2c3 = ir2c3; r2c4 = ir2c4;
		r3c1 = ir3c1; r3c2 = ir3c2; r3c3 = ir3c3; r3c4 = ir3c4;
		r4c1 = ir4c1; r4c2 = ir4c2; r4c3 = ir4c3; r4c4 = ir4c4;
	};
};


//-----------------

#define MAX_SCANLINES	1200 // maximum height of screen


//#define ABS(x) (((x)<0) ? (-(x)):(x))

#define MAX_POLYGONS 4000 // maximum number of polygons

class CMeshManager
{
public:

	CMeshManager();
	~CMeshManager();

	void Init_MeshManager(HWND hWnd) ;
	void Update_MeshManager();
	void Draw_MeshManager();

private:
	vector3 Vec3_Mat4x4_Mul(vector3 &VecIn, PHD_MATRIX MatIn);
	float Vec3_Dot(vector3 &VecIn1, vector3 &VecIn2);
	vector3 Vec3_Cross(vector3 &VecIn1, vector3 &VecIn2);
	vector3 Vec3_Normalize(vector3 &VecIn);
	vector3 Vec3_Scale(vector3 &VecIn, float ValIn);
	vector3 Vec3_Add(vector3 &VecIn1, vector3 &VecIn2);
	//matrix4x4 Matrix_Rotation_Axis(vector3 &VecIn, float Angle);
	void Get_View_Matrix();
	vector3 Calc_Edge(vector3 &VecIn1, vector3 &VecIn2);

	int Clip_Vertices_ZNear(int Num, vector3 *Source);

	int Get_TextureID(char * szFilename);

	int Load_BMP(char *szFilename, int Tile);

	int Clip_Vertices_Screen(int Num, vector3 *Source);
	int Visible_ZClip(vector3 &vn1, vector3 &vn2, vector3 &vn3);

	void Timer_Start();
	float Get_Elapsed_Time();

	void SortPolyList(int number, int buffer[][2]);
	void do_quickysorty(int left, int right, int buffer[][2]);
	void PrintPolyList(void *ptr);

	int32_t surfacenum;

	int16_t *info3dptr; // current pointer to info section
	int32_t *sort3dptr; // current pointer to Sort section

	int32_t sort3d_buffer[MAX_POLYGONS][2];   // buffer for sort info
	int16_t info3d_buffer[MAX_POLYGONS * 30]; // buffer for print info

	unsigned char **m_pLevelTile;
	UCHAR * m_Res;

    ULONG m_ViewWidth;
	ULONG m_ViewHeight;
	HWND m_hWnd;

	vector3 m_VecCamRight;
	vector3 m_VecCamUp;
	vector3 m_VecCamLook;
	vector3 m_VecCamPos;

	float **m_ZBuff;
	
	list m_PolygonsSource;
	list m_TransformedPoly;

	UINT m_TextureWidth;
	UINT m_TextureHeight;

	//начальная позиция для левой стороны треугольника
	float m_ul, m_vl, m_zl, m_xl;
	//приращение координат для левой стороны треугольника
	float  m_dudyl, m_dvdyl, m_dzdyl, m_dxdyl;

	//начальная позиция для правой стороны треугольника
	float m_ur, m_vr, m_zr, m_xr;
	//приращение координат для правой стороны треугольника
	float m_dudyr, m_dvdyr, m_dzdyr, m_dxdyr;

	//Добавим задний буфер
	//int m_BackLpitch;
	//BITMAPINFOHEADER m_Bih;
	//HDRAWDIB m_hDD;
	//LPBYTE m_Data;
	//LPBYTE m_DataTemp;

	//timer
	float m_ElapsedTime;
	float m_TimeScale;
	__int64 m_StartTime;
	__int64 m_LastTime;
	__int64 m_PerfFreq;

	int m_ZNear;

};

#endif