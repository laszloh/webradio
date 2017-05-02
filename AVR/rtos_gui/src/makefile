# WinAVR Sample makefile written by Eric B. Weddington, Jörg Wunsch, et al.
# Released to the Public Domain
# Please read the make user manual!
#
# Additional material for this makefile was submitted by:
#  Tim Henigan
#  Peter Fleury
#  Reiner Patommel
#  Sander Pool
#  Frederik Rouleau
#  Markus Pfaff
#
# On command line:
#
# make all = Make software.
#
# make clean = Clean out built project files.
#
# make coff = Convert ELF to AVR COFF (for use with AVR Studio 3.x or VMLAB).
#
# make extcoff = Convert ELF to AVR Extended COFF (for use with AVR Studio
#                4.07 or greater).
#
# make program = Download the hex file to the device, using avrdude.  Please
#                customize the avrdude settings below first!
#
# make filename.s = Just compile filename.c into the assembler code only
#
# To rebuild project do "make clean" then "make all".
#

MCU           = atmega32u4
ARCH          = AVR8
BOARD         = USER
F_CPU         = 16000000
F_USB         = $(F_CPU)
OPTIMIZATION  = s
TARGET        = rtos_gui

# Library paths
LUFA_PATH     = ../lib/lufa/LUFA
RTOS_DIR      = ../lib/freertos/FreeRTOS/Source
PORT_DIR      = ./portable/atmega32u4
IRMP_DIR      = ../lib/irmp

# List any extra directories to look for include files here.
#     Each directory must be seperated by a space.
EXTRAINCDIRS += ./config ./portable/atmega32u4
EXTRAINCDIRS += ../lib/irmp 
EXTRAINCDIRS += ../lib/lufa
EXTRAINCDIRS += ../lib/freertos/FreeRTOS/Source/include

# List all extra defines here
#     Each define must be seperated by a space.
EXTRADEFS   += IRMP_USE_AS_LIB
EXTRADEFS   += USE_LUFA_CONFIG_HEADER

SRC	         = main.c Descriptors.c \
$(LUFA_SRC_USB) $(LUFA_SRC_USBCLASS) \
$(RTOS_DIR)/tasks.c \
$(RTOS_DIR)/queue.c \
$(RTOS_DIR)/list.c \
$(RTOS_DIR)/croutine.c \
$(RTOS_DIR)/portable/MemMang/heap_1.c \
$(PORT_DIR)/port.c \
$(IRMP_DIR)/irmp.c

CC_FLAGS    = -flto -fshort-enums -DIRMP_USE_AS_LIB -DUSE_LUFA_CONFIG_HEADER -include "global.h"
CC_FLAGS   += -ffreestanding
CC_FLAGS   += $(patsubst %,-I%,$(EXTRAINCDIRS)) $(patsubst %,-D%,$(EXTRADEFS))

LD_FLAGS     = -flto -Os

# Default target
all:

# Include LUFA-specific DMBS extension modules
DMBS_LUFA_PATH ?= $(LUFA_PATH)/Build/LUFA
include $(DMBS_LUFA_PATH)/lufa-sources.mk
include $(DMBS_LUFA_PATH)/lufa-gcc.mk

# Include common DMBS build system modules
DMBS_PATH      ?= $(LUFA_PATH)/Build/DMBS/DMBS
include $(DMBS_PATH)/core.mk
include $(DMBS_PATH)/cppcheck.mk
include $(DMBS_PATH)/doxygen.mk
include $(DMBS_PATH)/dfu.mk
include $(DMBS_PATH)/gcc.mk
include $(DMBS_PATH)/hid.mk
include $(DMBS_PATH)/avrdude.mk
include $(DMBS_PATH)/atprogram.mk

