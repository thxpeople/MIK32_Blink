TARGET = blink

BUILD_DIR = build

# Путь до директории c mik32-uploader и загрузчиком
$(MIK32_TOOLCHAIN) := $(MIK32_TOOLCHAIN)

# Путь до директории mik32-uploader https://github.com/MikronMIK32/mik32-uploader
MIK32_UPLOADER_DIR := $(MIK32_TOOLCHAIN)/mik32-uploader

# Hex файл с загрузчиком
MIK32_BOOTLOADER_HEX := $(MIK32_TOOLCHAIN)/elbear_fw_bootloader_qpi_xip_cshigh_0.hex
# MIK32_BOOTLOADER_HEX := $(MIK32_TOOLCHAIN)/elbear_fw_bootloader_qspi_xip_cshigh_0.hex
# MIK32_BOOTLOADER_HEX := $(MIK32_TOOLCHAIN)/mik32-bootloader.hex



# Путь до директории framework-mik32v2-sdk https://github.com/MikronMIK32/framework-mik32v2-sdk
MIK32_FRAMEWORK_DIR = framework-mik32v2-sdk
SHARED_DIR = $(MIK32_FRAMEWORK_DIR)/shared
HAL_DIR    = $(MIK32_FRAMEWORK_DIR)/hal

######################################
# Исходники
######################################

SRC_DIR = src

# Инклуды
INC_DIRS += $(SHARED_DIR)/include
INC_DIRS += $(SHARED_DIR)/periphery
INC_DIRS += $(SHARED_DIR)/runtime
INC_DIRS += $(SHARED_DIR)/libs
INC_DIRS += $(HAL_DIR)/core/Include
INC_DIRS += $(HAL_DIR)/peripherals/Include
INC_DIRS += $(HAL_DIR)/utilities/Include

# Исходники
SRCS  = $(SHARED_DIR)/runtime/crt0.S
SRCS += $(wildcard $(SRC_DIR)/*.c) 
SRCS += $(wildcard $(SRC_DIR)/*.cpp) 
SRCS += $(HAL_DIR)/peripherals/Source/mik32_hal.c 
SRCS += $(HAL_DIR)/peripherals/Source/mik32_hal_pcc.c 
SRCS += $(HAL_DIR)/peripherals/Source/mik32_hal_gpio.c 
SRCS += $(HAL_DIR)/peripherals/Source/mik32_hal_irq.c 
# SRCS += $(HAL_DIR)/peripherals/Source/mik32_hal_adc.c 
# SRCS += $(SHARED_DIR)/libs/xprintf.c 
# SRCS += $(SHARED_DIR)/libs/uart_lib.c 

# Выбор ld скрипта, определяет куда будет загружена прошивка
LDSCRIPT   = spifi.ld
# LDSCRIPT = eeprom.ld
# LDSCRIPT = ram.ld

# Дефайны
DEFINES += MIK32V2

OBJS := $(SRCS)
OBJS := $(OBJS:.c=.o)
OBJS := $(OBJS:.cpp=.o)
OBJS := $(OBJS:.S=.o)
OBJS := $(OBJS:..=miaou)
OBJS := $(addprefix $(BUILD_DIR)/,$(OBJS))

DEPS := $(OBJS:.o=.d)

#######################################
# binaries
#######################################

# Префикс компилятора
PREFIX = riscv-none-elf-

CC      = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
SZ      = $(PREFIX)size


#######################################
# CFLAGS
#######################################

# Оптимизация
OPT = -Og -g3

MCU = -march=rv32imc_zicsr_zifencei -mabi=ilp32 -mcmodel=medlow

INC_FLAGS := $(addprefix -I,$(INC_DIRS))
DEF_FlAGS := $(addprefix -D,$(DEFINES))

CFLAGS := $(MCU) $(DEF_FlAGS) $(OPT) -std=gnu11 -Wall -fsigned-char -fdata-sections -ffunction-sections

# Generate dependency information
CFLAGS += -MMD -MP

ASSFLAGS := -x assembler-with-cpp $(CFLAGS)

LIBS += -Wl,--start-group -lc -Wl,--end-group

LDFLAGS := -T"$(LDSCRIPT)" -nostartfiles -Xlinker -Map="$(BUILD_DIR)/$(TARGET).map" -Wl,--gc-sections -Wl,--print-memory-usage
LDFLAGS += -L$(SHARED_DIR)/ldscripts

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).asm

$(BUILD_DIR)/%.o: %.S Makefile
	mkdir -p $(dir $@)
	$(CC) $(ASSFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.c Makefile
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC_FLAGS) -c $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJS)
	mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $(LIBS) $(OBJS) -o $@  
	$(SZ) $@

%.hex: %.elf
	$(OBJCOPY) -O ihex $^ $@

%.bin: %.elf
	$(OBJCOPY) -O binary $^ $@

%.asm: %.elf
	$(OBJDUMP) -S -d $^ > $@

clean:
	rm -rf $(BUILD_DIR)

upload: $(BUILD_DIR)/$(TARGET).hex
	python $(MIK32_UPLOADER_DIR)/mik32_upload.py --run-openocd --openocd-exec=openocd --openocd-scripts=$(MIK32_UPLOADER_DIR)/openocd-scripts $^

uploadBoot:
	python $(MIK32_UPLOADER_DIR)/mik32_upload.py --run-openocd --openocd-exec=openocd --openocd-scripts=$(MIK32_UPLOADER_DIR)/openocd-scripts $(MIK32_BOOTLOADER_HEX)

-include $(DEPS)
