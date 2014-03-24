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

#ifndef __UVC_SYSFS_H__
#define __UVC_SYSFS_H__

#include "uvc.h"
#include "uvc_rm.h"
#include "uvc_devctl.h"

extern dispatch_t* dispatch;

#define UVC_SYSFS_FILES 9

#define UVC_SYSFS_FILE_DEV               0x00000000
#define UVC_SYSFS_FILE_INDEX             0x00000001
#define UVC_SYSFS_FILE_NAME              0x00000002
#define UVC_SYSFS_FILE_UEVENT            0x00000003
#define UVC_SYSFS_FILE_IDVENDOR          0x00000004
#define UVC_SYSFS_FILE_IDPRODUCT         0x00000005
#define UVC_SYSFS_FILE_SPEED             0x00000006
#define UVC_SYSFS_FILE_DEVICE_MODALIAS   0x00000007
#define UVC_SYSFS_FILE_DEV_CHAR          0x00000008

#define UVC_SYSFS_MAX_FILESIZE           128

typedef struct _uvc_sysfs_device
{
    iofunc_attr_t          hdr[UVC_SYSFS_FILES];
    int                    resmgr_id[UVC_SYSFS_FILES];
    resmgr_io_funcs_t      io_funcs[UVC_SYSFS_FILES];
    resmgr_connect_funcs_t connect_funcs[UVC_SYSFS_FILES];
    iofunc_funcs_t         ocb_funcs[UVC_SYSFS_FILES];
    iofunc_mount_t         io_mount[UVC_SYSFS_FILES];
    resmgr_attr_t          rattr[UVC_SYSFS_FILES];
    char                   filedata[UVC_SYSFS_FILES][UVC_SYSFS_MAX_FILESIZE];
} uvc_sysfs_device_t;

int uvc_register_sysfs(uvc_device_t* dev, int mapid);
int uvc_unregister_sysfs(uvc_device_t* dev, int mapid);

#endif /* __UVC_SYSFS_H__ */
