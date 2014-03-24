ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

CCFLAGS+=-DHAVE_CONFIG_H -O3 -march=core2
EXCLUDE_OBJS += randutils.o

define PINFO
PINFO DESCRIPTION=UUID library
endef

include $(MKFILES_ROOT)/qtargets.mk
-include $(PROJECT_ROOT)/roots.mk
