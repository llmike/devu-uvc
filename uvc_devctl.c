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

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/mman.h>
#include <sys/usbdi.h>
#include <sys/types.h>
#include <sys/usb100.h>
#include <sys/procmgr.h>
#include <sys/dcmd_misc.h>
#include <sys/rsrcdbmgr.h>
#include <sys/slogcodes.h>

#include <linux/videodev2.h>
#include <linux/v4l2-controls.h>
#include <linux/uvcvideo.h>

#include "uvc.h"
#include "usbvc.h"
#include "uvc_driver.h"
#include "uvc_control.h"
#include "uvc_streaming.h"

extern int uvc_verbose;
extern int uvc_emulation;

typedef struct _control_data
{
    char* name;
    int   type;
    int   selector;
    int   unit;
    int   size;
} control_data_t;

int uvc_query_control_data(uvc_device_t* dev, uint32_t id, uint32_t subdev, control_data_t* data)
{
    switch(id & V4L2_CTRL_ID_MASK)
    {
        case V4L2_CTRL_CLASS_USER+1:
             {
                 if ((dev->vc_processing_unit.bmControls[0]==0) &&
                     (dev->vc_processing_unit.bmControls[1]==0) &&
                     (dev->vc_processing_unit.bmControls[2]==0))
                 {
                     /* If input processing unit capabilities are empty, then return */
                     /* absence of processing unit controls */
                     return 0;
                 }
                 if (data!=NULL)
                 {
                     data->name="Processing Unit Controls";
                     data->type=V4L2_CTRL_TYPE_CTRL_CLASS;
                     data->selector=0;
                     data->unit=0;
                     data->size=0;
                 }
                 return 1;
             }
             break;
        case V4L2_CTRL_CLASS_CAMERA+1:
             {
                 if ((dev->vc_iterminal.bmControls[0]==0) &&
                     (dev->vc_iterminal.bmControls[1]==0) &&
                     (dev->vc_iterminal.bmControls[2]==0))
                 {
                     /* If input terminal capabilities are empty, then return  */
                     /* absence of camera controls and check for private ctrls */
                     if (!((dev->vs_input_header[subdev].bTriggerSupport) &&
                         (dev->vs_input_header[subdev].bTriggerUsage)))
                     {
                         return 0;
                     }
                 }
                 if (data!=NULL)
                 {
                     data->name="Input Terminal Controls";
                     data->type=V4L2_CTRL_TYPE_CTRL_CLASS;
                     data->selector=0;
                     data->unit=0;
                     data->size=0;
                 }
                 return 1;
             }
             break;
        case V4L2_CID_BRIGHTNESS:
             if (dev->vc_processing_unit.bmControls[0] & VPU_B0_BRIGHTNESS)
             {
                 if (data!=NULL)
                 {
                     data->name="Brightness";
                     data->type=V4L2_CTRL_TYPE_INTEGER;
                     data->selector=VPU_BRIGHTNESS_CONTROL;
                     data->unit=UVC_PUNIT_SELECTOR;
                     data->size=2;
                 }
                 return 1;
             }
             break;
        case V4L2_CID_CONTRAST:
             if (dev->vc_processing_unit.bmControls[0] & VPU_B0_CONTRAST)
             {
                 if (data!=NULL)
                 {
                     data->name="Contrast";
                     data->type=V4L2_CTRL_TYPE_INTEGER;
                     data->selector=VPU_CONTRAST_CONTROL;
                     data->unit=UVC_PUNIT_SELECTOR;
                     data->size=2;
                 }
                 return 1;
             }
             break;
        case V4L2_CID_SATURATION:
             if (dev->vc_processing_unit.bmControls[0] & VPU_B0_SATURATION)
             {
                 if (data!=NULL)
                 {
                     data->name="Saturation";
                     data->type=V4L2_CTRL_TYPE_INTEGER;
                     data->selector=VPU_SATURATION_CONTROL;
                     data->unit=UVC_PUNIT_SELECTOR;
                     data->size=2;
                 }
                 return 1;
             }
             break;
        case V4L2_CID_HUE:
             if (dev->vc_processing_unit.bmControls[0] & VPU_B0_HUE)
             {
                 if (data!=NULL)
                 {
                     data->name="Hue";
                     data->type=V4L2_CTRL_TYPE_INTEGER;
                     data->selector=VPU_HUE_CONTROL;
                     data->unit=UVC_PUNIT_SELECTOR;
                     data->size=2;
                 }
                 return 1;
             }
             break;
        case V4L2_CID_SHARPNESS:
             if (dev->vc_processing_unit.bmControls[0] & VPU_B0_SHARPNESS)
             {
                 if (data!=NULL)
                 {
                     data->name="Sharpness";
                     data->type=V4L2_CTRL_TYPE_INTEGER;
                     data->selector=VPU_SHARPNESS_CONTROL;
                     data->unit=UVC_PUNIT_SELECTOR;
                     data->size=2;
                 }
                 return 1;
             }
             break;
        case V4L2_CID_GAMMA:
             if (dev->vc_processing_unit.bmControls[0] & VPU_B0_GAMMA)
             {
                 if (data!=NULL)
                 {
                     data->name="Gamma";
                     data->type=V4L2_CTRL_TYPE_INTEGER;
                     data->selector=VPU_GAMMA_CONTROL;
                     data->unit=UVC_PUNIT_SELECTOR;
                     data->size=2;
                 }
                 return 1;
             }
             break;
        case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
             if (dev->vc_processing_unit.bmControls[0] & VPU_B0_WHITE_BALANCE_TEMPERATURE)
             {
                 if (data!=NULL)
                 {
                     data->name="White Balance Temperature";
                     data->type=V4L2_CTRL_TYPE_INTEGER;
                     data->selector=VPU_WHITE_BALANCE_TEMPERATURE_CONTROL;
                     data->unit=UVC_PUNIT_SELECTOR;
                     data->size=2;
                 }
                 return 1;
             }
             break;
        case V4L2_CID_BLUE_BALANCE:
             if (dev->vc_processing_unit.bmControls[0] & VPU_B0_WHITE_BALANCE_COMPONENT)
             {
                 if (data!=NULL)
                 {
                     data->name="White Balance Blue Component";
                     data->type=V4L2_CTRL_TYPE_INTEGER;
                     data->selector=VPU_WHITE_BALANCE_COMPONENT_CONTROL;
                     data->unit=UVC_PUNIT_SELECTOR;
                     data->size=4;
                 }
                 return 1;
             }
             break;
        case V4L2_CID_RED_BALANCE:
             if (dev->vc_processing_unit.bmControls[0] & VPU_B0_WHITE_BALANCE_COMPONENT)
             {
                 if (data!=NULL)
                 {
                     data->name="White Balance Red Component";
                     data->type=V4L2_CTRL_TYPE_INTEGER;
                     data->selector=VPU_WHITE_BALANCE_COMPONENT_CONTROL;
                     data->unit=UVC_PUNIT_SELECTOR;
                     data->size=4;
                 }
                 return 1;
             }
             break;
        case V4L2_CID_BACKLIGHT_COMPENSATION:
             if (dev->vc_processing_unit.bmControls[1] & VPU_B1_BACKLIGHT_COMPENSATION)
             {
                 if (data!=NULL)
                 {
                     data->name="Backlight Compensation";
                     data->type=V4L2_CTRL_TYPE_INTEGER;
                     data->selector=VPU_BACKLIGHT_COMPENSATION_CONTROL;
                     data->unit=UVC_PUNIT_SELECTOR;
                     data->size=2;
                 }
                 return 1;
             }
             break;
        case V4L2_CID_GAIN:
             if (dev->vc_processing_unit.bmControls[1] & VPU_B1_GAIN)
             {
                 if (data!=NULL)
                 {
                     data->name="Gain";
                     data->type=V4L2_CTRL_TYPE_INTEGER;
                     data->selector=VPU_GAIN_CONTROL;
                     data->unit=UVC_PUNIT_SELECTOR;
                     data->size=2;
                 }
                 return 1;
             }
             break;
        case V4L2_CID_POWER_LINE_FREQUENCY:
             if (dev->vc_processing_unit.bmControls[1] & VPU_B1_POWER_LINE_FREQUENCY)
             {
                 if (data!=NULL)
                 {
                     data->name="Power Line Frequency";
                     data->type=V4L2_CTRL_TYPE_MENU;
                     data->selector=VPU_POWER_LINE_FREQUENCY_CONTROL;
                     data->unit=UVC_PUNIT_SELECTOR;
                     data->size=1;
                 }
                 return 1;
             }
             break;
        case V4L2_CID_HUE_AUTO:
             if (dev->vc_processing_unit.bmControls[1] & VPU_B1_HUE_AUTO)
             {
                 if (data!=NULL)
                 {
                     data->name="Hue, Auto";
                     data->type=V4L2_CTRL_TYPE_BOOLEAN;
                     data->selector=VPU_HUE_AUTO_CONTROL;
                     data->unit=UVC_PUNIT_SELECTOR;
                     data->size=1;
                 }
                 return 1;
             }
             break;
        case V4L2_CID_AUTO_WHITE_BALANCE:
             if (dev->vc_processing_unit.bmControls[1] & VPU_B1_WHITE_BALANCE_TEMPERATURE_AUTO)
             {
                 if (data!=NULL)
                 {
                     data->name="White Balance Temperature, Auto";
                     data->type=V4L2_CTRL_TYPE_BOOLEAN;
                     data->selector=VPU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL;
                     data->unit=UVC_PUNIT_SELECTOR;
                     data->size=1;
                 }
                 return 1;
             }
             if (dev->vc_processing_unit.bmControls[1] & VPU_B1_WHITE_BALANCE_COMPONENT_AUTO)
             {
                 if (data!=NULL)
                 {
                     data->name="White Balance Component, Auto";
                     data->type=V4L2_CTRL_TYPE_BOOLEAN;
                     data->selector=VPU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL;
                     data->unit=UVC_PUNIT_SELECTOR;
                     data->size=1;
                 }
                 return 1;
             }
             break;
        case V4L2_CID_EXPOSURE_AUTO:
             if (dev->vc_iterminal.bmControls[0] & VIT_B0_AUTO_EXPOSURE_MODE)
             {
                 if (data!=NULL)
                 {
                     data->name="Exposure, Auto";
                     data->type=V4L2_CTRL_TYPE_MENU;
                     data->selector=VCT_AE_MODE_CONTROL;
                     data->unit=UVC_CAMERA_SELECTOR;
                     data->size=1;
                 }
                 return 1;
             }
             break;
        case V4L2_CID_EXPOSURE_AUTO_PRIORITY:
             if (dev->vc_iterminal.bmControls[0] & VIT_B0_AUTO_EXPOSURE_PRIORITY)
             {
                 if (data!=NULL)
                 {
                     data->name="Exposure, Auto Priority";
                     data->type=V4L2_CTRL_TYPE_BOOLEAN;
                     data->selector=VCT_AE_PRIORITY_CONTROL;
                     data->unit=UVC_CAMERA_SELECTOR;
                     data->size=1;
                 }
                 return 1;
             }
             break;
        case V4L2_CID_EXPOSURE_ABSOLUTE:
             if (dev->vc_iterminal.bmControls[0] & VIT_B0_EXPOSURE_TIME_ABSOLUTE)
             {
                 if (data!=NULL)
                 {
                     data->name="Exposure, Absolute";
                     data->type=V4L2_CTRL_TYPE_INTEGER;
                     data->selector=VCT_EXPOSURE_TIME_ABSOLUTE_CONTROL;
                     data->unit=UVC_CAMERA_SELECTOR;
                     data->size=4;
                 }
                 return 1;
             }
             break;
        case V4L2_CID_FOCUS_ABSOLUTE:
             if (dev->vc_iterminal.bmControls[0] & VIT_B0_FOCUS_ABSOLUTE)
             {
                 if (data!=NULL)
                 {
                     data->name="Focus, Absolute";
                     data->type=V4L2_CTRL_TYPE_INTEGER;
                     data->selector=VCT_FOCUS_ABSOLUTE_CONTROL;
                     data->unit=UVC_CAMERA_SELECTOR;
                     data->size=2;
                 }
                 return 1;
             }
             break;
        case V4L2_CID_FOCUS_RELATIVE:
             if (dev->vc_iterminal.bmControls[0] & VIT_B0_FOCUS_RELATIVE)
             {
                 if (data!=NULL)
                 {
                     data->name="Focus, Relative";
                     data->type=V4L2_CTRL_TYPE_INTEGER;
                     data->selector=VCT_FOCUS_RELATIVE_CONTROL;
                     data->unit=UVC_CAMERA_SELECTOR;
                     data->size=2;
                 }
                 return 1;
             }
             break;
        case V4L2_CID_IRIS_ABSOLUTE:
             if (dev->vc_iterminal.bmControls[0] & VIT_B0_IRIS_ABSOLUTE)
             {
                 if (data!=NULL)
                 {
                     data->name="Iris, Absolute";
                     data->type=V4L2_CTRL_TYPE_INTEGER;
                     data->selector=VCT_IRIS_ABSOLUTE_CONTROL;
                     data->unit=UVC_CAMERA_SELECTOR;
                     data->size=2;
                 }
                 return 1;
             }
             break;
        case V4L2_CID_IRIS_RELATIVE:
             if (dev->vc_iterminal.bmControls[1] & VIT_B1_IRIS_RELATIVE)
             {
                 if (data!=NULL)
                 {
                     data->name="Iris, Relative";
                     data->type=V4L2_CTRL_TYPE_INTEGER;
                     data->selector=VCT_IRIS_RELATIVE_CONTROL;
                     data->unit=UVC_CAMERA_SELECTOR;
                     data->size=1;
                 }
                 return 1;
             }
             break;
        case V4L2_CID_ZOOM_ABSOLUTE:
             if (dev->vc_iterminal.bmControls[1] & VIT_B1_ZOOM_ABSOLUTE)
             {
                 if (data!=NULL)
                 {
                     data->name="Zoom, Absolute";
                     data->type=V4L2_CTRL_TYPE_INTEGER;
                     data->selector=VCT_ZOOM_ABSOLUTE_CONTROL;
                     data->unit=UVC_CAMERA_SELECTOR;
                     data->size=2;
                 }
                 return 1;
             }
             break;
        case V4L2_CID_ZOOM_RELATIVE:
             if (dev->vc_iterminal.bmControls[1] & VIT_B1_ZOOM_RELATIVE)
             {
                 if (data!=NULL)
                 {
                     data->name="Zoom, Relative";
                     data->type=V4L2_CTRL_TYPE_INTEGER;
                     data->selector=VCT_ZOOM_RELATIVE_CONTROL;
                     data->unit=UVC_CAMERA_SELECTOR;
                     data->size=3;
                 }
                 return 1;
             }
             break;
        case V4L2_CID_PAN_ABSOLUTE:
             if (dev->vc_iterminal.bmControls[1] & VIT_B1_PANTILT_ABSOLUTE)
             {
                 if (data!=NULL)
                 {
                     data->name="Pan, Absolute";
                     data->type=V4L2_CTRL_TYPE_INTEGER;
                     data->selector=VCT_PANTILT_ABSOLUTE_CONTROL;
                     data->unit=UVC_CAMERA_SELECTOR;
                     data->size=8;
                 }
                 return 1;
             }
             break;
        case V4L2_CID_TILT_ABSOLUTE:
             if (dev->vc_iterminal.bmControls[1] & VIT_B1_PANTILT_ABSOLUTE)
             {
                 if (data!=NULL)
                 {
                     data->name="Tilt, Absolute";
                     data->type=V4L2_CTRL_TYPE_INTEGER;
                     data->selector=VCT_PANTILT_ABSOLUTE_CONTROL;
                     data->unit=UVC_CAMERA_SELECTOR;
                     data->size=8;
                 }
                 return 1;
             }
             break;
        case V4L2_CID_PRIVACY:
             if (dev->vc_iterminal.bmControls[2] & VIT_B2_PRIVACY)
             {
                 if (data!=NULL)
                 {
                     data->name="Privacy Shutter";
                     data->type=V4L2_CTRL_TYPE_INTEGER;
                     data->selector=VCT_PRIVACY_CONTROL;
                     data->unit=UVC_CAMERA_SELECTOR;
                     data->size=1;
                 }
                 return 1;
             }
        case V4L2_CID_FOCUS_AUTO:
             if (dev->vc_iterminal.bmControls[2] & VIT_B2_FOCUS_AUTO)
             {
                 if (data!=NULL)
                 {
                     data->name="Focus, Auto";
                     data->type=V4L2_CTRL_TYPE_BOOLEAN;
                     data->selector=VCT_FOCUS_AUTO_CONTROL;
                     data->unit=UVC_CAMERA_SELECTOR;
                     data->size=1;
                 }
                 return 1;
             }
             break;
        case UVC_CID_EVENT_BUTTON:
             if ((dev->vs_input_header[subdev].bTriggerSupport) &&
                 (dev->vs_input_header[subdev].bTriggerUsage))
             {
                 if (data!=NULL)
                 {
                     data->name="Trigger Button";
                     data->type=V4L2_CTRL_TYPE_BOOLEAN;
                     data->selector=0;
                     data->unit=0;
                     data->size=1;
                 }
                 return 1;
             }
             break;

        /* These are not supported by V4L2          */
        /*                                          */
        /* VCT_ZOOM_RELATIVE_CONTROL, on/off dig    */
        /*                            zoom          */
        /*                                          */
        /* VPU_B1_DIGITAL_MULTIPLIER (size 2)       */
        /* VPU_B1_DIGITAL_MULTIPLIER_LIMIT (size 2) */
        /* VPU_B2_CONTRAST_AUTO (size 1)            */
        /* VPU_B2_ANALOG_VIDEO_STANDARD (size 1)    */
        /* VPU_B2_ANALOG_VIDEO_LOCK_STATUS (size 1) */
        /* VIT_B0_SCANNING_MODE (size 1)            */
        /* VIT_B0_EXPOSURE_TIME_RELATIVE (size 1)   */
        /* VIT_B1_PANTILT_RELATIVE (size 4) havesup */
        /* VIT_B1_ROLL_ABSOLUTE (size 2)            */
        /* VIT_B1_ROLL_RELATIVE (size 2)            */
        /* VIT_B2_FOCUS_SIMPLE (size 1)             */
        /* VIT_B2_WINDOW (size 12)                  */
        /* VIT_B2_REGION_OF_INTEREST (size 10)      */
    }

    return 0;
}

int uvc_read_ctrl(uvc_device_t* dev, struct v4l2_control* ctrl, control_data_t* data)
{
    unsigned char value[16];
    int status=0;
    int ret=EOK;

    switch (data->type)
    {
        case V4L2_CTRL_TYPE_INTEGER:
             switch (ctrl->id)
             {
                 case V4L2_CID_BLUE_BALANCE:
                      status|=uvc_control_get(dev, VGET_CUR, data->unit, data->selector, data->size, &value[0]);
                      ctrl->value=uvc_get_sinteger(2, &value[0]);
                      break;
                 case V4L2_CID_RED_BALANCE:
                      status|=uvc_control_get(dev, VGET_CUR, data->unit, data->selector, data->size, &value[0]);
                      ctrl->value=uvc_get_sinteger(2, &value[2]);
                      break;
                 default:
                      status|=uvc_control_get(dev, VGET_CUR, data->unit, data->selector, data->size, &value[0]);
                      ctrl->value=uvc_get_sinteger(data->size, &value[0]);
                      break;
             }
             break;
        case V4L2_CTRL_TYPE_MENU:
             switch (ctrl->id)
             {
                 case V4L2_CID_POWER_LINE_FREQUENCY:
                      status|=uvc_control_get(dev, VGET_CUR, data->unit, data->selector, data->size, &value[0]);
                      ctrl->value=uvc_get_sinteger(data->size, &value[0]);
                      break;
             }
             break;
        case V4L2_CTRL_TYPE_CTRL_CLASS:
             if (uvc_verbose>2)
             {
                 slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EACCES: control can't be retrieved");
             }
             ret=EACCES;
             break;
        case V4L2_CTRL_TYPE_BOOLEAN:
             switch (ctrl->id)
             {
                 default:
                      status|=uvc_control_get(dev, VGET_CUR, data->unit, data->selector, data->size, &value[0]);
                      ctrl->value=uvc_get_sinteger(data->size, &value[0]);
                      if (ctrl->value)
                      {
                          ctrl->value=1;
                      }
                      break;
             }
             break;
    }

    if ((ret==EOK) && (status))
    {
        if (uvc_verbose>2)
        {
            slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EIO: USB I/O error");
        }
        ret=EIO;
    }

    return ret;
}

int uvc_write_ctrl(uvc_device_t* dev, struct v4l2_control* ctrl, control_data_t* data, uvc_ocb_t* ocb)
{
    unsigned char value[16];
    unsigned char value1[16];
    unsigned char value2[16];
    unsigned char value3[16];
    int64_t min_value;
    int64_t max_value;
    int64_t res_value;
    int ret=EOK;
    int status=0;
    int it;

    switch (data->type)
    {
        case V4L2_CTRL_TYPE_INTEGER:
             switch (ctrl->id)
             {
                 case V4L2_CID_BLUE_BALANCE:
                      status|=uvc_control_get(dev, VGET_MIN, data->unit, data->selector, data->size, &value1[0]);
                      status|=uvc_control_get(dev, VGET_MAX, data->unit, data->selector, data->size, &value2[0]);
                      status|=uvc_control_get(dev, VGET_RES, data->unit, data->selector, data->size, &value3[0]);
                      min_value=uvc_get_sinteger(2, &value1[0]);
                      max_value=uvc_get_sinteger(2, &value2[0]);
                      res_value=uvc_get_sinteger(2, &value3[0]);
                      if ((ctrl->value<min_value) || (ctrl->value>max_value) ||
                          ((res_value!=1) && (ctrl->value%res_value!=0)))
                      {
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        ERANGE: control value is out of range");
                          }
                          ret=ERANGE;
                          break;
                      }
                      status|=uvc_control_get(dev, VGET_CUR, data->unit, data->selector, data->size, &value[0]);
                      uvc_set_sinteger(2, &value[0], ctrl->value);
                      status|=uvc_control_set(dev, VSET_CUR, data->unit, data->selector, data->size, &value[0]);
                      if (status)
                      {
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EIO: USB I/O error");
                          }
                          ret=EIO;
                          break;
                      }
                      break;
                 case V4L2_CID_RED_BALANCE:
                      status|=uvc_control_get(dev, VGET_MIN, data->unit, data->selector, data->size, &value1[0]);
                      status|=uvc_control_get(dev, VGET_MAX, data->unit, data->selector, data->size, &value2[0]);
                      status|=uvc_control_get(dev, VGET_RES, data->unit, data->selector, data->size, &value3[0]);
                      min_value=uvc_get_sinteger(2, &value1[2]);
                      max_value=uvc_get_sinteger(2, &value2[2]);
                      res_value=uvc_get_sinteger(2, &value3[2]);
                      if ((ctrl->value<min_value) || (ctrl->value>max_value) ||
                          ((res_value!=1) && (ctrl->value%res_value!=0)))
                      {
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        ERANGE: control value is out of range");
                          }
                          ret=ERANGE;
                          break;
                      }
                      status|=uvc_control_get(dev, VGET_CUR, data->unit, data->selector, data->size, &value[0]);
                      uvc_set_sinteger(2, &value[2], ctrl->value);
                      status|=uvc_control_set(dev, VSET_CUR, data->unit, data->selector, data->size, &value[0]);
                      if (status)
                      {
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EIO: USB I/O error");
                          }
                          ret=EIO;
                          break;
                      }
                      break;
                 default:
                      status|=uvc_control_get(dev, VGET_MIN, data->unit, data->selector, data->size, &value1[0]);
                      status|=uvc_control_get(dev, VGET_MAX, data->unit, data->selector, data->size, &value2[0]);
                      status|=uvc_control_get(dev, VGET_RES, data->unit, data->selector, data->size, &value3[0]);
                      min_value=uvc_get_sinteger(data->size, &value1[0]);
                      max_value=uvc_get_sinteger(data->size, &value2[0]);
                      res_value=uvc_get_sinteger(data->size, &value3[0]);
                      if ((ctrl->value<min_value) || (ctrl->value>max_value) ||
                          ((res_value!=1) && (ctrl->value%res_value!=0)))
                      {
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        ERANGE: control value is out of range");
                          }
                          ret=ERANGE;
                          break;
                      }
                      uvc_set_sinteger(data->size, &value[0], ctrl->value);
                      status|=uvc_control_set(dev, VSET_CUR, data->unit, data->selector, data->size, &value[0]);
                      if (status)
                      {
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EIO: USB I/O error");
                          }
                          ret=EIO;
                          break;
                      }
                      break;
             }
             break;
        case V4L2_CTRL_TYPE_MENU:
             switch (ctrl->id)
             {
                 case V4L2_CID_POWER_LINE_FREQUENCY:
                      if ((ctrl->value<0) || (ctrl->value>2))
                      {
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        ERANGE: control value is out of range");
                          }
                          ret=ERANGE;
                          break;
                      }
                      uvc_set_sinteger(data->size, &value[0], ctrl->value);
                      status=uvc_control_set(dev, VSET_CUR, data->unit, data->selector, data->size, &value[0]);
                      if (status)
                      {
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EIO: USB I/O error");
                          }
                          ret=EIO;
                          break;
                      }
                      break;
             }
             break;
        case V4L2_CTRL_TYPE_CTRL_CLASS:
             if (uvc_verbose>2)
             {
                 slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EACCES: control can't be set");
             }
             ret=EACCES;
             break;
        case V4L2_CTRL_TYPE_BOOLEAN:
             switch (ctrl->id)
             {
                 default:
                      if ((ctrl->value!=1) && (ctrl->value!=0))
                      {
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        ERANGE: control value is out of range");
                          }
                          ret=ERANGE;
                          break;
                      }
                      uvc_set_sinteger(data->size, &value[0], ctrl->value);
                      status=uvc_control_set(dev, VSET_CUR, data->unit, data->selector, data->size, &value[0]);
                      if (status)
                      {
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EIO: USB I/O error");
                          }
                          ret=EIO;
                          break;
                      }
                      break;
             }
    }

    if (ret==EOK)
    {
        for(it=0; it<UVC_MAX_OPEN_FDS; it++)
        {
            if (((dev->event[it].ocb==ocb) && ((dev->event[it].flags[V4L2_EVENT_CTRL] & V4L2_EVENT_SUB_FL_ALLOW_FEEDBACK)==V4L2_EVENT_SUB_FL_ALLOW_FEEDBACK)) ||
                (dev->event[it].ocb!=NULL))
            {
                /* Check file descriptor listening for ctrl event */
                if (dev->event[it].type[V4L2_EVENT_CTRL])
                {
                    /* Check if file descriptor listening for required control id */
                    if ((dev->event[it].type[V4L2_EVENT_CTRL]==0) || (dev->event[it].type[V4L2_EVENT_CTRL]==ctrl->id))
                    {
                        struct _uvc_event_entry* entry;

                        entry=calloc(1, sizeof(*entry));
                        if (entry==NULL)
                        {
                            ret=ENOMEM;
                            break;
                        }

                        /* Emit control change event */
                        entry->event.type=V4L2_EVENT_CTRL;
                        entry->event.pending=0;
                        entry->event.sequence=dev->event[it].sequence;
                        entry->event.id=ctrl->id;
                        clock_gettime(CLOCK_MONOTONIC, &entry->event.timestamp);
                        memset(entry->event.reserved, 0x00, sizeof(entry->event.reserved));
                        entry->event.u.ctrl.changes=V4L2_EVENT_CTRL_CH_VALUE;
                        entry->event.u.ctrl.type=data->type;
                        entry->event.u.ctrl.flags=0;
                        entry->event.u.ctrl.value=ctrl->value;
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

    return ret;
}

int uvc_read_ext_ctrl(uvc_device_t* dev, struct v4l2_ext_control* ctrl, control_data_t* data)
{
    unsigned char value[16];
    int status=0;
    int ret=EOK;

    switch (data->type)
    {
        case V4L2_CTRL_TYPE_CTRL_CLASS:
             if (uvc_verbose>2)
             {
                 slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EACCES: control can't be retrieved");
             }
             ret=EACCES;
             break;
        case V4L2_CTRL_TYPE_BOOLEAN:
             switch (ctrl->id)
             {
                 case UVC_CID_EVENT_BUTTON:
                      ctrl->value=dev->event_button;
                      break;
                 default:
                      status|=uvc_control_get(dev, VGET_CUR, data->unit, data->selector, data->size, &value[0]);
                      ctrl->value=uvc_get_sinteger(data->size, &value[0]);
                      break;
             }
             break;
        default:
             switch (ctrl->id)
             {
                 case V4L2_CID_FOCUS_RELATIVE:
                      status|=uvc_control_get(dev, VGET_CUR, data->unit, data->selector, data->size, &value[0]);
                      ctrl->value=uvc_get_uinteger(1, &value[1]);
                      if (value[0]==0xFF)
                      {
                          ctrl->value=-ctrl->value;
                      }
                      break;
                 case V4L2_CID_ZOOM_RELATIVE:
                      status|=uvc_control_get(dev, VGET_CUR, data->unit, data->selector, data->size, &value[0]);
                      ctrl->value=uvc_get_sinteger(1, &value[2]);
                      if (value[0]==0xFF)
                      {
                          ctrl->value=-ctrl->value;
                      }
                 case V4L2_CID_PAN_ABSOLUTE:
                      status|=uvc_control_get(dev, VGET_CUR, data->unit, data->selector, data->size, &value[0]);
                      ctrl->value=uvc_get_sinteger(4, &value[0]);
                      break;
                 case V4L2_CID_TILT_ABSOLUTE:
                      status|=uvc_control_get(dev, VGET_CUR, data->unit, data->selector, data->size, &value[0]);
                      ctrl->value=uvc_get_sinteger(4, &value[4]);
                      break;
                 case V4L2_CID_EXPOSURE_AUTO:
                      status|=uvc_control_get(dev, VGET_CUR, data->unit, data->selector, data->size, &value[0]);
                      ctrl->value=uvc_get_sinteger(data->size, &value[0]);
                      switch(ctrl->value)
                      {
                          case 8:
                               ctrl->value=3;
                               break;
                          case 4:
                               ctrl->value=2;
                               break;
                          case 2:
                               ctrl->value=1;
                               break;
                          case 1:
                               ctrl->value=0;
                               break;
                      }
                      break;
                 default:
                      status|=uvc_control_get(dev, VGET_CUR, data->unit, data->selector, data->size, &value[0]);
                      ctrl->value=uvc_get_sinteger(data->size, &value[0]);
                      break;
             }
    }

    if ((ret==EOK) && (status))
    {
        if (uvc_verbose>2)
        {
            slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EIO: USB I/O error");
        }
        ret=EIO;
    }

    return ret;
}

int uvc_write_ext_ctrl(uvc_device_t* dev, struct v4l2_ext_control* ctrl, control_data_t* data, uvc_ocb_t* ocb)
{
    unsigned char value[16];
    unsigned char value1[16];
    unsigned char value2[16];
    unsigned char value3[16];
    int64_t min_value;
    int64_t max_value;
    int64_t res_value;
    int ret=EOK;
    int status=0;
    int it;

    switch (data->type)
    {
        case V4L2_CTRL_TYPE_CTRL_CLASS:
             if (uvc_verbose>2)
             {
                 slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EACCES: attempt to write to class control");
             }
             ret=EACCES;
             break;
        case V4L2_CTRL_TYPE_INTEGER:
             switch (ctrl->id)
             {
                 int sign;

                 case V4L2_CID_FOCUS_RELATIVE:
                      status|=uvc_control_get(dev, VGET_MIN, data->unit, data->selector, data->size, &value1[0]);
                      status|=uvc_control_get(dev, VGET_MAX, data->unit, data->selector, data->size, &value2[0]);
                      status|=uvc_control_get(dev, VGET_RES, data->unit, data->selector, data->size, &value3[0]);
                      min_value=-uvc_get_uinteger(1, &value2[1]);
                      max_value=+uvc_get_uinteger(1, &value2[1]);
                      res_value=uvc_get_uinteger(1, &value3[1]);
                      if ((ctrl->value<min_value) || (ctrl->value>max_value) ||
                          ((res_value!=1) && (ctrl->value%res_value!=0)))
                      {
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        ERANGE: control value is out of range");
                          }
                          ret=ERANGE;
                          break;
                      }
                      if (ctrl->value<0)
                      {
                          sign=0xFF;
                      }
                      else
                      {
                          if (ctrl->value>0)
                          {
                              sign=0x01;
                          }
                          else
                          {
                              sign=0x00;
                          }
                      }
                      uvc_set_sinteger(data->size, &value[0], (abs(ctrl->value)<<8) | sign);
                      status|=uvc_control_set(dev, VSET_CUR, data->unit, data->selector, data->size, &value[0]);
                      if (status)
                      {
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EIO: USB I/O error");
                          }
                          ret=EIO;
                          break;
                      }
                      break;
                 case V4L2_CID_ZOOM_RELATIVE:
                      status|=uvc_control_get(dev, VGET_MIN, data->unit, data->selector, data->size, &value1[0]);
                      status|=uvc_control_get(dev, VGET_MAX, data->unit, data->selector, data->size, &value2[0]);
                      status|=uvc_control_get(dev, VGET_RES, data->unit, data->selector, data->size, &value3[0]);
                      min_value=-uvc_get_sinteger(1, &value2[2]);
                      max_value=+uvc_get_sinteger(1, &value2[2]);
                      res_value=uvc_get_sinteger(1, &value3[2]);
                      if ((ctrl->value<min_value) || (ctrl->value>max_value) ||
                          ((res_value!=1) && (ctrl->value%res_value!=0)))
                      {
                           if (uvc_verbose>2)
                           {
                               slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        ERANGE: control value is out of range");
                           }
                           ret=ERANGE;
                           break;
                      }
                      if (ctrl->value<0)
                      {
                          sign=0xFF;
                      }
                      else
                      {
                          if (ctrl->value>0)
                          {
                              sign=0x01;
                          }
                          else
                          {
                              sign=0x00;
                          }
                      }
                      uvc_set_sinteger(data->size, &value[0], (abs(ctrl->value)<<16) | sign);
                      status|=uvc_control_set(dev, VSET_CUR, data->unit, data->selector, data->size, &value[0]);
                      if (status)
                      {
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EIO: USB I/O error");
                          }
                          ret=EIO;
                          break;
                      }
                      break;
                 case V4L2_CID_PAN_ABSOLUTE:
                      status|=uvc_control_get(dev, VGET_MIN, data->unit, data->selector, data->size, &value1[0]);
                      status|=uvc_control_get(dev, VGET_MAX, data->unit, data->selector, data->size, &value2[0]);
                      status|=uvc_control_get(dev, VGET_RES, data->unit, data->selector, data->size, &value3[0]);
                      min_value=uvc_get_sinteger(4, &value1[0]);
                      max_value=uvc_get_sinteger(4, &value2[0]);
                      res_value=uvc_get_sinteger(4, &value3[0]);
                      if ((ctrl->value<min_value) || (ctrl->value>max_value) ||
                          ((res_value!=1) && (ctrl->value%res_value!=0)))
                      {
                           if (uvc_verbose>2)
                           {
                               slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        ERANGE: control value is out of range");
                           }
                           ret=ERANGE;
                           break;
                      }
                      status|=uvc_control_get(dev, VGET_CUR, data->unit, data->selector, data->size, &value[0]);
                      uvc_set_sinteger(4, &value[0], ctrl->value);
                      status|=uvc_control_set(dev, VSET_CUR, data->unit, data->selector, data->size, &value[0]);
                      if (status)
                      {
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EIO: USB I/O error");
                          }
                          ret=EIO;
                          break;
                      }
                      break;
                 case V4L2_CID_TILT_ABSOLUTE:
                      status|=uvc_control_get(dev, VGET_MIN, data->unit, data->selector, data->size, &value1[0]);
                      status|=uvc_control_get(dev, VGET_MAX, data->unit, data->selector, data->size, &value2[0]);
                      status|=uvc_control_get(dev, VGET_RES, data->unit, data->selector, data->size, &value3[0]);
                      min_value=uvc_get_sinteger(4, &value1[4]);
                      max_value=uvc_get_sinteger(4, &value2[4]);
                      res_value=uvc_get_sinteger(4, &value3[4]);
                      if ((ctrl->value<min_value) || (ctrl->value>max_value) ||
                          ((res_value!=1) && (ctrl->value%res_value!=0)))
                      {
                           if (uvc_verbose>2)
                           {
                               slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        ERANGE: control value is out of range");
                           }
                           ret=ERANGE;
                           break;
                      }
                      status|=uvc_control_get(dev, VGET_CUR, data->unit, data->selector, data->size, &value[0]);
                      uvc_set_sinteger(4, &value[4], ctrl->value);
                      status|=uvc_control_set(dev, VSET_CUR, data->unit, data->selector, data->size, &value[0]);
                      if (status)
                      {
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EIO: USB I/O error");
                          }
                          ret=EIO;
                          break;
                      }
                      break;
                 default:
                      status|=uvc_control_get(dev, VGET_MIN, data->unit, data->selector, data->size, &value1[0]);
                      status|=uvc_control_get(dev, VGET_MAX, data->unit, data->selector, data->size, &value2[0]);
                      status|=uvc_control_get(dev, VGET_RES, data->unit, data->selector, data->size, &value3[0]);
                      min_value=uvc_get_sinteger(data->size, &value1[0]);
                      max_value=uvc_get_sinteger(data->size, &value2[0]);
                      res_value=uvc_get_sinteger(data->size, &value3[0]);
                      if ((ctrl->value<min_value) || (ctrl->value>max_value) ||
                          ((res_value!=1) && (ctrl->value%res_value!=0)))
                      {
                           if (uvc_verbose>2)
                           {
                               slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        ERANGE: control value is out of range");
                           }
                           ret=ERANGE;
                           break;
                      }
                      uvc_set_sinteger(data->size, &value[0], ctrl->value);
                      status|=uvc_control_set(dev, VSET_CUR, data->unit, data->selector, data->size, &value[0]);
                      if (status)
                      {
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EIO: USB I/O error");
                          }
                          ret=EIO;
                          break;
                      }
                      break;
             }
             break;
        case V4L2_CTRL_TYPE_MENU:
             switch (ctrl->id)
             {
                 case V4L2_CID_POWER_LINE_FREQUENCY:
                      if ((ctrl->value<0) || (ctrl->value>2))
                      {
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        ERANGE: control value is out of range");
                          }
                          ret=ERANGE;
                          break;
                      }
                      uvc_set_sinteger(data->size, &value[0], ctrl->value);
                      status|=uvc_control_set(dev, VSET_CUR, data->unit, data->selector, data->size, &value[0]);
                      if (status)
                      {
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EIO: USB I/O error");
                          }
                          ret=EIO;
                          break;
                      }
                      break;
                 case V4L2_CID_EXPOSURE_AUTO:
                      if ((ctrl->value<0) || (ctrl->value>3))
                      {
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        ERANGE: control value is out of range");
                          }
                          ret=ERANGE;
                          break;
                      }
                      uvc_set_sinteger(data->size, &value[0], (1<<ctrl->value));
                      status=uvc_control_set(dev, VSET_CUR, data->unit, data->selector, data->size, &value[0]);
                      if (status)
                      {
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EIO: USB I/O error");
                          }
                          ret=EIO;
                          break;
                      }
                      break;
             }
             break;
        case V4L2_CTRL_TYPE_BOOLEAN:
             switch (ctrl->id)
             {
                 case UVC_CID_EVENT_BUTTON:
                      if (uvc_verbose>2)
                      {
                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EACCES: control can't be set");
                      }
                      ret=EACCES;
                      break;
                 default:
                      if ((ctrl->value!=1) && (ctrl->value!=0))
                      {
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        ERANGE: control value is out of range");
                          }
                          ret=ERANGE;
                          break;
                      }
                      uvc_set_sinteger(data->size, &value[0], ctrl->value);
                      status=uvc_control_set(dev, VSET_CUR, data->unit, data->selector, data->size, &value[0]);
                      if (status)
                      {
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EIO: USB I/O error");
                          }
                          ret=EIO;
                      }
                      break;
             }
             break;
    }

    if (ret==EOK)
    {
        for(it=0; it<UVC_MAX_OPEN_FDS; it++)
        {
            if (((dev->event[it].ocb==ocb) && ((dev->event[it].flags[V4L2_EVENT_CTRL] & V4L2_EVENT_SUB_FL_ALLOW_FEEDBACK)==V4L2_EVENT_SUB_FL_ALLOW_FEEDBACK)) ||
                (dev->event[it].ocb!=NULL))
            {
                /* Check file descriptor listening for ctrl event */
                if (dev->event[it].type[V4L2_EVENT_CTRL])
                {
                    /* Check if file descriptor listening for required control id */
                    if ((dev->event[it].type[V4L2_EVENT_CTRL]==0) || (dev->event[it].type[V4L2_EVENT_CTRL]==ctrl->id))
                    {
                        struct _uvc_event_entry* entry;

                        entry=calloc(1, sizeof(*entry));
                        if (entry==NULL)
                        {
                            ret=ENOMEM;
                            break;
                        }

                        /* Emit control change event */
                        entry->event.type=V4L2_EVENT_CTRL;
                        entry->event.pending=0;
                        entry->event.sequence=dev->event[it].sequence;
                        entry->event.id=ctrl->id;
                        clock_gettime(CLOCK_MONOTONIC, &entry->event.timestamp);
                        memset(entry->event.reserved, 0x00, sizeof(entry->event.reserved));
                        entry->event.u.ctrl.changes=V4L2_EVENT_CTRL_CH_VALUE;
                        entry->event.u.ctrl.type=data->type;
                        entry->event.u.ctrl.flags=0;
                        entry->event.u.ctrl.value=ctrl->value;
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

    return ret;
}

int uvc_query_control(uvc_device_t* dev, uint32_t subdev, struct v4l2_queryctrl* ctrl)
{
    control_data_t data;
    unsigned char value1[16];
    unsigned char value2[16];
    unsigned char value3[16];
    unsigned char value4[16];
    int status=0;
    int ret=EOK;

    data.type=0;
    uvc_query_control_data(dev, ctrl->id, subdev, &data);

    ctrl->type=data.type;
    strncpy((char*)ctrl->name, data.name, sizeof(ctrl->name));
    ctrl->flags=0;

    switch (ctrl->type)
    {
        case V4L2_CTRL_TYPE_INTEGER:
             switch (ctrl->id)
             {
                 case V4L2_CID_FOCUS_RELATIVE:
                      /* This control consist of two parameters */
                      status|=uvc_control_get(dev, VGET_MIN, data.unit, data.selector, data.size, &value1[0]);
                      status|=uvc_control_get(dev, VGET_MAX, data.unit, data.selector, data.size, &value2[0]);
                      status|=uvc_control_get(dev, VGET_RES, data.unit, data.selector, data.size, &value3[0]);
                      status|=uvc_control_get(dev, VGET_DEF, data.unit, data.selector, data.size, &value4[0]);
                      /* Use maximum value for minimum but with reverse sign */
                      ctrl->minimum=-uvc_get_uinteger(1, &value2[1]);
                      ctrl->maximum=+uvc_get_uinteger(1, &value2[1]);
                      ctrl->step=uvc_get_uinteger(1, &value3[1]);
                      ctrl->default_value=uvc_get_uinteger(1, &value4[1]);
                      break;
                 case V4L2_CID_IRIS_RELATIVE:
                      /* Do not support get max/min according to specification */
                      ctrl->minimum=-1;
                      ctrl->maximum=1;
                      ctrl->step=1;
                      ctrl->default_value=0;
                      break;
                 case V4L2_CID_ZOOM_RELATIVE:
                      /* This control consist of three parameters */
                      status|=uvc_control_get(dev, VGET_MIN, data.unit, data.selector, data.size, &value1[0]);
                      status|=uvc_control_get(dev, VGET_MAX, data.unit, data.selector, data.size, &value2[0]);
                      status|=uvc_control_get(dev, VGET_RES, data.unit, data.selector, data.size, &value3[0]);
                      status|=uvc_control_get(dev, VGET_DEF, data.unit, data.selector, data.size, &value4[0]);
                      /* Use maximum value for minimum but with reverse sign */
                      ctrl->minimum=-uvc_get_sinteger(1, &value2[2]);
                      ctrl->maximum=+uvc_get_sinteger(1, &value2[2]);
                      ctrl->step=uvc_get_sinteger(1, &value3[2]);
                      ctrl->default_value=uvc_get_sinteger(1, &value4[2]);
                      break;
                 case V4L2_CID_PAN_ABSOLUTE:
                      /* This control consist of two parameters */
                      status|=uvc_control_get(dev, VGET_MIN, data.unit, data.selector, data.size, &value1[0]);
                      status|=uvc_control_get(dev, VGET_MAX, data.unit, data.selector, data.size, &value2[0]);
                      status|=uvc_control_get(dev, VGET_RES, data.unit, data.selector, data.size, &value3[0]);
                      status|=uvc_control_get(dev, VGET_DEF, data.unit, data.selector, data.size, &value4[0]);
                      ctrl->minimum=uvc_get_sinteger(4, &value1[0]);
                      ctrl->maximum=uvc_get_sinteger(4, &value2[0]);
                      ctrl->step=uvc_get_sinteger(4, &value3[0]);
                      ctrl->default_value=uvc_get_sinteger(4, &value4[0]);
                      break;
                 case V4L2_CID_TILT_ABSOLUTE:
                      /* This control consist of two parameters */
                      status|=uvc_control_get(dev, VGET_MIN, data.unit, data.selector, data.size, &value1[0]);
                      status|=uvc_control_get(dev, VGET_MAX, data.unit, data.selector, data.size, &value2[0]);
                      status|=uvc_control_get(dev, VGET_RES, data.unit, data.selector, data.size, &value3[0]);
                      status|=uvc_control_get(dev, VGET_DEF, data.unit, data.selector, data.size, &value4[0]);
                      ctrl->minimum=uvc_get_sinteger(4, &value1[4]);
                      ctrl->maximum=uvc_get_sinteger(4, &value2[4]);
                      ctrl->step=uvc_get_sinteger(4, &value3[4]);
                      ctrl->default_value=uvc_get_sinteger(4, &value4[4]);
                      break;
                 case V4L2_CID_BLUE_BALANCE:
                      /* This control consist of two parameters */
                      status|=uvc_control_get(dev, VGET_MIN, data.unit, data.selector, data.size, &value1[0]);
                      status|=uvc_control_get(dev, VGET_MAX, data.unit, data.selector, data.size, &value2[0]);
                      status|=uvc_control_get(dev, VGET_RES, data.unit, data.selector, data.size, &value3[0]);
                      status|=uvc_control_get(dev, VGET_DEF, data.unit, data.selector, data.size, &value4[0]);
                      ctrl->minimum=uvc_get_sinteger(2, &value1[0]);
                      ctrl->maximum=uvc_get_sinteger(2, &value2[0]);
                      ctrl->step=uvc_get_sinteger(2, &value3[0]);
                      ctrl->default_value=uvc_get_sinteger(2, &value4[0]);
                      break;
                 case V4L2_CID_RED_BALANCE:
                      /* This control consist of two parameters */
                      status|=uvc_control_get(dev, VGET_MIN, data.unit, data.selector, data.size, &value1[0]);
                      status|=uvc_control_get(dev, VGET_MAX, data.unit, data.selector, data.size, &value2[0]);
                      status|=uvc_control_get(dev, VGET_RES, data.unit, data.selector, data.size, &value3[0]);
                      status|=uvc_control_get(dev, VGET_DEF, data.unit, data.selector, data.size, &value4[0]);
                      ctrl->minimum=uvc_get_sinteger(2, &value1[2]);
                      ctrl->maximum=uvc_get_sinteger(2, &value2[2]);
                      ctrl->step=uvc_get_sinteger(2, &value3[2]);
                      ctrl->default_value=uvc_get_sinteger(2, &value4[2]);
                      break;
                 default:
                      status|=uvc_control_get(dev, VGET_MIN, data.unit, data.selector, data.size, &value1[0]);
                      status|=uvc_control_get(dev, VGET_MAX, data.unit, data.selector, data.size, &value2[0]);
                      status|=uvc_control_get(dev, VGET_RES, data.unit, data.selector, data.size, &value3[0]);
                      status|=uvc_control_get(dev, VGET_DEF, data.unit, data.selector, data.size, &value4[0]);
                      ctrl->minimum=uvc_get_sinteger(data.size, &value1[0]);
                      ctrl->maximum=uvc_get_sinteger(data.size, &value2[0]);
                      ctrl->step=uvc_get_sinteger(data.size, &value3[0]);
                      ctrl->default_value=uvc_get_sinteger(data.size, &value4[0]);
                      break;
             }
             break;
        case V4L2_CTRL_TYPE_MENU:
             switch (ctrl->id)
             {
                 case V4L2_CID_POWER_LINE_FREQUENCY:
                      status|=uvc_control_get(dev, VGET_DEF, data.unit, data.selector, data.size, &value4[0]);
                      ctrl->default_value=uvc_get_sinteger(data.size, &value4[0]);
                      ctrl->minimum=0;
                      ctrl->maximum=2;
                      ctrl->step=1;
                      break;
                 case V4L2_CID_EXPOSURE_AUTO:
                      status|=uvc_control_get(dev, VGET_DEF, data.unit, data.selector, data.size, &value4[0]);
                      ctrl->default_value=uvc_get_sinteger(data.size, &value4[0]);
                      switch(ctrl->default_value)
                      {
                          case 8:
                               ctrl->default_value=3;
                               break;
                          case 4:
                               ctrl->default_value=2;
                               break;
                          case 2:
                               ctrl->default_value=1;
                               break;
                          case 1:
                               ctrl->default_value=0;
                               break;
                      }
                      ctrl->minimum=0;
                      ctrl->maximum=3;
                      ctrl->step=1;
                      break;
             }
             break;
        case V4L2_CTRL_TYPE_CTRL_CLASS:
             ctrl->minimum=0;
             ctrl->maximum=0;
             ctrl->step=0;
             ctrl->default_value=0;
             ctrl->flags=V4L2_CTRL_FLAG_READ_ONLY | V4L2_CTRL_FLAG_WRITE_ONLY;
             break;
        case V4L2_CTRL_TYPE_BOOLEAN:
             ctrl->minimum=0;
             ctrl->maximum=1;
             ctrl->step=1;
             switch (ctrl->id)
             {
                 case UVC_CID_EVENT_BUTTON:
                      ctrl->flags=V4L2_CTRL_FLAG_READ_ONLY | V4L2_CTRL_FLAG_VOLATILE;
                      ctrl->default_value=0;
                      break;
                 default:
                      status|=uvc_control_get(dev, VGET_DEF, data.unit, data.selector, data.size, &value4[0]);
                      ctrl->default_value=uvc_get_sinteger(data.size, &value4[0]);
                      break;
             }
             break;
    }

    if ((ret==EOK) && (status))
    {
        if (uvc_verbose>2)
        {
            slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EIO: USB I/O error");
        }
        ret=EIO;
    }

    return ret;
}

int uvc_devctl(resmgr_context_t* ctp, io_devctl_t* msg, uvc_ocb_t* ocb)
{
    int status, ret=EOK;
    uvc_device_t* dev=ocb->dev;
    void* dptr=_DEVCTL_DATA(msg->i);
    int dctldatasize=0;
    int dctlextdatasize=0;
    int subdev=-1;
    int it, jt;
    int match;

    status = iofunc_devctl_default(ctp, msg, &ocb->hdr);
    if (status != _RESMGR_DEFAULT)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] iofunc_devctl_default() failed");
        return _RESMGR_ERRNO(status);
    }

    for (it=0; it<dev->total_vs_devices; it++)
    {
        if (dev->map->devid[it]==minor(ocb->hdr.attr->hdr->rdev))
        {
            subdev=it;
            break;
        }
    }
    if (subdev==-1)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] devctl(): can't locate subdevice!");
        return _RESMGR_ERRNO(ENODEV);
    }

    if (uvc_verbose>2)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "devctl(): cmd %08X, subdevice %d, rdev=%08X, ocb=%08X", msg->i.dcmd, subdev, ocb->hdr.attr->hdr->rdev, (unsigned int)ocb);
    }

    switch (msg->i.dcmd)
    {
        case VIDIOC_QUERYCAP:
             {
                 struct v4l2_capability* cap;

                 cap=(struct v4l2_capability*)dptr;
                 memset(cap, 0x00, sizeof(*cap));

                 strncpy((char*)cap->driver, "devu-uvc", sizeof(cap->driver));
                 strncpy((char*)cap->card, dev->device_id_str, sizeof(cap->card));
                 snprintf((char*)cap->bus_info, sizeof(cap->bus_info), "usb-%d:%d", dev->map->usb_path, dev->map->usb_devno);
                 /* Pretend it is linux running 6.x.0 kernel */
                 cap->version=((_NTO_VERSION/100)<<16) | (((_NTO_VERSION-(_NTO_VERSION/100)*100)/10)<<8);
                 cap->capabilities=V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_DEVICE_CAPS |
                                   V4L2_CAP_STREAMING;
                 cap->device_caps=V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;
                 dctldatasize=sizeof(*cap);

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_QUERYCAP:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        driver: %s", cap->driver);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        card: %s", cap->card);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        bus_info: %s", cap->bus_info);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        version: %08X", cap->version);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        capabilities: %08X", cap->capabilities);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        device_caps: %08X", cap->device_caps);
                 }
             }
             break;
        case VIDIOC_ENUM_FMT:
             {
                 struct v4l2_fmtdesc* fmt;

                 fmt=(struct v4l2_fmtdesc*)dptr;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_ENUM_FMT:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        type: %08X", fmt->type);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        index: %d", fmt->index);
                 }

                 if (fmt->type!=V4L2_BUF_TYPE_VIDEO_CAPTURE)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: buffer type is not V4L2_BUF_TYPE_VIDEO_CAPTURE");
                     }
                     ret=EINVAL;
                     break;
                 }

                 if (fmt->index>=dev->vs_formats[subdev])
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: index %d, while %d format(s) available", fmt->index, dev->vs_formats[subdev]);
                     }
                     ret=EINVAL;
                     break;
                 }
                 fmt->reserved[0]=0;
                 fmt->reserved[1]=0;
                 fmt->reserved[2]=0;
                 fmt->reserved[3]=0;

                 switch(dev->vs_format[subdev][fmt->index])
                 {
                     case UVC_FORMAT_YUY2:
                          fmt->pixelformat=V4L2_PIX_FMT_YUYV;
                          fmt->flags=0;
                          strncpy((char*)fmt->description, "YUV 4:2:2 (YUY2/YUYV)", sizeof(fmt->description));
                          break;
                     case UVC_FORMAT_UYVY:
                          fmt->pixelformat=V4L2_PIX_FMT_UYVY;
                          fmt->flags=0; /* V4L2_FMT_FLAG_EMULATED */
                          /* Emulation flag looks reasonable, but v4l2_compliance claims about   */
                          /* this is a wrong flag for a driver, so it is better to strip it out. */
                          strncpy((char*)fmt->description, "YUV 4:2:2 (UYVY)", sizeof(fmt->description));
                          break;
                     case UVC_FORMAT_YVYU:
                          fmt->pixelformat=V4L2_PIX_FMT_YVYU;
                          fmt->flags=0; /* V4L2_FMT_FLAG_EMULATED */
                          strncpy((char*)fmt->description, "YUV 4:2:2 (YVYU)", sizeof(fmt->description));
                          break;
                     case UVC_FORMAT_VYUY:
                          fmt->pixelformat=V4L2_PIX_FMT_VYUY;
                          fmt->flags=0; /* V4L2_FMT_FLAG_EMULATED */
                          strncpy((char*)fmt->description, "YUV 4:2:2 (VYUY)", sizeof(fmt->description));
                          break;
                     case UVC_FORMAT_NV12:
                          fmt->pixelformat=V4L2_PIX_FMT_NV12;
                          fmt->flags=0;
                          strncpy((char*)fmt->description, "YUV 4:2:0 (NV12)", sizeof(fmt->description));
                          break;
                     case UVC_FORMAT_MJPG:
                          fmt->pixelformat=V4L2_PIX_FMT_MJPEG;
                          fmt->flags=V4L2_FMT_FLAG_COMPRESSED;
                          strncpy((char*)fmt->description, "MJPEG", sizeof(fmt->description));
                          break;
                     case UVC_FORMAT_H264:
                     case UVC_FORMAT_H264F:
                          fmt->pixelformat=V4L2_PIX_FMT_H264;
                          fmt->flags=V4L2_FMT_FLAG_COMPRESSED;
                          strncpy((char*)fmt->description, "H.264", sizeof(fmt->description));
                          break;
                     case UVC_FORMAT_DV:
                          fmt->pixelformat=V4L2_PIX_FMT_DV;
                          fmt->flags=V4L2_FMT_FLAG_COMPRESSED;
                          strncpy((char*)fmt->description, "DV", sizeof(fmt->description));
                          break;
                     case UVC_FORMAT_MPEG2TS:
                          fmt->pixelformat=V4L2_PIX_FMT_MPEG2;
                          fmt->flags=V4L2_FMT_FLAG_COMPRESSED;
                          strncpy((char*)fmt->description, "MPEG2 TS", sizeof(fmt->description));
                          break;
                     default:
                          fmt->pixelformat=v4l2_fourcc('B', 'A', 'D', 'F');
                          fmt->flags=0;
                          strncpy((char*)fmt->description, "Unsupported format, please report.", sizeof(fmt->description));
                          break;
                 }

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        pixelformat: %08X", fmt->pixelformat);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        flags: %08X", fmt->flags);
                 }

                 dctldatasize=sizeof(*fmt);
             }
             break;
        case VIDIOC_G_FMT:
             {
                 struct v4l2_format* fmt;
                 vs_color_format_t* color_format=NULL;

                 fmt=(struct v4l2_format*)dptr;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_G_FMT:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        type: %08X", fmt->type);
                 }

                 if (fmt->type!=V4L2_BUF_TYPE_VIDEO_CAPTURE)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: buffer type is not V4L2_BUF_TYPE_VIDEO_CAPTURE");
                     }
                     ret=EINVAL;
                     break;
                 }
                 memset(fmt, 0x00, sizeof(*fmt));

                 fmt->type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
                 fmt->fmt.pix.width=dev->current_width[subdev];
                 fmt->fmt.pix.height=dev->current_height[subdev];
                 fmt->fmt.pix.pixelformat=dev->current_pixelformat[subdev];
                 fmt->fmt.pix.field=V4L2_FIELD_NONE;
                 fmt->fmt.pix.bytesperline=dev->current_stride[subdev];
                 fmt->fmt.pix.sizeimage=dev->current_stride[subdev] * dev->current_height[subdev];

                 switch (fmt->fmt.pix.pixelformat)
                 {
                     case V4L2_PIX_FMT_YUYV:
                     case V4L2_PIX_FMT_NV12:
                          color_format=&dev->vs_color_format_uncompressed[subdev];
                          break;
                     case V4L2_PIX_FMT_MJPEG:
                          color_format=&dev->vs_color_format_mjpeg[subdev];
                          break;
                     case V4L2_PIX_FMT_H264:
                          color_format=&dev->vs_color_format_h264f[subdev];
                          break;
                     case V4L2_PIX_FMT_UYVY:
                     case V4L2_PIX_FMT_YVYU:
                     case V4L2_PIX_FMT_VYUY:
                          if (uvc_emulation)
                          {
                              color_format=&dev->vs_color_format_uncompressed[subdev];
                              break;
                          }
                          /* fall-through */
                     default:
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: internal error, unsupported format, please report");
                          }
                          ret=EINVAL;
                          break;
                 }

                 if (ret!=EOK)
                 {
                     break;
                 }

                 switch(color_format->bMatrixCoefficients)
                 {
                     case VCFMC_UNKNOWN:
                          fmt->fmt.pix.colorspace=V4L2_COLORSPACE_SRGB;
                          break;
                     case VCFMC_BT_709:
                          fmt->fmt.pix.colorspace=V4L2_COLORSPACE_REC709;
                          break;
                     case VCFMC_FCC:
                          fmt->fmt.pix.colorspace=V4L2_COLORSPACE_SMPTE170M;
                          break;
                     case VCFMC_BT_470_2_BG:
                          fmt->fmt.pix.colorspace=V4L2_COLORSPACE_470_SYSTEM_BG;
                          break;
                     case VCFMC_SMPTE_170M:
                          fmt->fmt.pix.colorspace=V4L2_COLORSPACE_SMPTE170M;
                          break;
                     case VCFMC_SMPTE_240M:
                          fmt->fmt.pix.colorspace=V4L2_COLORSPACE_SMPTE240M;
                          break;
                     default:
                          fmt->fmt.pix.colorspace=V4L2_COLORSPACE_SRGB;
                          break;
                 }

                 fmt->fmt.pix.priv=0;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        fmt.pix.width: %d", fmt->fmt.pix.width);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        fmt.pix.height: %d", fmt->fmt.pix.height);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        fmt.pix.pixelformat: %08X", fmt->fmt.pix.pixelformat);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        fmt.pix.field: %08X", fmt->fmt.pix.field);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        fmt.pix.bytesperline: %d", fmt->fmt.pix.bytesperline);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        fmt.pix.sizeimage: %d", fmt->fmt.pix.sizeimage);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        fmt.pix.colorspace: %08X", fmt->fmt.pix.colorspace);
                 }

                 dctldatasize=sizeof(*fmt);
             }
             break;
        case VIDIOC_S_FMT:
             {
                 struct v4l2_format* fmt;
                 vs_color_format_t* color_format=NULL;
                 int bpp=0;
                 int best_width=INT_MAX;
                 int best_height=INT_MAX;
                 int suggest_new_format=0;
                 int format_to_search=0;
                 unsigned int frameinterval=0;

                 fmt=(struct v4l2_format*)dptr;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_S_FMT:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        type: %08X", fmt->type);
                 }

                 if (fmt->type!=V4L2_BUF_TYPE_VIDEO_CAPTURE)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: buffer type is not V4L2_BUF_TYPE_VIDEO_CAPTURE");
                     }
                     ret=EINVAL;
                     break;
                 }

                 if ((dev->current_transfer[subdev]) || (dev->current_buffer_fds[subdev]!=-1))
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EBUSY: transfer is active");
                     }
                     ret=EBUSY;
                     break;
                 }

                 switch (fmt->fmt.pix.pixelformat)
                 {
                     case V4L2_PIX_FMT_YUYV:
                          format_to_search=UVC_FORMAT_YUY2;
                          break;
                     case V4L2_PIX_FMT_NV12:
                          format_to_search=UVC_FORMAT_NV12;
                          break;
                     case V4L2_PIX_FMT_MJPEG:
                          format_to_search=UVC_FORMAT_MJPG;
                          break;
                     case V4L2_PIX_FMT_H264:
                          format_to_search=UVC_FORMAT_H264F;
                          break;
                     case V4L2_PIX_FMT_UYVY:
                          if (uvc_emulation)
                          {
                              format_to_search=UVC_FORMAT_UYVY;
                              break;
                          }
                          suggest_new_format=1;
                          break;
                     case V4L2_PIX_FMT_YVYU:
                          if (uvc_emulation)
                          {
                              format_to_search=UVC_FORMAT_YVYU;
                              break;
                          }
                          suggest_new_format=1;
                          break;
                     case V4L2_PIX_FMT_VYUY:
                          if (uvc_emulation)
                          {
                              format_to_search=UVC_FORMAT_VYUY;
                              break;
                          }
                          suggest_new_format=1;
                          break;
                     default:
                          suggest_new_format=1;
                          break;
                 }

                 if ((suggest_new_format==0) && (format_to_search!=0))
                 {
                     suggest_new_format=1;
                     for (it=0; it<dev->vs_formats[subdev]; it++)
                     {
                         if (dev->vs_format[subdev][it]==format_to_search)
                         {
                             suggest_new_format=0;
                         }
                     }
                 }

                 /* Suggest new pixel format if current is not supported */
                 if (suggest_new_format)
                 {
                      switch (dev->vs_format[subdev][0])
                      {
                          case UVC_FORMAT_YUY2:
                               fmt->fmt.pix.pixelformat=V4L2_PIX_FMT_YUYV;
                               break;
                          case UVC_FORMAT_UYVY:
                               fmt->fmt.pix.pixelformat=V4L2_PIX_FMT_UYVY;
                               break;
                          case UVC_FORMAT_VYUY:
                               fmt->fmt.pix.pixelformat=V4L2_PIX_FMT_VYUY;
                               break;
                          case UVC_FORMAT_YVYU:
                               fmt->fmt.pix.pixelformat=V4L2_PIX_FMT_YVYU;
                               break;
                          case UVC_FORMAT_NV12:
                               fmt->fmt.pix.pixelformat=V4L2_PIX_FMT_NV12;
                               break;
                          case UVC_FORMAT_MJPG:
                               fmt->fmt.pix.pixelformat=V4L2_PIX_FMT_MJPEG;
                               break;
                          case UVC_FORMAT_H264:
                          case UVC_FORMAT_H264F:
                               fmt->fmt.pix.pixelformat=V4L2_PIX_FMT_H264;
                               break;
                      }
                 }

                 /* Check if selected resolution is available for selected pixel format */
                 match=0;
                 switch (fmt->fmt.pix.pixelformat)
                 {
                     case V4L2_PIX_FMT_YUYV:
                     case V4L2_PIX_FMT_UYVY:
                     case V4L2_PIX_FMT_YVYU:
                     case V4L2_PIX_FMT_VYUY:
                     case V4L2_PIX_FMT_NV12:
                          if (fmt->fmt.pix.pixelformat==V4L2_PIX_FMT_NV12)
                          {
                              bpp=1;
                          }
                          if ((fmt->fmt.pix.pixelformat==V4L2_PIX_FMT_YUYV) ||
                              (fmt->fmt.pix.pixelformat==V4L2_PIX_FMT_UYVY) ||
                              (fmt->fmt.pix.pixelformat==V4L2_PIX_FMT_YVYU) ||
                              (fmt->fmt.pix.pixelformat==V4L2_PIX_FMT_VYUY))
                          {
                              bpp=2;
                          }
                          color_format=&dev->vs_color_format_uncompressed[subdev];

                          for (it=0; it<dev->vs_format_uncompressed[subdev].bNumFrameDescriptors; it++)
                          {
                              if ((dev->vs_frame_uncompressed[subdev][it].wWidth==fmt->fmt.pix.width) &&
                                  (dev->vs_frame_uncompressed[subdev][it].wHeight==fmt->fmt.pix.height))
                              {
                                  frameinterval=dev->vs_frame_uncompressed[subdev][it].dwDefaultFrameInterval;
                                  match=1;
                                  break;
                              }
                          }
                          if (match)
                          {
                              break;
                          }

                          /* Suggest new video mode, close to desired by width */
                          for (it=0; it<dev->vs_format_uncompressed[subdev].bNumFrameDescriptors; it++)
                          {
                              if ((dev->vs_frame_uncompressed[subdev][it].wWidth-fmt->fmt.pix.width)<
                                  (best_width-fmt->fmt.pix.width))
                              {
                                  best_width=dev->vs_frame_uncompressed[subdev][it].wWidth;
                                  best_height=dev->vs_frame_uncompressed[subdev][it].wHeight;
                                  frameinterval=dev->vs_frame_uncompressed[subdev][it].dwDefaultFrameInterval;
                              }
                          }
                          fmt->fmt.pix.width=best_width;
                          fmt->fmt.pix.height=best_height;
                          break;
                     case V4L2_PIX_FMT_MJPEG:
                          bpp=3;
                          color_format=&dev->vs_color_format_mjpeg[subdev];
                          for (it=0; it<dev->vs_format_mjpeg[subdev].bNumFrameDescriptors; it++)
                          {
                              if ((dev->vs_frame_mjpeg[subdev][it].wWidth==fmt->fmt.pix.width) &&
                                  (dev->vs_frame_mjpeg[subdev][it].wHeight==fmt->fmt.pix.height))
                              {
                                  frameinterval=dev->vs_frame_mjpeg[subdev][it].dwDefaultFrameInterval;
                                  match=1;
                                  break;
                              }
                          }
                          if (match)
                          {
                              break;
                          }
                          /* Suggest new video mode, close to desired by width */
                          for (it=0; it<dev->vs_format_mjpeg[subdev].bNumFrameDescriptors; it++)
                          {
                              if ((dev->vs_frame_mjpeg[subdev][it].wWidth-fmt->fmt.pix.width)<
                                  (best_width-fmt->fmt.pix.width))
                              {
                                  best_width=dev->vs_frame_mjpeg[subdev][it].wWidth;
                                  best_height=dev->vs_frame_mjpeg[subdev][it].wHeight;
                                  frameinterval=dev->vs_frame_mjpeg[subdev][it].dwDefaultFrameInterval;
                              }
                          }
                          fmt->fmt.pix.width=best_width;
                          fmt->fmt.pix.height=best_height;
                          break;
                     case V4L2_PIX_FMT_H264:
                          bpp=3;
                          color_format=&dev->vs_color_format_h264f[subdev];
                          for (it=0; it<dev->vs_format_h264f[subdev].bNumFrameDescriptors; it++)
                          {
                              if ((dev->vs_frame_h264f[subdev][it].wWidth==fmt->fmt.pix.width) &&
                                  (dev->vs_frame_h264f[subdev][it].wHeight==fmt->fmt.pix.height))
                              {
                                  frameinterval=dev->vs_frame_h264f[subdev][it].dwDefaultFrameInterval;
                                  match=1;
                                  break;
                              }
                          }
                          if (match)
                          {
                              break;
                          }
                          /* Suggest new video mode, close to desired by width */
                          for (it=0; it<dev->vs_format_h264f[subdev].bNumFrameDescriptors; it++)
                          {
                              if ((dev->vs_frame_h264f[subdev][it].wWidth-fmt->fmt.pix.width)<
                                  (best_width-fmt->fmt.pix.width))
                              {
                                  best_width=dev->vs_frame_h264f[subdev][it].wWidth;
                                  best_height=dev->vs_frame_h264f[subdev][it].wHeight;
                                  frameinterval=dev->vs_frame_h264f[subdev][it].dwDefaultFrameInterval;
                              }
                          }
                          fmt->fmt.pix.width=best_width;
                          fmt->fmt.pix.height=best_height;
                          break;
                 }

                 /* Setup rest of fields */
                 fmt->fmt.pix.field=V4L2_FIELD_NONE;
                 if (!((match) && (fmt->fmt.pix.bytesperline!=0) &&
                     (fmt->fmt.pix.bytesperline>fmt->fmt.pix.width*bpp)))
                 {
                     fmt->fmt.pix.bytesperline=fmt->fmt.pix.width*bpp;
                 }
                 fmt->fmt.pix.sizeimage=fmt->fmt.pix.bytesperline * fmt->fmt.pix.height;

                 /* Always set hardware colorspace */
                 switch(color_format->bMatrixCoefficients)
                 {
                     case VCFMC_UNKNOWN:
                          fmt->fmt.pix.colorspace=V4L2_COLORSPACE_SRGB;
                          break;
                     case VCFMC_BT_709:
                          fmt->fmt.pix.colorspace=V4L2_COLORSPACE_REC709;
                          break;
                     case VCFMC_FCC:
                          fmt->fmt.pix.colorspace=V4L2_COLORSPACE_SMPTE170M;
                          break;
                     case VCFMC_BT_470_2_BG:
                          fmt->fmt.pix.colorspace=V4L2_COLORSPACE_470_SYSTEM_BG;
                          break;
                     case VCFMC_SMPTE_170M:
                          fmt->fmt.pix.colorspace=V4L2_COLORSPACE_SMPTE170M;
                          break;
                     case VCFMC_SMPTE_240M:
                          fmt->fmt.pix.colorspace=V4L2_COLORSPACE_SMPTE240M;
                          break;
                     default:
                          fmt->fmt.pix.colorspace=V4L2_COLORSPACE_SRGB;
                          break;
                 }

                 /* Store negotiated parameters */
                 dev->current_width[subdev]=fmt->fmt.pix.width;
                 dev->current_height[subdev]=fmt->fmt.pix.height;
                 dev->current_pixelformat[subdev]=fmt->fmt.pix.pixelformat;
                 dev->current_stride[subdev]=fmt->fmt.pix.bytesperline;
                 fmt->fmt.pix.priv=0;

                 /* Now reset frame interval to default */
                 dev->current_frameinterval[subdev]=frameinterval;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        fmt.pix.width: %d", fmt->fmt.pix.width);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        fmt.pix.height: %d", fmt->fmt.pix.height);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        fmt.pix.pixelformat: %08X", fmt->fmt.pix.pixelformat);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        fmt.pix.field: %08X", fmt->fmt.pix.field);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        fmt.pix.bytesperline: %d", fmt->fmt.pix.bytesperline);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        fmt.pix.sizeimage: %d", fmt->fmt.pix.sizeimage);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        fmt.pix.colorspace: %08X", fmt->fmt.pix.colorspace);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        frame interval reset to: %d", frameinterval);
                 }

                 dctldatasize=sizeof(*fmt);
             }
             break;
        case VIDIOC_TRY_FMT:
             {
                 struct v4l2_format* fmt;
                 vs_color_format_t* color_format=NULL;
                 int bpp=0;
                 int best_width=INT_MAX;
                 int best_height=INT_MAX;
                 int suggest_new_format=0;
                 int format_to_search=0;

                 fmt=(struct v4l2_format*)dptr;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_TRY_FMT:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        type: %08X", fmt->type);
                 }

                 if (fmt->type!=V4L2_BUF_TYPE_VIDEO_CAPTURE)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: buffer type is not V4L2_BUF_TYPE_VIDEO_CAPTURE");
                     }
                     ret=EINVAL;
                     break;
                 }

                 switch (fmt->fmt.pix.pixelformat)
                 {
                     case V4L2_PIX_FMT_YUYV:
                          format_to_search=UVC_FORMAT_YUY2;
                          break;
                     case V4L2_PIX_FMT_NV12:
                          format_to_search=UVC_FORMAT_NV12;
                          break;
                     case V4L2_PIX_FMT_MJPEG:
                          format_to_search=UVC_FORMAT_MJPG;
                          break;
                     case V4L2_PIX_FMT_H264:
                          format_to_search=UVC_FORMAT_H264F;
                          break;
                     case V4L2_PIX_FMT_UYVY:
                          if (uvc_emulation)
                          {
                              format_to_search=UVC_FORMAT_UYVY;
                              break;
                          }
                          suggest_new_format=1;
                          break;
                     case V4L2_PIX_FMT_YVYU:
                          if (uvc_emulation)
                          {
                              format_to_search=UVC_FORMAT_YVYU;
                              break;
                          }
                          suggest_new_format=1;
                          break;
                     case V4L2_PIX_FMT_VYUY:
                          if (uvc_emulation)
                          {
                              format_to_search=UVC_FORMAT_VYUY;
                              break;
                          }
                          suggest_new_format=1;
                          break;
                     default:
                          suggest_new_format=1;
                          break;
                 }

                 if ((suggest_new_format==0) && (format_to_search!=0))
                 {
                     suggest_new_format=1;
                     for (it=0; it<dev->vs_formats[subdev]; it++)
                     {
                         if (dev->vs_format[subdev][it]==format_to_search)
                         {
                             suggest_new_format=0;
                         }
                     }
                 }

                 /* Suggest new pixel format if current is not supported */
                 if (suggest_new_format)
                 {
                      switch (dev->vs_format[subdev][0])
                      {
                          case UVC_FORMAT_YUY2:
                               fmt->fmt.pix.pixelformat=V4L2_PIX_FMT_YUYV;
                               break;
                          case UVC_FORMAT_UYVY:
                               fmt->fmt.pix.pixelformat=V4L2_PIX_FMT_UYVY;
                               break;
                          case UVC_FORMAT_VYUY:
                               fmt->fmt.pix.pixelformat=V4L2_PIX_FMT_VYUY;
                               break;
                          case UVC_FORMAT_YVYU:
                               fmt->fmt.pix.pixelformat=V4L2_PIX_FMT_YVYU;
                               break;
                          case UVC_FORMAT_NV12:
                               fmt->fmt.pix.pixelformat=V4L2_PIX_FMT_NV12;
                               break;
                          case UVC_FORMAT_MJPG:
                               fmt->fmt.pix.pixelformat=V4L2_PIX_FMT_MJPEG;
                               break;
                          case UVC_FORMAT_H264:
                          case UVC_FORMAT_H264F:
                               fmt->fmt.pix.pixelformat=V4L2_PIX_FMT_H264;
                               break;
                      }
                 }


                 /* Check if selected resolution is available for selected pixel format */
                 match=0;
                 switch (fmt->fmt.pix.pixelformat)
                 {
                     case V4L2_PIX_FMT_YUYV:
                     case V4L2_PIX_FMT_UYVY:
                     case V4L2_PIX_FMT_YVYU:
                     case V4L2_PIX_FMT_VYUY:
                     case V4L2_PIX_FMT_NV12:
                          if (fmt->fmt.pix.pixelformat==V4L2_PIX_FMT_NV12)
                          {
                              bpp=1;
                          }
                          if ((fmt->fmt.pix.pixelformat==V4L2_PIX_FMT_YUYV) ||
                              (fmt->fmt.pix.pixelformat==V4L2_PIX_FMT_UYVY) ||
                              (fmt->fmt.pix.pixelformat==V4L2_PIX_FMT_VYUY) ||
                              (fmt->fmt.pix.pixelformat==V4L2_PIX_FMT_YVYU))
                          {
                              bpp=2;
                          }
                          color_format=&dev->vs_color_format_uncompressed[subdev];

                          for (it=0; it<dev->vs_format_uncompressed[subdev].bNumFrameDescriptors; it++)
                          {
                              if ((dev->vs_frame_uncompressed[subdev][it].wWidth==fmt->fmt.pix.width) &&
                                  (dev->vs_frame_uncompressed[subdev][it].wHeight==fmt->fmt.pix.height))
                              {
                                  match=1;
                                  break;
                              }
                          }
                          if (match)
                          {
                              break;
                          }

                          /* Suggest new video mode, close to desired by width */
                          for (it=0; it<dev->vs_format_uncompressed[subdev].bNumFrameDescriptors; it++)
                          {
                              if ((dev->vs_frame_uncompressed[subdev][it].wWidth-fmt->fmt.pix.width)<
                                  (best_width-fmt->fmt.pix.width))
                              {
                                  best_width=dev->vs_frame_uncompressed[subdev][it].wWidth;
                                  best_height=dev->vs_frame_uncompressed[subdev][it].wHeight;
                              }
                          }
                          fmt->fmt.pix.width=best_width;
                          fmt->fmt.pix.height=best_height;
                          break;
                     case V4L2_PIX_FMT_MJPEG:
                          bpp=3;
                          color_format=&dev->vs_color_format_mjpeg[subdev];
                          for (it=0; it<dev->vs_format_mjpeg[subdev].bNumFrameDescriptors; it++)
                          {
                              if ((dev->vs_frame_mjpeg[subdev][it].wWidth==fmt->fmt.pix.width) &&
                                  (dev->vs_frame_mjpeg[subdev][it].wHeight==fmt->fmt.pix.height))
                              {
                                  match=1;
                                  break;
                              }
                          }
                          if (match)
                          {
                              break;
                          }
                          /* Suggest new video mode, close to desired by width */
                          for (it=0; it<dev->vs_format_mjpeg[subdev].bNumFrameDescriptors; it++)
                          {
                              if ((dev->vs_frame_mjpeg[subdev][it].wWidth-fmt->fmt.pix.width)<
                                  (best_width-fmt->fmt.pix.width))
                              {
                                  best_width=dev->vs_frame_mjpeg[subdev][it].wWidth;
                                  best_height=dev->vs_frame_mjpeg[subdev][it].wHeight;
                              }
                          }
                          fmt->fmt.pix.width=best_width;
                          fmt->fmt.pix.height=best_height;
                          break;
                     case V4L2_PIX_FMT_H264:
                          bpp=3;
                          color_format=&dev->vs_color_format_h264f[subdev];
                          for (it=0; it<dev->vs_format_h264f[subdev].bNumFrameDescriptors; it++)
                          {
                              if ((dev->vs_frame_h264f[subdev][it].wWidth==fmt->fmt.pix.width) &&
                                  (dev->vs_frame_h264f[subdev][it].wHeight==fmt->fmt.pix.height))
                              {
                                  match=1;
                                  break;
                              }
                          }
                          if (match)
                          {
                              break;
                          }
                          /* Suggest new video mode, close to desired by width */
                          for (it=0; it<dev->vs_format_h264f[subdev].bNumFrameDescriptors; it++)
                          {
                              if ((dev->vs_frame_h264f[subdev][it].wWidth-fmt->fmt.pix.width)<
                                  (best_width-fmt->fmt.pix.width))
                              {
                                  best_width=dev->vs_frame_h264f[subdev][it].wWidth;
                                  best_height=dev->vs_frame_h264f[subdev][it].wHeight;
                              }
                          }
                          fmt->fmt.pix.width=best_width;
                          fmt->fmt.pix.height=best_height;
                          break;
                 }

                 /* Setup rest of fields */
                 fmt->fmt.pix.field=V4L2_FIELD_NONE;
                 if (!((match) && (fmt->fmt.pix.bytesperline!=0) &&
                     (fmt->fmt.pix.bytesperline>fmt->fmt.pix.width*bpp)))
                 {
                     fmt->fmt.pix.bytesperline=fmt->fmt.pix.width*bpp;
                 }
                 fmt->fmt.pix.sizeimage=fmt->fmt.pix.bytesperline*fmt->fmt.pix.height;

                 /* Always set hardware colorspace */
                 switch(color_format->bMatrixCoefficients)
                 {
                     case VCFMC_UNKNOWN:
                          fmt->fmt.pix.colorspace=V4L2_COLORSPACE_SRGB;
                          break;
                     case VCFMC_BT_709:
                          fmt->fmt.pix.colorspace=V4L2_COLORSPACE_REC709;
                          break;
                     case VCFMC_FCC:
                          fmt->fmt.pix.colorspace=V4L2_COLORSPACE_SMPTE170M;
                          break;
                     case VCFMC_BT_470_2_BG:
                          fmt->fmt.pix.colorspace=V4L2_COLORSPACE_470_SYSTEM_BG;
                          break;
                     case VCFMC_SMPTE_170M:
                          fmt->fmt.pix.colorspace=V4L2_COLORSPACE_SMPTE170M;
                          break;
                     case VCFMC_SMPTE_240M:
                          fmt->fmt.pix.colorspace=V4L2_COLORSPACE_SMPTE240M;
                          break;
                     default:
                          fmt->fmt.pix.colorspace=V4L2_COLORSPACE_SRGB;
                          break;
                 }

                 fmt->fmt.pix.priv=0;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        fmt.pix.width: %d", fmt->fmt.pix.width);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        fmt.pix.height: %d", fmt->fmt.pix.height);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        fmt.pix.pixelformat: %08X", fmt->fmt.pix.pixelformat);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        fmt.pix.field: %08X", fmt->fmt.pix.field);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        fmt.pix.bytesperline: %d", fmt->fmt.pix.bytesperline);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        fmt.pix.sizeimage: %d", fmt->fmt.pix.sizeimage);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        fmt.pix.colorspace: %08X", fmt->fmt.pix.colorspace);
                 }

                 dctldatasize=sizeof(*fmt);
             }
             break;
        case VIDIOC_CROPCAP:
             {
                 struct v4l2_cropcap* cap;

                 cap=(struct v4l2_cropcap*)dptr;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_CROPCAP:");
                 }

                 if (cap->type!=V4L2_BUF_TYPE_VIDEO_CAPTURE)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: buffer type is not V4L2_BUF_TYPE_VIDEO_CAPTURE");
                     }
                     ret=EINVAL;
                     break;
                 }

                 cap->bounds.left=0;
                 cap->bounds.top=0;
                 cap->bounds.width=dev->current_width[subdev];
                 cap->bounds.height=dev->current_height[subdev];

                 cap->defrect.left=0;
                 cap->defrect.top=0;
                 cap->defrect.width=dev->current_width[subdev];
                 cap->defrect.height=dev->current_height[subdev];

                 cap->pixelaspect.numerator=1;
                 cap->pixelaspect.denominator=1;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        bounds.left: %d", cap->bounds.left);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        bounds.top: %d", cap->bounds.top);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        bounds.width: %d", cap->bounds.width);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        bounds.height: %d", cap->bounds.height);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        defrect.left: %d", cap->defrect.left);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        defrect.top: %d", cap->defrect.top);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        defrect.width: %d", cap->defrect.width);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        defrect.height: %d", cap->defrect.height);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        pixelaspect.numerator: %d", cap->pixelaspect.numerator);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        pixelaspect.denominator: %d", cap->pixelaspect.denominator);
                 }

                 dctldatasize=sizeof(*cap);
             }
             break;
        case VIDIOC_G_CROP:
             {
                 struct v4l2_crop* crop;

                 crop=(struct v4l2_crop*)dptr;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_G_CROP:");
                 }

                 if (crop->type!=V4L2_BUF_TYPE_VIDEO_CAPTURE)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: buffer type is not V4L2_BUF_TYPE_VIDEO_CAPTURE");
                     }
                     ret=EINVAL;
                     break;
                 }
                 crop->c.left=0;
                 crop->c.top=0;
                 crop->c.width=dev->current_width[subdev];
                 crop->c.height=dev->current_height[subdev];

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        c.left: %d", crop->c.left);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        c.top: %d", crop->c.top);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        c.width: %d", crop->c.width);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        c.height: %d", crop->c.height);
                 }

                 dctldatasize=sizeof(*crop);
             }
             break;
        case VIDIOC_S_CROP:
             {
                 struct v4l2_crop* crop;

                 crop=(struct v4l2_crop*)dptr;
                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_S_CROP:");
                 }
                 if (crop->type!=V4L2_BUF_TYPE_VIDEO_CAPTURE)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: buffer type is not V4L2_BUF_TYPE_VIDEO_CAPTURE");
                     }
                     ret=EINVAL;
                     break;
                 }

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        c.left: %d", crop->c.left);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        c.top: %d", crop->c.top);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        c.width: %d", crop->c.width);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        c.height: %d", crop->c.height);
                 }

                 if ((crop->c.left!=0) || (crop->c.top!=0) ||
                     (crop->c.width!=dev->current_width[subdev]) ||
                     (crop->c.height!=dev->current_height[subdev]))
                 {
                     ret=EINVAL;
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: invalid dimensions");
                     }
                     break;
                 }
             }
             break;
        case VIDIOC_ENUMINPUT:
             {
                 struct v4l2_input* input;

                 input=(struct v4l2_input*)dptr;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_ENUMINPUT:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        index: %d", input->index);
                 }

                 if (input->index>0)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: index too high, 0 is only supported");
                     }
                     ret=EINVAL;
                     break;
                 }

                 memset(input, 0x00, sizeof(*input));
                 strncpy((char*)input->name, "Default video input", sizeof(input->name));
                 input->type=V4L2_INPUT_TYPE_CAMERA;
                 input->audioset=0;
                 input->tuner=0;
                 input->status=0;
                 if (dev->vc_processing_unit.bmVideoStandards)
                 {
                     input->capabilities=V4L2_IN_CAP_STD;
                     input->std=0;
                     if (dev->vc_processing_unit.bmVideoStandards & VPU_VSTD_NTSC_525_60)
                     {
                         input->std|=V4L2_STD_NTSC;
                     }
                     if (dev->vc_processing_unit.bmVideoStandards & VPU_VSTD_PAL_625_50)
                     {
                         input->std|=V4L2_STD_PAL;
                     }
                     if (dev->vc_processing_unit.bmVideoStandards & VPU_VSTD_SECAM_625_50)
                     {
                         input->std|=V4L2_STD_SECAM;
                     }
                     if (dev->vc_processing_unit.bmVideoStandards & VPU_VSTD_NTSC_625_50)
                     {
                         input->std|=V4L2_STD_NTSC;
                     }
                     if (dev->vc_processing_unit.bmVideoStandards & VPU_VSTD_PAL_525_60)
                     {
                         input->std|=V4L2_STD_PAL_60;
                     }
                 }
                 else
                 {
                     input->capabilities=0;
                     input->std=0;
                 }

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        name: %s", input->name);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        type: %08X", input->type);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        audioset: %08X", input->audioset);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        tuner: %08X", input->tuner);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        std: %08X", (unsigned int)input->std);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        status: %08X", input->status);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        capabilities: %08X", input->capabilities);
                 }

                 dctldatasize=sizeof(*input);
             }
             break;
        case VIDIOC_G_INPUT:
             {
                 int* input;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_G_INPUT:");
                 }

                 input=(int*)dptr;
                 *input=0;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        input: %d", *input);
                 }

                 dctldatasize=sizeof(int);
             }
             break;
        case VIDIOC_S_INPUT:
             {
                 int* input;

                 input=(int*)dptr;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_S_INPUT:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        input: %d", *input);
                 }

                 if ((*input>0) || (*input<0))
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: input index too high, 0 is only supported");
                     }
                     ret=EINVAL;
                     break;
                 }

                 dctldatasize=sizeof(int);
             }
             break;
        case VIDIOC_ENUMAUDIO:
             {
                 ret=EINVAL;
                 break;
             }
             break;
        case VIDIOC_ENUM_FRAMESIZES:
             {
                 struct v4l2_frmsizeenum* frm;

                 frm=(struct v4l2_frmsizeenum*)dptr;
                 frm->reserved[0]=0;
                 frm->reserved[1]=0;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_ENUM_FRAMESIZES:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        index: %d", frm->index);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        pixel_format: %08X", frm->pixel_format);
                 }

                 /* Check for supported format and index in range */
                 ret=EINVAL;
                 switch (frm->pixel_format)
                 {
                     case V4L2_PIX_FMT_YUYV:
                     case V4L2_PIX_FMT_UYVY:
                     case V4L2_PIX_FMT_VYUY:
                     case V4L2_PIX_FMT_YVYU:
                          if (((frm->pixel_format==V4L2_PIX_FMT_UYVY) ||
                               (frm->pixel_format==V4L2_PIX_FMT_YVYU) ||
                               (frm->pixel_format==V4L2_PIX_FMT_VYUY)) && (!uvc_emulation))
                          {
                              if (uvc_verbose>2)
                              {
                                  slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: unsupported pixel format");
                              }
                              break;
                          }
                          for (it=0; it<dev->vs_formats[subdev]; it++)
                          {
                              if ((dev->vs_format[subdev][it]==UVC_FORMAT_YUY2) ||
                                  (dev->vs_format[subdev][it]==UVC_FORMAT_UYVY) ||
                                  (dev->vs_format[subdev][it]==UVC_FORMAT_YVYU) ||
                                  (dev->vs_format[subdev][it]==UVC_FORMAT_VYUY))
                              {
                                  ret=EOK;
                                  if (frm->index>=dev->vs_format_uncompressed[subdev].bNumFrameDescriptors)
                                  {
                                      if (uvc_verbose>2)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: index higher than amount of frame descriptors");
                                      }
                                      ret=EINVAL;
                                      break;
                                  }
                                  break;
                              }
                          }
                          break;
                     case V4L2_PIX_FMT_NV12:
                          for (it=0; it<dev->vs_formats[subdev]; it++)
                          {
                              if (dev->vs_format[subdev][it]==UVC_FORMAT_NV12)
                              {
                                  ret=EOK;
                                  if (frm->index>=dev->vs_format_uncompressed[subdev].bNumFrameDescriptors)
                                  {
                                      if (uvc_verbose>2)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: index higher than amount of frame descriptors");
                                      }
                                      ret=EINVAL;
                                      break;
                                  }
                                  break;
                              }
                          }
                          break;
                     case V4L2_PIX_FMT_MJPEG:
                          for (it=0; it<dev->vs_formats[subdev]; it++)
                          {
                              if (dev->vs_format[subdev][it]==UVC_FORMAT_MJPG)
                              {
                                  ret=EOK;
                                  if (frm->index>=dev->vs_format_mjpeg[subdev].bNumFrameDescriptors)
                                  {
                                      if (uvc_verbose>2)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: index higher than amount of frame descriptors");
                                      }
                                      ret=EINVAL;
                                      break;
                                  }
                                  break;
                              }
                          }
                          break;
                     case V4L2_PIX_FMT_H264:
                          for (it=0; it<dev->vs_formats[subdev]; it++)
                          {
                              if ((dev->vs_format[subdev][it]==UVC_FORMAT_H264) ||
                                  (dev->vs_format[subdev][it]==UVC_FORMAT_H264F))
                              {
                                  ret=EOK;
                                  if (frm->index>=dev->vs_format_h264f[subdev].bNumFrameDescriptors)
                                  {
                                      if (uvc_verbose>2)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: index higher than amount of frame descriptors");
                                      }
                                      ret=EINVAL;
                                      break;
                                  }
                                  break;
                              }
                          }
                          break;
                     default:
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: unsupported pixel format");
                          }
                          ret=EINVAL;
                          break;
                 }

                 if (ret!=EOK)
                 {
                      if (uvc_verbose>2)
                      {
                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: pixel format can't be found");
                      }
                      break;
                 }

                 /* Fetch frame dimensions by specified index */
                 frm->type=V4L2_FRMSIZE_TYPE_DISCRETE;
                 switch (frm->pixel_format)
                 {
                     case V4L2_PIX_FMT_YUYV:
                     case V4L2_PIX_FMT_UYVY:
                     case V4L2_PIX_FMT_VYUY:
                     case V4L2_PIX_FMT_YVYU:
                     case V4L2_PIX_FMT_NV12:
                          frm->discrete.width=dev->vs_frame_uncompressed[subdev][frm->index].wWidth;
                          frm->discrete.height=dev->vs_frame_uncompressed[subdev][frm->index].wHeight;
                          break;
                     case V4L2_PIX_FMT_MJPEG:
                          frm->discrete.width=dev->vs_frame_mjpeg[subdev][frm->index].wWidth;
                          frm->discrete.height=dev->vs_frame_mjpeg[subdev][frm->index].wHeight;
                          break;
                     case V4L2_PIX_FMT_H264:
                          frm->discrete.width=dev->vs_frame_h264f[subdev][frm->index].wWidth;
                          frm->discrete.height=dev->vs_frame_h264f[subdev][frm->index].wHeight;
                          break;
                 }

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        type: %d", frm->type);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        discrete.width: %d", frm->discrete.width);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        discrete.height: %d", frm->discrete.height);
                 }

                 dctldatasize=sizeof(*frm);
             }
             break;
        case VIDIOC_ENUM_FRAMEINTERVALS:
             {
                 struct v4l2_frmivalenum* frm;
                 int frameno=-1;

                 frm=(struct v4l2_frmivalenum*)dptr;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_ENUM_FRAMEINTERVALS:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        index: %d", frm->index);
                 }

                 frm->reserved[0]=0;
                 frm->reserved[1]=0;

                 /* Check for supported format and index in range */
                 ret=EINVAL;
                 switch (frm->pixel_format)
                 {
                     case V4L2_PIX_FMT_YUYV:
                     case V4L2_PIX_FMT_UYVY:
                     case V4L2_PIX_FMT_YVYU:
                     case V4L2_PIX_FMT_VYUY:
                          if (((frm->pixel_format==V4L2_PIX_FMT_UYVY) ||
                               (frm->pixel_format==V4L2_PIX_FMT_YVYU) ||
                               (frm->pixel_format==V4L2_PIX_FMT_VYUY)) && (!uvc_emulation))
                          {
                              break;
                          }
                          for (jt=0; jt<dev->vs_formats[subdev]; jt++)
                          {
                              if ((dev->vs_format[subdev][jt]==UVC_FORMAT_YUY2) ||
                                  (dev->vs_format[subdev][jt]==UVC_FORMAT_UYVY) ||
                                  (dev->vs_format[subdev][jt]==UVC_FORMAT_YVYU) ||
                                  (dev->vs_format[subdev][jt]==UVC_FORMAT_VYUY))
                              {
                                  for (it=0; it<dev->vs_format_uncompressed[subdev].bNumFrameDescriptors; it++)
                                  {
                                      if ((dev->vs_frame_uncompressed[subdev][it].wWidth==frm->width) &&
                                          (dev->vs_frame_uncompressed[subdev][it].wHeight==frm->height))
                                      {
                                          frameno=it;

                                          if (dev->vs_frame_uncompressed[subdev][frameno].bFrameIntervalType)
                                          {
                                              if (frm->index>=dev->vs_frame_uncompressed[subdev][frameno].bFrameIntervalType)
                                              {
                                                  if (uvc_verbose>2)
                                                  {
                                                      slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: index higher than amount of interval descriptors");
                                                  }
                                                  break;
                                              }
                                              frm->type=V4L2_FRMIVAL_TYPE_DISCRETE;
                                              frm->discrete.numerator=dev->vs_frame_uncompressed[subdev][frameno].dwFrameInterval[frm->index];
                                              frm->discrete.denominator=10000000;
                                          }
                                          else
                                          {
                                              if (frm->index>=1)
                                              {
                                                  if (uvc_verbose>2)
                                                  {
                                                      slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: index higher than amount of interval descriptors");
                                                  }
                                                  break;
                                              }
                                              frm->type=V4L2_FRMIVAL_TYPE_STEPWISE;
                                              frm->stepwise.min.numerator=dev->vs_frame_uncompressed[subdev][frameno].dwMinFrameInterval;
                                              frm->stepwise.min.denominator=10000000;
                                              frm->stepwise.max.numerator=dev->vs_frame_uncompressed[subdev][frameno].dwMaxFrameInterval;
                                              frm->stepwise.max.denominator=10000000;
                                              frm->stepwise.step.numerator=dev->vs_frame_uncompressed[subdev][frameno].dwFrameIntervalStep;
                                              frm->stepwise.step.denominator=10000000;
                                          }

                                          ret=EOK;
                                          break;
                                      }
                                  }
                              }
                          }
                          break;
                     case V4L2_PIX_FMT_NV12:
                          for (jt=0; jt<dev->vs_formats[subdev]; jt++)
                          {
                              if (dev->vs_format[subdev][jt]==UVC_FORMAT_NV12)
                              {
                                  for (it=0; it<dev->vs_format_uncompressed[subdev].bNumFrameDescriptors; it++)
                                  {
                                      if ((dev->vs_frame_uncompressed[subdev][it].wWidth==frm->width) &&
                                          (dev->vs_frame_uncompressed[subdev][it].wHeight==frm->height))
                                      {
                                          frameno=it;

                                          if (dev->vs_frame_uncompressed[subdev][frameno].bFrameIntervalType)
                                          {
                                              if (frm->index>=dev->vs_frame_uncompressed[subdev][frameno].bFrameIntervalType)
                                              {
                                                  if (uvc_verbose>2)
                                                  {
                                                      slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: index higher than amount of interval descriptors");
                                                  }
                                                  break;
                                              }
                                              frm->type=V4L2_FRMIVAL_TYPE_DISCRETE;
                                              frm->discrete.numerator=dev->vs_frame_uncompressed[subdev][frameno].dwFrameInterval[frm->index];
                                              frm->discrete.denominator=10000000;
                                          }
                                          else
                                          {
                                              if (frm->index>=1)
                                              {
                                                  if (uvc_verbose>2)
                                                  {
                                                      slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: index higher than amount of interval descriptors");
                                                  }
                                                  break;
                                              }
                                              frm->type=V4L2_FRMIVAL_TYPE_STEPWISE;
                                              frm->stepwise.min.numerator=dev->vs_frame_uncompressed[subdev][frameno].dwMinFrameInterval;
                                              frm->stepwise.min.denominator=10000000;
                                              frm->stepwise.max.numerator=dev->vs_frame_uncompressed[subdev][frameno].dwMaxFrameInterval;
                                              frm->stepwise.max.denominator=10000000;
                                              frm->stepwise.step.numerator=dev->vs_frame_uncompressed[subdev][frameno].dwFrameIntervalStep;
                                              frm->stepwise.step.denominator=10000000;
                                          }

                                          ret=EOK;
                                          break;
                                      }
                                  }
                              }
                          }
                          break;
                     case V4L2_PIX_FMT_MJPEG:
                          for (jt=0; jt<dev->vs_formats[subdev]; jt++)
                          {
                              if (dev->vs_format[subdev][jt]==UVC_FORMAT_MJPG)
                              {
                                  for (it=0; it<dev->vs_format_mjpeg[subdev].bNumFrameDescriptors; it++)
                                  {
                                      if ((dev->vs_frame_mjpeg[subdev][it].wWidth==frm->width) &&
                                          (dev->vs_frame_mjpeg[subdev][it].wHeight==frm->height))
                                      {
                                          frameno=it;

                                          if (dev->vs_frame_mjpeg[subdev][frameno].bFrameIntervalType)
                                          {
                                              if (frm->index>=dev->vs_frame_mjpeg[subdev][frameno].bFrameIntervalType)
                                              {
                                                  if (uvc_verbose>2)
                                                  {
                                                      slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: index higher than amount of interval descriptors");
                                                  }
                                                  break;
                                              }
                                              frm->type=V4L2_FRMIVAL_TYPE_DISCRETE;
                                              frm->discrete.numerator=dev->vs_frame_mjpeg[subdev][frameno].dwFrameInterval[frm->index];
                                              frm->discrete.denominator=10000000;
                                          }
                                          else
                                          {
                                              if (frm->index>=1)
                                              {
                                                  if (uvc_verbose>2)
                                                  {
                                                      slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: index higher than amount of interval descriptors");
                                                  }
                                                  break;
                                              }
                                              frm->type=V4L2_FRMIVAL_TYPE_STEPWISE;
                                              frm->stepwise.min.numerator=dev->vs_frame_mjpeg[subdev][frameno].dwMinFrameInterval;
                                              frm->stepwise.min.denominator=10000000;
                                              frm->stepwise.max.numerator=dev->vs_frame_mjpeg[subdev][frameno].dwMaxFrameInterval;
                                              frm->stepwise.max.denominator=10000000;
                                              frm->stepwise.step.numerator=dev->vs_frame_mjpeg[subdev][frameno].dwFrameIntervalStep;
                                              frm->stepwise.step.denominator=10000000;
                                          }

                                          ret=EOK;
                                          break;
                                      }
                                  }
                              }
                          }
                          break;
                     case V4L2_PIX_FMT_H264:
                          for (jt=0; jt<dev->vs_formats[subdev]; jt++)
                          {
                              if ((dev->vs_format[subdev][jt]==UVC_FORMAT_H264) ||
                                  (dev->vs_format[subdev][jt]==UVC_FORMAT_H264F))
                              {
                                  for (it=0; it<dev->vs_format_h264f[subdev].bNumFrameDescriptors; it++)
                                  {
                                      if ((dev->vs_frame_h264f[subdev][it].wWidth==frm->width) &&
                                          (dev->vs_frame_h264f[subdev][it].wHeight==frm->height))
                                      {
                                          frameno=it;

                                          if (dev->vs_frame_h264f[subdev][frameno].bFrameIntervalType)
                                          {
                                              if (frm->index>=dev->vs_frame_h264f[subdev][frameno].bFrameIntervalType)
                                              {
                                                  if (uvc_verbose>2)
                                                  {
                                                      slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: index higher than amount of interval descriptors");
                                                  }
                                                  break;
                                              }
                                              frm->type=V4L2_FRMIVAL_TYPE_DISCRETE;
                                              frm->discrete.numerator=dev->vs_frame_h264f[subdev][frameno].dwFrameInterval[frm->index];
                                              frm->discrete.denominator=10000000;
                                          }
                                          else
                                          {
                                              if (frm->index>=1)
                                              {
                                                  if (uvc_verbose>2)
                                                  {
                                                      slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: index higher than amount of interval descriptors");
                                                  }
                                                  break;
                                              }
                                              frm->type=V4L2_FRMIVAL_TYPE_STEPWISE;
                                              frm->stepwise.min.numerator=dev->vs_frame_h264f[subdev][frameno].dwMinFrameInterval;
                                              frm->stepwise.min.denominator=10000000;
                                              frm->stepwise.max.numerator=dev->vs_frame_h264f[subdev][frameno].dwMaxFrameInterval;
                                              frm->stepwise.max.denominator=10000000;
                                              frm->stepwise.step.numerator=dev->vs_frame_h264f[subdev][frameno].dwFrameIntervalStep;
                                              frm->stepwise.step.denominator=10000000;
                                          }

                                          ret=EOK;
                                          break;
                                      }
                                  }
                              }
                          }
                          break;
                     default:
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: unsupported pixel format");
                          }
                          break;
                 }

                 if (ret!=EOK)
                 {
                     break;
                 }

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        type: %d", frm->type);
                     if (frm->type==V4L2_FRMIVAL_TYPE_DISCRETE)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        discrete.numerator: %d", frm->discrete.numerator);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        discrete.denominator: %d", frm->discrete.denominator);
                     }
                     if (frm->type==V4L2_FRMIVAL_TYPE_STEPWISE)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        min.stepwise.numerator: %d", frm->stepwise.min.numerator);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        min.stepwise.denominator: %d", frm->stepwise.min.denominator);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        max.stepwise.numerator: %d", frm->stepwise.max.numerator);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        max.stepwise.denominator: %d", frm->stepwise.max.denominator);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        step.stepwise.numerator: %d", frm->stepwise.step.numerator);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        step.stepwise.denominator: %d", frm->stepwise.step.denominator);
                     }
                 }

                 dctldatasize=sizeof(*frm);
             }
             break;
        case VIDIOC_QUERYCTRL:
             {
                 struct v4l2_queryctrl* ctrl;
                 int newid=-1;

                 ctrl=(struct v4l2_queryctrl*)dptr;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_QUERYCTRL:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        id: %08X", ctrl->id);
                 }

                 /* Next element search is requested */
                 if (ctrl->id & V4L2_CTRL_FLAG_NEXT_CTRL)
                 {
                     ctrl->id&=~(V4L2_CTRL_FLAG_NEXT_CTRL);

                     if ((V4L2_CTRL_ID2CLASS(ctrl->id)==V4L2_CTRL_CLASS_USER) || (newid==-1))
                     {
                         if (ctrl->id<V4L2_CTRL_CLASS_USER)
                         {
                             ctrl->id=V4L2_CTRL_CLASS_USER-1;
                         }
                         for (it=ctrl->id+1; it<V4L2_CTRL_CLASS_USER+0x00001FFF; it++)
                         {
                             if (uvc_query_control_data(dev, it, subdev, NULL))
                             {
                                 newid=it;
                                 break;
                             }
                         }
                     }

                     if ((V4L2_CTRL_ID2CLASS(ctrl->id)==V4L2_CTRL_CLASS_CAMERA) || (newid==-1))
                     {
                         if (ctrl->id<V4L2_CTRL_CLASS_CAMERA)
                         {
                             ctrl->id=V4L2_CTRL_CLASS_CAMERA-1;
                         }
                         for (it=ctrl->id+1; it<V4L2_CTRL_CLASS_CAMERA+0x00001FFF; it++)
                         {
                             if (uvc_query_control_data(dev, it, subdev, NULL))
                             {
                                 newid=it;
                                 break;
                             }
                         }
                     }

                     if (newid==-1)
                     {
                         if (uvc_verbose>2)
                         {
                             slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: no more controls left for search");
                         }
                         ret=EINVAL;
                         break;
                     }
                     ctrl->id=newid;
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        new id: %08X", ctrl->id);
                     }
                 }

                 /* Now work with passed or previously found id */
                 if (uvc_query_control_data(dev, ctrl->id, subdev, NULL))
                 {
                     ret=uvc_query_control(dev, subdev, ctrl);
                     if (ret!=EOK)
                     {
                         break;
                     }
                 }
                 else
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: control can't be found");
                     }
                     ret=EINVAL;
                     break;
                 }

                 ctrl->reserved[0]=0;
                 ctrl->reserved[1]=0;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        type: %08X", ctrl->type);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        name: %s", ctrl->name);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        minimum: %d", ctrl->minimum);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        maximum: %d", ctrl->maximum);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        step: %d", ctrl->step);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        default value: %d", ctrl->default_value);
                 }

                 dctldatasize=sizeof(*ctrl);
             }
             break;
        case VIDIOC_G_CTRL:
             {
                 struct v4l2_control* ctrl;
                 control_data_t data;

                 ctrl=(struct v4l2_control*)dptr;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_G_CTRL:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        id: %08X", ctrl->id);
                 }

                 if (uvc_query_control_data(dev, ctrl->id, subdev, &data))
                 {
                     if (V4L2_CTRL_ID2CLASS(ctrl->id)==V4L2_CTRL_CLASS_CAMERA)
                     {
                         struct v4l2_ext_control ectrl;

                         ectrl.id=ctrl->id;
                         ret=uvc_read_ext_ctrl(dev, &ectrl, &data);
                         if (ret!=EOK)
                         {
                             break;
                         }
                         ctrl->value=ectrl.value;
                     }
                     else
                     {
                         ret=uvc_read_ctrl(dev, ctrl, &data);
                         if (ret!=EOK)
                         {
                             break;
                         }
                     }
                 }
                 else
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: control can't be found");
                     }
                     ret=EINVAL;
                     break;
                 }

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        value: %08X", ctrl->value);
                 }

                 dctldatasize=sizeof(*ctrl);
             }
             break;
        case VIDIOC_S_CTRL:
             {
                 struct v4l2_control* ctrl;
                 control_data_t data;

                 ctrl=(struct v4l2_control*)dptr;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_S_CTRL:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        id: %08X", ctrl->id);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        value: %08X", ctrl->value);
                 }

                 if (uvc_query_control_data(dev, ctrl->id, subdev, &data))
                 {
                     if (V4L2_CTRL_ID2CLASS(ctrl->id)==V4L2_CTRL_CLASS_CAMERA)
                     {
                         struct v4l2_ext_control ectrl;

                         ectrl.id=ctrl->id;
                         ectrl.value=ctrl->value;
                         ret=uvc_write_ext_ctrl(dev, &ectrl, &data, ocb);
                         if (ret!=EOK)
                         {
                             break;
                         }
                     }
                     else
                     {
                         ret=uvc_write_ctrl(dev, ctrl, &data, ocb);
                         if (ret!=EOK)
                         {
                             break;
                         }
                     }
                 }
                 else
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: control can't be found");
                     }
                     ret=EINVAL;
                     break;
                 }

                 dctldatasize=sizeof(*ctrl);
             }
             break;
        case VIDIOC_QUERYMENU:
             {
                 struct v4l2_querymenu* menu;
                 control_data_t data;

                 menu=(struct v4l2_querymenu*)dptr;
                 menu->reserved=0;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_QUERYMENU:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        id: %08X", menu->id);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        index: %08X", menu->index);
                 }

                 if (uvc_query_control_data(dev, menu->id, subdev, &data))
                 {
                     if (data.type!=V4L2_CTRL_TYPE_MENU)
                     {
                         if (uvc_verbose>2)
                         {
                             slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: control type is not a menu");
                         }
                         ret=EINVAL;
                         break;
                     }

                     switch (menu->id)
                     {
                         case V4L2_CID_POWER_LINE_FREQUENCY:
                              switch (menu->index)
                              {
                                  case 0:
                                       strncpy((char*)menu->name, "Disabled", sizeof(menu->name));
                                       break;
                                  case 1:
                                       strncpy((char*)menu->name, "50 Hz", sizeof(menu->name));
                                       break;
                                  case 2:
                                       strncpy((char*)menu->name, "60 Hz", sizeof(menu->name));
                                       break;
                                  default:
                                       ret=EINVAL;
                                       break;
                              }
                              break;
                         case V4L2_CID_EXPOSURE_AUTO:
                              switch (menu->index)
                              {
                                  case 0:
                                       strncpy((char*)menu->name, "Auto Mode", sizeof(menu->name));
                                       break;
                                  case 1:
                                       strncpy((char*)menu->name, "Manual Mode", sizeof(menu->name));
                                       break;
                                  case 2:
                                       strncpy((char*)menu->name, "Shutter Priority Mode", sizeof(menu->name));
                                       break;
                                  case 3:
                                       strncpy((char*)menu->name, "Aperture Priority Mode", sizeof(menu->name));
                                       break;
                                  default:
                                       ret=EINVAL;
                                       break;
                              }
                              break;
                         default:
                              if (uvc_verbose>2)
                              {
                                  slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: control type is not a menu");
                              }
                              ret=EINVAL;
                              break;
                     }
                 }
                 else
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: control can't be found");
                     }
                     ret=EINVAL;
                     break;
                 }

                 if (ret!=EOK)
                 {
                     break;
                 }

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        name: %s", menu->name);
                 }

                 dctldatasize=sizeof(*menu);
             }
             break;
        case VIDIOC_G_EXT_CTRLS:
             {
                 struct v4l2_ext_controls* ctrl;
                 struct v4l2_ext_control* _ctrl;
                 control_data_t data;
                 unsigned int data_class;

                 /* We have to return the request stucture in any case */
                 ctrl=(struct v4l2_ext_controls*)dptr;
                 _ctrl=(struct v4l2_ext_control*)(ctrl+1);
                 dctldatasize=sizeof(*ctrl);
                 dctlextdatasize=sizeof(*_ctrl)*ctrl->count;

                 ctrl->reserved[0]=0;
                 ctrl->reserved[1]=0;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_G_EXT_CTRLS:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        ctrl_class: %08X", ctrl->ctrl_class);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        count: %08X", ctrl->count);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        controls: %08X", (uint32_t)ctrl->controls);
                 }

                 /* Check if we have to work with embedded pointers */
                 if ((ctrl->count>0) && (ctrl->controls!=NULL) && (dev->current_stage[subdev]==0))
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EMORE: asking for embedded pointer conversion");
                     }
                     dev->current_stage[subdev]=1;
                     dctlextdatasize=0;
                     ret=EMORE;
                     break;
                 }

                 /* Clear the second stage of embedded pointer gathering */
                 dev->current_stage[subdev]=0;

                 /* Check if multiple classes are accessed */
                 if (ctrl->count>0)
                 {
                     if (ctrl->ctrl_class!=0)
                     {
                         data_class = V4L2_CTRL_ID2CLASS(ctrl->ctrl_class);
                         for (it=0; it<ctrl->count; it++)
                         {
                             if (data_class!=V4L2_CTRL_ID2CLASS(_ctrl[it].id))
                             {
                                 if (uvc_verbose>2)
                                 {
                                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: multiple classes are accessed");
                                 }
                                 ret=EINVAL;
                                 break;
                             }
                         }
                     }
                     if (ret!=EOK)
                     {
                         ctrl->error_idx=ctrl->count;
                         break;
                     }
                 }

                 for (it=0; it<ctrl->count; it++)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        controls[%d].id: %08X", it, _ctrl[it].id);
                     }

                     if (uvc_query_control_data(dev, _ctrl[it].id, subdev, &data))
                     {
                         if (data.type==V4L2_CTRL_TYPE_CTRL_CLASS)
                         {
                             if (uvc_verbose>2)
                             {
                                 slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EACCES: attempt to read from class control");
                             }
                             ret=EACCES;
                             break;
                         }

                         if (V4L2_CTRL_ID2CLASS(_ctrl[it].id)==V4L2_CTRL_CLASS_CAMERA)
                         {
                             ret=uvc_read_ext_ctrl(dev, &_ctrl[it], &data);
                             if (ret!=EOK)
                             {
                                 break;
                             }
                         }
                         else
                         {
                             struct v4l2_control sctrl;

                             sctrl.id=_ctrl[it].id;
                             ret=uvc_read_ctrl(dev, &sctrl, &data);
                             if (ret!=EOK)
                             {
                                 break;
                             }
                             _ctrl[it].value=sctrl.value;
                         }

                         if (uvc_verbose>2)
                         {
                             slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        controls[%d].value: %08X", it, _ctrl[it].value);
                         }
                     }
                     else
                     {
                         if (uvc_verbose>2)
                         {
                             slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: control can't be found");
                         }
                         ret=EINVAL;
                         break;
                     }
                 }

                 ctrl->error_idx=it;
                 if (ret!=EOK)
                 {
                     ctrl->error_idx++;
                 }

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        error_idx: %08X", ctrl->error_idx);
                 }
             }
             break;
        case VIDIOC_S_EXT_CTRLS:
             {
                 struct v4l2_ext_controls* ctrl;
                 struct v4l2_ext_control* _ctrl;
                 control_data_t data;
                 unsigned int data_class;

                 /* We have to return the request stucture in any case */
                 ctrl=(struct v4l2_ext_controls*)dptr;
                 _ctrl=(struct v4l2_ext_control*)(ctrl+1);
                 dctldatasize=sizeof(*ctrl);
                 dctlextdatasize=sizeof(*_ctrl)*ctrl->count;

                 ctrl->reserved[0]=0;
                 ctrl->reserved[1]=0;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_S_EXT_CTRLS:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        class: %08X", ctrl->ctrl_class);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        count: %08X", ctrl->count);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        controls: %08X", (uint32_t)ctrl->controls);
                 }

                 /* Check if we have to work with embedded pointers */
                 if ((ctrl->count>0) && (ctrl->controls!=NULL) && (dev->current_stage[subdev]==0))
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EMORE: asking for embedded pointer lookup");
                     }
                     dev->current_stage[subdev]=1;
                     dctlextdatasize=0;
                     ret=EMORE;
                     break;
                 }

                 /* Clear the second stage of embedded pointer gathering */
                 dev->current_stage[subdev]=0;

                 /* Check if multiple classes are accessed */
                 if (ctrl->count>0)
                 {
                     if (ctrl->ctrl_class!=0)
                     {
                         data_class = V4L2_CTRL_ID2CLASS(ctrl->ctrl_class);
                         for (it=0; it<ctrl->count; it++)
                         {
                             if (data_class!=V4L2_CTRL_ID2CLASS(_ctrl[it].id))
                             {
                                 if (uvc_verbose>2)
                                 {
                                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: multiple classes are accessed");
                                 }
                                 ret=EINVAL;
                                 break;
                             }
                         }
                     }
                     if (ret!=EOK)
                     {
                         ctrl->error_idx=ctrl->count;
                         break;
                     }
                 }

                 for (it=0; it<ctrl->count; it++)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        controls[%d].id: %08X", it, _ctrl[it].id);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        controls[%d].value: %08X", it, _ctrl[it].value);
                     }
                     if (uvc_query_control_data(dev, _ctrl[it].id, subdev, &data))
                     {
                         if (V4L2_CTRL_ID2CLASS(_ctrl[it].id)==V4L2_CTRL_CLASS_CAMERA)
                         {
                             ret=uvc_write_ext_ctrl(dev, &_ctrl[it], &data, ocb);
                             if (ret!=EOK)
                             {
                                 break;
                             }
                         }
                         else
                         {
                             struct v4l2_control sctrl;

                             sctrl.id=_ctrl[it].id;
                             sctrl.value=_ctrl[it].value;
                             ret=uvc_write_ctrl(dev, &sctrl, &data, ocb);
                             if (ret!=EOK)
                             {
                                 break;
                             }
                         }
                     }
                     else
                     {
                         if (uvc_verbose>2)
                         {
                             slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: control can't be found");
                         }
                         ret=EINVAL;
                         break;
                     }

                     if (ret!=EOK)
                     {
                         break;
                     }
                 }

                 ctrl->error_idx=it;
                 if (ret!=EOK)
                 {
                     ctrl->error_idx++;
                 }

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        error_idx: %08X", ctrl->error_idx);
                 }
             }
             break;
        case VIDIOC_TRY_EXT_CTRLS:
             {
                 struct v4l2_ext_controls* ctrl;
                 struct v4l2_ext_control* _ctrl;
                 control_data_t data;
                 unsigned int data_class;

                 /* We have to return the request stucture in any case */
                 ctrl=(struct v4l2_ext_controls*)dptr;
                 _ctrl=(struct v4l2_ext_control*)(ctrl+1);
                 dctldatasize=sizeof(*ctrl);
                 dctlextdatasize=sizeof(*_ctrl)*ctrl->count;

                 ctrl->reserved[0]=0;
                 ctrl->reserved[1]=0;
                 ctrl->error_idx=0;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_TRY_EXT_CTRLS:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        class: %08X", ctrl->ctrl_class);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        count: %08X", ctrl->count);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        controls: %08X", (uint32_t)ctrl->controls);
                 }

                 /* Check if we have to work with embedded pointers */
                 if ((ctrl->count>0) && (ctrl->controls!=NULL) && (dev->current_stage[subdev]==0))
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EMORE: asking for embedded pointer lookup");
                     }
                     dev->current_stage[subdev]=1;
                     dctlextdatasize=0;
                     ret=EMORE;
                     break;
                 }

                 /* Clear the second stage of embedded pointer gathering */
                 dev->current_stage[subdev]=0;

                 /* Check if multiple classes are accessed */
                 if (ctrl->count>0)
                 {
                     if (ctrl->ctrl_class!=0)
                     {
                         data_class = V4L2_CTRL_ID2CLASS(ctrl->ctrl_class);
                         for (it=0; it<ctrl->count; it++)
                         {
                             if (data_class!=V4L2_CTRL_ID2CLASS(_ctrl[it].id))
                             {
                                 if (uvc_verbose>2)
                                 {
                                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: multiple classes are accessed");
                                 }
                                 ctrl->error_idx=it;
                                 ret=EINVAL;
                                 break;
                             }
                         }
                     }
                     if (ret!=EOK)
                     {
                         break;
                     }
                 }

                 for (it=0; it<ctrl->count; it++)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        controls[%d].id: %08X", it, _ctrl[it].id);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        controls[%d].value: %08X", it, _ctrl[it].value);
                     }
                     if (uvc_query_control_data(dev, _ctrl[it].id, subdev, &data))
                     {
                         if (data.type==V4L2_CTRL_TYPE_CTRL_CLASS)
                         {
                             if (uvc_verbose>2)
                             {
                                 slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EACCES: attempt to write to class control");
                             }
                             ret=EACCES;
                             break;
                         }
                         if (_ctrl[it].id==UVC_CID_EVENT_BUTTON)
                         {
                             if (uvc_verbose>2)
                             {
                                 slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EACCES: attempt to write to r/o volatile control");
                             }
                             ret=EACCES;
                             break;
                         }
                         continue;
                     }
                     else
                     {
                         if (uvc_verbose>2)
                         {
                             slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: control can't be found");
                         }
                         ret=EINVAL;
                         break;
                     }
                 }

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        error_idx: %08X", ctrl->error_idx);
                 }

                 ctrl->error_idx=0;
             }
             break;
        case VIDIOC_G_PRIORITY:
             {
                 uint32_t* prio;

                 prio=(uint32_t*)dptr;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_G_PRIORITY:");
                 }

                 *prio=dev->current_priority[subdev];

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        priority: %08X", *prio);
                 }

                 dctldatasize=sizeof(*prio);
             }
             break;
        case VIDIOC_S_PRIORITY:
             {
                 uint32_t* prio;

                 prio=(uint32_t*)dptr;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_S_PRIORITY:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        priority: %08X", *prio);
                 }

                 /* First, check for highest priority */
                 if ((dev->current_priority[subdev]==V4L2_PRIORITY_RECORD) &&
                     (dev->current_priority_ocb[subdev]!=ocb))
                 {
                     /* Deny priority change if the file descriptor */
                     /* is set to the highest priority.             */
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        owner ocb=%08X, caller ocb=%08X", (unsigned int)dev->current_priority_ocb[subdev], (unsigned int)ocb);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EBUSY: other file descriptor has highest priority");
                     }
                     ret=EBUSY;
                     break;
                 }

                 /* Check if record priority is acquired */
                 if ((dev->current_priority_ocb[subdev]==NULL) &&
                    (*prio==V4L2_PRIORITY_RECORD))
                 {
                     dev->current_priority_ocb[subdev]=ocb;
                 }

                 /* Check if reset priority to other than record */
                 if ((*prio!=V4L2_PRIORITY_RECORD) &&
                     (dev->current_priority_ocb[subdev]==ocb))
                 {
                     dev->current_priority_ocb[subdev]=NULL;
                 }

                 dev->current_priority[subdev]=*prio;
             }
             break;
        case VIDIOC_SUBSCRIBE_EVENT:
             {
                 struct v4l2_event_subscription* event;
                 int event_it=-1;
                 int events_count=0;

                 event=(struct v4l2_event_subscription*)dptr;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_SUBSCRIBE_EVENT:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        type: %08X", event->type);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        id: %08X", event->id);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        flags: %08X", event->flags);
                 }

                 /* Clear reserved fields */
                 memset(event->reserved, 0x00, sizeof(event->reserved));

                 /* Check if we already have event structure for this ocb */
                 for(it=0; it<UVC_MAX_OPEN_FDS; it++)
                 {
                     if (dev->event[it].ocb==ocb)
                     {
                         event_it=it;
                         break;
                     }
                 }
                 if (event_it==-1)
                 {
                     /* Find any free event entry */
                     for(it=0; it<UVC_MAX_OPEN_FDS; it++)
                     {
                         if (dev->event[it].ocb==NULL)
                         {
                             event_it=it;

                             /* Initialize the structure */
                             dev->event[event_it].ocb=ocb;
                             dev->event[event_it].sequence=0;
                             TAILQ_INIT(&dev->event[event_it].head);
                             pthread_mutex_init(&dev->event[event_it].access, NULL);
                             break;
                         }
                     }
                     if (event_it==-1)
                     {
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: Too many open file descriptors for one device");
                          }
                          ret=EINVAL;
                          break;
                     }
                 }

                 if (event->flags & ~(V4L2_EVENT_SUB_FL_SEND_INITIAL | V4L2_EVENT_SUB_FL_ALLOW_FEEDBACK))
                 {
                      if (uvc_verbose>2)
                      {
                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: Unsupported event flags");
                      }
                      ret=EINVAL;
                      break;
                 }

                 switch (event->type)
                 {
                     case V4L2_EVENT_VSYNC:
                     case V4L2_EVENT_EOS:
                     case V4L2_EVENT_CTRL:
                     case V4L2_EVENT_FRAME_SYNC:
                          dev->event[event_it].type[event->type]=1;
                          dev->event[event_it].flags[event->type]=event->flags;
                          dev->event[event_it].id[event->type]=event->id;
                          break;
                     default:
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: Unsupported event type");
                          }
                          ret=EINVAL;
                          break;
                 }

                 /* Fill queue with initial parameters */
                 if ((event->type==V4L2_EVENT_CTRL) &&
                     ((event->flags & V4L2_EVENT_SUB_FL_SEND_INITIAL)==V4L2_EVENT_SUB_FL_SEND_INITIAL))
                 {
                     struct _uvc_event_entry* entry;
                     struct v4l2_queryctrl qctrl;
                     control_data_t data;

                     for (it=V4L2_CID_USER_BASE; it<V4L2_CTRL_CLASS_USER+0x00001FFF; it++)
                     {
                         if (uvc_query_control_data(dev, it, subdev, &data))
                         {
                             struct v4l2_control ctrl;

                             if (event->id==V4L2_CID_USER_CLASS)
                             {
                                 break;
                             }

                             if ((event->id!=0) && (event->id!=it))
                             {
                                 if (uvc_verbose>2)
                                 {
                                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Skipping ID %08X as initial event", it);
                                 }
                                 continue;
                             }

                             if (uvc_verbose>2)
                             {
                                 slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Adding ID %08X as initial event", it);
                             }

                             entry=calloc(1, sizeof(*entry));
                             if (entry==NULL)
                             {
                                 ret=ENOMEM;
                                 break;
                             }

                             ctrl.id=it;
                             uvc_read_ctrl(dev, &ctrl, &data);
                             qctrl.id=it;
                             uvc_query_control(dev, subdev, &qctrl);

                             entry->event.type=V4L2_EVENT_CTRL;
                             entry->event.pending=0;
                             entry->event.sequence=dev->event[event_it].sequence;
                             entry->event.id=it;
                             clock_gettime(CLOCK_MONOTONIC, &entry->event.timestamp);
                             memset(entry->event.reserved, 0x00, sizeof(entry->event.reserved));
                             entry->event.u.ctrl.changes=V4L2_EVENT_CTRL_CH_VALUE |
                                 V4L2_EVENT_CTRL_CH_FLAGS | V4L2_EVENT_CTRL_CH_RANGE;
                             entry->event.u.ctrl.type=data.type;
                             entry->event.u.ctrl.flags=0;
                             entry->event.u.ctrl.value=ctrl.value;
                             entry->event.u.ctrl.minimum=qctrl.minimum;
                             entry->event.u.ctrl.maximum=qctrl.maximum;
                             entry->event.u.ctrl.default_value=qctrl.default_value;
                             entry->event.u.ctrl.step=qctrl.step;
                             pthread_mutex_lock(&dev->event[event_it].access);
                             dev->event[event_it].sequence++;
                             TAILQ_INSERT_TAIL(&dev->event[event_it].head, entry, link);
                             pthread_mutex_unlock(&dev->event[event_it].access);
                             events_count++;
                         }
                     }

                     for (it=V4L2_CID_CAMERA_CLASS_BASE; it<V4L2_CTRL_CLASS_CAMERA+0x00001FFF; it++)
                     {
                         if (uvc_query_control_data(dev, it, subdev, &data))
                         {
                             struct v4l2_ext_control ctrl;

                             if (event->id==V4L2_CID_CAMERA_CLASS)
                             {
                                 break;
                             }

                             if ((event->id!=0) && (event->id!=it))
                             {
                                 continue;
                             }

                             if (uvc_verbose>2)
                             {
                                 slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Adding ID %08X as initial event", it);
                             }

                             entry=calloc(1, sizeof(*entry));
                             if (entry==NULL)
                             {
                                 ret=ENOMEM;
                                 break;
                             }

                             ctrl.id=it;
                             uvc_read_ext_ctrl(dev, &ctrl, &data);
                             qctrl.id=it;
                             uvc_query_control(dev, subdev, &qctrl);

                             entry->event.type=V4L2_EVENT_CTRL;
                             entry->event.pending=0;
                             entry->event.sequence=dev->event[event_it].sequence;
                             entry->event.id=it;
                             clock_gettime(CLOCK_MONOTONIC, &entry->event.timestamp);
                             memset(entry->event.reserved, 0x00, sizeof(entry->event.reserved));
                             entry->event.u.ctrl.changes=V4L2_EVENT_CTRL_CH_VALUE |
                                 V4L2_EVENT_CTRL_CH_FLAGS | V4L2_EVENT_CTRL_CH_RANGE;
                             entry->event.u.ctrl.type=data.type;
                             entry->event.u.ctrl.flags=0;
                             entry->event.u.ctrl.value=ctrl.value;
                             entry->event.u.ctrl.minimum=qctrl.minimum;
                             entry->event.u.ctrl.maximum=qctrl.maximum;
                             entry->event.u.ctrl.default_value=qctrl.default_value;
                             entry->event.u.ctrl.step=qctrl.step;
                             pthread_mutex_lock(&dev->event[event_it].access);
                             dev->event[event_it].sequence++;
                             TAILQ_INSERT_TAIL(&dev->event[event_it].head, entry, link);
                             pthread_mutex_unlock(&dev->event[event_it].access);
                             events_count++;
                         }
                     }

                     /* Notify callers of select() only on this file descriptor */
                     if (events_count>0)
                     {
                         iofunc_notify_trigger(ocb->notify, 1, IOFUNC_NOTIFY_OBAND);
                     }
                 }
             }
             break;
        case VIDIOC_UNSUBSCRIBE_EVENT:
             {
                 struct v4l2_event_subscription* event;
                 int event_it=-1;

                 event=(struct v4l2_event_subscription*)dptr;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_UNSUBSCRIBE_EVENT:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        type: %08X", event->type);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        id: %08X", event->id);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        flags: %08X", event->flags);
                 }

                 /* Clear reserved fields */
                 memset(event->reserved, 0x00, sizeof(event->reserved));

                 /* Check if we already have event structure for this ocb */
                 for(it=0; it<UVC_MAX_OPEN_FDS; it++)
                 {
                     if (dev->event[it].ocb==ocb)
                     {
                         event_it=it;
                         break;
                     }
                 }
                 /* Can't find ocb, so this means that events are not collected for this handle */
                 if (event_it==-1)
                 {
                      ret=EOK;
                      break;
                 }

                 switch (event->type)
                 {
                     case V4L2_EVENT_VSYNC:
                     case V4L2_EVENT_EOS:
                     case V4L2_EVENT_CTRL:
                     case V4L2_EVENT_FRAME_SYNC:
                          dev->event[event_it].type[event->type]=0;
                          dev->event[event_it].flags[event->type]=event->flags;
                          break;
                     default:
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: Unsupported event type");
                          }
                          ret=EINVAL;
                          break;
                 }
             }
             break;
        case VIDIOC_DQEVENT:
             {
                 struct v4l2_event* event;
                 struct _uvc_event_entry* entry=NULL;
                 struct _uvc_event_entry* tentry;
                 int pending=-1;

                 event=(struct v4l2_event*)dptr;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_DQEVENT:");
                 }

                 pthread_mutex_lock(&dev->event[it].access);
                 TAILQ_FOREACH_SAFE(entry, &dev->event[it].head, link, tentry)
                 {
                     pending++;
                 }
                 pthread_mutex_unlock(&dev->event[it].access);

                 if (pending<0)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        ENOENT: No more events left");
                     }
                     ret=ENOENT;
                     break;
                 }

                 pthread_mutex_lock(&dev->event[it].access);
                 TAILQ_FOREACH_SAFE(entry, &dev->event[it].head, link, tentry)
                 {
                     *event=entry->event;
                     event->pending=pending;
                     TAILQ_REMOVE(&dev->event[it].head, entry, link);
                     if (entry!=NULL)
                     {
                         free(entry);
                     }
                     break;
                 };
                 pthread_mutex_unlock(&dev->event[it].access);

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        type: %08X", event->type);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        pending: %08X", event->pending);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        sequence: %08X", event->sequence);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        id: %08X", event->id);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        timestamp.tv_sec: %08X", event->timestamp.tv_sec);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        timestamp.tv_nsec: %08X", (unsigned int)event->timestamp.tv_nsec);
                 }

                 dctldatasize=sizeof(*event);
             }
             break;
        case VIDIOC_REQBUFS:
             {
                 struct v4l2_requestbuffers* buf;
                 char fdname[128];

                 buf=(struct v4l2_requestbuffers*)dptr;
                 memset(buf->reserved, 0x00, sizeof(buf->reserved));

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_REQBUFS:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        count: %d", buf->count);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        type: %d", buf->type);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        memory: %d", buf->memory);
                 }

                 if (buf->type!=V4L2_BUF_TYPE_VIDEO_CAPTURE)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: buffer type is not V4L2_BUF_TYPE_VIDEO_CAPTURE");
                     }
                     ret=EINVAL;
                     break;
                 }

                 if (buf->memory!=V4L2_MEMORY_MMAP)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: buffer i/o type is not V4L2_MEMORY_MMAP");
                     }
                     ret=EINVAL;
                     break;
                 }

                 /* Check if transfer/streaming is active */
                 if (dev->current_transfer[subdev])
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EBUSY: transfer is active");
                     }
                     ret=EBUSY;
                     break;
                 }

                 /* Check permissions for this operation */
                 if ((dev->current_reqbufs_ocb[subdev]!=NULL) &&
                     (dev->current_reqbufs_ocb[subdev]!=ocb))
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EBUSY: can't acquire permissions");
                     }
                     ret=EBUSY;
                     break;
                 }

                 /* Limit buffers amount to some reasonable value */
                 if (buf->count>VIDEO_MAX_FRAME)
                 {
                     buf->count=VIDEO_MAX_FRAME;
                 }

                 /* Shared memory name consist of device minor, usb path and usb devno */
                 sprintf(fdname, "/devu-uvc-%d-%d-%d", minor(ocb->hdr.attr->hdr->rdev), dev->map->usb_path, dev->map->usb_devno);

                 /* Free any previously allocated buffers */
                 if (dev->current_buffer_count[subdev])
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Deleting %d old buffer(s)", dev->current_buffer_count[subdev]);
                     }

                     if (dev->current_buffer_fds[subdev]!=-1)
                     {
                         close(dev->current_buffer_fds[subdev]);
                     }
                     dev->current_buffer_fds[subdev]=-1;
                     if (dev->input_buffer[subdev].mutex_inited)
                     {
                         pthread_mutex_destroy(&dev->input_buffer[subdev].access);
                         dev->input_buffer[subdev].mutex_inited=0;
                     }
                     if (dev->output_buffer[subdev].mutex_inited)
                     {
                         pthread_mutex_destroy(&dev->output_buffer[subdev].access);
                         dev->output_buffer[subdev].mutex_inited=0;
                     }
                 }

                 /* Unlink shared object anyway */
                 shm_unlink(fdname);

                 /* Negotiate buffers count with application, 0 is also valid */
                 if (buf->count==0)
                 {
                     dev->current_reqbufs_ocb[subdev]=NULL;
                 }
                 else
                 {
                     int fd;
                     unsigned int size;
                     unsigned int chunksize;

                     size=dev->current_stride[subdev]*dev->current_height[subdev];
                     chunksize=sysconf(_SC_PAGE_SIZE);
                     /* Adjust buffer size to system page size */
                     size=(size+chunksize-1) & ~(chunksize-1);

                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Allocating %d buffer(s), %d bytes each, total %d bytes", buf->count, size, size*buf->count);
                     }

                     /* Allocate buffers */
                     fd=shm_open(fdname, O_RDWR | O_CREAT,
                         S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH);
                     if (fd!=-1)
                     {
                         status=shm_ctl(fd, SHMCTL_GLOBAL | SHMCTL_ANON, 0, size*buf->count);
                         if (status==-1)
                         {
                             if (uvc_verbose>2)
                             {
                                 slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        ENOMEM: Can't change size of shared memory object");
                             }
                             ret=ENOMEM;
                             break;
                         }
                     }
                     else
                     {
                         if (uvc_verbose>2)
                         {
                             slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        ENOMEM: Can't create shared memory object");
                         }
                         ret=ENOMEM;
                         break;
                     }

                     /* Store requested buffer count */
                     dev->current_reqbufs_ocb[subdev]=ocb;
                     dev->current_buffer_count[subdev]=buf->count;
                     dev->buffer_mode_mmap[subdev]=1;
                     dev->current_buffer_fds[subdev]=fd;
                     TAILQ_INIT(&dev->input_buffer[subdev].head);
                     TAILQ_INIT(&dev->output_buffer[subdev].head);
                     pthread_mutex_init(&dev->input_buffer[subdev].access, NULL);
                     pthread_mutex_init(&dev->output_buffer[subdev].access, NULL);
                     dev->input_buffer[subdev].mutex_inited=1;
                     dev->output_buffer[subdev].mutex_inited=1;
                 }

                 dctldatasize=sizeof(*buf);
             }
             break;
        case VIDIOC_QUERYBUF:
             {
                 struct v4l2_buffer* buf;
                 unsigned int size;
                 unsigned int chunksize;
                 struct _uvc_buffer_entry* entry;
                 struct _uvc_buffer_entry* tentry;

                 buf=(struct v4l2_buffer*)dptr;
                 memset(&buf->reserved, 0x00, sizeof(buf->reserved));
                 memset(&buf->reserved2, 0x00, sizeof(buf->reserved2));

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_QUERYBUF:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        index: %d", buf->index);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        type: %d", buf->type);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        memory: %d", buf->memory);
                 }

                 if (buf->type!=V4L2_BUF_TYPE_VIDEO_CAPTURE)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: buffer type is not V4L2_BUF_TYPE_VIDEO_CAPTURE");
                     }
                     ret=EINVAL;
                     break;
                 }

                 if (buf->memory!=V4L2_MEMORY_MMAP)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: buffer i/o type is not V4L2_MEMORY_MMAP");
                     }
                     ret=EINVAL;
                     break;
                 }

                 /* Check requested buffer index */
                 if (buf->index>=dev->current_buffer_count[subdev])
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: buffer index is out of range (%d of %d)", buf->index, dev->current_buffer_count[subdev]);
                     }
                     ret=EINVAL;
                     break;
                 }

                 size=dev->current_stride[subdev]*dev->current_height[subdev];
                 chunksize=sysconf(_SC_PAGE_SIZE);
                 /* Adjust buffer size to system page size */
                 size=(size+chunksize-1) & ~(chunksize-1);

                 /* Setup the structure */
                 buf->field=V4L2_FIELD_NONE;
                 buf->flags=V4L2_BUF_FLAG_MAPPED | V4L2_BUF_FLAG_PREPARED;
                 buf->m.offset=size*buf->index;
                 buf->sequence=0;
                 buf->length=size;
                 buf->bytesused=0;
                 memset(&buf->timecode, 0x00, sizeof(buf->timecode));

                 /* Now search this buffer in the output queue */
                 if (dev->output_buffer[subdev].mutex_inited)
                 {
                     pthread_mutex_lock(&dev->output_buffer[subdev].access);
                 }
                 TAILQ_FOREACH_SAFE(entry, &dev->output_buffer[subdev].head, link, tentry)
                 {
                     /* If buffer has been found, fill the real data */
                     if ((entry!=NULL) && (entry->buffer.index==buf->index))
                     {
                         buf->flags|=V4L2_BUF_FLAG_DONE | V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;
                         buf->sequence=entry->buffer.sequence;
                         buf->bytesused=entry->buffer.bytesused;
                         buf->timestamp=entry->buffer.timestamp;
                         break;
                     }
                 }
                 if (dev->output_buffer[subdev].mutex_inited)
                 {
                     pthread_mutex_unlock(&dev->output_buffer[subdev].access);
                 }

                 /* Now search this buffer in the input queue */
                 if (dev->input_buffer[subdev].mutex_inited)
                 {
                     pthread_mutex_lock(&dev->input_buffer[subdev].access);
                 }
                 TAILQ_FOREACH_SAFE(entry, &dev->input_buffer[subdev].head, link, tentry)
                 {
                     /* If buffer has been found, fill the real data */
                     if ((entry!=NULL) && (entry->buffer.index==buf->index))
                     {
                         buf->flags|=V4L2_BUF_FLAG_QUEUED;
                         buf->sequence=entry->buffer.sequence;
                         buf->bytesused=entry->buffer.bytesused;
                         buf->timestamp=entry->buffer.timestamp;
                         break;
                     }
                 }
                 if (dev->input_buffer[subdev].mutex_inited)
                 {
                     pthread_mutex_unlock(&dev->input_buffer[subdev].access);
                 }

                 if (uvc_verbose>2)
                 {
                    slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        m.offset: %d (0x%08X)", buf->m.offset, buf->m.offset);
                    slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        length: %d (0x%08X)", buf->length, buf->length);
                 }

                 dctldatasize=sizeof(*buf);
             }
             break;
        case VIDIOC_QBUF:
             {
                 struct v4l2_buffer* buf;
                 struct _uvc_buffer_entry* entry;
                 struct _uvc_buffer_entry* tentry;

                 buf=(struct v4l2_buffer*)dptr;
                 memset(&buf->reserved, 0x00, sizeof(buf->reserved));
                 memset(&buf->reserved2, 0x00, sizeof(buf->reserved2));

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_QBUF:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        index: %d", buf->index);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        type: %d", buf->type);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        memory: %d", buf->memory);
                 }

                 if (buf->type!=V4L2_BUF_TYPE_VIDEO_CAPTURE)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: buffer type is not V4L2_BUF_TYPE_VIDEO_CAPTURE");
                     }
                     ret=EINVAL;
                     break;
                 }

                 if (buf->memory!=V4L2_MEMORY_MMAP)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: buffer i/o type is not V4L2_MEMORY_MMAP");
                     }
                     ret=EINVAL;
                     break;
                 }

                 /* Check requested buffer index */
                 if (buf->index>=dev->current_buffer_count[subdev])
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: buffer index is out of range (%d of %d)", buf->index, dev->current_buffer_count[subdev]);
                     }
                     ret=EINVAL;
                     break;
                 }

                 /* Search this buffer in the input queue */
                 if (dev->input_buffer[subdev].mutex_inited)
                 {
                     pthread_mutex_lock(&dev->input_buffer[subdev].access);
                 }
                 TAILQ_FOREACH_SAFE(entry, &dev->input_buffer[subdev].head, link, tentry)
                 {
                     /* If buffer has been found return an ERROR */
                     if ((entry!=NULL) && (entry->buffer.index==buf->index))
                     {
                         if (uvc_verbose>2)
                         {
                             slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EBUSY: buffer %d is alrady queued", buf->index);
                         }
                         ret=EBUSY;
                         break;
                     }
                 }
                 if (dev->input_buffer[subdev].mutex_inited)
                 {
                     pthread_mutex_unlock(&dev->input_buffer[subdev].access);
                 }

                 if (ret!=EOK)
                 {
                     break;
                 }

                 entry=calloc(1, sizeof(struct _uvc_buffer_entry));
                 if (entry==NULL)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        ENOMEM: no memory to enqueue buffer %d", buf->index);
                     }
                     ret=ENOMEM;
                     break;
                 }

                 buf->flags&=~(V4L2_BUF_FLAG_DONE);
                 buf->flags|=V4L2_BUF_FLAG_QUEUED;
                 buf->sequence=0;
                 buf->bytesused=0;

                 entry->buffer=*buf;

                 /* Enqueue buffer */
                 if (dev->input_buffer[subdev].mutex_inited)
                 {
                     pthread_mutex_lock(&dev->input_buffer[subdev].access);
                 }
                 TAILQ_INSERT_TAIL(&dev->input_buffer[subdev].head, entry, link);
                 if (dev->input_buffer[subdev].mutex_inited)
                 {
                     pthread_mutex_unlock(&dev->input_buffer[subdev].access);
                 }

                 dctldatasize=sizeof(*buf);
             }
             break;
        case VIDIOC_DQBUF:
             {
                 struct v4l2_buffer* buf;
                 struct _uvc_buffer_entry* entry;
                 int nonblocking=0;
                 struct timespec ts={0, 1};

                 buf=(struct v4l2_buffer*)dptr;
                 memset(&buf->reserved, 0x00, sizeof(buf->reserved));
                 memset(&buf->reserved2, 0x00, sizeof(buf->reserved2));

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_DQBUF:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        type: %d", buf->type);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        memory: %d", buf->memory);
                 }

                 if (buf->type!=V4L2_BUF_TYPE_VIDEO_CAPTURE)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: buffer type is not V4L2_BUF_TYPE_VIDEO_CAPTURE");
                     }
                     ret=EINVAL;
                     break;
                 }

                 if (buf->memory!=V4L2_MEMORY_MMAP)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: buffer i/o type is not V4L2_MEMORY_MMAP");
                     }
                     ret=EINVAL;
                     break;
                 }

                 /* Check if file descriptor was opened in non-blocking mode */
                 if ((ocb->hdr.ioflag & O_NONBLOCK)==O_NONBLOCK)
                 {
                     nonblocking=1;
                 }

                 /* Search this buffer in the input queue */
                 do {
                     if (dev->output_buffer[subdev].mutex_inited)
                     {
                         pthread_mutex_lock(&dev->output_buffer[subdev].access);
                     }
                     if (TAILQ_EMPTY(&dev->output_buffer[subdev].head))
                     {
                         if (nonblocking)
                         {
                             /* Yield a bit and then return EAGAIN     */
                             /* Otherwise CPU usage will be about 100% */
                             /* during such kind of polling mode.      */
                             nanosleep(&ts, NULL);
                             ret=EAGAIN;
                             if (dev->output_buffer[subdev].mutex_inited)
                             {
                                 pthread_mutex_unlock(&dev->output_buffer[subdev].access);
                             }
                             break;
                         }
                     }
                     else
                     {
                         /* There is at least one buffer is awaiting in the outgoing queue */
                         entry=TAILQ_FIRST(&dev->output_buffer[subdev].head);
                         if (entry!=NULL)
                         {
                             *buf=entry->buffer;
                             TAILQ_REMOVE(&dev->output_buffer[subdev].head, entry, link);
                             free(entry);
                             if (dev->output_buffer[subdev].mutex_inited)
                             {
                                 pthread_mutex_unlock(&dev->output_buffer[subdev].access);
                             }
                             break;
                         }
                     }
                     if (dev->output_buffer[subdev].mutex_inited)
                     {
                         pthread_mutex_unlock(&dev->output_buffer[subdev].access);
                     }

                     /* wait for the new buffer in the blocked state */
                     nanosleep(&ts, NULL);
                 } while(1);

                 if (ret!=EOK)
                 {
                     break;
                 }

                 /* at this place the buffer is dequeued */
                 buf->flags&=~(V4L2_BUF_FLAG_QUEUED);
                 buf->flags|=V4L2_BUF_FLAG_DONE;

                 dctldatasize=sizeof(*buf);
             }
             break;
        case VIDIOC_STREAMON:
             {
                 int* type;
                 int formatindex=0;
                 int frameindex=0;
                 int frameinterval=0;
                 int framesize=0;
                 probe_commit_control_t ctrl;
                 usbd_interface_descriptor_t* uvc_interface_descriptor;
                 usbd_descriptors_t* uvc_descriptor;
                 struct usbd_desc_node* uvc_node;
                 struct usbd_desc_node* uvc_node2;
                 int ctrl_length=0;
                 uint64_t estimated_payload_size=0;
                 int match=0;
                 int best_payload_size=INT_MAX;

                 type=(int*)dptr;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_STREAMON:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        type: %d", *type);
                 }

                 if (*type!=V4L2_BUF_TYPE_VIDEO_CAPTURE)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: buffer type is not V4L2_BUF_TYPE_VIDEO_CAPTURE");
                     }
                     ret=EINVAL;
                     break;
                 }

                 /* Check the priority */
                 if ((dev->current_priority_ocb[subdev]!=NULL) && (dev->current_priority_ocb[subdev]!=ocb))
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EBUSY: this is not a high priority file descriptor");
                     }
                     ret=EINVAL;
                     break;
                 }

                 /* Check if transfer is in progress, then do nothing */
                 if (dev->current_transfer[subdev])
                 {
                     ret=EOK;
                     break;
                 }

                 /* Check if we could perform streaming, we need at least one queued buffer */
                 if (dev->input_buffer[subdev].mutex_inited)
                 {
                     pthread_mutex_lock(&dev->input_buffer[subdev].access);
                 }
                 if (TAILQ_EMPTY(&dev->input_buffer[subdev].head))
                 {
                     ret=EIO;
                     if (dev->input_buffer[subdev].mutex_inited)
                     {
                         pthread_mutex_unlock(&dev->input_buffer[subdev].access);
                     }
                     break;
                 }
                 if (dev->input_buffer[subdev].mutex_inited)
                 {
                     pthread_mutex_unlock(&dev->input_buffer[subdev].access);
                 }

                 /* Initiate the transfer */
                 switch (dev->current_pixelformat[subdev])
                 {
                     case V4L2_PIX_FMT_YUYV:
                     case V4L2_PIX_FMT_UYVY:
                     case V4L2_PIX_FMT_YVYU:
                     case V4L2_PIX_FMT_VYUY:
                     case V4L2_PIX_FMT_NV12:
                          formatindex=dev->vs_format_uncompressed[subdev].bFormatIndex;
                          for (it=0; it<dev->vs_format_uncompressed[subdev].bNumFrameDescriptors; it++)
                          {
                              if ((dev->vs_frame_uncompressed[subdev][it].wWidth==dev->current_width[subdev]) &&
                                  (dev->vs_frame_uncompressed[subdev][it].wHeight==dev->current_height[subdev]))
                              {
                                  frameindex=dev->vs_frame_uncompressed[subdev][it].bFrameIndex;
                                  framesize=dev->vs_frame_uncompressed[subdev][it].dwMaxVideoFrameBufferSize;
                                  break;
                              }
                          }
                          break;
                     case V4L2_PIX_FMT_MJPEG:
                          formatindex=dev->vs_format_mjpeg[subdev].bFormatIndex;
                          for (it=0; it<dev->vs_format_mjpeg[subdev].bNumFrameDescriptors; it++)
                          {
                              if ((dev->vs_frame_mjpeg[subdev][it].wWidth==dev->current_width[subdev]) &&
                                  (dev->vs_frame_mjpeg[subdev][it].wHeight==dev->current_height[subdev]))
                              {
                                  frameindex=dev->vs_frame_mjpeg[subdev][it].bFrameIndex;
                                  framesize=dev->vs_frame_mjpeg[subdev][it].dwMaxVideoFrameBufferSize;
                                  break;
                              }
                          }
                          break;
                     case V4L2_PIX_FMT_H264:
                          formatindex=dev->vs_format_h264f[subdev].bFormatIndex;
                          for (it=0; it<dev->vs_format_h264f[subdev].bNumFrameDescriptors; it++)
                          {
                              if ((dev->vs_frame_h264f[subdev][it].wWidth==dev->current_width[subdev]) &&
                                  (dev->vs_frame_h264f[subdev][it].wHeight==dev->current_height[subdev]))
                              {
                                  frameindex=dev->vs_frame_h264f[subdev][it].bFrameIndex;
                                  framesize=dev->vs_frame_h264f[subdev][it].dwBytesPerLine *
                                            dev->vs_frame_h264f[subdev][it].wHeight;
                              }
                          }
                          break;
                     default:
                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "[devu-uvc] unsupported internal pixel format, please report!");
                          break;
                 }
                 frameinterval=dev->current_frameinterval[subdev];

                 /* Universal USB bandwidth calculation for isochronous and bulk transfers */
                 estimated_payload_size=(uint64_t)framesize * (10000000000ULL / (uint64_t)frameinterval);
                 estimated_payload_size/=1000000;
                 if (dev->port_speed==2)
                 {
                     /* TODO: For bulk, add +=14 and remove /=8 */
                     estimated_payload_size/=8;
                     estimated_payload_size+=11;
                 }
                 else
                 {
                     estimated_payload_size+=98;
                 }
                 if (estimated_payload_size<1024)
                 {
                     estimated_payload_size=1024;
                 }

                 memset(&ctrl, 0x00, sizeof(ctrl));
                 ctrl.bmHint=UVC_PROBE_COMMIT_HINT_DWFRAMEINTERVAL;
                 ctrl.bFormatIndex=formatindex;
                 ctrl.bFrameIndex=frameindex;
                 ctrl.dwFrameInterval=frameinterval;
                 ctrl.dwMaxVideoFrameSize=framesize;
                 ctrl.dwMaxPayloadTransferSize=estimated_payload_size;

                 switch (dev->vc_header.bcdUVC)
                 {
                     case 0x0100:
                          ctrl_length=UVC_PROBE_COMMIT_VER10_SIZE;
                          break;
                     case 0x0101:
                          ctrl_length=UVC_PROBE_COMMIT_VER11_SIZE;
                          ctrl.bmFramingInfo=UVC_PROBE_COMMIT_BMFRAMINGINFO_FID |
                                             UVC_PROBE_COMMIT_BMFRAMINGINFO_EOF;
                          break;
                     case 0x0105:
                          ctrl_length=UVC_PROBE_COMMIT_VER15_SIZE;
                          break;
                 }

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "      PROBE set (packet size %d):", ctrl_length);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        bmHint: %d", ctrl.bmHint);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        bFormatIndex: %d", ctrl.bFormatIndex);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        bFrameIndex: %d", ctrl.bFrameIndex);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        dwFrameInterval: %d", ctrl.dwFrameInterval);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        dwMaxVideoFrameSize: %d", ctrl.dwMaxVideoFrameSize);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        dwMaxPayloadTransferSize: %d", ctrl.dwMaxPayloadTransferSize);
                     if (ctrl_length>UVC_PROBE_COMMIT_VER10_SIZE)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        bmFramingInfo: %d", ctrl.bmFramingInfo);
                     }
                 }

                 /* Parse interfaces and alternative configurations to find the  */
                 /* first non-zero pipe configuration which satisfies our needs. */
                 match=0;
                 for (jt=0; ; jt++)
                 {
                     uvc_interface_descriptor=usbd_interface_descriptor(dev->uvc_vs_device[subdev], dev->vs_usb_config[subdev], dev->vs_usb_iface[subdev], jt, &uvc_node);
                     if (uvc_interface_descriptor==NULL)
                     {
                         break;
                     }
                     for (it=0; ; it++)
                     {
                         uvc_descriptor=usbd_parse_descriptors(dev->uvc_vs_device[subdev], uvc_node, USB_DESC_ENDPOINT, it, &uvc_node2);
                         if (uvc_descriptor==NULL)
                         {
                             break;
                         }
                         /* QNX USB stack do not support Asychronous/No synchronization   */
                         /* isochronous endpoints macros for detection. So use 0x03 mask. */
                         switch (uvc_descriptor->endpoint.bmAttributes & 0x03)
                         {
                             case USB_ATTRIB_ISOCHRONOUS:
                                  if (estimated_payload_size<=uvc_descriptor->endpoint.wMaxPacketSize)
                                  {
                                      match=1;
                                      status=usbd_select_interface(dev->uvc_vs_device[subdev], dev->vs_usb_iface[subdev], jt);
                                      if (status!=EOK)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't select interface");
                                      }
                                  }
                                  break;
                             case USB_ATTRIB_BULK:
                                  slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Bulk transfer is not supported, please report");
                                  break;
                         }
                         if (match)
                         {
                             break;
                         }
                     }
                     if (match)
                     {
                         break;
                     }
                 }
                 if (!match)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't fit video stream to available bandwidth");
                     ret=ENOSPC;
                     break;
                 }

                 /* Probe control */
                 if (!status)
                 {
                     status|=uvc_probe_commit_set(dev, subdev, VSET_CUR, dev->vs_usb_iface[subdev],
                         VVS_PROBE_CONTROL, ctrl_length, (uint8_t*)&ctrl);
                 }
                 if (!status)
                 {
                     status|=uvc_probe_commit_get(dev, subdev, VGET_CUR, dev->vs_usb_iface[subdev],
                         VVS_PROBE_CONTROL, ctrl_length, (uint8_t*)&ctrl);
                 }

                 /* Commit control */
                 if (!status)
                 {
                     status|=uvc_probe_commit_set(dev, subdev, VSET_CUR, dev->vs_usb_iface[subdev],
                         VVS_COMMIT_CONTROL, ctrl_length, (uint8_t*)&ctrl);
                 }
                 if (!status)
                 {
                     status|=uvc_probe_commit_get(dev, subdev, VGET_CUR, dev->vs_usb_iface[subdev],
                         VVS_COMMIT_CONTROL, ctrl_length, (uint8_t*)&ctrl);
                 }
                 if (ctrl.dwMaxVideoFrameSize==0)
                 {
                     ctrl.dwMaxVideoFrameSize=framesize;
                 }
                 if (ctrl.dwMaxPayloadTransferSize==0)
                 {
                     ctrl.dwMaxPayloadTransferSize=estimated_payload_size;
                 }
                 estimated_payload_size=ctrl.dwMaxPayloadTransferSize;

                 if (status!=0)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EIO: USB i/o error");
                     }
                     ret=EIO;
                     break;
                 }

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "      COMMIT get:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        bmHint: %d", ctrl.bmHint);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        bFormatIndex: %d", ctrl.bFormatIndex);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        bFrameIndex: %d", ctrl.bFrameIndex);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        dwFrameInterval: %d", ctrl.dwFrameInterval);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        wKeyFrameRate: %d", ctrl.wKeyFrameRate);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        wPFrameRate: %d", ctrl.wPFrameRate);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        wCompQuality: %d", ctrl.wCompQuality);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        wCompWindowSize: %d", ctrl.wCompWindowSize);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        wDelay: %d", ctrl.wDelay);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        dwMaxVideoFrameSize: %d", ctrl.dwMaxVideoFrameSize);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        dwMaxPayloadTransferSize: %d", ctrl.dwMaxPayloadTransferSize);
                     if (ctrl_length>UVC_PROBE_COMMIT_VER10_SIZE)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        dwClockFrequency: %d", ctrl.dwClockFrequency);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        bmFramingInfo: %d", ctrl.bmFramingInfo);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        bPreferedVersion: %d", ctrl.bPreferedVersion);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        bMinVersion: %d", ctrl.bMinVersion);
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        bMaxVersion: %d", ctrl.bMaxVersion);
                     }
                 }

                 /* Parse all alternative configurations to find best payload size */
                 for (jt=0; ; jt++)
                 {
                     uvc_interface_descriptor=usbd_interface_descriptor(dev->uvc_vs_device[subdev], dev->vs_usb_config[subdev], dev->vs_usb_iface[subdev], jt, &uvc_node);
                     if (uvc_interface_descriptor==NULL)
                     {
                         break;
                     }
                     for (it=0; ; it++)
                     {
                         uvc_descriptor=usbd_parse_descriptors(dev->uvc_vs_device[subdev], uvc_node, USB_DESC_ENDPOINT, it, &uvc_node2);
                         if (uvc_descriptor==NULL)
                         {
                             break;
                         }
                         /* QNX USB stack do not support Asychronous/No synchronization   */
                         /* isochronous endpoints macros for detection. So use 0x03 mask. */
                         switch (uvc_descriptor->endpoint.bmAttributes & 0x03)
                         {
                             case USB_ATTRIB_ISOCHRONOUS:
                                  if (abs(estimated_payload_size-uvc_descriptor->endpoint.wMaxPacketSize)<
                                      abs(estimated_payload_size-best_payload_size))
                                  {
                                      best_payload_size=uvc_descriptor->endpoint.wMaxPacketSize;
                                  }
                                  break;
                             case USB_ATTRIB_BULK:
                                  slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Bulk transfer is not supported, please report");
                                  break;
                         }
                     }
                 }

                 /* Parse interfaces and alternative configurations to find the  */
                 /* first non-zero pipe configuration which satisfies our needs. */
                 match=0;
                 for (jt=0; ; jt++)
                 {
                     uvc_interface_descriptor=usbd_interface_descriptor(dev->uvc_vs_device[subdev], dev->vs_usb_config[subdev], dev->vs_usb_iface[subdev], jt, &uvc_node);
                     if (uvc_interface_descriptor==NULL)
                     {
                         break;
                     }
                     for (it=0; ; it++)
                     {
                         uvc_descriptor=usbd_parse_descriptors(dev->uvc_vs_device[subdev], uvc_node, USB_DESC_ENDPOINT, it, &uvc_node2);
                         if (uvc_descriptor==NULL)
                         {
                             break;
                         }
                         /* QNX USB stack do not support Asychronous/No synchronization   */
                         /* isochronous endpoints macros for detection. So use 0x03 mask. */
                         switch (uvc_descriptor->endpoint.bmAttributes & 0x03)
                         {
                             case USB_ATTRIB_ISOCHRONOUS:
                                  if (best_payload_size==uvc_descriptor->endpoint.wMaxPacketSize)
                                  {
                                      match=1;
                                      if (dev->vs_isochronous_pipe[subdev]!=NULL)
                                      {
                                          usbd_close_pipe(dev->vs_isochronous_pipe[subdev]);
                                          dev->vs_isochronous_pipe[subdev]=NULL;
                                      }
                                      status=usbd_open_pipe(dev->uvc_vs_device[subdev], uvc_descriptor, &dev->vs_isochronous_pipe[subdev]);
                                      if (status!=EOK)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't open VS isochronous pipe");
                                      }
                                      status=usbd_select_interface(dev->uvc_vs_device[subdev], dev->vs_usb_iface[subdev], jt);
                                      if (status!=EOK)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't select interface");
                                      }
                                      if (uvc_verbose>2)
                                      {
                                          slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        Alt iface %d has been selected (%d of %d)", jt, (uint32_t)estimated_payload_size, uvc_descriptor->endpoint.wMaxPacketSize);
                                      }
                                  }
                                  break;
                             case USB_ATTRIB_BULK:
                                  slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Bulk transfer is not supported, please report");
                                  break;
                         }
                         if (match)
                         {
                             break;
                         }
                     }
                     if (match)
                     {
                         break;
                     }
                 }
                 if (!match)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't fit video stream to available bandwidth");
                     ret=ENOSPC;
                     break;
                 }

                 /* Allocate isochronous packets */
                 for (it=0; it<UVC_MAX_ISO_BUFFERS; it++)
                 {
                     /* Free any allocated resources before */
                     if (dev->iso_list[subdev][it]!=NULL)
                     {
                         usbd_free_isochronous_frame_list(dev->iso_list[subdev][it]);
                         dev->iso_list[subdev][it]=NULL;
                     }
                     if (dev->iso_urb[subdev][it]!=NULL)
                     {
                         usbd_free_urb(dev->iso_urb[subdev][it]);
                         dev->iso_urb[subdev][it]=NULL;
                     }
                     if (dev->iso_buffer[subdev][it]!=NULL)
                     {
                         usbd_free(dev->iso_buffer[subdev][it]);
                         dev->iso_buffer[subdev][it]=NULL;
                     }

                     /* Allocate new resources */
                     status=usbd_alloc_isochronous_frame_list(UVC_MAX_ISO_FRAMES, &dev->iso_list[subdev][it]);
                     if (status!=EOK)
                     {
                         ret=ENOMEM;
                         break;
                     }
                     dev->iso_payload_size[subdev]=best_payload_size;
                     for (jt=0; jt<UVC_MAX_ISO_FRAMES; jt++)
                     {
                         dev->iso_list[subdev][it][jt].frame_status=0;
                         dev->iso_list[subdev][it][jt].frame_len=best_payload_size;
                     }
                     dev->iso_urb[subdev][it]=usbd_alloc_urb(NULL);
                     if (dev->iso_urb[subdev][it]==NULL)
                     {
                         ret=ENOMEM;
                         break;
                     }
                     dev->iso_buffer[subdev][it]=usbd_alloc(UVC_MAX_ISO_FRAMES*best_payload_size);
                     if (dev->iso_buffer[subdev][it]==NULL)
                     {
                         ret=ENOMEM;
                         break;
                     }
                     status=usbd_setup_isochronous_stream(dev->iso_urb[subdev][it],
                         URB_DIR_IN | URB_ISOCH_ASAP, 0, dev->iso_buffer[subdev][it],
                         UVC_MAX_ISO_FRAMES*best_payload_size, dev->iso_list[subdev][it],
                         UVC_MAX_ISO_FRAMES);
                     if (status!=EOK)
                     {
                         if (uvc_verbose>2)
                         {
                             slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        ENOMEM: can't allocate memory for isochronous operations");
                         }
                         ret=ENOMEM;
                         break;
                     }
                 }

                 /* Fire all packets at once */
                 status=0;
                 for (it=0; it<UVC_MAX_ISO_BUFFERS; it++)
                 {
                     status|=usbd_io(dev->iso_urb[subdev][it], dev->vs_isochronous_pipe[subdev],
                         uvc_isochronous_completion, dev, USBD_TIME_INFINITY);
                 }

                 if (status)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EIO: USB i/o error");
                     }
                     ret=EIO;
                     break;
                 }
                 else
                 {
                     dev->current_transfer[subdev]=1;
                 }
             }
             break;
        case VIDIOC_STREAMOFF:
             {
                 int* type;
                 struct _uvc_buffer_entry* entry;
                 struct _uvc_buffer_entry* tentry;

                 type=(int*)dptr;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_STREAMOFF:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        type: %d", *type);
                 }

                 if (*type!=V4L2_BUF_TYPE_VIDEO_CAPTURE)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: buffer type is not V4L2_BUF_TYPE_VIDEO_CAPTURE");
                     }
                     ret=EINVAL;
                     break;
                 }

                 /* Check the priority */
                 if ((dev->current_priority_ocb[subdev]!=NULL) && (dev->current_priority_ocb[subdev]!=ocb))
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EBUSY: this is not a high priority file descriptor");
                     }
                     ret=EINVAL;
                     break;
                 }

                 /* Check if transfer is not in progress, then do nothing */
                 if (!dev->current_transfer[subdev])
                 {
                     ret=EOK;
                     break;
                 }

                 /* Stop the transfer */
                 dev->current_transfer[subdev]=0;

                 /* Drain input and output queues, all information will be lost */
                 if (dev->input_buffer[subdev].mutex_inited)
                 {
                     pthread_mutex_lock(&dev->input_buffer[subdev].access);
                 }
                 if (dev->output_buffer[subdev].mutex_inited)
                 {
                     pthread_mutex_lock(&dev->output_buffer[subdev].access);
                 }
                 TAILQ_FOREACH_SAFE(entry, &dev->input_buffer[subdev].head, link, tentry)
                 {
                     TAILQ_REMOVE(&dev->input_buffer[subdev].head, entry, link);
                     if (entry!=NULL)
                     {
                         free(entry);
                     }
                 }
                 TAILQ_FOREACH_SAFE(entry, &dev->output_buffer[subdev].head, link, tentry)
                 {
                     TAILQ_REMOVE(&dev->output_buffer[subdev].head, entry, link);
                     if (entry!=NULL)
                     {
                         free(entry);
                     }
                 }
                 if (dev->input_buffer[subdev].mutex_inited)
                 {
                     pthread_mutex_unlock(&dev->input_buffer[subdev].access);
                 }
                 if (dev->output_buffer[subdev].mutex_inited)
                 {
                     pthread_mutex_unlock(&dev->output_buffer[subdev].access);
                 }
             }
             break;
        case VIDIOC_G_PARM:
             {
                 struct v4l2_streamparm* parm;

                 parm=(struct v4l2_streamparm*)dptr;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_G_PARM:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        type: %d", parm->type);
                 }

                 if (parm->type!=V4L2_BUF_TYPE_VIDEO_CAPTURE)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: buffer type is not V4L2_BUF_TYPE_VIDEO_CAPTURE");
                     }
                     ret=EINVAL;
                     break;
                 }

                 memset(&parm->parm.capture.reserved, 0x00, sizeof(parm->parm.capture.reserved));
                 parm->parm.capture.capability=V4L2_CAP_TIMEPERFRAME;
                 parm->parm.capture.capturemode=0;
                 parm->parm.capture.timeperframe.numerator=dev->current_frameinterval[subdev];
                 parm->parm.capture.timeperframe.denominator=10000000;
                 parm->parm.capture.extendedmode=0;
                 parm->parm.capture.readbuffers=0;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        capability: %d", parm->parm.capture.capability);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        capturemode: %d", parm->parm.capture.capturemode);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        timeperframe.numerator: %d", parm->parm.capture.timeperframe.numerator);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        timeperframe.denominator: %d", parm->parm.capture.timeperframe.denominator);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        extendedmode: %d", parm->parm.capture.extendedmode);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        readbuffers: %d", parm->parm.capture.readbuffers);
                 }

                 dctldatasize=sizeof(*parm);
             }
             break;
        case VIDIOC_S_PARM:
             {
                 struct v4l2_streamparm* parm;
                 uint64_t best_frameinterval=LONGLONG_MAX;
                 unsigned int best_frameinterval_num=INT_MAX;
                 uint64_t kt;

                 parm=(struct v4l2_streamparm*)dptr;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_S_PARM:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        type: %d", parm->type);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        capability: %d", parm->parm.capture.capability);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        capturemode: %d", parm->parm.capture.capturemode);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        timeperframe.numerator: %d", parm->parm.capture.timeperframe.numerator);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        timeperframe.denominator: %d", parm->parm.capture.timeperframe.denominator);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        extendedmode: %d", parm->parm.capture.extendedmode);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        readbuffers: %d", parm->parm.capture.readbuffers);
                 }

                 if (parm->type!=V4L2_BUF_TYPE_VIDEO_CAPTURE)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: buffer type is not V4L2_BUF_TYPE_VIDEO_CAPTURE");
                     }
                     ret=EINVAL;
                     break;
                 }

                 memset(&parm->parm.capture.reserved, 0x00, sizeof(parm->parm.capture.reserved));
                 parm->parm.capture.capability=V4L2_CAP_TIMEPERFRAME;
                 if (parm->parm.capture.capturemode & V4L2_MODE_HIGHQUALITY)
                 {
                     parm->parm.capture.capturemode=V4L2_MODE_HIGHQUALITY;
                 }
                 else
                 {
                     parm->parm.capture.capturemode=0;
                 }
                 parm->parm.capture.extendedmode=0;
                 parm->parm.capture.readbuffers=0;

                 if ((parm->parm.capture.timeperframe.numerator==0) &&
                     (parm->parm.capture.timeperframe.denominator==0))
                 {
                     parm->parm.capture.timeperframe.numerator=dev->current_frameinterval[subdev];
                     parm->parm.capture.timeperframe.denominator=10000000;
                 }
                 else
                 {
                     uint64_t desired_frameinterval=
                         (((uint64_t)parm->parm.capture.timeperframe.denominator)<<32) /
                         parm->parm.capture.timeperframe.numerator;

                     /* Find a suitable frame rate */
                     switch (dev->current_pixelformat[subdev])
                     {
                         case V4L2_PIX_FMT_YUYV:
                         case V4L2_PIX_FMT_UYVY:
                         case V4L2_PIX_FMT_YVYU:
                         case V4L2_PIX_FMT_VYUY:
                         case V4L2_PIX_FMT_NV12:
                              for (it=0; it<dev->vs_format_uncompressed[subdev].bNumFrameDescriptors; it++)
                              {
                                  if ((dev->vs_frame_uncompressed[subdev][it].wWidth==dev->current_width[subdev]) &&
                                      (dev->vs_frame_uncompressed[subdev][it].wHeight==dev->current_height[subdev]))
                                  {
                                      if (dev->vs_frame_uncompressed[subdev][it].bFrameIntervalType)
                                      {
                                          for (kt=0; kt<dev->vs_frame_uncompressed[subdev][it].bFrameIntervalType; kt++)
                                          {
                                              uint64_t value=((uint64_t)10000000ULL<<32) /
                                                  dev->vs_frame_uncompressed[subdev][it].dwFrameInterval[kt];

                                              if (abs(desired_frameinterval-value)<abs(best_frameinterval-value))
                                              {
                                                  best_frameinterval=value;
                                                  best_frameinterval_num=dev->vs_frame_uncompressed[subdev][it].dwFrameInterval[kt];
                                              }
                                          }
                                      }
                                      else
                                      {
                                          for (kt=dev->vs_frame_uncompressed[subdev][it].dwMinFrameInterval;
                                               kt<=dev->vs_frame_uncompressed[subdev][it].dwMaxFrameInterval;
                                               kt+=dev->vs_frame_uncompressed[subdev][it].dwFrameIntervalStep)
                                          {
                                              uint64_t value=((uint64_t)10000000ULL<<32) / kt;

                                              if (abs(desired_frameinterval-value)<abs(best_frameinterval-value))
                                              {
                                                  best_frameinterval=value;
                                                  best_frameinterval_num=kt;
                                              }
                                          }
                                      }
                                  }
                              }
                              break;
                         case V4L2_PIX_FMT_MJPEG:
                              for (it=0; it<dev->vs_format_mjpeg[subdev].bNumFrameDescriptors; it++)
                              {
                                  if ((dev->vs_frame_mjpeg[subdev][it].wWidth==dev->current_width[subdev]) &&
                                      (dev->vs_frame_mjpeg[subdev][it].wHeight==dev->current_height[subdev]))
                                  {
                                      if (dev->vs_frame_mjpeg[subdev][it].bFrameIntervalType)
                                      {
                                          for (kt=0; kt<dev->vs_frame_mjpeg[subdev][it].bFrameIntervalType; kt++)
                                          {
                                              uint64_t value=((uint64_t)10000000ULL<<32) /
                                                  dev->vs_frame_mjpeg[subdev][it].dwFrameInterval[kt];

                                              if (abs(desired_frameinterval-value)<abs(best_frameinterval-value))
                                              {
                                                  best_frameinterval=value;
                                                  best_frameinterval_num=dev->vs_frame_mjpeg[subdev][it].dwFrameInterval[kt];
                                              }
                                          }
                                      }
                                      else
                                      {
                                          for (kt=dev->vs_frame_mjpeg[subdev][it].dwMinFrameInterval;
                                               kt<=dev->vs_frame_mjpeg[subdev][it].dwMaxFrameInterval;
                                               kt+=dev->vs_frame_mjpeg[subdev][it].dwFrameIntervalStep)
                                          {
                                              uint64_t value=((uint64_t)10000000ULL<<32) / kt;

                                              if (abs(desired_frameinterval-value)<abs(best_frameinterval-value))
                                              {
                                                  best_frameinterval=value;
                                                  best_frameinterval_num=kt;
                                              }
                                          }
                                      }
                                  }
                              }
                              break;
                         case V4L2_PIX_FMT_H264:
                              for (it=0; it<dev->vs_format_h264f[subdev].bNumFrameDescriptors; it++)
                              {
                                  if ((dev->vs_frame_h264f[subdev][it].wWidth==dev->current_width[subdev]) &&
                                      (dev->vs_frame_h264f[subdev][it].wHeight==dev->current_height[subdev]))
                                  {
                                      if (dev->vs_frame_h264f[subdev][it].bFrameIntervalType)
                                      {
                                          for (kt=0; kt<dev->vs_frame_h264f[subdev][it].bFrameIntervalType; kt++)
                                          {
                                              uint64_t value=((uint64_t)10000000ULL<<32) /
                                                  dev->vs_frame_h264f[subdev][it].dwFrameInterval[kt];

                                              if (abs(desired_frameinterval-value)<abs(best_frameinterval-value))
                                              {
                                                  best_frameinterval=value;
                                                  best_frameinterval_num=dev->vs_frame_h264f[subdev][it].dwFrameInterval[kt];
                                              }
                                          }
                                      }
                                      else
                                      {
                                          for (kt=dev->vs_frame_h264f[subdev][it].dwMinFrameInterval;
                                               kt<=dev->vs_frame_h264f[subdev][it].dwMaxFrameInterval;
                                               kt+=dev->vs_frame_h264f[subdev][it].dwFrameIntervalStep)
                                          {
                                              uint64_t value=((uint64_t)10000000ULL<<32) / kt;

                                              if (abs(desired_frameinterval-value)<abs(best_frameinterval-value))
                                              {
                                                  best_frameinterval=value;
                                                  best_frameinterval_num=kt;
                                              }
                                          }
                                      }
                                  }
                              }
                              break;
                     }
                     parm->parm.capture.timeperframe.numerator=best_frameinterval_num;
                     parm->parm.capture.timeperframe.denominator=10000000;
                 }

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "      on return:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        type: %d", parm->type);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        capability: %d", parm->parm.capture.capability);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        capturemode: %d", parm->parm.capture.capturemode);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        timeperframe.numerator: %d", parm->parm.capture.timeperframe.numerator);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        timeperframe.denominator: %d", parm->parm.capture.timeperframe.denominator);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        extendedmode: %d", parm->parm.capture.extendedmode);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        readbuffers: %d", parm->parm.capture.readbuffers);
                 }

                 dctldatasize=sizeof(*parm);
             }
             break;
        case VIDIOC_ENUMSTD:
             {
                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_ENUMSTD:");
                 }

                 /* Output and input enumeration are not supported */
                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        ENODATA: Can't enumerate video standard (according to UVC standard)");
                 }
                 ret=ENODATA;
                 break;
             }
             break;
        case VIDIOC_S_STD:
             {
                 v4l2_std_id* id;

                 id=(v4l2_std_id*)dptr;

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_S_STD:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        index: %08llX", *id);
                 }

                 /* Output and input enumeration are not supported */
                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        ENODATA: Can't set video standard (according to UVC standard)");
                 }
                 ret=ENODATA;
                 break;
             }
             break;
        case VIDIOC_G_STD:
             {
                 v4l2_std_id* id;

                 status=0;
                 id=(v4l2_std_id*)dptr;
                 *id=V4L2_STD_UNKNOWN;

                 dctldatasize=sizeof(*id);

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_G_STD:");
                 }

                 if ((dev->vc_processing_unit.bmControls[2] &
                     (VPU_B2_ANALOG_VIDEO_STANDARD | VPU_B2_ANALOG_VIDEO_LOCK_STATUS))==
                     (VPU_B2_ANALOG_VIDEO_STANDARD | VPU_B2_ANALOG_VIDEO_LOCK_STATUS))
                 {
                     unsigned char value;

                     status|=uvc_control_get(dev, VGET_CUR, UVC_PUNIT_SELECTOR,
                         VPU_ANALOG_LOCK_STATUS_CONTROL, 1, &value);
                     if (!value)
                     {
                         status|=uvc_control_get(dev, VGET_CUR, UVC_PUNIT_SELECTOR,
                             VPU_ANALOG_VIDEO_STANDARD_CONTROL, 1, &value);
                         switch(value)
                         {
                             case VPU_STD_NTSC_525_60:
                                  *id=V4L2_STD_NTSC;
                                  break;
                             case VPU_STD_PAL_625_50:
                                  *id=V4L2_STD_PAL;
                                  break;
                             case VPU_STD_SECAM_625_50:
                                  *id=V4L2_STD_SECAM;
                                  break;
                             case VPU_STD_NTSC_625_50:
                                  *id=V4L2_STD_NTSC;
                                  break;
                             case VPU_STD_PAL_525_60:
                                  *id=V4L2_STD_PAL_60;
                                  break;
                         }
                     }
                     else
                     {
                         *id=V4L2_STD_UNKNOWN;
                     }
                 }
                 else
                 {
                     if ((dev->vc_processing_unit.bmControls[2] &
                         VPU_B2_ANALOG_VIDEO_STANDARD)==VPU_B2_ANALOG_VIDEO_STANDARD)
                     {
                         unsigned char value;

                         status|=uvc_control_get(dev, VGET_CUR, UVC_PUNIT_SELECTOR,
                             VPU_ANALOG_VIDEO_STANDARD_CONTROL, 1, &value);
                         switch(value)
                         {
                             case VPU_STD_NTSC_525_60:
                                  *id=V4L2_STD_NTSC;
                                  break;
                             case VPU_STD_PAL_625_50:
                                  *id=V4L2_STD_PAL;
                                  break;
                             case VPU_STD_SECAM_625_50:
                                  *id=V4L2_STD_SECAM;
                                  break;
                             case VPU_STD_NTSC_625_50:
                                  *id=V4L2_STD_NTSC;
                                  break;
                             case VPU_STD_PAL_525_60:
                                  *id=V4L2_STD_PAL_60;
                                  break;
                         }
                     }
                     else
                     {
                         if (*id==V4L2_STD_UNKNOWN)
                         {
                             if (uvc_verbose>2)
                             {
                                 slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        ENODATA: video standards are not supported");
                             }
                             ret=ENODATA;
                             break;
                         }
                     }
                 }

                 if (status)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EIO: USB I/O error");
                     }
                     ret=EIO;
                     break;
                 }

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        index: %08llX", *id);
                 }
             }
             break;
        case VIDIOC_QUERYSTD:
             {
                 v4l2_std_id* id;

                 status=0;
                 id=(v4l2_std_id*)dptr;
                 *id=V4L2_STD_UNKNOWN;

                 dctldatasize=sizeof(*id);

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    VIDIOC_QUERYSTD:");
                 }

                 if ((dev->vc_processing_unit.bmControls[2] &
                     (VPU_B2_ANALOG_VIDEO_STANDARD | VPU_B2_ANALOG_VIDEO_LOCK_STATUS))==
                     (VPU_B2_ANALOG_VIDEO_STANDARD | VPU_B2_ANALOG_VIDEO_LOCK_STATUS))
                 {
                     unsigned char value;

                     status|=uvc_control_get(dev, VGET_CUR, UVC_PUNIT_SELECTOR,
                         VPU_ANALOG_LOCK_STATUS_CONTROL, 1, &value);
                     if (!value)
                     {
                         status|=uvc_control_get(dev, VGET_CUR, UVC_PUNIT_SELECTOR,
                             VPU_ANALOG_VIDEO_STANDARD_CONTROL, 1, &value);
                         switch(value)
                         {
                             case VPU_STD_NTSC_525_60:
                                  *id=V4L2_STD_NTSC;
                                  break;
                             case VPU_STD_PAL_625_50:
                                  *id=V4L2_STD_PAL;
                                  break;
                             case VPU_STD_SECAM_625_50:
                                  *id=V4L2_STD_SECAM;
                                  break;
                             case VPU_STD_NTSC_625_50:
                                  *id=V4L2_STD_NTSC;
                                  break;
                             case VPU_STD_PAL_525_60:
                                  *id=V4L2_STD_PAL_60;
                                  break;
                         }
                     }
                     else
                     {
                         *id=V4L2_STD_UNKNOWN;
                     }
                 }
                 else
                 {
                     if ((dev->vc_processing_unit.bmControls[2] &
                         VPU_B2_ANALOG_VIDEO_STANDARD)==VPU_B2_ANALOG_VIDEO_STANDARD)
                     {
                         unsigned char value;

                         status|=uvc_control_get(dev, VGET_CUR, UVC_PUNIT_SELECTOR,
                             VPU_ANALOG_VIDEO_STANDARD_CONTROL, 1, &value);
                         switch(value)
                         {
                             case VPU_STD_NTSC_525_60:
                                  *id=V4L2_STD_NTSC;
                                  break;
                             case VPU_STD_PAL_625_50:
                                  *id=V4L2_STD_PAL;
                                  break;
                             case VPU_STD_SECAM_625_50:
                                  *id=V4L2_STD_SECAM;
                                  break;
                             case VPU_STD_NTSC_625_50:
                                  *id=V4L2_STD_NTSC;
                                  break;
                             case VPU_STD_PAL_525_60:
                                  *id=V4L2_STD_PAL_60;
                                  break;
                         }
                     }
                     else
                     {
                         if (*id==V4L2_STD_UNKNOWN)
                         {
                             if (uvc_verbose>2)
                             {
                                 slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        ENODATA: video standards are not supported");
                             }
                             ret=ENODATA;
                             break;
                         }
                     }
                 }

                 if (status)
                 {
                     if (uvc_verbose>2)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EIO: USB I/O error");
                     }
                     ret=EIO;
                     break;
                 }

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        index: %08llX", *id);
                 }
             }
             break;
        case DCMD_MISC_GETPTREMBED:
             {
                 struct __ioctl_getptrembed* getptrembed;
                 getptrembed=(struct __ioctl_getptrembed*)dptr;
                 iov_t* iov=(iov_t*)(dptr+sizeof(struct __ioctl_getptrembed));
                 struct v4l2_ext_controls* ctrl=(struct v4l2_ext_controls*)(dptr+sizeof(struct __ioctl_getptrembed)+6*sizeof(iov_t));

                 /* All this stuff looks like a great hack, but we do not have       */
                 /* a separated kernel and process space, so we have to pass request */
                 /* to underlying level to ask for transfer of some additional data. */
                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    DCMD_MISC_GETPTREMBED:");
                 }

                 switch(getptrembed->dcmd)
                 {
                     case VIDIOC_G_EXT_CTRLS:
                     case VIDIOC_S_EXT_CTRLS:
                     case VIDIOC_TRY_EXT_CTRLS:
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        embedding pointer: %08X", (uint32_t)ctrl->controls);
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        pointer data size: %08X", ctrl->count*sizeof(struct v4l2_ext_control));
                          }
                          /* setup controls pointer */
                          SETIOV(iov+0, ctrl->controls, ctrl->count*sizeof(struct v4l2_ext_control));
                          SETIOV(iov+1, 0x00000000, 0);
                          SETIOV(iov+2, 0x00000000, 0);
                          SETIOV(iov+3, 0x00000000, 0);
                          SETIOV(iov+4, 0x00000000, 0);
                          SETIOV(iov+5, 0x00000000, 0);
                          /* return an original structure as is */
                          _RESMGR_STATUS(ctp, EOK);
                          msg->o.ret_val=0;
                          status=resmgr_msgwrite(ctp, &msg->o, sizeof(struct __ioctl_getptrembed)+
                              6*sizeof(iov_t)+sizeof(struct v4l2_ext_controls), 0);
                          if (uvc_verbose>2)
                          {
                              slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EOK: supported request");
                          }
                          return EOK;
                 }
                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        _RESMGR_DEFAULT: unsupported request");
                 }
                 return _RESMGR_DEFAULT;
             }
             break;
        case VIDIOC_ENUMOUTPUT:
        case VIDIOC_S_OUTPUT:
        case VIDIOC_G_OUTPUT:
             {
                 ret=ENOTTY;
                 break;
             }
             break;
        case VIDIOC_G_SLICED_VBI_CAP:
        case VIDIOC_ENUMAUDOUT:
        case VIDIOC_S_AUDIO:
        case VIDIOC_G_AUDIO:
        case VIDIOC_S_AUDOUT:
        case VIDIOC_G_AUDOUT:
        case VIDIOC_S_TUNER:
        case VIDIOC_G_TUNER:
        case VIDIOC_G_MODULATOR:
        case VIDIOC_S_MODULATOR:
        case VIDIOC_G_FREQUENCY:
        case VIDIOC_S_FREQUENCY:
             {
                 ret=EINVAL;
                 break;
             }
             break;
        default:
             ret=ENOTTY;
             break;
    }

    /* Setup error code and reply size */
    msg->o.nbytes=dctldatasize+dctlextdatasize;
    msg->o.ret_val=0;
    if (ret!=EOK)
    {
        msg->o.ret_val=-1;
    }

    _RESMGR_STATUS(ctp, ret);
    {
        iov_t iovs[3];
        int parts=2;

        SETIOV(iovs+0, (char*)&msg->o, sizeof(msg->o));
        SETIOV(iovs+1, (char*)&msg->o+sizeof(msg->o), dctldatasize);
        if (dctlextdatasize!=0)
        {
            parts++;
            SETIOV(iovs+2, (char*)&msg->o+sizeof(msg->o)+dctldatasize, dctlextdatasize);
        }

        status=resmgr_msgwritev(ctp, &iovs[0], parts, 0);
    }

    return _RESMGR_ERRNO(ret);
}
