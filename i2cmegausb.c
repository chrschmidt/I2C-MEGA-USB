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

#include "Config/LUFAConfig.h"
#include "version.h"
#include "Descriptors.h"
#include "LUFA/Drivers/Peripheral/TWI.h"
#include "debug.h"
#include "i2c.h"
#include "i2ctinyusb.h"

int main (void) {
    USB_Init ();
#ifdef __IMU__DEBUG__
    Serial_CreateStream (NULL);
    Serial_Init (DEBUGSPEED, true);
#endif
    GlobalInterruptEnable ();
    /* Enable on-chip Pullups for I2C, see Datasheet Section 20.5.1 */
// TODO: This doesn't go well with LUFA's multi-device support
    DDRD  &= 0x03;
    PORTD |= 0x03;
    DPRINTF ("\033[2J\033[0;0HI2C-MEGA-USB " VERSION_STRING "\r\n");
    for (;;) {
        USB_USBTask ();
        i2c_task ();
    }
}

/* The driver sends Control Requests with USB_TYPE_VENDOR | USB_RECIP_INTERFACE
 * for use. The Direction signals read or write. */
#define REQMASK (CONTROL_REQTYPE_TYPE | CONTROL_REQTYPE_RECIPIENT)
#define REQTARGET (REQTYPE_VENDOR | REQREC_INTERFACE)

/* We handle everything but an IO Request directly in the Control Request
 * Event handler. Note that this is called from within an interrupt handler. */
void EVENT_USB_Device_ControlRequest (void) {
    if ((USB_DeviceState == DEVICE_STATE_Configured) &&
        ((USB_ControlRequest.bmRequestType & REQMASK) == REQTARGET)) {
        Endpoint_ClearSETUP ();
        /* bRequest is the actual command from the driver to the device */
        switch (USB_ControlRequest.bRequest) {
        case CMD_ECHO:
            /* ECHO is a read request that expects the two bytes
             * held in wValue to be sent back  */
            Endpoint_Write_16_LE (USB_ControlRequest.wValue);
            Endpoint_ClearIN ();
            DPRINTF ("EC\r\n");
            break;
        case CMD_GET_FUNC:
            /* GET_FUNC is a read requests that expects the
             * capability information of the I2C controller back
             * as a 32bit value in the same format used by the
             * linux kernel */
            Endpoint_Write_32_LE (getcaps());
            Endpoint_ClearIN ();
            DPRINTF ("GF\r\n");
            break;
        case CMD_SET_DELAY:
            /* SET_DELAY is a write request with 0 byte length
             * This will fail with an USB error if the value is invalid */
            if (!i2c_set_delay (USB_ControlRequest.wValue))
                Endpoint_ClearIN ();
            DPRINTF ("SD\r\n");
            break;
        case CMD_GET_STATUS:
            /* GET_STATUS returns the result of the last I2C IO
             * transaction and expects one byte back */
            Endpoint_Write_8 (i2c_status);
            Endpoint_ClearIN ();
            DPRINTF ("GS\r\n");
            break;
        case CMD_I2C_IO:
        case CMD_I2C_IO + CMD_I2C_IO_BEGIN:
        case CMD_I2C_IO +                    CMD_I2C_IO_END:
        case CMD_I2C_IO + CMD_I2C_IO_BEGIN + CMD_I2C_IO_END:
            /* I2C_IO performs the actual IO on the bus. The flags
             * signal if this is the start of a new transaction, the
             * transaction should end after this, or if this part of
             * ongoing set of transactions */
            i2c_handle_io_request ();
            break;
        default:
            DPRINTF ("Invalid command received\r\n");
            break;
        }
    } else {
        DPRINTF ("%02x %02x %04x %04x %04x\r\n",
                 USB_ControlRequest.bmRequestType,
                 USB_ControlRequest.bRequest,
                 USB_ControlRequest.wValue,
                 USB_ControlRequest.wIndex,
                 USB_ControlRequest.wLength);
    }
}
