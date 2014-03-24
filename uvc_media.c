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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/procmgr.h>
#include <sys/rsrcdbmgr.h>
#include <sys/slogcodes.h>
#include <sys/dcmd_misc.h>

#include <linux/videodev2.h>
#include <linux/media.h>

#include "bsdqueue.h"

#include "uvc.h"
#include "uvc_rm.h"
#include "uvc_media.h"
#include "uvc_driver.h"
#include "uvc_devctl.h"

extern uvc_device_mapping_t devmap[MAX_UVC_DEVICES];
extern int uvc_verbose;

uvc_ocb_t* _uvc_media_ocb_calloc(resmgr_context_t* ctp, uvc_device_t* dev)
{
    uvc_ocb_t* ocb;
    int it;
    int jt;

    ocb = calloc(1, sizeof(uvc_ocb_t));
    ocb->dev=NULL;
    ocb->handle=dev;
    ocb->subdev=-1;

    if (ocb != NULL)
    {
        /* Reconstruct device pointer, it could be displaced  */
        /* if we have more than one VS interface. At the same */
        /* time, iofunc_attr_t[0] is always valid as handle.  */
        for (it=0; it<MAX_UVC_DEVICES; it++)
        {
            if ((devmap[it].initialized) && (devmap[it].uvcd!=NULL))
            {
                for(jt=0; jt<devmap[it].uvcd->total_vs_devices; jt++)
                {
                    if (devmap[it].uvcd->media[jt]!=NULL)
                    {
                        if (&devmap[it].uvcd->media[jt]->hdr==(void*)dev)
                        {
                            ocb->dev=devmap[it].uvcd;
                            ocb->subdev=jt;
                            break;
                        }
                    }
                }
            }
            if (ocb->dev!=NULL)
            {
                break;
            }
        }

        IOFUNC_NOTIFY_INIT(ocb->notify);
    }

    return ocb;
}

void _uvc_media_ocb_free(uvc_ocb_t* ocb)
{
    free(ocb);
}

int uvc_open_media(resmgr_context_t* ctp, io_open_t* msg, RESMGR_HANDLE_T* handle, void* extra)
{
    int status;

    status=iofunc_open_default(ctp, msg, (iofunc_attr_t*)handle, extra);
    if (status)
    {
        return status;
    }

    return EOK;
}

int uvc_close_media(resmgr_context_t* ctp, void* reserved, uvc_ocb_t* ocb)
{
    int status;

    status=iofunc_close_ocb_default(ctp, reserved, (iofunc_ocb_t*)ocb);
    if (status)
    {
        return status;
    }

    return EOK;
}

int uvc_devctl_media(resmgr_context_t* ctp, io_devctl_t* msg, uvc_ocb_t* ocb)
{
    int status, ret=EOK;
    uvc_device_t* dev=ocb->dev;
    void* dptr=_DEVCTL_DATA(msg->i);
    int dctldatasize=0;
    int dctlextdatasize=0;
    int subdev=ocb->subdev;
    struct _uvc_media_device* media;
    int it;

    status = iofunc_devctl_default(ctp, msg, &ocb->hdr);
    if (status != _RESMGR_DEFAULT)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] media iofunc_devctl_default() failed");
        return _RESMGR_ERRNO(status);
    }

    if (subdev==-1)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] media devctl(): can't locate subdevice!");
        return _RESMGR_ERRNO(ENODEV);
    }
    media=dev->media[subdev];

    if (uvc_verbose>2)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "media devctl(): cmd %08X, subdevice %d, rdev=%08X, ocb=%08X", msg->i.dcmd, subdev, ocb->hdr.attr->hdr->rdev, (unsigned int)ocb);
    }

    switch (msg->i.dcmd)
    {
        case MEDIA_IOC_DEVICE_INFO:
             {
                 struct media_device_info* info;

                 info=(struct media_device_info*)dptr;
                 memset(info, 0x00, sizeof(*info));
                 strncpy((char*)info->driver, "devu-uvc", sizeof(info->driver));
                 strncpy((char*)info->model, dev->device_id_str, sizeof(info->model));
                 strncpy((char*)info->serial, dev->serial_str, sizeof(info->serial));
                 info->media_version=MEDIA_API_VERSION;
                 info->hw_revision=_MEDIA_KERNEL_VERSION(0, (dev->bcddevice>>8) & 0x000000FF, dev->bcddevice & 0x000000FF);
                 info->driver_version=DEVU_UVC_VERSION;

                 snprintf((char*)info->bus_info, sizeof(info->bus_info), "usb-%d:%d", dev->map->usb_path, dev->map->usb_devno);

                 dctldatasize=sizeof(*info);

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    MEDIA_IOC_DEVICE_INFO:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        driver: %s", info->driver);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        model: %s", info->model);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        serial: %s", info->serial);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        bus_info: %s", info->bus_info);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        media_version: %08X", info->media_version);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        hw_revision: %08X", info->hw_revision);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        driver_version: %08X", info->driver_version);
                 }

                     for (it=0; it<dev->media_entities; it++)
                     {
                         slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        entity id: %d", dev->entity[it].id);
                     }
             }
             break;
        case MEDIA_IOC_ENUM_ENTITIES:
             {
                 struct media_entity_desc* desc;
                 int match=0;

                 desc=(struct media_entity_desc*)dptr;
                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "    MEDIA_IOC_ENUM_ENTITIES:");
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        input id: %08X", desc->id);
                 }

                 if (desc->id & MEDIA_ENT_ID_FLAG_NEXT)
                 {
                     desc->id&=~(MEDIA_ENT_ID_FLAG_NEXT);
                     do {
                         desc->id++;
                         for (it=0; it<dev->media_entities; it++)
                         {
                             if (dev->entity[it].id==desc->id)
                             {
                                 match=1;
                                 *desc=dev->entity[it];
                                 break;
                             }
                         }
                         if (match)
                         {
                             break;
                         }
                         /* 16-bit UVC units IDs */
                         if (desc->id>=65535)
                         {
                             break;
                         }
                     } while(1);
                     if (!match)
                     {
                         if (uvc_verbose>2)
                         {
                             slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: no more entities are left");
                         }
                         ret=EINVAL;
                         break;
                     }
                 }
                 else
                 {
                     /* Search exact id number */
                     for (it=0; it<dev->media_entities; it++)
                     {
                         if (dev->entity[it].id==desc->id)
                         {
                             match=1;
                             *desc=dev->entity[it];
                             break;
                         }
                     }
                     if (!match)
                     {
                         if (uvc_verbose>2)
                         {
                             slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        EINVAL: entity can't be found");
                         }
                         ret=EINVAL;
                         break;
                     }
                 }

                 if (uvc_verbose>2)
                 {
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        id: %08X", desc->id);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        name: %s", desc->name);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        type: %08X", desc->type);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        revision: %08X", desc->revision);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        flags: %08X", desc->flags);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        group_id: %d", desc->group_id);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        pads: %d", desc->pads);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        links: %d", desc->links);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        v4l.major: %d", desc->v4l.major);
                     slogf(_SLOGC_USB_GEN, _SLOG_INFO, "        v4l.minor: %d", desc->v4l.minor);
                 }

                 dctldatasize=sizeof(*desc);
             }
             break;
        case MEDIA_IOC_ENUM_LINKS:
             {
                 ret=EINVAL;
             }
             break;
        case MEDIA_IOC_SETUP_LINK:
             {
                 ret=EINVAL;
             }
             break;
        case DCMD_MISC_GETPTREMBED:
             {
                 ret=EINVAL;
             }
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

int uvc_register_media(uvc_device_t* dev, int mapid)
{
    char name[PATH_MAX];
    struct _uvc_media_device* media;

    if (dev->media[dev->total_vs_devices])
    {
        free(dev->media[dev->total_vs_devices]);
        dev->media[dev->total_vs_devices]=NULL;
    }
    dev->media[dev->total_vs_devices]=calloc(1, sizeof(struct _uvc_media_device));
    if (dev->media[dev->total_vs_devices]==NULL)
    {
        return -1;
    }
    media=dev->media[dev->total_vs_devices];

    memset(&media->ocb_funcs, 0x00, sizeof(media->ocb_funcs));
    media->ocb_funcs.nfuncs=_IOFUNC_NFUNCS;
    media->ocb_funcs.ocb_calloc=_uvc_media_ocb_calloc;
    media->ocb_funcs.ocb_free=_uvc_media_ocb_free;
    memset(&media->io_mount, 0x00, sizeof(media->io_mount));
    media->io_mount.funcs=&media->ocb_funcs;

    memset(&media->rattr, 0, sizeof(media->rattr));
    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &media->connect_funcs,
                     _RESMGR_IO_NFUNCS, &media->io_funcs);
    iofunc_attr_init(&media->hdr, S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP |
        S_IWGRP | S_IROTH | S_IWOTH, NULL, NULL);

    media->hdr.rdev=rsrcdbmgr_devno_attach("media", -1, 0);
    if (media->hdr.rdev==-1)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] rsrcdbmgr_devno_attach() failed: %s", strerror(errno));
        return -1;
    }

    media->connect_funcs.open=uvc_open_media;
    media->io_funcs.devctl=uvc_devctl_media;
    media->io_funcs.close_ocb=uvc_close_media;

    media->hdr.mount=&media->io_mount;

    snprintf(name, PATH_MAX, "/dev/media%d", minor(media->hdr.rdev));

    media->resmgr_id=resmgr_attach(dispatch,
        &media->rattr, name, _FTYPE_ANY, 0, &media->connect_funcs,
        &media->io_funcs, (RESMGR_HANDLE_T*)&media->hdr);
    if (media->resmgr_id==-1)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] resmgr_attach() failed: %s", strerror(errno));
        return -1;
    }

    resmgr_devino(media->resmgr_id, &media->io_mount.dev, &media->hdr.inode);

    return 0;
}

int uvc_unregister_media(uvc_device_t* dev, int mapid)
{
    struct _uvc_media_device* media;
    int status;
    int it;

    for (it=0; it<dev->total_vs_devices; it++)
    {
        media=dev->media[it];
        if (media==NULL)
        {
            continue;
        }
        if ((media->resmgr_id!=-1) && (media->resmgr_id!=0))
        {
            status=resmgr_detach(dispatch, media->resmgr_id,
                _RESMGR_DETACH_ALL | _RESMGR_DETACH_CLOSE);
            if (status)
            {
                slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] resmgr_detach() failed: %s", strerror(errno));
                /* fall through */
            }
        }

        free(dev->media[it]);
        dev->media[it]=NULL;
    }

    return 0;
}
