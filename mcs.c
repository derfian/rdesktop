/* -*- c-basic-offset: 8 -*-
   rdesktop: A Remote Desktop Protocol client.
   Protocol services - Multipoint Communications Service
   Copyright (C) Matthew Chapman <matthewc.unsw.edu.au> 1999-2008
   Copyright 2005-2011 Peter Astrand <astrand@cendio.se> for Cendio AB

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

uint16_t g_mcs_userid;
extern VCHANNEL g_channels[];
extern unsigned int g_num_channels;


/* Output a DOMAIN_PARAMS structure (ASN.1 BER) */
static void
mcs_out_domain_params(STREAM s, int max_channels, int max_users, int max_tokens, int max_pdusize)
{
	ber_out_header(s, MCS_TAG_DOMAIN_PARAMS, 32);
	ber_out_integer(s, max_channels);
	ber_out_integer(s, max_users);
	ber_out_integer(s, max_tokens);
	ber_out_integer(s, 1);	/* num_priorities */
	ber_out_integer(s, 0);	/* min_throughput */
	ber_out_integer(s, 1);	/* max_height */
	ber_out_integer(s, max_pdusize);
	ber_out_integer(s, 2);	/* ver_protocol */
}

/* Parse a DOMAIN_PARAMS structure (ASN.1 BER) */
static bool
mcs_parse_domain_params(STREAM s)
{
	int length;

	ber_parse_header(s, MCS_TAG_DOMAIN_PARAMS, &length);
	in_uint8s(s, length);

	return s_check(s);
}

/* Send an MCS_CONNECT_INITIAL message (ASN.1 BER) */
static void
mcs_send_connect_initial(STREAM mcs_data)
{
	int datalen = mcs_data->end - mcs_data->data;
	int length = 9 + 3 * 34 + 4 + datalen;
	STREAM s;

	s = iso_init(length + 5);

	ber_out_header(s, MCS_CONNECT_INITIAL, length);
	ber_out_header(s, BER_TAG_OCTET_STRING, 1);	/* calling domain */
	out_uint8(s, 1);
	ber_out_header(s, BER_TAG_OCTET_STRING, 1);	/* called domain */
	out_uint8(s, 1);

	ber_out_header(s, BER_TAG_BOOLEAN, 1);
	out_uint8(s, 0xff);	/* upward flag */

	mcs_out_domain_params(s, 34, 2, 0, 0xffff);	/* target params */
	mcs_out_domain_params(s, 1, 1, 1, 0x420);	/* min params */
	mcs_out_domain_params(s, 0xffff, 0xfc17, 0xffff, 0xffff);	/* max params */

	ber_out_header(s, BER_TAG_OCTET_STRING, datalen);
	out_uint8p(s, mcs_data->data, datalen);

	s_mark_end(s);
	iso_send(s);
}

/* Expect a MCS_CONNECT_RESPONSE message (ASN.1 BER) */
static bool
mcs_recv_connect_response(STREAM mcs_data)
{
	UNUSED(mcs_data);
	uint8_t result;
	int length;
	STREAM s;

	s = iso_recv(NULL);
	if (s == NULL)
		return false;

	ber_parse_header(s, MCS_CONNECT_RESPONSE, &length);

	ber_parse_header(s, BER_TAG_RESULT, &length);
	in_uint8(s, result);
	if (result != 0)
	{
		logger(Protocol, Error, "mcs_recv_connect_response(), result=%d", result);
		return false;
	}

	ber_parse_header(s, BER_TAG_INTEGER, &length);
	in_uint8s(s, length);	/* connect id */
	mcs_parse_domain_params(s);

	ber_parse_header(s, BER_TAG_OCTET_STRING, &length);

	sec_process_mcs_data(s);
	/*
	   if (length > mcs_data->size)
	   {
	   logger(Protocol, Error, "mcs_recv_connect_response(), expected length=%d, got %d",length, mcs_data->size);
	   length = mcs_data->size;
	   }

	   in_uint8a(s, mcs_data->data, length);
	   mcs_data->p = mcs_data->data;
	   mcs_data->end = mcs_data->data + length;
	 */
	return s_check_end(s);
}

/* Send an EDrq message (ASN.1 PER) */
static void
mcs_send_edrq(void)
{
	STREAM s;

	s = iso_init(5);

	out_uint8(s, (MCS_EDRQ << 2));
	out_uint16_be(s, 1);	/* height */
	out_uint16_be(s, 1);	/* interval */

	s_mark_end(s);
	iso_send(s);
}

/* Send an AUrq message (ASN.1 PER) */
static void
mcs_send_aurq(void)
{
	STREAM s;

	s = iso_init(1);

	out_uint8(s, (MCS_AURQ << 2));

	s_mark_end(s);
	iso_send(s);
}

/* Expect a AUcf message (ASN.1 PER) */
static bool
mcs_recv_aucf(uint16_t * mcs_userid)
{
	uint8_t opcode, result;
	STREAM s;

	s = iso_recv(NULL);
	if (s == NULL)
		return false;

	in_uint8(s, opcode);
	if ((opcode >> 2) != MCS_AUCF)
	{
		logger(Protocol, Error, "mcs_recv_aucf(), expected opcode AUcf, got %d", opcode);
		return false;
	}

	in_uint8(s, result);
	if (result != 0)
	{
		logger(Protocol, Error, "mcs_recv_aucf(), expected result 0, got %d", result);
		return false;
	}

	if (opcode & 2)
		in_uint16_be(s, *mcs_userid);

	return s_check_end(s);
}

/* Send a CJrq message (ASN.1 PER) */
static void
mcs_send_cjrq(uint16_t chanid)
{
	STREAM s;

	logger(Protocol, Debug, "mcs_send_cjrq(), chanid=%d", chanid);

	s = iso_init(5);

	out_uint8(s, (MCS_CJRQ << 2));
	out_uint16_be(s, g_mcs_userid);
	out_uint16_be(s, chanid);

	s_mark_end(s);
	iso_send(s);
}

/* Expect a CJcf message (ASN.1 PER) */
static bool
mcs_recv_cjcf(void)
{
	uint8_t opcode, result;
	STREAM s;

	s = iso_recv(NULL);
	if (s == NULL)
		return false;

	in_uint8(s, opcode);
	if ((opcode >> 2) != MCS_CJCF)
	{
		logger(Protocol, Error, "mcs_recv_cjcf(), expected opcode CJcf, got %d", opcode);
		return false;
	}

	in_uint8(s, result);
	if (result != 0)
	{
		logger(Protocol, Error, "mcs_recv_cjcf(), expected result 0, got %d", result);
		return false;
	}

	in_uint8s(s, 4);	/* mcs_userid, req_chanid */
	if (opcode & 2)
		in_uint8s(s, 2);	/* join_chanid */

	return s_check_end(s);
}

/* Initialise an MCS transport data packet */
STREAM
mcs_init(int length)
{
	STREAM s;

	s = iso_init(length + 8);
	s_push_layer(s, mcs_hdr, 8);

	return s;
}

/* Send an MCS transport data packet to a specific channel */
void
mcs_send_to_channel(STREAM s, uint16_t channel)
{
	uint16_t length;

	s_pop_layer(s, mcs_hdr);
	length = s->end - s->p - 8;
	length |= 0x8000;

	out_uint8(s, (MCS_SDRQ << 2));
	out_uint16_be(s, g_mcs_userid);
	out_uint16_be(s, channel);
	out_uint8(s, 0x70);	/* flags */
	out_uint16_be(s, length);

	iso_send(s);
}

/* Send an MCS transport data packet to the global channel */
void
mcs_send(STREAM s)
{
	mcs_send_to_channel(s, MCS_GLOBAL_CHANNEL);
}

/* Receive an MCS transport data packet */
STREAM
mcs_recv(uint16_t * channel, uint8_t * rdpver)
{
	uint8_t opcode, appid, length;
	STREAM s;

	s = iso_recv(rdpver);
	if (s == NULL)
		return NULL;
	if (rdpver != NULL)
		if (*rdpver != 3)
			return s;
	in_uint8(s, opcode);
	appid = opcode >> 2;
	if (appid != MCS_SDIN)
	{
		if (appid != MCS_DPUM)
		{
			logger(Protocol, Error, "mcs_recv(), expected data, got %d", opcode);
		}
		return NULL;
	}
	in_uint8s(s, 2);	/* userid */
	in_uint16_be(s, *channel);
	in_uint8s(s, 1);	/* flags */
	in_uint8(s, length);
	if (length & 0x80)
		in_uint8s(s, 1);	/* second byte of length */
	return s;
}

bool
mcs_connect_start(char *server, char *username, char *domain, char *password,
		  bool reconnect, uint32_t * selected_protocol)
{
	return iso_connect(server, username, domain, password, reconnect, selected_protocol);
}

bool
mcs_connect_finalize(STREAM mcs_data)
{
	unsigned int i;

	mcs_send_connect_initial(mcs_data);
	if (!mcs_recv_connect_response(mcs_data))
		goto error;

	mcs_send_edrq();

	mcs_send_aurq();
	if (!mcs_recv_aucf(&g_mcs_userid))
		goto error;

	mcs_send_cjrq(g_mcs_userid + MCS_USERCHANNEL_BASE);

	if (!mcs_recv_cjcf())
		goto error;

	mcs_send_cjrq(MCS_GLOBAL_CHANNEL);
	if (!mcs_recv_cjcf())
		goto error;

	for (i = 0; i < g_num_channels; i++)
	{
		mcs_send_cjrq(g_channels[i].mcs_id);
		if (!mcs_recv_cjcf())
			goto error;
	}
	return true;

      error:
	iso_disconnect();
	return false;
}

/* Disconnect from the MCS layer */
void
mcs_disconnect(void)
{
	iso_disconnect();
}

/* reset the state of the mcs layer */
void
mcs_reset_state(void)
{
	g_mcs_userid = 0;
	iso_reset_state();
}
