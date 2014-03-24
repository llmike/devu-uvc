ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR=sbin

EXTRA_INCVPATH+=../../../3dparty/uuid/
EXTRA_LIBVPATH+=../../../3dparty/uuid/nto/x86/a/
EXTRA_INCVPATH+=../../../3dparty/jpeg/
EXTRA_LIBVPATH+=../../../3dparty/jpeg/nto/x86/a/
EXTRA_INCVPATH+=../../../include/

LIBS+=usbdi uuid jpeg

CCFLAGS+=-O3 -march=core2

define PINFO
PINFO DESCRIPTION=USB Video Class driver
endef

USEFILE=../../../devu-uvc.use

include $(MKFILES_ROOT)/qtargets.mk
-include $(PROJECT_ROOT)/roots.mk
