######################################
# target
######################################
TARGET = main

######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Os

#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES =  \
			 $(shell find ./ -name '*.c')
#src/main.c \
#src/system_ch32v00x.c \
#library/Debug/debug.c \
#library/Peripheral/src/ch32v00x_wwdg.c

## ASM sources
ASM_SOURCES =  \
startup_ch32v00x.s

##########################################################################################################################
# OPENOCD PATH  !!!full path needed
##########################################################################################################################
	OCD = /home/carver/MRS_Toolchain_Linux_x64/OpenOCD/OpenOCD/bin/rvopenocd

#######################################
# binaries
#######################################
PREFIX = riscv-none-embed-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
	CC = $(PREFIX)gcc
	AS = $(PREFIX)gcc -x assembler-with-cpp
	CP = $(PREFIX)objcopy
	SZ = $(PREFIX)size
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -march=rv32imac

# fpu
# NONE for Cortex-M0/M0+/M3

# float-abi

# mcu
MCU = $(CPU) -mabi=ilp32 -msmall-data-limit=0 -msave-restore -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized

#riscv-none-embed-gcc   -c -o "$@" "$<"
# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  

# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-I. \
-Isrc \
-Ilibrary/Core \
-Ilibrary/Debug \
-Ilibrary/Peripheral/inc

# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT)

CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT)

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF "$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = Link.ld

# libraries
#LIBS = -lc -lm -lnosys 
LIBS = 
LIBDIR = 
LDFLAGS = $(MCU) -g --specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -nostartfiles -Xlinker --gc-sections -Wl,-Map=$(BUILD_DIR)/$(TARGET).map --specs=nosys.specs $(OPT)

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -g -std=gnu11 -MT "$(@)" -c "$<" -o "$@"

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) -g -MT "$(@)" -c "$<" -o "$@"

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)

#######################################
# flash
#######################################
flash: $(BUILD_DIR)/$(TARGET).hex
	sudo chmod -R 666 /dev/ttyACM0
	sudo $(OCD) -f ./wch-riscv.cfg  -c init -c halt  -c "program $(BUILD_DIR)/$(TARGET).hex" -c "verify_image $(BUILD_DIR)/$(TARGET).hex" -c wlink_reset_resume -c exit

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

