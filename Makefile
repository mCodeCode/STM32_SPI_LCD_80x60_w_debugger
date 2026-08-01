# **************************************************************************** #
#                                                                              #
# Copyright (c) 2017 Andrea Loi                                                #
#                                                                              #
# Permission is hereby granted, free of charge, to any person obtaining a      #
# copy of this software and associated documentation files (the "Software"),    #
# to deal in the Software without restriction, including without limitation    #
# the rights to use, copy, modify, merge, publish, distribute, sublicense,     #
# and/or sell copies of the Software, and to permit persons to whom the        #
# Software is furnished to do so, subject to the following conditions:         #
#                                                                              #
# The above copyright notice and this permission notice shall be included      #
# in all copies or substantial portions of the Software.                       #
#                                                                              #
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR   #
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,     #
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL      #
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER   #
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING      #
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER          #
# DEALINGS IN THE SOFTWARE.                                                    #
#                                                                              #
# **************************************************************************** #



# **************************************************************************** #
#                    < Project configuration variables >                       #
# **************************************************************************** #
# You probably need to change these.                                           #
# **************************************************************************** #



# **************************************************************************** #
# TARGET: This sets the name of the program.                                   #
#         It's used as a prefix for the output files.                          #
# **************************************************************************** #
TARGET = hello_led_stm32

# **************************************************************************** #
# BUILD_DIR: Directory where all compiled outputs will be stored.              #
# **************************************************************************** #
BUILD_DIR = compiledProgram



# **************************************************************************** #
# OTHER_SRCS: List here all the .c files that needs to be compiled.            #
#             You can separete them using a space.                             #
# **************************************************************************** #
OTHER_SRCS = main.c gpio_spi_helper.c drawlib.c



# **************************************************************************** #
# OPTFLAGS: Select GCC optimization level.                                     #
#           This is only used for the release build ("make").                  #
#           The debug build ("make debug") optimization flags are set          #
#           in DBG_OPTFLAGS down below.                                        #
#           Also be aware that some levels (usually -O3) may require you to    #
#           add additional support functions like memcpy and memset.           #
# **************************************************************************** #
OPTFLAGS = -O0 -g



# **************************************************************************** #
#                    < Advanced configuration variables >                      #
# **************************************************************************** #
# You probably don't need to change these.                                     #
# **************************************************************************** #
INIT_SRC = init.c
LINKER_SCRIPT = STM32F103C8.ld
STUTIL_PORT = 4242
DBG_OPTFLAGS = -O0
REMOVE_OBJS = Y
VERBOSE = N
CROSS_COMPILE = arm-none-eabi-
STFLASH = st-flash
STUTIL = st-util



# **************************************************************************** #
#                            < Makefile guts >                                 #
# **************************************************************************** #
#           DON'T CHANGE ANYTHING UNLESS YOU KNOW WHAT YOU'RE DOING            #
# **************************************************************************** #

SRCS += $(INIT_SRC)
SRCS += $(OTHER_SRCS)

OBJS = $(addsuffix .o,  $(basename $(SRCS)))
SUS  = $(addsuffix .su, $(basename $(OBJS)))

CFLAGS += -mcpu=cortex-m3 -mthumb -mabi=aapcs
CFLAGS += -Wall -Wextra
CFLAGS += -fno-common -static
CFLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections

# LDFLAGS += -march=armv7-m -mabi=aapcs
# LDFLAGS += -nostartfiles -nostdlib -lgcc
# LDFLAGS += -T$(LINKER_SCRIPT)
LDFLAGS += -march=armv7-m -mabi=aapcs
LDFLAGS += -nostartfiles
LDFLAGS += -T$(LINKER_SCRIPT)
LDFLAGS += -lm -lgcc
# LDFLAGS += -Wl,--start-group -lc -lm -lgcc -Wl,--end-group

CC = $(CROSS_COMPILE)gcc
# LD = $(CROSS_COMPILE)ld
LD = $(CROSS_COMPILE)gcc
OBJDUMP = $(CROSS_COMPILE)objdump
OBJCOPY = $(CROSS_COMPILE)objcopy
SIZE = $(CROSS_COMPILE)size
GDB = $(CROSS_COMPILE)gdb

ECHO = echo
RM = rm
MKDIR = mkdir -p

ifeq ($(VERBOSE),Y)
SILENCE =
else
SILENCE = @
endif

GDB_PARAM = -quiet --eval-command="target extended-remote localhost:$(STUTIL_PORT)"

.PHONY: all debug build debug_build output info size flash erase server gdb clean_objs clean

all: clean $(BUILD_DIR) $(SRCS) build clean_objs output size

debug: clean $(BUILD_DIR) $(SRCS) debug_build info output size

$(BUILD_DIR):
	$(SILENCE)$(MKDIR) $(BUILD_DIR)

build: COMFLAGS = $(OPTFLAGS)
build: $(BUILD_DIR)/$(TARGET).elf

debug_build: COMFLAGS = $(DBG_OPTFLAGS) -ggdb3
debug_build: CFLAGS += -fstack-usage
debug_build: LDFLAGS += -Xlinker -Map=$(BUILD_DIR)/$(TARGET).map
debug_build: $(BUILD_DIR)/$(TARGET).elf

# $(BUILD_DIR)/$(TARGET).elf: $(OBJS) | $(BUILD_DIR)
# 	$(SILENCE)$(CC) $(LDFLAGS) $(COMFLAGS) $(OBJS) -o "$@"
$(BUILD_DIR)/$(TARGET).elf: $(OBJS) | $(BUILD_DIR)
	$(SILENCE)$(CC) $(COMFLAGS) $(OBJS) $(LDFLAGS) -o "$@"

.SECONDARY: $(OBJS)

%.o: %.c
	$(SILENCE)$(CC) $(CFLAGS) $(COMFLAGS) -c "$<" -o "$@"

output: $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	$(SILENCE)$(OBJCOPY) -O ihex $< $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(SILENCE)$(OBJCOPY) -O binary $< $@

info: $(BUILD_DIR)/$(TARGET).elf
	$(SILENCE)$(OBJDUMP) -x -S $(BUILD_DIR)/$(TARGET).elf > $(BUILD_DIR)/$(TARGET).lst
	$(SILENCE)$(OBJDUMP) -D $(BUILD_DIR)/$(TARGET).elf > $(BUILD_DIR)/$(TARGET).dis
	$(SILENCE)$(SIZE) $(BUILD_DIR)/$(TARGET).elf > $(BUILD_DIR)/$(TARGET).size

size: $(BUILD_DIR)/$(TARGET).elf
	$(SILENCE)$(SIZE) $(BUILD_DIR)/$(TARGET).elf

flash: $(BUILD_DIR)/$(TARGET).bin
	$(SILENCE)$(STFLASH) --format binary --flash=0x10000 write $(BUILD_DIR)/$(TARGET).bin 0x8000000

erase:
	$(SILENCE)$(STFLASH) erase

server: $(BUILD_DIR)/$(TARGET).elf
	$(SILENCE)$(ECHO) "On another terminal run \"make gdb\""
	$(SILENCE)$(STUTIL) -p $(STUTIL_PORT)

gdb: $(BUILD_DIR)/$(TARGET).elf
	$(SILENCE)$(ECHO) "Run \"load\" to start debugging."
	$(SILENCE)$(GDB) $(GDB_PARAM) $(BUILD_DIR)/$(TARGET).elf

clean_objs:
ifeq ($(REMOVE_OBJS),Y)
	$(SILENCE)$(RM) -f $(OBJS)
endif

clean:
	$(SILENCE)$(RM) -rf $(BUILD_DIR)
	$(SILENCE)$(RM) -f $(OBJS)
	$(SILENCE)$(RM) -f $(SUS)