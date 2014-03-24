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
#include "uvc_sysfs.h"
#include "uvc_driver.h"
#include "uvc_devctl.h"

extern uvc_device_mapping_t devmap[MAX_UVC_DEVICES];
extern int uvc_verbose;

uvc_ocb_t* _uvc_sysfs_ocb_calloc(resmgr_context_t* ctp, uvc_device_t* dev)
{
    uvc_ocb_t* ocb;
    int it;
    int jt;
    int kt;

    ocb = calloc(1, sizeof(uvc_ocb_t));
    ocb->dev=NULL;
    ocb->handle=dev;
    ocb->subdev=-1;

    if (ocb!=NULL)
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
                    if (devmap[it].uvcd->sysfs[jt]!=NULL)
                    {
                        for(kt=0; kt<UVC_SYSFS_FILES; kt++)
                        {
                            if (&devmap[it].uvcd->sysfs[jt]->hdr[kt]==(void*)dev)
                            {
                                ocb->dev=devmap[it].uvcd;
                                ocb->subdev=jt;
                                break;
                            }
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

void _uvc_sysfs_ocb_free(uvc_ocb_t* ocb)
{
    free(ocb);
}

int uvc_open_sysfs(resmgr_context_t* ctp, io_open_t* msg, RESMGR_HANDLE_T* handle, void* extra)
{
    int status;

    status=iofunc_open_default(ctp, msg, (iofunc_attr_t*)handle, extra);
    if (status)
    {
        return status;
    }

    return EOK;
}

int uvc_readlink_sysfs(resmgr_context_t* ctp, io_readlink_t* msg, RESMGR_HANDLE_T* handle, void* reserved)
{
    int it, jt, kt;
    struct _uvc_sysfs_device* sysfs=NULL;
    struct _io_connect_link_reply* link=(struct _io_connect_link_reply*)msg;
    char* path_str;
    int buffer_size=msg->connect.reply_max;
    int fileid=-1;
    int status;

    status=iofunc_readlink(ctp, msg, (iofunc_attr_t*)handle, reserved);
    if (status)
    {
        return status;
    }

    for (it=0; it<MAX_UVC_DEVICES; it++)
    {
        if ((devmap[it].initialized) && (devmap[it].uvcd!=NULL))
        {
            for(jt=0; jt<devmap[it].uvcd->total_vs_devices; jt++)
            {
                if (devmap[it].uvcd->sysfs[jt]!=NULL)
                {
                    for(kt=0; kt<UVC_SYSFS_FILES; kt++)
                    {
                        if (&devmap[it].uvcd->sysfs[jt]->hdr[kt]==(void*)handle)
                        {
                            sysfs=devmap[it].uvcd->sysfs[jt];
                            fileid=kt;
                            break;
                        }
                    }
                }
            }
        }
    }

    if (sysfs==NULL)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] readlink(): can't locate sysfs file!");
        return ENODEV;
    }

    link->reserved1=0;
    link->file_type=_FTYPE_SYMLINK;
    link->eflag=_IO_CONNECT_EFLAG_DIR;
    link->reserved2[0]=0;
    link->chroot_len=0;
    link->umask=0;
    link->nentries=0;
    link->path_len=strlen(&sysfs->filedata[fileid][0]);
    path_str=(char*)(link+1);
    strncpy(path_str, &sysfs->filedata[fileid][0], buffer_size);

    return _RESMGR_PTR(ctp, link, sizeof(*link)+link->path_len);
}

int uvc_close_sysfs(resmgr_context_t* ctp, void* reserved, uvc_ocb_t* ocb)
{
    int status;

    status=iofunc_close_ocb_default(ctp, reserved, (iofunc_ocb_t*)ocb);
    if (status)
    {
        return status;
    }

    return EOK;
}

int uvc_read_sysfs(resmgr_context_t *ctp, io_read_t *msg, uvc_ocb_t *ocb)
{
    uvc_device_t* dev=ocb->dev;
    int fileno=-1;
    int result;
    int it;
    int nparts=0;
    int nonblock=0;
    int nbytes;
    int nleft;

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

    /* Locate sysfs file id */
    for (it=0; it<UVC_SYSFS_FILES; it++)
    {
        if (ocb->handle==&dev->sysfs[ocb->subdev]->hdr[it])
        {
            fileno=it;
            break;
        }
    }
    if (fileno==-1)
    {
        slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] read(): can't locate sysfs file!");
        _IO_SET_READ_NBYTES(ctp, 0);
        return ENODEV;
    }

    nleft=ocb->hdr.attr->hdr->nbytes-ocb->hdr.offset;
    nbytes=min(msg->i.nbytes, nleft);

    if (nbytes>0)
    {
         SETIOV(ctp->iov, &dev->sysfs[ocb->subdev]->filedata[fileno][ocb->hdr.offset], nbytes);
         _IO_SET_READ_NBYTES(ctp, nbytes);
         ocb->hdr.offset+=nbytes;
         nparts=1;
    }
    else
    {
        _IO_SET_READ_NBYTES(ctp, 0);
    }

    return (_RESMGR_NPARTS(nparts));
}

char* sysfs_files[UVC_SYSFS_FILES]=
{
    "/sys/class/video4linux/video%d/dev",
    "/sys/class/video4linux/video%d/index",
    "/sys/class/video4linux/video%d/name",
    "/sys/class/video4linux/video%d/uevent",
    "/sys/class/video4linux/video%d/idVendor",
    "/sys/class/video4linux/video%d/idProduct",
    "/sys/class/video4linux/video%d/speed",
    "/sys/class/video4linux/video%d/device/modalias",
    "/sys/dev/char/%d:%d"
};

int uvc_register_sysfs(uvc_device_t* dev, int mapid)
{
    char name[PATH_MAX];
    struct _uvc_sysfs_device* sysfs;
    int it;

    if (dev->sysfs[dev->total_vs_devices])
    {
        free(dev->sysfs[dev->total_vs_devices]);
        dev->sysfs[dev->total_vs_devices]=NULL;
    }
    dev->sysfs[dev->total_vs_devices]=calloc(1, sizeof(struct _uvc_sysfs_device));
    if (dev->sysfs[dev->total_vs_devices]==NULL)
    {
        return -1;
    }
    sysfs=dev->sysfs[dev->total_vs_devices];

    for (it=0; it<UVC_SYSFS_FILES; it++)
    {
        memset(&sysfs->ocb_funcs[it], 0x00, sizeof(sysfs->ocb_funcs[it]));
        sysfs->ocb_funcs[it].nfuncs=_IOFUNC_NFUNCS;
        sysfs->ocb_funcs[it].ocb_calloc=_uvc_sysfs_ocb_calloc;
        sysfs->ocb_funcs[it].ocb_free=_uvc_sysfs_ocb_free;
        memset(&sysfs->io_mount[it], 0x00, sizeof(sysfs->io_mount[it]));
        sysfs->io_mount[it].funcs=&sysfs->ocb_funcs[it];

        memset(&sysfs->rattr[it], 0, sizeof(sysfs->rattr[it]));
        iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &sysfs->connect_funcs[it],
                         _RESMGR_IO_NFUNCS, &sysfs->io_funcs[it]);
        iofunc_attr_init(&sysfs->hdr[it], S_IRUSR | S_IRGRP |
            S_IROTH, NULL, NULL);

        sysfs->connect_funcs[it].open=uvc_open_sysfs;
        if (it==UVC_SYSFS_FILE_DEV_CHAR)
        {
            sysfs->connect_funcs[it].readlink=uvc_readlink_sysfs;
        }
        sysfs->io_funcs[it].read=uvc_read_sysfs;
        sysfs->io_funcs[it].close_ocb=uvc_close_sysfs;

        sysfs->hdr[it].mount=&sysfs->io_mount[it];

        if (it!=UVC_SYSFS_FILE_DEV_CHAR)
        {
            snprintf(name, PATH_MAX, sysfs_files[it], minor(dev->hdr[dev->total_vs_devices].rdev));
        }
        else
        {
            snprintf(name, PATH_MAX, sysfs_files[it], major(dev->hdr[dev->total_vs_devices].rdev), minor(dev->hdr[dev->total_vs_devices].rdev));
        }

        sysfs->resmgr_id[it]=resmgr_attach(dispatch,
            &sysfs->rattr[it], name, _FTYPE_ANY, 0,
            &sysfs->connect_funcs[it],
            &sysfs->io_funcs[it], (RESMGR_HANDLE_T*)&sysfs->hdr[it]);

        if (sysfs->resmgr_id[it]==-1)
        {
            slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] resmgr_attach() failed: %s", strerror(errno));
            return -1;
        }

        resmgr_devino(sysfs->resmgr_id[it], &sysfs->io_mount[it].dev, &sysfs->hdr[it].inode);
        /* Pretend it is a generic file */
        if (it!=UVC_SYSFS_FILE_DEV_CHAR)
        {
            sysfs->hdr[it].mode|=_S_IFREG;
        }
        else
        {
            sysfs->hdr[it].mode|=_S_IFLNK;
        }

        switch (it)
        {
            case UVC_SYSFS_FILE_DEV:
                 snprintf(&sysfs->filedata[it][0], UVC_SYSFS_MAX_FILESIZE, "%d:%d\n",
                     major(dev->hdr[dev->total_vs_devices].rdev), minor(dev->hdr[dev->total_vs_devices].rdev));
                 break;
            case UVC_SYSFS_FILE_INDEX:
                 snprintf(&sysfs->filedata[it][0], UVC_SYSFS_MAX_FILESIZE, "%d\n",
                     minor(dev->hdr[dev->total_vs_devices].rdev));
                 break;
            case UVC_SYSFS_FILE_NAME:
                 snprintf(&sysfs->filedata[it][0], UVC_SYSFS_MAX_FILESIZE, "%s\n",
                     dev->device_id_str);
                 break;
            case UVC_SYSFS_FILE_UEVENT:
                 snprintf(&sysfs->filedata[it][0], UVC_SYSFS_MAX_FILESIZE,
                     "MAJOR=%d\nMINOR=%d\nDEVNAME=video%d\n",
                     major(dev->hdr[dev->total_vs_devices].rdev),
                     minor(dev->hdr[dev->total_vs_devices].rdev),
                     minor(dev->hdr[dev->total_vs_devices].rdev));
                 break;
            case UVC_SYSFS_FILE_IDVENDOR:
                 snprintf(&sysfs->filedata[it][0], UVC_SYSFS_MAX_FILESIZE, "%04x\n",
                     dev->vendor_id);
                 break;
            case UVC_SYSFS_FILE_IDPRODUCT:
                 snprintf(&sysfs->filedata[it][0], UVC_SYSFS_MAX_FILESIZE, "%04x\n",
                     dev->device_id);
                 break;
            case UVC_SYSFS_FILE_SPEED:
                 switch (dev->port_speed)
                 {
                     case 0:
                          strncpy(&sysfs->filedata[it][0], "1\n", UVC_SYSFS_MAX_FILESIZE);
                          break;
                     case 1:
                          strncpy(&sysfs->filedata[it][0], "12\n", UVC_SYSFS_MAX_FILESIZE);
                          break;
                     case 2:
                          strncpy(&sysfs->filedata[it][0], "480\n", UVC_SYSFS_MAX_FILESIZE);
                          break;
                     case 3:
                          strncpy(&sysfs->filedata[it][0], "5000\n", UVC_SYSFS_MAX_FILESIZE);
                          break;
                 }
                 break;
            case UVC_SYSFS_FILE_DEVICE_MODALIAS:
                 snprintf(&sysfs->filedata[it][0], UVC_SYSFS_MAX_FILESIZE, "usb:v%04xp%04xd\n",
                     dev->vendor_id, dev->device_id);
                 break;
            case UVC_SYSFS_FILE_DEV_CHAR:
                 snprintf(&sysfs->filedata[it][0], UVC_SYSFS_MAX_FILESIZE, "/sys/class/video4linux/video%d",
                     minor(dev->hdr[dev->total_vs_devices].rdev));
                 break;
        }

        /* Set sysfs file size */
        sysfs->hdr[it].nbytes=strlen(&sysfs->filedata[it][0]);
    }

    return 0;
}

int uvc_unregister_sysfs(uvc_device_t* dev, int mapid)
{
    struct _uvc_sysfs_device* sysfs;
    int status;
    int it;
    int jt;

    for (it=0; it<dev->total_vs_devices; it++)
    {
        sysfs=dev->sysfs[it];
        if (sysfs==NULL)
        {
            continue;
        }

        for (jt=0; jt<UVC_SYSFS_FILES; jt++)
        {
            if ((sysfs->resmgr_id[jt]==-1) || (sysfs->resmgr_id[jt]==0))
            {
                continue;
            }
            status=resmgr_detach(dispatch, sysfs->resmgr_id[jt],
                _RESMGR_DETACH_ALL | _RESMGR_DETACH_CLOSE);
            if (status)
            {
                slogf(_SLOGC_USB_GEN, _SLOG_ERROR, "[devu-uvc] resmgr_detach() failed: %s", strerror(errno));
                /* fall through */
            }
        }
        free(dev->sysfs[it]);
        dev->sysfs[it]=NULL;
    }

    return 0;
}
