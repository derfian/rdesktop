/* -*- c-basic-offset: 8 -*-
   rdesktop: A Remote Desktop Protocol client.
   Protocol services - Clipboard functions
   Copyright 2003 Erik Forsberg <forsberg@cendio.se> for Cendio AB
   Copyright (C) Matthew Chapman <matthewc.unsw.edu.au> 2003-2008
   Copyright 2017 Henrik Andersson <hean01@cendio.se> for Cendio AB

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

#define CLIPRDR_CONNECT			1
#define CLIPRDR_FORMAT_ANNOUNCE		2
#define CLIPRDR_FORMAT_ACK		3
#define CLIPRDR_DATA_REQUEST		4
#define CLIPRDR_DATA_RESPONSE		5

#define CLIPRDR_REQUEST			0
#define CLIPRDR_RESPONSE		1
#define CLIPRDR_ERROR			2

static VCHANNEL *cliprdr_channel;

static uint8_t *last_formats = NULL;
static uint32_t last_formats_length = 0;

static void
cliprdr_send_packet(uint16_t type, uint16_t status, uint8_t * data, uint32_t length)
{
	STREAM s;

	logger(Clipboard, Debug, "cliprdr_send_packet(), type=%d, status=%d, length=%d", type,
	       status, length);

	s = channel_init(cliprdr_channel, length + 12);
	out_uint16_le(s, type);
	out_uint16_le(s, status);
	out_uint32_le(s, length);
	out_uint8p(s, data, length);
	out_uint32(s, 0);	/* pad? */
	s_mark_end(s);
	channel_send(s, cliprdr_channel);
}

/* Helper which announces our readiness to supply clipboard data
   in a single format (such as CF_TEXT) to the RDP side.
   To announce more than one format at a time, use
   cliprdr_send_native_format_announce.
 */
void
cliprdr_send_simple_native_format_announce(uint32_t format)
{
	uint8_t buffer[36];

	logger(Clipboard, Debug, "cliprdr_send_simple_native_format_announce() format 0x%x",
	       format);

	buf_out_uint32(buffer, format);
	memset(buffer + 4, 0, sizeof(buffer) - 4);	/* description */
	cliprdr_send_native_format_announce(buffer, sizeof(buffer));
}

/* Announces our readiness to supply clipboard data in multiple
   formats, each denoted by a 36-byte format descriptor of
   [ uint32_t format + 32-byte description ].
 */
void
cliprdr_send_native_format_announce(uint8_t * formats_data, uint32_t formats_data_length)
{
	logger(Clipboard, Debug, "cliprdr_send_native_format_announce()");

	cliprdr_send_packet(CLIPRDR_FORMAT_ANNOUNCE, CLIPRDR_REQUEST, formats_data,
			    formats_data_length);

	if (formats_data != last_formats)
	{
		if (last_formats)
			xfree(last_formats);

		last_formats = xmalloc(formats_data_length);
		memcpy(last_formats, formats_data, formats_data_length);
		last_formats_length = formats_data_length;
	}
}

void
cliprdr_send_data_request(uint32_t format)
{
	uint8_t buffer[4];

	logger(Clipboard, Debug, "cliprdr_send_data_request(), format 0x%x", format);
	buf_out_uint32(buffer, format);
	cliprdr_send_packet(CLIPRDR_DATA_REQUEST, CLIPRDR_REQUEST, buffer, sizeof(buffer));
}

void
cliprdr_send_data(uint8_t * data, uint32_t length)
{
	logger(Clipboard, Debug, "cliprdr_send_data(), length %d bytes", length);
	cliprdr_send_packet(CLIPRDR_DATA_RESPONSE, CLIPRDR_RESPONSE, data, length);
}

static void
cliprdr_process(STREAM s)
{
	uint16_t type, status;
	uint32_t length, format;
	uint8_t *data;

	in_uint16_le(s, type);
	in_uint16_le(s, status);
	in_uint32_le(s, length);
	data = s->p;

	logger(Clipboard, Debug, "cliprdr_process(), type=%d, status=%d, length=%d", type, status,
	       length);

	if (status == CLIPRDR_ERROR)
	{
		switch (type)
		{
			case CLIPRDR_FORMAT_ACK:
				/* FIXME: We seem to get this when we send an announce while the server is
				   still processing a paste. Try sending another announce. */
				cliprdr_send_native_format_announce(last_formats,
								    last_formats_length);
				break;
			case CLIPRDR_DATA_RESPONSE:
				ui_clip_request_failed();
				break;
			default:
				logger(Clipboard, Warning,
				       "cliprdr_process(), unhandled error (type=%d)", type);
		}

		return;
	}

	switch (type)
	{
		case CLIPRDR_CONNECT:
			ui_clip_sync();
			break;
		case CLIPRDR_FORMAT_ANNOUNCE:
			ui_clip_format_announce(data, length);
			cliprdr_send_packet(CLIPRDR_FORMAT_ACK, CLIPRDR_RESPONSE, NULL, 0);
			return;
		case CLIPRDR_FORMAT_ACK:
			break;
		case CLIPRDR_DATA_REQUEST:
			in_uint32_le(s, format);
			ui_clip_request_data(format);
			break;
		case CLIPRDR_DATA_RESPONSE:
			ui_clip_handle_data(data, length);
			break;
		case 7:	/* TODO: W2K3 SP1 sends this on connect with a value of 1 */
			break;
		default:
			logger(Clipboard, Warning, "cliprdr_process(), unhandled packet type %d",
			       type);
	}
}

void
cliprdr_set_mode(const char *optarg)
{
	ui_clip_set_mode(optarg);
}

RD_BOOL
cliprdr_init(void)
{
	cliprdr_channel =
		channel_register("cliprdr",
				 CHANNEL_OPTION_INITIALIZED | CHANNEL_OPTION_ENCRYPT_RDP |
				 CHANNEL_OPTION_COMPRESS_RDP | CHANNEL_OPTION_SHOW_PROTOCOL,
				 cliprdr_process);
	return (cliprdr_channel != NULL);
}
