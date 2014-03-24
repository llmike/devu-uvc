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
#include <stdlib.h>
#include <sys/slog.h>
#include <sys/usbdi.h>
#include <sys/slogcodes.h>

#include "uvc.h"
#include "usbvc.h"
#include "uvc_control.h"
#include "uvc_interrupt.h"

extern int uvc_verbose;

int uvc_selector_common_handler(uvc_device_t* dev, int selector, uint8_t* data, int datasize, int offset, struct _uvc_event_entry* entry)
{
    uint8_t value[16];
    int status=0;

    entry->event.u.ctrl.changes=0;
    entry->event.u.ctrl.value=0;
    entry->event.u.ctrl.minimum=0;
    entry->event.u.ctrl.maximum=0;

    switch(entry->event.u.ctrl.type)
    {
        case V4L2_CTRL_TYPE_INTEGER:
             switch (data[4])
             {
                 case UVC_INTERRUPT_VC_CONTROL_VALUE_CHANGE:
                      entry->event.u.ctrl.changes=V4L2_EVENT_CTRL_CH_VALUE;
                      switch(entry->event.id)
                      {
                          case V4L2_CID_FOCUS_RELATIVE:
                               entry->event.u.ctrl.value=uvc_get_uinteger(1, &data[6]);
                               if (data[5]==0xFF)
                               {
                                   entry->event.u.ctrl.value=-entry->event.u.ctrl.value;
                               }
                               break;
                          case V4L2_CID_ZOOM_RELATIVE:
                               entry->event.u.ctrl.value=uvc_get_uinteger(1, &data[7]);
                               if (data[5]==0xFF)
                               {
                                   entry->event.u.ctrl.value=-entry->event.u.ctrl.value;
                               }
                               break;
                          case V4L2_CID_PAN_ABSOLUTE:
                               entry->event.u.ctrl.value=uvc_get_uinteger(4, &data[5]);
                               break;
                          case V4L2_CID_TILT_ABSOLUTE:
                               entry->event.u.ctrl.value=uvc_get_uinteger(4, &data[9]);
                               break;
                          default:
                               entry->event.u.ctrl.value=uvc_get_sinteger(datasize, &data[5]);
                               break;
                      }
                      break;
                 case UVC_INTERRUPT_VC_CONTROL_MIN_CHANGE:
                      entry->event.u.ctrl.changes=V4L2_EVENT_CTRL_CH_RANGE;
                      switch(entry->event.id)
                      {
                          case V4L2_CID_FOCUS_RELATIVE:
                               /* Ignore minimum, because we use +/- maximum */
                               return 0;
                          case V4L2_CID_ZOOM_RELATIVE:
                               /* Ignore minimum, because we use +/- maximum */
                               return 0;
                          case V4L2_CID_PAN_ABSOLUTE:
                               entry->event.u.ctrl.minimum=uvc_get_sinteger(4, &data[5]);
                               status|=uvc_control_get(dev, VGET_MAX, selector, data[3], datasize, &value[0]);
                               entry->event.u.ctrl.maximum=uvc_get_sinteger(4, &value[0]);
                               break;
                          case V4L2_CID_TILT_ABSOLUTE:
                               entry->event.u.ctrl.minimum=uvc_get_sinteger(4, &data[9]);
                               status|=uvc_control_get(dev, VGET_MAX, selector, data[3], datasize, &value[0]);
                               entry->event.u.ctrl.maximum=uvc_get_sinteger(4, &value[4]);
                               break;
                          default:
                               entry->event.u.ctrl.minimum=uvc_get_sinteger(datasize, &data[5]);
                               status|=uvc_control_get(dev, VGET_MAX, selector, data[3], datasize, &value[0]);
                               entry->event.u.ctrl.maximum=uvc_get_sinteger(datasize, &value[0]);
                               break;
                      }
                      break;
                 case UVC_INTERRUPT_VC_CONTROL_MAX_CHANGE:
                      entry->event.u.ctrl.changes=V4L2_EVENT_CTRL_CH_RANGE;
                      switch(entry->event.id)
                      {
                          case V4L2_CID_FOCUS_RELATIVE:
                               entry->event.u.ctrl.maximum=uvc_get_uinteger(1, &data[6]);
                               entry->event.u.ctrl.minimum=-entry->event.u.ctrl.maximum;
                               break;
                          case V4L2_CID_ZOOM_RELATIVE:
                               entry->event.u.ctrl.maximum=uvc_get_uinteger(1, &data[7]);
                               entry->event.u.ctrl.minimum=-entry->event.u.ctrl.maximum;
                               break;
                          case V4L2_CID_PAN_ABSOLUTE:
                               entry->event.u.ctrl.maximum=uvc_get_sinteger(4, &data[5]);
                               status|=uvc_control_get(dev, VGET_MAX, selector, data[3], datasize, &value[0]);
                               entry->event.u.ctrl.minimum=uvc_get_sinteger(4, &value[0]);
                               break;
                          case V4L2_CID_TILT_ABSOLUTE:
                               entry->event.u.ctrl.maximum=uvc_get_sinteger(4, &data[9]);
                               status|=uvc_control_get(dev, VGET_MAX, selector, data[3], datasize, &value[0]);
                               entry->event.u.ctrl.minimum=uvc_get_sinteger(4, &value[4]);
                               break;
                          default:
                               entry->event.u.ctrl.maximum=uvc_get_sinteger(datasize, &data[5]);
                               status|=uvc_control_get(dev, VGET_MIN, selector, data[3], datasize, &value[0]);
                               entry->event.u.ctrl.minimum=uvc_get_sinteger(datasize, &value[0]);
                               break;
                      }
                      break;
             }
             break;
        case V4L2_CTRL_TYPE_MENU:
             switch (data[4])
             {
                 case UVC_INTERRUPT_VC_CONTROL_VALUE_CHANGE:
                      entry->event.u.ctrl.changes=V4L2_EVENT_CTRL_CH_VALUE;
                      entry->event.u.ctrl.value=uvc_get_sinteger(datasize, &data[5]);
                      break;
             }
             break;
        case V4L2_CTRL_TYPE_BOOLEAN:
             switch (data[4])
             {
                 case UVC_INTERRUPT_VC_CONTROL_VALUE_CHANGE:
                      entry->event.u.ctrl.changes=V4L2_EVENT_CTRL_CH_VALUE;
                      entry->event.u.ctrl.value=uvc_get_sinteger(datasize, &data[5]);
                      break;
             }
             break;
    }

    return 1;
}

int uvc_selector_to_cid(uvc_device_t* dev, uint8_t* data, struct _uvc_event_entry* entry)
{
    /* Check if it was a processing unit as originator */
    if (data[1]==dev->vc_processing_unit.bUnitID)
    {
        switch (data[3])
        {
            case VPU_CONTROL_UNDEFINED:
                 slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Undefined PU control at unit %d, please report", data[1]);
                 break;
            case VPU_BACKLIGHT_COMPENSATION_CONTROL:
                 if (entry)
                 {
                     entry->event.id=V4L2_CID_BACKLIGHT_COMPENSATION;
                     entry->event.u.ctrl.type=V4L2_CTRL_TYPE_INTEGER;
                     entry->event.u.ctrl.flags=0;
                     uvc_selector_common_handler(dev, UVC_PUNIT_SELECTOR, data, 2, 0, entry);
                 }
                 return 1;
            case VPU_BRIGHTNESS_CONTROL:
                 if (entry)
                 {
                     entry->event.id=V4L2_CID_BRIGHTNESS;
                     entry->event.u.ctrl.type=V4L2_CTRL_TYPE_INTEGER;
                     entry->event.u.ctrl.flags=0;
                     uvc_selector_common_handler(dev, UVC_PUNIT_SELECTOR, data, 2, 0, entry);
                 }
                 return 1;
            case VPU_CONTRAST_CONTROL:
                 if (entry)
                 {
                     entry->event.id=V4L2_CID_CONTRAST;
                     entry->event.u.ctrl.type=V4L2_CTRL_TYPE_INTEGER;
                     entry->event.u.ctrl.flags=0;
                     uvc_selector_common_handler(dev, UVC_PUNIT_SELECTOR, data, 2, 0, entry);
                 }
                 return 1;
            case VPU_GAIN_CONTROL:
                 if (entry)
                 {
                     entry->event.id=V4L2_CID_GAIN;
                     entry->event.u.ctrl.type=V4L2_CTRL_TYPE_INTEGER;
                     entry->event.u.ctrl.flags=0;
                     uvc_selector_common_handler(dev, UVC_PUNIT_SELECTOR, data, 2, 0, entry);
                 }
                 return 1;
            case VPU_POWER_LINE_FREQUENCY_CONTROL:
                 if (entry)
                 {
                     entry->event.id=V4L2_CID_POWER_LINE_FREQUENCY;
                     entry->event.u.ctrl.type=V4L2_CTRL_TYPE_MENU;
                     entry->event.u.ctrl.flags=0;
                     uvc_selector_common_handler(dev, UVC_PUNIT_SELECTOR, data, 1, 0, entry);
                 }
                 return 1;
            case VPU_HUE_CONTROL:
                 if (entry)
                 {
                     entry->event.id=V4L2_CID_HUE;
                     entry->event.u.ctrl.type=V4L2_CTRL_TYPE_INTEGER;
                     entry->event.u.ctrl.flags=0;
                     uvc_selector_common_handler(dev, UVC_PUNIT_SELECTOR, data, 2, 0, entry);
                 }
                 return 1;
            case VPU_SATURATION_CONTROL:
                 if (entry)
                 {
                     entry->event.id=V4L2_CID_HUE;
                     entry->event.u.ctrl.type=V4L2_CTRL_TYPE_INTEGER;
                     entry->event.u.ctrl.flags=0;
                     uvc_selector_common_handler(dev, UVC_PUNIT_SELECTOR, data, 2, 0, entry);
                 }
                 return 1;
            case VPU_SHARPNESS_CONTROL:
                 if (entry)
                 {
                     entry->event.id=V4L2_CID_SHARPNESS;
                     entry->event.u.ctrl.type=V4L2_CTRL_TYPE_INTEGER;
                     entry->event.u.ctrl.flags=0;
                     uvc_selector_common_handler(dev, UVC_PUNIT_SELECTOR, data, 2, 0, entry);
                 }
                 return 1;
            case VPU_GAMMA_CONTROL:
                 if (entry)
                 {
                     entry->event.id=V4L2_CID_GAMMA;
                     entry->event.u.ctrl.type=V4L2_CTRL_TYPE_INTEGER;
                     entry->event.u.ctrl.flags=0;
                     uvc_selector_common_handler(dev, UVC_PUNIT_SELECTOR, data, 2, 0, entry);
                 }
                 return 1;
            case VPU_WHITE_BALANCE_TEMPERATURE_CONTROL:
                 if (entry)
                 {
                     entry->event.id=V4L2_CID_WHITE_BALANCE_TEMPERATURE;
                     entry->event.u.ctrl.type=V4L2_CTRL_TYPE_INTEGER;
                     entry->event.u.ctrl.flags=0;
                     uvc_selector_common_handler(dev, UVC_PUNIT_SELECTOR, data, 2, 0, entry);
                 }
                 return 1;
            case VPU_WHITE_BALANCE_COMPONENT_CONTROL:
                 if (entry)
                 {
                     entry->event.id=V4L2_CID_BLUE_BALANCE;
                     entry->event.u.ctrl.type=V4L2_CTRL_TYPE_INTEGER;
                     entry->event.u.ctrl.flags=0;
                     uvc_selector_common_handler(dev, UVC_PUNIT_SELECTOR, data, 4, 0, entry);
                 }
                 return 1;
                 /* According to UVC spec they are mutually exclusive, so */
                 /* only one control can exist at the same time.          */
            case VPU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL:
            case VPU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL:
                 if (entry)
                 {
                     entry->event.id=V4L2_CID_AUTO_WHITE_BALANCE;
                     entry->event.u.ctrl.type=V4L2_CTRL_TYPE_BOOLEAN;
                     entry->event.u.ctrl.flags=0;
                     uvc_selector_common_handler(dev, UVC_PUNIT_SELECTOR, data, 1, 0, entry);
                 }
                 return 1;
            case VPU_HUE_AUTO_CONTROL:
                 if (entry)
                 {
                     entry->event.id=V4L2_CID_HUE_AUTO;
                     entry->event.u.ctrl.type=V4L2_CTRL_TYPE_BOOLEAN;
                     entry->event.u.ctrl.flags=0;
                     uvc_selector_common_handler(dev, UVC_PUNIT_SELECTOR, data, 1, 0, entry);
                 }
                 return 1;
                 /* Unhandled cases due to limitation of V4L2 */
            case VPU_ANALOG_VIDEO_STANDARD_CONTROL:
            case VPU_ANALOG_LOCK_STATUS_CONTROL:
            case VPU_DIGITAL_MULTIPLIER_CONTROL:
            case VPU_DIGITAL_MULTIPLIER_LIMIT_CONTROL:
            case VPU_CONTRAST_AUTO_CONTROL:
                 slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Unhandled PU control %d at unit %d, please report", data[3], data[1]);
                 return 0;
        }
    }
    else
    {
        /* Check if it was an input terminal as originator */
        if (data[1]==dev->vc_iterminal.bTerminalID)
        {
            switch (data[3])
            {
                case VCT_CONTROL_UNDEFINED:
                     slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Undefined IT control at unit %d, please report", data[1]);
                     break;
                case VCT_AE_MODE_CONTROL:
                     if (entry)
                     {
                         entry->event.id=V4L2_CID_EXPOSURE_AUTO;
                         entry->event.u.ctrl.type=V4L2_CTRL_TYPE_MENU;
                         entry->event.u.ctrl.flags=0;
                         uvc_selector_common_handler(dev, UVC_CAMERA_SELECTOR, data, 1, 0, entry);
                         switch(entry->event.u.ctrl.value)
                         {
                             case 8:
                                  entry->event.u.ctrl.value=3;
                                  break;
                             case 4:
                                  entry->event.u.ctrl.value=2;
                                  break;
                             case 2:
                                  entry->event.u.ctrl.value=1;
                                  break;
                             case 1:
                                  entry->event.u.ctrl.value=0;
                                  break;
                         }
                     }
                     return 1;
                case VCT_AE_PRIORITY_CONTROL:
                     if (entry)
                     {
                         entry->event.id=V4L2_CID_EXPOSURE_AUTO_PRIORITY;
                         entry->event.u.ctrl.type=V4L2_CTRL_TYPE_BOOLEAN;
                         entry->event.u.ctrl.flags=0;
                         uvc_selector_common_handler(dev, UVC_CAMERA_SELECTOR, data, 1, 0, entry);
                     }
                     return 1;
                case VCT_EXPOSURE_TIME_ABSOLUTE_CONTROL:
                     if (entry)
                     {
                         entry->event.id=V4L2_CID_EXPOSURE_ABSOLUTE;
                         entry->event.u.ctrl.type=V4L2_CTRL_TYPE_INTEGER;
                         entry->event.u.ctrl.flags=0;
                         uvc_selector_common_handler(dev, UVC_CAMERA_SELECTOR, data, 4, 0, entry);
                     }
                     return 1;
                case VCT_FOCUS_ABSOLUTE_CONTROL:
                     if (entry)
                     {
                         entry->event.id=V4L2_CID_FOCUS_ABSOLUTE;
                         entry->event.u.ctrl.type=V4L2_CTRL_TYPE_INTEGER;
                         entry->event.u.ctrl.flags=0;
                         uvc_selector_common_handler(dev, UVC_CAMERA_SELECTOR, data, 2, 0, entry);
                     }
                     return 1;
                case VCT_FOCUS_RELATIVE_CONTROL:
                     if (entry)
                     {
                         entry->event.id=V4L2_CID_FOCUS_RELATIVE;
                         entry->event.u.ctrl.type=V4L2_CTRL_TYPE_INTEGER;
                         entry->event.u.ctrl.flags=0;
                         uvc_selector_common_handler(dev, UVC_CAMERA_SELECTOR, data, 2, 0, entry);
                     }
                     return 1;
                case VCT_FOCUS_AUTO_CONTROL:
                     if (entry)
                     {
                         entry->event.id=V4L2_CID_FOCUS_AUTO;
                         entry->event.u.ctrl.type=V4L2_CTRL_TYPE_BOOLEAN;
                         entry->event.u.ctrl.flags=0;
                         uvc_selector_common_handler(dev, UVC_CAMERA_SELECTOR, data, 1, 0, entry);
                     }
                     return 1;
                case VCT_IRIS_ABSOLUTE_CONTROL:
                     if (entry)
                     {
                         entry->event.id=V4L2_CID_IRIS_ABSOLUTE;
                         entry->event.u.ctrl.type=V4L2_CTRL_TYPE_INTEGER;
                         entry->event.u.ctrl.flags=0;
                         uvc_selector_common_handler(dev, UVC_CAMERA_SELECTOR, data, 2, 0, entry);
                     }
                     return 1;
                case VCT_IRIS_RELATIVE_CONTROL:
                     if (entry)
                     {
                         entry->event.id=V4L2_CID_IRIS_RELATIVE;
                         entry->event.u.ctrl.type=V4L2_CTRL_TYPE_INTEGER;
                         entry->event.u.ctrl.flags=0;
                         uvc_selector_common_handler(dev, UVC_CAMERA_SELECTOR, data, 1, 0, entry);
                     }
                     return 1;
                case VCT_ZOOM_ABSOLUTE_CONTROL:
                     if (entry)
                     {
                         entry->event.id=V4L2_CID_ZOOM_ABSOLUTE;
                         entry->event.u.ctrl.type=V4L2_CTRL_TYPE_INTEGER;
                         entry->event.u.ctrl.flags=0;
                         uvc_selector_common_handler(dev, UVC_CAMERA_SELECTOR, data, 2, 0, entry);
                     }
                     return 1;
                case VCT_ZOOM_RELATIVE_CONTROL:
                     if (entry)
                     {
                         entry->event.id=V4L2_CID_ZOOM_RELATIVE;
                         entry->event.u.ctrl.type=V4L2_CTRL_TYPE_INTEGER;
                         entry->event.u.ctrl.flags=0;
                         uvc_selector_common_handler(dev, UVC_CAMERA_SELECTOR, data, 3, 0, entry);
                     }
                     return 1;
                case VCT_PANTILT_ABSOLUTE_CONTROL:
                     if (entry)
                     {
                         entry->event.id=V4L2_CID_PAN_ABSOLUTE;
                         entry->event.u.ctrl.type=V4L2_CTRL_TYPE_INTEGER;
                         entry->event.u.ctrl.flags=0;
                         uvc_selector_common_handler(dev, UVC_CAMERA_SELECTOR, data, 8, 0, entry);
                     }
                     return 1;
                case VCT_PRIVACY_CONTROL:
                     if (entry)
                     {
                         entry->event.id=V4L2_CID_PRIVACY;
                         entry->event.u.ctrl.type=V4L2_CTRL_TYPE_INTEGER;
                         entry->event.u.ctrl.flags=0;
                         uvc_selector_common_handler(dev, UVC_CAMERA_SELECTOR, data, 1, 0, entry);
                     }
                     return 1;
                     /* Unhandled cases due to limitation of V4L2 */
                case VCT_PANTILT_RELATIVE_CONTROL:
                case VCT_ROLL_ABSOLUTE_CONTROL:
                case VCT_ROLL_RELATIVE_CONTROL:
                case VCT_EXPOSURE_TIME_RELATIVE_CONTROL:
                case VCT_FOCUS_SIMPLE_CONTROL:
                case VCT_SCANNING_MODE_CONTROL:
                case VCT_WINDOW_CONTROL:
                case VCT_REGION_OF_INTEREST_CONTROL:
                     slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Unhandled IT control %d at unit %d, please report", data[3], data[1]);
                     return 0;
            }
        }
        else
        {
            slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Unknown interrupt originator unit %d, please report", data[1]);
        }
    }

    return 0;
}

void uvc_interrupt_completion(struct usbd_urb* urb, struct usbd_pipe* pipe, void* handle)
{
    uvc_device_t* dev=(uvc_device_t*)handle;
    uint8_t* data=(uint8_t*)dev->interrupt_buffer;
    int status;
    int it;

    if (uvc_verbose>3)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "  INTERRUPT:");
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bStatusType: %02X", data[0]);
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bOriginator: %02X", data[1]);
        switch (data[0] & UVC_INTERRUPT_ORIGINATOR_MASK)
        {
            case UVC_INTERRUPT_ORIGINATOR_VC:
                 slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bEvent: %02X", data[2]);
                 slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bSelector: %02X", data[3]);
                 slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bAttribute: %02X", data[4]);
                 slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bValue: %02X", data[5]);
                 break;
            case UVC_INTERRUPT_ORIGINATOR_VS:
                 slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bEvent: %02X", data[2]);
                 slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    bValue: %02X", data[3]);
                 break;
            default:
                 slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Unknown interrupt originator, please report");
                 break;
        }
    }

    switch (data[0] & UVC_INTERRUPT_ORIGINATOR_MASK)
    {
        case UVC_INTERRUPT_ORIGINATOR_VC:
             if (data[2]==UVC_INTERRUPT_VC_EVENT_CONTROL_CHANGE)
             {
                 struct _uvc_event_entry* entry=NULL;
                 struct _uvc_event_entry tentry;

                 if (uvc_selector_to_cid(dev, data, &tentry))
                 {
                     /* Emit control change event */
                     for(it=0; it<UVC_MAX_OPEN_FDS; it++)
                     {
                         if (dev->event[it].ocb!=NULL)
                         {
                             /* Check file descriptor listening for ctrl event */
                             if (dev->event[it].type[V4L2_EVENT_CTRL])
                             {
                                 /* Check if file descriptor listening for required control id */
                                 if ((dev->event[it].type[V4L2_EVENT_CTRL]==0) ||
                                     (dev->event[it].type[V4L2_EVENT_CTRL]==tentry.event.id))
                                 {
                                     switch (tentry.event.id)
                                     {
                                         case V4L2_CID_BLUE_BALANCE:
                                              /* Expand combined balance to two components: blue and red */
                                              entry=calloc(1, sizeof(*entry));
                                              if (entry==NULL)
                                              {
                                                  break;
                                              }

                                              *entry=tentry;

                                              /* Leave BLUE component only */
                                              switch(entry->event.u.ctrl.changes)
                                              {
                                                  case V4L2_EVENT_CTRL_CH_VALUE:
                                                       entry->event.u.ctrl.value=
                                                           (entry->event.u.ctrl.value & 0x00008000) ?
                                                           -(entry->event.u.ctrl.value & 0x0000FFFF) :
                                                             entry->event.u.ctrl.value & 0x0000FFFF;
                                                       break;
                                                  case V4L2_EVENT_CTRL_CH_RANGE:
                                                       entry->event.u.ctrl.minimum=
                                                           (entry->event.u.ctrl.minimum & 0x00008000) ?
                                                           -(entry->event.u.ctrl.minimum & 0x0000FFFF) :
                                                             entry->event.u.ctrl.minimum & 0x0000FFFF;
                                                       entry->event.u.ctrl.maximum=
                                                           (entry->event.u.ctrl.maximum & 0x00008000) ?
                                                           -(entry->event.u.ctrl.maximum & 0x0000FFFF) :
                                                             entry->event.u.ctrl.maximum & 0x0000FFFF;
                                                       break;
                                              }

                                              entry->event.type=V4L2_EVENT_CTRL;
                                              entry->event.pending=0;
                                              entry->event.sequence=dev->event[it].sequence;
                                              clock_gettime(CLOCK_MONOTONIC, &entry->event.timestamp);
                                              memset(entry->event.reserved, 0x00, sizeof(entry->event.reserved));
                                              pthread_mutex_lock(&dev->event[it].access);
                                              dev->event[it].sequence++;
                                              TAILQ_INSERT_TAIL(&dev->event[it].head, entry, link);
                                              pthread_mutex_unlock(&dev->event[it].access);

                                              /* Notify callers of select() if any */
                                              iofunc_notify_trigger(dev->event[it].ocb->notify, 1, IOFUNC_NOTIFY_OBAND);

                                              /* Then process red component */
                                              entry=calloc(1, sizeof(*entry));
                                              if (entry==NULL)
                                              {
                                                  break;
                                              }

                                              tentry.event.id=V4L2_CID_RED_BALANCE;
                                              *entry=tentry;

                                              /* Leave RED component only */
                                              switch(entry->event.u.ctrl.changes)
                                              {
                                                  case V4L2_EVENT_CTRL_CH_VALUE:
                                                       entry->event.u.ctrl.value>>=16;
                                                       break;
                                                  case V4L2_EVENT_CTRL_CH_RANGE:
                                                       entry->event.u.ctrl.minimum>>=16;
                                                       entry->event.u.ctrl.maximum>>=16;
                                                       break;
                                              }

                                              entry->event.type=V4L2_EVENT_CTRL;
                                              entry->event.pending=0;
                                              entry->event.sequence=dev->event[it].sequence;
                                              clock_gettime(CLOCK_MONOTONIC, &entry->event.timestamp);
                                              memset(entry->event.reserved, 0x00, sizeof(entry->event.reserved));
                                              pthread_mutex_lock(&dev->event[it].access);
                                              dev->event[it].sequence++;
                                              TAILQ_INSERT_TAIL(&dev->event[it].head, entry, link);
                                              pthread_mutex_unlock(&dev->event[it].access);

                                              /* Notify callers of select() if any */
                                              iofunc_notify_trigger(dev->event[it].ocb->notify, 1, IOFUNC_NOTIFY_OBAND);
                                              break;
                                         case V4L2_CID_PAN_ABSOLUTE:
                                              /* Expand combined pantilt to two components: pan and tilt */
                                              entry=calloc(1, sizeof(*entry));
                                              if (entry==NULL)
                                              {
                                                  break;
                                              }

                                              *entry=tentry;

                                              /* Leave PAN component only */
                                              entry->event.type=V4L2_EVENT_CTRL;
                                              entry->event.pending=0;
                                              entry->event.sequence=dev->event[it].sequence;
                                              clock_gettime(CLOCK_MONOTONIC, &entry->event.timestamp);
                                              memset(entry->event.reserved, 0x00, sizeof(entry->event.reserved));
                                              pthread_mutex_lock(&dev->event[it].access);
                                              dev->event[it].sequence++;
                                              TAILQ_INSERT_TAIL(&dev->event[it].head, entry, link);
                                              pthread_mutex_unlock(&dev->event[it].access);

                                              /* Notify callers of select() if any */
                                              iofunc_notify_trigger(dev->event[it].ocb->notify, 1, IOFUNC_NOTIFY_OBAND);

                                              tentry.event.id=V4L2_CID_TILT_ABSOLUTE;
                                              tentry.event.u.ctrl.type=V4L2_CTRL_TYPE_INTEGER;
                                              tentry.event.u.ctrl.flags=0;
                                              uvc_selector_common_handler(dev, UVC_CAMERA_SELECTOR, data, 8, 0, &tentry);

                                              *entry=tentry;

                                              /* Leave TILT component only */
                                              entry->event.type=V4L2_EVENT_CTRL;
                                              entry->event.pending=0;
                                              entry->event.sequence=dev->event[it].sequence;
                                              clock_gettime(CLOCK_MONOTONIC, &entry->event.timestamp);
                                              memset(entry->event.reserved, 0x00, sizeof(entry->event.reserved));
                                              pthread_mutex_lock(&dev->event[it].access);
                                              dev->event[it].sequence++;
                                              TAILQ_INSERT_TAIL(&dev->event[it].head, entry, link);
                                              pthread_mutex_unlock(&dev->event[it].access);

                                              /* Notify callers of select() if any */
                                              iofunc_notify_trigger(dev->event[it].ocb->notify, 1, IOFUNC_NOTIFY_OBAND);

                                              break;
                                         default:
                                              entry=calloc(1, sizeof(*entry));
                                              if (entry==NULL)
                                              {
                                                  break;
                                              }

                                              *entry=tentry;
                                              entry->event.type=V4L2_EVENT_CTRL;
                                              entry->event.pending=0;
                                              entry->event.sequence=dev->event[it].sequence;
                                              clock_gettime(CLOCK_MONOTONIC, &entry->event.timestamp);
                                              memset(entry->event.reserved, 0x00, sizeof(entry->event.reserved));
                                              pthread_mutex_lock(&dev->event[it].access);
                                              dev->event[it].sequence++;
                                              TAILQ_INSERT_TAIL(&dev->event[it].head, entry, link);
                                              pthread_mutex_unlock(&dev->event[it].access);

                                              /* Notify callers of select() if any */
                                              iofunc_notify_trigger(dev->event[it].ocb->notify, 1, IOFUNC_NOTIFY_OBAND);
                                              break;
                                     }
                                 }
                             }
                         }
                     }
                 }
             }
             break;
        case UVC_INTERRUPT_ORIGINATOR_VS:
             if (data[2]==UVC_INTERRUPT_VS_EVENT_BUTTON_PRESS)
             {
                 if (data[3]==UVC_INTERRUPT_VS_BUTTON_PRESS)
                 {
                     /* UVC standard does not recognize different VS sources as originator */
                     dev->event_button=1;
                 }
                 else
                 {
                     dev->event_button=0;
                 }

                 /* enqueue control event on button state change */
                 for(it=0; it<UVC_MAX_OPEN_FDS; it++)
                 {
                     if (dev->event[it].ocb!=NULL)
                     {
                         /* Check file descriptor listening for ctrl event */
                         if (dev->event[it].type[V4L2_EVENT_CTRL])
                         {
                             /* Check if file descriptor listening for required control id */
                             if ((dev->event[it].type[V4L2_EVENT_CTRL]==0) ||
                                 (dev->event[it].type[V4L2_EVENT_CTRL]==UVC_CID_EVENT_BUTTON))
                             {
                                 struct _uvc_event_entry* entry;

                                 entry=calloc(1, sizeof(*entry));
                                 if (entry==NULL)
                                 {
                                     break;
                                 }

                                 /* Emit control change event */
                                 entry->event.type=V4L2_EVENT_CTRL;
                                 entry->event.pending=0;
                                 entry->event.sequence=dev->event[it].sequence;
                                 entry->event.id=UVC_CID_EVENT_BUTTON;
                                 clock_gettime(CLOCK_MONOTONIC, &entry->event.timestamp);
                                 memset(entry->event.reserved, 0x00, sizeof(entry->event.reserved));
                                 entry->event.u.ctrl.changes=V4L2_EVENT_CTRL_CH_VALUE;
                                 entry->event.u.ctrl.type=V4L2_CTRL_TYPE_BOOLEAN;
                                 entry->event.u.ctrl.flags=V4L2_CTRL_FLAG_READ_ONLY | V4L2_CTRL_FLAG_VOLATILE;
                                 entry->event.u.ctrl.value=dev->event_button;
                                 pthread_mutex_lock(&dev->event[it].access);
                                 dev->event[it].sequence++;
                                 TAILQ_INSERT_TAIL(&dev->event[it].head, entry, link);
                                 pthread_mutex_unlock(&dev->event[it].access);

                                 /* Notify callers of select() if any */
                                 iofunc_notify_trigger(dev->event[it].ocb->notify, 1, IOFUNC_NOTIFY_OBAND);
                             }
                         }
                     }
                 }
             }
             break;
        default:
             slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Unknown interrupt originator, please report");
             break;
    }

    /* Request for new interrupts */
    usbd_setup_interrupt(dev->interrupt_urb, URB_DIR_IN, dev->interrupt_buffer, UVC_MAX_INTERRUPT_PAYLOAD_SIZE);
    status=usbd_io(dev->interrupt_urb, dev->vc_interrupt_pipe, uvc_interrupt_completion, dev, USBD_TIME_INFINITY);
    if (status!=EOK)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't perform usb i/o operation (interrupt)");
        return;
    }
}

int uvc_setup_interrupt(uvc_device_t* dev)
{
    int status;

    if (dev->vc_interrupt_pipe==NULL)
    {
        /* Interrupts will never be used till device is present in the system */
        return -1;
    }

    dev->interrupt_urb=usbd_alloc_urb(NULL);
    if (dev->interrupt_urb==NULL)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't allocate interrupt urb");
        return -1;
    }

    dev->interrupt_buffer=usbd_alloc(UVC_MAX_INTERRUPT_PAYLOAD_SIZE);
    if (dev->interrupt_buffer==NULL)
    {
        usbd_free_urb(dev->interrupt_urb);
        dev->interrupt_urb=NULL;
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't allocate buffer for interrupt");
        return -1;
    }

    usbd_setup_interrupt(dev->interrupt_urb, URB_DIR_IN, dev->interrupt_buffer, UVC_MAX_INTERRUPT_PAYLOAD_SIZE);
    status=usbd_io(dev->interrupt_urb, dev->vc_interrupt_pipe, uvc_interrupt_completion, dev, USBD_TIME_INFINITY);
    if (status!=EOK)
    {
        usbd_free_urb(dev->interrupt_urb);
        usbd_free(dev->interrupt_buffer);
        dev->interrupt_urb=NULL;
        dev->interrupt_buffer=NULL;
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't perform usb i/o operation");
        return -1;
    }

    return 0;
}

int uvc_unsetup_interrupt(uvc_device_t* dev)
{
    if (dev->vc_interrupt_pipe==NULL)
    {
        /* Interrupts will never be used till device is present in the system */
        return -1;
    }

    /* Cancel any pending operations */
    usbd_abort_pipe(dev->vc_interrupt_pipe);
    dev->vc_interrupt_pipe=NULL;

    if (dev->interrupt_urb!=NULL)
    {
        usbd_free_urb(dev->interrupt_urb);
        dev->interrupt_urb=NULL;
    }
    if (dev->interrupt_buffer!=NULL)
    {
        usbd_free(dev->interrupt_buffer);
        dev->interrupt_buffer=NULL;
    }

    return 0;
}
