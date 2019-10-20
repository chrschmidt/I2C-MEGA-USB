/* SPDX-License-Identifier: GPL-2.0 */
/* ------------------------------------------------------------------------- */
/*									     */
/* i2cmegausb - a i2c-tiny-usb compatible, LUFA-based I2C interface via USB  */
/*									     */
/* ------------------------------------------------------------------------- */
/*   Copyright (C) 2019 Christian Schmidt

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    ublished by the Free Software Foundation

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
    MA 02110-1301 USA.							     */
/* ------------------------------------------------------------------------- */

#ifndef __i2c_h_included__
#define __i2c_h_included__

#include <inttypes.h>

extern volatile int8_t  i2c_status;
extern volatile int8_t  i2c_status_int;
extern volatile int8_t  i2c_datadir;
extern volatile int16_t i2c_expected;
extern volatile int8_t  i2c_stopafter;

void i2c_stop (void);
uint8_t i2c_start (const uint8_t address, const uint8_t timeout);

/* Resets the software part of the I2C engine */
void i2c_reset (void);

/* The driver sets a delay in µs. Calculate Prescaler and frequency from it,
 * then convert back to bitlength in host clock cycles. */
// TODO: check USB timeouts vs. I2C request duration to find the lower limit
//       of I2C speed
int i2c_set_delay (int16_t delay);

uint32_t getcaps (void);

void i2c_handle_io_request (void);
void i2c_task (void);


#endif
