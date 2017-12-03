
PORT ?= /dev/ttyUSB0
BAUD ?= 115200

# all:
# 	$(ECHO) "building ....."

# all: $(BUILD)/libaxtls.a $(FWBIN)

# CONFVARS_FILE = $(BUILD)/confvars

# ifeq ($(wildcard $(CONFVARS_FILE)),)
# $(shell $(MKDIR) -p $(BUILD))
# $(shell echo $(FROZEN_DIR) $(UART_OS) > $(CONFVARS_FILE))
# else ifneq ($(shell cat $(CONFVARS_FILE)), $(FROZEN_DIR) $(UART_OS))
# $(shell echo $(FROZEN_DIR) $(UART_OS) > $(CONFVARS_FILE))
# endif

# $(BUILD)/uart.o: $(CONFVARS_FILE)

# FROZEN_EXTRA_DEPS = $(CONFVARS_FILE)

# .PHONY: deploy

# deploy: $(BUILD)/firmware-combined.bin
# 	$(ECHO) "Writing $< to the board"
# 	$(Q)esptool.py --port $(PORT) --baud $(BAUD) write_flash --verify --flash_size=$(FLASH_SIZE) --flash_mode=$(FLASH_MODE) 0 $<

erase:
	# $(ECHO) "eraseing flash"
	esptool.py --port $(PORT) --baud $(BAUD) erase_flash

clean:
	rm -f  t1-0x10000.bin	

# reset:
# 	echo -e "\r\nimport machine; machine.reset()\r\n" >$(PORT)

# $(FWBIN): $(BUILD)/firmware.elf
# 	$(ECHO) "Create $@"
# 	$(Q)esptool.py elf2image $^
# 	$(Q)$(PYTHON) makeimg.py $(BUILD)/firmware.elf-0x00000.bin $(BUILD)/firmware.elf-0x[0-5][1-f]000.bin $@

# $(BUILD)/firmware.elf: $(OBJ)
# 	$(ECHO) "LINK $@"
# 	$(Q)$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
# 	$(Q)$(SIZE) $@

# 512k:
# 	$(MAKE) LDSCRIPT=esp8266_512k.ld CFLAGS_EXTRA='-DMP_CONFIGFILE="<mpconfigport_512k.h>"' MICROPY_FATFS=0 MICROPY_PY_BTREE=0

# ota:
# 	rm -f $(BUILD)/firmware.elf $(BUILD)/firmware.elf*.bin
# 	$(MAKE) LDSCRIPT=esp8266_ota.ld FWBIN=$(BUILD)/firmware-ota.bin

#MAKE_PINS = boards/make-pins.py
#BOARD_PINS = boards/$(BOARD)/pins.csv
#AF_FILE = boards/stm32f4xx_af.csv
#PREFIX_FILE = boards/stm32f4xx_prefix.c
#GEN_PINS_SRC = $(BUILD)/pins_$(BOARD).c
#GEN_PINS_HDR = $(HEADER_BUILD)/pins.h
#GEN_PINS_QSTR = $(BUILD)/pins_qstr.h
#GEN_PINS_AF_CONST = $(HEADER_BUILD)/pins_af_const.h
#GEN_PINS_AF_PY = $(BUILD)/pins_af.py

# Making OBJ use an order-only depenedency on the generated pins.h file
# has the side effect of making the pins.h file before we actually compile
# any of the objects. The normal dependency generation will deal with the
# case when pins.h is modified. But when it doesn't exist, we don't know
# which source files might need it.
#$(OBJ): | $(HEADER_BUILD)/pins.h

# Use a pattern rule here so that make will only call make-pins.py once to make
