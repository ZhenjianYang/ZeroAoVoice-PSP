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
# $Id: build.mak 771 2005-07-24 10:43:54Z tyranid $

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

FINAL_TARGET = $(DIR_BIN)/$(TARGET).$(ATTR)
FINAL_ELF = $(DIR_TMP)/$(TARGET).elf
FINAL_OBJS = $(addprefix $(DIR_TMP)/, $(OBJS))

ifneq ($(RCS),)
FINAL_OBJS += $(DIR_TMP)/$(patsubst %.dat,%.o,$(RCS))
endif

ifdef PRX_EXPORTS
EXPORT_OBJ  += $(DIR_TMP)/$(patsubst %.exp,%.o,$(PRX_EXPORTS))
EXTRA_CLEAN += $(EXPORT_OBJ)
else 
EXPORT_OBJ+=$(PSPSDK)/lib/prxexports.o
endif

CC       = psp-gcc
CXX      = psp-g++
AS       = psp-gcc
LD       = psp-ld
FIXUP    = psp-fixup-imports
MD       = mkdir

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

LDFLAGS := -specs=$(PSPSDK)/lib/prxspecs \
    -Wl,-q,-T$(PSPSDK)/lib/linkfile.prx -nostartfiles $(LDFLAGS)


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
ifeq ($(USE_KERNEL_LIBS),1)
PSPSDK_LIBS = -lpspdebug -lpspdisplay_driver -lpspctrl_driver -lpspsdk
LIBS     := $(LIBS) $(PSPSDK_LIBS) $(PSPSDK_LIBC_LIB) -lpspkernel
else
PSPSDK_LIBS = -lpspdebug -lpspdisplay -lpspge -lpspctrl -lpspsdk
LIBS     := $(LIBS) $(PSPSDK_LIBS) $(PSPSDK_LIBC_LIB) -lpsputility -lpspuser -lpspkernel
endif

all: $(DIR_BIN) $(DIR_TMP) $(FINAL_TARGET)

$(DIR_BIN):
	$(MD) -p $(DIR_BIN)

$(DIR_TMP):
	$(MD) -p $(DIR_TMP)

$(DIR_BIN)/%.BIN: $(DIR_TMP)/%.elf
	psp-prxgen $< $@

$(DIR_BIN)/%.prx: $(DIR_TMP)/%.elf
	psp-prxgen $< $@

$(FINAL_ELF): $(FINAL_OBJS) $(EXPORT_OBJ)
	$(CC) $(LIBDIR_FLAGS) $(LDFLAGS) $^ $(LIBS) -o $@
ifneq ($(NO_FIXUP_IMPORTS), 1)
	$(FIXUP) $@
endif

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

$(DIR_TMP)/%.o: $(DIR_SRC)/%.dat
	$(LD) -r -b binary -o $@ $^

clean:
	-rm -f $(FINAL_TARGET) $(FINAL_ELF) $(FINAL_OBJS) $(EXTRA_CLEAN)

rebuild: clean all
