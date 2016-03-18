#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

#########################################################################

ifneq ($(OBJTREE),$(SRCTREE))
ifeq ($(CURDIR),$(SRCTREE))
dir :=
else
dir := $(subst $(SRCTREE)/,,$(CURDIR))
endif

obj := $(if $(dir),$(OBJTREE)/$(dir)/,$(OBJTREE)/)
src := $(if $(dir),$(SRCTREE)/$(dir)/,$(SRCTREE)/)

$(shell mkdir -p $(obj))
else
obj :=
src :=
endif

# clean the slate ...
PLATFORM_RELFLAGS =
PLATFORM_CPPFLAGS =
PLATFORM_LDFLAGS =

#########################################################################

ifeq ($(HOSTOS),darwin)
HOSTCC		= cc
else
HOSTCC		= gcc
endif
HOSTCFLAGS	= -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer
HOSTSTRIP	= strip

#########################################################################
#
# Option checker (courtesy linux kernel) to ensure
# only supported compiler options are used
#
cc-option = $(shell if $(CC) $(CFLAGS) $(1) -S -o /dev/null -xc /dev/null \
		> /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi ;)

#
# Include the make variables (CC, etc...)
#
AS	= $(CROSS_COMPILE)as
LD	= $(CROSS_COMPILE)ld
CC	= $(CROSS_COMPILE)gcc
CPP	= $(CC) -E
AR	= $(CROSS_COMPILE)ar
NM	= $(CROSS_COMPILE)nm
LDR	= $(CROSS_COMPILE)ldr
STRIP	= $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
RANLIB	= $(CROSS_COMPILE)RANLIB

#########################################################################

# Load generated board configuration
#sinclude $(OBJTREE)/include/autoconf.mk

CROSS_COMPILE ?= arm-linux-

PLATFORM_CPPFLAGS += -DCONFIG_ARM -D__ARM__

# Explicitly specifiy 32-bit ARM ISA since toolchain default can be -mthumb:
PLATFORM_CPPFLAGS += $(call cc-option,-marm,)

# Try if EABI is supported, else fall back to old API,
# i. e. for example:
# - with ELDK 4.2 (EABI supported), use:
#	-mabi=aapcs-linux -mno-thumb-interwork
# - with ELDK 4.1 (gcc 4.x, no EABI), use:
#	-mabi=apcs-gnu -mno-thumb-interwork
# - with ELDK 3.1 (gcc 3.x), use:
#	-mapcs-32 -mno-thumb-interwork
PLATFORM_CPPFLAGS += $(call cc-option,\
				-mabi=aapcs-linux -mno-thumb-interwork,\
				$(call cc-option,\
					-mapcs-32,\
					$(call cc-option,\
						-mabi=apcs-gnu,\
					)\
				) $(call cc-option,-mno-thumb-interwork,)\
			)

# For EABI, make sure to provide raise()
ifneq (,$(findstring -mabi=aapcs-linux,$(PLATFORM_CPPFLAGS)))
# This file is parsed several times; make sure to add only once.
ifeq (,$(findstring lib_arm/eabi_compat.o,$(PLATFORM_LIBS)))
PLATFORM_LIBS += $(OBJTREE)/${ARCH}/eabi_compat.o
endif
endif


PLATFORM_RELFLAGS += -fno-strict-aliasing -fno-common -ffixed-r8 \
		     -msoft-float

# Make ARMv5 to allow more compilers to work, even though its v7a.
PLATFORM_CPPFLAGS += -march=armv5
# =========================================================================
#
# Supply options according to compiler version
#
# =========================================================================
#ifdef CONFIG_SYS_APCS_GNU
#PLATFORM_CPPFLAGS +=$(call cc-option,-mapcs-32,-mabi=apcs-gnu)
#else
#PLATFORM_CPPFLAGS +=$(call cc-option)
#endif
#PLATFORM_CPPFLAGS +=$(call cc-option,-mno-thumb-interwork,\
#		    $(call cc-option,-mshort-load-bytes,\
#		    $(call cc-option,-malignment-traps,)))

PLATFORM_CPPFLAGS +=$(call cc-option,-mshort-load-bytes,\
		    $(call cc-option,-malignment-traps,))

LDSCRIPT := $(SRCTREE)/imx6q.lds

ifndef TEXT_BASE
	TEXT_BASE = 0x27800000
endif

#########################################################################

ifneq (,$(findstring s,$(MAKEFLAGS)))
ARFLAGS = cr
else
ARFLAGS = crv
endif
RELFLAGS= $(PLATFORM_RELFLAGS)
DBGFLAGS= -g # -DDEBUG
OPTFLAGS= -Os #-fomit-frame-pointer
OBJCFLAGS += --gap-fill=0xff

gccincdir := $(shell $(CC) -print-file-name=include)

CPPFLAGS := $(DBGFLAGS) $(OPTFLAGS) $(RELFLAGS)	\
	-D__KERNEL__

ifneq ($(BOOT_MEDIA),)
	BOOT_MEDIA_MACRO = BOOT_MEDIA_$(shell echo $(BOOT_MEDIA) | tr '[a-z]' '[A-Z]')
	CPPFLAGS += -D$(BOOT_MEDIA_MACRO)
endif

ifneq ($(TEXT_BASE),)
	CPPFLAGS += -DTEXT_BASE=$(TEXT_BASE)
endif

ifneq ($(OBJTREE),$(SRCTREE))
	CPPFLAGS += -I$(OBJTREE)/include2 -I$(OBJTREE)/include
endif

CPPFLAGS += -I$(TOPDIR)/include
CPPFLAGS += -fno-builtin -ffreestanding -nostdinc	\
	-isystem $(gccincdir) -pipe $(PLATFORM_CPPFLAGS)

ifdef BUILD_TAG
CFLAGS := $(CPPFLAGS) -Wall -Wstrict-prototypes \
	-DBUILD_TAG='"$(BUILD_TAG)"'
else
CFLAGS := $(CPPFLAGS) -Wall -Wstrict-prototypes
endif

CFLAGS += $(call cc-option,-fno-stack-protector)

# avoid trigraph warnings while parsing pci.h (produced by NIOS gcc-2.9)
# this option have to be placed behind -Wall -- that's why it is here
ifeq ($(ARCH),nios)
ifeq ($(findstring 2.9,$(shell $(CC) --version)),2.9)
CFLAGS := $(CPPFLAGS) -Wall -Wno-trigraphs
endif
endif

# $(CPPFLAGS) sets -g, which causes gcc to pass a suitable -g<format>
# option to the assembler.
AFLAGS_DEBUG :=

# turn jbsr into jsr for m68k
ifeq ($(ARCH),m68k)
ifeq ($(findstring 3.4,$(shell $(CC) --version)),3.4)
AFLAGS_DEBUG := -Wa,-gstabs,-S
endif
endif

AFLAGS := $(AFLAGS_DEBUG) -D__ASSEMBLY__ $(CPPFLAGS)

LDFLAGS += -Bstatic -T $(obj)imx6q.lds $(PLATFORM_LDFLAGS)
ifneq ($(TEXT_BASE),)
LDFLAGS += -Ttext $(TEXT_BASE)
endif

# Location of a usable BFD library, where we define "usable" as
# "built for ${HOST}, supports ${TARGET}".  Sensible values are
# - When cross-compiling: the root of the cross-environment
# - Linux/ppc (native): /usr
# - NetBSD/ppc (native): you lose ... (must extract these from the
#   binutils build directory, plus the native and U-Boot include
#   files don't like each other)
#
# So far, this is used only by tools/gdb/Makefile.

ifeq ($(HOSTOS),darwin)
BFD_ROOT_DIR =		/usr/local/tools
else
ifeq ($(HOSTARCH),$(ARCH))
# native
BFD_ROOT_DIR =		/usr
else
#BFD_ROOT_DIR =		/LinuxPPC/CDK		# Linux/i386
#BFD_ROOT_DIR =		/usr/pkg/cross		# NetBSD/i386
BFD_ROOT_DIR =		/opt/powerpc
endif
endif

#########################################################################

export	HOSTCC HOSTCFLAGS CROSS_COMPILE \
	AS LD CC CPP AR NM STRIP OBJCOPY OBJDUMP MAKE
export	TEXT_BASE PLATFORM_CPPFLAGS PLATFORM_RELFLAGS CPPFLAGS CFLAGS AFLAGS

#########################################################################

CSRCS := $(wildcard *.c)
COBJS := $(patsubst %.c, %.o,$(notdir ${CSRCS}))

SSRCS := $(wildcard *.S)
SOBJS := $(patsubst %.S, %.o,$(notdir ${SSRCS}))

SRCS  := $(SOBJS:.o=.S) $(COBJS:.o=.c)
COBJS := $(addprefix $(obj),$(COBJS))
SOBJS := $(addprefix $(obj),$(SOBJS))


# Allow boards to use custom optimize flags on a per dir/file basis
BCURDIR := $(notdir $(CURDIR))
$(obj)%.s:	%.S
	$(CPP) $(AFLAGS) $(AFLAGS_$(@F)) $(AFLAGS_$(BCURDIR)) -o $@ $<
$(obj)%.o:	%.S
	$(CC)  $(AFLAGS) $(AFLAGS_$(@F)) $(AFLAGS_$(BCURDIR)) -o $@ $< -c
$(obj)%.o:	%.c
	$(CC)  $(CFLAGS) $(CFLAGS_$(@F)) $(CFLAGS_$(BCURDIR)) -o $@ $< -c
$(obj)%.i:	%.c
	$(CPP) $(CFLAGS) $(CFLAGS_$(@F)) $(CFLAGS_$(BCURDIR)) -o $@ $< -c
$(obj)%.s:	%.c
	$(CC)  $(CFLAGS) $(CFLAGS_$(@F)) $(CFLAGS_$(BCURDIR)) -o $@ $< -c -S

#########################################################################
