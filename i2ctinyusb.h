/* SPDX-License-Identifier: GPL-2.0 */
/* ------------------------------------------------------------------------- */
/*									     */
/* i2ctinyusb.h - compatibility defines for the Linux kernel driver	     */
/*                verbatim copy from linux-5.0.21                            */
/*									     */
/* ------------------------------------------------------------------------- */
/*  In parts Copyright (C) 1995-2000 Simon G. Vogl
    (I2C API)
    With some changes from Kyösti Mälkki <kmalkki@cc.hut.fi> and
    Frodo Looijaard <frodol@dds.nl>
    In parts Copyright (C) 2006-2007 Till Harbaum (Till@Harbaum.org)
    (i2c-tiny-usb)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
    MA 02110-1301 USA.							     */
/* ------------------------------------------------------------------------- */

#ifndef __i2ctinyusb_h_included__
#define __i2ctinyusb_h_included__

/* Defines according to linux kernel I2C API */
/* from: include/uapi/linux/i2c.h)           */
#define I2C_M_RD		0x0001	/* read data, from slave to master */
					/* I2C_M_RD is guaranteed to be 0x0001! */
#define I2C_M_TEN		0x0010	/* this is a ten bit chip address */
#define I2C_M_DMA_SAFE		0x0200	/* the buffer of this message is DMA safe */
					/* makes only sense in kernelspace */
					/* userspace buffers are copied anyway */
#define I2C_M_RECV_LEN		0x0400	/* length will be first received byte */
#define I2C_M_NO_RD_ACK		0x0800	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_IGNORE_NAK	0x1000	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_REV_DIR_ADDR	0x2000	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_NOSTART		0x4000	/* if I2C_FUNC_NOSTART */
#define I2C_M_STOP		0x8000	/* if I2C_FUNC_PROTOCOL_MANGLING */

#define I2C_FUNC_I2C                    0x00000001
#define I2C_FUNC_10BIT_ADDR             0x00000002
#define I2C_FUNC_PROTOCOL_MANGLING      0x00000004 /* I2C_M_IGNORE_NAK etc. */
#define I2C_FUNC_SMBUS_PEC              0x00000008
#define I2C_FUNC_NOSTART                0x00000010 /* I2C_M_NOSTART */
#define I2C_FUNC_SLAVE                  0x00000020
#define I2C_FUNC_SMBUS_BLOCK_PROC_CALL  0x00008000 /* SMBus 2.0 */
#define I2C_FUNC_SMBUS_QUICK            0x00010000
#define I2C_FUNC_SMBUS_READ_BYTE        0x00020000
#define I2C_FUNC_SMBUS_WRITE_BYTE       0x00040000
#define I2C_FUNC_SMBUS_READ_BYTE_DATA   0x00080000
#define I2C_FUNC_SMBUS_WRITE_BYTE_DATA  0x00100000
#define I2C_FUNC_SMBUS_READ_WORD_DATA   0x00200000
#define I2C_FUNC_SMBUS_WRITE_WORD_DATA  0x00400000
#define I2C_FUNC_SMBUS_PROC_CALL        0x00800000
#define I2C_FUNC_SMBUS_READ_BLOCK_DATA  0x01000000
#define I2C_FUNC_SMBUS_WRITE_BLOCK_DATA 0x02000000
#define I2C_FUNC_SMBUS_READ_I2C_BLOCK   0x04000000 /* I2C-like block xfer  */
#define I2C_FUNC_SMBUS_WRITE_I2C_BLOCK  0x08000000 /* w/ 1-byte reg. addr. */
#define I2C_FUNC_SMBUS_HOST_NOTIFY      0x10000000

#define I2C_FUNC_SMBUS_BYTE             (I2C_FUNC_SMBUS_READ_BYTE | \
                                         I2C_FUNC_SMBUS_WRITE_BYTE)
#define I2C_FUNC_SMBUS_BYTE_DATA        (I2C_FUNC_SMBUS_READ_BYTE_DATA | \
                                         I2C_FUNC_SMBUS_WRITE_BYTE_DATA)
#define I2C_FUNC_SMBUS_WORD_DATA        (I2C_FUNC_SMBUS_READ_WORD_DATA | \
                                         I2C_FUNC_SMBUS_WRITE_WORD_DATA)
#define I2C_FUNC_SMBUS_BLOCK_DATA       (I2C_FUNC_SMBUS_READ_BLOCK_DATA | \
                                         I2C_FUNC_SMBUS_WRITE_BLOCK_DATA)
#define I2C_FUNC_SMBUS_I2C_BLOCK        (I2C_FUNC_SMBUS_READ_I2C_BLOCK | \
                                         I2C_FUNC_SMBUS_WRITE_I2C_BLOCK)

#define I2C_FUNC_SMBUS_EMUL             (I2C_FUNC_SMBUS_QUICK | \
                                         I2C_FUNC_SMBUS_BYTE | \
                                         I2C_FUNC_SMBUS_BYTE_DATA | \
                                         I2C_FUNC_SMBUS_WORD_DATA | \
                                         I2C_FUNC_SMBUS_PROC_CALL | \
                                         I2C_FUNC_SMBUS_WRITE_BLOCK_DATA | \
                                         I2C_FUNC_SMBUS_I2C_BLOCK | \
                                         I2C_FUNC_SMBUS_PEC)

/* Defines according to linux kernel i2c-tiny-usb driver */
/* from: drivers/i2c/busses/i2c-tiny-usb.c */

#define CMD_ECHO                0
#define CMD_GET_FUNC            1
#define CMD_SET_DELAY           2
#define CMD_GET_STATUS          3

#define CMD_I2C_IO              4
#define CMD_I2C_IO_BEGIN        (1<<0)
#define CMD_I2C_IO_END          (1<<1)

#define STATUS_UNCONFIGURED     -1
#define STATUS_IDLE		0
#define STATUS_ADDRESS_ACK	1
#define STATUS_ADDRESS_NAK	2
#define STATUS_RUNNING          3
#define STATUS_READ_FAILED      4
#define STATUS_WRITE_FAILED     5

#endif
