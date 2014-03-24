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

#ifndef __UVC_RM_H__
#define __UVC_RM_H__

#include "uvc.h"
#include "uvc_devctl.h"

extern dispatch_t* dispatch;

int uvc_register_name(uvc_device_t* dev, int mapid);
int uvc_unregister_name(uvc_device_t* dev, int mapid);

uvc_ocb_t* _uvc_ocb_calloc(resmgr_context_t* ctp, uvc_device_t* dev);
void _uvc_ocb_free(uvc_ocb_t* ocb);

#endif /* __UVC_RM_H__ */
