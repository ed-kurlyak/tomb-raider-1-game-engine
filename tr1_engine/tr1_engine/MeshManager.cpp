//======================================================================================
//	Ed Kurlyak 2023 Software Rendering
//======================================================================================

#include "MeshManager.h"
#include "phd_math.h"
#include <malloc.h>


#pragma pack(push,foo,1)

typedef struct uvgp_info {
	short int	xs;
	short int	ys;
	short int	shade;
	float	ooz;
	float	uoz;
	float	voz;
} UVGP_INFO;



typedef struct xbuf_persp_fp {
			int	Xleft;
			int	Gleft;
			float	UOZleft;
			float	VOZleft;
			float 	OOZleft;
			int	Xright;
			int	Gright;
			float	UOZright;
			float	VOZright;
			float 	OOZright;
}XBUF_PERSP_FP;

#pragma pack(pop,foo)

#define MAX_SCANLINES 1200

XBUF_PERSP_FP 	xbuffer[MAX_SCANLINES] = {0};         	// Edge buffer for Polygon Draw

int xgen_ymin;
int xgen_ymax;

int phd_scrwidth;

CMeshManager::CMeshManager()
{
}

CMeshManager::~CMeshManager()
{
	Delete_BackBuffer();
	
	delete [] m_pLevelTile[0];
	m_pLevelTile[0] = NULL;

	delete [] m_pLevelTile[1];
	m_pLevelTile[1] = NULL;

	delete [] m_pLevelTile[2];
	m_pLevelTile[2] = NULL;

	delete [] m_pLevelTile;
	m_pLevelTile = NULL;
}

vector3 CMeshManager::Vec3_Mat4x4_Mul(vector3 &VecIn, PHD_MATRIX MatIn)
{
	    vector3 VecOut;

		VecOut.x = VecIn.x * MatIn._00 + VecIn.y * MatIn._01 + VecIn.z * MatIn._02 + MatIn._03;

		VecOut.y = VecIn.x * MatIn._10 + VecIn.y * MatIn._11 + VecIn.z * MatIn._12 + MatIn._13;

		VecOut.z = VecIn.x * MatIn._20 + VecIn.y * MatIn._21 + VecIn.z * MatIn._22 + MatIn._23;

		
	VecOut.tu = VecIn.tu;
	VecOut.tv = VecIn.tv;

	return VecOut;
}

int list::Is_Empty_List ()
{
		if(PolygonCount == 0)
			return true;

		return false;
}

void list::Add_To_List(polygon *p)
{
	if ( PolygonCount == 0)
	{	
		PolygonCount++;
		PolyList=(polygon*)malloc(PolygonCount*sizeof(polygon));
	}
	else
	{
		PolygonCount++;
		PolyList=(polygon*)realloc(PolyList, PolygonCount*sizeof(polygon));
	}

	for(int i=0;i<3;i++)
		PolyList[PolygonCount-1].Vertex[i] = p->Vertex[i];

	PolyList[PolygonCount-1].TexID = p->TexID;
}

polygon* list::Get_From_List ()
{
	polygon *Plg;
	Plg = &PolyList[PolygonCurr];
	PolygonCurr++;
	if(PolygonCurr > PolygonCount)
		return NULL;

	return Plg;
}

/* Mask for deciding whether to single or double pixel */
#define SINGLE_MASK 0xffff8000

int xgen_xguvpersp_fp(short int* iptr)
{
	UVGP_INFO* ptr1, * ptr2;
	XBUF_PERSP_FP* xptr;
	int g, g_add;
	int x, x_add;
	float scale;
	float uoz, uoz_add;
	float voz, voz_add;
	float ooz, ooz_add;
	int y1, y2, ydif;
	int min_y, max_y;
	int numcoords;

	numcoords = (int) * (iptr++); // Get Number Coords
	ptr2 = (UVGP_INFO*)iptr;
	ptr1 = ptr2 + numcoords - 1;
	min_y = max_y = ptr1->ys;
	for (; numcoords > 0;
		numcoords--, ptr1 = ptr2, ptr2++) // For All Side in Clockwise manner
	{
		y1 = ptr1->ys;
		y2 = ptr2->ys;
		if (y1 < y2) // We are on RHS of Polygon
		{
			if (y1 < min_y)
				min_y = y1;

			ydif = y2 - y1;

			x = ptr1->xs;
			x_add = ((ptr2->xs - x) << 16) / ydif;
			x = (x << 16) + 0x0ffff;

			g = ptr1->shade << 8; // Get Lerps and Deltas
			scale = 1.0f / ydif;
			g_add = ((ptr2->shade << 8) - g) / ydif;
			uoz = ptr1->uoz;
			uoz_add = (ptr2->uoz - uoz) * scale;
			voz = ptr1->voz;
			voz_add = (ptr2->voz - voz) * scale;
			ooz = ptr1->ooz;
			ooz_add = (ptr2->ooz - ooz) * scale;

			xptr = ((XBUF_PERSP_FP*)xbuffer) + y1;
			for (; y1 < y2; y1++, xptr++)
			{
				x += x_add;
				g += g_add;
				uoz += uoz_add;
				voz += voz_add;
				ooz += ooz_add;
				xptr->Xright = x;
				xptr->Gright = g;
				xptr->UOZright = uoz;
				xptr->VOZright = voz;
				xptr->OOZright = ooz;
			}
		}
		else if (y2 < y1) // We are on LHS of Polygon
		{
			if (y1 > max_y)
				max_y = y1;

			ydif = y1 - y2;

			x = ptr2->xs;
			x_add = ((ptr1->xs - x) << 16) / ydif;
			x = (x << 16) + 0x0001;

			g = ptr2->shade << 8; // Get Lerps and Deltas
			scale = 1.0f / ydif;
			g_add = ((ptr1->shade << 8) - g) / ydif;
			uoz = ptr2->uoz;
			uoz_add = (ptr1->uoz - uoz) * scale;
			voz = ptr2->voz;
			voz_add = (ptr1->voz - voz) * scale;
			ooz = ptr2->ooz;
			ooz_add = (ptr1->ooz - ooz) * scale;

			xptr = ((XBUF_PERSP_FP*)xbuffer) + y2;
			for (; y2 < y1; y2++, xptr++)
			{
				x += x_add;
				g += g_add;
				uoz += uoz_add;
				voz += voz_add;
				ooz += ooz_add;
				xptr->Xleft = x;
				xptr->Gleft = g;
				xptr->UOZleft = uoz;
				xptr->VOZleft = voz;
				xptr->OOZleft = ooz;
			}
		}
	}
	if (min_y == max_y)
		return (0);
	xgen_ymin = min_y; // Insert Global Y bounds..
	xgen_ymax = max_y;
	return (1);
}


/******************************************************************************
 *						   TextureMapper
 * 		 		Perspective Correction every 32 Pixels
 *****************************************************************************/
void	gtmap_persp32_fp( int ymin, int ymax, unsigned char *tptr )
{
	/* This version does 32 pixels at a time linearly, and adaptively double pixels for each 32 pixel span.
		Should only be used in 640x480 though */
	XBUF_PERSP_FP	*dataptr;
	unsigned char	*lineptr,*pixptr;
	float				ooz,uoz,voz,scale;
	float				oozadd32,uozadd32,vozadd32;
	int				start_g,add_g,add_g2;
	int				end_u,end_v;
	int				start_u,start_v;
	int				add_u,add_v;
	int				start_x,end_x;
	int				x;

	dataptr = ((XBUF_PERSP_FP *)xbuffer)+ymin;
	lineptr = (unsigned char*)phd_winptr + ( ymin * phd_scrwidth );

	for (ymax-=ymin; ymax>0; ymax--, dataptr++, lineptr+=phd_scrwidth)
	{
		start_x = dataptr->Xleft >> 16;
		end_x = dataptr->Xright >> 16;
		x = end_x - start_x;

		if (x <= 0)
			continue;
		
		if ( x>32 )
		{
			start_g = dataptr->Gleft;
			add_g = (dataptr->Gright-start_g) / x;
			add_g2 = add_g<<1;

			scale = 32.0f/x;
			uoz = dataptr->UOZleft;
			uozadd32 = (dataptr->UOZright-uoz) * scale;
			voz = dataptr->VOZleft;
			vozadd32 = (dataptr->VOZright-voz) * scale;
			ooz = dataptr->OOZleft;
			oozadd32 = (dataptr->OOZright-ooz) * scale;

			scale = 256.0f/ooz;
			end_u = (int)(uoz * scale);
			end_v = (int)(voz * scale);

			pixptr = lineptr + start_x;
		
			ooz += oozadd32;
			scale = 256.0f/ooz;
			
			for ( ; x>=32; x-=32)
			{
				start_u = end_u;
				uoz += uozadd32;
				end_u = (int)(uoz*scale);
				add_u = (end_u-start_u)>>5;

				start_v = end_v;
				voz += vozadd32;
				end_v = (int)(voz*scale);
				add_v = (end_v-start_v)>>5;

				// FP divide in parallel with the pixel drawing
				ooz += oozadd32;
				scale = 256.0f/ooz;

				if ((ABS(add_u) + ABS(add_v)) & SINGLE_MASK)
				{
					// Single pixel it
					*(pixptr) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;
					*(pixptr+1) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;
					*(pixptr+2) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;
					*(pixptr+3) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;

					*(pixptr+4) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;
					*(pixptr+5) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;
					*(pixptr+6) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;
					*(pixptr+7) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;

					*(pixptr+8) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;
					*(pixptr+9) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;
					*(pixptr+10) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;
					*(pixptr+11) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;

					*(pixptr+12) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;
					*(pixptr+13) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;
					*(pixptr+14) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;
					*(pixptr+15) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_g += add_g;
					start_u += add_u;
					start_v += add_v;

					*(pixptr+16) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;
					*(pixptr+17) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;
					*(pixptr+18) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;
					*(pixptr+19) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;

					*(pixptr+20) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;
					*(pixptr+21) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;
					*(pixptr+22) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;
					*(pixptr+23) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;

					*(pixptr+24) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;
					*(pixptr+25) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;
					*(pixptr+26) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;
					*(pixptr+27) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;

					*(pixptr+28) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;
					*(pixptr+29) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;
					*(pixptr+30) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g;
					*(pixptr+31) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_g += add_g;
				}
				else
				{
					// Double pixel it
					add_u <<= 1;
					add_v <<= 1;

					*(pixptr) = *(pixptr+1) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g2;
					*(pixptr+2) = *(pixptr+3) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g2;

					*(pixptr+4) = *(pixptr+5) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g2;
					*(pixptr+6) = *(pixptr+7) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g2;

					*(pixptr+8) = *(pixptr+9) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g2;
					*(pixptr+10) = *(pixptr+11) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g2;

					*(pixptr+12) = *(pixptr+13) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g2;
					*(pixptr+14) = *(pixptr+15) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g2;

					*(pixptr+16) = *(pixptr+17) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g2;
					*(pixptr+18) = *(pixptr+19) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g2;

					*(pixptr+20) = *(pixptr+21) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g2;
					*(pixptr+22) = *(pixptr+23) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g2;

					*(pixptr+24) = *(pixptr+25) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g2;
					*(pixptr+26) = *(pixptr+27) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g2;

					*(pixptr+28) = *(pixptr+29) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_u += add_u;
					start_v += add_v;
					start_g += add_g2;
					*(pixptr+30) = *(pixptr+31) = *(tptr + ((start_v>>16)<<8) + (start_u>>16) );
					start_g += add_g2;
				}

				pixptr += 32;
			}

			if (!x)
				continue;

			// Do the odd bit at the end
			scale = 256.0f/dataptr->OOZright;

			start_u = end_u;
			end_u = (int)(dataptr->UOZright*scale);
			add_u = (end_u-start_u)/x;

			start_v = end_v;
			end_v = (int)(dataptr->VOZright*scale);
			add_v = (end_v-start_v)/x;
		}
		else
		{
			/* Special case for spans of 32 pixels or less */
			start_g = dataptr->Gleft;
			add_g = (dataptr->Gright-start_g) / x;

			scale = 256.0f/dataptr->OOZleft;
			start_u = (int)(dataptr->UOZleft * scale);
			start_v = (int)(dataptr->VOZleft * scale);

			scale = 256.0f/dataptr->OOZright;
			end_u = (int)(dataptr->UOZright * scale);
			end_v = (int)(dataptr->VOZright * scale);

			add_u = (end_u-start_u)/x;
			add_v = (end_v-start_v)/x;

			pixptr = lineptr + start_x;
		}

		/* Do bits of span less than 32 pixels */
		if ((ABS(add_u) + ABS(add_v)) & SINGLE_MASK)
		{
			/* Single pixel it */
			for ( ; x>1; x-=2, pixptr+=2)
			{
				pixptr[0] = *(tptr + ((start_v>>16)<<8) + (start_u>>16));
				start_u += add_u;
				start_v += add_v;
				start_g += add_g;
				pixptr[1] = *(tptr + ((start_v>>16)<<8) + (start_u>>16));
				start_u += add_u;
				start_v += add_v;
				start_g += add_g;
			}
		}
		else
		{
			/* Double pixel it */
			add_u <<= 1;
			add_v <<= 1;
			add_g2 = add_g<<1;

			for ( ; x>1; x-=2, pixptr+=2)
			{
				pixptr[0] = pixptr[1] = *(tptr + ((start_v>>16)<<8) + (start_u>>16));
				start_u += add_u;
				start_v += add_v;
				start_g += add_g2;
			}
		}

		
		if (x)
			pixptr[0] = *(tptr + ((start_v>>16)<<8) + (start_u>>16));
		
	}
}



int CMeshManager::Get_TextureID(char * szFilename)
{

	if(!_strcmpi(szFilename, "texture1.bmp\n")) //floor texture
	{
		return 0;
	}
	else if(!_strcmpi(szFilename, "texture2.bmp\n")) //wall texture
	{
		return 1;
	}
	else if(!_strcmpi(szFilename, "texture3.bmp\n")) //ceiling texture
	{
		return 2;
	}
	
	return -1;
}

void CMeshManager::Init_MeshManager(HWND hWnd)
{
	ShowCursor(FALSE);

	m_hWnd = hWnd;

	RECT Rc;
	GetClientRect(hWnd, &Rc);

	m_ViewWidth = Rc.right;
	m_ViewHeight = Rc.bottom;

	phd_scrwidth = Rc.right;

	Timer_Start();

	//ширина текстуры 256, высота текстуры 256
	//256 * 256 = 65536, три цвета r,g,b
	//всего на сцене у нас 3 текстуры
	m_pLevelTile = new unsigned char *[3];

	//три изображения глубина цвета 8 бит
	//с одинаковой палитрой 256 цветов
	Load_BMP((char*)"texture1.bmp", 0);
	Load_BMP((char*)"texture2.bmp", 1);
	Load_BMP((char*)"texture3.bmp", 2);

	Create_BackBuffer();
	//Create_Normal_Palette();


	FILE *f;
	fopen_s(&f,"level.txt", "rt");

	char Buffer[1024];
	fgets(Buffer, 1024, f);

	int iSize;
	sscanf_s(Buffer,"%d",&iSize);

	polygon p;
	
	//while(!feof(f))
	int i = 0;
	while( i < iSize)
	{
		i++;

		vector3 VecPos[4];
		
		fgets(Buffer, 1024, f);
		sscanf_s(Buffer,"%f %f %f %f %f",&VecPos[0].x, &VecPos[0].y, &VecPos[0].z, &VecPos[0].tu, &VecPos[0].tv);
	
		fgets(Buffer, 1024, f);
		sscanf_s(Buffer,"%f %f %f %f %f",&VecPos[1].x, &VecPos[1].y, &VecPos[1].z, &VecPos[1].tu, &VecPos[1].tv);
	
		fgets(Buffer, 1024, f);
		sscanf_s(Buffer,"%f %f %f %f %f",&VecPos[2].x, &VecPos[2].y, &VecPos[2].z, &VecPos[2].tu, &VecPos[2].tv);
	
		fgets(Buffer, 1024, f);
		sscanf_s(Buffer,"%f %f %f %f %f",&VecPos[3].x, &VecPos[3].y, &VecPos[3].z, &VecPos[3].tu, &VecPos[3].tv);
						
		fgets(Buffer, 1024, f);
						
		char szTexFName[256];
		strcpy_s(szTexFName,256,Buffer);
		int tId = Get_TextureID(szTexFName);

		VecPos[0].x *= 102.4f;
		VecPos[0].y *= 102.4f;
		VecPos[0].z *= 102.4f;
		
		if(VecPos[0].tu == 0.0f)
			VecPos[0].tu = 10.0f;

		if(VecPos[0].tu == 65535.0f)
			VecPos[0].tu = 65535 - 10.0f;

		if(VecPos[0].tv == 0.0f)
			VecPos[0].tv = 10.0f;

		if(VecPos[0].tv == 65535.0f)
			VecPos[0].tv = 65535 - 10.0f;


		VecPos[1].x *= 102.4f;
		VecPos[1].y *= 102.4f;
		VecPos[1].z *= 102.4f;

		if(VecPos[1].tu == 0.0f)
			VecPos[1].tu = 10.0f;

		if(VecPos[1].tu == 65535.0f)
			VecPos[1].tu = 65535 - 10.0f;

		if(VecPos[1].tv == 0.0f)
			VecPos[1].tv = 10.0f;

		if(VecPos[1].tv == 65535.0f)
			VecPos[1].tv = 65535 - 10.0f;

		VecPos[2].x *= 102.4f;
		VecPos[2].y *= 102.4f;
		VecPos[2].z *= 102.4f;

		if(VecPos[2].tu == 0.0f)
			VecPos[2].tu = 10.0f;

		if(VecPos[2].tu == 65535.0f)
			VecPos[2].tu = 65535 - 10.0f;

		if(VecPos[2].tv == 0.0f)
			VecPos[2].tv = 10.0f;

		if(VecPos[2].tv == 65535.0f)
			VecPos[2].tv = 65535 - 10.0f;

		VecPos[3].x *= 102.4f;
		VecPos[3].y *= 102.4f;
		VecPos[3].z *= 102.4f;

		if(VecPos[3].tu == 0.0f)
			VecPos[3].tu = 10.0f;

		if(VecPos[3].tu == 65535.0f)
			VecPos[3].tu = 65535 - 10.0f;

		if(VecPos[3].tv == 0.0f)
			VecPos[3].tv = 10.0f;

		if(VecPos[3].tv == 65535.0f)
			VecPos[3].tv = 65535 - 10.0f;

		p.Vertex[0] = VecPos[0];
		p.Vertex[1] = VecPos[1];
		p.Vertex[2] = VecPos[2];

		p.TexID = tId;

		m_PolygonsSource.Add_To_List(&p);

		p.Vertex[0] = VecPos[0];
		p.Vertex[1] = VecPos[2];
		p.Vertex[2] = VecPos[3];

		p.TexID = tId;

		m_PolygonsSource.Add_To_List(&p);
	}

	fclose(f);

}

void CMeshManager::Get_View_Matrix()
{
	float Time = Get_Elapsed_Time();

	static int xtar = (int)(25 * 102.4f);
	static int ytar = (int)(5 * 102.4f);
	static int ztar = (int)(15 * 102.4f);

	static int xsrc = (int)(25 * 102.4f);
	static int ysrc = (int)(5 * 102.4f);
	static int zsrc = (int)(5 * 102.4f);

	static int y_rot = 0;
	
	if (GetAsyncKeyState('Q') & 0xFF00)
	{
		y_rot -= (int)(7500 * Time);
		if (y_rot < 0)
		{
			y_rot += 0x10000;
		}
	}

	if (GetAsyncKeyState('E') & 0xFF00)
	{
		y_rot += (int)(7500 * Time);
		if (y_rot >= 0x10000)
		{
			y_rot -= 0x10000;
		}
	}

	//всегда считаем позицию заново от угла
	int dist = 5000;

	xtar = xsrc + ((phd_sin(y_rot) * dist) >> W2V_SHIFT);
	ztar = zsrc + ((phd_cos(y_rot) * dist) >> W2V_SHIFT);
	
	//реакция на клавиши W,S,A,D
	float RatioMove = 5000;

	//в зависимости от угла поворота камеры
	//перемещаем саму камеру клавиша W,S,A,D

	if (GetAsyncKeyState('W') & 0xFF00)
	{
		xsrc += (int)(phd_sin(y_rot) * Time * RatioMove) >> W2V_SHIFT;
		zsrc += (int)(phd_cos(y_rot) * Time * RatioMove) >> W2V_SHIFT;
	}

	if (GetAsyncKeyState('S') & 0xFF00)
	{
		xsrc += (int)(-phd_sin(y_rot) * Time * RatioMove) >> W2V_SHIFT;
		zsrc += (int)(-phd_cos(y_rot) * Time * RatioMove) >> W2V_SHIFT;
	}

	if (GetAsyncKeyState('A') & 0xFF00)
	{
		xsrc += (int)(-phd_cos(y_rot) * Time * RatioMove) >> W2V_SHIFT;
		zsrc += (int)(phd_sin(y_rot) * Time * RatioMove) >> W2V_SHIFT;
	}

	if (GetAsyncKeyState('D') & 0xFF00)
	{
		xsrc += (int)(phd_cos(y_rot) * Time * RatioMove) >> W2V_SHIFT;
		zsrc += (int)(-phd_sin(y_rot) * Time * RatioMove) >> W2V_SHIFT;
	}

	phd_LookAt(xsrc, ysrc, zsrc, xtar, ytar, ztar, 0);
}

int CMeshManager::Clip_Vertices_Screen(int Num, vector3 *Source)
{
	float MinClipX = 0;
	float MaxClipX = (float)m_ViewWidth - 1;

	float MinClipY = 0;
	float MaxClipY = (float)m_ViewHeight - 1;

    float Scale;
    vector3 Vertices[8];

	//последняя вершина
	vector3 *l = &Source[Num - 1];
    int j = 0;

    for (int i = 0; i < Num; i++)
	{
		//сюда записываем результат
        vector3 *v1 = &Vertices[j];
        //последняя вершина v2
		vector3 *v2 = l;
        //нулевая (первая) вершина l
		l = &Source[i];

        if (v2->x < MinClipX)
		{
            if (l->x < MinClipX)
			{
                continue;
            }
            Scale = (MinClipX - l->x) / (v2->x - l->x);
            v1->x = MinClipX;
            v1->y = (v2->y - l->y) * Scale + l->y;
            v1->z = (v2->z - l->z) * Scale + l->z;
            v1->tu = (v2->tu - l->tu) * Scale + l->tu;
            v1->tv = (v2->tv - l->tv) * Scale + l->tv;
			v1 = &Vertices[++j];
        }
		else if (v2->x > MaxClipX)
		{
            if (l->x > MaxClipX)
			{
                continue;
            }
            Scale = (MaxClipX - l->x) / (v2->x - l->x);
            v1->x = MaxClipX;
            v1->y = (v2->y - l->y) * Scale + l->y;
            v1->z = (v2->z - l->z) * Scale + l->z;
            v1->tu = (v2->tu - l->tu) * Scale + l->tu;
            v1->tv = (v2->tv - l->tv) * Scale + l->tv;
			v1 = &Vertices[++j];
        }

        if (l->x < MinClipX)
		{
            Scale = (MinClipX - l->x) / (v2->x - l->x);
            v1->x = MinClipX;
            v1->y = (v2->y - l->y) * Scale + l->y;
            v1->z = (v2->z - l->z) * Scale + l->z;
            v1->tu = (v2->tu - l->tu) * Scale + l->tu;
            v1->tv = (v2->tv - l->tv) * Scale + l->tv;
			v1 = &Vertices[++j];
        }
		else if (l->x > MaxClipX)
		{
            Scale = (MaxClipX - l->x) / (v2->x - l->x);
            v1->x = MaxClipX;
            v1->y = (v2->y - l->y) * Scale + l->y;
            v1->z = (v2->z - l->z) * Scale + l->z;
            v1->tu = (v2->tu - l->tu) * Scale + l->tu;
            v1->tv = (v2->tv - l->tv) * Scale + l->tv;
			v1 = &Vertices[++j];
        }
		else
		{
            v1->x = l->x;
            v1->y = l->y;
            v1->z = l->z;
            v1->tu = l->tu;
            v1->tv = l->tv;
			v1 = &Vertices[++j];
        }
    }

    if (j < 3)
	{
        return 0;
    }

    Num = j;
    l = &Vertices[j - 1];
    j = 0;

    for (int i = 0; i < Num; i++)
	{
        vector3 *v1 = &Source[j];
        vector3 *v2 = l;
        l = &Vertices[i];

        if (v2->y < MinClipY)
		{
            if (l->y < MinClipY)
			{
                continue;
            }
            Scale = (MinClipY - l->y) / (v2->y - l->y);
            v1->x = (v2->x - l->x) * Scale + l->x;
            v1->y = MinClipY;
            v1->z = (v2->z - l->z) * Scale + l->z;
            v1->tu = (v2->tu - l->tu) * Scale + l->tu;
            v1->tv = (v2->tv - l->tv) * Scale + l->tv;
			v1 = &Source[++j];
        }
		else if (v2->y > MaxClipY)
		{
            if (l->y > MaxClipY)
			{
                continue;
            }
            Scale = (MaxClipY - l->y) / (v2->y - l->y);
            v1->x = (v2->x - l->x) * Scale + l->x;
            v1->y = MaxClipY;
            v1->z = (v2->z - l->z) * Scale + l->z;
            v1->tu = (v2->tu - l->tu) * Scale + l->tu;
            v1->tv = (v2->tv - l->tv) * Scale + l->tv;
			v1 = &Source[++j];
        }

        if (l->y < MinClipY)
		{
            Scale = (MinClipY - l->y) / (v2->y - l->y);
            v1->x = (v2->x - l->x) * Scale + l->x;
            v1->y = MinClipY;
            v1->z = (v2->z - l->z) * Scale + l->z;
            v1->tu = (v2->tu - l->tu) * Scale + l->tu;
            v1->tv = (v2->tv - l->tv) * Scale + l->tv;
			v1 = &Source[++j];
        }
		else if (l->y > MaxClipY)
		{
            Scale = (MaxClipY - l->y) / (v2->y - l->y);
            v1->x = (v2->x - l->x) * Scale + l->x;
            v1->y = MaxClipY;
            v1->z = (v2->z - l->z) * Scale + l->z;
            v1->tu = (v2->tu - l->tu) * Scale + l->tu;
            v1->tv = (v2->tv - l->tv) * Scale + l->tv;
            v1 = &Source[++j];
        }
		else
		{
            v1->x = l->x;
            v1->y = l->y;
            v1->z = l->z;
            v1->tu = l->tu;
            v1->tv = l->tv;
			v1 = &Source[++j];
        }
    }
	
    if (j < 3)
	{
        return 0;
    }

	return j;

}

int CMeshManager::Visible_ZClip(vector3 &VecIn1, vector3 &VecIn2, vector3 &VecIn3)
{
    double v1x = VecIn1.x;
    double v1y = VecIn1.y;
    double v1z = VecIn1.z;
    double v2x = VecIn2.x;
    double v2y = VecIn2.y;
    double v2z = VecIn2.z;
    double v3x = VecIn3.x;
    double v3y = VecIn3.y;
    double v3z = VecIn3.z;

    double a = v3y * v1x - v1y * v3x;
    double b = v3x * v1z - v1x * v3z;
    double c = v3z * v1y - v1z * v3y;

    return a * v2z + b * v2y + c * v2x > 0.0;
}

int CMeshManager::Clip_Vertices_ZNear(int Num, vector3 *Source)
{

        float Scale;
    vector3 Vertices[8];

	//последняя вершина
	vector3 *l = &Source[Num - 1];
    int j = 0;

    for (int i = 0; i < Num; i++)
	{
		//сюда записываем результат
        vector3 *v1 = &Vertices[j];
        //последняя вершина v2
		vector3 *v2 = l;
        //нулевая (первая) вершина l
		l = &Source[i];

        if (v2->z < m_ZNear)
		{
            if (l->z < m_ZNear)
			{
                continue;
            }
            Scale = (m_ZNear - l->z) / (v2->z - l->z);
			v1->x = (v2->x - l->x) * Scale + l->x;
            v1->y = (v2->y - l->y) * Scale + l->y;
            //v1->z = (v2->z - l->z) * Scale + l->z;
			v1->z = (float)m_ZNear;
            v1->tu = (v2->tu - l->tu) * Scale + l->tu;
            v1->tv = (v2->tv - l->tv) * Scale + l->tv;
			v1 = &Vertices[++j];
        }

		if (l->z < m_ZNear)
		{
            Scale = (m_ZNear - l->z) / (v2->z - l->z);
            v1->x = (v2->x - l->x) * Scale + l->x;
            v1->y = (v2->y - l->y) * Scale + l->y;
            //v1->z = (v2->z - l->z) * Scale + l->z;
			v1->z = (float)m_ZNear;
            v1->tu = (v2->tu - l->tu) * Scale + l->tu;
            v1->tv = (v2->tv - l->tv) * Scale + l->tv;
			v1 = &Vertices[++j];
        }
		else
		{
            v1->x = l->x;
            v1->y = l->y;
            v1->z = l->z;
            v1->tu = l->tu;
            v1->tv = l->tv;
			v1 = &Vertices[++j];
        }
    }

    if (j < 3)
	{
        return 0;
    }

	for ( int i = 0; i < j; i++ )
	{
		Source[i] = Vertices[i];
	}

	return j;
}


void CMeshManager::Update_MeshManager()
{

	sort3dptr = (int32_t *)sort3d_buffer;
	info3dptr = (int16_t *)info3d_buffer;

	surfacenum = 0;

	Get_View_Matrix();

	m_ZNear = (20 << W2V_SHIFT);

	m_TransformedPoly.Reset();
		
	for ( int j =0; j < m_PolygonsSource.PolygonCount; j++)
	{
		vector3 Vec1 = m_PolygonsSource.PolyList[j].Vertex[0];
		vector3 Vec2 = m_PolygonsSource.PolyList[j].Vertex[1];
		vector3 Vec3 = m_PolygonsSource.PolyList[j].Vertex[2];

		phd_PushMatrix();

		phd_TranslateAbs(0, 0, 0);

		Vec1 = Vec3_Mat4x4_Mul(Vec1, *g_PhdMatrixPtr);
		Vec2 = Vec3_Mat4x4_Mul(Vec2, *g_PhdMatrixPtr);
		Vec3 = Vec3_Mat4x4_Mul(Vec3, *g_PhdMatrixPtr);

		phd_PopMatrix();

		float z1 = Vec1.z;
		float z2 = Vec2.z;
		float z3 = Vec3.z;

		//backface culling
		
		if ( !Visible_ZClip(Vec1, Vec2, Vec3) )
		{
			continue;
		}
		
		vector3 ClippedVertices[8];

		memset(ClippedVertices, 0, sizeof(vector3) * 8);

		ClippedVertices[0] = Vec1;
		ClippedVertices[1] = Vec2;
		ClippedVertices[2] = Vec3;

		int Verts = Clip_Vertices_ZNear(3, &ClippedVertices[0]);

		list FrontList;


		for ( int k = 1; k < Verts - 1; k++ )
		{
			polygon pt1;
					
			pt1.Vertex[0] = ClippedVertices[0];
			pt1.Vertex[1] = ClippedVertices[k];
			pt1.Vertex[2] = ClippedVertices[k+1];

			pt1.TexID = m_PolygonsSource.PolyList[j].TexID; 
				
			FrontList.Add_To_List(&pt1);
		}


		//отсечение в пространстве вида сделано
		//можно дальше трансформировать вершины
		//умножаем вершины на матрицу проекции
		//и далее деление на z, после отсечения
		//в пространстве вида ZNear треугольников может
		//быть 1 или 2 FrontList.PolygonCount

		for ( int i = 0; i < FrontList.PolygonCount; i++)
		{
			vector3 Vec1, Vec2, Vec3;

			Vec1 = FrontList.PolyList[i].Vertex[0];
			Vec2 = FrontList.PolyList[i].Vertex[1];
			Vec3 = FrontList.PolyList[i].Vertex[2];
			
			//field of view 80 degree, 65536 / 360 = 182
			//1 градус равен 182 единицам
			int fov = 80 * 182;

			int c = phd_cos(fov / 2);
			int s = phd_sin(fov / 2);

			float g_Persp = (float)(((m_ViewWidth / 2)  * c) / s);

			Vec1.x = Vec1.x/(Vec1.z / g_Persp) + m_ViewWidth / 2.0f;
			Vec1.y = -Vec1.y/(Vec1.z / g_Persp) + m_ViewHeight / 2.0f;
			
			Vec2.x = Vec2.x/(Vec2.z / g_Persp) + m_ViewWidth / 2.0f;
			Vec2.y = -Vec2.y/(Vec2.z / g_Persp) + m_ViewHeight / 2.0f;
			
			Vec3.x = Vec3.x/(Vec3.z / g_Persp) + m_ViewWidth / 2.0f;
			Vec3.y = -Vec3.y/(Vec3.z / g_Persp) + m_ViewHeight / 2.0f;

			vector3 Vertex[8];

			memset(Vertex, 0, sizeof(vector3) * 8);

				float one = 8589934592.0f;

				Vertex[0].x = Vec1.x;
				Vertex[0].y = Vec1.y;
				
				Vertex[0].z = one / Vec1.z;
				
				Vertex[0].tu = Vec1.tu * Vertex[0].z;
				Vertex[0].tv = Vec1.tv * Vertex[0].z;


				Vertex[1].x = Vec2.x;
				Vertex[1].y = Vec2.y;
				
				Vertex[1].z =  one / Vec2.z;
				
				Vertex[1].tu = Vec2.tu * Vertex[1].z;
				Vertex[1].tv = Vec2.tv * Vertex[1].z;


				Vertex[2].x = Vec3.x;
				Vertex[2].y = Vec3.y;
				Vertex[2].z =  one / Vec3.z;

				Vertex[2].tu = Vec3.tu * Vertex[2].z;
				Vertex[2].tv = Vec3.tv * Vertex[2].z;

				int Verts = Clip_Vertices_Screen(3, &Vertex[0]);

				float depth = z1;

				if (depth < z2)
				{
					depth = z2;
				}

				if (depth < z3)
				{
					depth = z3;
				}

				int32_t *sort = sort3dptr;
				int16_t *info = info3dptr;

				sort[0] = (int32_t)info;
				sort[1] = (int32_t)depth;

				sort3dptr += 2;

				info[0] = FrontList.PolyList[i].TexID;
				info[1] = Verts;

				info += 2;

				int32_t indx = 0;
/*

		if (Vec1.x > m_ViewWidth)
			if (Vec1.x > 32760)
				Vec1.x = 32760;

		if (Vec1.x < 0)
			if (Vec1.x < -32760)
				Vec1.x = -32760;

		if (Vec1.y > m_ViewHeight)
			if (Vec1.y > 32760)
				Vec1.y = 32760;

		if (Vec1.y < 0)
			if (Vec1.y < -32760)
				Vec1.y = -32760;


		

		if (Vec2.x > m_ViewWidth)
			if (Vec2.x > 32760)
				Vec2.x = 32760;

		if (Vec2.x < 0)
			if (Vec2.x < -32760)
				Vec2.x = -32760;

		if (Vec2.y > m_ViewHeight)
			if (Vec2.y > 32760)
				Vec2.y = 32760;

		if (Vec2.y < 0)
			if (Vec2.y < -32760)
				Vec2.y = -32760;

		
		
		
		if (Vec3.x > m_ViewWidth)
			if (Vec3.x > 32760)
				Vec3.x = 32760;

		if (Vec3.x < 0)
			if (Vec3.x < -32760)
				Vec3.x = -32760;

		if (Vec3.y > m_ViewHeight)
			if (Vec3.y > 32760)
				Vec3.y = 32760;

		if (Vec3.y < 0)
			if (Vec3.y < -32760)
				Vec3.y = -32760;

*/


				do
				{
					info[0] = (short int)Vertex[indx].x;
					info[1] = (short int)Vertex[indx].y;
					//info[2] = (short int)Vertex[indx].g;
					info[2] = (short int)0;

					*(float *)&info[3] = Vertex[indx].z;
					*(float *)&info[5] = Vertex[indx].tu;
					*(float *)&info[7] = Vertex[indx].tv;

					info += 9;
					indx++;

				} while (indx < Verts);

				info3dptr = info;

				surfacenum++;
			
		} //end for FrontList.PolygonCount

		FrontList.Reset();
	}
}

void CMeshManager::Draw_MeshManager()
{
	Clear_BackBuffer();

	SortPolyList(surfacenum, sort3d_buffer);
	PrintPolyList(dibdc->surface);

	Present_BackBuffer();
}


int CMeshManager::Load_BMP(char *szFilename, int Tile)
{
		FILE *fp = NULL;
	fopen_s(&fp, szFilename, "rb");
	if(fp==NULL) printf("Error Open File");

	BITMAPFILEHEADER bfh;
	fread(&bfh, sizeof(BITMAPFILEHEADER), 1, fp);

	BITMAPINFOHEADER bih;
	fread(&bih, sizeof(BITMAPINFOHEADER), 1, fp);

	fread(&palette, 256 * 4,1,fp);

	fseek(fp, bfh.bfOffBits, SEEK_SET);

	m_pLevelTile[Tile] = new unsigned char [bih.biWidth*bih.biHeight];

	fread(m_pLevelTile[Tile],bih.biWidth*bih.biHeight,1,fp);

	m_TextureWidth = bih.biWidth << 8;
	m_TextureHeight = bih.biHeight << 8;

	return true;
}

void CMeshManager::Timer_Start()
{
	QueryPerformanceFrequency((LARGE_INTEGER*)& m_PerfFreq);
	QueryPerformanceCounter((LARGE_INTEGER*)& m_LastTime);
	m_StartTime = m_LastTime;
	m_TimeScale = 1.0f / m_PerfFreq;
}

float CMeshManager::Get_Elapsed_Time()
{
	__int64 nowTime;

	QueryPerformanceCounter((LARGE_INTEGER*)& nowTime);
	m_ElapsedTime = (nowTime - m_StartTime) * m_TimeScale;
	m_StartTime = nowTime;

	return m_ElapsedTime;
}

void CMeshManager::SortPolyList(int number, int buffer[][2])
{
	int i;
	if (number)
	{
		for (i = 0; i < number; i++) // eliminate polygon flicker
			buffer[i][1] += i;

		do_quickysorty(0, number - 1, buffer);
	}
}

void CMeshManager::do_quickysorty(int left, int right, int buffer[][2])
{
	int i, j;
	int compare, swap;

	i = left;
	j = right;
	compare = buffer[(left + right) / 2][1]; /* get middle value*/

	do
	{
		while (buffer[i][1] > compare && i < right)
			++i; /* was <x*/
		while (compare > buffer[j][1] && j > left)
			--j;	/* was x<*/
		if (i <= j) /* was ( i<=j )*/
		{
			swap = buffer[i][1];
			buffer[i][1] = buffer[j][1]; /* swap elements*/
			buffer[j][1] = swap;

			swap = buffer[i][0];
			buffer[i][0] = buffer[j][0];
			buffer[j][0] = swap;

			i++;
			j--;
		}
	} while (i <= j);

	if (left < j)
		do_quickysorty(left, j, buffer);
	if (i < right)
		do_quickysorty(i, right, buffer);
}

void CMeshManager::PrintPolyList(void *ptr)
{
	/* Draw onto render surface */
	int i;
	int *sptr;
	short int *iptr;
	short int tex;

	phd_winptr = (char *)ptr;

	sptr = (int *)sort3d_buffer;
	for (i = surfacenum; i > 0; i--)
	{
		iptr = (short int *)(*sptr);
		tex = *(iptr++);

		if(xgen_xguvpersp_fp(iptr))
			gtmap_persp32_fp( xgen_ymin, xgen_ymax, (unsigned char*)m_pLevelTile[tex] );

		sptr += 2;
	}
}
