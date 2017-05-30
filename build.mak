# PSP Software Development Kit - http://www.pspdev.org
# -----------------------------------------------------------------------
# Licensed under the BSD license, see LICENSE in PSPSDK root for details.
#
# build.mak - Base makefile for projects using PSPSDK.
#
# Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
# Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
# Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
#
# $Id: build.mak 2333 2007-10-31 19:37:40Z tyranid $

# Note: The PSPSDK make variable must be defined before this file is included.
ifeq ($(PSPSDK),)
$(error $$(PSPSDK) is undefined.  Use "PSPSDK := $$(shell psp-config --pspsdk-path)" in your Makefile)
endif

ifeq ($(DIR_SRC),)
DIR_SRC = .
endif

ifeq ($(DIR_BIN),)
DIR_BIN = .
endif

ifeq ($(DIR_TMP),)
DIR_TMP = .
endif

FINAL_PRX = $(DIR_TMP)/$(TARGET).prx
FINAL_ELF = $(DIR_TMP)/$(TARGET).elf
FINAL_OBJS = $(addprefix $(DIR_TMP)/, $(OBJS))
ifndef ($(EXTRA_TARGETS),)
EXTRA_TARGETS := $(addprefix $(DIR_BIN)/, $(EXTRA_TARGETS))
endif

CC       = psp-gcc
CXX      = psp-g++
AS       = psp-gcc
LD       = psp-gcc
AR       = psp-ar
RANLIB   = psp-ranlib
STRIP    = psp-strip
MKSFO    = mksfo
PACK_PBP = pack-pbp
FIXUP    = psp-fixup-imports
ENC		 = PrxEncrypter
MD       = mkdir
CP		 = cp

# ifneq ($(NO_AUTO_STDCVER),0)

# MAIN_VER = $(shell $(CC) -dumpversion | cut -f1 -d.)
# MINOR_VER = $(shell $(CC) -dumpversion | cut -f2 -d.)
# STDCFLAG = -std=c99

# ifeq ($(shell expr $(MAIN_VER) \>= 4), 1)
	# ifeq ($(shell expr $(MAIN_VER) \>= 5 \| $(MINOR_VER) \>= 7), 1)
		# STDCFLAG = -std=c11
	# else
		# ifeq ($(shell expr $(MINOR_VER) \>= 6), 1)
			# STDCFLAG = -std=c1x
		# endif
	# endif
# endif

# CFLAGS += $(STDCFLAG)
# endif

# Add in PSPSDK includes and libraries.
INCDIR   := $(INCDIR) $(DIR_SRC) $(PSPSDK)/include
LIBDIR   := $(LIBDIR) $(DIR_BIN) $(PSPSDK)/lib

INCDIR_FLAGS  := $(addprefix -I,$(INCDIR))
LIBDIR_FLAGS  := $(addprefix -L,$(LIBDIR))

ifeq ($(PSP_FW_VERSION),)
PSP_FW_VERSION=150
endif

CFLAGS += -D_PSP_FW_VERSION=$(PSP_FW_VERSION)
CXXFLAGS += -D_PSP_FW_VERSION=$(PSP_FW_VERSION)

ifeq ($(BUILD_PRX),1)
LDFLAGS := -specs=$(PSPSDK)/lib/prxspecs \
    -Wl,-q,-T$(PSPSDK)/lib/linkfile.prx -nostartfiles $(LDFLAGS)
EXTRA_CLEAN += $(FINAL_ELF)
# Setup default exports if needed
ifdef PRX_EXPORTS
EXPORT_OBJ=$(patsubst %.exp,%.o,$(PRX_EXPORTS))
EXTRA_CLEAN += $(DIR_TMP)/$(EXPORT_OBJ)
else 
EXPORT_OBJ=$(PSPSDK)/lib/prxexports.o
endif
endif

# Library selection.  By default we link with Newlib's libc.  Allow the
# user to link with PSPSDK's libc if USE_PSPSDK_LIBC is set to 1.

ifeq ($(USE_KERNEL_LIBC),1)
# Use the PSP's kernel libc
PSPSDK_LIBC_LIB = 
CFLAGS := -I$(PSPSDK)/include/libc $(CFLAGS)
else
ifeq ($(USE_PSPSDK_LIBC),1)
# Use the pspsdk libc
PSPSDK_LIBC_LIB = -lpsplibc
CFLAGS := -I$(PSPSDK)/include/libc $(CFLAGS)
else
# Use newlib (urgh)
PSPSDK_LIBC_LIB = -lc
endif
endif


# Link with following default libraries.  Other libraries should be specified in the $(LIBS) variable.
# TODO: This library list needs to be generated at configure time.
#
ifeq ($(USE_KERNEL_LIBS),1)
PSPSDK_LIBS = -lpspdebug -lpspdisplay_driver -lpspctrl_driver -lpspsdk
LIBS     := $(LIBS) $(PSPSDK_LIBS) $(PSPSDK_LIBC_LIB) -lpspkernel
else
ifeq ($(USE_USER_LIBS),1)
PSPSDK_LIBS = -lpspdebug -lpspdisplay -lpspge -lpspctrl -lpspsdk
LIBS     := $(LIBS) $(PSPSDK_LIBS) $(PSPSDK_LIBC_LIB) -lpspnet \
			-lpspnet_inet -lpspnet_apctl -lpspnet_resolver -lpsputility \
			-lpspuser
else
PSPSDK_LIBS = -lpspdebug -lpspdisplay -lpspge -lpspctrl -lpspsdk
LIBS     := $(LIBS) $(PSPSDK_LIBS) $(PSPSDK_LIBC_LIB) -lpspnet \
			-lpspnet_inet -lpspnet_apctl -lpspnet_resolver -lpsputility \
			-lpspuser -lpspkernel
endif
endif

# Define the overridable parameters for EBOOT.PBP
ifndef PSP_EBOOT_TITLE
PSP_EBOOT_TITLE = $(TARGET)
endif

ifndef PSP_EBOOT_SFO
PSP_EBOOT_SFO = PARAM.SFO
endif

ifndef PSP_EBOOT_ICON
PSP_EBOOT_ICON = NULL
endif

ifndef PSP_EBOOT_ICON1
PSP_EBOOT_ICON1 = NULL
endif

ifndef PSP_EBOOT_UNKPNG
PSP_EBOOT_UNKPNG = NULL
endif

ifndef PSP_EBOOT_PIC1
PSP_EBOOT_PIC1 = NULL
endif

ifndef PSP_EBOOT_SND0
PSP_EBOOT_SND0 = NULL
endif

ifndef PSP_EBOOT_PSAR
PSP_EBOOT_PSAR = NULL
endif

ifndef PSP_EBOOT
PSP_EBOOT = EBOOT.PBP
endif

ifeq ($(BUILD_PRX),1)
ifneq ($(TARGET_LIB),)
$(error TARGET_LIB should not be defined when building a prx)
else
FINAL_TARGET = $(DIR_BIN)/$(TARGET).prx
endif
else
ifneq ($(TARGET_LIB),)
FINAL_TARGET = $(DIR_BIN)/$(TARGET_LIB)
else
FINAL_TARGET = $(DIR_BIN)/$(PSP_EBOOT)
endif
endif

all: $(DIR_BIN) $(DIR_TMP) $(EXTRA_TARGETS) $(FINAL_TARGET)

$(DIR_BIN):
	$(MD) -p $(DIR_BIN)

$(DIR_TMP):
	$(MD) -p $(DIR_TMP)

$(DIR_BIN)/%: $(DIR_TMP)/%
	$(CP) -f $^ $@

$(FINAL_ELF): $(FINAL_OBJS) $(EXPORT_OBJ)
	$(CC) $(LIBDIR_FLAGS) $(LDFLAGS) $^ $(LIBS) -o $@
ifneq ($(NO_FIXUP_IMPORTS), 1)
	$(FIXUP) $@
endif

$(DIR_BIN)/$(TARGET_LIB): $(FINAL_OBJS)
	$(AR) cru $@ $(FINAL_OBJS)
	$(RANLIB) $@

$(DIR_BIN)/$(PSP_EBOOT_SFO): 
	$(MKSFO) '$(PSP_EBOOT_TITLE)' $@
	
ifeq ($(BUILD_PRX),1)
$(DIR_BIN)/$(PSP_EBOOT): $(FINAL_PRX) $(DIR_BIN)/$(PSP_EBOOT_SFO)
ifeq ($(ENCRYPT), 1)
	- $(ENC) $(FINAL_PRX) $(FINAL_PRX)
endif
	$(PACK_PBP) $(DIR_BIN)/$(PSP_EBOOT) $(PSP_EBOOT_SFO) $(PSP_EBOOT_ICON)  \
		$(PSP_EBOOT_ICON1) $(PSP_EBOOT_UNKPNG) $(PSP_EBOOT_PIC1)  \
		$(PSP_EBOOT_SND0)  $(FINAL_PRX) $(PSP_EBOOT_PSAR)
else
$(DIR_BIN)/$(PSP_EBOOT): $(FINAL_ELF) $(DIR_BIN)/$(PSP_EBOOT_SFO)
	$(STRIP) $(FINAL_ELF) -o $(DIR_TMP)/$(TARGET)_strip.elf
	$(PACK_PBP) $(DIR_BIN)/$(PSP_EBOOT) $(DIR_BIN)/$(PSP_EBOOT_SFO) $(PSP_EBOOT_ICON)  \
		$(PSP_EBOOT_ICON1) $(PSP_EBOOT_UNKPNG) $(PSP_EBOOT_PIC1)  \
		$(PSP_EBOOT_SND0)  $(DIR_TMP)/$(TARGET)_strip.elf $(PSP_EBOOT_PSAR)
	-rm -f $(DIR_TMP)/$(TARGET)_strip.elf
endif

$(FINAL_PRX): $(FINAL_ELF)
	psp-prxgen $< $@

$(DIR_SRC)/%.c: $(DIR_SRC)/%.exp
	psp-build-exports -b $< > $@

$(DIR_TMP)/%.o: $(DIR_SRC)/%.S
	$(CC) -c $< -o $@ $(INCDIR_FLAGS) $(CFLAGS)

$(DIR_TMP)/%.o: $(DIR_SRC)/%.c
	$(CC) -c $< -o $@ $(INCDIR_FLAGS) $(CFLAGS)
	
$(DIR_TMP)/%.o: $(DIR_SRC)/%.cpp
	$(CXX) -c $< -o $@ $(INCDIR_FLAGS) $(CXXFLAGS)

$(DIR_TMP)/%.o: $(DIR_SRC)/%.cc
	$(CXX) -c $< -o $@ $(INCDIR_FLAGS) $(CXXFLAGS)

clean: 
	-rm -f $(FINAL_TARGET) $(EXTRA_CLEAN) $(FINAL_ELF) $(FINAL_OBJS) $(DIR_BIN)/$(PSP_EBOOT_SFO) $(DIR_BIN)/$(PSP_EBOOT) $(EXTRA_TARGETS)

rebuild: clean all
