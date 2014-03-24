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

#ifndef __UVC_CONTROL_H__
#define __UVC_CONTROL_H__

#include <stdint.h>

void uvc_parse_control_descriptor(uvc_device_t* uvcd, uint8_t* data);

#define UVC_UNKNOWN_SELECTOR     0x00000000
#define UVC_CAMERA_SELECTOR      0x00000001
#define UVC_PUNIT_SELECTOR       0x00000002
#define UVC_ISEL_SELECTOR        0x00000003

int uvc_control_get(uvc_device_t* dev, int operation, int unit, int selector, int size, unsigned char* data);
int uvc_control_set(uvc_device_t* dev, int operation, int unit, int selector, int size, unsigned char* data);

uint64_t uvc_get_uinteger(int size, unsigned char* data);
int64_t  uvc_get_sinteger(int size, unsigned char* data);
void     uvc_set_uinteger(int size, unsigned char* data, uint64_t value);
void     uvc_set_sinteger(int size, unsigned char* data, int64_t value);

#endif /* __UVC_CONTROL_H__ */
