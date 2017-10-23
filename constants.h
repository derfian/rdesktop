/*
   rdesktop: A Remote Desktop Protocol client.
   Miscellaneous protocol constants
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

/* TCP port for Remote Desktop Protocol */
#define TCP_PORT_RDP 3389

#define DEFAULT_CODEPAGE	"UTF-8"
#define WINDOWS_CODEPAGE	"UTF-16LE"

/* T-REC-T.123-200701, section 8 */
#define T123_HEADER_VERSION 0x3

/* [MS-RDPBCGR] 2.2.9.1.2 */
#define FASTPATH_OUTPUT_ACTION_FASTPATH	0x0
#define FASTPATH_OUTPUT_ACTION_X224	T123_HEADER_VERSION

#define FASTPATH_OUTPUT_SECURE_CHECKSUM 0x1
#define FASTPATH_OUTPUT_ENCRYPTED       0x2

#define IS_FASTPATH(hdr) ((hdr & 0x03) == FASTPATH_OUTPUT_ACTION_FASTPATH)
#define IS_SLOWPATH(hdr) ((hdr) == FASTPATH_OUTPUT_ACTION_X224)

/* [MS-RDPBCGR] 2.2.9.1.2.1 */
/* adjusted for position in updateHeader */
#define FASTPATH_UPDATETYPE_ORDERS		0x0
#define FASTPATH_UPDATETYPE_BITMAP		0x1
#define FASTPATH_UPDATETYPE_PALETTE		0x2
#define FASTPATH_UPDATETYPE_SYNCHRONIZE		0x3
#define FASTPATH_UPDATETYPE_SURFCMDS		0x4
#define FASTPATH_UPDATETYPE_PTR_NULL		0x5
#define FASTPATH_UPDATETYPE_PTR_DEFAULT		0x6
#define FASTPATH_UPDATETYPE_PTR_POSITION	0x8
#define FASTPATH_UPDATETYPE_COLOR		0x9
#define FASTPATH_UPDATETYPE_CACHED		0xA
#define FASTPATH_UPDATETYPE_POINTER		0xB

#define FASTPATH_FRAGMENT_SINGLE	(0x0 << 4)
#define FASTPATH_FRAGMENT_LAST		(0x1 << 4)
#define FASTPATH_FRAGMENT_FIRST		(0x2 << 4)
#define FASTPATH_FRAGMENT_NEXT		(0x3 << 4)

#define FASTPATH_OUTPUT_COMPRESSION_USED	(0x2 << 6)

#define RDESKTOP_FASTPATH_MULTIFRAGMENT_MAX_SIZE 65535

/* ISO PDU codes */
enum ISO_PDU_CODE
{
	ISO_PDU_CR = 0xE0,	/* Connection Request */
	ISO_PDU_CC = 0xD0,	/* Connection Confirm */
	ISO_PDU_DR = 0x80,	/* Disconnect Request */
	ISO_PDU_DT = 0xF0,	/* Data */
	ISO_PDU_ER = 0x70	/* Error */
};

/* RDP protocol negotiating constants */
enum RDP_NEG_TYPE_CODE
{
	RDP_NEG_REQ = 1,
	RDP_NEG_RSP = 2,
	RDP_NEG_FAILURE = 3
};

enum RDP_NEG_REQ_CODE
{
	PROTOCOL_RDP = 0,
	PROTOCOL_SSL = 1,
	PROTOCOL_HYBRID = 2
};

enum RDP_NEG_FAILURE_CODE
{
	SSL_REQUIRED_BY_SERVER = 1,
	SSL_NOT_ALLOWED_BY_SERVER = 2,
	SSL_CERT_NOT_ON_SERVER = 3,
	INCONSISTENT_FLAGS = 4,
	HYBRID_REQUIRED_BY_SERVER = 5,
	SSL_WITH_USER_AUTH_REQUIRED_BY_SERVER = 6
};

/* MCS PDU codes */
enum MCS_PDU_TYPE
{
	MCS_EDRQ = 1,		/* Erect Domain Request */
	MCS_DPUM = 8,		/* Disconnect Provider Ultimatum */
	MCS_AURQ = 10,		/* Attach User Request */
	MCS_AUCF = 11,		/* Attach User Confirm */
	MCS_CJRQ = 14,		/* Channel Join Request */
	MCS_CJCF = 15,		/* Channel Join Confirm */
	MCS_SDRQ = 25,		/* Send Data Request */
	MCS_SDIN = 26		/* Send Data Indication */
};

#define MCS_CONNECT_INITIAL	0x7f65
#define MCS_CONNECT_RESPONSE	0x7f66

#define BER_TAG_BOOLEAN		1
#define BER_TAG_INTEGER		2
#define BER_TAG_OCTET_STRING	4
#define BER_TAG_RESULT		10
#define BER_TAG_SEQUENCE	16
#define BER_TAG_CONSTRUCTED	0x20
#define BER_TAG_CTXT_SPECIFIC	0x80

#define MCS_TAG_DOMAIN_PARAMS	0x30

#define MCS_GLOBAL_CHANNEL	1003
#define MCS_USERCHANNEL_BASE    1001

/* RDP secure transport constants */
#define SEC_RANDOM_SIZE		32
#define SEC_MODULUS_SIZE	64
#define SEC_MAX_MODULUS_SIZE	256
#define SEC_PADDING_SIZE	8
#define SEC_EXPONENT_SIZE	4

/* TS_SECURITY_HEADER.flags */
#define SEC_EXCHANGE_PKT	0x0001
#define SEC_TRANSPORT_REQ	0x0002
#define RDP_SEC_TRANSPORT_RSP	0x0004
#define SEC_ENCRYPT		0x0008
#define SEC_RESET_SEQNO		0x0010
#define SEC_IGNORE_SEQNO	0x0020
#define SEC_INFO_PKT		0x0040
#define SEC_LICENSE_PKT		0x0080
#define SEC_LICENSE_ENCRYPT_CS	0x0200
#define SEC_LICENSE_ENCRYPT_SC	0x0200
#define SEC_REDIRECTION_PKT	0x0400
#define SEC_SECURE_CHECKSUM	0x0800
#define SEC_AUTODETECT_REQ	0x1000
#define SEC_AUTODETECT_RSP	0x2000
#define SEC_HEARTBEAT		0x4000
#define SEC_FLAGSHI_VALID	0x8000

#define SEC_TAG_SRV_INFO	0x0c01
#define SEC_TAG_SRV_CRYPT	0x0c02
#define SEC_TAG_SRV_CHANNELS	0x0c03

#define CS_CORE			0xc001
#define CS_SECURITY		0xc002
#define CS_NET			0xc003
#define CS_CLUSTER		0xc004

#define SEC_TAG_PUBKEY		0x0006
#define SEC_TAG_KEYSIG		0x0008

#define SEC_RSA_MAGIC		0x31415352	/* RSA1 */

/* Client cluster constants */
#define SEC_CC_REDIRECTION_SUPPORTED          0x00000001
#define SEC_CC_REDIRECT_SESSIONID_FIELD_VALID 0x00000002
#define SEC_CC_REDIRECTED_SMARTCARD           0x00000040
#define SEC_CC_REDIRECT_VERSION_MASK          0x0000003c

#define SEC_CC_REDIRECT_VERSION_3             0x02
#define SEC_CC_REDIRECT_VERSION_4             0x03
#define SEC_CC_REDIRECT_VERSION_5             0x04
#define SEC_CC_REDIRECT_VERSION_6             0x05

/* RDP licensing constants */
#define LICENCE_TOKEN_SIZE	10
#define LICENCE_HWID_SIZE	20
#define LICENCE_SIGNATURE_SIZE	16

#define LICENCE_TAG_REQUEST                     0x01
#define LICENCE_TAG_PLATFORM_CHALLANGE          0x02
#define LICENCE_TAG_NEW_LICENCE                 0x03
#define LICENCE_TAG_UPGRADE_LICENCE             0x04
#define LICENCE_TAG_LICENCE_INFO                0x12
#define LICENCE_TAG_NEW_LICENCE_REQUEST         0x13
#define LICENCE_TAG_PLATFORM_CHALLANGE_RESPONSE 0x15
#define LICENCE_TAG_ERROR_ALERT                 0xff

#define BB_CLIENT_USER_NAME_BLOB	0x000f
#define BB_CLIENT_MACHINE_NAME_BLOB	0x0010

/* RDP PDU codes */
enum RDP_PDU_TYPE
{
	RDP_PDU_DEMAND_ACTIVE = 1,
	RDP_PDU_CONFIRM_ACTIVE = 3,
	RDP_PDU_REDIRECT = 4,	/* Standard Server Redirect */
	RDP_PDU_DEACTIVATE = 6,
	RDP_PDU_DATA = 7,
	RDP_PDU_ENHANCED_REDIRECT = 10	/* Enhanced Server Redirect */
};

enum RDP_DATA_PDU_TYPE
{
	RDP_DATA_PDU_UPDATE = 2,
	RDP_DATA_PDU_CONTROL = 20,
	RDP_DATA_PDU_POINTER = 27,
	RDP_DATA_PDU_INPUT = 28,
	RDP_DATA_PDU_SYNCHRONISE = 31,
	RDP_DATA_PDU_BELL = 34,
	RDP_DATA_PDU_CLIENT_WINDOW_STATUS = 35,
	RDP_DATA_PDU_LOGON = 38,	/* PDUTYPE2_SAVE_SESSION_INFO */
	RDP_DATA_PDU_FONT2 = 39,
	RDP_DATA_PDU_KEYBOARD_INDICATORS = 41,
	RDP_DATA_PDU_DISCONNECT = 47,
	RDP_DATA_PDU_AUTORECONNECT_STATUS = 50
};

enum RDP_SAVE_SESSION_PDU_TYPE
{
	INFOTYPE_LOGON = 0,
	INFOTYPE_LOGON_LONG = 1,
	INFOTYPE_LOGON_PLAINNOTIFY = 2,
	INFOTYPE_LOGON_EXTENDED_INF = 3
};

enum RDP_LOGON_INFO_EXTENDED_TYPE
{
	LOGON_EX_AUTORECONNECTCOOKIE = 1,
	LOGON_EX_LOGONERRORS = 2
};

enum RDP_CONTROL_PDU_TYPE
{
	RDP_CTL_REQUEST_CONTROL = 1,
	RDP_CTL_GRANT_CONTROL = 2,
	RDP_CTL_DETACH = 3,
	RDP_CTL_COOPERATE = 4
};

enum RDP_UPDATE_PDU_TYPE
{
	RDP_UPDATE_ORDERS = 0,
	RDP_UPDATE_BITMAP = 1,
	RDP_UPDATE_PALETTE = 2,
	RDP_UPDATE_SYNCHRONIZE = 3
};

enum RDP_POINTER_PDU_TYPE
{
	RDP_POINTER_SYSTEM = 1,
	RDP_POINTER_MOVE = 3,
	RDP_POINTER_COLOR = 6,
	RDP_POINTER_CACHED = 7,
	RDP_POINTER_NEW = 8
};

/* [MS-RDPBCGR] 2.2.9.1.1.4.3 */
enum RDP_SYSTEM_POINTER_TYPE
{
	SYSPTR_NULL    = 0x00000000,
	SYSPTR_DEFAULT = 0x00007F00
};

enum RDP_INPUT_DEVICE
{
	RDP_INPUT_SYNCHRONIZE = 0,
	RDP_INPUT_CODEPOINT = 1,
	RDP_INPUT_VIRTKEY = 2,
	RDP_INPUT_SCANCODE = 4,
	RDP_INPUT_MOUSE = 0x8001,
	RDP_INPUT_MOUSEX = 0x8002
};

/* Device flags */
#define KBD_FLAG_RIGHT          0x0001
#define KBD_FLAG_EXT            0x0100
#define KBD_FLAG_EXT1           0x0200
#define KBD_FLAG_QUIET          0x1000
#define KBD_FLAG_DOWN           0x4000
#define KBD_FLAG_UP             0x8000

/* These are for synchronization; not for keystrokes */
#define KBD_FLAG_SCROLL   0x0001
#define KBD_FLAG_NUMLOCK  0x0002
#define KBD_FLAG_CAPITAL  0x0004

/* See T.128 */
#define RDP_KEYPRESS 0
#define RDP_KEYRELEASE (KBD_FLAG_DOWN | KBD_FLAG_UP)

#define MOUSE_FLAG_MOVE         0x0800
#define MOUSE_FLAG_BUTTON1      0x1000
#define MOUSE_FLAG_BUTTON2      0x2000
#define MOUSE_FLAG_BUTTON3      0x4000
#define MOUSE_FLAG_BUTTON4      0x0280
#define MOUSE_FLAG_BUTTON5      0x0380
#define MOUSEX_FLAG_BUTTON1     0x0001
#define MOUSEX_FLAG_BUTTON2     0x0002
#define MOUSE_FLAG_DOWN         0x8000

/* Raster operation masks */
#define ROP2_S(rop3) (rop3 & 0xf)
#define ROP2_P(rop3) ((rop3 & 0x3) | ((rop3 & 0x30) >> 2))

#define ROP2_COPY	0xc
#define ROP2_XOR	0x6
#define ROP2_AND	0x8
#define ROP2_NXOR	0x9
#define ROP2_OR		0xe

#define MIX_TRANSPARENT	0
#define MIX_OPAQUE	1

#define TEXT2_VERTICAL		0x04
#define TEXT2_IMPLICIT_X	0x20

#define ALTERNATE	1
#define WINDING		2

/* RDP bitmap cache (version 2) constants */
#define BMPCACHE2_C0_CELLS	0x78
#define BMPCACHE2_C1_CELLS	0x78
#define BMPCACHE2_C2_CELLS	0x150
#define BMPCACHE2_NUM_PSTCELLS	0x9f6

#define PDU_FLAG_FIRST		0x01
#define PDU_FLAG_LAST		0x02

/* RDP capabilities */
#define RDP_CAPSET_GENERAL	1	/* Maps to generalCapabilitySet in T.128 page 138 */
#define RDP_CAPLEN_GENERAL	0x18
#define OS_MAJOR_TYPE_UNIX	4
#define OS_MINOR_TYPE_XSERVER	7

#define RDP_CAPSET_BITMAP	2
#define RDP_CAPLEN_BITMAP	0x1C

#define RDP_CAPSET_ORDER	3
#define RDP_CAPLEN_ORDER	0x58
#define ORDER_CAP_NEGOTIATE	2
#define ORDER_CAP_NOSUPPORT	4

#define RDP_CAPSET_BMPCACHE	4
#define RDP_CAPLEN_BMPCACHE	0x28

#define RDP_CAPSET_CONTROL	5
#define RDP_CAPLEN_CONTROL	0x0C

#define RDP_CAPSET_ACTIVATE	7
#define RDP_CAPLEN_ACTIVATE	0x0C

#define RDP_CAPSET_POINTER	8
#define RDP_CAPLEN_POINTER	0x08
#define RDP_CAPLEN_NEWPOINTER	0x0a

#define RDP_CAPSET_SHARE	9
#define RDP_CAPLEN_SHARE	0x08

#define RDP_CAPSET_COLCACHE	10
#define RDP_CAPLEN_COLCACHE	0x08

#define RDP_CAPSET_SOUND	12
#define RDP_CAPLEN_SOUND	8

#define RDP_CAPSET_INPUT	13
#define RDP_CAPLEN_INPUT	88

#define RDP_CAPSET_FONT		14
#define RDP_CAPLEN_FONT		8

#define RDP_CAPSET_BRUSHCACHE	15
#define RDP_CAPLEN_BRUSHCACHE	0x08

#define RDP_CAPSET_GLYPHCACHE	16
#define RDP_CAPLEN_GLYPHCACHE	52

#define RDP_CAPSET_BMPCACHE2	19
#define RDP_CAPLEN_BMPCACHE2	0x28
#define BMPCACHE2_FLAG_PERSIST	((uint32)1<<31)

#define RDP_CAPSET_MULTIFRAGMENTUPDATE 26
#define RDP_CAPLEN_MULTIFRAGMENTUPDATE 8

#define RDP_CAPSET_LARGE_POINTER	27
#define RDP_CAPLEN_LARGE_POINTER	6

#define RDP_SOURCE		"MSTSC"

/* [MS-RDPBCGR] TS_INFO_PACKET flags */
#define INFO_MOUSE			0x00000001
#define INFO_DISABLECTRLALTDEL		0x00000002
#define INFO_AUTOLOGON			0x00000008
#define INFO_UNICODE			0x00000010
#define INFO_MAXIMIZESHELL		0x00000020
#define INFO_LOGONNOTIFY		0x00000040
#define INFO_COMPRESSION		0x00000080	/* compressionTypeMask holds valid data if set */
#define INFO_ENABLEWINDOWSKEY		0x00000100
#define INFO_COMPRESSIONTYPEMASK	0x00001E00
#define INFO_REMOTE_CONSOLE_AUDIO	0x00002000
#define INFO_FORCE_ENCRYPTED_CS_PDU	0x00004000
#define INFO_RAIL			0x00008000
#define INFO_LOGONERRORS		0x00010000
#define INFO_MOUSE_HAS_WHEEL		0x00020000
#define INFO_PASSWORD_IS_SC_PIN		0x00040000
#define INFO_NOAUDIOPLAYBACK		0x00080000
#define INFO_USING_SAVED_CREDS		0x00100000
#define INFO_AUDIOCAPTURE		0x00200000
#define INFO_VIDEO_DISABLE		0x00400000
#define INFO_HIDEF_RAIL_SUPPORTED	0x02000000

/* Compression types, fits into INFO_COMPRESSIONTYPEMASK */
#define PACKET_COMPR_TYPE_8K		0x00000000	/* RDP 4.0 bulk compression */
#define PACKET_COMPR_TYPE_64K		0x00000200	/* RDP 5.0 bulk compression */
#define PACKET_COMPR_TYPE_RDP6		0x00000400	/* RDP 6.0 bulk compression */
#define PACKET_COMPR_TYPE_RDP61		0x00000800	/* RDP 6.1 bulk compression */

/* [MS-RDPBCGR] TS_EXTENDED_INFO_PACKET clientAddressFamily */
#define RDP_AF_INET	0x0002
#define RDP_AF_INET6	0x0017

/* [MS-RDPBCGR] TS_EXTENDED_INFO_PACKET performanceFlags */
#define PERF_DISABLE_WALLPAPER	        0x01
#define PERF_DISABLE_FULLWINDOWDRAG	0x02
#define PERF_DISABLE_MENUANIMATIONS	0x04
#define PERF_DISABLE_THEMING		0x08
#define PERF_DISABLE_CURSOR_SHADOW	0x20
#define PERF_DISABLE_CURSORSETTINGS	0x40	/* disables cursor blinking */
#define PERF_ENABLE_FONT_SMOOTHING	0x80

/* compression types */
#define RDP_MPPC_BIG		0x01
#define RDP_MPPC_COMPRESSED	0x20
#define RDP_MPPC_RESET		0x40
#define RDP_MPPC_FLUSH		0x80
#define RDP_MPPC_DICT_SIZE      65536

#define RDP5_COMPRESSED		0x80

/* Keymap flags */
#define MapRightShiftMask   (1<<0)
#define MapLeftShiftMask    (1<<1)
#define MapShiftMask (MapRightShiftMask | MapLeftShiftMask)

#define MapRightAltMask     (1<<2)
#define MapLeftAltMask      (1<<3)
#define MapAltGrMask MapRightAltMask

#define MapRightCtrlMask    (1<<4)
#define MapLeftCtrlMask     (1<<5)
#define MapCtrlMask (MapRightCtrlMask | MapLeftCtrlMask)

#define MapRightWinMask     (1<<6)
#define MapLeftWinMask      (1<<7)
#define MapWinMask (MapRightWinMask | MapLeftWinMask)

#define MapNumLockMask      (1<<8)
#define MapCapsLockMask     (1<<9)

#define MapLocalStateMask   (1<<10)

#define MapInhibitMask      (1<<11)

#define MASK_ADD_BITS(var, mask) (var |= mask)
#define MASK_REMOVE_BITS(var, mask) (var &= ~mask)
#define MASK_HAS_BITS(var, mask) ((var & mask)>0)
#define MASK_CHANGE_BIT(var, mask, active) (var = ((var & ~mask) | (active ? mask : 0)))

/* Clipboard constants, "borrowed" from GCC system headers in 
   the w32 cross compiler
   this is the CF_ set when WINVER is 0x0400 */

#ifndef CF_TEXT
#define CF_TEXT         1
#define CF_BITMAP       2
#define CF_METAFILEPICT 3
#define CF_SYLK         4
#define CF_DIF          5
#define CF_TIFF         6
#define CF_OEMTEXT      7
#define CF_DIB          8
#define CF_PALETTE      9
#define CF_PENDATA      10
#define CF_RIFF         11
#define CF_WAVE         12
#define CF_UNICODETEXT  13
#define CF_ENHMETAFILE  14
#define CF_HDROP        15
#define CF_LOCALE       16
#define CF_MAX          17
#define CF_OWNERDISPLAY 128
#define CF_DSPTEXT      129
#define CF_DSPBITMAP    130
#define CF_DSPMETAFILEPICT      131
#define CF_DSPENHMETAFILE       142
#define CF_PRIVATEFIRST 512
#define CF_PRIVATELAST  767
#define CF_GDIOBJFIRST  768
#define CF_GDIOBJLAST   1023
#endif

/* Sound format constants */
#define WAVE_FORMAT_PCM		1
#define WAVE_FORMAT_ADPCM	2
#define WAVE_FORMAT_ALAW	6
#define WAVE_FORMAT_MULAW	7

/* Virtual channel options */
#define CHANNEL_OPTION_INITIALIZED	0x80000000
#define CHANNEL_OPTION_ENCRYPT_RDP	0x40000000
#define CHANNEL_OPTION_COMPRESS_RDP	0x00800000
#define CHANNEL_OPTION_SHOW_PROTOCOL	0x00200000

/* NT status codes for RDPDR */
#define RD_STATUS_SUCCESS                  0x00000000
#define RD_STATUS_NOT_IMPLEMENTED          0x00000001
#define RD_STATUS_PENDING                  0x00000103

#define RD_STATUS_NO_MORE_FILES            0x80000006
#define RD_STATUS_DEVICE_PAPER_EMPTY       0x8000000e
#define RD_STATUS_DEVICE_POWERED_OFF       0x8000000f
#define RD_STATUS_DEVICE_OFF_LINE          0x80000010
#define RD_STATUS_DEVICE_BUSY              0x80000011

#define RD_STATUS_INVALID_HANDLE           0xc0000008
#define RD_STATUS_INVALID_PARAMETER        0xc000000d
#define RD_STATUS_NO_SUCH_FILE             0xc000000f
#define RD_STATUS_INVALID_DEVICE_REQUEST   0xc0000010
#define RD_STATUS_ACCESS_DENIED            0xc0000022
#define RD_STATUS_OBJECT_NAME_COLLISION    0xc0000035
#define RD_STATUS_DISK_FULL                0xc000007f
#define RD_STATUS_FILE_IS_A_DIRECTORY      0xc00000ba
#define RD_STATUS_NOT_SUPPORTED            0xc00000bb
#define RD_STATUS_TIMEOUT                  0xc0000102
#define RD_STATUS_NOTIFY_ENUM_DIR          0xc000010c
#define RD_STATUS_CANCELLED                0xc0000120
#define RD_STATUS_DIRECTORY_NOT_EMPTY      0xc0000101

/* RDPSND constants */
#define TSSNDCAPS_ALIVE                    0x00000001
#define TSSNDCAPS_VOLUME                   0x00000002

/* RDPDR constants */

#define RDPDR_CTYP_CORE                 0x4472
#define RDPDR_CTYP_PRN                  0x5052

#define PAKID_CORE_SERVER_ANNOUNCE      0x496e
#define PAKID_CORE_CLIENTID_CONFIRM     0x4343
#define PAKID_CORE_CLIENT_NAME          0x434e
#define PAKID_CORE_DEVICE_LIST_ANNOUNCE 0x4441
#define PAKID_CORE_DEVICE_REPLY         0x6472
#define PAKID_CORE_DEVICE_IOREQUEST     0x4952
#define PAKID_CORE_DEVICE_IOCOMPLETION  0x4943
#define PAKID_CORE_SERVER_CAPABILITY    0x5350
#define PAKID_CORE_CLIENT_CAPABILITY    0x4350
#define PAKID_CORE_DEVICELIST_REMOVE    0x444d
#define PAKID_PRN_CACHE_DATA            0x5043
#define PAKID_CORE_USER_LOGGEDON        0x554c
#define PAKID_PRN_USING_XPS             0x5543

#define RDPDR_MAX_DEVICES               0x10
#define DEVICE_TYPE_SERIAL              0x01
#define DEVICE_TYPE_PARALLEL            0x02
#define DEVICE_TYPE_PRINTER             0x04
#define DEVICE_TYPE_DISK                0x08
#define DEVICE_TYPE_SCARD               0x20

#define FILE_DIRECTORY_FILE             0x00000001
#define FILE_NON_DIRECTORY_FILE         0x00000040
#define FILE_COMPLETE_IF_OPLOCKED       0x00000100
#define FILE_DELETE_ON_CLOSE            0x00001000
#define FILE_OPEN_FOR_FREE_SPACE_QUERY  0x00800000

#define CAP_GENERAL_TYPE   0x0001
#define CAP_PRINTER_TYPE   0x0002
#define CAP_PORT_TYPE      0x0003
#define CAP_DRIVE_TYPE     0x0004
#define CAP_SMARTCARD_TYPE 0x0005

#define GENERAL_CAPABILITY_VERSION_01   0x00000001
#define GENERAL_CAPABILITY_VERSION_02   0x00000002
#define PRINT_CAPABILITY_VERSION_01     0x00000001
#define PORT_CAPABILITY_VERSION_01      0x00000001
#define DRIVE_CAPABILITY_VERSION_01     0x00000001
#define DRIVE_CAPABILITY_VERSION_02     0x00000002
#define SMARTCARD_CAPABILITY_VERSION_01 0x00000001

#define RDPDR_IRP_MJ_CREATE                   0x00000001
#define RDPDR_IRP_MJ_CLEANUP                  0x00000002
#define RDPDR_IRP_MJ_CLOSE                    0x00000004
#define RDPDR_IRP_MJ_READ                     0x00000008
#define RDPDR_IRP_MJ_WRITE                    0x00000010
#define RDPDR_IRP_MJ_FLUSH_BUFFERS            0x00000020
#define RDPDR_IRP_MJ_SHUTDOWN                 0x00000040
#define RDPDR_IRP_MJ_DEVICE_CONTROL           0x00000080
#define RDPDR_IRP_MJ_QUERY_VOLUME_INFORMATION 0x00000100
#define RDPDR_IRP_MJ_SET_VOLUME_INFORMATION   0x00000200
#define RDPDR_IRP_MJ_QUERY_INFORMATION        0x00000400
#define RDPDR_IRP_MJ_SET_INFORMATION          0x00000800
#define RDPDR_IRP_MJ_DIRECTORY_CONTROL        0x00001000
#define RDPDR_IRP_MJ_LOCK_CONTROL             0x00002000
#define RDPDR_IRP_MJ_QUERY_SECURITY           0x00004000
#define RDPDR_IRP_MJ_SET_SECURITY             0x00008000
#define ALL_RDPDR_IRP_MJ                      0x0000FFFF

#define RDPDR_PRINTER_ANNOUNCE_FLAG_ASCII		0x00000001
#define RDPDR_PRINTER_ANNOUNCE_FLAG_DEFAULTPRINTER	0x00000002
#define RDPDR_PRINTER_ANNOUNCE_FLAG_NETWORKPRINTER	0x00000004
#define RDPDR_PRINTER_ANNOUNCE_FLAG_TSPRINTER		0x00000008
#define RDPDR_PRINTER_ANNOUNCE_FLAG_XPSFORMAT		0x00000010

#define RDPDR_DEVICE_REMOVE_PDUS      0x00000001
#define RDPDR_CLIENT_DISPLAY_NAME_PDU 0x00000002
#define RDPDR_USER_LOGGEDON_PDU       0x00000004

/* RDP5 disconnect PDU
 *
 * Named after the corresponding names on the server side:
 * https://msdn.microsoft.com/en-us/library/cc240544.aspx
 */
#define ERRINFO_NO_INFO					0x0000
#define ERRINFO_RPC_INITIATED_DISCONNECT		0x0001
#define ERRINFO_RPC_INITIATED_LOGOFF			0x0002
#define ERRINFO_IDLE_TIMEOUT				0x0003
#define ERRINFO_LOGON_TIMEOUT				0x0004
#define ERRINFO_DISCONNECTED_BY_OTHERCONNECTION		0x0005
#define ERRINFO_OUT_OF_MEMORY				0x0006
#define ERRINFO_SERVER_DENIED_CONNECTION		0x0007
#define ERRINFO_SERVER_DENIED_CONNECTION_FIPS		0x0008
#define ERRINFO_SERVER_INSUFFICIENT_PRIVILEGES		0x0009
#define ERRINFO_SERVER_FRESH_CREDENTIALS_REQUIRED	0x000a
#define ERRINFO_RPC_INITIATED_DISCONNECT_BYUSER		0x000b
#define ERRINFO_LOGOFF_BYUSER				0x000c
#define ERRINFO_LICENSE_INTERNAL			0x0100
#define ERRINFO_LICENSE_NO_LICENSE_SERVER		0x0101
#define ERRINFO_LICENSE_NO_LICENSE			0x0102
#define ERRINFO_LICENSE_BAD_CLIENT_MSG			0x0103
#define ERRINFO_LICENSE_HWID_DOESNT_MATCH_LICENSE	0x0104
#define ERRINFO_LICENSE_BAD_CLIENT_LICENSE		0x0105
#define ERRINFO_LICENSE_CANT_FINISH_PROTOCOL		0x0106
#define ERRINFO_LICENSE_CLIENT_ENDED_PROTOCOL		0x0107
#define ERRINFO_LICENSE_BAD_CLIENT_ENCRYPTION		0x0108
#define ERRINFO_LICENSE_CANT_UPGRADE_LICENSE		0x0109
#define ERRINFO_LICENSE_NO_REMOTE_CONNECTIONS		0x010a
#define ERRINFO_CB_DESTINATION_NOT_FOUND		0x0400
#define ERRINFO_CB_LOADING_DESTINATION			0x0402
#define ERRINFO_CB_REDIRECTING_TO_DESTINATION		0x0404
#define ERRINFO_CB_SESSION_ONLINE_VM_WAKE		0x0405
#define ERRINFO_CB_SESSION_ONLINE_VM_BOOT		0x0406
#define ERRINFO_CB_SESSION_ONLINE_VM_NO_DNS		0x0407
#define ERRINFO_CB_DESTINATION_POOL_NOT_FREE		0x0408
#define ERRINFO_CB_CONNECTION_CANCELLED			0x0409
#define ERRINFO_CB_CONNECTION_ERROR_INVALID_SETTINGS	0x0410
#define ERRINFO_CB_SESSION_ONLINE_VM_BOOT_TIMEOUT	0x0411
#define ERRINFO_CB_SESSION_ONLINE_VM_SESSMON_FAILED	0x0412
#define ERRINFO_REMOTEAPPSNOTENABLED			0x10f3
#define ERRINFO_UPDATESESSIONKEYFAILED			0x1191
#define ERRINFO_DECRYPTFAILED				0x1192
#define ERRINFO_ENCRYPTFAILED				0x1193

/* SeamlessRDP constants */
#define SEAMLESSRDP_NOTYETMAPPED -1
#define SEAMLESSRDP_NORMAL 0
#define SEAMLESSRDP_MINIMIZED 1
#define SEAMLESSRDP_MAXIMIZED 2
#define SEAMLESSRDP_POSITION_TIMER 200000

#define SEAMLESSRDP_CREATE_MODAL	0x0001
#define SEAMLESSRDP_CREATE_TOPMOST	0x0002

#define SEAMLESSRDP_HELLO_RECONNECT	0x0001
#define SEAMLESSRDP_HELLO_HIDDEN	0x0002

/* Smartcard constants */
#define SCARD_LOCK_TCP		0
#define SCARD_LOCK_SEC		1
#define SCARD_LOCK_CHANNEL	2
#define SCARD_LOCK_RDPDR	3
#define SCARD_LOCK_LAST		4


/* redirect flags, from [MS-RDPBCGR] 2.2.13.1 */
enum RDP_PDU_REDIRECT_FLAGS
{
	LB_TARGET_NET_ADDRESS = 0x1,
	LB_LOAD_BALANCE_INFO = 0x2,
	LB_USERNAME = 0x4,
	LB_DOMAIN = 0x8,
	LB_PASSWORD = 0x10,
	LB_DONTSTOREUSERNAME = 0x20,
	LB_SMARTCARD_LOGON = 0x40,
	LB_NOREDIRECT = 0x80,
	LB_TARGET_FQDN = 0x100,
	LB_TARGET_NETBIOS = 0x200,
	LB_TARGET_NET_ADDRESSES = 0x800,
	LB_CLIENT_TSV_URL = 0x1000,
	LB_SERVER_TSV_CAPABLE = 0x2000,
	LB_PASSWORD_IS_PK_ENCRYPTED = 0x4000,
	LB_REDIRECTION_GUID = 0x8000,
	LB_TARGET_CERTIFICATE = 0x10000
};

/* color depths, from [MS-RDPBCGR] 2.2.1.3.2 */
#define RNS_UD_COLOR_4BPP	0xCA00
#define RNS_UD_COLOR_8BPP	0xCA01
#define RNS_UD_COLOR_16BPP_555	0xCA02
#define RNS_UD_COLOR_16BPP_565	0xCA03
#define RNS_UD_COLOR_24BPP	0xCA04

#define RNS_UD_SAS_DEL		0xAA03

/* version, [MS-RDPBCGR] 2.2.1.3.2 */
#define RDP_40		0x00080001	/* RDP 4.0 clients */
#define RDP_50		0x00080004	/* RDP 5.0, 5.1, 5.2, 6.0, 6.1, 7.0, 7.1, 8.0, and 8.1 clients */
#define RDP_10_0	0x00080005	/* RDP 10.0 clients */
#define RDP_10_1	0x00080006	/* RDP 10.1 clients */
#define RDP_10_2	0x00080007	/* RDP 10.2 clients */
#define RDP_10_3	0x00080008	/* RDP 10.3 clients */

/* supportedColorDepths, [MS-RDPBCGR] 2.2.1.3.2 */
#define RNS_UD_24BPP_SUPPORT	0x0001
#define RNS_UD_16BPP_SUPPORT	0x0002
#define RNS_UD_15BPP_SUPPORT	0x0004
#define RNS_UD_32BPP_SUPPORT	0x0008

/* earlyCapabilityFlags, [MS-RDPBCGR] 2.2.1.3.2 */
#define RNS_UD_CS_SUPPORT_ERRINFO_PDU		0x0001
#define RNS_UD_CS_WANT_32BPP_SESSION		0x0002
#define RNS_UD_CS_SUPPORT_STATUSINFO_PDU	0x0004
#define RNS_UD_CS_STRONG_ASYMMETRIC_KEYS	0x0008
#define RNS_UD_CS_UNUSED			0x0010
#define RNS_UD_CS_VALID_CONNECTION_TYPE		0x0020
#define RNS_UD_CS_SUPPORT_MONITOR_LAYOUT_PDU	0x0040
#define RNS_UD_CS_SUPPORT_NETCHAR_AUTODETECT	0x0080
#define RNS_UD_CS_SUPPORT_DYNVC_GFX_PROTOCOL	0x0100
#define RNS_UD_CS_SUPPORT_DYNAMIC_TIME_ZONE	0x0200
#define RNS_UD_CS_SUPPORT_HEARTBEAT_PDU		0x0400

/* [MS-RDPBCGR] 2.2.7.1.1 */
#define OSMAJORTYPE_WINDOWS	0x0001
#define OSMINORTYPE_WINDOWSNT	0x0003
#define TS_CAPS_PROTOCOLVERSION	0x0200

/* extraFlags, [MS-RDPBCGR] 2.2.7.1.1 */
#define FASTPATH_OUTPUT_SUPPORTED	0x0001
#define LONG_CREDENTIALS_SUPPORTED	0x0004
#define AUTORECONNECT_SUPPORTED		0x0008
#define ENC_SALTED_CHECKSUM		0x0010
#define NO_BITMAP_COMPRESSION_HDR	0x0400

/* orderFlags, [MS-RDPBCGR] 2.2.7.1.3 */
#define NEGOTIATEORDERSUPPORT	0x0002
#define ZEROBOUNDSDELTASSUPPORT 0x0008
#define COLORINDEXSUPPORT	0x0020
#define SOLIDPATTERNBRUSHONLY	0x0040
#define ORDERFLAGS_EXTRA_FLAGS	0x0080

/* orderSupport index, [MS-RDPBCGR] 2.2.7.1.3 */
#define TS_NEG_DSTBLT_INDEX		0x00
#define TS_NEG_PATBLT_INDEX		0x01
#define TS_NEG_SCRBLT_INDEX		0x02
#define TS_NEG_MEMBLT_INDEX		0x03
#define TS_NEG_MEM3BLT_INDEX		0x04
#define TS_NEG_DRAWNINEGRID_INDEX	0x07
#define TS_NEG_LINETO_INDEX		0x08
#define TS_NEG_MULTI_DRAWNINEGRID_INDEX 0x09
#define TS_NEG_SAVEBITMAP_INDEX		0x0B
#define TS_NEG_MULTIDSTBLT_INDEX	0x0F
#define TS_NEG_MULTIPATBLT_INDEX	0x10
#define TS_NEG_MULTISCRBLT_INDEX	0x11
#define TS_NEG_MULTIOPAQUERECT_INDEX	0x12
#define TS_NEG_FAST_INDEX_INDEX		0x13
#define TS_NEG_POLYGON_SC_INDEX		0x14
#define TS_NEG_POLYGON_CB_INDEX		0x15
#define TS_NEG_POLYLINE_INDEX		0x16
#define TS_NEG_FAST_GLYPH_INDEX		0x18
#define TS_NEG_ELLIPSE_SC_INDEX		0x19
#define TS_NEG_ELLIPSE_CB_INDEX		0x1A
#define TS_NEG_INDEX_INDEX		0x1B

/* [MS-RDPBCGR] 2.2.7.1.6 */
#define INPUT_FLAG_SCANCODES		0x0001
#define INPUT_FLAG_MOUSEX		0x0004
#define INPUT_FLAG_FASTPATH_INPUT	0x0008
#define INPUT_FLAG_UNICODE		0x0010
#define INPUT_FLAG_FASTPATH_INPUT2	0x0020
#define INPUT_FLAG_UNUSED1		0x0040
#define INPUT_FLAG_UNUSED2		0x0080
#define TS_INPUT_FLAG_MOUSE_HWHEEL	0x0100
#define TS_INPUT_FLAG_QOE_TIMESTAMPS	0x0200

/* [MS-RDPBCGR] 2.2.7.1.8 */
#define GLYPH_SUPPORT_NONE    0x0000
#define GLYPH_SUPPORT_PARTIAL 0x0001
#define GLYPH_SUPPORT_FULL    0x0002
#define GLYPH_SUPPORT_ENCODE  0x0003

/* [MS-RDPBCGR] 2.2.7.1.11 */
#define SOUND_BEEPS_FLAG 0x0001

/* [MS-RDPBCGR] 2.2.7.2.5 */
#define FONTSUPPORT_FONTLIST 0x0001

/* [MS-RDPBCGR] 2.2.7.2.7 */
#define LARGE_POINTER_FLAG_96x96	1
