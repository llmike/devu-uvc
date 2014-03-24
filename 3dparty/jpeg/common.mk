ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

CCFLAGS+=-O3 -march=core2
EXCLUDE_OBJS += cdjpeg.o cjpeg.o ckconfig.o djpeg.o example.o rdbmp.o \
                rdcolmap.o rdgif.o rdjpgcom.o rdppm.o rdrle.o rdswitch.o \
                rdtarga.o transupp.o wrbmp.o wrgif.o wrjpgcom.o wrppm.o \
                wrrle.o wrtarga.o jmemmac.o jmemdos.o jmemname.o jmemnobs.o

define PINFO
PINFO DESCRIPTION=JPEG library
endef

include $(MKFILES_ROOT)/qtargets.mk
-include $(PROJECT_ROOT)/roots.mk
