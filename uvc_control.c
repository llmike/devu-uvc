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

#include <linux/media.h>

#include "uuid.h"

#include "uvc.h"
#include "usbvc.h"
#include "uvc_control.h"

extern int uvc_verbose;

void uvc_parse_control_descriptor(uvc_device_t* uvcd, uint8_t* data)
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
        case VVC_HEADER:
             {
                 /* Clear header fields in case we will receive incomplete packet */
                 uvcd->vc_header.bcdUVC=0;
                 uvcd->vc_header.wTotalLength=0;
                 uvcd->vc_header.dwClockFrequency=0;
                 uvcd->vc_header.bInCollection=0;
                 memset(uvcd->vc_header.baInterfaceNr, 0x00, UVC_MAX_INTERFACES_COUNT);

                 if (size>4)
                 {
                     uvcd->vc_header.bcdUVC=data[3] | ((unsigned int)data[4]<<8);
                 }
                 if (size>6)
                 {
                     uvcd->vc_header.wTotalLength=data[5] | ((unsigned int)data[6]<<8);
                 }
                 if (size>10)
                 {
                     uvcd->vc_header.dwClockFrequency=data[7] | ((unsigned int)data[8]<<8) |
                        ((unsigned int)data[9]<<16) | ((unsigned int)data[10]<<24);
                 }
                 if (size>11)
                 {
                     uvcd->vc_header.bInCollection=data[11];
                     if (uvcd->vc_header.bInCollection>UVC_MAX_INTERFACES_COUNT)
                     {
                         uvcd->vc_header.bInCollection=UVC_MAX_INTERFACES_COUNT;
                         slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] VC/HEADER: too many interfaces");
                     }
                 }
                 for (it=0; it<uvcd->vc_header.bInCollection; it++)
                 {
                     if (size>12+it)
                     {
                         uvcd->vc_header.baInterfaceNr[it]=data[12+it];
                     }
                 }

                 if (uvc_verbose>1)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VC_HEADER:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bcdUVC: %04X", uvcd->vc_header.bcdUVC);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    wTotalLength: %04X", uvcd->vc_header.wTotalLength);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    dwClockFrequency: %08X (%d)", uvcd->vc_header.dwClockFrequency, uvcd->vc_header.dwClockFrequency);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bInCollection: %02X", uvcd->vc_header.bInCollection);
                     for (it=0; it<uvcd->vc_header.bInCollection; it++)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    baInterfaceNr[%d]: %02X", it, uvcd->vc_header.baInterfaceNr[it]);
                     }
                 }

                 /* Fill entity */
                 uvcd->entity[uvcd->media_entities].id=1;
                 strncpy(uvcd->entity[uvcd->media_entities].name, uvcd->device_id_str, sizeof(uvcd->entity[uvcd->media_entities].name));
                 uvcd->entity[uvcd->media_entities].type=MEDIA_ENT_T_DEVNODE_V4L;
                 uvcd->entity[uvcd->media_entities].revision=_MEDIA_KERNEL_VERSION(0, (uvcd->bcddevice>>8) & 0x000000FF, uvcd->bcddevice & 0x000000FF);
                 uvcd->entity[uvcd->media_entities].flags=MEDIA_ENT_FL_DEFAULT;
                 uvcd->entity[uvcd->media_entities].group_id=0;
                 uvcd->entity[uvcd->media_entities].pads=0;
                 uvcd->entity[uvcd->media_entities].links=0;
                 uvcd->media_entities++;
             }
             break;
        case VVC_INPUT_TERMINAL:
             {
                 /* Clear header fields in case we will receive incomplete packet */
                 uvcd->vc_iterminal.bTerminalID=0;
                 uvcd->vc_iterminal.wTerminalType=0;
                 uvcd->vc_iterminal.bAssocTerminal=0;
                 uvcd->vc_iterminal.iTerminal=0;
                 uvcd->vc_iterminal.wObjectiveFocalLengthMin=0;
                 uvcd->vc_iterminal.wObjectiveFocalLengthMax=0;
                 uvcd->vc_iterminal.wOcularFocalLength=0;
                 uvcd->vc_iterminal.bControlSize=0;
                 memset(uvcd->vc_iterminal.bmControls, 0x00, UVC_MAX_BMCONTROLS_COUNT);

                 if (size>3)
                 {
                     uvcd->vc_iterminal.bTerminalID=data[3];
                 }
                 if (size>5)
                 {
                     uvcd->vc_iterminal.wTerminalType=data[4] | ((unsigned int)data[5]<<8);
                 }
                 if (size>6)
                 {
                     uvcd->vc_iterminal.bAssocTerminal=data[6];
                 }
                 if (size>7)
                 {
                     uvcd->vc_iterminal.iTerminal=data[7];
                 }

                 if (uvc_verbose>1)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VC_INPUT_TERMINAL:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bTerminalID: %02X", uvcd->vc_iterminal.bTerminalID);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    wTerminalType: %04X", uvcd->vc_iterminal.wTerminalType);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bAssocTerminal: %02X", uvcd->vc_iterminal.bAssocTerminal);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    iTerminal: %02X", uvcd->vc_iterminal.iTerminal);
                 }

                 /* The rest fields available only for camera type */
                 if (uvcd->vc_iterminal.wTerminalType==VITT_CAMERA)
                 {
                     if (size>9)
                     {
                         uvcd->vc_iterminal.wObjectiveFocalLengthMin=data[8] | ((unsigned int)data[9]<<8);
                     }
                     if (size>11)
                     {
                         uvcd->vc_iterminal.wObjectiveFocalLengthMax=data[10] | ((unsigned int)data[11]<<8);
                     }
                     if (size>13)
                     {
                         uvcd->vc_iterminal.wOcularFocalLength=data[12] | ((unsigned int)data[13]<<8);
                     }
                     if (size>14)
                     {
                         uvcd->vc_iterminal.bControlSize=data[14];
                         if (uvcd->vc_iterminal.bControlSize>UVC_MAX_BMCONTROLS_COUNT)
                         {
                             uvcd->vc_iterminal.bControlSize=UVC_MAX_BMCONTROLS_COUNT;
                             slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] VC/ITERMINAL: too many controls");
                         }
                     }
                     for (it=0; it<uvcd->vc_iterminal.bControlSize; it++)
                     {
                         if (size>15+it)
                         {
                             uvcd->vc_iterminal.bmControls[it]=data[15+it];
                         }
                     }

                     if (uvc_verbose>1)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    wObjectiveFocalLengthMin: %04X", uvcd->vc_iterminal.wObjectiveFocalLengthMin);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    wObjectiveFocalLengthMax: %04X", uvcd->vc_iterminal.wObjectiveFocalLengthMax);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    wOcularFocalLength: %04X", uvcd->vc_iterminal.wOcularFocalLength);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bControlSize: %02X", uvcd->vc_iterminal.bControlSize);
                         for (it=0; it<uvcd->vc_iterminal.bControlSize; it++)
                         {
                             slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bmControls[%d]: %02X", it, uvcd->vc_iterminal.bmControls[it]);
                             if (uvc_verbose>2)
                             switch (it)
                             {
                                 case 0:
                                      if (uvcd->vc_iterminal.bmControls[it]&VIT_B0_SCANNING_MODE)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Scanning mode");
                                      }
                                      if (uvcd->vc_iterminal.bmControls[it]&VIT_B0_AUTO_EXPOSURE_MODE)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Auto-exposure mode");
                                      }
                                      if (uvcd->vc_iterminal.bmControls[it]&VIT_B0_AUTO_EXPOSURE_PRIORITY)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Auto-exposure priority");
                                      }
                                      if (uvcd->vc_iterminal.bmControls[it]&VIT_B0_EXPOSURE_TIME_ABSOLUTE)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Exposure time (absolute)");
                                      }
                                      if (uvcd->vc_iterminal.bmControls[it]&VIT_B0_EXPOSURE_TIME_RELATIVE)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Exposure time (relative)");
                                      }
                                      if (uvcd->vc_iterminal.bmControls[it]&VIT_B0_FOCUS_ABSOLUTE)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Focus (absolute)");
                                      }
                                      if (uvcd->vc_iterminal.bmControls[it]&VIT_B0_FOCUS_RELATIVE)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Focus (relative)");
                                      }
                                      if (uvcd->vc_iterminal.bmControls[it]&VIT_B0_IRIS_ABSOLUTE)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Iris (absolute)");
                                      }
                                      break;
                                 case 1:
                                      if (uvcd->vc_iterminal.bmControls[it]&VIT_B1_IRIS_RELATIVE)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Iris (relative)");
                                      }
                                      if (uvcd->vc_iterminal.bmControls[it]&VIT_B1_ZOOM_ABSOLUTE)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Zoom (absolute)");
                                      }
                                      if (uvcd->vc_iterminal.bmControls[it]&VIT_B1_ZOOM_RELATIVE)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Zoom (relative)");
                                      }
                                      if (uvcd->vc_iterminal.bmControls[it]&VIT_B1_PANTILT_ABSOLUTE)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        PanTilt (absolute)");
                                      }
                                      if (uvcd->vc_iterminal.bmControls[it]&VIT_B1_PANTILT_RELATIVE)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        PanTilt (relative)");
                                      }
                                      if (uvcd->vc_iterminal.bmControls[it]&VIT_B1_ROLL_ABSOLUTE)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Roll (absolute)");
                                      }
                                      if (uvcd->vc_iterminal.bmControls[it]&VIT_B1_ROLL_RELATIVE)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Roll (relative)");
                                      }
                                      break;
                                 case 2:
                                      if (uvcd->vc_iterminal.bmControls[it]&VIT_B2_FOCUS_AUTO)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Focus, auto");
                                      }
                                      if (uvcd->vc_iterminal.bmControls[it]&VIT_B2_PRIVACY)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Privacy");
                                      }
                                      if (uvcd->vc_iterminal.bmControls[it]&VIT_B2_FOCUS_SIMPLE)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Focus, simple");
                                      }
                                      if (uvcd->vc_iterminal.bmControls[it]&VIT_B2_WINDOW)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Window");
                                      }
                                      if (uvcd->vc_iterminal.bmControls[it]&VIT_B2_REGION_OF_INTEREST)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Region of interest");
                                      }
                                      break;
                             }
                         }
                     }
                 }

                 /* Fill entity */
                 uvcd->entity[uvcd->media_entities].id=uvcd->vc_iterminal.bTerminalID+1;
                 if (uvcd->vc_iterminal.iTerminal!=0)
                 {
                     strncpy(uvcd->entity[uvcd->media_entities].name,
                         usbd_string(uvcd->uvc_vc_device, uvcd->vc_iterminal.iTerminal, 0),
                         sizeof(uvcd->entity[uvcd->media_entities].name));
                 }
                 else
                 {
                     switch(uvcd->vc_iterminal.wTerminalType)
                     {
                         case VITT_VENDOR_SPECIFIC:
                              strncpy(uvcd->entity[uvcd->media_entities].name, "Vendor Input Terminal", sizeof(uvcd->entity[uvcd->media_entities].name));
                              break;
                         case VITT_CAMERA:
                              strncpy(uvcd->entity[uvcd->media_entities].name, "Camera", sizeof(uvcd->entity[uvcd->media_entities].name));
                              break;
                         case VITT_MEDIA_TRANSPORT_UNIT:
                              strncpy(uvcd->entity[uvcd->media_entities].name, "Media Transport", sizeof(uvcd->entity[uvcd->media_entities].name));
                              break;
                         case VETT_VENDOR_SPECIFIC:
                              strncpy(uvcd->entity[uvcd->media_entities].name, "Vendor External Terminal", sizeof(uvcd->entity[uvcd->media_entities].name));
                              break;
                         case VETT_COMPOSITE_CONNECTOR:
                              strncpy(uvcd->entity[uvcd->media_entities].name, "Composite Input", sizeof(uvcd->entity[uvcd->media_entities].name));
                              break;
                         case VETT_SVIDEO_CONNECTOR:
                              strncpy(uvcd->entity[uvcd->media_entities].name, "S-Video Input", sizeof(uvcd->entity[uvcd->media_entities].name));
                              break;
                         case VETT_COMPONENT_CONNECTOR:
                              strncpy(uvcd->entity[uvcd->media_entities].name, "Component Input", sizeof(uvcd->entity[uvcd->media_entities].name));
                              break;
                         case VTT_VENDOR_SPECIFIC:
                              strncpy(uvcd->entity[uvcd->media_entities].name, "Vendor Terminal", sizeof(uvcd->entity[uvcd->media_entities].name));
                              break;
                         case VTT_STREAMING:
                              strncpy(uvcd->entity[uvcd->media_entities].name, "Streaming", sizeof(uvcd->entity[uvcd->media_entities].name));
                              break;
                         default:
                              strncpy(uvcd->entity[uvcd->media_entities].name, "Unknown Input Terminal", sizeof(uvcd->entity[uvcd->media_entities].name));
                              break;
                     }
                 }
                 uvcd->entity[uvcd->media_entities].type=MEDIA_ENT_T_V4L2_SUBDEV;
                 uvcd->entity[uvcd->media_entities].revision=0;
                 uvcd->entity[uvcd->media_entities].flags=0;
                 uvcd->entity[uvcd->media_entities].group_id=0;
                 uvcd->entity[uvcd->media_entities].pads=1;
                 uvcd->entity[uvcd->media_entities].links=0;
                 if (uvcd->vc_iterminal.bAssocTerminal)
                 {
                     /* bi-derectional terminal */
                     uvcd->entity[uvcd->media_entities].pads++;
                 }

                 /* Fill pad information */
                 uvcd->entity_pads[uvcd->media_pads].entity=uvcd->entity[uvcd->media_entities].id;
                 uvcd->entity_pads[uvcd->media_pads].index=0;
                 uvcd->entity_pads[uvcd->media_pads].flags=MEDIA_PAD_FL_SOURCE;
                 uvcd->media_pads++;
                 if (uvcd->vc_iterminal.bAssocTerminal)
                 {
                     uvcd->entity_pads[uvcd->media_pads].entity=uvcd->entity[uvcd->media_entities].id;
                     uvcd->entity_pads[uvcd->media_pads].index=1;
                     uvcd->entity_pads[uvcd->media_pads].flags=MEDIA_PAD_FL_SINK;
                     uvcd->media_pads++;
                 }

                 uvcd->media_entities++;
             }
             break;
        case VVC_OUTPUT_TERMINAL:
             {
                 /* Clear header fields in case we will receive incomplete packet */
                 uvcd->vc_oterminal[uvcd->total_output_terminals].bTerminalID=0;
                 uvcd->vc_oterminal[uvcd->total_output_terminals].wTerminalType=0;
                 uvcd->vc_oterminal[uvcd->total_output_terminals].bAssocTerminal=0;
                 uvcd->vc_oterminal[uvcd->total_output_terminals].bSourceID=0;
                 uvcd->vc_oterminal[uvcd->total_output_terminals].iTerminal=0;

                 if (size>3)
                 {
                     uvcd->vc_oterminal[uvcd->total_output_terminals].bTerminalID=data[3];
                 }
                 if (size>5)
                 {
                     uvcd->vc_oterminal[uvcd->total_output_terminals].wTerminalType=data[4] | ((unsigned int)data[5]<<8);
                 }
                 if (size>6)
                 {
                     uvcd->vc_oterminal[uvcd->total_output_terminals].bAssocTerminal=data[6];
                 }
                 if (size>7)
                 {
                     uvcd->vc_oterminal[uvcd->total_output_terminals].bSourceID=data[7];
                 }
                 if (size>8)
                 {
                     uvcd->vc_oterminal[uvcd->total_output_terminals].iTerminal=data[8];
                 }

                 if (uvc_verbose>1)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VC_OUTPUT_TERMINAL:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bTerminalID: %02X", uvcd->vc_oterminal[uvcd->total_output_terminals].bTerminalID);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    wTerminalType: %04X", uvcd->vc_oterminal[uvcd->total_output_terminals].wTerminalType);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bAssocTerminal: %02X", uvcd->vc_oterminal[uvcd->total_output_terminals].bAssocTerminal);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bSourceID: %02X", uvcd->vc_oterminal[uvcd->total_output_terminals].bSourceID);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    iTerminal: %02X", uvcd->vc_oterminal[uvcd->total_output_terminals].iTerminal);
                 }

                 /* Fill entity */
                 uvcd->entity[uvcd->media_entities].id=uvcd->vc_oterminal[uvcd->total_output_terminals].bTerminalID+1;
                 if (uvcd->vc_oterminal[uvcd->total_output_terminals].iTerminal!=0)
                 {
                     strncpy(uvcd->entity[uvcd->media_entities].name,
                         usbd_string(uvcd->uvc_vc_device, uvcd->vc_oterminal[uvcd->total_output_terminals].iTerminal, 0),
                         sizeof(uvcd->entity[uvcd->media_entities].name));
                 }
                 else
                 {
                     switch(uvcd->vc_oterminal[uvcd->total_output_terminals].wTerminalType)
                     {
                         case VOTT_VENDOR_SPECIFIC:
                              strncpy(uvcd->entity[uvcd->media_entities].name, "Vendor Output Terminal", sizeof(uvcd->entity[uvcd->media_entities].name));
                              break;
                         case VOTT_DISPLAY:
                              strncpy(uvcd->entity[uvcd->media_entities].name, "Display", sizeof(uvcd->entity[uvcd->media_entities].name));
                              break;
                         case VOTT_MEDIA_TRANSPORT_UNIT:
                              strncpy(uvcd->entity[uvcd->media_entities].name, "Media Transport", sizeof(uvcd->entity[uvcd->media_entities].name));
                              break;
                         case VETT_COMPOSITE_CONNECTOR:
                              strncpy(uvcd->entity[uvcd->media_entities].name, "Composite Output", sizeof(uvcd->entity[uvcd->media_entities].name));
                              break;
                         case VETT_SVIDEO_CONNECTOR:
                              strncpy(uvcd->entity[uvcd->media_entities].name, "S-Video Output", sizeof(uvcd->entity[uvcd->media_entities].name));
                              break;
                         case VETT_COMPONENT_CONNECTOR:
                              strncpy(uvcd->entity[uvcd->media_entities].name, "Component Output", sizeof(uvcd->entity[uvcd->media_entities].name));
                              break;
                         case VTT_VENDOR_SPECIFIC:
                              strncpy(uvcd->entity[uvcd->media_entities].name, "Vendor Terminal", sizeof(uvcd->entity[uvcd->media_entities].name));
                              break;
                         case VTT_STREAMING:
                              strncpy(uvcd->entity[uvcd->media_entities].name, "Streaming", sizeof(uvcd->entity[uvcd->media_entities].name));
                              break;
                         default:
                              strncpy(uvcd->entity[uvcd->media_entities].name, "Unknown Output Terminal", sizeof(uvcd->entity[uvcd->media_entities].name));
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "OT: %08X", uvcd->vc_oterminal[uvcd->total_output_terminals].wTerminalType);
                              break;
                     }
                 }
                 uvcd->entity[uvcd->media_entities].type=MEDIA_ENT_T_V4L2_SUBDEV;
                 uvcd->entity[uvcd->media_entities].revision=0;
                 uvcd->entity[uvcd->media_entities].flags=0;
                 uvcd->entity[uvcd->media_entities].group_id=0;
                 uvcd->entity[uvcd->media_entities].pads=1;
                 uvcd->entity[uvcd->media_entities].links=0;
                 if (uvcd->vc_oterminal[uvcd->total_output_terminals].bAssocTerminal)
                 {
                     uvcd->entity[uvcd->media_entities].pads++;
                 }

                 /* Fill pad information */
                 uvcd->entity_pads[uvcd->media_pads].entity=uvcd->entity[uvcd->media_entities].id;
                 uvcd->entity_pads[uvcd->media_pads].index=0;
                 uvcd->entity_pads[uvcd->media_pads].flags=MEDIA_PAD_FL_SINK;
                 uvcd->media_pads++;
                 if (uvcd->vc_oterminal[uvcd->total_output_terminals].bAssocTerminal)
                 {
                     uvcd->entity_pads[uvcd->media_pads].entity=uvcd->entity[uvcd->media_entities].id;
                     uvcd->entity_pads[uvcd->media_pads].index=1;
                     uvcd->entity_pads[uvcd->media_pads].flags=MEDIA_PAD_FL_SOURCE;
                     uvcd->media_pads++;
                 }

                 /* Fill links information */
                 uvcd->entity_links[uvcd->media_links].flags=MEDIA_LNK_FL_IMMUTABLE | MEDIA_LNK_FL_ENABLED;
                 uvcd->entity_links[uvcd->media_links].source.entity=uvcd->vc_oterminal[uvcd->total_output_terminals].bSourceID+1;
                 uvcd->entity_links[uvcd->media_links].source.index=0;
                 uvcd->entity_links[uvcd->media_links].source.flags=MEDIA_PAD_FL_SOURCE;
                 uvcd->entity_links[uvcd->media_links].sink.entity=uvcd->vc_oterminal[uvcd->total_output_terminals].bTerminalID+1;
                 uvcd->entity_links[uvcd->media_links].sink.index=0;
                 uvcd->entity_links[uvcd->media_links].sink.flags=MEDIA_PAD_FL_SINK;
                 uvcd->media_links++;
                 if (uvcd->vc_oterminal[uvcd->total_output_terminals].bAssocTerminal)
                 {
                     uvcd->entity_links[uvcd->media_links].flags=MEDIA_LNK_FL_IMMUTABLE | MEDIA_LNK_FL_ENABLED;
                     uvcd->entity_links[uvcd->media_links].source.entity=uvcd->vc_oterminal[uvcd->total_output_terminals].bTerminalID+1;
                     uvcd->entity_links[uvcd->media_links].source.index=1;
                     uvcd->entity_links[uvcd->media_links].source.flags=MEDIA_PAD_FL_SOURCE;
                     uvcd->entity_links[uvcd->media_links].sink.entity=uvcd->vc_oterminal[uvcd->total_output_terminals].bSourceID+1;
                     uvcd->entity_links[uvcd->media_links].sink.index=1;
                     uvcd->entity_links[uvcd->media_links].sink.flags=MEDIA_PAD_FL_SINK;
                     uvcd->media_links++;
                 }

                 uvcd->media_entities++;
                 uvcd->total_output_terminals++;
             }
             break;
        case VVC_SELECTOR_UNIT:
             {
                 /* Clear header fields in case we will receive incomplete packet */
                 uvcd->vc_selector_unit.bUnitID=0;
                 uvcd->vc_selector_unit.bNrInPins=0;
                 memset(uvcd->vc_selector_unit.baSourceID, 0x00, UVC_MAX_SOURCEIDS_COUNT);
                 uvcd->vc_selector_unit.iSelector=0;

                 if (size>3)
                 {
                     uvcd->vc_selector_unit.bUnitID=data[3];
                 }
                 if (size>4)
                 {
                     uvcd->vc_selector_unit.bNrInPins=data[4];
                     if (uvcd->vc_selector_unit.bNrInPins>UVC_MAX_SOURCEIDS_COUNT)
                     {
                         uvcd->vc_selector_unit.bNrInPins=UVC_MAX_SOURCEIDS_COUNT;
                         slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] VC/SELECTOR_UNIT: too many source ids");
                     }
                 }

                 for (it=0; it<uvcd->vc_selector_unit.bNrInPins; it++)
                 {
                     if (size>5+it)
                     {
                         uvcd->vc_selector_unit.baSourceID[it]=data[5+it];
                     }
                 }

                 if (size>5+uvcd->vc_selector_unit.bNrInPins)
                 {
                     uvcd->vc_selector_unit.iSelector=data[5+uvcd->vc_selector_unit.bNrInPins];
                 }

                 if (uvc_verbose>1)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VC_SELECTOR_UNIT:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bUnitID: %02X", uvcd->vc_selector_unit.bUnitID);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bNrInPins: %02X", uvcd->vc_selector_unit.bNrInPins);
                     for (it=0; it<uvcd->vc_selector_unit.bNrInPins; it++)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    baSourceID[%d]: %02X", it, uvcd->vc_selector_unit.baSourceID[it]);
                     }
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    iSelector: %02X", uvcd->vc_selector_unit.iSelector);
                 }

                 /* Fill entity */
                 uvcd->entity[uvcd->media_entities].id=uvcd->vc_selector_unit.bUnitID+1;
                 if (uvcd->vc_selector_unit.iSelector!=0)
                 {
                     strncpy(uvcd->entity[uvcd->media_entities].name,
                         usbd_string(uvcd->uvc_vc_device, uvcd->vc_selector_unit.iSelector, 0),
                         sizeof(uvcd->entity[uvcd->media_entities].name));
                 }
                 else
                 {
                     strncpy(uvcd->entity[uvcd->media_entities].name, "Input Selector", sizeof(uvcd->entity[uvcd->media_entities].name));
                 }
                 uvcd->entity[uvcd->media_entities].type=MEDIA_ENT_T_V4L2_SUBDEV;
                 uvcd->entity[uvcd->media_entities].revision=0;
                 uvcd->entity[uvcd->media_entities].flags=0;
                 uvcd->entity[uvcd->media_entities].group_id=0;
                 uvcd->entity[uvcd->media_entities].pads=2;
                 uvcd->entity[uvcd->media_entities].links=0;

                 /* Fill pad information */
                 uvcd->entity_pads[uvcd->media_pads].entity=uvcd->entity[uvcd->media_entities].id;
                 uvcd->entity_pads[uvcd->media_pads].index=0;
                 uvcd->entity_pads[uvcd->media_pads].flags=MEDIA_PAD_FL_SOURCE;
                 uvcd->media_pads++;
                 uvcd->entity_pads[uvcd->media_pads].entity=uvcd->entity[uvcd->media_entities].id;
                 uvcd->entity_pads[uvcd->media_pads].index=1;
                 uvcd->entity_pads[uvcd->media_pads].flags=MEDIA_PAD_FL_SINK;
                 uvcd->media_pads++;

                 /* Fill links information */
                 for (it=0; it<uvcd->vc_selector_unit.bNrInPins; it++)
                 {
                     uvcd->entity_links[uvcd->media_links].flags=MEDIA_LNK_FL_IMMUTABLE;
                     if (it==0)
                     {
                         uvcd->entity_links[uvcd->media_links].flags|=MEDIA_LNK_FL_ENABLED;
                     }
                     uvcd->entity_links[uvcd->media_links].source.entity=uvcd->vc_selector_unit.baSourceID[it]+1;
                     uvcd->entity_links[uvcd->media_links].source.index=0;
                     uvcd->entity_links[uvcd->media_links].source.flags=MEDIA_PAD_FL_SOURCE;
                     uvcd->entity_links[uvcd->media_links].sink.entity=uvcd->vc_selector_unit.bUnitID+1;
                     uvcd->entity_links[uvcd->media_links].sink.index=1;
                     uvcd->entity_links[uvcd->media_links].sink.flags=MEDIA_PAD_FL_SINK;
                     uvcd->media_links++;
                 }

                 uvcd->media_entities++;
             }
             break;
        case VVC_PROCESSING_UNIT:
             {
                 /* Clear header fields in case we will receive incomplete packet */
                 uvcd->vc_processing_unit.bUnitID=0;
                 uvcd->vc_processing_unit.bSourceID=0;
                 uvcd->vc_processing_unit.wMaxMultiplier=0;
                 uvcd->vc_processing_unit.bControlSize=0;
                 uvcd->vc_processing_unit.iProcessing=0;
                 uvcd->vc_processing_unit.bmVideoStandards=0;
                 memset(uvcd->vc_processing_unit.bmControls, 0x00, UVC_MAX_BMCONTROLS_COUNT);

                 if (size>3)
                 {
                     uvcd->vc_processing_unit.bUnitID=data[3];
                 }
                 if (size>4)
                 {
                     uvcd->vc_processing_unit.bSourceID=data[4];
                 }
                 if (size>6)
                 {
                     uvcd->vc_processing_unit.wMaxMultiplier=data[5] | ((unsigned int)data[6]<<8);
                 }

                 if (size>7)
                 {
                     uvcd->vc_processing_unit.bControlSize=data[7];
                     if (uvcd->vc_processing_unit.bControlSize>UVC_MAX_BMCONTROLS_COUNT)
                     {
                         uvcd->vc_processing_unit.bControlSize=UVC_MAX_BMCONTROLS_COUNT;
                         slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] VC/PROCESSING_UNIT: too many controls");
                     }
                 }

                 for (it=0; it<uvcd->vc_processing_unit.bControlSize; it++)
                 {
                     if (size>8+it)
                     {
                         uvcd->vc_processing_unit.bmControls[it]=data[8+it];
                     }
                 }
                 if (size>8+uvcd->vc_processing_unit.bControlSize)
                 {
                     uvcd->vc_processing_unit.iProcessing=data[8+uvcd->vc_processing_unit.bControlSize];
                 }
                 if (size>9+uvcd->vc_processing_unit.bControlSize)
                 {
                      uvcd->vc_processing_unit.bmVideoStandards=data[9+uvcd->vc_processing_unit.bControlSize];
                 }

                 if (uvc_verbose>1)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VC_PROCESSING_UNIT:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bUnitID: %02X", uvcd->vc_processing_unit.bUnitID);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bSourceID: %02X", uvcd->vc_processing_unit.bSourceID);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    wMaxMultiplier: %04X", uvcd->vc_processing_unit.wMaxMultiplier);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bControlSize: %02X", uvcd->vc_processing_unit.bControlSize);
                     for (it=0; it<uvcd->vc_processing_unit.bControlSize; it++)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bmControls[%d]: %02X", it, uvcd->vc_processing_unit.bmControls[it]);
                         if (uvc_verbose>2)
                         {
                             switch(it)
                             {
                                 case 0:
                                      if (uvcd->vc_processing_unit.bmControls[it]&VPU_B0_BRIGHTNESS)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Brightness");
                                      }
                                      if (uvcd->vc_processing_unit.bmControls[it]&VPU_B0_CONTRAST)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Contrast");
                                      }
                                      if (uvcd->vc_processing_unit.bmControls[it]&VPU_B0_HUE)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Hue");
                                      }
                                      if (uvcd->vc_processing_unit.bmControls[it]&VPU_B0_SATURATION)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Saturation");
                                      }
                                      if (uvcd->vc_processing_unit.bmControls[it]&VPU_B0_SHARPNESS)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Sharpness");
                                      }
                                      if (uvcd->vc_processing_unit.bmControls[it]&VPU_B0_GAMMA)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Gamma");
                                      }
                                      if (uvcd->vc_processing_unit.bmControls[it]&VPU_B0_WHITE_BALANCE_TEMPERATURE)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        White balance temperature");
                                      }
                                      if (uvcd->vc_processing_unit.bmControls[it]&VPU_B0_WHITE_BALANCE_COMPONENT)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        White balance component");
                                      }
                                      break;
                                 case 1:
                                      if (uvcd->vc_processing_unit.bmControls[it]&VPU_B1_BACKLIGHT_COMPENSATION)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Backlight compensation");
                                      }
                                      if (uvcd->vc_processing_unit.bmControls[it]&VPU_B1_GAIN)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Gain");
                                      }
                                      if (uvcd->vc_processing_unit.bmControls[it]&VPU_B1_POWER_LINE_FREQUENCY)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Power line frequency");
                                      }
                                      if (uvcd->vc_processing_unit.bmControls[it]&VPU_B1_HUE_AUTO)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Hue, auto");
                                      }
                                      if (uvcd->vc_processing_unit.bmControls[it]&VPU_B1_WHITE_BALANCE_TEMPERATURE_AUTO)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        White balance temperature, auto");
                                      }
                                      if (uvcd->vc_processing_unit.bmControls[it]&VPU_B1_WHITE_BALANCE_COMPONENT_AUTO)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        White balance component, auto");
                                      }
                                      if (uvcd->vc_processing_unit.bmControls[it]&VPU_B1_DIGITAL_MULTIPLIER)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Digital multiplier");
                                      }
                                      if (uvcd->vc_processing_unit.bmControls[it]&VPU_B1_DIGITAL_MULTIPLIER_LIMIT)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Digital multiplier limit");
                                      }
                                      break;
                                 case 2:
                                      if (uvcd->vc_processing_unit.bmControls[it]&VPU_B2_ANALOG_VIDEO_STANDARD)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Analog video standard");
                                      }
                                      if (uvcd->vc_processing_unit.bmControls[it]&VPU_B2_ANALOG_VIDEO_LOCK_STATUS)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Analog video lock status");
                                      }
                                      if (uvcd->vc_processing_unit.bmControls[it]&VPU_B2_CONTRAST_AUTO)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Contrast, auto");
                                      }
                                      break;
                             }
                         }
                     }
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    iProcessing: %02X", uvcd->vc_processing_unit.iProcessing);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bmVideoStandards: %02X", uvcd->vc_processing_unit.bmVideoStandards);
                 }

                 /* Fill entity */
                 uvcd->entity[uvcd->media_entities].id=uvcd->vc_processing_unit.bUnitID+1;
                 if (uvcd->vc_processing_unit.iProcessing!=0)
                 {
                     strncpy(uvcd->entity[uvcd->media_entities].name,
                         usbd_string(uvcd->uvc_vc_device, uvcd->vc_processing_unit.iProcessing, 0),
                         sizeof(uvcd->entity[uvcd->media_entities].name));
                 }
                 else
                 {
                     strncpy(uvcd->entity[uvcd->media_entities].name, "Processing Unit", sizeof(uvcd->entity[uvcd->media_entities].name));
                 }
                 uvcd->entity[uvcd->media_entities].type=MEDIA_ENT_T_V4L2_SUBDEV;
                 uvcd->entity[uvcd->media_entities].revision=0;
                 uvcd->entity[uvcd->media_entities].flags=0;
                 uvcd->entity[uvcd->media_entities].group_id=0;
                 uvcd->entity[uvcd->media_entities].pads=2;
                 uvcd->entity[uvcd->media_entities].links=0;

                 /* Fill pad information */
                 uvcd->entity_pads[uvcd->media_pads].entity=uvcd->entity[uvcd->media_entities].id;
                 uvcd->entity_pads[uvcd->media_pads].index=1;
                 uvcd->entity_pads[uvcd->media_pads].flags=MEDIA_PAD_FL_SOURCE;
                 uvcd->media_pads++;
                 uvcd->entity_pads[uvcd->media_pads].entity=uvcd->entity[uvcd->media_entities].id;
                 uvcd->entity_pads[uvcd->media_pads].index=0;
                 uvcd->entity_pads[uvcd->media_pads].flags=MEDIA_PAD_FL_SINK;
                 uvcd->media_pads++;

                 /* Fill links information */
                 uvcd->entity_links[uvcd->media_links].flags=MEDIA_LNK_FL_IMMUTABLE | MEDIA_LNK_FL_ENABLED;
                 uvcd->entity_links[uvcd->media_links].source.entity=uvcd->vc_processing_unit.bSourceID+1;
                 uvcd->entity_links[uvcd->media_links].source.index=0;
                 uvcd->entity_links[uvcd->media_links].source.flags=MEDIA_PAD_FL_SOURCE;
                 uvcd->entity_links[uvcd->media_links].sink.entity=uvcd->vc_processing_unit.bUnitID+1;
                 uvcd->entity_links[uvcd->media_links].sink.index=1;
                 uvcd->entity_links[uvcd->media_links].sink.flags=MEDIA_PAD_FL_SINK;
                 uvcd->media_links++;

                 uvcd->media_entities++;
             }
             break;
        case VVC_EXTENSION_UNIT:
             {
                 /* Clear header fields in case we will receive incomplete packet */
                 uvcd->vc_extension[uvcd->total_vc_extensions].bUnitID=0;
                 memset(uvcd->vc_extension[uvcd->total_vc_extensions].guidExtensionCode, 0x00, 16);
                 uvcd->vc_extension[uvcd->total_vc_extensions].bNumControls=0;
                 uvcd->vc_extension[uvcd->total_vc_extensions].bNrInPins=0;
                 memset(uvcd->vc_extension[uvcd->total_vc_extensions].baSourceID, 0x00, UVC_MAX_SOURCEPINS_COUNT);
                 uvcd->vc_extension[uvcd->total_vc_extensions].bControlSize=0;
                 memset(uvcd->vc_extension[uvcd->total_vc_extensions].bmControls, 0x00, UVC_MAX_BMCONTROLS_COUNT);
                 uvcd->vc_extension[uvcd->total_vc_extensions].iExtension=0;

                 if (size>3)
                 {
                     uvcd->vc_extension[uvcd->total_vc_extensions].bUnitID=data[3];
                 }
                 for (it=0; it<16; it++)
                 {
                     if (size>4+it)
                     {
                         uvcd->vc_extension[uvcd->total_vc_extensions].guidExtensionCode[it]=data[4+it];
                     }
                 }

                 if (size>4+16)
                 {
                      uvcd->vc_extension[uvcd->total_vc_extensions].bNumControls=data[4+16];
                 }

                 if (size>5+16)
                 {
                     uvcd->vc_extension[uvcd->total_vc_extensions].bNrInPins=data[5+16];
                     if (uvcd->vc_extension[uvcd->total_vc_extensions].bNrInPins>UVC_MAX_SOURCEPINS_COUNT)
                     {
                         uvcd->vc_extension[uvcd->total_vc_extensions].bNrInPins=UVC_MAX_SOURCEPINS_COUNT;
                         slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] VC/EXTENSION_UNIT: too many source ids");
                     }
                 }

                 for (it=0; it<uvcd->vc_extension[uvcd->total_vc_extensions].bNrInPins; it++)
                 {
                     if (size>6+16+it)
                     {
                         uvcd->vc_extension[uvcd->total_vc_extensions].baSourceID[it]=data[6+16+it];
                     }
                 }

                 if (size>6+16+uvcd->vc_extension[uvcd->total_vc_extensions].bNrInPins)
                 {
                     uvcd->vc_extension[uvcd->total_vc_extensions].bControlSize=
                         data[6+16+uvcd->vc_extension[uvcd->total_vc_extensions].bNrInPins];
                     if (uvcd->vc_extension[uvcd->total_vc_extensions].bControlSize>UVC_MAX_BMCONTROLS_COUNT)
                     {
                         uvcd->vc_extension[uvcd->total_vc_extensions].bControlSize=UVC_MAX_BMCONTROLS_COUNT;
                         slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] VC/EXTENSION_UNIT: too many controls");
                     }
                 }

                 for (it=0; it<uvcd->vc_extension[uvcd->total_vc_extensions].bControlSize; it++)
                 {
                     if (size>7+16+uvcd->vc_extension[uvcd->total_vc_extensions].bNrInPins+it)
                     {
                         uvcd->vc_extension[uvcd->total_vc_extensions].bmControls[it]=
                             data[7+16+uvcd->vc_extension[uvcd->total_vc_extensions].bNrInPins+it];
                     }
                 }

                 if (size>7+16+uvcd->vc_extension[uvcd->total_vc_extensions].bNrInPins+
                     uvcd->vc_extension[uvcd->total_vc_extensions].bControlSize)
                 {
                     uvcd->vc_extension[uvcd->total_vc_extensions].iExtension=
                         data[7+16+uvcd->vc_extension[uvcd->total_vc_extensions].bNrInPins+
                         uvcd->vc_extension[uvcd->total_vc_extensions].bControlSize];
                 }

                 if (uvc_verbose>1)
                 {
                     char temp_str[128];

                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VC_EXTENSION:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bUnitID: %02X", uvcd->vc_extension[uvcd->total_vc_extensions].bUnitID);
                     uuid_unparse_upper(uvcd->vc_extension[uvcd->total_vc_extensions].guidExtensionCode, temp_str);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    guidExtensionCode: %s", temp_str);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bNumControls: %02X", uvcd->vc_extension[uvcd->total_vc_extensions].bNumControls);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bNrInPins: %02X", uvcd->vc_extension[uvcd->total_vc_extensions].bNrInPins);
                     for (it=0; it<uvcd->vc_extension[uvcd->total_vc_extensions].bNrInPins; it++)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    baSourceID[%d]: %02X", it, uvcd->vc_extension[uvcd->total_vc_extensions].baSourceID[it]);
                     }
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bControlSize: %02X", uvcd->vc_extension[uvcd->total_vc_extensions].bControlSize);
                     for (it=0; it<uvcd->vc_extension[uvcd->total_vc_extensions].bControlSize; it++)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bmControls[%d]: %02X", it, uvcd->vc_extension[uvcd->total_vc_extensions].bmControls[it]);
                     }
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    iExtension: %02X", uvcd->vc_extension[uvcd->total_vc_extensions].iExtension);
                 }

                 /* Fill entity */
                 uvcd->entity[uvcd->media_entities].id=uvcd->vc_extension[uvcd->total_vc_extensions].bUnitID+1;
                 if (uvcd->vc_extension[uvcd->total_vc_extensions].iExtension!=0)
                 {
                     strncpy(uvcd->entity[uvcd->media_entities].name,
                         usbd_string(uvcd->uvc_vc_device, uvcd->vc_extension[uvcd->total_vc_extensions].iExtension, 0),
                         sizeof(uvcd->entity[uvcd->media_entities].name));
                 }
                 else
                 {
                     strncpy(uvcd->entity[uvcd->media_entities].name, "Extension Unit", sizeof(uvcd->entity[uvcd->media_entities].name));
                 }
                 uvcd->entity[uvcd->media_entities].type=MEDIA_ENT_T_V4L2_SUBDEV;
                 uvcd->entity[uvcd->media_entities].revision=0;
                 uvcd->entity[uvcd->media_entities].flags=0;
                 uvcd->entity[uvcd->media_entities].group_id=0;
                 uvcd->entity[uvcd->media_entities].pads=2;
                 uvcd->entity[uvcd->media_entities].links=0;

                 /* Fill pad information */
                 uvcd->entity_pads[uvcd->media_pads].entity=uvcd->entity[uvcd->media_entities].id;
                 uvcd->entity_pads[uvcd->media_pads].index=0;
                 uvcd->entity_pads[uvcd->media_pads].flags=MEDIA_PAD_FL_SOURCE;
                 uvcd->media_pads++;
                 uvcd->entity_pads[uvcd->media_pads].entity=uvcd->entity[uvcd->media_entities].id;
                 uvcd->entity_pads[uvcd->media_pads].index=1;
                 uvcd->entity_pads[uvcd->media_pads].flags=MEDIA_PAD_FL_SINK;
                 uvcd->media_pads++;

                 /* Fill links information */
                 for (it=0; it<uvcd->vc_extension[uvcd->total_vc_extensions].bNrInPins; it++)
                 {
                     uvcd->entity_links[uvcd->media_links].flags=MEDIA_LNK_FL_IMMUTABLE | MEDIA_LNK_FL_ENABLED;
                     uvcd->entity_links[uvcd->media_links].source.entity=uvcd->vc_processing_unit.bSourceID+1;
                     uvcd->entity_links[uvcd->media_links].source.index=0;
                     uvcd->entity_links[uvcd->media_links].source.flags=MEDIA_PAD_FL_SOURCE;
                     uvcd->entity_links[uvcd->media_links].sink.entity=uvcd->vc_extension[uvcd->total_vc_extensions].baSourceID[it]+1;
                     uvcd->entity_links[uvcd->media_links].sink.index=1;
                     uvcd->entity_links[uvcd->media_links].sink.flags=MEDIA_PAD_FL_SINK;
                     uvcd->media_links++;
                 }

                 uvcd->media_entities++;
                 uvcd->total_vc_extensions++;
             }
             break;
        case VVC_ENCODING_UNIT:
             {
                 /* Clear header fields in case we will receive incomplete packet */
                 uvcd->vc_encoding_unit.bUnitID=0;
                 uvcd->vc_encoding_unit.bSourceID=0;
                 uvcd->vc_encoding_unit.iEncoding=0;
                 uvcd->vc_encoding_unit.bControlSize=0;
                 memset(uvcd->vc_encoding_unit.bmControls, 0x00, 0x03);
                 memset(uvcd->vc_encoding_unit.bmControlsRuntime, 0x00, 0x03);

                 if (size>3)
                 {
                     uvcd->vc_encoding_unit.bUnitID=data[3];
                 }
                 if (size>4)
                 {
                     uvcd->vc_encoding_unit.bSourceID=data[4];
                 }
                 if (size>5)
                 {
                     uvcd->vc_encoding_unit.iEncoding=data[5];
                 }
                 if (size>6)
                 {
                     uvcd->vc_encoding_unit.bControlSize=data[6];
                 }
                 if (size>9)
                 {
                     uvcd->vc_encoding_unit.bmControls[0]=data[7];
                     uvcd->vc_encoding_unit.bmControls[1]=data[8];
                     uvcd->vc_encoding_unit.bmControls[2]=data[9];
                 }
                 if (size>12)
                 {
                     uvcd->vc_encoding_unit.bmControlsRuntime[0]=data[10];
                     uvcd->vc_encoding_unit.bmControlsRuntime[1]=data[11];
                     uvcd->vc_encoding_unit.bmControlsRuntime[2]=data[12];
                 }
                 if (uvc_verbose>1)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  VC_ENCODING_UNIT:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bUnitID: %02X", uvcd->vc_encoding_unit.bUnitID);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bSourceID: %02X", uvcd->vc_encoding_unit.bSourceID);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    iEncoding: %02X", uvcd->vc_encoding_unit.iEncoding);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bControlSize: %02X", uvcd->vc_encoding_unit.bControlSize);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bmControls[0]: %02X", uvcd->vc_encoding_unit.bmControls[0]);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bmControls[1]: %02X", uvcd->vc_encoding_unit.bmControls[1]);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bmControls[2]: %02X", uvcd->vc_encoding_unit.bmControls[2]);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bmControlsRuntime[0]: %02X", uvcd->vc_encoding_unit.bmControlsRuntime[0]);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bmControlsRuntime[1]: %02X", uvcd->vc_encoding_unit.bmControlsRuntime[1]);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bmControlsRuntime[2]: %02X", uvcd->vc_encoding_unit.bmControlsRuntime[2]);
                 }

                 /* Fill entity */
                 uvcd->entity[uvcd->media_entities].id=uvcd->vc_encoding_unit.bUnitID+1;
                 if (uvcd->vc_encoding_unit.iEncoding!=0)
                 {
                     strncpy(uvcd->entity[uvcd->media_entities].name,
                         usbd_string(uvcd->uvc_vc_device, uvcd->vc_encoding_unit.iEncoding, 0),
                         sizeof(uvcd->entity[uvcd->media_entities].name));
                 }
                 else
                 {
                     strncpy(uvcd->entity[uvcd->media_entities].name, "Encoding Unit", sizeof(uvcd->entity[uvcd->media_entities].name));
                 }
                 uvcd->entity[uvcd->media_entities].type=MEDIA_ENT_T_V4L2_SUBDEV;
                 uvcd->entity[uvcd->media_entities].revision=0;
                 uvcd->entity[uvcd->media_entities].flags=0;
                 uvcd->entity[uvcd->media_entities].group_id=0;
                 uvcd->entity[uvcd->media_entities].pads=2;
                 uvcd->entity[uvcd->media_entities].links=0;

                 /* Fill pad information */
                 uvcd->entity_pads[uvcd->media_pads].entity=uvcd->entity[uvcd->media_entities].id;
                 uvcd->entity_pads[uvcd->media_pads].index=0;
                 uvcd->entity_pads[uvcd->media_pads].flags=MEDIA_PAD_FL_SOURCE;
                 uvcd->media_pads++;

                 uvcd->entity_pads[uvcd->media_pads].entity=uvcd->entity[uvcd->media_entities].id;
                 uvcd->entity_pads[uvcd->media_pads].index=1;
                 uvcd->entity_pads[uvcd->media_pads].flags=MEDIA_PAD_FL_SINK;
                 uvcd->media_pads++;

                 /* Fill links information */
                 uvcd->entity_links[uvcd->media_links].flags=MEDIA_LNK_FL_IMMUTABLE | MEDIA_LNK_FL_ENABLED;
                 uvcd->entity_links[uvcd->media_links].source.entity=uvcd->vc_encoding_unit.bSourceID+1;
                 uvcd->entity_links[uvcd->media_links].source.index=0;
                 uvcd->entity_links[uvcd->media_links].source.flags=MEDIA_PAD_FL_SOURCE;
                 uvcd->entity_links[uvcd->media_links].sink.entity=uvcd->vc_encoding_unit.bUnitID+1;
                 uvcd->entity_links[uvcd->media_links].sink.index=1;
                 uvcd->entity_links[uvcd->media_links].sink.flags=MEDIA_PAD_FL_SINK;
                 uvcd->media_links++;

                 uvcd->media_entities++;
             }
             break;
        default:
            return;
    }
}

int uvc_control_get(uvc_device_t* dev, int operation, int unit, int selector, int size, uint8_t* data)
{
    struct usbd_urb* urb;
    uint8_t* buffer;
    int status;
    uint16_t unit_id=0;
    int it;

    switch (unit)
    {
        case UVC_PUNIT_SELECTOR:
             unit_id=dev->vc_processing_unit.bUnitID;
             break;
        case UVC_CAMERA_SELECTOR:
             unit_id=dev->vc_iterminal.bTerminalID;
             break;
        case UVC_ISEL_SELECTOR:
             unit_id=dev->vc_selector_unit.bUnitID;
             break;
        default:
             slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Unknown destination unit");
             return -1;
    }

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
    memset(buffer, 0x00, size);

    usbd_setup_vendor(urb, URB_DIR_IN, operation, UVC_REQUEST_GET_VC, selector<<8, unit_id<<8, buffer, size);
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
        char temp[128]={0};

        switch (operation)
        {
            case VGET_CUR:
                 sprintf(temp+strlen(temp), "GET_CUR: ");
                 break;
            case VGET_MIN:
                 sprintf(temp+strlen(temp), "GET_MIN: ");
                 break;
            case VGET_MAX:
                 sprintf(temp+strlen(temp), "GET_MAX: ");
                 break;
            case VGET_RES:
                 sprintf(temp+strlen(temp), "GET_RES: ");
                 break;
            case VGET_LEN:
                 sprintf(temp+strlen(temp), "GET_LEN: ");
                 break;
            case VGET_INFO:
                 sprintf(temp+strlen(temp), "GET_INFO: ");
                 break;
            case VGET_DEF:
                 sprintf(temp+strlen(temp), "GET_DEF: ");
                 break;
            case VGET_CUR_ALL:
                 sprintf(temp+strlen(temp), "GET_CUR_ALL: ");
                 break;
            case VGET_MIN_ALL:
                 sprintf(temp+strlen(temp), "GET_MIN_ALL: ");
                 break;
            case VGET_MAX_ALL:
                 sprintf(temp+strlen(temp), "GET_MAX_ALL: ");
                 break;
            case VGET_RES_ALL:
                 sprintf(temp+strlen(temp), "GET_RES_ALL: ");
                 break;
            case VGET_DEF_ALL:
                 sprintf(temp+strlen(temp), "GET_DEF_ALL: ");
                 break;
            default:
                 sprintf(temp+strlen(temp), "UNKNOWN: ");
                 break;
        }

        for (it=0; it<size; it++)
        {
            sprintf(temp+strlen(temp), "%02X ", buffer[it]);
        }
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "            USB ctrl recv, %s", temp);
    }

    /* copy answer back */
    for (it=0; it<size; it++)
    {
        data[it]=buffer[it];
    }

    usbd_free_urb(urb);
    usbd_free(buffer);

    return 0;
}

int uvc_control_set(uvc_device_t* dev, int operation, int unit, int selector, int size, uint8_t* data)
{
    struct usbd_urb* urb;
    uint8_t* buffer;
    int status;
    uint16_t unit_id=0;
    int it;

    switch (unit)
    {
        case UVC_PUNIT_SELECTOR:
             unit_id=dev->vc_processing_unit.bUnitID;
             break;
        case UVC_CAMERA_SELECTOR:
             unit_id=dev->vc_iterminal.bTerminalID;
             break;
        case UVC_ISEL_SELECTOR:
             unit_id=dev->vc_selector_unit.bUnitID;
             break;
        default:
             slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Unknown destination unit");
             return -1;
    }

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

    /* Copy data to URB buffer */
    memcpy(buffer, data, size);

    if (uvc_verbose>3)
    {
        char temp[128]={0};

        switch (operation)
        {
            case VSET_CUR:
                 sprintf(temp+strlen(temp), "SET_CUR: ");
                 break;
            default:
                 sprintf(temp+strlen(temp), "UNKNOWN: ");
                 break;
        }

        for (it=0; it<size; it++)
        {
            sprintf(temp+strlen(temp), "%02X ", buffer[it]);
        }
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "            USB ctrl send, %s", temp);
    }

    usbd_setup_vendor(urb, URB_DIR_OUT, operation, UVC_REQUEST_SET_VC, selector<<8, unit_id<<8, buffer, size);
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

/* Always accepts usb endian (little endian) */
uint64_t uvc_get_uinteger(int size, unsigned char* data)
{
    uint64_t temp=0;
    uint64_t temp_shift=0;
    int it;

    if (size==0)
    {
        return temp;
    }

    temp=data[0];
    temp_shift=8;

    for (it=1; it<size; it++)
    {
        temp|=(uint64_t)data[it]<<temp_shift;
        temp_shift+=8;
    }

    return temp;
}

/* Always accepts usb endian (little endian) */
int64_t uvc_get_sinteger(int size, unsigned char* data)
{
    int64_t  temp=0;
    uint64_t temp_shift=0;
    int sign_index=0;
    int it;

    if (size==0)
    {
        return temp;
    }

    temp=data[0];
    temp_shift=8;

    for (it=1; it<size; it++)
    {
        temp|=(int64_t)data[it]<<temp_shift;
        temp_shift+=8;
    }

    sign_index=it-1;
    for (it=size; it<8; it++)
    {
        if (data[sign_index]&0x80)
        {
            temp|=(int64_t)0xFF<<temp_shift;
            temp_shift+=8;
        }
    }

    return temp;
}

void uvc_set_uinteger(int size, unsigned char* data, uint64_t value)
{
    uint64_t temp_shift=0;
    int it;

    data[0]=value & 0x00000000000000FFULL;
    temp_shift=8;

    for (it=1; it<size; it++)
    {
        data[it]=(value>>temp_shift) & 0x00000000000000FFULL;
        temp_shift+=8;
    }
}

void uvc_set_sinteger(int size, unsigned char* data, int64_t value)
{
    uint64_t temp_shift=0;
    int it;

    data[0]=value & 0x00000000000000FFULL;
    temp_shift=8;

    for (it=1; it<size; it++)
    {
        data[it]=(value>>temp_shift) & 0x00000000000000FFULL;
        temp_shift+=8;
    }
}
