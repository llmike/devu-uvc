/*
 * Copyright 2013-2014 Mike Gorchak
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You
 * may not reproduce, modify or distribute this software except in
 * compliance with the License. You may obtain a copy of the License
 * at: http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied,
 *
 * This file may contain contributions from others, either as
 * contributors under the License or as licensors under other terms.
 * Please review this entire file for other proprietary rights or license
 * notices, as well as the QNX Development Suite License Guide at
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

#ifndef __USBVC_H__
#define __USBVC_H__

/* All definitions are from "USB Device Class Definition for Video Devices" */
/* Appendix A. Video Device Class Codes                                     */

/* A1 Video interface class code */
#define VCC_VIDEO                                                  0x0000000E

/* A2 Video interface subclass codes */
#define VSC_UNDEFINED                                              0x00000000
#define VSC_VIDEOCONTROL                                           0x00000001
#define VSC_VIDEOSTREAMING                                         0x00000002
#define VSC_VIDEO_INTERFACE_COLLECTION                             0x00000003

/* A3 Video interface protocol codes */
#define VPC_PROTOCOL_UNDEFINED                                     0x00000000
#define VPC_PROTOCOL_I5                                            0x00000001

/* A4 Video class-specific descriptor types */
#define VCS_UNDEFINED                                              0x00000020
#define VCS_DEVICE                                                 0x00000021
#define VCS_CONFIGURATION                                          0x00000022
#define VCS_STRING                                                 0x00000023
#define VCS_INTERFACE                                              0x00000024
#define VCS_ENDPOINT                                               0x00000025

/* A5 Video class-specific VC interface descriptor subtypes */
#define VVC_DESCRIPTOR_UNDEFINED                                   0x00000000
#define VVC_HEADER                                                 0x00000001
#define VVC_INPUT_TERMINAL                                         0x00000002
#define VVC_OUTPUT_TERMINAL                                        0x00000003
#define VVC_SELECTOR_UNIT                                          0x00000004
#define VVC_PROCESSING_UNIT                                        0x00000005
#define VVC_EXTENSION_UNIT                                         0x00000006
#define VVC_ENCODING_UNIT                                          0x00000007

/* A6 Video class-specific VS interface descriptor subtypes */
#define VVS_UNDEFINED                                              0x00000000
#define VVS_INPUT_HEADER                                           0x00000001
#define VVS_OUTPUT_HEADER                                          0x00000002
#define VVS_STILL_IMAGE_FRAME                                      0x00000003
#define VVS_FORMAT_UNCOMPRESSED                                    0x00000004
#define VVS_FRAME_UNCOMPRESSED                                     0x00000005
#define VVS_FORMAT_MJPEG                                           0x00000006
#define VVS_FRAME_MJPEG                                            0x00000007
#define VVS_RESERVED1                                              0x00000008
#define VVS_RESERVED2                                              0x00000009
#define VVS_FORMAT_MPEG2TS                                         0x0000000A
#define VVS_RESERVED3                                              0x0000000B
#define VVS_FORMAT_DV                                              0x0000000C
#define VVS_COLORFORMAT                                            0x0000000D
#define VVS_RESERVED4                                              0x0000000E
#define VVS_RESERVED5                                              0x0000000F
#define VVS_FORMAT_FRAME_BASED                                     0x00000010
#define VVS_FRAME_FRAME_BASED                                      0x00000011
#define VVS_FORMAT_STREAM_BASED                                    0x00000012
#define VVS_FORMAT_H264                                            0x00000013
#define VVS_FRAME_H264                                             0x00000014
#define VVS_FORMAT_H264_SIMULCAST                                  0x00000015
#define VVS_FORMAT_VP8                                             0x00000016
#define VVS_FRAME_VP8                                              0x00000017
#define VVS_FORMAT_VP8_SIMULCAST                                   0x00000018

/* A7 Video class-specific endpoint descriptor subtypes */
#define VEP_UNDEFINED                                              0x00000000
#define VEP_GENERAL                                                0x00000001
#define VEP_ENDPOINT                                               0x00000002
#define VEP_INTERRUPT                                              0x00000003

/* A8 Video class-specific request codes */
#define VRC_UNDEFINED                                              0x00000000
#define VSET_CUR                                                   0x00000001
#define VGET_CUR                                                   0x00000081
#define VGET_MIN                                                   0x00000082
#define VGET_MAX                                                   0x00000083
#define VGET_RES                                                   0x00000084
#define VGET_LEN                                                   0x00000085
#define VGET_INFO                                                  0x00000086
#define VGET_DEF                                                   0x00000087
#define VGET_CUR_ALL                                               0x00000091
#define VGET_MIN_ALL                                               0x00000092
#define VGET_MAX_ALL                                               0x00000093
#define VGET_RES_ALL                                               0x00000094
#define VGET_DEF_ALL                                               0x00000097

/* A9.1 VideoControl (VC) interface control selectors */
#define VVC_CONTROL_UNDEFINED                                      0x00000000
#define VVC_VIDEO_POWER_MODE_CONTROL                               0x00000001
#define VVC_REQUEST_ERROR_CODE_CONTROL                             0x00000002
#define VVC_RESERVED1                                              0x00000003

/* A9.2 Terminal control selectors */
#define VTE_CONTROL_UNDEFINED                                      0x00000000

/* A9.3 Selector unit control selectors */
#define VSU_CONTROL_UNDEFINED                                      0x00000000
#define VSU_INPUT_SELECT_CONTROL                                   0x00000001

/* A9.4 Camera terminal control selectors */
#define VCT_CONTROL_UNDEFINED                                      0x00000000
#define VCT_SCANNING_MODE_CONTROL                                  0x00000001
#define VCT_AE_MODE_CONTROL                                        0x00000002
#define VCT_AE_PRIORITY_CONTROL                                    0x00000003
#define VCT_EXPOSURE_TIME_ABSOLUTE_CONTROL                         0x00000004
#define VCT_EXPOSURE_TIME_RELATIVE_CONTROL                         0x00000005
#define VCT_FOCUS_ABSOLUTE_CONTROL                                 0x00000006
#define VCT_FOCUS_RELATIVE_CONTROL                                 0x00000007
#define VCT_FOCUS_AUTO_CONTROL                                     0x00000008
#define VCT_IRIS_ABSOLUTE_CONTROL                                  0x00000009
#define VCT_IRIS_RELATIVE_CONTROL                                  0x0000000A
#define VCT_ZOOM_ABSOLUTE_CONTROL                                  0x0000000B
#define VCT_ZOOM_RELATIVE_CONTROL                                  0x0000000C
#define VCT_PANTILT_ABSOLUTE_CONTROL                               0x0000000D
#define VCT_PANTILT_RELATIVE_CONTROL                               0x0000000E
#define VCT_ROLL_ABSOLUTE_CONTROL                                  0x0000000F
#define VCT_ROLL_RELATIVE_CONTROL                                  0x00000010
#define VCT_PRIVACY_CONTROL                                        0x00000011
#define VCT_FOCUS_SIMPLE_CONTROL                                   0x00000012
#define VCT_WINDOW_CONTROL                                         0x00000013
#define VCT_REGION_OF_INTEREST_CONTROL                             0x00000014

/* A9.5 processing unit control selectors */
#define VPU_CONTROL_UNDEFINED                                      0x00000000
#define VPU_BACKLIGHT_COMPENSATION_CONTROL                         0x00000001
#define VPU_BRIGHTNESS_CONTROL                                     0x00000002
#define VPU_CONTRAST_CONTROL                                       0x00000003
#define VPU_GAIN_CONTROL                                           0x00000004
#define VPU_POWER_LINE_FREQUENCY_CONTROL                           0x00000005
#define VPU_HUE_CONTROL                                            0x00000006
#define VPU_SATURATION_CONTROL                                     0x00000007
#define VPU_SHARPNESS_CONTROL                                      0x00000008
#define VPU_GAMMA_CONTROL                                          0x00000009
#define VPU_WHITE_BALANCE_TEMPERATURE_CONTROL                      0x0000000A
#define VPU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL                 0x0000000B
#define VPU_WHITE_BALANCE_COMPONENT_CONTROL                        0x0000000C
#define VPU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL                   0x0000000D
#define VPU_DIGITAL_MULTIPLIER_CONTROL                             0x0000000E
#define VPU_DIGITAL_MULTIPLIER_LIMIT_CONTROL                       0x0000000F
#define VPU_HUE_AUTO_CONTROL                                       0x00000010
#define VPU_ANALOG_VIDEO_STANDARD_CONTROL                          0x00000011
#define VPU_ANALOG_LOCK_STATUS_CONTROL                             0x00000012
#define VPU_CONTRAST_AUTO_CONTROL                                  0x00000013

/* A9.6 encoding unit control selectors */
#define VEU_CONTROL_UNDEFINED                                      0x00000000
#define VEU_SELECT_LAYER_CONTROL                                   0x00000001
#define VEU_PROFILE_TOOLSET_CONTROL                                0x00000002
#define VEU_VIDEO_RESOLUTION_CONTROL                               0x00000003
#define VEU_MIN_FRAME_INTERVAL_CONTROL                             0x00000004
#define VEU_SLICE_MODE_CONTROL                                     0x00000005
#define VEU_RATE_CONTROL_MODE_CONTROL                              0x00000006
#define VEU_AVERAGE_BITRATE_CONTROL                                0x00000007
#define VEU_CPB_SIZE_CONTROL                                       0x00000008
#define VEU_PEAK_BIT_RATE_CONTROL                                  0x00000009
#define VEU_QUANTIZATION_PARAMS_CONTROL                            0x0000000A
#define VEU_SYNC_REF_FRAME_CONTROL                                 0x0000000B
#define VEU_LTR_BUFFER_CONTROL                                     0x0000000C
#define VEU_LTR_PICTURE_CONTROL                                    0x0000000D
#define VEU_LTR_VALIDATION_CONTROL                                 0x0000000E
#define VEU_LEVEL_IDC_LIMIT_CONTROL                                0x0000000F
#define VEU_SEI_PAYLOADTYPE_CONTROL                                0x00000010
#define VEU_QP_RANGE_CONTROL                                       0x00000011
#define VEU_PRIORITY_CONTROL                                       0x00000012
#define VEU_START_OR_STOP_LAYER_CONTROL                            0x00000013
#define VEU_ERROR_RESILIENCY_CONTROL                               0x00000014

/* A9.7 extension unit control selectors */
#define VXU_CONTROL_UNDEFINED                                      0x00000000

/* A9.8 videostreaming interface control selectors */
#define VVS_CONTROL_UNDEFINED                                      0x00000000
#define VVS_PROBE_CONTROL                                          0x00000001
#define VVS_COMMIT_CONTROL                                         0x00000002
#define VVS_STILL_PROBE_CONTROL                                    0x00000003
#define VVS_STILL_COMMIT_CONTROL                                   0x00000004
#define VVS_STILL_IMAGE_TRIGGER_CONTROL                            0x00000005
#define VVS_STREAM_ERROR_CODE_CONTROL                              0x00000006
#define VVS_GENERATE_KEY_FRAME_CONTROL                             0x00000007
#define VVS_UPDATE_FRAME_SEGMENT_CONTROL                           0x00000008
#define VVS_SYNCH_DELAY_CONTROL                                    0x00000009

/* All definitions are from "USB Device Class Definition for Video Devices" */
/* Appendix B. Terminal Types                                               */

/* B1 usb terminal types */
#define VTT_VENDOR_SPECIFIC                                        0x00000100
#define VTT_STREAMING                                              0x00000101

/* B2 input terminal types */
#define VITT_VENDOR_SPECIFIC                                       0x00000200
#define VITT_CAMERA                                                0x00000201
#define VITT_MEDIA_TRANSPORT_UNIT                                  0x00000202

/* B3 output terminal types */
#define VOTT_VENDOR_SPECIFIC                                       0x00000300
#define VOTT_DISPLAY                                               0x00000301
#define VOTT_MEDIA_TRANSPORT_UNIT                                  0x00000302

/* B4 external terminal types */
#define VETT_VENDOR_SPECIFIC                                       0x00000400
#define VETT_COMPOSITE_CONNECTOR                                   0x00000401
#define VETT_SVIDEO_CONNECTOR                                      0x00000402
#define VETT_COMPONENT_CONNECTOR                                   0x00000403

/* All definitions are from "USB Device Class Definition for Video Devices: */
/* Uncompressed payload"                                                    */
#define VUP_GUID_FORMAT_YUY2 {'Y', 'U', 'Y', '2', 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}
#define VUP_GUID_FORMAT_NV12 {'N', 'V', '1', '2', 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}
/* Frame based payload */
#define VUP_GUID_FORMAT_H264  {'H', '2', '6', '4', 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}
#define VUP_GUID_XU_CONTROL_H264 {0x41, 0x76, 0x9E, 0xA2, 0x04, 0xDE, 0xE3, 0x47, 0x8B, 0x2B, 0xF4, 0x34, 0x1A, 0xFF, 0x00, 0x3B}

/* Help macros for input terminal capabilities                              */
#define VIT_B0_SCANNING_MODE                                       0x00000001
#define VIT_B0_AUTO_EXPOSURE_MODE                                  0x00000002
#define VIT_B0_AUTO_EXPOSURE_PRIORITY                              0x00000004
#define VIT_B0_EXPOSURE_TIME_ABSOLUTE                              0x00000008
#define VIT_B0_EXPOSURE_TIME_RELATIVE                              0x00000010
#define VIT_B0_FOCUS_ABSOLUTE                                      0x00000020
#define VIT_B0_FOCUS_RELATIVE                                      0x00000040
#define VIT_B0_IRIS_ABSOLUTE                                       0x00000080
#define VIT_B1_IRIS_RELATIVE                                       0x00000001
#define VIT_B1_ZOOM_ABSOLUTE                                       0x00000002
#define VIT_B1_ZOOM_RELATIVE                                       0x00000004
#define VIT_B1_PANTILT_ABSOLUTE                                    0x00000008
#define VIT_B1_PANTILT_RELATIVE                                    0x00000010
#define VIT_B1_ROLL_ABSOLUTE                                       0x00000020
#define VIT_B1_ROLL_RELATIVE                                       0x00000040
#define VIT_B1_RESERVED                                            0x00000080
#define VIT_B2_RESERVED                                            0x00000001
#define VIT_B2_FOCUS_AUTO                                          0x00000002
#define VIT_B2_PRIVACY                                             0x00000004
#define VIT_B2_FOCUS_SIMPLE                                        0x00000008
#define VIT_B2_WINDOW                                              0x00000010
#define VIT_B2_REGION_OF_INTEREST                                  0x00000020
#define VIT_B2_RESERVED2                                           0x00000040
#define VIT_B2_RESERVED3                                           0x00000080

/* Help macros for processing unit capabilities                             */
#define VPU_B0_BRIGHTNESS                                          0x00000001
#define VPU_B0_CONTRAST                                            0x00000002
#define VPU_B0_HUE                                                 0x00000004
#define VPU_B0_SATURATION                                          0x00000008
#define VPU_B0_SHARPNESS                                           0x00000010
#define VPU_B0_GAMMA                                               0x00000020
#define VPU_B0_WHITE_BALANCE_TEMPERATURE                           0x00000040
#define VPU_B0_WHITE_BALANCE_COMPONENT                             0x00000080
#define VPU_B1_BACKLIGHT_COMPENSATION                              0x00000001
#define VPU_B1_GAIN                                                0x00000002
#define VPU_B1_POWER_LINE_FREQUENCY                                0x00000004
#define VPU_B1_HUE_AUTO                                            0x00000008
#define VPU_B1_WHITE_BALANCE_TEMPERATURE_AUTO                      0x00000010
#define VPU_B1_WHITE_BALANCE_COMPONENT_AUTO                        0x00000020
#define VPU_B1_DIGITAL_MULTIPLIER                                  0x00000040
#define VPU_B1_DIGITAL_MULTIPLIER_LIMIT                            0x00000080
#define VPU_B2_ANALOG_VIDEO_STANDARD                               0x00000001
#define VPU_B2_ANALOG_VIDEO_LOCK_STATUS                            0x00000002
#define VPU_B2_CONTRAST_AUTO                                       0x00000004
#define VPU_B2_RESERVED                                            0x00000008
#define VPU_B2_RESERVED2                                           0x00000010
#define VPU_B2_RESERVED3                                           0x00000020
#define VPU_B2_RESERVED4                                           0x00000040
#define VPU_B2_RESERVED5                                           0x00000080

/* Help macros for encoding unit capabilities                               */
#define VEU_B0_SELECT_LAYER                                        0x00000001
#define VEU_B0_PROFILE_AND_TOOLSET                                 0x00000002
#define VEU_B0_VIDEO_RESOLUTION                                    0x00000004
#define VEU_B0_MINIMUM_FRAME_INTERVAL                              0x00000008
#define VEU_B0_SLICE_MODE                                          0x00000010
#define VEU_B0_RATE_CONTROL_MODE                                   0x00000020
#define VEU_B0_AVERAGE_BIT_RATE                                    0x00000040
#define VEU_B0_CPB_SIZE                                            0x00000080
#define VEU_B1_PEAK_BIT_RATE                                       0x00000001
#define VEU_B1_QUANTIZATION_PARAMETER                              0x00000002
#define VEU_B1_SYNCHRONIZATION_AND_LT_REFERENCE_FRAME              0x00000004
#define VEU_B1_LONG_TERM_BUFFER                                    0x00000008
#define VEU_B1_PICTURE_LONG_TERM_REFERENCE                         0x00000010
#define VEU_B1_LTR_VALIDATION                                      0x00000020
#define VEU_B1_LEVEL_IDC                                           0x00000040
#define VEU_B1_SEI_MESSAGE                                         0x00000080
#define VEU_B2_QP_RANGE                                            0x00000001
#define VEU_B2_PRIORITY_ID                                         0x00000002
#define VEU_B2_START_STOP_LAYER_VIEW                               0x00000004
#define VEU_B2_ERROR_RESILIENCY                                    0x00000008

/* Help macros for color format properties                                  */
#define VCFCP_UNKNOWN                                              0x00000000
#define VCFCP_BT_709                                               0x00000001
#define VCFCP_SRGB                                                 0x00000001
#define VCFCP_BT_470_2_M                                           0x00000002
#define VCFCP_BT_470_2_BG                                          0x00000003
#define VCFCP_SMPTE_170M                                           0x00000004
#define VCFCP_SMPTE_240M                                           0x00000005

#define VCFTC_UNKNOWN                                              0x00000000
#define VCFTC_BT_709                                               0x00000001
#define VCFTC_BT_470_2_M                                           0x00000002
#define VCFTC_BT_470_2_BG                                          0x00000003
#define VCFTC_SMPTE_170M                                           0x00000004
#define VCFTC_SMPTE_240M                                           0x00000005
#define VCFTC_BT_LINEAR                                            0x00000006
#define VCFTC_SRGB                                                 0x00000007

#define VCFMC_UNKNOWN                                              0x00000000
#define VCFMC_BT_709                                               0x00000001
#define VCFMC_FCC                                                  0x00000002
#define VCFMC_BT_470_2_BG                                          0x00000003
#define VCFMC_SMPTE_170M                                           0x00000004
#define VCFMC_BT_601                                               0x00000004
#define VCFMC_SMPTE_240M                                           0x00000005

/* Help macros for supported video standards by PU                          */
#define VPU_VSTD_NTSC_525_60                                       0x00000001
#define VPU_VSTD_PAL_625_50                                        0x00000002
#define VPU_VSTD_SECAM_625_50                                      0x00000004
#define VPU_VSTD_NTSC_625_50                                       0x00000008
#define VPU_VSTD_PAL_525_60                                        0x00000010

#define VPU_STD_NTSC_525_60                                        0x00000001
#define VPU_STD_PAL_625_50                                         0x00000002
#define VPU_STD_SECAM_625_50                                       0x00000003
#define VPU_STD_NTSC_625_50                                        0x00000004
#define VPU_STD_PAL_525_60                                         0x00000005

/* Get/Set requests                                                         */

#define UVC_REQUEST_SET_VC                                         0x00000021
#define UVC_REQUEST_GET_VC                                         0x000000A1
#define UVC_REQUEST_SET_VS                                         0x00000022
#define UVC_REQUEST_GET_VS                                         0x000000A2

/* Interrupt packets */
#define UVC_INTERRUPT_ORIGINATOR_MASK                              0x00000003
#define UVC_INTERRUPT_ORIGINATOR_VC                                0x00000001
#define UVC_INTERRUPT_ORIGINATOR_VS                                0x00000002

#define UVC_INTERRUPT_VC_EVENT_CONTROL_CHANGE                      0x00000000

#define UVC_INTERRUPT_VS_EVENT_BUTTON_PRESS                        0x00000000
#define UVC_INTERRUPT_VS_BUTTON_PRESS                              0x00000001

#define UVC_INTERRUPT_VC_CONTROL_VALUE_CHANGE                      0x00000000
#define UVC_INTERRUPT_VC_CONTROL_INFO_CHANGE                       0x00000001
#define UVC_INTERRUPT_VC_CONTROL_FAILURE_CHANGE                    0x00000002
#define UVC_INTERRUPT_VC_CONTROL_MIN_CHANGE                        0x00000003
#define UVC_INTERRUPT_VC_CONTROL_MAX_CHANGE                        0x00000004

#endif /* __USBVC_H__ */
