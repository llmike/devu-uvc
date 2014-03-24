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

#ifndef __UVC_STREAMING_H__
#define __UVC_STREAMING_H__

#include <stdint.h>

void uvc_parse_streaming_descriptor(uvc_device_t* uvcd, uint8_t* data);

int uvc_probe_commit_get(uvc_device_t* dev, int subdev, int operation, int unit, int selector, int size, uint8_t* data);
int uvc_probe_commit_set(uvc_device_t* dev, int subdev, int operation, int unit, int selector, int size, uint8_t* data);

void uvc_isochronous_completion(struct usbd_urb* urb, struct usbd_pipe* pipe, void* handle);

#endif /* __UVC_STREAMING_H__ */
