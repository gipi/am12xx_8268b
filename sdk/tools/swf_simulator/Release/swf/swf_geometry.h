#ifndef SWF_GEOMETRY_H
#define SWF_GEOMETRY_H

#include "swf_base.h"

typedef struct _FillEdge
{
	int x;
	int y;
	int dx;
#if ANTI_ALIASING_QUALITY== AA_MID_QUALITY
	int dy;
#endif
} FEDGE;

typedef struct _EdgeTab
{
	int		EdgeNum;
	FEDGE	Edge[SWF_MAX_EDGE_NUM];
} ETAB;

typedef struct _AA_Edge
{
	int x0, x1;
	int y0, y1;
	int ey;
	int dx, dy;
} AA_EDGE;

typedef struct _AA_EdgeTab
{
	int     EdgeNum;
	AA_EDGE	Edge[SWF_MAX_AA_EDGE_NUM];
} AA_ETAB;

typedef struct _AA_Cell
{
	short x;
	short cover;
	short area;
} AA_CELL;

typedef struct _AA_Line
{
	AA_CELL  cell[1024];
	AA_CELL* free_cell;
	int      col, row;
} AA_LINE;

typedef struct _AA_Point
{
	int flag : 2;
	int x    : 30;
	int y;
} AA_POINT;

typedef struct _SimpleLink
{
	union
	{
		struct {
			union 
			{
				FEDGE		* edge;
				SCANLINE	* scanline;
				AA_EDGE		* aa_edge;
			};
			struct _SimpleLink * next;
		};
		AA_POINT point;
	};
} SIMPLE_LINK;

typedef struct _SimpleLinkHead
{
	SIMPLE_LINK * head;
	SIMPLE_LINK * tail;
	SIMPLE_LINK * free;
	int currIndex;
	int linkLen;
} SIMPLE_LINK_HEAD;


#define SET_POINT(p,_x,_y,_flag) \
{\
	p->x = _x;\
	p->y = _y;\
	p->flag = _flag;\
}

//////////////////////////////////////////////////////////////////////////
// added for special use 
/*

int SWFDrawLine(UILINE* line);
int SWFFillPolygon(UIPOLYGON* Polygon);
int SWFDrawText(UITEXT *text);
*/
//////////////////////////////////////////////////////////////////////////

#endif
