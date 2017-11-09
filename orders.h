/*
   rdesktop: A Remote Desktop Protocol client.
   RDP order processing
   Copyright (C) Matthew Chapman 1999-2008

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define RDP_ORDER_STANDARD   0x01
#define RDP_ORDER_SECONDARY  0x02
#define RDP_ORDER_BOUNDS     0x04
#define RDP_ORDER_CHANGE     0x08
#define RDP_ORDER_DELTA      0x10
#define RDP_ORDER_LASTBOUNDS 0x20
#define RDP_ORDER_SMALL      0x40
#define RDP_ORDER_TINY       0x80

enum RDP_ORDER_TYPE
{
	RDP_ORDER_DESTBLT = 0,
	RDP_ORDER_PATBLT = 1,
	RDP_ORDER_SCREENBLT = 2,
	RDP_ORDER_LINE = 9,
	RDP_ORDER_RECT = 10,
	RDP_ORDER_DESKSAVE = 11,
	RDP_ORDER_MEMBLT = 13,
	RDP_ORDER_TRIBLT = 14,
	RDP_ORDER_POLYGON = 20,
	RDP_ORDER_POLYGON2 = 21,
	RDP_ORDER_POLYLINE = 22,
	RDP_ORDER_ELLIPSE = 25,
	RDP_ORDER_ELLIPSE2 = 26,
	RDP_ORDER_TEXT2 = 27
};

enum RDP_SECONDARY_ORDER_TYPE
{
	RDP_ORDER_RAW_BMPCACHE = 0,
	RDP_ORDER_COLCACHE = 1,
	RDP_ORDER_BMPCACHE = 2,
	RDP_ORDER_FONTCACHE = 3,
	RDP_ORDER_RAW_BMPCACHE2 = 4,
	RDP_ORDER_BMPCACHE2 = 5,
	RDP_ORDER_BRUSHCACHE = 7
};

typedef struct _DESTBLT_ORDER
{
	int16_t x;
	int16_t y;
	int16_t cx;
	int16_t cy;
	uint8_t opcode;

}
DESTBLT_ORDER;

typedef struct _PATBLT_ORDER
{
	int16_t x;
	int16_t y;
	int16_t cx;
	int16_t cy;
	uint8_t opcode;
	uint32_t bgcolour;
	uint32_t fgcolour;
	BRUSH brush;

}
PATBLT_ORDER;

typedef struct _SCREENBLT_ORDER
{
	int16_t x;
	int16_t y;
	int16_t cx;
	int16_t cy;
	uint8_t opcode;
	int16_t srcx;
	int16_t srcy;

}
SCREENBLT_ORDER;

typedef struct _LINE_ORDER
{
	uint16_t mixmode;
	int16_t startx;
	int16_t starty;
	int16_t endx;
	int16_t endy;
	uint32_t bgcolour;
	uint8_t opcode;
	PEN pen;

}
LINE_ORDER;

typedef struct _RECT_ORDER
{
	int16_t x;
	int16_t y;
	int16_t cx;
	int16_t cy;
	uint32_t colour;

}
RECT_ORDER;

typedef struct _DESKSAVE_ORDER
{
	uint32_t offset;
	int16_t left;
	int16_t top;
	int16_t right;
	int16_t bottom;
	uint8_t action;

}
DESKSAVE_ORDER;

typedef struct _TRIBLT_ORDER
{
	uint8_t colour_table;
	uint8_t cache_id;
	int16_t x;
	int16_t y;
	int16_t cx;
	int16_t cy;
	uint8_t opcode;
	int16_t srcx;
	int16_t srcy;
	uint32_t bgcolour;
	uint32_t fgcolour;
	BRUSH brush;
	uint16_t cache_idx;
	uint16_t unknown;

}
TRIBLT_ORDER;

typedef struct _MEMBLT_ORDER
{
	uint8_t colour_table;
	uint8_t cache_id;
	int16_t x;
	int16_t y;
	int16_t cx;
	int16_t cy;
	uint8_t opcode;
	int16_t srcx;
	int16_t srcy;
	uint16_t cache_idx;

}
MEMBLT_ORDER;

#define MAX_DATA 256

typedef struct _POLYGON_ORDER
{
	int16_t x;
	int16_t y;
	uint8_t opcode;
	uint8_t fillmode;
	uint32_t fgcolour;
	uint8_t npoints;
	uint8_t datasize;
	uint8_t data[MAX_DATA];

}
POLYGON_ORDER;

typedef struct _POLYGON2_ORDER
{
	int16_t x;
	int16_t y;
	uint8_t opcode;
	uint8_t fillmode;
	uint32_t bgcolour;
	uint32_t fgcolour;
	BRUSH brush;
	uint8_t npoints;
	uint8_t datasize;
	uint8_t data[MAX_DATA];

}
POLYGON2_ORDER;

typedef struct _POLYLINE_ORDER
{
	int16_t x;
	int16_t y;
	uint8_t opcode;
	uint32_t fgcolour;
	uint8_t lines;
	uint8_t datasize;
	uint8_t data[MAX_DATA];

}
POLYLINE_ORDER;

typedef struct _ELLIPSE_ORDER
{
	int16_t left;
	int16_t top;
	int16_t right;
	int16_t bottom;
	uint8_t opcode;
	uint8_t fillmode;
	uint32_t fgcolour;

}
ELLIPSE_ORDER;

typedef struct _ELLIPSE2_ORDER
{
	int16_t left;
	int16_t top;
	int16_t right;
	int16_t bottom;
	uint8_t opcode;
	uint8_t fillmode;
	BRUSH brush;
	uint32_t bgcolour;
	uint32_t fgcolour;

}
ELLIPSE2_ORDER;

#define MAX_TEXT 256

typedef struct _TEXT2_ORDER
{
	uint8_t font;
	uint8_t flags;
	uint8_t opcode;
	uint8_t mixmode;
	uint32_t bgcolour;
	uint32_t fgcolour;
	int16_t clipleft;
	int16_t cliptop;
	int16_t clipright;
	int16_t clipbottom;
	int16_t boxleft;
	int16_t boxtop;
	int16_t boxright;
	int16_t boxbottom;
	BRUSH brush;
	int16_t x;
	int16_t y;
	uint8_t length;
	uint8_t text[MAX_TEXT];

}
TEXT2_ORDER;

typedef struct _RDP_ORDER_STATE
{
	uint8_t order_type;
	BOUNDS bounds;

	DESTBLT_ORDER destblt;
	PATBLT_ORDER patblt;
	SCREENBLT_ORDER screenblt;
	LINE_ORDER line;
	RECT_ORDER rect;
	DESKSAVE_ORDER desksave;
	MEMBLT_ORDER memblt;
	TRIBLT_ORDER triblt;
	POLYGON_ORDER polygon;
	POLYGON2_ORDER polygon2;
	POLYLINE_ORDER polyline;
	ELLIPSE_ORDER ellipse;
	ELLIPSE2_ORDER ellipse2;
	TEXT2_ORDER text2;

}
RDP_ORDER_STATE;

typedef struct _RDP_RAW_BMPCACHE_ORDER
{
	uint8_t cache_id;
	uint8_t pad1;
	uint8_t width;
	uint8_t height;
	uint8_t bpp;
	uint16_t bufsize;
	uint16_t cache_idx;
	uint8_t *data;

}
RDP_RAW_BMPCACHE_ORDER;

typedef struct _RDP_BMPCACHE_ORDER
{
	uint8_t cache_id;
	uint8_t pad1;
	uint8_t width;
	uint8_t height;
	uint8_t bpp;
	uint16_t bufsize;
	uint16_t cache_idx;
	uint16_t pad2;
	uint16_t size;
	uint16_t row_size;
	uint16_t final_size;
	uint8_t *data;

}
RDP_BMPCACHE_ORDER;

/* RDP_BMPCACHE2_ORDER */
#define ID_MASK			0x0007
#define MODE_MASK		0x0038
#define SQUARE			0x0080
#define PERSIST			0x0100
#define FLAG_51_UNKNOWN		0x0800

#define MODE_SHIFT		3

#define LONG_FORMAT		0x80
#define BUFSIZE_MASK		0x3FFF	/* or 0x1FFF? */

#define MAX_GLYPH 32

typedef struct _RDP_FONT_GLYPH
{
	uint16_t character;
	uint16_t unknown;
	uint16_t baseline;
	uint16_t width;
	uint16_t height;
	uint8_t data[MAX_GLYPH];

}
RDP_FONT_GLYPH;

#define MAX_GLYPHS 256

typedef struct _RDP_FONTCACHE_ORDER
{
	uint8_t font;
	uint8_t nglyphs;
	RDP_FONT_GLYPH glyphs[MAX_GLYPHS];

}
RDP_FONTCACHE_ORDER;

typedef struct _RDP_COLCACHE_ORDER
{
	uint8_t cache_id;
	COLOURMAP map;

}
RDP_COLCACHE_ORDER;
