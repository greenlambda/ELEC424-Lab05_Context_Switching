# Makefile for Lab05-Context_Switching, ELEC424 Fall 2014.
# Target: CrazyFlie
# Authors: Jeremy Hunt, Christopher Buck. Rice University 
# Date: 10-26-14

# Path Definitions
# This path should point to the STM32 Library directory
STM_LIB ?= local_install/symlinks/stm_std_libs/

# Project Directories
INCDIR = inc
BINDIR = bin
BUILDDIR = build
SRCDIR = src
LIBDIR = lib
LINKER_SCRIPT = linker_script

# These are all the local project source files
SRCS = $(addprefix $(SRCDIR)/, systick_context_switcher.c thread.c doubly_linked_list.c)
SRCS_ASM = $(addprefix $(SRCDIR)/, context_switch.s)
LIBS := $(LIBDIR)/libtasks.a
ELF := $(BINDIR)/systick_context_switcher.elf

# Compiler
CC = arm-none-eabi-gcc

# Particular processor
PROCESSOR = -mcpu=cortex-m3 -mthumb

# STM specific info
STFLAGS = -DSTM32F10X_MD
STM_BUILD_DIR := $(BUILDDIR)/stm_lib
STM_CORE = CMSIS/CM3/CoreSupport
STM_DEVICE_CORE = CMSIS/CM3/DeviceSupport/ST/STM32F10x
STM_DEVICE_PERIPH = STM32F10x_StdPeriph_Driver

# These libary sources will be built into the STM libary for linking
STM_LIB_PKG = $(LIBDIR)/libstm32f10x.a
STM_INCLUDES = $(addprefix -I$(STM_LIB), $(STM_CORE) $(STM_DEVICE_CORE) $(STM_DEVICE_PERIPH)/inc/) -include $(INCDIR)/stm32f10x_conf.h
STM_SOURCES_ASM = $(STM_LIB)/$(STM_DEVICE_CORE)/startup/gcc_ride7/startup_stm32f10x_md.s
STM_SOURCES_C = $(addprefix $(STM_LIB)/$(STM_DEVICE_PERIPH)/src/, stm32f10x_flash.c stm32f10x_gpio.c stm32f10x_rcc.c stm32f10x_tim.c misc.c)
STM_OBJS := $(patsubst $(STM_LIB)/%,$(STM_BUILD_DIR)/%,$(STM_SOURCES_C:.c=.o) $(STM_SOURCES_ASM:.s=.o)) 
STM_DEPS := $(patsubst $(STM_LIB)/%,$(STM_BUILD_DIR)/%,$(STM_SOURCES_C:.c=.d))

# Directories of used header files
INCLUDE = -I$(INCDIR) $(STM_INCLUDES)

# Define the compiler flags
CFLAGS = -g -Wall -Wextra -Werror $(PROCESSOR) $(INCLUDE) $(STFLAGS) -Wl,--gc-sections -T $(LINKER_SCRIPT)/stm32_flash.ld

# This clever bit of make-fu builds dependency files for each source file so
# that if the included files for that source file are updated, the object for
# that file is also rebuilt. This rule generates a coresponding %.d file in
# the build directory for each source file.
DEPGENFLAGS = -MMD -MP

# Create the objects and dependencies based on the sources
OBJS = $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SRCS:.c=.o) $(SRCS_ASM:.s=.o))
DEPENDENCIES := $(STM_DEPS) $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SRCS:.c=.d))

# Build all relevant files and create .elf
all: $(ELF)
	
# Build source files into objects
$(STM_BUILD_DIR)/%.o: $(STM_LIB)/%.s
	@mkdir -p $(patsubst %/,%,$(dir $@)) # Create necessary dirs in build
	$(CC) $(CFLAGS) -o $@ -c $<

$(STM_BUILD_DIR)/%.o: $(STM_LIB)/%.c
	@mkdir -p $(patsubst %/,%,$(dir $@)) # Create necessary dirs in build
	$(CC) $(DEPGENFLAGS) $(CFLAGS) -o $@ -c $<

$(BUILDDIR)/%.o: $(SRCDIR)/%.s
	@mkdir -p $(patsubst %/,%,$(dir $@)) # Create necessary dirs in build
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(patsubst %/,%,$(dir $@)) # Create necessary dirs in build
	$(CC) $(DEPGENFLAGS) $(CFLAGS) -o $@ -c $<
	
# Archive the STM object files into a library 
$(STM_LIB_PKG): $(STM_OBJS) 
	ar rcs $@ $^

# Link the program's object files and the libraries together into an executable elf file.
$(ELF): $(OBJS) $(LIBS) $(STM_LIB_PKG)
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: flash debug clean

# Program .elf into Crazyflie flash memory via the busblaster
flash: $(ELF)
	@openocd -d0 -f interface/busblaster.cfg -f target/stm32f1x.cfg -c init -c targets -c "reset halt" -c "flash write_image erase $(ELF)" -c "verify_image $(ELF)" -c "reset run" -c shutdown

# Runs OpenOCD, opens GDB terminal, and establishes connection with Crazyflie
debug: $(ELF)
	@xterm -iconic -e openocd -f interface/busblaster.cfg -f target/stm32f1x.cfg -c 'init; targets; reset halt;' &
	@arm-none-eabi-gdb -ex 'target remote localhost:3333' $(ELF)
	@killall openocd

# Remove all files generated by target 'all'
clean:
	rm -rf $(BUILDDIR)
	rm -f $(STM_LIB_PKG) $(ELF)

# Weakly include the dependencies. If there is no rule to rebuild them,
# make won't try, so phony targets still work properly.
-include $(DEPENDENCIES)


