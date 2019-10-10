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

#include "version.h"
#include "Descriptors.h"

const USB_Descriptor_Device_t PROGMEM DeviceDescriptor = {
    .Header                 = {
                                  .Size = sizeof (USB_Descriptor_Device_t),
                                  .Type = DTYPE_Device
    },

    .USBSpecification       = VERSION_BCD(1,1,0),
    .Class                  = 0xff,
    .SubClass               = 0,
    .Protocol               = 0,

    .Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,

    .VendorID               = 0x0403,
    .ProductID              = 0xc631,
    .ReleaseNumber          = VERSION_BCD(VERSION_MAJOR,
                                          VERSION_MINOR,
                                          VERSION_REVISION),

    .ManufacturerStrIndex   = STRING_ID_Manufacturer,
    .ProductStrIndex        = STRING_ID_Product,
    .SerialNumStrIndex      = USE_INTERNAL_SERIAL,

    .NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
};

const USB_Descriptor_Configuration_t PROGMEM ConfigurationDescriptor = {
    .Config = {
        .Header                 = {
            .Size = sizeof (USB_Descriptor_Configuration_Header_t),
            .Type = DTYPE_Configuration
        },

        .TotalConfigurationSize = sizeof (USB_Descriptor_Configuration_t),
        .TotalInterfaces        = 1,

        .ConfigurationNumber    = 1,
        .ConfigurationStrIndex  = NO_DESCRIPTOR,

        .ConfigAttributes       = USB_CONFIG_ATTR_RESERVED,

        .MaxPowerConsumption    = USB_CONFIG_POWER_MA(500)
    },

    .Interface =
    {
        .Header                 = {
            .Size = sizeof (USB_Descriptor_Interface_t),
            .Type = DTYPE_Interface
        },

        .InterfaceNumber        = INTERFACE_ID_MAIN,
        .AlternateSetting       = 0,

        .TotalEndpoints         = 0,

        .Class                  = 0xff,
        .SubClass               = 0,
        .Protocol               = 0,

        .InterfaceStrIndex      = NO_DESCRIPTOR
    }
};

const USB_Descriptor_String_t PROGMEM LanguageString = USB_STRING_DESCRIPTOR_ARRAY(LANGUAGE_ID_ENG);
const USB_Descriptor_String_t PROGMEM ManufacturerString = USB_STRING_DESCRIPTOR(L"Christian Schmidt");
const USB_Descriptor_String_t PROGMEM ProductString = USB_STRING_DESCRIPTOR(L"i2c-mega-usb interface");

uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint16_t wIndex,
                                    const void** const DescriptorAddress) {
    const uint8_t  DescriptorType   = (wValue >> 8);
    const uint8_t  DescriptorNumber = (wValue & 0xFF);

    const void* Address = NULL;
    uint16_t    Size    = NO_DESCRIPTOR;

    switch (DescriptorType) {
    case DTYPE_Device:
        Address = &DeviceDescriptor;
        Size    = sizeof (USB_Descriptor_Device_t);
        break;
    case DTYPE_Configuration:
        Address = &ConfigurationDescriptor;
        Size    = sizeof (USB_Descriptor_Configuration_t);
        break;
    case DTYPE_String:
        switch (DescriptorNumber) {
        case STRING_ID_Language:
            Address = &LanguageString;
            Size    = pgm_read_byte (&LanguageString.Header.Size);
            break;
        case STRING_ID_Manufacturer:
            Address = &ManufacturerString;
            Size    = pgm_read_byte (&ManufacturerString.Header.Size);
            break;
        case STRING_ID_Product:
            Address = &ProductString;
            Size    = pgm_read_byte (&ProductString.Header.Size);
            break;
        }

        break;
    }

    *DescriptorAddress = Address;
    return Size;
}
