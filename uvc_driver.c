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
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <strings.h>
#include <sys/slog.h>
#include <sys/usbdi.h>
#include <sys/procmgr.h>
#include <sys/slogcodes.h>
#include <linux/videodev2.h>

#include "jversion.h"

#include "uvc.h"
#include "usbvc.h"
#include "usbids.h"
#include "uvc_rm.h"
#include "uvc_sysfs.h"
#include "uvc_media.h"
#include "uvc_driver.h"
#include "uvc_control.h"
#include "uvc_streaming.h"
#include "uvc_interrupt.h"

/* Global driver state */
int uvc_exit=0;
int uvc_verbose=0;
int uvc_emulation=1;
int uvc_audio=1;

int coid;
int chid;
struct usbd_connection* uvc_connection;
thread_pool_t*     thread_pool;
dispatch_t*        dispatch;
thread_pool_attr_t pool_attr;

uvc_device_mapping_t devmap[MAX_UVC_DEVICES];

void uvc_insertion(struct usbd_connection* connection, usbd_device_instance_t* instance)
{
    struct usbd_device*              uvc_device;
    usbd_interface_descriptor_t*     uvc_interface_descriptor;
    usbd_configuration_descriptor_t* uvc_configuration_descriptor;
    usbd_device_descriptor_t*        uvc_device_descriptor;
    struct usbd_desc_node*           uvc_node;
    struct usbd_desc_node*           uvc_node2;
    usbd_descriptors_t*              uvc_descriptor;
    uvc_device_t*                    uvcd=NULL;
    int devmap_id;
    char* tempstr;
    int error;
    int it;

    if (uvc_verbose)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "[devu-uvc] Device insertion");
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "[devu-uvc]    ID %04X:%04X", instance->ident.vendor, instance->ident.device);
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "[devu-uvc]    Class %02X, %02X, %02X", instance->ident.dclass, instance->ident.subclass, instance->ident.protocol);
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "[devu-uvc]    Config %02X, iface %02X, alternate %02X, path %02X, devno %02X, generation %04X", instance->config, instance->iface, instance->alternate, instance->path, instance->devno, instance->generation);
    }

    /* Skip video interface collection, QNX will call uvc_insertion() function for */
    /* the each subclass: control and streaming independently.                     */
    if ((instance->ident.subclass==VSC_VIDEO_INTERFACE_COLLECTION) && (instance->ident.protocol==VPC_PROTOCOL_UNDEFINED))
    {
        if (uvc_verbose)
        {
            slogf(_SLOGC_USB_GEN, _SLOG_INFO, "[devu-uvc] Skipping video interface collection");
        }
        return;
    }

    /* Check if it is a video control device */
    if ((instance->ident.subclass==VSC_VIDEOCONTROL) && (instance->ident.protocol==VPC_PROTOCOL_UNDEFINED))
    {
        error=usbd_attach(connection, instance, sizeof(uvc_device_t), &uvc_device);
        if (error!=EOK)
        {
            slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't attach to USB device");
            return;
        }

        /* Obtain internal structure */
        uvcd=(uvc_device_t*)usbd_device_extra(uvc_device);

        /* Fill device IDs and device name */
        uvc_device_descriptor=usbd_device_descriptor(uvc_device, &uvc_node);
        if (uvc_device_descriptor==NULL)
        {
            slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't find device descriptor of config:%02X, iface:%02X, alternate:%02X", instance->config, instance->iface, instance->alternate);
            return;
        }
        uvcd->vendor_id=uvc_device_descriptor->idVendor;
        uvcd->device_id=uvc_device_descriptor->idProduct;
        tempstr=usbd_string(uvc_device, uvc_device_descriptor->iManufacturer, 0);
        if ((tempstr!=NULL) && (strcmp(tempstr, "")!=0) && (strcmp(tempstr, " ")!=0))
        {
            strncpy(uvcd->vendor_id_str, usbd_string(uvc_device, uvc_device_descriptor->iManufacturer, 0), 255);
        }
        else
        {
            char* vendor_string;

            vendor_string=uvc_get_vendor(uvcd->vendor_id);
            if (vendor_string!=NULL)
            {
                strncpy(uvcd->vendor_id_str, vendor_string, 255);
            }
            else
            {
                strcpy(uvcd->vendor_id_str, "<null>");
            }
        }

        {
            char* device_string;

            device_string=uvc_get_device(uvcd->vendor_id, uvcd->device_id);
            if (device_string!=NULL)
            {
                strncpy(uvcd->device_id_str, device_string, 255);
            }
            else
            {
                tempstr=usbd_string(uvc_device, uvc_device_descriptor->iProduct, 0);
                if ((tempstr!=NULL) && (strcmp(tempstr, "")!=0) && (strcmp(tempstr, " ")!=0))
                {
                    strncpy(uvcd->device_id_str, usbd_string(uvc_device, uvc_device_descriptor->iProduct, 0), 255);
                }
                else
                {
                    strcpy(uvcd->device_id_str, "<null>");
                }
            }
        }

        /* Get device serial for media controller device */
        tempstr=usbd_string(uvc_device, uvc_device_descriptor->iSerialNumber, 0);
        if ((tempstr!=NULL) && (strcmp(tempstr, "")!=0) && (strcmp(tempstr, " ")!=0))
        {
            strncpy(uvcd->serial_str, usbd_string(uvc_device, uvc_device_descriptor->iSerialNumber, 0), 255);
        }
        uvcd->bcddevice=uvc_device_descriptor->bcdDevice;

        /* Find video control interface descriptor */
        uvc_interface_descriptor=usbd_interface_descriptor(uvc_device, instance->config, instance->iface, instance->alternate, &uvc_node);
        if (uvc_interface_descriptor==NULL)
        {
            slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't find interface descriptor of config:%02X, iface:%02X, alternate:%02X", instance->config, instance->iface, instance->alternate);
            return;
        }

        /* Skip video control interface descriptor content */
        uvc_descriptor=usbd_parse_descriptors(uvc_device, uvc_node, 0, 0, &uvc_node2);
        if (uvc_descriptor==NULL)
        {
            slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't read interface descriptor");
        }

        /* Read all vendor-specific descriptors in the interface descriptor */
        for (it=1;;it++)
        {
            uvc_descriptor=usbd_parse_descriptors(uvc_device, uvc_node, 0, it, &uvc_node2);
            /* Check if it is last descriptor */
            if (uvc_descriptor==NULL)
            {
                break;
            }
            uvc_parse_control_descriptor(uvcd, (uint8_t*)uvc_descriptor);
        }
        if ((it==1) && (uvc_descriptor==NULL))
        {
            slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't read class specific descriptors at VC interface");
        }

        /* Store USB device for future usage */
        uvcd->uvc_vc_device=uvc_device;

        /* Find a free resource manager entry */
        for (it=0; it<MAX_UVC_DEVICES; it++)
        {
            if (devmap[it].initialized==0)
            {
                break;
            }
        }

        if (it==MAX_UVC_DEVICES)
        {
            usbd_detach(uvc_device);
            slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Too many devices, can't create new");
            return;
        }
        else
        {
            /* Map device to resource manager */
            devmap[it].initialized=1;
            devmap[it].uvcd=uvcd;
            devmap[it].usb_devno=instance->devno;
            devmap[it].usb_path=instance->path;
            devmap[it].usb_generation=instance->generation;
            uvcd->map=&devmap[it];
        }

        uvc_interface_descriptor=usbd_interface_descriptor(uvc_device, instance->config, instance->iface, instance->alternate, &uvc_node);
        for (it=0;; it++)
        {
            uvc_descriptor=usbd_parse_descriptors(uvc_device, uvc_node, USB_DESC_ENDPOINT, it, &uvc_node2);
            if (uvc_descriptor==NULL)
            {
                break;
            }
            switch (uvc_descriptor->endpoint.bmAttributes)
            {
                int status;

                case USB_ATTRIB_CONTROL:
                     status=usbd_open_pipe(uvc_device, uvc_descriptor, &uvcd->vc_control_pipe);
                     if (status!=EOK)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't open VC control pipe");
                     }
                     usbd_reset_pipe(uvcd->vc_control_pipe);
                     break;
                case USB_ATTRIB_ISOCHRONOUS:
                     slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Isochronous pipe in VC interface, please report");
                     break;
                case USB_ATTRIB_BULK:
                     slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Bulk pipe in VC interface, please report");
                     break;
                case USB_ATTRIB_INTERRUPT:
                     status=usbd_open_pipe(uvc_device, uvc_descriptor, &uvcd->vc_interrupt_pipe);
                     if (status!=EOK)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't open VC control pipe");
                     }
                     usbd_reset_pipe(uvcd->vc_interrupt_pipe);
                     break;
            }
        }

        /* Initialize USB interrupt pipe */
        uvc_setup_interrupt(uvcd);

        /* Read USB bus topology to discover port speed */
        {
            usbd_bus_topology_t tp;
            int status;

            status=usbd_topology_ext(uvc_connection, instance->path, &tp);
            if (status!=EOK)
            {
                /* Assume full speed by default */
                uvcd->port_speed=0;
            }
            else
            {
                uvcd->port_speed=tp.ports[instance->devno].upstream_port_speed;
            }
        }
        if (uvc_verbose)
        {
            slogf(_SLOGC_USB_GEN, _SLOG_INFO, "[devu-uvc] Device port speed is %d", uvcd->port_speed);
        }

        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "[devu-uvc] %s/%s", uvcd->vendor_id_str, uvcd->device_id_str);
    }

    /* Check if it is a video control device */
    if ((instance->ident.subclass==VSC_VIDEOSTREAMING) && (instance->ident.protocol==VPC_PROTOCOL_UNDEFINED))
    {
        /* Search for already created device */
        for (it=0; it<MAX_UVC_DEVICES; it++)
        {
            if ((devmap[it].usb_devno==instance->devno) &&
                (devmap[it].usb_path==instance->path) &&
                (devmap[it].usb_generation==instance->generation) &&
                (devmap[it].initialized))
            {
                break;
            }
        }

        if (it==MAX_UVC_DEVICES)
        {
            slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't find corresponding VideoControl device");
            return;
        }

        devmap_id=it;
        uvcd=devmap[devmap_id].uvcd;

        if (uvcd->total_vs_devices>=UVC_MAX_VS_COUNT)
        {
            slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Maximum video streaming interfaces is reached");
            return;
        }

        error=usbd_attach(connection, instance, 0, &uvcd->uvc_vs_device[uvcd->total_vs_devices]);
        if (error!=EOK)
        {
            slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't attach to USB device");
            return;
        }

        /* Select zero alternate configuration to read VS descriptors */
        usbd_select_interface(uvcd->uvc_vs_device[uvcd->total_vs_devices], instance->iface, 0);

        /* Find video streaming interface descriptor */
        uvc_interface_descriptor=usbd_interface_descriptor(uvcd->uvc_vs_device[uvcd->total_vs_devices], instance->config, instance->iface, 0, &uvc_node);
        if (uvc_interface_descriptor==NULL)
        {
            slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't find interface descriptor of config:%02X, iface:%02X, alternate:%02X", instance->config, instance->iface, 0);
            return;
        }

        /* Skip video streaming interface descriptor content */
        uvc_descriptor=usbd_parse_descriptors(uvcd->uvc_vs_device[uvcd->total_vs_devices], uvc_node, 0, 0, &uvc_node2);
        if (uvc_descriptor==NULL)
        {
            slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't read interface descriptor");
            return;
        }

        /* Read all vendor-specific descriptors in the interface descriptor */
        for (it=1;;it++)
        {
            uvc_descriptor=usbd_parse_descriptors(uvcd->uvc_vs_device[uvcd->total_vs_devices], uvc_node, 0, it, &uvc_node2);
            /* Check if it is last descriptor */
            if (uvc_descriptor==NULL)
            {
                break;
            }
            uvc_parse_streaming_descriptor(uvcd, (uint8_t*)uvc_descriptor);
        }

        if ((it==1) && (uvc_descriptor==NULL))
        {
            /* Find video streaming interface descriptor */
            uvc_configuration_descriptor=usbd_configuration_descriptor(uvcd->uvc_vs_device[uvcd->total_vs_devices], instance->config, &uvc_node);
            if (uvc_configuration_descriptor==NULL)
            {
                slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't find configuration descriptor of config:%02X", instance->config);
                return;
            }

            /* Read all vendor-specific descriptors in the configuration descriptor */
            for (it=1;;it++)
            {
                uvc_descriptor=usbd_parse_descriptors(uvcd->uvc_vs_device[uvcd->total_vs_devices], uvc_node, 0, it, &uvc_node2);
                /* Check if it is last descriptor */
                if (uvc_descriptor==NULL)
                {
                    break;
                }
                uvc_parse_streaming_descriptor(uvcd, (uint8_t*)uvc_descriptor);
            }

            if ((it==1) && (uvc_descriptor==NULL))
            {
                slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't read class specific descriptors at VS interface");
                return;
            }
        }

        if (uvc_register_name(uvcd, devmap_id)<0)
        {
            usbd_detach(uvc_device);
            slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't register device name");
            return;
        }

        if (uvc_register_sysfs(uvcd, devmap_id)<0)
        {
            usbd_detach(uvc_device);
            slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't register sysfs files");
            return;
        }

        if (uvc_register_media(uvcd, devmap_id)<0)
        {
            usbd_detach(uvc_device);
            slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't register media device name");
            return;
        }

        /* Store interface number for future references */
        uvcd->vs_usb_iface[uvcd->total_vs_devices]=instance->iface;
        uvcd->vs_usb_config[uvcd->total_vs_devices]=instance->config;

        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "[devu-uvc]     Device /dev/video%d, USB ID %04X:%04X",
            devmap[devmap_id].devid[uvcd->total_vs_devices], uvcd->vendor_id, uvcd->device_id);

        {
            char cap[256];

            strcpy(cap, "[devu-uvc]     Capabilities: ");
            for (it=0; it<uvcd->vs_formats[uvcd->total_vs_devices]; it++)
            {
                if ((it%4==0) && ((it!=0)) &&
                    (uvcd->vs_format[uvcd->total_vs_devices][it]!=UVC_FORMAT_NONE))
                {
                    strcat(cap, ", ");
                    slogf(_SLOGC_USB_GEN, _SLOG_INFO, "%s", cap);
                    strcpy(cap, "[devu-uvc]                   ");
                }
                else
                {
                    if ((uvcd->vs_format[uvcd->total_vs_devices][it]!=UVC_FORMAT_NONE) && (it!=0))
                    {
                        strcat(cap, ", ");
                    }
                }

                switch(uvcd->vs_format[uvcd->total_vs_devices][it])
                {
                    case UVC_FORMAT_YUY2:
                         strcat(cap, "YUY2");
                         break;
                    case UVC_FORMAT_UYVY:
                         strcat(cap, "UYVY");
                         break;
                    case UVC_FORMAT_YVYU:
                         strcat(cap, "YVYU");
                         break;
                    case UVC_FORMAT_VYUY:
                         strcat(cap, "VYUY");
                         break;
                    case UVC_FORMAT_NV12:
                         strcat(cap, "NV12");
                         break;
                    case UVC_FORMAT_MJPG:
                         strcat(cap, "MJPG");
                         break;
                    case UVC_FORMAT_H264:
                    case UVC_FORMAT_H264F:
                         strcat(cap, "H264");
                         break;
                    case UVC_FORMAT_DV:
                         strcat(cap, "DV");
                         break;
                    case UVC_FORMAT_MPEG2TS:
                         strcat(cap, "MPEG2-TS");
                         break;
                    case UVC_FORMAT_VP8:
                         strcat(cap, "VP8");
                         break;
                    case UVC_FORMAT_NONE:
                         strcat(cap, "????");
                         break;
                    default:
                         sprintf(cap+strlen(cap), "Not supported format %08X", uvcd->vs_format[uvcd->total_vs_devices][it]);
                         break;
                }
            }
            slogf(_SLOGC_USB_GEN, _SLOG_INFO, "%s", cap);

            /* Initialize default states */
            uvcd->current_priority[uvcd->total_vs_devices]=V4L2_PRIORITY_DEFAULT;
            uvcd->current_priority_ocb[uvcd->total_vs_devices]=NULL;
            uvcd->current_buffer_fds[uvcd->total_vs_devices]=-1;

            /* Find and fill default frame settings */
            error=1;
            switch(uvcd->vs_format[uvcd->total_vs_devices][0])
            {
                case UVC_FORMAT_YUY2:
                case UVC_FORMAT_UYVY:
                case UVC_FORMAT_VYUY:
                case UVC_FORMAT_YVYU:
                case UVC_FORMAT_NV12:
                     for (it=0; it<uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bNumFrameDescriptors; it++)
                     {
                         if (uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][it].bFrameIndex ==
                             uvcd->vs_format_uncompressed[uvcd->total_vs_devices].bDefaultFrameIndex)
                         {
                             uvcd->current_width[uvcd->total_vs_devices]=
                                 uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][it].wWidth;
                             uvcd->current_height[uvcd->total_vs_devices]=
                                 uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][it].wHeight;
                             uvcd->current_frameinterval[uvcd->total_vs_devices]=
                                 uvcd->vs_frame_uncompressed[uvcd->total_vs_devices][it].dwDefaultFrameInterval;
                             switch (uvcd->vs_format[uvcd->total_vs_devices][0])
                             {
                                 case UVC_FORMAT_YUY2:
                                      uvcd->current_pixelformat[uvcd->total_vs_devices]=V4L2_PIX_FMT_YUYV;
                                      uvcd->current_stride[uvcd->total_vs_devices]=
                                          uvcd->current_width[uvcd->total_vs_devices]*2;
                                      break;
                                 case UVC_FORMAT_UYVY:
                                      uvcd->current_pixelformat[uvcd->total_vs_devices]=V4L2_PIX_FMT_UYVY;
                                      uvcd->current_stride[uvcd->total_vs_devices]=
                                          uvcd->current_width[uvcd->total_vs_devices]*2;
                                      break;
                                 case UVC_FORMAT_YVYU:
                                      uvcd->current_pixelformat[uvcd->total_vs_devices]=V4L2_PIX_FMT_YVYU;
                                      uvcd->current_stride[uvcd->total_vs_devices]=
                                          uvcd->current_width[uvcd->total_vs_devices]*2;
                                      break;
                                 case UVC_FORMAT_VYUY:
                                      uvcd->current_pixelformat[uvcd->total_vs_devices]=V4L2_PIX_FMT_VYUY;
                                      uvcd->current_stride[uvcd->total_vs_devices]=
                                          uvcd->current_width[uvcd->total_vs_devices]*2;
                                      break;
                                 case UVC_FORMAT_NV12:
                                      uvcd->current_pixelformat[uvcd->total_vs_devices]=V4L2_PIX_FMT_NV12;
                                      uvcd->current_stride[uvcd->total_vs_devices]=
                                          uvcd->current_width[uvcd->total_vs_devices];
                                      break;
                             }
                             error=0;
                         }
                     }
                     break;
                case UVC_FORMAT_MJPG:
                     for (it=0; it<uvcd->vs_format_mjpeg[uvcd->total_vs_devices].bNumFrameDescriptors; it++)
                     {
                         if (uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][it].bFrameIndex ==
                             uvcd->vs_format_mjpeg[uvcd->total_vs_devices].bDefaultFrameIndex)
                         {
                             uvcd->current_width[uvcd->total_vs_devices]=
                                 uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][it].wWidth;
                             uvcd->current_height[uvcd->total_vs_devices]=
                                 uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][it].wHeight;
                             uvcd->current_pixelformat[uvcd->total_vs_devices]=V4L2_PIX_FMT_MJPEG;
                                 uvcd->current_stride[uvcd->total_vs_devices]=
                                 uvcd->current_width[uvcd->total_vs_devices]*4;
                             uvcd->current_frameinterval[uvcd->total_vs_devices]=
                                 uvcd->vs_frame_mjpeg[uvcd->total_vs_devices][it].dwDefaultFrameInterval;
                             error=0;
                         }
                     }
                     break;
                case UVC_FORMAT_H264F:
                     for (it=0; it<uvcd->vs_format_h264f[uvcd->total_vs_devices].bNumFrameDescriptors; it++)
                     {
                         if (uvcd->vs_frame_h264f[uvcd->total_vs_devices][it].bFrameIndex ==
                             uvcd->vs_format_h264f[uvcd->total_vs_devices].bDefaultFrameIndex)
                         {
                             uvcd->current_width[uvcd->total_vs_devices]=
                                 uvcd->vs_frame_h264f[uvcd->total_vs_devices][it].wWidth;
                             uvcd->current_height[uvcd->total_vs_devices]=
                                 uvcd->vs_frame_h264f[uvcd->total_vs_devices][it].wHeight;
                             uvcd->current_pixelformat[uvcd->total_vs_devices]=V4L2_PIX_FMT_H264;
                                 uvcd->current_stride[uvcd->total_vs_devices]=
                                 uvcd->current_width[uvcd->total_vs_devices]*4;
                             uvcd->current_frameinterval[uvcd->total_vs_devices]=
                                 uvcd->vs_frame_h264f[uvcd->total_vs_devices][it].dwDefaultFrameInterval;
                             error=0;
                         }
                     }
                     break;
                default:
                     slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Unsupported UVC format, please report.");
                     break;
            }

            if (error)
            {
                 slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't find default frame format, please report.");
            }
        }

        uvcd->total_vs_devices++;
    }
}

void uvc_removal(struct usbd_connection* connection, usbd_device_instance_t* instance)
{
    int it, jt;
    int devmap_id;

    if (uvc_verbose)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "[devu-uvc] UVC Device removal");
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "[devu-uvc]    ID %04X:%04X", instance->ident.vendor, instance->ident.device);
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "[devu-uvc]    Class %02X, %02X, %02X", instance->ident.dclass, instance->ident.subclass, instance->ident.protocol);
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "[devu-uvc]    Config %02X, iface %02X, alternate %02X, path %02X, devno %02X, generation %04X", instance->config, instance->iface, instance->alternate, instance->path, instance->devno, instance->generation);
    }

    /* Check if it is a video control device */
    if ((instance->ident.subclass==VSC_VIDEOCONTROL) && (instance->ident.protocol==VPC_PROTOCOL_UNDEFINED))
    {
        /* Search for already created device */
        for (it=0; it<MAX_UVC_DEVICES; it++)
        {
            if ((devmap[it].usb_devno==instance->devno) &&
                (devmap[it].usb_path==instance->path) &&
                (devmap[it].usb_generation==instance->generation) &&
                (devmap[it].initialized))
            {
                break;
            }
        }

        if (it==MAX_UVC_DEVICES)
        {
            slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Can't find VideoControl device, unable to remove");
            return;
        }
        devmap_id=it;

        /* Stop handling of interrupts */
        uvc_unsetup_interrupt(devmap[devmap_id].uvcd);

        /* Destroy isochronous pipes, buffers and lists */
        for (jt=0; jt<devmap[devmap_id].uvcd->total_vs_devices; jt++)
        {
            if (devmap[devmap_id].uvcd->vs_isochronous_pipe[jt]!=NULL)
            {
                usbd_abort_pipe(devmap[devmap_id].uvcd->vs_isochronous_pipe[jt]);
                usbd_close_pipe(devmap[devmap_id].uvcd->vs_isochronous_pipe[jt]);
                devmap[devmap_id].uvcd->vs_isochronous_pipe[jt]=NULL;
            }
            for (it=0; it<UVC_MAX_ISO_BUFFERS; it++)
            {
                if (devmap[devmap_id].uvcd->iso_list[jt][it]!=NULL)
                {
                    usbd_free_isochronous_frame_list(devmap[devmap_id].uvcd->iso_list[jt][it]);
                    devmap[devmap_id].uvcd->iso_list[jt][it]=NULL;
                }
                if (devmap[devmap_id].uvcd->iso_urb[jt][it]!=NULL)
                {
                    usbd_free_urb(devmap[devmap_id].uvcd->iso_urb[jt][it]);
                    devmap[devmap_id].uvcd->iso_urb[jt][it]=NULL;
                }
                if (devmap[devmap_id].uvcd->iso_buffer[jt][it]!=NULL)
                {
                    usbd_free(devmap[devmap_id].uvcd->iso_buffer[jt][it]);
                    devmap[devmap_id].uvcd->iso_buffer[jt][it]=NULL;
                }
            }
        }

        /* Destroy /dev/mediaX, /dev/videoX devices and sysfs files */
        uvc_unregister_media(devmap[devmap_id].uvcd, devmap_id);
        uvc_unregister_sysfs(devmap[devmap_id].uvcd, devmap_id);
        uvc_unregister_name(devmap[devmap_id].uvcd, devmap_id);

        /* Close control pipes */
        if (devmap[devmap_id].uvcd->vc_control_pipe!=NULL)
        {
            usbd_close_pipe(devmap[devmap_id].uvcd->vc_control_pipe);
            devmap[devmap_id].uvcd->vc_control_pipe=NULL;
        }

        for (it=0; it<devmap[devmap_id].uvcd->total_vs_devices; it++)
        {
            if (devmap[devmap_id].uvcd->uvc_vs_device[it]!=NULL)
            {
                usbd_detach(devmap[devmap_id].uvcd->uvc_vs_device[it]);
            }
        }
        if (devmap[devmap_id].uvcd->uvc_vc_device!=NULL)
        {
            usbd_detach(devmap[devmap_id].uvcd->uvc_vc_device);
        }

        devmap[devmap_id].initialized=0;
        for (it=0; it<UVC_MAX_VS_COUNT; it++)
        {
            devmap[devmap_id].devid[it]=0;
        }
        devmap[devmap_id].uvcd=NULL;
        devmap[devmap_id].usb_devno=0x00;
        devmap[devmap_id].usb_path=0x00;
        devmap[devmap_id].usb_generation=0x0000;
    }
}

void uvc_event(struct usbd_connection* connection, usbd_device_instance_t* instance, uint16_t type)
{
    if (uvc_verbose)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "[devu-uvc] UVC Device event");
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "[devu-uvc]    ID %04X:%04X", instance->ident.vendor, instance->ident.device);
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "[devu-uvc]    Class %02X, %02X, %02X", instance->ident.dclass, instance->ident.subclass, instance->ident.protocol);
    }
}

usbd_device_ident_t uvc_device_ident=
{
    USBD_CONNECT_WILDCARD, /* Vendor ID              */
    USBD_CONNECT_WILDCARD, /* Device ID              */
    VCC_VIDEO,             /* Device Class           */
    USBD_CONNECT_WILDCARD, /* Device SubClass        */
    USBD_CONNECT_WILDCARD, /* Device Protocol        */
};

usbd_funcs_t uvc_functions=
{
    _USBDI_NFUNCS,
    uvc_insertion,
    uvc_removal,
    uvc_event
};

usbd_connect_parm_t uvc_connection_params=
{
    NULL,              /* Default connection path        */
    USB_VERSION,       /* Version of USB stack           */
    USBD_VERSION,      /* Version of USB DDK             */
    0,                 /* Flags                          */
    0,                 /* Arguments count                */
    NULL,              /* Arguments                      */
    0,                 /* Default event buffer size      */
    &uvc_device_ident, /* Device identification settings */
    &uvc_functions,    /* Device attach/detach functions */
    USBD_CONNECT_WAIT, /* Default connection timeout     */
    0,                 /* Event priority                 */
};

int main(int argc, char** argv)
{
    resmgr_attr_t attr;
    int error;
    int c;

    memset(devmap, 0x00, sizeof(devmap));

    /* Parse command line options */
    while (optind < argc)
    {
        if ((c=getopt(argc, argv, "vlea")) == -1)
        {
            optind++;
            continue;
        }
        switch (c)
        {
            case 'v':
                 uvc_verbose++;
                 break;
            case 'e':
                 uvc_emulation=0;
                 break;
            case 'a':
                 uvc_audio=0;
                 break;
            case 'l':
                 uvc_exit=1;
                 fprintf(stdout, "Static compiled in libraries:\n");
                 fprintf(stdout, "\n");
                 fprintf(stdout, "libuuid, version 1.0.2\n");
                 fprintf(stdout, "    Copyright (C) 1996, 1997 Theodore Ts'o\n");
                 fprintf(stdout, "libjpeg, release "JVERSION"\n");
                 fprintf(stdout, "    "JCOPYRIGHT"\n");
                 fprintf(stdout, "\n");
                 break;
        }
    }

    if ((chid = ChannelCreate( _NTO_CHF_DISCONNECT | _NTO_CHF_UNBLOCK)) == -1 ||
        (coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0)) == -1)
    {
        fprintf(stderr, "Unable to attach channel and connection\n" );
        return errno;
    }

    /* Daemonize driver */
    procmgr_daemon(EXIT_SUCCESS, PROCMGR_DAEMON_NOCLOSE);

    if ((dispatch=dispatch_create())==NULL)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Unable to allocate dispatch context");
        return -1;
    }

    memset(&pool_attr, 0, sizeof(pool_attr));
    if ((pool_attr.attr=malloc(sizeof(*pool_attr.attr)))==NULL)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Unable to alloc pool attr");
        return -1;
    }

    pthread_attr_init(pool_attr.attr);
    pthread_attr_setstacksize(pool_attr.attr, 8192);
    pool_attr.handle=dispatch;
    pool_attr.context_alloc=resmgr_context_alloc;
    pool_attr.block_func=resmgr_block;
    pool_attr.handler_func=(void*)dispatch_handler;
    pool_attr.context_free=resmgr_context_free;
    pool_attr.lo_water=1;
    pool_attr.hi_water=2;
    pool_attr.increment=1;
    pool_attr.maximum=2;

    if ((thread_pool=thread_pool_create(&pool_attr, 0))==NULL)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Unable to initialize thread pool");
        return -1;
    }

    memset(&attr, 0, sizeof(attr));
    attr.nparts_max=10;
    attr.msg_max_size=65536;

    if (resmgr_attach(dispatch, &attr, NULL, 0, 0, NULL, NULL, NULL)==-1)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Unable to attach name");
        return -1;
    }

    if (thread_pool_start(thread_pool))
    {
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] Unable to initialize thread_pool");
        return -1;
    }

    error=usbd_connect(&uvc_connection_params, &uvc_connection);
    if (error!=EOK)
    {
        fprintf(stderr, "Can't connect to USB stack\n");
        return -1;
    }

    /* Execute an empty loop */
    do {
        sleep(1);
    } while(!uvc_exit);

    return 0;
}
