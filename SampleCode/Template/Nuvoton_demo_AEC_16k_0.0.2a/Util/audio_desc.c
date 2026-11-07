/******************************************************************************
 * @file     audio_desc.c
 * @brief    USBD driver source file
 * @version  1.0.0
 * @date     August, 2018
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
/*!<Includes */
#include <string.h>
#include "Platform.h"
#include "audioclass.h"
#include "usb_audio_const.h"
#include "usb_audio_def.h"


#define WBVAL(x) (x&0xFF), ((x >>8) & 0xFF) 
#define B3VAL(x) (x&0xFF), ((x >>8) & 0xFF), ((x >>16) & 0xFF)  

const uint8_t gu8HidReportDesc[67] = 
{
    0x05, 0x0C,      // Usage Page (Consumer)
    0x09, 0x01,      // Usage(Consumer Control)
    0xA1, 0x01,      // Collection(Application )
    0x15, 0x00,      // Logical Minimum(0x0 )
    0x25, 0x01,      // Logical Maximum(0x1 )
    0x09, 0xE2,      // Usage(Mute)
    0x09, 0xE9,      // Usage(Volume Increment)
    0x09, 0xEA,      // Usage(Volume Decrement)
    0x75, 0x01,      // Report Size(0x1 )
    0x95, 0x03,      // Report Count(0x3 )
    0x81, 0x02,      // Input(Data, Variable, Absolute, No Wrap, Linear, Preferred State, No Null Position, Bit Field)
    0x75, 0x01,      // Report Size(0x1 )
    0x95, 0x05,      // Report Count(0x5 )
    0x81, 0x03,      // Input(Constant, Variable, Absolute) - Padding 
	
    0x09, 0xB0,      // Usage(Play)
    0x09, 0xB7,      // Usage(Stop)
    0x09, 0xCD,      // Usage(Play/Pause)
    0x09, 0xB5,      // Usage(Scan Next Track)
    0x09, 0xB6,      // Usage(Scan Previous Track)
    0x09, 0xB2,      // Usage(Record)
    0x09, 0xB4,      // Usage(Rewind)
    0x09, 0xB3,      // Usage(Fast Forward)
    0x75, 0x01,      // Report Size(0x1 )
    0x95, 0x08,      // Report Count(0x8 )
    0x81, 0x02,      // Input(Data, Variable, Absolute, No Wrap, Linear, Preferred State, No Null Position, Bit Field)
		
    0x09, 0x00,      // Usage(Undefined)
    0x75, 0x08,      // Report Size(0x8 )
    0x95, 0x06,      // Report Count(0x6 )
    0x81, 0x02,      // Input(Data, Variable, Absolute, No Wrap, Linear, Preferred State, No Null Position, Bit Field)
		
    0x09, 0x00,      // Usage(Undefined)
    0x75, 0x08,      // Report Size(0x8 )
    0x95, 0x08,      // Report Count(0x8 )
    0x91, 0x00,
    0xC0
};


/*----------------------------------------------------------------------------*/
/*!<USB Device Descriptor */
const uint8_t gu8DeviceDescriptor[] = 
{
    LEN_DEVICE,        /* bLength */
    DESC_DEVICE,       /* bDescriptorType */
	WBVAL(0x0200),     /* bcdUSB */
    0x00,              /* bDeviceClass */
    0x00,              /* bDeviceSubClass */
    0x00,              /* bDeviceProtocol */
    EP0_MAX_PKT_SIZE,  /* bMaxPacketSize0 */
	WBVAL(USBD_VID),   /* idVendor */
	WBVAL(USBD_PID),   /* idProduct */ 
    0x00, 0x00,        /* bcdDevice */
    0x01,              /* iManufacture */
    0x02,              /* iProduct */
    0x03,              /* iSerialNumber
                          NOTE: The serial number must be different between each MassStorage device. */
    0x01               /* bNumConfigurations */
};

#define HID_REPORT_DESCRIPTOR_SIZE  sizeof(gu8HidReportDesc) 

/*!<USB Configure Descriptor */
const uint8_t gu8ConfigDescriptor[] = 
{
    LEN_CONFIG,        // bLength
    DESC_CONFIG,        // bDescriptorType (Configuration)
    WBVAL(203),  // wTotalLength 
    TOTAL_INTERFACE_NUM,        // bNumInterfaces 
    0x01,        // bConfigurationValue
    0x00,        // iConfiguration (String Index)
    USB_CONFIG_BUS_POWERED,        // bmAttributes
    USB_CONFIG_POWER_MA(100),        // bMaxPower 100mA

    LEN_INTERFACE,        // bLength
    DESC_INTERFACE,        // bDescriptorType (Interface)
    UAC_AC_IFNUM,        // bInterfaceNumber 0
    0x00,        // bAlternateSetting
    0x00,        // bNumEndpoints 0
    USB_DEVICE_CLASS_AUDIO,        // bInterfaceClass (Audio)
    AUDIO_SUBCLASS_AUDIOCONTROL,        // bInterfaceSubClass (Audio Control)
    0x00,        // bInterfaceProtocol
    0x00,        // iInterface (String Index)

    0x0a,        // bLength
    CS_INTERFACE,        // bDescriptorType (See Next Line)
    HEADER,        // bDescriptorSubtype (CS_INTERFACE -> HEADER)
    0x00, 0x01,  // bcdADC 1.00
    WBVAL(72),   // wTotalLength 
    0x02,        // bInCollection 
    REC_IFNUM,        
	PLAY_IFNUM,

    0x0C,        // bLength
    CS_INTERFACE,        // bDescriptorType (See Next Line)
    INPUT_TERMINAL,        // bDescriptorSubtype (CS_INTERFACE -> INPUT_TERMINAL)
    REC_INPUT_ID,        // bTerminalID
    WBVAL(IN_TERM_MIC),  // wTerminalType (Microphone)
    0x00,        // bAssocTerminal
    REC_CHANNELS,        // bNrChannels 
    WBVAL(REC_CH_CFG),  // wChannelConfig (Left and Right Front,Center Front,Low Freq Enh)
    0x00,        // iChannelNames
    0x04,        // iTerminal

    0x0a,        // bLength
    CS_INTERFACE,        // bDescriptorType (See Next Line)
    FEATURE_UNIT,        // bDescriptorSubtype (CS_INTERFACE -> FEATURE_UNIT)
    REC_FEATURE_UNITID,        // bUnitID
    REC_INPUT_ID,        // bSourceID
    0x01,        // bControlSize 1
    0x01,        // Mute 
	0x00,        // Mute
    0x00,		 // Mute
	0x00,  

    0x09,        // bLength
    CS_INTERFACE,        // bDescriptorType (See Next Line)
    OUTPUT_TERMINAL,        // bDescriptorSubtype (CS_INTERFACE -> OUTPUT_TERMINAL)
    REC_OUTPUT_ID,        // bTerminalID
    WBVAL(USB_STREAMING),  // wTerminalType (USB Streaming)
    0x00,        // bAssocTerminal
    REC_FEATURE_UNITID,        // bSourceID
    0x00,        // iTerminal

    0x0C,                           // bLength 12
    CS_INTERFACE,                   // bDescriptorType:CS_INTERFACE 
    INPUT_TERMINAL,                 // bDescriptorSubType:INPUT_TERMINAL 
    PLAY_INPUT_ID,         	    // bTerminalID  **
    WBVAL(USB_STREAMING),           // wTerminalType (USB Streaming)  **
    0x00,                           // bAssocTerminal  **
    PLAY_CHANNELS,      	        // bNrChannels ** (Stereo)
    WBVAL(PLAY_CH_CFG),  	        // wChannelConfig ** (Left Front and Right Front)
    0x00,                           // iChannelNames 
    0x00,                           // iTerminal 

    /* Feature Unit */
    0x0A,                           // bLength 
    CS_INTERFACE,                   // bDescriptorType 
    FEATURE_UNIT,                           // bDescriptorSubType 
    PLAY_FEATURE_UNITID,            // bUnitID   **
    PLAY_INPUT_ID,                 // bSourceID  **
    0x01,                           // bControlSize  **
    0x01,                           // bmaControls(0)  ** (mute, volume)
    0x00,                           // bmaControls(1)  **
    0x00,                           // bmaControls(2)  **
    0x00,                           // iFeature */

    /* Output for Speakerphone */  
    0x09,                           // bLength
    CS_INTERFACE,                   // bDescriptorType:CS_INTERFACE
    OUTPUT_TERMINAL,                // bDescriptorSubType:OUTPUT_TERMINAL
    PLAY_OUTPUT_ID,                 // bTerminalID
    WBVAL(OUT_TERM_SPK),            // wTerminalType: Headphone  **
    0x00,                           // bAssocTerminal  **
    PLAY_FEATURE_UNITID,            // bSourceID  **
    0x00,                           // iTerminal  
	
    LEN_INTERFACE,        // bLength
    DESC_INTERFACE,        // bDescriptorType (Interface)
    REC_IFNUM,        // bInterfaceNumber 
    0x00,        // bAlternateSetting
    0x00,        // bNumEndpoints 0
    USB_DEVICE_CLASS_AUDIO,        // bInterfaceClass (Audio)
    AUDIO_SUBCLASS_AUDIOSTREAMING,        // bInterfaceSubClass (Audio Streaming)
    0x00,        // bInterfaceProtocol
    0x00,        // iInterface (String Index)

    LEN_INTERFACE,        // bLength
    DESC_INTERFACE,        // bDescriptorType (Interface)
    REC_IFNUM,        // bInterfaceNumber 4
    0x01,        // bAlternateSetting
    0x01,        // bNumEndpoints 1
    USB_DEVICE_CLASS_AUDIO,        // bInterfaceClass (Audio)
    AUDIO_SUBCLASS_AUDIOSTREAMING,        // bInterfaceSubClass (Audio Streaming)
    0x00,        // bInterfaceProtocol
    0x00,        // iInterface (String Index)

    0x07,        // bLength
    CS_INTERFACE,        // bDescriptorType (See Next Line)
    AS_GENERAL,        // bDescriptorSubtype (CS_INTERFACE -> AS_GENERAL)
    REC_OUTPUT_ID,        // bTerminalLink
    0x01,        // bDelay 1
    WBVAL(0x0001),   // wFormatTag (PCM)

    0x0b,        // bLength
    CS_INTERFACE,        // bDescriptorType (See Next Line)
    AS_FORMAT_TYPE,        // bDescriptorSubtype (CS_INTERFACE -> FORMAT_TYPE)
    0x01,        // bFormatType 1
    REC_CHANNELS,        // bNrChannels 
    MIC_SUB_FRM,        // bSubFrameSize 
    MIC_BIT_RES,        // bBitResolution 
    0x01,        // bSamFreqType 
    B3VAL(REC_RATE),  // tSamFreq[1] 
	
    0x09,        // bLength
    DESC_ENDPOINT,        // bDescriptorType (See Next Line)
    EP_INPUT | EP_REC,        // bEndpointAddress (IN/D2H)
    0x0D,        // bmAttributes (Isochronous, Sync, Data EP)
    WBVAL(EP_REC_PKT_SIZE),  // wMaxPacketSize 
    0x01,        // bInterval 1 (unit depends on device speed)
    0x00,        // bRefresh
    0x00,        // bSyncAddress

    0x07,        // bLength
    CS_ENDPOINT,        // bDescriptorType (See Next Line)
    EP_GENERAL,        // bDescriptorSubtype (CS_ENDPOINT -> EP_GENERAL)
    0x00,        // bmAttributes (None)
    0x00,        // bLockDelayUnits
    0x00, 0x00,  // wLockDelay 0
	
    /* Standard AS interface 1, alternate 0 */
    LEN_INTERFACE,                  // bLength
    DESC_INTERFACE,                 // bDescriptorType (Interface)
    PLAY_IFNUM,                     // bInterfaceNumber 1
    0x00,                           // bAlternateSetting
    0x00,                           // bNumEndpoints 0
    USB_DEVICE_CLASS_AUDIO,         // bInterfaceClass (Audio)
    AUDIO_SUBCLASS_AUDIOSTREAMING,  // bInterfaceSubClass (Audio Streaming)
    0x00,                           // bInterfaceProtocol
    0x00,                           // iInterface (String Index)

    /* Standard AS interface 1, alternate 1 */
    LEN_INTERFACE,                  // bLength
    DESC_INTERFACE,                 // bDescriptorType (Interface)
    PLAY_IFNUM,                     // bInterfaceNumber 1
    0x01,                           // bAlternateSetting
    0x02,                           // bNumEndpoints 2  **
    USB_DEVICE_CLASS_AUDIO,         // bInterfaceClass (Audio)
    AUDIO_SUBCLASS_AUDIOSTREAMING,  // bInterfaceSubClass (Audio Streaming)
    0x00,                           // bInterfaceProtocol
    0x00,                           // iInterface (String Index)

	/* Class-spec AS interface */
    0x07,                           // bLength
    CS_INTERFACE,                   // bDescriptorType (CS_INTERFACE)
    AS_GENERAL,                     // bDescriptorSubtype (AS_GENERAL)
    PLAY_INPUT_ID,                 // bTerminalLink  **
    0x01,                           // bDelay ()  **
    WBVAL(0x0001),                  // wFormatTag (PCM)

	/* Type I format type Descriptor */
    0x0B,                           // bLength
    CS_INTERFACE,                   // bDescriptorType (CS_INTERFACE)
    AS_FORMAT_TYPE,                 // bDescriptorSubtype (FORMAT_TYPE)
    0x01,                           // bFormatType 1
    PLAY_CHANNELS,                  // bNrChannels  **
    PLAY_SUB_FRM,                   // bSubFrameSize **
    PLAY_BIT_RES,    		        // bBitResolution   **
    0x01,                           // bSamFreqType 1  **
    B3VAL(PLAY_RATE),					// tSamFreq   **

    /* Standard AS ISO Audio Data Endpoint */
    0x09,                           // bLength
    DESC_ENDPOINT,                  // bDescriptorType (ENDPOINT)
    EP_OUTPUT | EP_SPK,             // bEndpointAddress (OUT/H2D)  **
    0x05,                           // bmAttributes (ISOCHRONOUS , ASync, Data EP)  **
    WBVAL(EP_SPK_PKT_SIZE),        // wMaxPacketSize   **
    0x01,                           // bInterval 1 (unit depends on device speed)
    0x00,                           // bRefresh
    EP_INPUT | EP_SYNC,            // bSyncAddress

    /* Class-spec AS ISO Audio Data endpoint Descriptor */
    0x07,                           // bLength
    CS_ENDPOINT,                    // bDescriptorType (CS_ENDPOINT)
    EP_GENERAL,                     // bDescriptorSubtype (EP_GENERAL)
    0x01,                           // bmAttributes ()
    0x00,                           // bLockDelayUnits
    0x00, 0x00,                     // wLockDelay 0

    0x09, 							/* bLength */ 
    DESC_ENDPOINT,					/* bDescriptorType */
    (EP_INPUT | EP_SYNC),   		/* bEndpointAddress */
    0x11,							/* bmAttributes */
    WBVAL(EP_SYNC_PKT_SIZE),  		/* wMaxPacketSize */
    0x01,							/* bInterval */
    0x05,							/* bRefresh */	  
    0x00,	    	
};

/*!<USB Language String Descriptor */
const uint8_t gu8StringLang[4] = 
{
    4,              /* bLength */
    DESC_STRING,    /* bDescriptorType */
    0x09, 0x04
};

/*!<USB Vendor String Descriptor */
const uint8_t gu8VendorStringDesc[16] = 
{
    16,
    DESC_STRING,
    'N', 0, 'u', 0, 'v', 0, 'o', 0, 't', 0, 'o', 0, 'n', 0
};

/*!<USB Product String Descriptor */
const uint8_t gu8ProductStringDesc[] = {
    24,             /* bLength          */
    DESC_STRING,    /* bDescriptorType  */
    'N', 0,
    'u', 0,
    'v', 0,
    'o', 0,
    't', 0,
    'o', 0,
    'n', 0,
    ' ', 0,
    'U', 0, 
	'A', 0, 
	'C', 0, 
	' ', 0, 
};


uint8_t gu8StringSerial[26] = 
{
    26,             // bLength
    DESC_STRING,    // bDescriptorType
    'A', 0, '0', 0, '0', 0, '0', 0, '0', 0, '8', 0, '0', 0, '4', 0, '0', 0, '1', 0, '1', 0, '5', 0

};


uint8_t gu8DeviceString[] = 
{
    28,             // bLength
    DESC_STRING,    // bDescriptorType
    'C', 0, 'o', 0, 'l', 0, 's', 0, 'o', 0, 'n', 0, 'i', 0, 'c', 0, ' ', 0, 'd', 0, 'e', 0, 'm', 0, 'o', 0,

};

/*!<USB BOS Descriptor */
uint8_t gu8BOSDescriptor[] = 
{
    LEN_BOS,        /* bLength */
    DESC_BOS,       /* bDescriptorType */
    /* wTotalLength */
    0x0C & 0x00FF,
    (0x0C & 0xFF00) >> 8,
    0x01,           /* bNumDeviceCaps */

    /* Device Capability */
    0x7,            /* bLength */
    DESC_CAPABILITY,/* bDescriptorType */
    CAP_USB20_EXT,  /* bDevCapabilityType */
    0x02, 0x00, 0x00, 0x00  /* bmAttributes */
};


const uint8_t *gpu8UsbString[] = {
    gu8StringLang,
    gu8VendorStringDesc,
    gu8ProductStringDesc,
    gu8StringSerial,
	gu8DeviceString,
};

const uint8_t *gu8UsbHidReport[8] = {
    NULL,
    NULL,
    NULL,
    NULL,
	NULL,
    NULL,
    gu8HidReportDesc,
    NULL    
};

const uint32_t gu32UsbHidReportLen[8] = {
    0,
    0,
    0,
    0,
    0,
    0,
    sizeof(gu8HidReportDesc),
    0
};

const uint32_t gu32ConfigHidDescIdx[4] = {
    0,
    0,
    0,
    (sizeof(gu8ConfigDescriptor) - LEN_ENDPOINT - LEN_HID)
};


const S_USBD_INFO_T gsInfo = 
{
    (uint8_t *)gu8DeviceDescriptor,
    (uint8_t *)gu8ConfigDescriptor,
    (const uint8_t **)gpu8UsbString,
    (const uint8_t **)gu8UsbHidReport,
    (uint8_t *)gu8BOSDescriptor,
    (uint32_t *)gu32UsbHidReportLen,
    (uint32_t *)gu32ConfigHidDescIdx
};

