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

#include <linux/videodev2.h>

#include "bsdqueue.h"

#include "uvc.h"
#include "uvc_rm.h"
#include "uvc_driver.h"
#include "uvc_devctl.h"

extern uvc_device_mapping_t devmap[MAX_UVC_DEVICES];
extern int uvc_verbose;

uvc_ocb_t* _uvc_ocb_calloc(resmgr_context_t* ctp, uvc_device_t* dev)
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
                    if (devmap[it].devid[jt]==minor(dev->hdr[0].rdev))
                    {
                        ocb->dev=devmap[it].uvcd;
                        break;
                    }
                }
            }
            if (ocb->dev!=NULL)
            {
                break;
            }
        }

        if (ocb->dev==NULL)
        {
            ocb->dev=dev;
        }
        IOFUNC_NOTIFY_INIT(ocb->notify);
    }

    return ocb;
}

void _uvc_ocb_free(uvc_ocb_t* ocb)
{
    free(ocb);
}

int uvc_open(resmgr_context_t* ctp, io_open_t* msg, RESMGR_HANDLE_T* handle, void* extra)
{
    int status;

    if (uvc_verbose>2)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "open():");
    }

    status=iofunc_open_default(ctp, msg, (iofunc_attr_t*)handle, extra);
    if (status)
    {
        return status;
    }

    return EOK;
}

int uvc_close(resmgr_context_t* ctp, void* reserved, uvc_ocb_t* ocb)
{
    int status;
    uvc_device_t* dev=ocb->dev;
    int subdev=-1;
    int it;

    if (uvc_verbose>2)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "close(): ocb=%08X", (uint32_t)ocb);
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
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] close(): can't locate subdevice!");
        /* fall through to invoke default close */
    }
    else
    {
        /* remove high priority status if this file descriptor is an owner */
        if (dev->current_priority_ocb[subdev]==ocb)
        {
            dev->current_priority[subdev]=V4L2_PRIORITY_DEFAULT;
            dev->current_priority_ocb[subdev]=NULL;
        }

        /* Remove buffers ownership */
        if (dev->current_reqbufs_ocb[subdev]==ocb)
        {
            char fdname[128];
            struct _uvc_buffer_entry* entry;
            struct _uvc_buffer_entry* tentry;

            /* Stop the current transfer */
            dev->current_transfer[subdev]=0;
            dev->buffer_mode_mmap[subdev]=0;

            dev->current_reqbufs_ocb[subdev]=NULL;

            /* Shared memory name consist of device minor, usb path and usb devno */
            sprintf(fdname, "/devu-uvc-%d-%d-%d", minor(ocb->hdr.attr->hdr->rdev), dev->map->usb_path, dev->map->usb_devno);

            /* Free any previously allocated buffers */
            dev->current_buffer_count[subdev]=0;
            if (dev->current_buffer_fds[subdev]!=-1)
            {
                close(dev->current_buffer_fds[subdev]);
            }
            dev->current_buffer_fds[subdev]=-1;
            shm_unlink(fdname);

            if (dev->input_buffer[subdev].mutex_inited)
            {
                pthread_mutex_lock(&dev->input_buffer[subdev].access);
            }
            TAILQ_FOREACH_SAFE(entry, &dev->input_buffer[subdev].head, link, tentry)
            {
                TAILQ_REMOVE(&dev->input_buffer[subdev].head, entry, link);
                if (entry!=NULL)
                {
                    free(entry);
                }
            }
            if (dev->input_buffer[subdev].mutex_inited)
            {
                dev->input_buffer[subdev].mutex_inited=0;
                pthread_mutex_unlock(&dev->input_buffer[subdev].access);
                pthread_mutex_destroy(&dev->input_buffer[subdev].access);
            }

            if (dev->output_buffer[subdev].mutex_inited)
            {
                pthread_mutex_lock(&dev->output_buffer[subdev].access);
            }
            TAILQ_FOREACH_SAFE(entry, &dev->output_buffer[subdev].head, link, tentry)
            {
                TAILQ_REMOVE(&dev->output_buffer[subdev].head, entry, link);
                if (entry!=NULL)
                {
                    free(entry);
                }
            }
            if (dev->output_buffer[subdev].mutex_inited)
            {
                dev->output_buffer[subdev].mutex_inited=0;
                pthread_mutex_unlock(&dev->output_buffer[subdev].access);
                pthread_mutex_destroy(&dev->output_buffer[subdev].access);
            }
        }

        /* Check if we have event structure for this ocb */
        for(it=0; it<UVC_MAX_OPEN_FDS; it++)
        {
            /* Destroy event queue for this ocb */
            if (dev->event[it].ocb==ocb)
            {
                struct _uvc_event_entry* entry;
                struct _uvc_event_entry* tentry;

                pthread_mutex_lock(&dev->event[it].access);
                TAILQ_FOREACH_SAFE(entry, &dev->event[it].head, link, tentry)
                {
                    TAILQ_REMOVE(&dev->event[it].head, entry, link);
                    if (entry!=NULL)
                    {
                        free(entry);
                    }
                }

                dev->event[it].ocb=NULL;
                dev->event[it].sequence=0;
                TAILQ_INIT(&dev->event[it].head);
                pthread_mutex_unlock(&dev->event[it].access);

                memset(dev->event[it].type, 0x00, sizeof(dev->event[it].type));
                memset(dev->event[it].flags, 0x00, sizeof(dev->event[it].flags));
                pthread_mutex_destroy(&dev->event[it].access);
                break;
            }
        }
    }

    /* Remove notify queues at exit */
    iofunc_notify_remove(ctp, ocb->notify);

    status=iofunc_close_ocb_default(ctp, reserved, (iofunc_ocb_t*)ocb);
    if (status)
    {
        return status;
    }

    return EOK;
}

int uvc_notify(resmgr_context_t* ctp, io_notify_t* msg, uvc_ocb_t* ocb)
{
    uvc_device_t* dev=ocb->dev;
    int trigger=0;
    int subdev=-1;
    int it;

    if (uvc_verbose>2)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "notify(): ocb=%08X", (uint32_t)ocb);
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
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] notify(): can't locate subdevice!");
        return ENODEV;
    }

    /* Check if we have event structure for this ocb */
    for(it=0; it<UVC_MAX_OPEN_FDS; it++)
    {
        /* Destroy event queue for this ocb */
        if (dev->event[it].ocb==ocb)
        {
            /* Do not lock mutex, it is just a r/o check */
            if (!TAILQ_EMPTY(&dev->event[it].head))
            {
                /* Events are out of band data for read */
                /* According to docs, this is the same as _NOTIFY_COND_OBAND */
                trigger|=_NOTIFY_COND_EXTEN | _NOTIFY_CONDE_RDBAND;
                break;
            }
        }
    }

    /* Check if driver got some new buffers */
    if (!TAILQ_EMPTY(&dev->output_buffer[subdev].head))
    {
        /* We have some buffers in the output queue */
        trigger|=_NOTIFY_COND_EXTEN | _NOTIFY_CONDE_RDNORM;
    }

    /* We always have a room for some buffers in the input queue */
    trigger|=_NOTIFY_COND_EXTEN | _NOTIFY_CONDE_WRNORM;

    return iofunc_notify(ctp, msg, ocb->notify, trigger, NULL, NULL);
}

int uvc_mmap(resmgr_context_t *ctp, io_mmap_t *msg, uvc_ocb_t* ocb)
{
    uvc_device_t* dev=ocb->dev;
    int subdev=-1;
    int result;
    unsigned int size;
    unsigned int chunksize;
    unsigned int offset=msg->i.offset;
    int it;

    if (uvc_verbose>2)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "mmap(), ocb %08X, type %08X, prot %08X, offset %08llX", (uint32_t)ocb, msg->i.type, msg->i.prot, msg->i.offset);
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
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] mmap(): can't locate subdevice!");
        return ENODEV;
    }

    if (dev->current_buffer_fds[subdev]==-1)
    {
        return ENOMEM;
    }

    size=dev->current_stride[subdev]*dev->current_height[subdev];
    chunksize=sysconf(_SC_PAGE_SIZE);
    /* Adjust buffer size to system page size */
    size=(size+chunksize-1) & ~(chunksize-1);

    result=iofunc_mmap_default(ctp, msg, (iofunc_ocb_t*)ocb);
    if (result<0)
    {
        /* Now perform a hack, catch the message, which has been created */
        /* by iofunc_mmap_default() function, and replace coid with our  */
        /* shared memory descriptor.                                     */
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "mmap(), coid=%d at exit", msg->o.coid);
        msg->o.fd=-1;
        msg->o.coid=dev->current_buffer_fds[subdev];
        msg->o.offset=offset;
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "mmap(), new coid=%d at exit", msg->o.coid);
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "mmap(), fd=%d, offset=%08llX at exit", msg->o.fd, msg->o.offset);
    }

    return result;
}

int uvc_read(resmgr_context_t *ctp, io_read_t *msg, uvc_ocb_t *ocb)
{
    uvc_device_t* dev=ocb->dev;
    int subdev=-1;
    int result;
    int it;
    int nparts=0;
    int nonblock=0;

    if (uvc_verbose>2)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "read(): ocb %08X", (uint32_t)ocb);
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
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] read(): can't locate subdevice!");
        _IO_SET_READ_NBYTES(ctp, -1);
        return ENODEV;
    }

    result=iofunc_read_verify(ctp, msg, (iofunc_ocb_t*)ocb, &nonblock);
    if (result!=EOK)
    {
        _IO_SET_READ_NBYTES(ctp, -1);
        return result;
    }
    if ((msg->i.xtype & _IO_XTYPE_MASK)!=_IO_XTYPE_NONE)
    {
        /* TODO: handle different xtypes */
        _IO_SET_READ_NBYTES(ctp, -1);
        return ENOSYS;
    }

    /* If MMAP access method is active, deny read/write functions */
    if (dev->buffer_mode_mmap[subdev])
    {
        _IO_SET_READ_NBYTES(ctp, -1);
        return EBUSY;
    }

    _IO_SET_READ_NBYTES(ctp, 0);

    return (_RESMGR_NPARTS(nparts));
}

int uvc_write(resmgr_context_t *ctp, io_write_t *msg, uvc_ocb_t *ocb)
{
    uvc_device_t* dev=ocb->dev;
    int subdev=-1;
    int result;
    int it;
    int nparts=0;
    int nonblock=0;

    if (uvc_verbose>2)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_INFO, "write(): ocb %08X", (uint32_t)ocb);
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
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] read(): can't locate subdevice!");
        _IO_SET_WRITE_NBYTES(ctp, -1);
        return ENODEV;
    }

    result=iofunc_write_verify(ctp, msg, (iofunc_ocb_t*)ocb, &nonblock);
    if (result!=EOK)
    {
        _IO_SET_WRITE_NBYTES(ctp, -1);
        return result;
    }
    if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE)
    {
        /* TODO: handle different xtypes */
        _IO_SET_WRITE_NBYTES(ctp, -1);
        return ENOSYS;
    }

    /* If MMAP access method is active, deny read/write functions */
    if (dev->buffer_mode_mmap[subdev])
    {
        _IO_SET_WRITE_NBYTES(ctp, -1);
        return EBUSY;
    }

    _IO_SET_WRITE_NBYTES(ctp, 0);

    return (_RESMGR_NPARTS(nparts));
}

int uvc_register_name(uvc_device_t* dev, int mapid)
{
    char name[PATH_MAX];

    memset(&dev->ocb_funcs[dev->total_vs_devices], 0x00, sizeof(dev->ocb_funcs[dev->total_vs_devices]));
    dev->ocb_funcs[dev->total_vs_devices].nfuncs=_IOFUNC_NFUNCS;
    dev->ocb_funcs[dev->total_vs_devices].ocb_calloc=_uvc_ocb_calloc;
    dev->ocb_funcs[dev->total_vs_devices].ocb_free=_uvc_ocb_free;
    memset(&dev->io_mount[dev->total_vs_devices], 0x00, sizeof(dev->io_mount[dev->total_vs_devices]));
    dev->io_mount[dev->total_vs_devices].funcs=&dev->ocb_funcs[dev->total_vs_devices];

    memset(&dev->rattr[dev->total_vs_devices], 0, sizeof(dev->rattr[dev->total_vs_devices]));
    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &dev->connect_funcs[dev->total_vs_devices],
                     _RESMGR_IO_NFUNCS, &dev->io_funcs[dev->total_vs_devices]);
    iofunc_attr_init(&dev->hdr[dev->total_vs_devices], S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP |
        S_IWGRP | S_IROTH | S_IWOTH, NULL, NULL);

    dev->hdr[dev->total_vs_devices].rdev=rsrcdbmgr_devno_attach("video", -1, 0);
    if (dev->hdr[dev->total_vs_devices].rdev==-1)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] rsrcdbmgr_devno_attach() failed: %s", strerror(errno));
        return -1;
    }

    dev->connect_funcs[dev->total_vs_devices].open=uvc_open;
    dev->io_funcs[dev->total_vs_devices].devctl=uvc_devctl;
    dev->io_funcs[dev->total_vs_devices].notify=uvc_notify;
    dev->io_funcs[dev->total_vs_devices].mmap=uvc_mmap;
    dev->io_funcs[dev->total_vs_devices].read=uvc_read;
    dev->io_funcs[dev->total_vs_devices].write=uvc_write;
    dev->io_funcs[dev->total_vs_devices].close_ocb=uvc_close;

    dev->hdr[dev->total_vs_devices].mount=&dev->io_mount[dev->total_vs_devices];

    /* Find free device name */
    snprintf(name, PATH_MAX, "/dev/video%d", minor(dev->hdr[dev->total_vs_devices].rdev));

    dev->resmgr_id[dev->total_vs_devices]=resmgr_attach(dispatch,
        &dev->rattr[dev->total_vs_devices], name, _FTYPE_ANY, 0,
        &dev->connect_funcs[dev->total_vs_devices],
        &dev->io_funcs[dev->total_vs_devices], (RESMGR_HANDLE_T*)&dev->hdr[dev->total_vs_devices]);
    if (dev->resmgr_id[dev->total_vs_devices]==-1)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] resmgr_attach() failed: %s", strerror(errno));
        rsrcdbmgr_devno_detach(dev->hdr[dev->total_vs_devices].rdev, 0);
        return -1;
    }

    resmgr_devino(dev->resmgr_id[dev->total_vs_devices], &dev->io_mount[dev->total_vs_devices].dev, &dev->hdr[dev->total_vs_devices].inode);

    devmap[mapid].devid[dev->total_vs_devices]=minor(dev->hdr[dev->total_vs_devices].rdev);

    /* Fill the rest of device entity information */
    dev->entity[0].v4l.major=major(dev->hdr[dev->total_vs_devices].rdev);
    dev->entity[0].v4l.minor=minor(dev->hdr[dev->total_vs_devices].rdev);

    return 0;
}

int uvc_unregister_name(uvc_device_t* dev, int mapid)
{
    int status;
    int it;

    for (it=0; it<dev->total_vs_devices; it++)
    {
        status=rsrcdbmgr_devno_detach(dev->hdr[it].rdev, 0);
        if (status)
        {
            slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] rsrcdbmgr_devno_detach() failed: %s", strerror(errno));
            /* fall through */
        }

        status=resmgr_detach(dispatch, dev->resmgr_id[it],
            _RESMGR_DETACH_ALL | _RESMGR_DETACH_CLOSE);
        if (status)
        {
            slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] resmgr_detach() failed: %s", strerror(errno));
            /* fall through */
        }
    }

    return 0;
}
