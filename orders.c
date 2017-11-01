/* -*- c-basic-offset: 8 -*-
   rdesktop: A Remote Desktop Protocol client.
   RDP order processing
   Copyright (C) Matthew Chapman <matthewc.unsw.edu.au> 1999-2008

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

#include "rdesktop.h"
#include "orders.h"

extern uint8 *g_next_packet;
static RDP_ORDER_STATE g_order_state;
extern RDP_VERSION g_rdp_version;

/* Read field indicating which parameters are present */
static void
rdp_in_present(STREAM s, uint32 * present, uint8 flags, int size)
{
	uint8 bits;
	int i;

	if (flags & TS_ZERO_FIELD_BYTE_BIT0)
	{
		size--;
	}

	if (flags & TS_ZERO_FIELD_BYTE_BIT1)
	{
		if (size < 2)
			size = 0;
		else
			size -= 2;
	}

	*present = 0;
	for (i = 0; i < size; i++)
	{
		in_uint8(s, bits);
		*present |= bits << (i * 8);
	}
}

/* Read a co-ordinate (16-bit, or 8-bit delta) */
static void
rdp_in_coord(STREAM s, sint16 * coord, RD_BOOL delta)
{
	sint8 change;

	if (delta)
	{
		in_uint8(s, change);
		*coord += change;
	}
	else
	{
		in_uint16_le(s, *coord);
	}
}

/* Parse a delta co-ordinate in polyline/polygon order form */
static int
parse_delta(uint8 * buffer, int *offset)
{
	int value = buffer[(*offset)++];
	int two_byte = value & 0x80;

	if (value & 0x40)	/* sign bit */
		value |= ~0x3f;
	else
		value &= 0x3f;

	if (two_byte)
		value = (value << 8) | buffer[(*offset)++];

	return value;
}

/* Read a colour entry */
static void
rdp_in_colour(STREAM s, uint32 * colour)
{
	uint32 i;
	in_uint8(s, i);
	*colour = i;
	in_uint8(s, i);
	*colour |= i << 8;
	in_uint8(s, i);
	*colour |= i << 16;
}

/* Parse bounds information */
static RD_BOOL
rdp_parse_bounds(STREAM s, BOUNDS * bounds)
{
	uint8 present;

	in_uint8(s, present);

	if (present & 1)
		rdp_in_coord(s, &bounds->left, False);
	else if (present & 16)
		rdp_in_coord(s, &bounds->left, True);

	if (present & 2)
		rdp_in_coord(s, &bounds->top, False);
	else if (present & 32)
		rdp_in_coord(s, &bounds->top, True);

	if (present & 4)
		rdp_in_coord(s, &bounds->right, False);
	else if (present & 64)
		rdp_in_coord(s, &bounds->right, True);

	if (present & 8)
		rdp_in_coord(s, &bounds->bottom, False);
	else if (present & 128)
		rdp_in_coord(s, &bounds->bottom, True);

	return s_check(s);
}

/* Parse a pen */
static RD_BOOL
rdp_parse_pen(STREAM s, PEN * pen, uint32 present)
{
	if (present & 1)
		in_uint8(s, pen->style);

	if (present & 2)
		in_uint8(s, pen->width);

	if (present & 4)
		rdp_in_colour(s, &pen->colour);

	return s_check(s);
}

static void
setup_brush(BRUSH * out_brush, BRUSH * in_brush)
{
	BRUSHDATA *brush_data;
	uint8 cache_idx;
	uint8 colour_code;

	memcpy(out_brush, in_brush, sizeof(BRUSH));
	if (out_brush->style & 0x80)
	{
		colour_code = out_brush->style & 0x0f;
		cache_idx = out_brush->pattern[0];
		brush_data = cache_get_brush_data(colour_code, cache_idx);
		if ((brush_data == NULL) || (brush_data->data == NULL))
		{
			logger(Graphics, Error, "setup_brush(), error getting brush data, style %x",
			       out_brush->style);
			out_brush->bd = NULL;
			memset(out_brush->pattern, 0, 8);
		}
		else
		{
			out_brush->bd = brush_data;
		}
		out_brush->style = 3;
	}
}

/* Parse a brush */
static RD_BOOL
rdp_parse_brush(STREAM s, BRUSH * brush, uint32 present)
{
	if (present & 1)
		in_uint8(s, brush->xorigin);

	if (present & 2)
		in_uint8(s, brush->yorigin);

	if (present & 4)
		in_uint8(s, brush->style);

	if (present & 8)
		in_uint8(s, brush->pattern[0]);

	if (present & 16)
		in_uint8a(s, &brush->pattern[1], 7);

	return s_check(s);
}

/* Process a destination blt order */
static void
process_destblt(STREAM s, DESTBLT_ORDER * os, uint32 present, RD_BOOL delta)
{
	if (present & 0x01)
		rdp_in_coord(s, &os->x, delta);

	if (present & 0x02)
		rdp_in_coord(s, &os->y, delta);

	if (present & 0x04)
		rdp_in_coord(s, &os->cx, delta);

	if (present & 0x08)
		rdp_in_coord(s, &os->cy, delta);

	if (present & 0x10)
		in_uint8(s, os->opcode);

	logger(Graphics, Debug, "process_destblt(), op=0x%x, x=%d, y=%d, cx=%d, cy=%d",
	       os->opcode, os->x, os->y, os->cx, os->cy);

	ui_destblt(ROP2_S(os->opcode), os->x, os->y, os->cx, os->cy);
}

/* Process a pattern blt order */
static void
process_patblt(STREAM s, PATBLT_ORDER * os, uint32 present, RD_BOOL delta)
{
	BRUSH brush;

	if (present & 0x0001)
		rdp_in_coord(s, &os->x, delta);

	if (present & 0x0002)
		rdp_in_coord(s, &os->y, delta);

	if (present & 0x0004)
		rdp_in_coord(s, &os->cx, delta);

	if (present & 0x0008)
		rdp_in_coord(s, &os->cy, delta);

	if (present & 0x0010)
		in_uint8(s, os->opcode);

	if (present & 0x0020)
		rdp_in_colour(s, &os->bgcolour);

	if (present & 0x0040)
		rdp_in_colour(s, &os->fgcolour);

	rdp_parse_brush(s, &os->brush, present >> 7);

	logger(Graphics, Debug,
	       "process_patblt(), op=0x%x, x=%d, y=%d, cx=%d, cy=%d, bs=%d, bg=0x%x, fg=0x%x)",
	       os->opcode, os->x, os->y, os->cx, os->cy, os->brush.style, os->bgcolour,
	       os->fgcolour);

	setup_brush(&brush, &os->brush);

	ui_patblt(ROP2_P(os->opcode), os->x, os->y, os->cx, os->cy,
		  &brush, os->bgcolour, os->fgcolour);
}

/* Process a screen blt order */
static void
process_screenblt(STREAM s, SCREENBLT_ORDER * os, uint32 present, RD_BOOL delta)
{
	if (present & 0x0001)
		rdp_in_coord(s, &os->x, delta);

	if (present & 0x0002)
		rdp_in_coord(s, &os->y, delta);

	if (present & 0x0004)
		rdp_in_coord(s, &os->cx, delta);

	if (present & 0x0008)
		rdp_in_coord(s, &os->cy, delta);

	if (present & 0x0010)
		in_uint8(s, os->opcode);

	if (present & 0x0020)
		rdp_in_coord(s, &os->srcx, delta);

	if (present & 0x0040)
		rdp_in_coord(s, &os->srcy, delta);

	logger(Graphics, Debug,
	       "process_screenblt(), op=0x%x, x=%d, y=%d, cx=%d, cy=%d, srcx=%d, srcy=%d)",
	       os->opcode, os->x, os->y, os->cx, os->cy, os->srcx, os->srcy);

	ui_screenblt(ROP2_S(os->opcode), os->x, os->y, os->cx, os->cy, os->srcx, os->srcy);
}

/* Process a line order */
static void
process_line(STREAM s, LINE_ORDER * os, uint32 present, RD_BOOL delta)
{
	if (present & 0x0001)
		in_uint16_le(s, os->mixmode);

	if (present & 0x0002)
		rdp_in_coord(s, &os->startx, delta);

	if (present & 0x0004)
		rdp_in_coord(s, &os->starty, delta);

	if (present & 0x0008)
		rdp_in_coord(s, &os->endx, delta);

	if (present & 0x0010)
		rdp_in_coord(s, &os->endy, delta);

	if (present & 0x0020)
		rdp_in_colour(s, &os->bgcolour);

	if (present & 0x0040)
		in_uint8(s, os->opcode);

	rdp_parse_pen(s, &os->pen, present >> 7);

	logger(Graphics, Debug, "process_line(), op=0x%x, sx=%d, sy=%d, dx=%d, dy=%d, fg=0x%x)",
	       os->opcode, os->startx, os->starty, os->endx, os->endy, os->pen.colour);

	if (os->opcode < 0x01 || os->opcode > 0x10)
	{
		logger(Graphics, Error, "process_line(), bad ROP2 0x%x", os->opcode);
		return;
	}

	ui_line(os->opcode - 1, os->startx, os->starty, os->endx, os->endy, &os->pen);
}

/* Process an opaque rectangle order */
static void
process_rect(STREAM s, RECT_ORDER * os, uint32 present, RD_BOOL delta)
{
	uint32 i;
	if (present & 0x01)
		rdp_in_coord(s, &os->x, delta);

	if (present & 0x02)
		rdp_in_coord(s, &os->y, delta);

	if (present & 0x04)
		rdp_in_coord(s, &os->cx, delta);

	if (present & 0x08)
		rdp_in_coord(s, &os->cy, delta);

	if (present & 0x10)
	{
		in_uint8(s, i);
		os->colour = (os->colour & 0xffffff00) | i;
	}

	if (present & 0x20)
	{
		in_uint8(s, i);
		os->colour = (os->colour & 0xffff00ff) | (i << 8);
	}

	if (present & 0x40)
	{
		in_uint8(s, i);
		os->colour = (os->colour & 0xff00ffff) | (i << 16);
	}

	logger(Graphics, Debug, "process_rect(), x=%d, y=%d, cx=%d, cy=%d, fg=0x%x",
	       os->x, os->y, os->cx, os->cy, os->colour);

	ui_rect(os->x, os->y, os->cx, os->cy, os->colour);
}

/* Process a desktop save order */
static void
process_desksave(STREAM s, DESKSAVE_ORDER * os, uint32 present, RD_BOOL delta)
{
	int width, height;

	if (present & 0x01)
		in_uint32_le(s, os->offset);

	if (present & 0x02)
		rdp_in_coord(s, &os->left, delta);

	if (present & 0x04)
		rdp_in_coord(s, &os->top, delta);

	if (present & 0x08)
		rdp_in_coord(s, &os->right, delta);

	if (present & 0x10)
		rdp_in_coord(s, &os->bottom, delta);

	if (present & 0x20)
		in_uint8(s, os->action);

	logger(Graphics, Debug, "process_desksave(), l=%d, t=%d, r=%d, b=%d, off=%d, op=%d",
	       os->left, os->top, os->right, os->bottom, os->offset, os->action);

	width = os->right - os->left + 1;
	height = os->bottom - os->top + 1;

	if (os->action == 0)
		ui_desktop_save(os->offset, os->left, os->top, width, height);
	else
		ui_desktop_restore(os->offset, os->left, os->top, width, height);
}

/* Process a memory blt order */
static void
process_memblt(STREAM s, MEMBLT_ORDER * os, uint32 present, RD_BOOL delta)
{
	RD_HBITMAP bitmap;

	if (present & 0x0001)
	{
		in_uint8(s, os->cache_id);
		in_uint8(s, os->colour_table);
	}

	if (present & 0x0002)
		rdp_in_coord(s, &os->x, delta);

	if (present & 0x0004)
		rdp_in_coord(s, &os->y, delta);

	if (present & 0x0008)
		rdp_in_coord(s, &os->cx, delta);

	if (present & 0x0010)
		rdp_in_coord(s, &os->cy, delta);

	if (present & 0x0020)
		in_uint8(s, os->opcode);

	if (present & 0x0040)
		rdp_in_coord(s, &os->srcx, delta);

	if (present & 0x0080)
		rdp_in_coord(s, &os->srcy, delta);

	if (present & 0x0100)
		in_uint16_le(s, os->cache_idx);

	logger(Graphics, Debug,
	       "process_memblt(), op=0x%x, x=%d, y=%d, cx=%d, cy=%d, id=%d, idx=%d", os->opcode,
	       os->x, os->y, os->cx, os->cy, os->cache_id, os->cache_idx);

	bitmap = cache_get_bitmap(os->cache_id, os->cache_idx);
	if (bitmap == NULL)
		return;

	ui_memblt(ROP2_S(os->opcode), os->x, os->y, os->cx, os->cy, bitmap, os->srcx, os->srcy);
}

/* Process a 3-way blt order */
static void
process_triblt(STREAM s, TRIBLT_ORDER * os, uint32 present, RD_BOOL delta)
{
	RD_HBITMAP bitmap;
	BRUSH brush;

	if (present & 0x000001)
	{
		in_uint8(s, os->cache_id);
		in_uint8(s, os->colour_table);
	}

	if (present & 0x000002)
		rdp_in_coord(s, &os->x, delta);

	if (present & 0x000004)
		rdp_in_coord(s, &os->y, delta);

	if (present & 0x000008)
		rdp_in_coord(s, &os->cx, delta);

	if (present & 0x000010)
		rdp_in_coord(s, &os->cy, delta);

	if (present & 0x000020)
		in_uint8(s, os->opcode);

	if (present & 0x000040)
		rdp_in_coord(s, &os->srcx, delta);

	if (present & 0x000080)
		rdp_in_coord(s, &os->srcy, delta);

	if (present & 0x000100)
		rdp_in_colour(s, &os->bgcolour);

	if (present & 0x000200)
		rdp_in_colour(s, &os->fgcolour);

	rdp_parse_brush(s, &os->brush, present >> 10);

	if (present & 0x008000)
		in_uint16_le(s, os->cache_idx);

	if (present & 0x010000)
		in_uint16_le(s, os->unknown);

	logger(Graphics, Debug,
	       "process_triblt(), op=0x%x, x=%d, y=%d, cx=%d, cy=%d, id=%d, idx=%d, bs=%d, bg=0x%x, fg=0x%x",
	       os->opcode, os->x, os->y, os->cx, os->cy, os->cache_id, os->cache_idx,
	       os->brush.style, os->bgcolour, os->fgcolour);

	bitmap = cache_get_bitmap(os->cache_id, os->cache_idx);
	if (bitmap == NULL)
		return;

	setup_brush(&brush, &os->brush);

	ui_triblt(os->opcode, os->x, os->y, os->cx, os->cy,
		  bitmap, os->srcx, os->srcy, &brush, os->bgcolour, os->fgcolour);
}

/* Process a polygon order */
static void
process_polygon(STREAM s, POLYGON_ORDER * os, uint32 present, RD_BOOL delta)
{
	int index, data, next;
	uint8 flags = 0;
	RD_POINT *points;

	if (present & 0x01)
		rdp_in_coord(s, &os->x, delta);

	if (present & 0x02)
		rdp_in_coord(s, &os->y, delta);

	if (present & 0x04)
		in_uint8(s, os->opcode);

	if (present & 0x08)
		in_uint8(s, os->fillmode);

	if (present & 0x10)
		rdp_in_colour(s, &os->fgcolour);

	if (present & 0x20)
		in_uint8(s, os->npoints);

	if (present & 0x40)
	{
		in_uint8(s, os->datasize);
		in_uint8a(s, os->data, os->datasize);
	}

	logger(Graphics, Debug,
	       "process_polygon(), x=%d, y=%d, op=0x%x, fm=%d, fg=0x%x, n=%d, sz=%d", os->x, os->y,
	       os->opcode, os->fillmode, os->fgcolour, os->npoints, os->datasize);

	if (os->opcode < 0x01 || os->opcode > 0x10)
	{
		logger(Graphics, Error, "process_polygon(), bad ROP2 0x%x", os->opcode);
		return;
	}

	points = (RD_POINT *) xmalloc((os->npoints + 1) * sizeof(RD_POINT));
	memset(points, 0, (os->npoints + 1) * sizeof(RD_POINT));

	points[0].x = os->x;
	points[0].y = os->y;

	index = 0;
	data = ((os->npoints - 1) / 4) + 1;
	for (next = 1; (next <= os->npoints) && (next < 256) && (data < os->datasize); next++)
	{
		if ((next - 1) % 4 == 0)
			flags = os->data[index++];

		if (~flags & 0x80)
			points[next].x = parse_delta(os->data, &data);

		if (~flags & 0x40)
			points[next].y = parse_delta(os->data, &data);

		flags <<= 2;
	}

	if (next - 1 == os->npoints)
		ui_polygon(os->opcode - 1, os->fillmode, points, os->npoints + 1, NULL, 0,
			   os->fgcolour);
	else
		logger(Graphics, Error, "process_polygon(), polygon parse error");

	xfree(points);
}

/* Process a polygon2 order */
static void
process_polygon2(STREAM s, POLYGON2_ORDER * os, uint32 present, RD_BOOL delta)
{
	int index, data, next;
	uint8 flags = 0;
	RD_POINT *points;
	BRUSH brush;

	if (present & 0x0001)
		rdp_in_coord(s, &os->x, delta);

	if (present & 0x0002)
		rdp_in_coord(s, &os->y, delta);

	if (present & 0x0004)
		in_uint8(s, os->opcode);

	if (present & 0x0008)
		in_uint8(s, os->fillmode);

	if (present & 0x0010)
		rdp_in_colour(s, &os->bgcolour);

	if (present & 0x0020)
		rdp_in_colour(s, &os->fgcolour);

	rdp_parse_brush(s, &os->brush, present >> 6);

	if (present & 0x0800)
		in_uint8(s, os->npoints);

	if (present & 0x1000)
	{
		in_uint8(s, os->datasize);
		in_uint8a(s, os->data, os->datasize);
	}

	logger(Graphics, Debug,
	       "process_polygon2(), x=%d, y=%d, op=0x%x, fm=%d, bs=%d, bg=0x%x, fg=0x%x, n=%d, sz=%d)",
	       os->x, os->y, os->opcode, os->fillmode, os->brush.style, os->bgcolour, os->fgcolour,
	       os->npoints, os->datasize);

	if (os->opcode < 0x01 || os->opcode > 0x10)
	{
		logger(Graphics, Error, "process_polygon2(), bad ROP2 0x%x", os->opcode);
		return;
	}

	setup_brush(&brush, &os->brush);

	points = (RD_POINT *) xmalloc((os->npoints + 1) * sizeof(RD_POINT));
	memset(points, 0, (os->npoints + 1) * sizeof(RD_POINT));

	points[0].x = os->x;
	points[0].y = os->y;

	index = 0;
	data = ((os->npoints - 1) / 4) + 1;
	for (next = 1; (next <= os->npoints) && (next < 256) && (data < os->datasize); next++)
	{
		if ((next - 1) % 4 == 0)
			flags = os->data[index++];

		if (~flags & 0x80)
			points[next].x = parse_delta(os->data, &data);

		if (~flags & 0x40)
			points[next].y = parse_delta(os->data, &data);

		flags <<= 2;
	}

	if (next - 1 == os->npoints)
		ui_polygon(os->opcode - 1, os->fillmode, points, os->npoints + 1,
			   &brush, os->bgcolour, os->fgcolour);
	else
		logger(Graphics, Error, "process_polygon2(), polygon parse error");

	xfree(points);
}

/* Process a polyline order */
static void
process_polyline(STREAM s, POLYLINE_ORDER * os, uint32 present, RD_BOOL delta)
{
	int index, next, data;
	uint8 flags = 0;
	PEN pen;
	RD_POINT *points;

	if (present & 0x01)
		rdp_in_coord(s, &os->x, delta);

	if (present & 0x02)
		rdp_in_coord(s, &os->y, delta);

	if (present & 0x04)
		in_uint8(s, os->opcode);

	if (present & 0x10)
		rdp_in_colour(s, &os->fgcolour);

	if (present & 0x20)
		in_uint8(s, os->lines);

	if (present & 0x40)
	{
		in_uint8(s, os->datasize);
		in_uint8a(s, os->data, os->datasize);
	}

	logger(Graphics, Debug, "process_polyline(), x=%d, y=%d, op=0x%x, fg=0x%x, n=%d, sz=%d)",
	       os->x, os->y, os->opcode, os->fgcolour, os->lines, os->datasize);

	if (os->opcode < 0x01 || os->opcode > 0x10)
	{
		logger(Graphics, Error, "process_polyline(), bad ROP2 0x%x", os->opcode);
		return;
	}

	points = (RD_POINT *) xmalloc((os->lines + 1) * sizeof(RD_POINT));
	memset(points, 0, (os->lines + 1) * sizeof(RD_POINT));

	points[0].x = os->x;
	points[0].y = os->y;
	pen.style = pen.width = 0;
	pen.colour = os->fgcolour;

	index = 0;
	data = ((os->lines - 1) / 4) + 1;
	for (next = 1; (next <= os->lines) && (data < os->datasize); next++)
	{
		if ((next - 1) % 4 == 0)
			flags = os->data[index++];

		if (~flags & 0x80)
			points[next].x = parse_delta(os->data, &data);

		if (~flags & 0x40)
			points[next].y = parse_delta(os->data, &data);

		flags <<= 2;
	}

	if (next - 1 == os->lines)
		ui_polyline(os->opcode - 1, points, os->lines + 1, &pen);
	else
		logger(Graphics, Error, "process_polyline(), parse error");

	xfree(points);
}

/* Process an ellipse order */
static void
process_ellipse(STREAM s, ELLIPSE_ORDER * os, uint32 present, RD_BOOL delta)
{
	if (present & 0x01)
		rdp_in_coord(s, &os->left, delta);

	if (present & 0x02)
		rdp_in_coord(s, &os->top, delta);

	if (present & 0x04)
		rdp_in_coord(s, &os->right, delta);

	if (present & 0x08)
		rdp_in_coord(s, &os->bottom, delta);

	if (present & 0x10)
		in_uint8(s, os->opcode);

	if (present & 0x20)
		in_uint8(s, os->fillmode);

	if (present & 0x40)
		rdp_in_colour(s, &os->fgcolour);

	logger(Graphics, Debug,
	       "process_ellipse(), l=%d, t=%d, r=%d, b=%d, op=0x%x, fm=%d, fg=0x%x", os->left,
	       os->top, os->right, os->bottom, os->opcode, os->fillmode, os->fgcolour);

	ui_ellipse(os->opcode - 1, os->fillmode, os->left, os->top, os->right - os->left,
		   os->bottom - os->top, NULL, 0, os->fgcolour);
}

/* Process an ellipse2 order */
static void
process_ellipse2(STREAM s, ELLIPSE2_ORDER * os, uint32 present, RD_BOOL delta)
{
	BRUSH brush;

	if (present & 0x0001)
		rdp_in_coord(s, &os->left, delta);

	if (present & 0x0002)
		rdp_in_coord(s, &os->top, delta);

	if (present & 0x0004)
		rdp_in_coord(s, &os->right, delta);

	if (present & 0x0008)
		rdp_in_coord(s, &os->bottom, delta);

	if (present & 0x0010)
		in_uint8(s, os->opcode);

	if (present & 0x0020)
		in_uint8(s, os->fillmode);

	if (present & 0x0040)
		rdp_in_colour(s, &os->bgcolour);

	if (present & 0x0080)
		rdp_in_colour(s, &os->fgcolour);

	rdp_parse_brush(s, &os->brush, present >> 8);

	logger(Graphics, Debug,
	       "process_ellipse2(), l=%d, t=%d, r=%d, b=%d, op=0x%x, fm=%d, bs=%d, bg=0x%x, fg=0x%x",
	       os->left, os->top, os->right, os->bottom, os->opcode, os->fillmode, os->brush.style,
	       os->bgcolour, os->fgcolour);

	setup_brush(&brush, &os->brush);

	ui_ellipse(os->opcode - 1, os->fillmode, os->left, os->top, os->right - os->left,
		   os->bottom - os->top, &brush, os->bgcolour, os->fgcolour);
}

/* Process a text order */
static void
process_text2(STREAM s, TEXT2_ORDER * os, uint32 present, RD_BOOL delta)
{
	UNUSED(delta);
	BRUSH brush;

	if (present & 0x000001)
		in_uint8(s, os->font);

	if (present & 0x000002)
		in_uint8(s, os->flags);

	if (present & 0x000004)
		in_uint8(s, os->opcode);

	if (present & 0x000008)
		in_uint8(s, os->mixmode);

	if (present & 0x000010)
		rdp_in_colour(s, &os->fgcolour);

	if (present & 0x000020)
		rdp_in_colour(s, &os->bgcolour);

	if (present & 0x000040)
		in_uint16_le(s, os->clipleft);

	if (present & 0x000080)
		in_uint16_le(s, os->cliptop);

	if (present & 0x000100)
		in_uint16_le(s, os->clipright);

	if (present & 0x000200)
		in_uint16_le(s, os->clipbottom);

	if (present & 0x000400)
		in_uint16_le(s, os->boxleft);

	if (present & 0x000800)
		in_uint16_le(s, os->boxtop);

	if (present & 0x001000)
		in_uint16_le(s, os->boxright);

	if (present & 0x002000)
		in_uint16_le(s, os->boxbottom);

	rdp_parse_brush(s, &os->brush, present >> 14);

	if (present & 0x080000)
		in_uint16_le(s, os->x);

	if (present & 0x100000)
		in_uint16_le(s, os->y);

	if (present & 0x200000)
	{
		in_uint8(s, os->length);
		in_uint8a(s, os->text, os->length);
	}

	logger(Graphics, Debug,
	       "process_text2(), x=%d, y=%d, cl=%d, ct=%d, cr=%d, cb=%d, bl=%d, bt=%d, br=%d, bb=%d, bs=%d, bg=0x%x, fg=0x%x, font=%d, fl=0x%x, op=0x%x, mix=%d, n=%d",
	       os->x, os->y, os->clipleft, os->cliptop, os->clipright, os->clipbottom, os->boxleft,
	       os->boxtop, os->boxright, os->boxbottom, os->brush.style, os->bgcolour, os->fgcolour,
	       os->font, os->flags, os->opcode, os->mixmode, os->length);

	setup_brush(&brush, &os->brush);

	ui_draw_text(os->font, os->flags, os->opcode - 1, os->mixmode, os->x, os->y,
		     os->clipleft, os->cliptop, os->clipright - os->clipleft,
		     os->clipbottom - os->cliptop, os->boxleft, os->boxtop,
		     os->boxright - os->boxleft, os->boxbottom - os->boxtop,
		     &brush, os->bgcolour, os->fgcolour, os->text, os->length);
}

/* Process a raw bitmap cache order */
static void
process_raw_bmpcache(STREAM s)
{
	RD_HBITMAP bitmap;
	uint16 cache_idx, bufsize;
	uint8 cache_id, width, height, bpp, Bpp;
	uint8 *data, *inverted;
	int y;

	in_uint8(s, cache_id);
	in_uint8s(s, 1);	/* pad */
	in_uint8(s, width);
	in_uint8(s, height);
	in_uint8(s, bpp);
	Bpp = (bpp + 7) / 8;
	in_uint16_le(s, bufsize);
	in_uint16_le(s, cache_idx);
	in_uint8p(s, data, bufsize);

	logger(Graphics, Debug, "process_raw_bpmcache(), cx=%d, cy=%d, id=%d, idx=%d", width,
	       height, cache_id, cache_idx);
	inverted = (uint8 *) xmalloc(width * height * Bpp);
	for (y = 0; y < height; y++)
	{
		memcpy(&inverted[(height - y - 1) * (width * Bpp)], &data[y * (width * Bpp)],
		       width * Bpp);
	}

	bitmap = ui_create_bitmap(width, height, inverted);
	xfree(inverted);
	cache_put_bitmap(cache_id, cache_idx, bitmap);
}

/* Process a bitmap cache order */
static void
process_bmpcache(STREAM s)
{
	RD_HBITMAP bitmap;
	uint16 cache_idx, size;
	uint8 cache_id, width, height, bpp, Bpp;
	uint8 *data, *bmpdata;
	uint16 bufsize, pad2, row_size, final_size;
	uint8 pad1;

	pad2 = row_size = final_size = 0xffff;	/* Shut the compiler up */

	in_uint8(s, cache_id);
	in_uint8(s, pad1);	/* pad */
	in_uint8(s, width);
	in_uint8(s, height);
	in_uint8(s, bpp);
	Bpp = (bpp + 7) / 8;
	in_uint16_le(s, bufsize);	/* bufsize */
	in_uint16_le(s, cache_idx);

	if (g_rdp_version >= RDP_V5)
	{
		size = bufsize;
	}
	else
	{

		/* Begin compressedBitmapData */
		in_uint16_le(s, pad2);	/* pad */
		in_uint16_le(s, size);
		/*      in_uint8s(s, 4);  *//* row_size, final_size */
		in_uint16_le(s, row_size);
		in_uint16_le(s, final_size);

	}
	in_uint8p(s, data, size);
	logger(Graphics, Debug,
	       "process_bmpcache(), cx=%d, cy=%d, id=%d, idx=%d, bpp=%d, size=%d, pad1=%d, bufsize=%d, pad2=%d, rs=%d, fs=%d",
	       width, height, cache_id, cache_idx, bpp, size, pad1, bufsize, pad2, row_size,
	       final_size);

	bmpdata = (uint8 *) xmalloc(width * height * Bpp);

	if (bitmap_decompress(bmpdata, width, height, data, size, Bpp))
	{
		bitmap = ui_create_bitmap(width, height, bmpdata);
		cache_put_bitmap(cache_id, cache_idx, bitmap);
	}
	else
	{
		logger(Graphics, Error, "process_bmpcache(), Failed to decompress bitmap data");
	}

	xfree(bmpdata);
}

/* Process a bitmap cache v2 order */
static void
process_bmpcache2(STREAM s, uint16 flags, RD_BOOL compressed)
{
	RD_HBITMAP bitmap;
	int y;
	uint8 cache_id, cache_idx_low, width, height, Bpp;
	uint16 cache_idx, bufsize;
	uint8 *data, *bmpdata, *bitmap_id;

	bitmap_id = NULL;	/* prevent compiler warning */
	cache_id = flags & ID_MASK;
	Bpp = ((flags & MODE_MASK) >> MODE_SHIFT) - 2;

	if (flags & PERSIST)
	{
		in_uint8p(s, bitmap_id, 8);
	}

	if (flags & SQUARE)
	{
		in_uint8(s, width);
		height = width;
	}
	else
	{
		in_uint8(s, width);
		in_uint8(s, height);
	}

	in_uint16_be(s, bufsize);
	bufsize &= BUFSIZE_MASK;
	in_uint8(s, cache_idx);

	if (cache_idx & LONG_FORMAT)
	{
		in_uint8(s, cache_idx_low);
		cache_idx = ((cache_idx ^ LONG_FORMAT) << 8) + cache_idx_low;
	}

	in_uint8p(s, data, bufsize);

	logger(Graphics, Debug,
	       "process_bmpcache2(), compr=%d, flags=%x, cx=%d, cy=%d, id=%d, idx=%d, Bpp=%d, bs=%d",
	       compressed, flags, width, height, cache_id, cache_idx, Bpp, bufsize);

	bmpdata = (uint8 *) xmalloc(width * height * Bpp);

	if (compressed)
	{
		if (!bitmap_decompress(bmpdata, width, height, data, bufsize, Bpp))
		{
			logger(Graphics, Error,
			       "process_bmpcache2(), failed to decompress bitmap data");
			xfree(bmpdata);
			return;
		}
	}
	else
	{
		for (y = 0; y < height; y++)
			memcpy(&bmpdata[(height - y - 1) * (width * Bpp)],
			       &data[y * (width * Bpp)], width * Bpp);
	}

	bitmap = ui_create_bitmap(width, height, bmpdata);

	if (bitmap)
	{
		cache_put_bitmap(cache_id, cache_idx, bitmap);
		if (flags & PERSIST)
			pstcache_save_bitmap(cache_id, cache_idx, bitmap_id, width, height,
					     width * height * Bpp, bmpdata);
	}
	else
	{
		logger(Graphics, Error, "process_bmpcache2(), ui_create_bitmap(), failed");
	}

	xfree(bmpdata);
}

/* Process a colourmap cache order */
static void
process_colcache(STREAM s)
{
	COLOURENTRY *entry;
	COLOURMAP map;
	RD_HCOLOURMAP hmap;
	uint8 cache_id;
	int i;

	in_uint8(s, cache_id);
	in_uint16_le(s, map.ncolours);

	map.colours = (COLOURENTRY *) xmalloc(sizeof(COLOURENTRY) * map.ncolours);

	for (i = 0; i < map.ncolours; i++)
	{
		entry = &map.colours[i];
		in_uint8(s, entry->blue);
		in_uint8(s, entry->green);
		in_uint8(s, entry->red);
		in_uint8s(s, 1);	/* pad */
	}

	logger(Graphics, Debug, "process_colcache(), id=%d, n=%d", cache_id, map.ncolours);

	hmap = ui_create_colourmap(&map);

	if (cache_id)
		ui_set_colourmap(hmap);

	xfree(map.colours);
}

/* Process a font cache order */
static void
process_fontcache(STREAM s)
{
	RD_HGLYPH bitmap;
	uint8 font, nglyphs;
	uint16 character, offset, baseline, width, height;
	int i, datasize;
	uint8 *data;

	in_uint8(s, font);
	in_uint8(s, nglyphs);

	logger(Graphics, Debug, "process_fontcache(), font=%d, n=%d", font, nglyphs);

	for (i = 0; i < nglyphs; i++)
	{
		in_uint16_le(s, character);
		in_uint16_le(s, offset);
		in_uint16_le(s, baseline);
		in_uint16_le(s, width);
		in_uint16_le(s, height);

		datasize = (height * ((width + 7) / 8) + 3) & ~3;
		in_uint8p(s, data, datasize);

		bitmap = ui_create_glyph(width, height, data);
		cache_put_font(font, character, offset, baseline, width, height, bitmap);
	}
}

static void
process_compressed_8x8_brush_data(uint8 * in, uint8 * out, int Bpp)
{
	int x, y, pal_index, in_index, shift, do2, i;
	uint8 *pal;

	in_index = 0;
	pal = in + 16;
	/* read it bottom up */
	for (y = 7; y >= 0; y--)
	{
		/* 2 bytes per row */
		x = 0;
		for (do2 = 0; do2 < 2; do2++)
		{
			/* 4 pixels per byte */
			shift = 6;
			while (shift >= 0)
			{
				pal_index = (in[in_index] >> shift) & 3;
				/* size of palette entries depends on Bpp */
				for (i = 0; i < Bpp; i++)
				{
					out[(y * 8 + x) * Bpp + i] = pal[pal_index * Bpp + i];
				}
				x++;
				shift -= 2;
			}
			in_index++;
		}
	}
}

/* Process a brush cache order */
static void
process_brushcache(STREAM s, uint16 flags)
{
	UNUSED(flags);
	BRUSHDATA brush_data;
	uint8 cache_idx, colour_code, width, height, size, type;
	uint8 *comp_brush;
	int index;
	int Bpp;

	in_uint8(s, cache_idx);
	in_uint8(s, colour_code);
	in_uint8(s, width);
	in_uint8(s, height);
	in_uint8(s, type);	/* type, 0x8x = cached */
	in_uint8(s, size);

	logger(Graphics, Debug, "process_brushcache(), idx=%d, wd=%d, ht=%d, type=0x%x sz=%d",
	       cache_idx, width, height, type, size);

	if ((width == 8) && (height == 8))
	{
		if (colour_code == 1)
		{
			brush_data.colour_code = 1;
			brush_data.data_size = 8;
			brush_data.data = xmalloc(8);
			if (size == 8)
			{
				/* read it bottom up */
				for (index = 7; index >= 0; index--)
				{
					in_uint8(s, brush_data.data[index]);
				}
			}
			else
			{
				logger(Graphics, Warning,
				       "process_brushcache(), incompatible brush, colour_code %d size %d",
				       colour_code, size);
			}
			cache_put_brush_data(1, cache_idx, &brush_data);
		}
		else if ((colour_code >= 3) && (colour_code <= 6))
		{
			Bpp = colour_code - 2;
			brush_data.colour_code = colour_code;
			brush_data.data_size = 8 * 8 * Bpp;
			brush_data.data = xmalloc(8 * 8 * Bpp);
			if (size == 16 + 4 * Bpp)
			{
				in_uint8p(s, comp_brush, 16 + 4 * Bpp);
				process_compressed_8x8_brush_data(comp_brush, brush_data.data, Bpp);
			}
			else
			{
				in_uint8a(s, brush_data.data, 8 * 8 * Bpp);
			}
			cache_put_brush_data(colour_code, cache_idx, &brush_data);
		}
		else
		{
			logger(Graphics, Warning,
			       "process_brushcache(), incompatible brush, colour_code %d size %d",
			       colour_code, size);
		}
	}
	else
	{
		logger(Graphics, Warning,
		       "process_brushcache(), incompatible brush, width height %d %d", width,
		       height);
	}
}

/* Process a secondary order */
static void
process_secondary_order(STREAM s)
{
	/* The length isn't calculated correctly by the server.
	 * For very compact orders the length becomes negative
	 * so a signed integer must be used. */
	uint16 length;
	uint16 flags;
	uint8 type;
	uint8 *next_order;

	in_uint16_le(s, length);
	in_uint16_le(s, flags);	/* used by bmpcache2 */
	in_uint8(s, type);

	next_order = s->p + (sint16) length + 7;

	switch (type)
	{
		case RDP_ORDER_RAW_BMPCACHE:
			process_raw_bmpcache(s);
			break;

		case RDP_ORDER_COLCACHE:
			process_colcache(s);
			break;

		case RDP_ORDER_BMPCACHE:
			process_bmpcache(s);
			break;

		case RDP_ORDER_FONTCACHE:
			process_fontcache(s);
			break;

		case RDP_ORDER_RAW_BMPCACHE2:
			process_bmpcache2(s, flags, False);	/* uncompressed */
			break;

		case RDP_ORDER_BMPCACHE2:
			process_bmpcache2(s, flags, True);	/* compressed */
			break;

		case RDP_ORDER_BRUSHCACHE:
			process_brushcache(s, flags);
			break;

		default:
			logger(Graphics, Warning,
			       "process_secondary_order(), unhandled secondary order %d", type);
	}

	s->p = next_order;
}

static void
process_primary_order(STREAM s, uint8 order_flags)
{
	RDP_ORDER_STATE *os = &g_order_state;
	uint32 present;
	int size;
	RD_BOOL delta;

	if (order_flags & TS_TYPE_CHANGE)
	{
		in_uint8(s, os->order_type);
	}

	switch (os->order_type)
	{
		case TS_ENC_MEM3BLT_ORDER:
		case TS_ENC_INDEX_ORDER:
			size = 3;
			break;

		case TS_ENC_PATBLT_ORDER:
		case TS_ENC_MEMBLT_ORDER:
		case TS_ENC_LINETO_ORDER:
		case TS_ENC_POLYGON_CB_ORDER:
		case TS_ENC_ELLIPSE_CB_ORDER:
			size = 2;
			break;

		default:
			size = 1;
	}

	rdp_in_present(s, &present, order_flags, size);

	if (order_flags & TS_BOUNDS)
	{
		if (!(order_flags & TS_ZERO_BOUNDS_DELTAS))
			rdp_parse_bounds(s, &os->bounds);

		ui_set_clip(os->bounds.left,
			    os->bounds.top,
			    os->bounds.right -
			    os->bounds.left + 1,
			    os->bounds.bottom - os->bounds.top + 1);
	}

	delta = order_flags & TS_DELTA_COORDINATES;

	switch (os->order_type)
	{
		case TS_ENC_DSTBLT_ORDER:
			process_destblt(s, &os->destblt, present, delta);
			break;

		case TS_ENC_PATBLT_ORDER:
			process_patblt(s, &os->patblt, present, delta);
			break;

		case TS_ENC_SCRBLT_ORDER:
			process_screenblt(s, &os->screenblt, present, delta);
			break;

		case TS_ENC_LINETO_ORDER:
			process_line(s, &os->line, present, delta);
			break;

		case TS_ENC_OPAQUERECT_ORDER:
			process_rect(s, &os->rect, present, delta);
			break;

		case TS_ENC_SAVEBITMAP_ORDER:
			process_desksave(s, &os->desksave, present, delta);
			break;

		case TS_ENC_MEMBLT_ORDER:
			process_memblt(s, &os->memblt, present, delta);
			break;

		case TS_ENC_MEM3BLT_ORDER:
			process_triblt(s, &os->triblt, present, delta);
			break;

		case TS_ENC_POLYGON_SC_ORDER:
			process_polygon(s, &os->polygon, present, delta);
			break;

		case TS_ENC_POLYGON_CB_ORDER:
			process_polygon2(s, &os->polygon2, present, delta);
			break;

		case TS_ENC_POLYLINE_ORDER:
			process_polyline(s, &os->polyline, present, delta);
			break;

		case TS_ENC_ELLIPSE_SC_ORDER:
			process_ellipse(s, &os->ellipse, present, delta);
			break;

		case TS_ENC_ELLIPSE_CB_ORDER:
			process_ellipse2(s, &os->ellipse2, present, delta);
			break;

		case TS_ENC_INDEX_ORDER:
			process_text2(s, &os->text2, present, delta);
			break;

		default:
			logger(Graphics, Warning,
			       "process_primary_orders(), unhandled order type %d",
			       os->order_type);
			return;
	}

	if (order_flags & TS_BOUNDS)
		ui_reset_clip();
}

/* Process an order PDU */
void
process_orders(STREAM s, uint16 num_orders)
{
	uint8 order_flags;
	uint16 processed = 0;


	while (processed < num_orders)
	{
		in_uint8(s, order_flags);

		/* order_flags can hold more data than just the type, so we
		   only want to look at the TS_STANDARD and TS_SECONDARY
		   bits. */
		switch (order_flags & (TS_STANDARD | TS_SECONDARY))
		{
			case TS_STANDARD:
				process_primary_order(s, order_flags);
				break;
			case (TS_STANDARD | TS_SECONDARY):
				process_secondary_order(s);
				break;
			default:
				/* FIXME: alternate secondary drawing orders */
				logger(Graphics, Warning,
				       "process_orders(), Unhandled order type 0x%x",
				       order_flags);
		}
		processed++;
	}
#if 0
	/* not true when RDP_COMPRESSION is set */
	if (s->p != g_next_packet)
		logger(Graphics, Error, "process_orders(), %d bytes remaining",
		       (int) (g_next_packet - s->p));
#endif

}

/* Reset order state */
void
reset_order_state(void)
{
	memset(&g_order_state, 0, sizeof(g_order_state));
	g_order_state.order_type = TS_ENC_PATBLT_ORDER;
}
