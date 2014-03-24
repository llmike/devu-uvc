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

#ifndef __UVC_DRIVER_H__
#define __UVC_DRIVER_H__

#include <stdint.h>

#include "uvc.h"

#define DEVU_UVC_VERSION 0x00010000

/* I do not think, that somebody will use more than 16 UVC devices */
/* simultaneously, because he will require 8-16 USB root hubs to   */
/* provide proper bandwidth.                                       */
#define MAX_UVC_DEVICES 16

typedef struct _uvc_device_mapping
{
    int initialized;             /* 1 - if this device is initialized          */
    int devid[UVC_MAX_VS_COUNT]; /* device minors, i.e. /dev/video[devid]      */
    uvc_device_t* uvcd;          /* uvc_device_t pointer of associated device  */
    uint8_t  usb_devno;
    uint8_t  usb_path;
    uint16_t usb_generation;
} uvc_device_mapping_t;

#endif /* __UVC_DRIVER_H__ */
