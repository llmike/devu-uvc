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

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/slog.h>
#include <sys/usbdi.h>
#include <sys/slogcodes.h>

#include "uuid.h"

#include "uvc.h"
#include "usbvc.h"
#include "uvc_control.h"

extern int uvc_verbose;
extern int uvc_emulation;

void uvc_parse_streaming_descriptor(uvc_device_t* uvcd, uint8_t* data)
{
    int size=0;
    int dtype=0;
    int dsubtype=0;
    int it;

    size=data[0];

    /* Do sanity check of buffer size, because cheap devices can provide trash in vendor */
    /* specific data.                                                                    */
    if (size>1)
    {
        dtype=data[1];
        if (size>2)
        {
            dsubtype=data[2];
        }
        else
        {
            return;
        }
    }
    else
    {
        return;
    }

    /* Check if it belongs to interface descriptor */
    if (dtype!=VCS_INTERFACE)
    {
        return;
    }

    if (uvc_verbose>1)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "Parsing descriptor ID %08X", dsubtype);
    }

    /* Parse subtypes */
    switch (dsubtype)
    {
        case VVS_INPUT_HEADER:
             {
                 uvcd->vs_input_header[uvcd->total_vs_devices].bNumFormats=0;
                 uvcd->vs_input_header[uvcd->total_vs_devices].wTotalLength=0;
                 uvcd->vs_input_header[uvcd->total_vs_devices].bEndpointAddress=0;
                 uvcd->vs_input_header[uvcd->total_vs_devices].bmInfo=0;
                 uvcd->vs_input_header[uvcd->total_vs_devices].bTerminalLink=0;
                 uvcd->vs_input_header[uvcd->total_vs_devices].bStillCaptureMethod=0;
                 uvcd->vs_input_header[uvcd->total_vs_devices].bTriggerSupport=0;
                 uvcd->vs_input_header[uvcd->total_vs_devices].bTriggerUsage=0;
                 uvcd->vs_input_header[uvcd->total_vs_devices].bControlSize=0;
                 memset(uvcd->vs_input_header[uvcd->total_vs_devices].bmaControls, 0x00, UVC_MAX_BMCONTROLS_COUNT);

                 if (size>3)
                 {
                     uvcd->vs_input_header[uvcd->total_vs_devices].bNumFormats=data[3];
                 }
                 if (size>5)
                 {
                     uvcd->vs_input_header[uvcd->total_vs_devices].wTotalLength=data[4] | ((unsigned int)data[5]<<8);
                 }
                 if (size>6)
                 {
                     uvcd->vs_input_header[uvcd->total_vs_devices].bEndpointAddress=data[6];
                 }
                 if (size>7)
                 {
                     uvcd->vs_input_header[uvcd->total_vs_devices].bmInfo=data[7];
                 }
                 if (size>8)
                 {
                     uvcd->vs_input_header[uvcd->total_vs_devices].bTerminalLink=data[8];
                 }
                 if (size>9)
                 {
                     uvcd->vs_input_header[uvcd->total_vs_devices].bStillCaptureMethod=data[9];
                 }
                 if (size>10)
                 {
                     uvcd->vs_input_header[uvcd->total_vs_devices].bTriggerSupport=data[10];
                 }
                 if (size>11)
                 {
                     uvcd->vs_input_header[uvcd->total_vs_devices].bTriggerUsage=data[11];
                 }
                 if (size>12)
                 {
                     uvcd->vs_input_header[uvcd->total_vs_devices].bControlSize=data[12];
                     if (uvcd->vs_input_header[uvcd->total_vs_devices].bControlSize>UVC_MAX_BMCONTROLS_COUNT)
                     {
                         uvcd->vs_input_header[uvcd->total_vs_devices].bControlSize=UVC_MAX_BMCONTROLS_COUNT;
                         slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] VS/INPUT_HEADER: too many controls");
                     }
                 }

                 for (it=0; it<uvcd->vs_input_header[uvcd->total_vs_devices].bControlSize; it++)
                 {
                     if (size>13+it)
                     {
                         uvcd->vs_input_header[uvcd->total_vs_devices].bmaControls[it]=data[13+it];
                     }
                 }
                 if (uvc_verbose>1)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VS_INPUT_HEADER:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bNumFormats: %02X", uvcd->vs_input_header[uvcd->total_vs_devices].bNumFormats);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    wTotalLength: %04X", uvcd->vs_input_header[uvcd->total_vs_devices].wTotalLength);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bEndpointAddress: %02X", uvcd->vs_input_header[uvcd->total_vs_devices].bEndpointAddress);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bmInfo: %02X", uvcd->vs_input_header[uvcd->total_vs_devices].bmInfo);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bTerminalLink: %02X", uvcd->vs_input_header[uvcd->total_vs_devices].bTerminalLink);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bStillCaptureMethod: %02X", uvcd->vs_input_header[uvcd->total_vs_devices].bStillCaptureMethod);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bTriggerSupport: %02X", uvcd->vs_input_header[uvcd->total_vs_devices].bTriggerSupport);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bTriggerUsage: %02X", uvcd->vs_input_header[uvcd->total_vs_devices].bTriggerUsage);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bControlSize: %02X", uvcd->vs_input_header[uvcd->total_vs_devices].bControlSize);
                     for (it=0; it<uvcd->vs_input_header[uvcd->total_vs_devices].bControlSize; it++)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bmaControls[%d]: %02X", it, uvcd->vs_input_header[uvcd->total_vs_devices].bmaControls[it]);
                     }
                 }
             }
             break;
        case VVS_OUTPUT_HEADER:
             {
                 uvcd->vs_output_header[uvcd->total_vs_devices].bNumFormats=0;
                 uvcd->vs_output_header[uvcd->total_vs_devices].wTotalLength=0;
                 uvcd->vs_output_header[uvcd->total_vs_devices].bEndpointAddress=0;
                 uvcd->vs_output_header[uvcd->total_vs_devices].bTerminalLink=0;
                 uvcd->vs_output_header[uvcd->total_vs_devices].bControlSize=0;
                 memset(uvcd->vs_output_header[uvcd->total_vs_devices].bmaControls, 0x00, UVC_MAX_BMCONTROLS_COUNT);

                 if (size>3)
                 {
                     uvcd->vs_output_header[uvcd->total_vs_devices].bNumFormats=data[3];
                 }
                 if (size>5)
                 {
                     uvcd->vs_output_header[uvcd->total_vs_devices].wTotalLength=data[4] | ((unsigned int)data[5]<<8);
                 }
                 if (size>6)
                 {
                     uvcd->vs_output_header[uvcd->total_vs_devices].bEndpointAddress=data[6];
                 }
                 if (size>7)
                 {
                     uvcd->vs_output_header[uvcd->total_vs_devices].bTerminalLink=data[7];
                 }
                 if (size>8)
                 {
                     uvcd->vs_output_header[uvcd->total_vs_devices].bControlSize=data[8];
                     if (uvcd->vs_output_header[uvcd->total_vs_devices].bControlSize>UVC_MAX_BMCONTROLS_COUNT)
                     {
                         uvcd->vs_output_header[uvcd->total_vs_devices].bControlSize=UVC_MAX_BMCONTROLS_COUNT;
                         slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] VS/OUTPUT_HEADER: too many controls");
                     }
                 }

                 for (it=0; it<uvcd->vs_output_header[uvcd->total_vs_devices].bControlSize; it++)
                 {
                     if (size>9+it)
                     {
                         uvcd->vs_output_header[uvcd->total_vs_devices].bmaControls[it]=data[9+it];
                     }
                 }
                 if (uvc_verbose>1)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VS_OUTPUT_HEADER:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bNumFormats: %02X", uvcd->vs_output_header[uvcd->total_vs_devices].bNumFormats);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    wTotalLength: %04X", uvcd->vs_output_header[uvcd->total_vs_devices].wTotalLength);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bEndpointAddress: %02X", uvcd->vs_output_header[uvcd->total_vs_devices].bEndpointAddress);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bTerminalLink: %02X", uvcd->vs_output_header[uvcd->total_vs_devices].bTerminalLink);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bControlSize: %02X", uvcd->vs_output_header[uvcd->total_vs_devices].bControlSize);
                     for (it=0; it<uvcd->vs_output_header[uvcd->total_vs_devices].bControlSize; it++)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bmaControls[%d]: %02X", it, uvcd->vs_output_header[uvcd->total_vs_devices].bmaControls[it]);
                     }
                 }
             }
             break;
        case VVS_STILL_IMAGE_FRAME:
             {
                 vs_still_image_frame_t* still_image;

                 switch(uvcd->vs_current_format)
                 {
                     case UVC_FORMAT_YUY2:
                          still_image=&uvcd->vs_still_image_frame_uncompressed[uvcd->total_vs_devices];
                          break;
                     case UVC_FORMAT_MJPG:
                          still_image=&uvcd->vs_still_image_frame_mjpeg[uvcd->total_vs_devices];
                          break;
                     case UVC_FORMAT_H264F:
                          still_image=&uvcd->vs_still_image_frame_h264f[uvcd->total_vs_devices];
                          break;
                     default:
                          slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] VS/STILL_IMAGE_FRAME without FORMAT_FRAME");
                          return;
                 }

                 still_image->bEndpointAddress=0;
                 still_image->bNumImageSizePatterns=0;
                 memset(still_image->wWidth, 0x00, 2*UVC_MAX_SIZE_PATTERNS);
                 memset(still_image->wHeight, 0x00, 2*UVC_MAX_SIZE_PATTERNS);
                 still_image->bNumCompressionPattern=0;
                 memset(still_image->bCompression, 0x00, UVC_MAX_COMPRESSION_PATTERNS);

                 if (size>3)
                 {
                     still_image->bEndpointAddress=data[3];
                 }
                 if (size>4)
                 {
                     still_image->bNumImageSizePatterns=data[4];
                     if (still_image->bNumImageSizePatterns>UVC_MAX_SIZE_PATTERNS)
                     {
                         still_image->bNumImageSizePatterns=UVC_MAX_SIZE_PATTERNS;
                         slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] VS/STILL_IMAGE_FRAME: too many patterns");
                     }
                 }

                 for (it=0; it<still_image->bNumImageSizePatterns*4; it+=4)
                 {
                     if (size>6+it)
                     {
                         still_image->wWidth[it/4]=data[5+it] | ((unsigned int)data[6+it]<<8);
                     }
                     if (size>8+it)
                     {
                         still_image->wHeight[it/4]=data[7+it] | ((unsigned int)data[8+it]<<8);
                     }
                 }

                 if (size>5+still_image->bNumImageSizePatterns*4)
                 {
                     still_image->bNumCompressionPattern=data[5+still_image->bNumImageSizePatterns*4];
                 }

                 for (it=0; it<still_image->bNumCompressionPattern; it++)
                 {
                     if (size>6+it+still_image->bNumImageSizePatterns*4)
                     {
                         still_image->bCompression[it]=
                             data[6+it+still_image->bNumImageSizePatterns*4];
                     }
                 }

                 if (uvc_verbose>1)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VS_STILL_IMAGE_FRAME:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bEndpointAddress: %02X", still_image->bEndpointAddress);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bNumImageSizePatterns: %02X", still_image->bNumImageSizePatterns);
                     for(it=0; it<still_image->bNumImageSizePatterns; it++)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    Size[%d]: %04Xx%04X (%dx%d)", it,
                             still_image->wWidth[it],
                             still_image->wHeight[it],
                             still_image->wWidth[it],
                             still_image->wHeight[it]);
                     }
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bNumCompressionPattern: %02X", still_image->bNumCompressionPattern);
                     for(it=0; it<still_image->bNumCompressionPattern; it++)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bCompression(%d): %02X", it, still_image->bCompression[it]);
                     }
                 }
             }
             break;
        case VVS_FORMAT_UNCOMPRESSED:
             {
                 /* Initialize default color format, which could be overrided */
                 /* later by specific color format descriptor.                */
                 uvcd->vs_color_format_uncompressed[uvcd->total_vs_devices].bColorPrimaries=VCFCP_BT_709;
                 uvcd->vs_color_format_uncompressed[uvcd->total_vs_devices].bTransferCharacteristics=VCFTC_BT_709;
                 uvcd->vs_color_format_uncompressed[uvcd->total_vs_devices].bMatrixCoefficients=VCFMC_SMPTE_170M;

                 uvcd->vs_current_format=UVC_FORMAT_YUY2;
                 uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bFormatIndex=0;
                 uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bNumFrameDescriptors=0;
                 memset(uvcd->vs_format_uncompressed[uvcd->total_vs_devices].guidFormat, 0x00, 16);
                 uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bBitsPerPixel=0;
                 uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bDefaultFrameIndex=0;
                 uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bAspectRatioX=0;
                 uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bAspectRatioY=0;
                 uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bmInterlaceFlags=0;
                 uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bCopyProtect=0;

                 if (size>3)
                 {
                     uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bFormatIndex=data[3];
                 }
                 if (size>4)
                 {
                     uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bNumFrameDescriptors=data[4];
                 }
                 if (size>5+15)
                 {
                     for (it=0; it<16; it++)
                     {
                         uvcd->vs_format_uncompressed[uvcd->total_vs_devices].guidFormat[it]=data[5+it];
                     }
                 }
                 if (size>5+16)
                 {
                     uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bBitsPerPixel=data[5+16];
                 }
                 if (size>6+16)
                 {
                     uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bDefaultFrameIndex=data[6+16];
                 }
                 if (size>7+16)
                 {
                     uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bAspectRatioX=data[7+16];
                 }
                 if (size>8+16)
                 {
                     uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bAspectRatioY=data[8+16];
                 }
                 if (size>9+16)
                 {
                     uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bmInterlaceFlags=data[9+16];
                 }
                 if (size>10+16)
                 {
                     uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bCopyProtect=data[10+16];
                 }

                 if (uvc_verbose>1)
                 {
                     char temp_str[128];

                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VS_FORMAT_UNCOMPRESSED:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bFormatIndex: %02X", uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bFormatIndex);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bNumFrameDescriptors: %02X", uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bNumFrameDescriptors);
                     uuid_unparse_upper(uvcd->vs_format_uncompressed[uvcd->total_vs_devices].guidFormat, temp_str);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    guidFormat: %s", temp_str);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bBitsPerPixel: %02X (%d)", uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bBitsPerPixel, uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bBitsPerPixel);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bDefaultFrameIndex: %02X", uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bDefaultFrameIndex);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bAspectRatioX: %02X", uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bAspectRatioX);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bAspectRatioY: %02X", uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bAspectRatioY);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bmInterlaceFlags: %02X", uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bmInterlaceFlags);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bCopyProtect: %02X", uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bCopyProtect);
                 }

                 /* Store format identifiers instead of GUIDs for easy handling */
                 {
                     uuid_t yuy2=VUP_GUID_FORMAT_YUY2;
                     uuid_t nv12=VUP_GUID_FORMAT_NV12;

                     if (uuid_compare(uvcd->vs_format_uncompressed[uvcd->total_vs_devices].guidFormat, yuy2)==0)
                     {
                         uvcd->vs_format[uvcd->total_vs_devices][uvcd->vs_formats[uvcd->total_vs_devices]]=UVC_FORMAT_YUY2;
                         if (uvc_emulation)
                         {
                             uvcd->vs_formats[uvcd->total_vs_devices]++;
                             uvcd->vs_format[uvcd->total_vs_devices][uvcd->vs_formats[uvcd->total_vs_devices]]=UVC_FORMAT_UYVY;
                             uvcd->vs_formats[uvcd->total_vs_devices]++;
                             uvcd->vs_format[uvcd->total_vs_devices][uvcd->vs_formats[uvcd->total_vs_devices]]=UVC_FORMAT_YVYU;
                             uvcd->vs_formats[uvcd->total_vs_devices]++;
                             uvcd->vs_format[uvcd->total_vs_devices][uvcd->vs_formats[uvcd->total_vs_devices]]=UVC_FORMAT_VYUY;
                         }
                     }
                     if (uuid_compare(uvcd->vs_format_uncompressed[uvcd->total_vs_devices].guidFormat, nv12)==0)
                     {
                         uvcd->vs_format[uvcd->total_vs_devices][uvcd->vs_formats[uvcd->total_vs_devices]]=UVC_FORMAT_NV12;
                     }
                     uvcd->vs_formats[uvcd->total_vs_devices]++;
                 }
             }
             break;
        case VVS_FRAME_UNCOMPRESSED:
             {
                 uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].bFrameIndex=0;
                 uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].bmCapabilities=0;
                 uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].wWidth=0;
                 uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].wHeight=0;
                 uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwMinBitRate=0;
                 uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwMaxBitRate=0;
                 uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwMaxVideoFrameBufferSize=0;
                 uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwDefaultFrameInterval=0;
                 uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].bFrameIntervalType=0;
                 uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwMinFrameInterval=0;
                 uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwMaxFrameInterval=0;
                 uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwFrameIntervalStep=0;
                 memset(uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwFrameInterval, 0x00,
                     sizeof(uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwFrameInterval));

                 if (size>3)
                 {
                     uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].bFrameIndex=data[3];
                 }
                 if (size>4)
                 {
                     uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].bmCapabilities=data[4];
                 }
                 if (size>6)
                 {
                     uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].wWidth=data[5] | ((unsigned int)data[6]<<8);
                 }
                 if (size>8)
                 {
                     uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].wHeight=data[7] | ((unsigned int)data[8]<<8);
                 }
                 if (size>12)
                 {
                     uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwMinBitRate=
                         data[9] | ((unsigned int)data[10]<<8) |
                        ((unsigned int)data[11]<<16) | ((unsigned int)data[12]<<24);
                 }
                 if (size>16)
                 {
                     uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwMaxBitRate=
                         data[13] | ((unsigned int)data[14]<<8) |
                        ((unsigned int)data[15]<<16) | ((unsigned int)data[16]<<24);
                 }
                 if (size>20)
                 {
                     uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwMaxVideoFrameBufferSize=
                         data[17] | ((unsigned int)data[18]<<8) |
                        ((unsigned int)data[19]<<16) | ((unsigned int)data[20]<<24);
                 }
                 if (size>24)
                 {
                     uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwDefaultFrameInterval=
                         data[21] | ((unsigned int)data[22]<<8) |
                        ((unsigned int)data[23]<<16) | ((unsigned int)data[24]<<24);
                 }
                 if (size>25)
                 {
                     uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].bFrameIntervalType=data[25];
                     if (uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].bFrameIntervalType>UVC_MAX_FRAME_INTERVALS)
                     {
                         uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].bFrameIntervalType=UVC_MAX_FRAME_INTERVALS;
                         slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] VS/FRAME_UNCOMPRESSED: too many frames");
                     }
                 }

                 if (uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].bFrameIntervalType)
                 {
                     /* descrete intervals */
                     for (it=0; it<uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].bFrameIntervalType*4; it+=4)
                     {
                         if (size>29+it)
                         {
                             uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwFrameInterval[it/4]=
                                 data[26+it] | ((unsigned int)data[27+it]<<8) |
                                 ((unsigned int)data[28+it]<<16) | ((unsigned int)data[29+it]<<24);
                         }
                     }
                 }
                 else
                 {
                     /* continuous intervals */
                     if (size>29)
                     {
                         uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwMinFrameInterval=
                             data[26] | ((unsigned int)data[27]<<8) |
                            ((unsigned int)data[28]<<16) | ((unsigned int)data[29]<<24);
                     }
                     if (size>33)
                     {
                         uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwMaxFrameInterval=
                             data[30] | ((unsigned int)data[31]<<8) |
                            ((unsigned int)data[32]<<16) | ((unsigned int)data[33]<<24);
                     }
                     if (size>37)
                     {
                         uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwFrameIntervalStep=
                             data[34] | ((unsigned int)data[35]<<8) |
                            ((unsigned int)data[36]<<16) | ((unsigned int)data[37]<<24);
                     }
                 }

                 if (uvc_verbose>1)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VS_FRAME_UNCOMPRESSED:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bFrameIndex: %02X", uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].bFrameIndex);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bmCapabilities: %02X", uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].bmCapabilities);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    wWidth: %04X (%d)",
                         uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].wWidth,
                         uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].wWidth);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    wHeight: %04X (%d)",
                         uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].wHeight,
                         uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].wHeight);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    dwMinBitRate: %08X (%d)",
                         uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwMinBitRate,
                         uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwMinBitRate);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    dwMaxBitRate: %08X (%d)",
                         uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwMaxBitRate,
                         uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwMaxBitRate);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    dwMaxVideoFrameBufferSize: %08X (%d)",
                         uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwMaxVideoFrameBufferSize,
                         uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwMaxVideoFrameBufferSize);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    dwDefaultFrameInterval: %08X (%d)",
                         uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwDefaultFrameInterval,
                         uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwDefaultFrameInterval);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bFrameIntervalType: %02X", uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].bFrameIntervalType);
                     if (uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].bFrameIntervalType)
                     {
                         /* descrete intervals */
                         for(it=0; it<uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].bFrameIntervalType; it++)
                         {
                             slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    dwFrameInterval[%d]: %08X (%d, %f fps)", it, 
                                 uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwFrameInterval[it],
                                 uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwFrameInterval[it],
                                 10000000.0f/uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwFrameInterval[it]);
                         }
                     }
                     else
                     {
                         /* continuous intervals */
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    dwMinFrameInterval: %08X (%d, %f fps)",
                             uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwMinFrameInterval,
                             uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwMinFrameInterval,
                             10000000.0f/uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwMinFrameInterval);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    dwMaxFrameInterval: %08X (%d, %f fps)",
                             uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwMaxFrameInterval,
                             uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwMaxFrameInterval,
                             10000000.0f/uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwMaxFrameInterval);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    dwFrameIntervalStep: %08X (%d, %f fps)",
                             uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwFrameIntervalStep,
                             uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwFrameIntervalStep,
                             10000000.0f/uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][uvcd->total_uncompressed_frames].dwFrameIntervalStep);
                     }
                 }

                 uvcd->total_uncompressed_frames++;
             }
             break;
        case VVS_FORMAT_MJPEG:
             {
                 /* Initialize default color format, which could be overrided */
                 /* later by specific color format descriptor.                */
                 uvcd->vs_color_format_mjpeg[uvcd->total_vs_devices].bColorPrimaries=VCFCP_BT_709;
                 uvcd->vs_color_format_mjpeg[uvcd->total_vs_devices].bTransferCharacteristics=VCFTC_BT_709;
                 uvcd->vs_color_format_mjpeg[uvcd->total_vs_devices].bMatrixCoefficients=VCFMC_SMPTE_170M;

                 uvcd->vs_current_format=UVC_FORMAT_MJPG;
                 uvcd->vs_format_mjpeg[uvcd->total_vs_devices].bFormatIndex=0;
                 uvcd->vs_format_mjpeg[uvcd->total_vs_devices].bNumFrameDescriptors=0;
                 uvcd->vs_format_mjpeg[uvcd->total_vs_devices].bmFlags=0;
                 uvcd->vs_format_mjpeg[uvcd->total_vs_devices].bDefaultFrameIndex=0;
                 uvcd->vs_format_mjpeg[uvcd->total_vs_devices].bAspectRatioX=0;
                 uvcd->vs_format_mjpeg[uvcd->total_vs_devices].bAspectRatioY=0;
                 uvcd->vs_format_mjpeg[uvcd->total_vs_devices].bmInterlaceFlags=0;
                 uvcd->vs_format_mjpeg[uvcd->total_vs_devices].bCopyProtect=0;

                 if (size>3)
                 {
                     uvcd->vs_format_mjpeg[uvcd->total_vs_devices].bFormatIndex=data[3];
                 }
                 if (size>4)
                 {
                     uvcd->vs_format_mjpeg[uvcd->total_vs_devices].bNumFrameDescriptors=data[4];
                 }
                 if (size>5)
                 {
                     uvcd->vs_format_mjpeg[uvcd->total_vs_devices].bmFlags=data[5];
                 }
                 if (size>6)
                 {
                     uvcd->vs_format_mjpeg[uvcd->total_vs_devices].bDefaultFrameIndex=data[6];
                 }
                 if (size>7)
                 {
                     uvcd->vs_format_mjpeg[uvcd->total_vs_devices].bAspectRatioX=data[7];
                 }
                 if (size>8)
                 {
                     uvcd->vs_format_mjpeg[uvcd->total_vs_devices].bAspectRatioY=data[8];
                 }
                 if (size>9)
                 {
                     uvcd->vs_format_mjpeg[uvcd->total_vs_devices].bmInterlaceFlags=data[9];
                 }
                 if (size>10)
                 {
                     uvcd->vs_format_mjpeg[uvcd->total_vs_devices].bCopyProtect=data[10];
                 }

                 if (uvc_verbose>1)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VS_FORMAT_MJPEG:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bFormatIndex: %02X", uvcd->vs_format_mjpeg[uvcd->total_vs_devices].bFormatIndex);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bNumFrameDescriptors: %02X", uvcd->vs_format_mjpeg[uvcd->total_vs_devices].bNumFrameDescriptors);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bmFlags: %02X", uvcd->vs_format_mjpeg[uvcd->total_vs_devices].bmFlags);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bDefaultFrameIndex: %02X", uvcd->vs_format_mjpeg[uvcd->total_vs_devices].bDefaultFrameIndex);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bAspectRatioX: %02X", uvcd->vs_format_mjpeg[uvcd->total_vs_devices].bAspectRatioX);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bAspectRatioY: %02X", uvcd->vs_format_mjpeg[uvcd->total_vs_devices].bAspectRatioY);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bmInterlaceFlags: %02X", uvcd->vs_format_mjpeg[uvcd->total_vs_devices].bmInterlaceFlags);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bCopyProtect: %02X", uvcd->vs_format_mjpeg[uvcd->total_vs_devices].bCopyProtect);
                 }

                 uvcd->vs_format[uvcd->total_vs_devices][uvcd->vs_formats[uvcd->total_vs_devices]]=UVC_FORMAT_MJPG;
                 uvcd->vs_formats[uvcd->total_vs_devices]++;
             }
             break;
        case VVS_FRAME_MJPEG:
             {
                 uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].bFrameIndex=0;
                 uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].bmCapabilities=0;
                 uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].wWidth=0;
                 uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].wHeight=0;
                 uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwMinBitRate=0;
                 uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwMaxBitRate=0;
                 uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwMaxVideoFrameBufferSize=0;
                 uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwDefaultFrameInterval=0;
                 uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].bFrameIntervalType=0;
                 uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwMinFrameInterval=0;
                 uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwMaxFrameInterval=0;
                 uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwFrameIntervalStep=0;
                 memset(uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwFrameInterval, 0x00,
                     sizeof(uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwFrameInterval));

                 if (size>3)
                 {
                     uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].bFrameIndex=data[3];
                 }
                 if (size>4)
                 {
                     uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].bmCapabilities=data[4];
                 }
                 if (size>6)
                 {
                     uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].wWidth=data[5] | ((unsigned int)data[6]<<8);
                 }
                 if (size>8)
                 {
                     uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].wHeight=data[7] | ((unsigned int)data[8]<<8);
                 }
                 if (size>12)
                 {
                     uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwMinBitRate=
                         data[9] | ((unsigned int)data[10]<<8) |
                        ((unsigned int)data[11]<<16) | ((unsigned int)data[12]<<24);
                 }
                 if (size>16)
                 {
                     uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwMaxBitRate=
                         data[13] | ((unsigned int)data[14]<<8) |
                        ((unsigned int)data[15]<<16) | ((unsigned int)data[16]<<24);
                 }
                 if (size>20)
                 {
                     uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwMaxVideoFrameBufferSize=
                         data[17] | ((unsigned int)data[18]<<8) |
                        ((unsigned int)data[19]<<16) | ((unsigned int)data[20]<<24);
                 }
                 if (size>24)
                 {
                     uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwDefaultFrameInterval=
                         data[21] | ((unsigned int)data[22]<<8) |
                        ((unsigned int)data[23]<<16) | ((unsigned int)data[24]<<24);
                 }
                 if (size>25)
                 {
                     uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].bFrameIntervalType=data[25];
                     if (uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].bFrameIntervalType>UVC_MAX_FRAME_INTERVALS)
                     {
                         uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].bFrameIntervalType=UVC_MAX_FRAME_INTERVALS;
                         slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] VS/FRAME_MJPEG: too many frames");
                     }
                 }

                 if (uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].bFrameIntervalType)
                 {
                     /* descrete intervals */
                     for (it=0; it<uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].bFrameIntervalType*4; it+=4)
                     {
                         if (size>29+it)
                         {
                             uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwFrameInterval[it/4]=
                                 data[26+it] | ((unsigned int)data[27+it]<<8) |
                                 ((unsigned int)data[28+it]<<16) | ((unsigned int)data[29+it]<<24);
                         }
                     }
                 }
                 else
                 {
                     /* continuous intervals */
                     if (size>29)
                     {
                         uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwMinFrameInterval=
                             data[26] | ((unsigned int)data[27]<<8) |
                            ((unsigned int)data[28]<<16) | ((unsigned int)data[29]<<24);
                     }
                     if (size>33)
                     {
                         uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwMaxFrameInterval=
                             data[30] | ((unsigned int)data[31]<<8) |
                            ((unsigned int)data[32]<<16) | ((unsigned int)data[33]<<24);
                     }
                     if (size>37)
                     {
                         uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwFrameIntervalStep=
                             data[34] | ((unsigned int)data[35]<<8) |
                            ((unsigned int)data[36]<<16) | ((unsigned int)data[37]<<24);
                     }
                 }

                 if (uvc_verbose>1)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VS_FRAME_MJPEG:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bFrameIndex: %02X", uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].bFrameIndex);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bmCapabilities: %02X", uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].bmCapabilities);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    wWidth: %04X (%d)",
                         uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].wWidth,
                         uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].wWidth);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    wHeight: %04X (%d)",
                         uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].wHeight,
                         uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].wHeight);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    dwMinBitRate: %08X (%d)",
                         uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwMinBitRate,
                         uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwMinBitRate);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    dwMaxBitRate: %08X (%d)",
                         uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwMaxBitRate,
                         uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwMaxBitRate);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    dwMaxVideoFrameBufferSize: %08X (%d)",
                         uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwMaxVideoFrameBufferSize,
                         uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwMaxVideoFrameBufferSize);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    dwDefaultFrameInterval: %08X (%d)",
                         uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwDefaultFrameInterval,
                         uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwDefaultFrameInterval);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bFrameIntervalType: %02X", uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].bFrameIntervalType);
                     if (uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].bFrameIntervalType)
                     {
                         /* descrete intervals */
                         for(it=0; it<uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].bFrameIntervalType; it++)
                         {
                             slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    dwFrameInterval[%d]: %08X (%d, %f fps)", it, 
                                 uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwFrameInterval[it],
                                 uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwFrameInterval[it],
                                 10000000.0f/uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwFrameInterval[it]);
                         }
                     }
                     else
                     {
                         /* continuous intervals */
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    dwMinFrameInterval: %08X (%d, %f fps)",
                             uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwMinFrameInterval,
                             uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwMinFrameInterval,
                             10000000.0f/uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwMinFrameInterval);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    dwMaxFrameInterval: %08X (%d, %f fps)",
                             uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwMaxFrameInterval,
                             uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwMaxFrameInterval,
                             10000000.0f/uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwMaxFrameInterval);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    dwFrameIntervalStep: %08X (%d, %f fps)",
                             uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwFrameIntervalStep,
                             uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwFrameIntervalStep,
                             10000000.0f/uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][uvcd->total_mjpeg_frames].dwFrameIntervalStep);
                     }
                 }

                 uvcd->total_mjpeg_frames++;
             }
             break;
        case VVS_FORMAT_MPEG2TS:
             {
                 if (uvc_verbose>1)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VS_FORMAT_MPEG2TS:");
                 }
                 uvcd->vs_format[uvcd->total_vs_devices][uvcd->vs_formats[uvcd->total_vs_devices]]=UVC_FORMAT_MPEG2TS;
                 uvcd->vs_formats[uvcd->total_vs_devices]++;
             }
             break;
        case VVS_FORMAT_DV:
             {
                 /* DV format doesn't have frame descriptors, so frames must */
                 /* be created manually depending on bFormatType.            */

                 if (uvc_verbose>1)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VS_FORMAT_DV:");
                 }
                 uvcd->vs_format[uvcd->total_vs_devices][uvcd->vs_formats[uvcd->total_vs_devices]]=UVC_FORMAT_DV;
                 uvcd->vs_formats[uvcd->total_vs_devices]++;
             }
             break;
        case VVS_COLORFORMAT:
             {
                 vs_color_format_t* color_format;

                 switch(uvcd->vs_current_format)
                 {
                     case UVC_FORMAT_YUY2:
                          color_format=&uvcd->vs_color_format_uncompressed[uvcd->total_vs_devices];
                          break;
                     case UVC_FORMAT_MJPG:
                          color_format=&uvcd->vs_color_format_mjpeg[uvcd->total_vs_devices];
                          break;
                     case UVC_FORMAT_H264F:
                          color_format=&uvcd->vs_color_format_h264f[uvcd->total_vs_devices];
                          break;
                     default:
                          slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] VS/COLOR_FORMAT without FORMAT_FRAME");
                          return;
                 }

                 color_format->bColorPrimaries=VCFCP_BT_709;
                 color_format->bTransferCharacteristics=VCFTC_BT_709;
                 color_format->bMatrixCoefficients=VCFMC_SMPTE_170M;

                 if (size>3)
                 {
                     color_format->bColorPrimaries=data[3];
                 }

                 if (size>4)
                 {
                     color_format->bTransferCharacteristics=data[4];
                 }

                 if (size>5)
                 {
                     color_format->bMatrixCoefficients=data[5];
                 }

                 if (uvc_verbose>1)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VS_COLORFORMAT:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bColorPrimaries: %02X", color_format->bColorPrimaries);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bTransferCharacteristics: %02X", color_format->bTransferCharacteristics);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bMatrixCoefficients: %02X", color_format->bMatrixCoefficients);
                 }
             }
             break;
        case VVS_FORMAT_FRAME_BASED:
             {
                 uuid_t h264=VUP_GUID_FORMAT_H264;

                 if (uuid_compare(&data[5], h264)==0)
                 {
                     /* Initialize default color format, which could be overrided */
                     /* later by specific color format descriptor.                */
                     uvcd->vs_color_format_h264f[uvcd->total_vs_devices].bColorPrimaries=VCFCP_BT_709;
                     uvcd->vs_color_format_h264f[uvcd->total_vs_devices].bTransferCharacteristics=VCFTC_BT_709;
                     uvcd->vs_color_format_h264f[uvcd->total_vs_devices].bMatrixCoefficients=VCFMC_SMPTE_170M;

                     uvcd->vs_current_format=UVC_FORMAT_H264F;
                     uvcd->vs_format_h264f[uvcd->total_vs_devices].bFormatIndex=0;
                     uvcd->vs_format_h264f[uvcd->total_vs_devices].bNumFrameDescriptors=0;
                     memset(uvcd->vs_format_h264f[uvcd->total_vs_devices].guidFormat, 0x00, 16);
                     uvcd->vs_format_h264f[uvcd->total_vs_devices].bBitsPerPixel=0;
                     uvcd->vs_format_h264f[uvcd->total_vs_devices].bDefaultFrameIndex=0;
                     uvcd->vs_format_h264f[uvcd->total_vs_devices].bAspectRatioX=0;
                     uvcd->vs_format_h264f[uvcd->total_vs_devices].bAspectRatioY=0;
                     uvcd->vs_format_h264f[uvcd->total_vs_devices].bmInterlaceFlags=0;
                     uvcd->vs_format_h264f[uvcd->total_vs_devices].bCopyProtect=0;
                     uvcd->vs_format_h264f[uvcd->total_vs_devices].bVariableSize=0;

                     if (size>3)
                     {
                         uvcd->vs_format_h264f[uvcd->total_vs_devices].bFormatIndex=data[3];
                     }
                     if (size>4)
                     {
                         uvcd->vs_format_h264f[uvcd->total_vs_devices].bNumFrameDescriptors=data[4];
                     }
                     if (size>5+15)
                     {
                         for (it=0; it<16; it++)
                         {
                             uvcd->vs_format_h264f[uvcd->total_vs_devices].guidFormat[it]=data[5+it];
                         }
                     }
                     if (size>5+16)
                     {
                         uvcd->vs_format_h264f[uvcd->total_vs_devices].bBitsPerPixel=data[5+16];
                     }
                     if (size>6+16)
                     {
                         uvcd->vs_format_h264f[uvcd->total_vs_devices].bDefaultFrameIndex=data[6+16];
                     }
                     if (size>7+16)
                     {
                         uvcd->vs_format_h264f[uvcd->total_vs_devices].bAspectRatioX=data[7+16];
                     }
                     if (size>8+16)
                     {
                         uvcd->vs_format_h264f[uvcd->total_vs_devices].bAspectRatioY=data[8+16];
                     }
                     if (size>9+16)
                     {
                         uvcd->vs_format_h264f[uvcd->total_vs_devices].bmInterlaceFlags=data[9+16];
                     }
                     if (size>10+16)
                     {
                         uvcd->vs_format_h264f[uvcd->total_vs_devices].bCopyProtect=data[10+16];
                     }
                     if (size>11+16)
                     {
                         uvcd->vs_format_h264f[uvcd->total_vs_devices].bVariableSize=data[11+16];
                     }

                     if (uvc_verbose>1)
                     {
                         char temp_str[128];

                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VS_FORMAT_FRAME_BASED (H.264):");
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bFormatIndex: %02X", uvcd->vs_format_h264f[uvcd->total_vs_devices].bFormatIndex);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bNumFrameDescriptors: %02X", uvcd->vs_format_h264f[uvcd->total_vs_devices].bNumFrameDescriptors);
                         uuid_unparse_upper(uvcd->vs_format_h264f[uvcd->total_vs_devices].guidFormat, temp_str);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    guidFormat: %s", temp_str);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bBitsPerPixel: %02X (%d)", uvcd->vs_format_h264f[uvcd->total_vs_devices].bBitsPerPixel, uvcd->vs_format_h264f[uvcd->total_vs_devices].bBitsPerPixel);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bDefaultFrameIndex: %02X", uvcd->vs_format_h264f[uvcd->total_vs_devices].bDefaultFrameIndex);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bAspectRatioX: %02X", uvcd->vs_format_h264f[uvcd->total_vs_devices].bAspectRatioX);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bAspectRatioY: %02X", uvcd->vs_format_h264f[uvcd->total_vs_devices].bAspectRatioY);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bmInterlaceFlags: %02X", uvcd->vs_format_h264f[uvcd->total_vs_devices].bmInterlaceFlags);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bCopyProtect: %02X", uvcd->vs_format_h264f[uvcd->total_vs_devices].bCopyProtect);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bVariableSize: %02X", uvcd->vs_format_h264f[uvcd->total_vs_devices].bVariableSize);
                     }

                     /* Store format identifiers instead of GUIDs for easy handling */
                     uvcd->vs_format[uvcd->total_vs_devices][uvcd->vs_formats[uvcd->total_vs_devices]]=UVC_FORMAT_H264F;
                     uvcd->vs_formats[uvcd->total_vs_devices]++;
                 }
                 else
                 {
                     uvcd->vs_current_format=UVC_FORMAT_NONE;

                     if (uvc_verbose>1)
                     {
                         char temp_str[128];

                         uuid_unparse_upper(&data[5], temp_str);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VS_FORMAT_FRAME_BASED:");
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    Unsupported guidFormat: %s", temp_str);
                     }
                 }
             }
             break;
        case VVS_FRAME_FRAME_BASED:
             {
                 if (uvcd->vs_current_format==UVC_FORMAT_H264F)
                 {
                     uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].bFrameIndex=0;
                     uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].bmCapabilities=0;
                     uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].wWidth=0;
                     uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].wHeight=0;
                     uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwMinBitRate=0;
                     uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwMaxBitRate=0;
                     uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwDefaultFrameInterval=0;
                     uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].bFrameIntervalType=0;
                     uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwBytesPerLine=0;
                     uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwMinFrameInterval=0;
                     uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwMaxFrameInterval=0;
                     uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwFrameIntervalStep=0;
                     memset(uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwFrameInterval, 0x00,
                         sizeof(uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwFrameInterval));

                     if (size>3)
                     {
                         uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].bFrameIndex=data[3];
                     }
                     if (size>4)
                     {
                         uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].bmCapabilities=data[4];
                     }
                     if (size>6)
                     {
                         uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].wWidth=data[5] | ((unsigned int)data[6]<<8);
                     }
                     if (size>8)
                     {
                         uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].wHeight=data[7] | ((unsigned int)data[8]<<8);
                     }
                     if (size>12)
                     {
                         uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwMinBitRate=
                             data[9] | ((unsigned int)data[10]<<8) |
                            ((unsigned int)data[11]<<16) | ((unsigned int)data[12]<<24);
                     }
                     if (size>16)
                     {
                         uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwMaxBitRate=
                             data[13] | ((unsigned int)data[14]<<8) |
                            ((unsigned int)data[15]<<16) | ((unsigned int)data[16]<<24);
                     }
                     if (size>20)
                     {
                         uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwDefaultFrameInterval=
                             data[17] | ((unsigned int)data[18]<<8) |
                            ((unsigned int)data[19]<<16) | ((unsigned int)data[20]<<24);
                     }
                     if (size>21)
                     {
                         uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].bFrameIntervalType=data[21];
                         if (uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].bFrameIntervalType>UVC_MAX_FRAME_INTERVALS)
                         {
                             uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].bFrameIntervalType=UVC_MAX_FRAME_INTERVALS;
                             slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] VS/FRAME_FRAME: too many frame intervals");
                         }
                     }
                     if (size>25)
                     {
                         uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwBytesPerLine=
                             data[22] | ((unsigned int)data[23]<<8) |
                            ((unsigned int)data[24]<<16) | ((unsigned int)data[25]<<24);
                     }

                     if (uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].bFrameIntervalType)
                     {
                         /* descrete intervals */
                         for (it=0; it<uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].bFrameIntervalType*4; it+=4)
                         {
                             if (size>29+it)
                             {
                                 uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwFrameInterval[it/4]=
                                     data[26+it] | ((unsigned int)data[27+it]<<8) |
                                     ((unsigned int)data[28+it]<<16) | ((unsigned int)data[29+it]<<24);
                             }
                         }
                     }
                     else
                     {
                         /* continuous intervals */
                         if (size>29)
                         {
                             uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwMinFrameInterval=
                                 data[26] | ((unsigned int)data[27]<<8) |
                                ((unsigned int)data[28]<<16) | ((unsigned int)data[29]<<24);
                         }
                         if (size>33)
                         {
                             uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwMaxFrameInterval=
                                 data[30] | ((unsigned int)data[31]<<8) |
                                ((unsigned int)data[32]<<16) | ((unsigned int)data[33]<<24);
                         }
                         if (size>37)
                         {
                             uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwFrameIntervalStep=
                                 data[34] | ((unsigned int)data[35]<<8) |
                                ((unsigned int)data[36]<<16) | ((unsigned int)data[37]<<24);
                         }
                     }

                     if (uvc_verbose>1)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VS_FRAME_FRAME_BASED (H.264):");
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bFrameIndex: %02X", uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].bFrameIndex);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bmCapabilities: %02X", uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].bmCapabilities);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    wWidth: %04X (%d)",
                             uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].wWidth,
                             uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].wWidth);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    wHeight: %04X (%d)",
                             uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].wHeight,
                             uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].wHeight);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    dwMinBitRate: %08X (%d)",
                             uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwMinBitRate,
                             uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwMinBitRate);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    dwMaxBitRate: %08X (%d)",
                             uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwMaxBitRate,
                             uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwMaxBitRate);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    dwDefaultFrameInterval: %08X (%d)",
                             uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwDefaultFrameInterval,
                             uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwDefaultFrameInterval);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bFrameIntervalType: %02X", uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].bFrameIntervalType);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    dwBytesPerLine: %08X (%d)",
                             uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwBytesPerLine,
                             uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwBytesPerLine);
                         if (uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].bFrameIntervalType)
                         {
                             /* descrete intervals */
                             for(it=0; it<uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].bFrameIntervalType; it++)
                             {
                                 slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    dwFrameInterval[%d]: %08X (%d, %f fps)", it,
                                     uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwFrameInterval[it],
                                     uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwFrameInterval[it],
                                     10000000.0f/uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwFrameInterval[it]);
                             }
                         }
                         else
                         {
                             /* continuous intervals */
                             slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    dwMinFrameInterval: %08X (%d, %f fps)",
                                 uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwMinFrameInterval,
                                 uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwMinFrameInterval,
                                 10000000.0f/uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwMinFrameInterval);
                             slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    dwMaxFrameInterval: %08X (%d, %f fps)",
                                 uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwMaxFrameInterval,
                                 uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwMaxFrameInterval,
                                 10000000.0f/uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwMaxFrameInterval);
                             slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    dwFrameIntervalStep: %08X (%d, %f fps)",
                                 uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwFrameIntervalStep,
                                 uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwFrameIntervalStep,
                                 10000000.0f/uvcd->vs_frame_h264f[uvcd->total_vs_devices][uvcd->total_h264f_frames].dwFrameIntervalStep);
                         }
                     }

                     uvcd->total_h264f_frames++;
                 }
             }
             break;
        case VVS_FORMAT_STREAM_BASED:
             {
                 if (uvc_verbose>1)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VS_FORMAT_STREAM_BASED:");
                 }
             }
             break;
        case VVS_FORMAT_H264:
             {
                 if (uvc_verbose>1)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VS_FORMAT_H264:");
                 }
                 uvcd->vs_format[uvcd->total_vs_devices][uvcd->vs_formats[uvcd->total_vs_devices]]=UVC_FORMAT_H264;
                 uvcd->vs_formats[uvcd->total_vs_devices]++;
             }
             break;
        case VVS_FRAME_H264:
             {
                 if (uvc_verbose>1)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VS_FRAME_H264:");
                 }
             }
             break;
        case VVS_FORMAT_H264_SIMULCAST:
             {
                 if (uvc_verbose>1)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VS_FORMAT_H264_SIMULCAST:");
                 }
             }
             break;
        case VVS_FORMAT_VP8:
             {
                 if (uvc_verbose>1)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VS_FORMAT_VP8:");
                 }

                 uvcd->vs_format[uvcd->total_vs_devices][uvcd->vs_formats[uvcd->total_vs_devices]]=UVC_FORMAT_VP8;
                 uvcd->vs_formats[uvcd->total_vs_devices]++;
             }
             break;
        case VVS_FRAME_VP8:
             {
                 if (uvc_verbose>1)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VS_FRAME_VP8:");
                 }

                 uvcd->vs_format[uvcd->total_vs_devices][uvcd->vs_formats[uvcd->total_vs_devices]]=UVC_FORMAT_VP8;
                 uvcd->vs_formats[uvcd->total_vs_devices]++;
             }
             break;
        case VVS_FORMAT_VP8_SIMULCAST:
             {
                 if (uvc_verbose>1)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VS_FORMAT_VP8_SIMULCAST:");
                 }
             }
             break;
        default:
            return;
    }
}

int uvc_probe_commit_get(uvc_device_t* dev, int subdev, int operation, int iface, int selector, int size, uint8_t* data)
{
    probe_commit_control_t* ctrl=(probe_commit_control_t*)data;
    struct usbd_urb* urb;
    uint8_t* buffer;
    int status;

    urb=usbd_alloc_urb(NULL);
    if (urb==NULL)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't allocate urb");
        return -1;
    }

    buffer=usbd_alloc(size);
    if (buffer==NULL)
    {
        usbd_free_urb(urb);
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't allocate buffer");
        return -1;
    }

    if (uvc_verbose>3)
    {
        char* selector_string=NULL;

        switch (selector)
        {
             case VVS_PROBE_CONTROL:
                  selector_string="probe";
                  break;
             case VVS_COMMIT_CONTROL:
                  selector_string="commit";
                  break;
             default:
                  selector_string="unknown";
                  break;
        }

        switch (operation)
        {
            case VGET_CUR:
                 slogf(_SLOGC_USB_GEN, _SLOG_INFO, "            USB streaming ctrl recv, GET_CUR (%s)", selector_string);
                 break;
            case VGET_LEN:
                 slogf(_SLOGC_USB_GEN, _SLOG_INFO, "            USB streaming ctrl recv, GET_LEN (%s)", selector_string);
                 break;
            case VGET_RES:
                 slogf(_SLOGC_USB_GEN, _SLOG_INFO, "            USB streaming ctrl recv, GET_RES (%s)", selector_string);
                 break;
            case VGET_MIN:
                 slogf(_SLOGC_USB_GEN, _SLOG_INFO, "            USB streaming ctrl recv, GET_MIN (%s)", selector_string);
                 break;
            case VGET_MAX:
                 slogf(_SLOGC_USB_GEN, _SLOG_INFO, "            USB streaming ctrl recv, GET_MAX (%s)", selector_string);
                 break;
            case VGET_DEF:
                 slogf(_SLOGC_USB_GEN, _SLOG_INFO, "            USB streaming ctrl recv, GET_DEF (%s)", selector_string);
                 break;
            default:
                 slogf(_SLOGC_USB_GEN, _SLOG_INFO, "            USB streaming ctrl recv, UNKNOWN (%s)", selector_string);
                 break;
        }
    }

    slogf(_SLOGC_USB_GEN, _SLOG_INFO, "%02X %02X %02X %02X %02X %02X %02X %02X", UVC_REQUEST_GET_VC, operation, (selector<<8) & 0xFF, ((selector<<8)>>8) & 0xFF, iface, 0x00, size, 0x00);
    usbd_setup_vendor(urb, URB_DIR_IN, operation, UVC_REQUEST_GET_VC, selector<<8, iface, buffer, size);
    status=usbd_io(urb, dev->vc_control_pipe, NULL, dev, USBD_TIME_INFINITY);
    if (status!=EOK)
    {
        usbd_free_urb(urb);
        usbd_free(buffer);
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't perform usb i/o operation");
        return -1;
    }

    if (uvc_verbose>3)
    {
        char tempbuf[128];
        int it, jt;

        for (jt=0; jt<(size+15)/16; jt++)
        {
            strcpy(tempbuf, "                ");
            for (it=0; it<16; it++)
            {
                if (jt*16+it>=size)
                {
                    break;
                }
                sprintf(tempbuf+strlen(tempbuf), "%02X ", buffer[jt*16+it]);
            }
            slogf(_SLOGC_USB_GEN, _SLOG_INFO, "%s", tempbuf);
        }
    }

    data[0]=buffer[0];
    data[1]=buffer[1];

    /* UVC 1.0 */
    if (size>2)
    {
        /* Copy from USB buffer, handling USB endianess */
        ctrl->bmHint=((uint16_t)buffer[1]<<8) | buffer[0];
        ctrl->bFormatIndex=buffer[2];
        ctrl->bFrameIndex=buffer[3];
        ctrl->dwFrameInterval=((uint32_t)buffer[7]<<24) | ((uint32_t)buffer[6]<<16) |
                              ((uint32_t)buffer[5]<<8) | buffer[4];
        ctrl->wKeyFrameRate=((uint16_t)buffer[9]<<8) | buffer[8];
        ctrl->wPFrameRate=((uint16_t)buffer[11]<<8) | buffer[10];
        ctrl->wCompQuality=((uint16_t)buffer[13]<<8) | buffer[12];
        ctrl->wCompWindowSize=((uint16_t)buffer[15]<<8) | buffer[14];
        ctrl->wDelay=((uint16_t)buffer[17]<<8) | buffer[16];
        ctrl->dwMaxVideoFrameSize=((uint32_t)buffer[21]<<24) | ((uint32_t)buffer[20]<<16) |
                                  ((uint32_t)buffer[19]<<8) | buffer[18];
        ctrl->dwMaxPayloadTransferSize=((uint32_t)buffer[25]<<24) | ((uint32_t)buffer[24]<<16) |
                                       ((uint32_t)buffer[23]<<8) | buffer[22];
    }

    /* UVC 1.1 */
    if (size>UVC_PROBE_COMMIT_VER10_SIZE)
    {
        ctrl->dwClockFrequency=((uint32_t)buffer[29]<<24) | ((uint32_t)buffer[28]<<16) |
                               ((uint32_t)buffer[27]<<8) | buffer[26];
        ctrl->bmFramingInfo=buffer[30];
        ctrl->bPreferedVersion=buffer[31];
        ctrl->bMinVersion=buffer[32];
        ctrl->bMaxVersion=buffer[33];
    }

    /* UVC 1.5 */
    if (size>UVC_PROBE_COMMIT_VER11_SIZE)
    {
        ctrl->bUsage=buffer[34];
        ctrl->bBitDepthLuma=buffer[35];
        ctrl->bmSettings=buffer[36];
        ctrl->bMaxNumberOfRefFramesPlus1=buffer[37];
        ctrl->bmRateControlModes =((uint16_t)buffer[39]<<8) | buffer[38];
        ctrl->bmLayoutPerStream=((uint64_t)buffer[47]<<56) | ((uint64_t)buffer[46]<<48) |
                                ((uint64_t)buffer[45]<<40) | ((uint64_t)buffer[44]<<32) |
                                ((uint64_t)buffer[43]<<24) | ((uint64_t)buffer[42]<<16) |
                                ((uint64_t)buffer[41]<<8) | buffer[40];
    }

    usbd_free_urb(urb);
    usbd_free(buffer);

    return 0;
}

int uvc_probe_commit_set(uvc_device_t* dev, int subdev, int operation, int iface, int selector, int size, uint8_t* data)
{
    probe_commit_control_t* ctrl=(probe_commit_control_t*)data;
    struct usbd_urb* urb;
    uint8_t* buffer;
    int status;

    urb=usbd_alloc_urb(NULL);
    if (urb==NULL)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't allocate urb");
        return -1;
    }

    buffer=usbd_alloc(size);
    if (buffer==NULL)
    {
        usbd_free_urb(urb);
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't allocate buffer");
        return -1;
    }

    /* Copy to USB buffer, handling USB endianess, UVC 1.0 */
    buffer[0]=ctrl->bmHint & 0x000000FF;
    buffer[1]=(ctrl->bmHint >> 8) & 0x000000FF;
    buffer[2]=ctrl->bFormatIndex;
    buffer[3]=ctrl->bFrameIndex;
    buffer[4]=ctrl->dwFrameInterval & 0x000000FF;
    buffer[5]=(ctrl->dwFrameInterval >> 8) & 0x000000FF;
    buffer[6]=(ctrl->dwFrameInterval >> 16) & 0x000000FF;
    buffer[7]=(ctrl->dwFrameInterval >> 24) & 0x000000FF;
    buffer[8]=ctrl->wKeyFrameRate & 0x000000FF;
    buffer[9]=(ctrl->wKeyFrameRate >> 8) & 0x000000FF;
    buffer[10]=ctrl->wPFrameRate & 0x000000FF;
    buffer[11]=(ctrl->wPFrameRate >> 8) & 0x000000FF;
    buffer[12]=ctrl->wCompQuality & 0x000000FF;
    buffer[13]=(ctrl->wCompQuality >> 8) & 0x000000FF;
    buffer[14]=ctrl->wCompWindowSize & 0x000000FF;
    buffer[15]=(ctrl->wCompWindowSize >> 8) & 0x000000FF;
    buffer[16]=ctrl->wDelay & 0x000000FF;
    buffer[17]=(ctrl->wDelay >> 8) & 0x000000FF;
    buffer[18]=ctrl->dwMaxVideoFrameSize & 0x000000FF;
    buffer[19]=(ctrl->dwMaxVideoFrameSize >> 8) & 0x000000FF;
    buffer[20]=(ctrl->dwMaxVideoFrameSize >> 16) & 0x000000FF;
    buffer[21]=(ctrl->dwMaxVideoFrameSize >> 24) & 0x000000FF;
    buffer[22]=ctrl->dwMaxPayloadTransferSize & 0x000000FF;
    buffer[23]=(ctrl->dwMaxPayloadTransferSize >> 8) & 0x000000FF;
    buffer[24]=(ctrl->dwMaxPayloadTransferSize >> 16) & 0x000000FF;
    buffer[25]=(ctrl->dwMaxPayloadTransferSize >> 24) & 0x000000FF;

    /* UVC 1.1 */
    if (size>UVC_PROBE_COMMIT_VER10_SIZE)
    {
        buffer[26]=ctrl->dwClockFrequency & 0x000000FF;
        buffer[27]=(ctrl->dwClockFrequency >> 8) & 0x000000FF;
        buffer[28]=(ctrl->dwClockFrequency >> 16) & 0x000000FF;
        buffer[29]=(ctrl->dwClockFrequency >> 24) & 0x000000FF;
        buffer[30]=ctrl->bmFramingInfo;
        buffer[31]=ctrl->bPreferedVersion;
        buffer[32]=ctrl->bMinVersion;
        buffer[33]=ctrl->bMaxVersion;
    }

    /* UVC 1.5 */
    if (size>UVC_PROBE_COMMIT_VER11_SIZE)
    {
        buffer[34]=ctrl->bUsage;
        buffer[35]=ctrl->bBitDepthLuma;
        buffer[36]=ctrl->bmSettings;
        buffer[37]=ctrl->bMaxNumberOfRefFramesPlus1;
        buffer[38]=ctrl->bmRateControlModes & 0x000000FF;
        buffer[39]=(ctrl->bmRateControlModes >> 8) & 0x000000FF;
        buffer[40]=ctrl->bmLayoutPerStream & 0x000000FF;
        buffer[41]=(ctrl->bmLayoutPerStream >> 8) & 0x000000FF;
        buffer[42]=(ctrl->bmLayoutPerStream >> 16) & 0x000000FF;
        buffer[43]=(ctrl->bmLayoutPerStream >> 24) & 0x000000FF;
        buffer[44]=(ctrl->bmLayoutPerStream >> 32) & 0x000000FF;
        buffer[45]=(ctrl->bmLayoutPerStream >> 40) & 0x000000FF;
        buffer[46]=(ctrl->bmLayoutPerStream >> 48) & 0x000000FF;
        buffer[47]=(ctrl->bmLayoutPerStream >> 56) & 0x000000FF;
    }

    if (uvc_verbose>3)
    {
        char tempbuf[128];
        int it, jt;
        char* selector_string=NULL;

        switch (selector)
        {
             case VVS_PROBE_CONTROL:
                  selector_string="probe";
                  break;
             case VVS_COMMIT_CONTROL:
                  selector_string="commit";
                  break;
             default:
                  selector_string="unknown";
                  break;
        }

        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "            USB streaming ctrl send, SET_CUR (%s)", selector_string);

        for (jt=0; jt<(size+15)/16; jt++)
        {
            strcpy(tempbuf, "                ");
            for (it=0; it<16; it++)
            {
                if (jt*16+it>=size)
                {
                    break;
                }
                sprintf(tempbuf+strlen(tempbuf), "%02X ", buffer[jt*16+it]);
            }
            slogf(_SLOGC_USB_GEN, _SLOG_INFO, "%s", tempbuf);
        }
    }

    slogf(_SLOGC_USB_GEN, _SLOG_INFO, "%02X %02X %02X %02X %02X %02X %02X %02X", UVC_REQUEST_SET_VC, operation, (selector<<8) & 0xFF, ((selector<<8)>>8) & 0xFF, iface, 0x00, size, 0x00);
    usbd_setup_vendor(urb, URB_DIR_OUT, operation, UVC_REQUEST_SET_VC, selector<<8, iface, buffer, size);
    status=usbd_io(urb, dev->vc_control_pipe, NULL, dev, USBD_TIME_INFINITY);
    if (status!=EOK)
    {
        usbd_free_urb(urb);
        usbd_free(buffer);
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't perform usb i/o operation");
        return -1;
    }

    usbd_free_urb(urb);
    usbd_free(buffer);

    return 0;
}

void uvc_isochronous_completion(struct usbd_urb* urb, struct usbd_pipe* pipe, void* handle)
{
    uvc_device_t* dev=(uvc_device_t*)handle;
    int subdev=-1;
    int status;
    uint32_t urb_status;
    uint32_t urb_len;
    int urb_id=-1;
    int it;

    slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "uvc_isochronous_completion():");
    status=usbd_urb_status(urb, &urb_status, &urb_len);
    slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "    status=%d, urb_status=%08X, urb_len=%d", status, urb_status, urb_len);

    for (it=0; it<dev->total_vs_devices; it++)
    {
        if (dev->vs_isochronous_pipe[it]==pipe)
        {
            subdev=it;
            break;
        }
    }

    if (subdev==-1)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Unknown isochronous pipe, please report");
        return;
    }

    for (it=0; it<UVC_MAX_ISO_BUFFERS; it++)
    {
        if (dev->iso_urb[subdev][it]==urb)
        {
            urb_id=it;
            break;
        }
    }
    slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "    subdev=%d, urb_id=%d", subdev, urb_id);

    if (urb_len>0)
    {
        for (it=0; it<UVC_MAX_ISO_FRAMES; it++)
        {
            int bytes_to_print;

            slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "        [%d] frame_status=%08X, frame_len=%d", it, dev->iso_list[subdev][urb_id][it].frame_status, dev->iso_list[subdev][urb_id][it].frame_len);
            if (dev->iso_list[subdev][urb_id][it].frame_len>=12)
            {
                bytes_to_print=(dev->iso_list[subdev][urb_id][it].frame_len>20)?20:12;
            }
            else
            {
                bytes_to_print=0;
            }
            if (bytes_to_print>0)
            {
                char tempbuf[256];
                int jt;

                tempbuf[0]=0;
                for (jt=0; jt<bytes_to_print; jt++)
                {
                    sprintf(tempbuf+strlen(tempbuf), "%02X ", dev->iso_buffer[subdev][urb_id][it*dev->iso_payload_size[subdev]+jt]);
                }
                slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "            %s", tempbuf);
            }
        }
    }

    /* Fill this URB with new data and push it back to USB stack */
    for (it=0; it<UVC_MAX_ISO_FRAMES; it++)
    {
        dev->iso_list[subdev][urb_id][it].frame_status=0;
        dev->iso_list[subdev][urb_id][it].frame_len=dev->iso_payload_size[subdev];
    }

    status=usbd_setup_isochronous_stream(dev->iso_urb[subdev][urb_id],
        URB_DIR_IN | URB_ISOCH_ASAP, 0, dev->iso_buffer[subdev][urb_id],
        UVC_MAX_ISO_FRAMES*dev->iso_payload_size[subdev], dev->iso_list[subdev][urb_id],
        UVC_MAX_ISO_FRAMES);
    if (status!=EOK)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't setup isochronous urb, please report");
    }
    status=usbd_io(dev->iso_urb[subdev][urb_id], dev->vs_isochronous_pipe[subdev],
        uvc_isochronous_completion, dev, USBD_TIME_INFINITY);
    if (status!=EOK)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't send isochronous urb, please report");
    }
}
