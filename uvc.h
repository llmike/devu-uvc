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

#ifndef __UVC_H__
#define __UVC_H__

struct _uvc_device;
struct _uvc_ocb;
#define IOFUNC_ATTR_T   struct _uvc_device
#define IOFUNC_OCB_T    struct _uvc_ocb

#include <stdint.h>
#include <pthread.h>
#include <sys/usbdi.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/resmgr.h>

#include <linux/media.h>
#include <linux/videodev2.h>

#include "bsdqueue.h"
#include "uuid.h"

#define UVC_MAX_INTERFACES_COUNT 8

typedef struct _vc_header
{
    uint16_t bcdUVC;
    uint16_t wTotalLength;
    uint32_t dwClockFrequency;
    uint8_t  bInCollection;
    uint8_t  baInterfaceNr[UVC_MAX_INTERFACES_COUNT];
} vc_header_t;

#define UVC_MAX_BMCONTROLS_COUNT 8

typedef struct _vc_input_terminal
{
    uint8_t  bTerminalID;
    uint16_t wTerminalType;
    uint8_t  bAssocTerminal;
    uint8_t  iTerminal;
    uint16_t wObjectiveFocalLengthMin;
    uint16_t wObjectiveFocalLengthMax;
    uint16_t wOcularFocalLength;
    uint8_t  bControlSize;
    uint8_t  bmControls[UVC_MAX_BMCONTROLS_COUNT];
} vc_input_terminal_t;

typedef struct _vc_output_terminal
{
    uint8_t  bTerminalID;
    uint16_t wTerminalType;
    uint8_t  bAssocTerminal;
    uint8_t  bSourceID;
    uint8_t  iTerminal;
} vc_output_terminal_t;

#define UVC_MAX_SOURCEIDS_COUNT 8

typedef struct _vc_selector_unit
{
    uint8_t bUnitID;
    uint8_t bNrInPins;
    uint8_t baSourceID[UVC_MAX_SOURCEIDS_COUNT];
    uint8_t iSelector;
} vc_selector_unit_t;

typedef struct _vc_processing_unit
{
    uint8_t  bUnitID;
    uint8_t  bSourceID;
    uint16_t wMaxMultiplier;
    uint8_t  bControlSize;
    uint8_t  bmControls[UVC_MAX_BMCONTROLS_COUNT];
    uint8_t  iProcessing;
    uint8_t  bmVideoStandards;
} vc_processing_unit_t;

#define UVC_MAX_SOURCEPINS_COUNT 32

typedef struct _vc_extension_unit
{
    uint8_t  bUnitID;
    uuid_t   guidExtensionCode;
    uint8_t  bNumControls;
    uint8_t  bNrInPins;
    uint8_t  baSourceID[UVC_MAX_SOURCEPINS_COUNT];
    uint8_t  bControlSize;
    uint8_t  bmControls[UVC_MAX_BMCONTROLS_COUNT];
    uint8_t  iExtension;
} vc_extension_unit_t;

typedef struct _vc_encoding_unit
{
    uint8_t bUnitID;
    uint8_t bSourceID;
    uint8_t iEncoding;
    uint8_t bControlSize;
    uint8_t bmControls[3];
    uint8_t bmControlsRuntime[3];
} vc_encoding_unit_t;

typedef struct _vs_input_header
{
    uint8_t  bNumFormats;
    uint16_t wTotalLength;
    uint8_t  bEndpointAddress;
    uint8_t  bmInfo;
    uint8_t  bTerminalLink;
    uint8_t  bStillCaptureMethod;
    uint8_t  bTriggerSupport;
    uint8_t  bTriggerUsage;
    uint8_t  bControlSize;
    uint8_t  bmaControls[UVC_MAX_BMCONTROLS_COUNT];
} vs_input_header_t;

typedef struct _vs_output_header
{
    uint8_t  bNumFormats;
    uint16_t wTotalLength;
    uint8_t  bEndpointAddress;
    uint8_t  bTerminalLink;
    uint8_t  bControlSize;
    uint8_t  bmaControls[UVC_MAX_BMCONTROLS_COUNT];
} vs_output_header_t;

#define UVC_MAX_SIZE_PATTERNS          32
#define UVC_MAX_COMPRESSION_PATTERNS   8

typedef struct _vs_still_image_frame
{
    uint8_t  bEndpointAddress;
    uint8_t  bNumImageSizePatterns;
    uint16_t wWidth[UVC_MAX_SIZE_PATTERNS];
    uint16_t wHeight[UVC_MAX_SIZE_PATTERNS];
    uint8_t  bNumCompressionPattern;
    uint8_t  bCompression[UVC_MAX_COMPRESSION_PATTERNS];
} vs_still_image_frame_t;

typedef struct _vs_color_format
{
    uint8_t  bColorPrimaries;
    uint8_t  bTransferCharacteristics;
    uint8_t  bMatrixCoefficients;
} vs_color_format_t;

typedef struct _vs_format_uncompressed
{
    uint8_t  bFormatIndex;
    uint8_t  bNumFrameDescriptors;
    uuid_t   guidFormat;
    uint8_t  bBitsPerPixel;
    uint8_t  bDefaultFrameIndex;
    uint8_t  bAspectRatioX;
    uint8_t  bAspectRatioY;
    uint8_t  bmInterlaceFlags;
    uint8_t  bCopyProtect;
} vs_format_uncompressed_t;

#define UVC_MAX_FRAME_INTERVALS 32

typedef struct _vs_frame_uncompressed
{
    uint8_t  bFrameIndex;
    uint8_t  bmCapabilities;
    uint16_t wWidth;
    uint16_t wHeight;
    uint32_t dwMinBitRate;
    uint32_t dwMaxBitRate;
    uint32_t dwMaxVideoFrameBufferSize;
    uint32_t dwDefaultFrameInterval;
    uint8_t  bFrameIntervalType;
    /* continuous frame interval */
    uint32_t dwMinFrameInterval;
    uint32_t dwMaxFrameInterval;
    uint32_t dwFrameIntervalStep;
    /* descrete frame interval */
    uint32_t dwFrameInterval[UVC_MAX_FRAME_INTERVALS];
} vs_frame_uncompressed_t;

typedef struct _vs_format_mjpeg
{
    uint8_t  bFormatIndex;
    uint8_t  bNumFrameDescriptors;
    uint8_t  bmFlags;
    uint8_t  bDefaultFrameIndex;
    uint8_t  bAspectRatioX;
    uint8_t  bAspectRatioY;
    uint8_t  bmInterlaceFlags;
    uint8_t  bCopyProtect;
} vs_format_mjpeg_t;

typedef struct _vs_frame_mjpeg
{
    uint8_t  bFrameIndex;
    uint8_t  bmCapabilities;
    uint16_t wWidth;
    uint16_t wHeight;
    uint32_t dwMinBitRate;
    uint32_t dwMaxBitRate;
    uint32_t dwMaxVideoFrameBufferSize;
    uint32_t dwDefaultFrameInterval;
    uint8_t  bFrameIntervalType;
    /* continuous frame interval */
    uint32_t dwMinFrameInterval;
    uint32_t dwMaxFrameInterval;
    uint32_t dwFrameIntervalStep;
    /* descrete frame interval */
    uint32_t dwFrameInterval[UVC_MAX_FRAME_INTERVALS];
} vs_frame_mjpeg_t;

typedef struct _vs_format_frame_based
{
    uint8_t  bFormatIndex;
    uint8_t  bNumFrameDescriptors;
    uuid_t   guidFormat;
    uint8_t  bBitsPerPixel;
    uint8_t  bDefaultFrameIndex;
    uint8_t  bAspectRatioX;
    uint8_t  bAspectRatioY;
    uint8_t  bmInterlaceFlags;
    uint8_t  bCopyProtect;
    uint8_t  bVariableSize;
} vs_format_frame_t;

typedef struct _vs_frame_frame_based
{
    uint8_t  bFrameIndex;
    uint8_t  bmCapabilities;
    uint16_t wWidth;
    uint16_t wHeight;
    uint32_t dwMinBitRate;
    uint32_t dwMaxBitRate;
    uint32_t dwDefaultFrameInterval;
    uint8_t  bFrameIntervalType;
    uint32_t dwBytesPerLine;
    /* continuous frame interval */
    uint32_t dwMinFrameInterval;
    uint32_t dwMaxFrameInterval;
    uint32_t dwFrameIntervalStep;
    /* descrete frame interval */
    uint32_t dwFrameInterval[UVC_MAX_FRAME_INTERVALS];
} vs_frame_frame_t;

#define UVC_FORMAT_DV_SDDV                           0x00000000
#define UVC_FORMAT_DV_SDLDV                          0x00000001
#define UVC_FORMAT_DV_HDDV                           0x00000002
#define UVC_FORMAT_DV_50HZ                           0x00000000
#define UVC_FORMAT_DV_60HZ                           0x80000000

typedef struct _vs_dv_format
{
    uint8_t  bFormatIndex;
    uint32_t dwMaxVideoFrameBufferSize;
    uint8_t  bFormatType;
} vs_dv_format_t;

#define UVC_PROBE_COMMIT_HINT_DWFRAMEINTERVAL        0x00000001
#define UVC_PROBE_COMMIT_HINT_WKEYFRAMERATE          0x00000002
#define UVC_PROBE_COMMIT_HINT_WPFRAMERATE            0x00000004
#define UVC_PROBE_COMMIT_HINT_WCOMPQUALITY           0x00000008
#define UVC_PROBE_COMMIT_HINT_WCOMPWINDOWSIZE        0x00000010

#define UVC_PROBE_COMMIT_BMFRAMINGINFO_FID           0x00000001
#define UVC_PROBE_COMMIT_BMFRAMINGINFO_EOF           0x00000002

#define UVC_PROBE_COMMIT_VER10_SIZE                  26
#define UVC_PROBE_COMMIT_VER11_SIZE                  34
#define UVC_PROBE_COMMIT_VER15_SIZE                  48

typedef struct _probe_commit_control
{
    uint16_t bmHint;
    uint8_t  bFormatIndex;
    uint8_t  bFrameIndex;
    uint32_t dwFrameInterval;
    uint16_t wKeyFrameRate;
    uint16_t wPFrameRate;
    uint16_t wCompQuality;
    uint16_t wCompWindowSize;
    uint16_t wDelay;
    uint32_t dwMaxVideoFrameSize;
    uint32_t dwMaxPayloadTransferSize;
    uint32_t dwClockFrequency;
    uint8_t  bmFramingInfo;
    uint8_t  bPreferedVersion;
    uint8_t  bMinVersion;
    uint8_t  bMaxVersion;
    uint8_t  bUsage;
    uint8_t  bBitDepthLuma;
    uint8_t  bmSettings;
    uint8_t  bMaxNumberOfRefFramesPlus1;
    uint16_t bmRateControlModes;
    uint64_t bmLayoutPerStream;
} probe_commit_control_t;

typedef struct _uvc_ocb
{
    iofunc_ocb_t        hdr;
    struct _uvc_device* dev;
    void*               handle;
    int                 subdev;
    iofunc_notify_t     notify[3];
} uvc_ocb_t;

#define UVC_MAX_EXTENSION_UNITS 16
#define UVC_MAX_VS_COUNT        UVC_MAX_INTERFACES_COUNT
#define UVC_MAX_FRAMES          32
#define UVC_MAX_ENTITIES        (UVC_MAX_EXTENSION_UNITS*2)
#define UVC_MAX_ENTITY_PADS     8

struct _uvc_device_mapping;
struct _uvc_sysfs_device;
struct _uvc_media_device;

/* native formats */
#define UVC_FORMAT_NONE    (0)
#define UVC_FORMAT_YUY2    (1)
#define UVC_FORMAT_NV12    (2)
#define UVC_FORMAT_MJPG    (3)
#define UVC_FORMAT_H264F   (4)
#define UVC_FORMAT_H264    (5)
#define UVC_FORMAT_DV      (6)
#define UVC_FORMAT_MPEG2TS (7)
#define UVC_FORMAT_VP8     (8)
/* emulated formats */
#define UVC_FORMAT_UYVY    (16+1)
#define UVC_FORMAT_VYUY    (17+1)
#define UVC_FORMAT_YVYU    (18+1)
#define UVC_TOTAL_FORMATS  12

#define UVC_MAX_OPEN_FDS    32
#define UVC_MAX_ISO_BUFFERS 4
#define UVC_MAX_ISO_FRAMES  32

typedef struct _uvc_event_entry
{
    TAILQ_ENTRY(_uvc_event_entry) link;
    struct v4l2_event event;
} uvc_event_entry_t;

typedef struct _uvc_event
{
    uvc_ocb_t* ocb;
    uint32_t type[V4L2_EVENT_FRAME_SYNC+1];
    uint32_t flags[V4L2_EVENT_FRAME_SYNC+1];
    uint32_t id[V4L2_EVENT_FRAME_SYNC+1];
    uint32_t sequence;
    TAILQ_HEAD(, _uvc_event_entry) head;
    pthread_mutex_t access;
} uvc_event_t;

typedef struct _uvc_buffer_entry
{
    TAILQ_ENTRY(_uvc_buffer_entry) link;
    struct v4l2_buffer buffer;
} uvc_buffer_entry_t;

typedef struct _uvc_buffer
{
    uvc_ocb_t* ocb;
    uint32_t sequence;
    pthread_mutex_t access;
    int mutex_inited;
    TAILQ_HEAD(, _uvc_buffer_entry) head;
} uvc_buffer_t;

typedef struct _uvc_device
{
    /* Resource manager data, hdr must be first! */
    iofunc_attr_t          hdr[UVC_MAX_VS_COUNT];
    int                    resmgr_id[UVC_MAX_VS_COUNT];
    resmgr_io_funcs_t      io_funcs[UVC_MAX_VS_COUNT];
    resmgr_connect_funcs_t connect_funcs[UVC_MAX_VS_COUNT];
    iofunc_funcs_t         ocb_funcs[UVC_MAX_VS_COUNT];
    iofunc_mount_t         io_mount[UVC_MAX_VS_COUNT];
    resmgr_attr_t          rattr[UVC_MAX_VS_COUNT];

    /* USB data */
    uint16_t vendor_id;
    uint16_t device_id;
    char     vendor_id_str[256];
    char     device_id_str[256];
    char     serial_str[256];
    int      port_speed;
    int      bcddevice;
    struct usbd_device* uvc_vc_device;
    int total_vs_devices;
    struct usbd_device* uvc_vs_device[UVC_MAX_VS_COUNT];
    struct usbd_pipe* vc_control_pipe;
    struct usbd_pipe* vc_interrupt_pipe;
    struct usbd_pipe* vs_isochronous_pipe[UVC_MAX_VS_COUNT];
    struct usbd_pipe* vs_bulk_pipe[UVC_MAX_VS_COUNT];
    int vs_usb_iface[UVC_MAX_VS_COUNT];
    int vs_usb_config[UVC_MAX_VS_COUNT];
    /* USB: interrupt data */
    struct usbd_urb* interrupt_urb;
    uint8_t* interrupt_buffer;
    /* USB: isochronous data */
    usbd_isoch_frame_request_t* iso_list[UVC_MAX_VS_COUNT][UVC_MAX_ISO_BUFFERS];
    struct usbd_urb* iso_urb[UVC_MAX_VS_COUNT][UVC_MAX_ISO_BUFFERS];
    uint8_t* iso_buffer[UVC_MAX_VS_COUNT][UVC_MAX_ISO_BUFFERS];
    int iso_payload_size[UVC_MAX_VS_COUNT];

    /* Upper level device map */
    struct _uvc_device_mapping* map;

    /* Sysfs emulation data */
    struct _uvc_sysfs_device* sysfs[UVC_MAX_VS_COUNT];

    /* Media device data */
    struct _uvc_media_device* media[UVC_MAX_VS_COUNT];
    int                       media_entities;
    struct media_entity_desc  entity[UVC_MAX_ENTITIES];
    int                       media_pads;
    struct media_pad_desc     entity_pads[UVC_MAX_ENTITIES*UVC_MAX_ENTITY_PADS];
    int                       media_links;
    struct media_link_desc    entity_links[UVC_MAX_ENTITIES*UVC_MAX_ENTITY_PADS];

    /* VideControl data */
    vc_header_t               vc_header;
    vc_input_terminal_t       vc_iterminal;
    int                       total_output_terminals;
    vc_output_terminal_t      vc_oterminal[UVC_MAX_VS_COUNT];
    vc_selector_unit_t        vc_selector_unit;
    vc_processing_unit_t      vc_processing_unit;
    int                       total_vc_extensions;
    vc_extension_unit_t       vc_extension[UVC_MAX_EXTENSION_UNITS];
    vc_encoding_unit_t        vc_encoding_unit;

    /* VideoStreaming data */
    int                       vs_current_format;
    vs_input_header_t         vs_input_header[UVC_MAX_VS_COUNT];
    vs_output_header_t        vs_output_header[UVC_MAX_VS_COUNT];
    int                       vs_format[UVC_MAX_VS_COUNT][UVC_TOTAL_FORMATS];
    int                       vs_formats[UVC_MAX_VS_COUNT];

    /* One video stream interface could incorporate a few formats */
    vs_format_uncompressed_t  vs_format_uncompressed[UVC_MAX_VS_COUNT];
    vs_still_image_frame_t    vs_still_image_frame_uncompressed[UVC_MAX_VS_COUNT];
    int                       total_uncompressed_frames;
    vs_frame_uncompressed_t   vs_frame_uncompressed[UVC_MAX_VS_COUNT][UVC_MAX_FRAMES];
    vs_color_format_t         vs_color_format_uncompressed[UVC_MAX_VS_COUNT];

    vs_format_mjpeg_t         vs_format_mjpeg[UVC_MAX_VS_COUNT];
    vs_still_image_frame_t    vs_still_image_frame_mjpeg[UVC_MAX_VS_COUNT];
    int                       total_mjpeg_frames;
    vs_frame_mjpeg_t          vs_frame_mjpeg[UVC_MAX_VS_COUNT][UVC_MAX_FRAMES];
    vs_color_format_t         vs_color_format_mjpeg[UVC_MAX_VS_COUNT];

    vs_format_frame_t         vs_format_h264f[UVC_MAX_VS_COUNT];
    vs_still_image_frame_t    vs_still_image_frame_h264f[UVC_MAX_VS_COUNT];
    int                       total_h264f_frames;
    vs_frame_frame_t          vs_frame_h264f[UVC_MAX_VS_COUNT][UVC_MAX_FRAMES];
    vs_color_format_t         vs_color_format_h264f[UVC_MAX_VS_COUNT];

    /* V4L2 data */
    int current_transfer[UVC_MAX_VS_COUNT];
    int current_width[UVC_MAX_VS_COUNT];
    int current_height[UVC_MAX_VS_COUNT];
    int current_pixelformat[UVC_MAX_VS_COUNT];
    int current_stride[UVC_MAX_VS_COUNT];
    unsigned int current_frameinterval[UVC_MAX_VS_COUNT];
    int current_priority[UVC_MAX_VS_COUNT];
    uvc_ocb_t* current_priority_ocb[UVC_MAX_VS_COUNT];
    int current_stage[UVC_MAX_VS_COUNT];
    uvc_event_t event[UVC_MAX_OPEN_FDS];
    int current_buffer_count[UVC_MAX_VS_COUNT];
    int buffer_mode_mmap[UVC_MAX_VS_COUNT];
    uvc_ocb_t* current_reqbufs_ocb[UVC_MAX_VS_COUNT];
    int current_buffer_fds[UVC_MAX_VS_COUNT];
    uvc_buffer_t input_buffer[UVC_MAX_VS_COUNT];
    uvc_buffer_t output_buffer[UVC_MAX_VS_COUNT];
    void* buffer_ptr[UVC_MAX_VS_COUNT];
    int event_button;
} uvc_device_t;

/* Private V4L2 controls */
#define V4L2_CID_USER_PRIVATE_BASE              (V4L2_CTRL_CLASS_USER | 0x00001000)
#define V4L2_CID_CAMERA_PRIVATE_BASE            (V4L2_CTRL_CLASS_CAMERA | 0x00001000)

#define UVC_CID_EVENT_BUTTON                    (V4L2_CID_CAMERA_PRIVATE_BASE+0)

#endif /* __UVC_H__ */
