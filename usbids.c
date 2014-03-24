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

#include <stdint.h>
#include <stdlib.h>

typedef struct uvc_vendor
{
    uint16_t vendor_id;
    char* vendor_name;
} uvc_vendor_t;

typedef struct uvc_device
{
    uint16_t vendor_id;
    uint16_t device_id;
    char* device_name;
} uvc_device_t;

uvc_vendor_t uvc_vendor_list[]=
{
    {0x03F0, "HP"},
    {0x0458, "Kye Systems Corp."},
    {0x045E, "Microsoft Corp."},
    {0x046D, "Logitech, Inc."},
    {0x04F2, "Chicony Electronics Co., Ltd"},
    {0x04FC, "Sunplus Technology Co., Ltd"},
    {0x0C45, "Microdia"},
};

uvc_device_t uvc_device_list[]=
{
    {0x03F0, 0xCF07, "Webcam HD 5210"},
    {0x0458, 0x505E, "Genius iSlim 330"},
    {0x0458, 0x705D, "Genius iSlim 2000AF"},
    {0x0458, 0x7061, "Genius iLook 1321 V2"},
    {0x0458, 0x7066, "Acer Crystal Eye Webcam"},
    {0x0458, 0x7067, "Genius iSlim 1300AF V2"},
    {0x0458, 0x7068, "Genius eFace 1325R"},
    {0x0458, 0x7076, "Genius FaceCam 312"},
    {0x0458, 0x707E, "Genius FaceCam 1000"},
    {0x0458, 0x7089, "Genius FaceCam 320"},
    {0x0458, 0x708C, "Genius WideCam F100"},
    {0x045E, 0x00F8, "LifeCam NX-6000"},
    {0x045E, 0x0721, "LifeCam NX-3000"},
    {0x045E, 0x0728, "LifeCam VX-5000"},
    {0x045E, 0x0766, "LifeCam VX-800"},
    {0x045E, 0x076D, "LifeCam HD-5000"},
    {0x045E, 0x07F6, "LifeCam HD-6000 for Notebooks"},
    {0x046D, 0x0802, "Webcam C200"},
    {0x046D, 0x0804, "Webcam C250"},
    {0x046D, 0x0805, "Webcam C300"},
    {0x046D, 0x0807, "Webcam B500/C500"},
    {0x046D, 0x0808, "Webcam C600"},
    {0x046D, 0x0809, "Webcam Pro 9000"},
    {0x046D, 0x080A, "Webcam C905"},
    {0x046D, 0x080F, "Webcam C120"},
    {0x046D, 0x0819, "Webcam C210"},
    {0x046D, 0x081B, "HD Webcam C310"},
    {0x046D, 0x081D, "HD Webcam C510"},
    {0x046D, 0x0821, "HD Webcam C910"},
    {0x046D, 0x0825, "Webcam C270"},
    {0x046D, 0x0826, "HD Webcam C525"},
    {0x046D, 0x0828, "HD Webcam B990"},
    {0x046D, 0x082B, "Webcam C170"},
    {0x046D, 0x082C, "HD Webcam C615"},
    {0x046D, 0x082D, "HD Pro Webcam C920"},
    {0x04F2, 0xB15C, "Sony Visual Communication Camera"},
    {0x04F2, 0xB213, "Fujitsu Integrated Camera"},
    {0x04FC, 0x6333, "Siri A9 UVC Interface"},
    {0x0C45, 0x6340, "MI2010 UVC Camera"},
};

char* uvc_get_vendor(uint16_t vendor_id)
{
    int it;

    for (it=0;;it++)
    {
        if (uvc_vendor_list[it].vendor_id==0x0000)
        {
            break;
        }
        if (uvc_vendor_list[it].vendor_id==vendor_id)
        {
            return uvc_vendor_list[it].vendor_name;
        }
    }

    return NULL;
}

char* uvc_get_device(uint16_t vendor_id, uint16_t device_id)
{
    int it;

    for (it=0;;it++)
    {
        if (uvc_device_list[it].vendor_id==0x0000)
        {
            break;
        }
        if ((uvc_device_list[it].vendor_id==vendor_id) &&
            (uvc_device_list[it].device_id==device_id))
        {
            return uvc_device_list[it].device_name;
        }
    }

    return NULL;
}
