/*
   rdesktop: A Remote Desktop Protocol client.
   Sound infrastructure
   Copyright (C) Michael Gernoth 2006-2008

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

struct audio_packet
{
	struct stream s;
	uint16_t tick;
	uint8_t index;

	struct timeval arrive_tv;
	struct timeval completion_tv;
};

struct audio_driver
{
	void (*add_fds) (int *n, fd_set * rfds, fd_set * wfds, struct timeval * tv);
	void (*check_fds) (fd_set * rfds, fd_set * wfds);

	  bool(*wave_out_open) (void);
	void (*wave_out_close) (void);
	  bool(*wave_out_format_supported) (RD_WAVEFORMATEX * pwfx);
	  bool(*wave_out_set_format) (RD_WAVEFORMATEX * pwfx);
	void (*wave_out_volume) (uint16_t left, uint16_t right);

	  bool(*wave_in_open) (void);
	void (*wave_in_close) (void);
	  bool(*wave_in_format_supported) (RD_WAVEFORMATEX * pwfx);
	  bool(*wave_in_set_format) (RD_WAVEFORMATEX * pwfx);
	void (*wave_in_volume) (uint16_t left, uint16_t right);

	char *name;
	char *description;
	int need_byteswap_on_be;
	int need_resampling;
	struct audio_driver *next;
};

/* Driver register functions */
struct audio_driver *alsa_register(char *options);
struct audio_driver *libao_register(char *options);
struct audio_driver *oss_register(char *options);
struct audio_driver *sgi_register(char *options);
struct audio_driver *sun_register(char *options);
