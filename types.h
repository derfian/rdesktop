/*
   rdesktop: A Remote Desktop Protocol client.
   Common data types
   Copyright (C) Matthew Chapman 1999-2008
   Copyright 2014 Henrik Andersson <hean01@cendio.se> for Cendio AB
   Copyright 2017 Karl Mikaelsson <derfian@cendio.se> for Cendio AB

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

#ifndef _TYPES_H
#define _TYPES_H

#include <inttypes.h>

#include "constants.h"
#include "stream.h"

typedef int RD_BOOL;

#ifndef True
#define True  (1)
#define False (0)
#endif

#ifndef PATH_MAX
#define PATH_MAX 256
#endif

typedef void *RD_HBITMAP;
typedef void *RD_HGLYPH;
typedef void *RD_HCOLOURMAP;
typedef void *RD_HCURSOR;


typedef enum _RDP_VERSION
{
	RDP_V4 = 4,
	RDP_V5 = 5,
	RDP_V6 = 6
} RDP_VERSION;


typedef struct _RD_POINT
{
	int16_t x, y;
}
RD_POINT;

typedef struct _COLOURENTRY
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;

}
COLOURENTRY;

typedef struct _COLOURMAP
{
	uint16_t ncolours;
	COLOURENTRY *colours;

}
COLOURMAP;

typedef struct _BOUNDS
{
	int16_t left;
	int16_t top;
	int16_t right;
	int16_t bottom;

}
BOUNDS;

typedef struct _PEN
{
	uint8_t style;
	uint8_t width;
	uint32_t colour;

}
PEN;

/* this is whats in the brush cache */
typedef struct _BRUSHDATA
{
	uint32_t colour_code;
	uint32_t data_size;
	uint8_t *data;
}
BRUSHDATA;

typedef struct _BRUSH
{
	uint8_t xorigin;
	uint8_t yorigin;
	uint8_t style;
	uint8_t pattern[8];
	BRUSHDATA *bd;
}
BRUSH;

typedef struct _FONTGLYPH
{
	int16_t offset;
	int16_t baseline;
	uint16_t width;
	uint16_t height;
	RD_HBITMAP pixmap;

}
FONTGLYPH;

typedef struct _DATABLOB
{
	void *data;
	int size;

}
DATABLOB;

typedef struct _key_translation
{
	/* For normal scancode translations */
	uint8_t scancode;
	uint16_t modifiers;
	/* For sequences. If keysym is nonzero, the fields above are not used. */
	uint32_t seq_keysym;	/* Really KeySym */
	struct _key_translation *next;
}
key_translation;

typedef struct _key_translation_entry
{
	key_translation *tr;
	/* The full KeySym for this entry, not KEYMAP_MASKed */
	uint32_t keysym;
	/* This will be non-NULL if there has been a hash collision */
	struct _key_translation_entry *next;
}
key_translation_entry;

typedef struct _VCHANNEL
{
	uint16_t mcs_id;
	char name[8];
	uint32_t flags;
	struct stream in;
	void (*process) (STREAM);
}
VCHANNEL;

/* PSTCACHE */
typedef uint8_t HASH_KEY[8];

/* Header for an entry in the persistent bitmap cache file */
typedef struct _PSTCACHE_CELLHEADER
{
	HASH_KEY key;
	uint8_t width, height;
	uint16_t length;
	uint32_t stamp;
}
CELLHEADER;

#define MAX_CBSIZE 256

/* RDPSND */
typedef struct _RD_WAVEFORMATEX
{
	uint16_t wFormatTag;
	uint16_t nChannels;
	uint32_t nSamplesPerSec;
	uint32_t nAvgBytesPerSec;
	uint16_t nBlockAlign;
	uint16_t wBitsPerSample;
	uint16_t cbSize;
	uint8_t cb[MAX_CBSIZE];
} RD_WAVEFORMATEX;

typedef struct _RDPCOMP
{
	uint32_t roff;
	uint8_t hist[RDP_MPPC_DICT_SIZE];
	struct stream ns;
}
RDPCOMP;

/* RDPDR */
typedef uint32_t RD_NTSTATUS;
typedef uint32_t RD_NTHANDLE;

typedef struct _DEVICE_FNS
{
	RD_NTSTATUS(*create) (uint32_t device, uint32_t desired_access, uint32_t share_mode,
			      uint32_t create_disposition, uint32_t flags_and_attributes,
			      char *filename, RD_NTHANDLE * handle);
	RD_NTSTATUS(*close) (RD_NTHANDLE handle);
	RD_NTSTATUS(*read) (RD_NTHANDLE handle, uint8_t * data, uint32_t length, uint32_t offset,
			    uint32_t * result);
	RD_NTSTATUS(*write) (RD_NTHANDLE handle, uint8_t * data, uint32_t length, uint32_t offset,
			     uint32_t * result);
	RD_NTSTATUS(*device_control) (RD_NTHANDLE handle, uint32_t request, STREAM in, STREAM out);
}
DEVICE_FNS;


typedef struct rdpdr_device_info
{
	uint32_t device_type;
	RD_NTHANDLE handle;
	char name[8];
	char *local_path;
	void *pdevice_data;
}
RDPDR_DEVICE;

typedef struct rdpdr_disk_device_info
{
	char name[PATH_MAX];
}
DISK_DEVICE;

typedef struct rdpdr_serial_device_info
{
	int dtr;
	int rts;
	uint32_t control, xonoff, onlimit, offlimit;
	uint32_t baud_rate,
		queue_in_size,
		queue_out_size,
		wait_mask,
		read_interval_timeout,
		read_total_timeout_multiplier,
		read_total_timeout_constant,
		write_total_timeout_multiplier, write_total_timeout_constant, posix_wait_mask;
	uint8_t stop_bits, parity, word_length;
	uint8_t chars[6];
	struct termios *ptermios, *pold_termios;
	int event_txempty, event_cts, event_dsr, event_rlsd, event_pending;
}
SERIAL_DEVICE;

typedef struct rdpdr_parallel_device_info
{
	char *driver, *printer;
	uint32_t queue_in_size,
		queue_out_size,
		wait_mask,
		read_interval_timeout,
		read_total_timeout_multiplier,
		read_total_timeout_constant,
		write_total_timeout_multiplier,
		write_total_timeout_constant, posix_wait_mask, bloblen;
	uint8_t *blob;
}
PARALLEL_DEVICE;

typedef struct rdpdr_printer_info
{
	FILE *printer_fp;
	char *driver, *printer;
	uint32_t bloblen;
	uint8_t *blob;
	RD_BOOL default_printer;
}
PRINTER;

typedef struct notify_data
{
	time_t modify_time;
	time_t status_time;
	time_t total_time;
	unsigned int num_entries;
}
NOTIFY;

typedef struct fileinfo
{
	uint32_t device_id, flags_and_attributes, accessmask;
	char path[PATH_MAX];
	DIR *pdir;
	struct dirent *pdirent;
	char pattern[PATH_MAX];
	RD_BOOL delete_on_close;
	NOTIFY notify;
	uint32_t info_class;
}
FILEINFO;

typedef RD_BOOL(*str_handle_lines_t) (const char *line, void *data);

#endif /* _TYPES_H */
