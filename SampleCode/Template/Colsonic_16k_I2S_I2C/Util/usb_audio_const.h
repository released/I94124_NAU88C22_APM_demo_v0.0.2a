#ifndef __USB_AUDIO_CONST_H__
#define __USB_AUDIO_CONST_H__

/* Audio Interface Subclass Codes */
#define AUDIO_SUBCLASS_UNDEFINED               0x00
#define AUDIO_SUBCLASS_AUDIOCONTROL            0x01
#define AUDIO_SUBCLASS_AUDIOSTREAMING          0x02
#define AUDIO_SUBCLASS_MIDISTREAMING           0x03

/* Audio Interface Protocol Codes */
#define AUDIO_PROTOCOL_UNDEFINED               0x00


/* bmAttributes in Configuration Descriptor */
#define USB_CONFIG_POWERED_MASK                0x40
#define USB_CONFIG_BUS_POWERED                 0x80
#define USB_CONFIG_SELF_POWERED                0xC0
#define USB_CONFIG_REMOTE_WAKEUP               0x20

/* bMaxPower in Configuration Descriptor */
#define USB_CONFIG_POWER_MA(mA)                ((mA)/2)

/* Audio Class-Specific Descriptor Types */
#define CS_INTERFACE        0x24
#define CS_ENDPOINT         0x25

/* Audio Class-Specific AS Interface Descriptor Subtypes */
#define AS_GENERAL 			0x01
#define AS_FORMAT_TYPE	    0x02		

/* Audio Class-Specific Endpoint Descriptor Subtypes */
#define EP_GENERAL 			0x01

/* Audio Class-Specific AC Interface Descriptor Subtypes */
#define HEADER				0x01
#define INPUT_TERMINAL	    0x02
#define OUTPUT_TERMINAL	    0x03
#define FEATURE_UNIT		0x06

/* USB Terminal Types */
#define USB_STREAMING       0x101

/* Input Terminal Types */
#define IN_TERM_MIC         0x201

/* Output Terminal Types */
#define OUT_TERM_SPK        0x301

// Unit ID  
#define PLAY_INPUT_ID   	   0x01
#define PLAY_OUTPUT_ID   	   0x03
#define PLAY_FEATURE_UNITID    0x02
#define REC_INPUT_ID 	       0x04
#define REC_OUTPUT_ID 	       0x06
#define REC_FEATURE_UNITID     0x05

/********************************************/
/* Audio Class Current State                */
/********************************************/
/*!<Define Audio Class Current State */
#define UAC_STOP_AUDIO_RECORD           0
#define UAC_START_AUDIO_RECORD          1
#define UAC_PROCESSING_AUDIO_RECORD     2
#define UAC_PROCESS_AUDIO_RECORD        2
#define UAC_READY_AUDIO_RECORD          3
#define UAC_BUSY_AUDIO_RECORD           4


#define UAC_STOP_AUDIO_SPEAK            0
#define UAC_START_AUDIO_SPEAK           1
#define UAC_PROCESS1_AUDIO_SPEAK        2
#define UAC_PROCESS2_AUDIO_SPEAK        3
#define UAC_BUSY_AUDIO_SPEAK            4


/***************************************************/
/*      Audio Class-Specific Request Codes         */
/***************************************************/
/*!<Define Audio Class Specific Request */
#define UAC_REQUEST_CODE_UNDEFINED  0x00
#define UAC_SET_CUR                 0x01
#define UAC_GET_CUR                 0x81
#define UAC_SET_MIN                 0x02
#define UAC_GET_MIN                 0x82
#define UAC_SET_MAX                 0x03
#define UAC_GET_MAX                 0x83
#define UAC_SET_RES                 0x04
#define UAC_GET_RES                 0x84
#define UAC_SET_MEM                 0x05
#define UAC_GET_MEM                 0x85
#define UAC_GET_STAT                0xFF

/*!<Define HID Class Specific Request */
#define HID_SET_REPORT              0x09
#define HID_SET_IDLE                0x0A
#define HID_SET_PROTOCOL            0x0B

#define MUTE_CONTROL                0x01
#define VOLUME_CONTROL              0x02

#endif
