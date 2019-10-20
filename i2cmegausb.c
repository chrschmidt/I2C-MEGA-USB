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

#define __IMU__DEBUG__

#include "Config/LUFAConfig.h"
#include "version.h"
#include "Descriptors.h"
#include "LUFA/Drivers/Peripheral/TWI.h"
#ifdef __IMU__DEBUG__
#include "LUFA/Drivers/Peripheral/Serial.h"
#define DPRINTF(...) printf (__VA_ARGS__)
#define DEBUGSPEED   2000000
#else
#define DPRINTF(...)
#endif

#include "i2ctinyusb.h"

/* ms - could expand tinyusb protocol to change this */
const int8_t i2c_timeout = 2;
// TODO: move to progmem
const uint32_t supported_caps = I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;

volatile int8_t  i2c_status     = STATUS_UNCONFIGURED;
volatile int8_t  i2c_status_int = STATUS_UNCONFIGURED;
volatile int8_t  i2c_datadir;
volatile int16_t i2c_expected;
volatile int8_t  i2c_stopafter;

void i2c_stop (void) {
    if (i2c_status_int != STATUS_IDLE) {
        DPRINTF ("I2C STOP\r\n");
        TWI_StopTransmission ();
        i2c_status_int = STATUS_IDLE;
    }
}

uint8_t i2c_start (const uint8_t address, const uint8_t timeout) {
    uint8_t result;

    DPRINTF ("I2C START %02x\r\n", address);
    result = TWI_StartTransmission (address, timeout);
    if (result == TWI_ERROR_NoError) {
        i2c_status     = STATUS_ADDRESS_ACK;
        i2c_status_int = STATUS_RUNNING;
        DPRINTF ("ACK\r\n");
    } else {
        i2c_status     = STATUS_ADDRESS_NAK;
        i2c_status_int = STATUS_ADDRESS_NAK;
        DPRINTF ("NAK (%d)\r\n", result);
        i2c_stop ();
    }
    return result;
}

/* Resets the software part of the I2C engine */
void i2c_reset (void) {
    if (i2c_status_int != STATUS_UNCONFIGURED &&
        i2c_status_int != STATUS_IDLE)
        i2c_stop ();
    i2c_expected = 0;
    i2c_stopafter = 0;
}

/* The driver sets a delay in µs. Calculate Prescaler and frequency from it,
 * then convert back to bitlength in host clock cycles. */
// TODO: check USB timeouts vs. I2C request duration to find the lower limit
//       of I2C speed
int i2c_set_delay (int16_t delay) {
    uint32_t freq, bitlen;

    i2c_reset ();
    freq = 1000000 / delay;
    bitlen = TWI_BITLENGTH_FROM_FREQ (1, freq);
    if (bitlen > 255) {
        bitlen = TWI_BITLENGTH_FROM_FREQ (4, freq);
        if (bitlen > 255) {
            bitlen = TWI_BITLENGTH_FROM_FREQ (16, freq);
            if (bitlen > 255) {
                bitlen = TWI_BITLENGTH_FROM_FREQ (64, freq);
                if (bitlen > 255)
                    return -1;
                TWI_Init (TWI_BIT_PRESCALE_64, bitlen);
                DPRINTF ("TWI Init, Prescale64, %lu clocks\r\n", bitlen);
            } else {
                TWI_Init (TWI_BIT_PRESCALE_16, bitlen);
                DPRINTF ("TWI Init, Prescale16, %lu clocks\r\n", bitlen);
            }
        } else {
            TWI_Init (TWI_BIT_PRESCALE_4, bitlen);
            DPRINTF ("TWI Init, Prescale4, %lu clocks\r\n", bitlen);
        }
    } else {
        TWI_Init (TWI_BIT_PRESCALE_1, bitlen);
        DPRINTF ("TWI Init, Prescale1, %lu clocks\r\n", bitlen);
    }

    i2c_status = STATUS_IDLE;
    return 0;
}

/* This function is called from within the USB Setup Request Callback
 * and only handles 0 byte requests completely. Longer requests are
 * set up to be handled from the mainloop. */
void i2c_handle_io_request (void) {
    uint8_t cmd         = USB_ControlRequest.bRequest;
    uint8_t addr        = (USB_ControlRequest.wIndex) & 0x7f;
    uint8_t result;

    i2c_datadir  = (USB_ControlRequest.wValue & I2C_M_RD) ? 1 : 0;
    i2c_expected = USB_ControlRequest.wLength;

    DPRINTF ("i2c %s at 0x%02x%s%s, len = %d\r\n",
             i2c_datadir ? "rd" : "wr", addr,
             (cmd&CMD_I2C_IO_BEGIN) ? " START" : "",
             (cmd&CMD_I2C_IO_END)   ? " STOP"  : "",
             i2c_expected);

    /* This WILL lead to USB errors due to missing acknowledge. It's up
     * to the driver to prevent this by using correct initialization. */
    if (i2c_status == STATUS_UNCONFIGURED)
        return;

    addr = (addr << 1) | i2c_datadir;

    /* Start / Repeated Start */
    result = i2c_start (addr, i2c_timeout);
    if (result != TWI_ERROR_NoError)
        i2c_stop ();

    i2c_stopafter = 0;
    if ((cmd & CMD_I2C_IO_END)) {
        if (i2c_expected) {
            i2c_stopafter = 1;
            DPRINTF ("No STOP, pending transaction\r\n");
        } else {
            i2c_stop ();
        }
    }

    /* Send the empty USB acknowledge frame on 0-byte data operations */
    if (!i2c_expected) {
        if (i2c_datadir)
            Endpoint_ClearIN ();
        Endpoint_ClearStatusStage ();
    }
}

/* This function is called from within the main loop to finish
 * transfers set up in the USB Setup Request Callback. This function
 * fills at maximum one frame (8 data Bytes) per call. */
void i2c_task (void) {
    uint8_t result;
    uint8_t data;

    if (!i2c_expected)
        return;
    DPRINTF ("i2c_task: %d byte%s pending for %s%s\r\n",
             i2c_expected, i2c_expected==1?"":"s",
             i2c_datadir?"read":"write",
             i2c_stopafter ? " STOP" : "");
    if (i2c_datadir) {
        /* pending read */
        Endpoint_SetEndpointDirection (ENDPOINT_DIR_IN);
        while (i2c_expected && Endpoint_IsReadWriteAllowed ()) {
            i2c_expected--;
            if (i2c_status_int == STATUS_RUNNING) {
                result = TWI_ReceiveByte (&data, i2c_expected==0);
                if (!result) {
                    DPRINTF ("ERR RX\r\n");
                    i2c_status_int = STATUS_READ_FAILED;
                    Endpoint_Write_8 (0xff);
                } else {
                    Endpoint_Write_8 (data);
                }
            } else {
                Endpoint_Write_8 (0xff);
            }
        }
        Endpoint_ClearIN ();
    } else {
        /* pending write */
        Endpoint_SetEndpointDirection (ENDPOINT_DIR_OUT);
        DPRINTF ("%d BIE, %d IRWA\r\n",
                 Endpoint_BytesInEndpoint (), Endpoint_IsReadWriteAllowed ());
        while (i2c_expected && Endpoint_BytesInEndpoint ()) {
            i2c_expected--;
            data = Endpoint_Read_8 ();
            if (i2c_status_int == STATUS_RUNNING) {
                result = TWI_SendByte (data);
                if (!result) {
                    DPRINTF ("ERR TX\r\n");
                    i2c_status_int = STATUS_WRITE_FAILED;
                }
            }
        }
        Endpoint_ClearOUT ();
    }
    DPRINTF ("i2c_task: %d byte%s still pending for %s\r\n",
             i2c_expected, i2c_expected==1?"":"s",
             i2c_datadir?"read":"write");

    if (!i2c_expected) {
        /* Bring on RX or TX errors to the driver, if it asks */
        // TODO: check if the driver could handle the other answers
        if (i2c_status_int == STATUS_READ_FAILED ||
            i2c_status_int == STATUS_WRITE_FAILED)
            i2c_status = STATUS_ADDRESS_NAK;
        /* Send a STOP on the bus if requested */
        if (i2c_stopafter)
            i2c_stop ();
        /* Handle acknowledge packet after everything is done */
        if (!i2c_datadir)
            Endpoint_ClearIN();
    }
}

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
            Endpoint_Write_32_LE (supported_caps);
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
