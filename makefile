# WinAVR Sample makefile written by Eric B. Weddington, J�rg Wunsch, et al.
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
# Customized for bootloader by Jens Mundhenke
#  - avrdude
#  - AVR ISP mkII
#  - fusebits are set while programming, EEPROM remains untouched
#  - make reset added
#
#
# On command line:
#
# make all = Make software.
#
# make reset = resets AVR by checking the configuration bits
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

# MCU name
#MCU = atmega32
#MCU = atmega644
#MCU = atmega644p
MCU = atmega1284p
#MCU = atmega128
#MCU = atmega640
#MCU = atmega1280
#MCU = atmega1281
#MCU = atmega2560
#MCU = atmega2561

#         F_CPU =  1000000
#         F_CPU =  1843200
#         F_CPU =  2000000
#         F_CPU =  3686400
#         F_CPU =  4000000
#         F_CPU =  7372800
#         F_CPU =  8000000
#         F_CPU = 11059200
#         F_CPU = 14745600
         F_CPU = 16000000
#         F_CPU = 18432000
#         F_CPU = 20000000

HEX_FILE_NAME = $(MCU)

BOOTLDRSIZE = 0x2000

ifeq ($(MCU), atmega644)
# 	BOOTRST ist nicht gesetzt, nach RESET wird Anwendung gestartet
#	FUSE_BITS = -u -U lfuse:w:0xff:m -U hfuse:w:0xd1:m
# 	BOOTRST ist  gesetzt, nach RESET wird Bootloader gestartet
	FUSE_BITS = -u -U lfuse:w:0xff:m -U hfuse:w:0xd0:m
	BOOTLOADERSTARTADR = 0xE000
endif

ifeq ($(MCU), atmega644p)
# 	BOOTRST ist nicht gesetzt, nach RESET wird Anwendung gestartet
#	FUSE_BITS = -u -U lfuse:w:0xff:m -U hfuse:w:0xd1:m
# 	BOOTRST ist  gesetzt, nach RESET wird Bootloader gestartet
	FUSE_BITS = -u -U lfuse:w:0xff:m -U hfuse:w:0xd0:m
	BOOTLOADERSTARTADR = 0xE000
endif

ifeq ($(MCU), atmega1284p)
# 	BOOTRST ist nicht gesetzt, nach RESET wird Anwendung gestartet
#	FUSE_BITS = -u -U lfuse:w:0xff:m -U hfuse:w:0xd1:m
# 	BOOTRST ist  gesetzt, nach RESET wird Bootloader gestartet
	FUSE_BITS = -u -U lfuse:w:0xff:m -U hfuse:w:0xd0:m
	BOOTLOADERSTARTADR = 0x1E000
endif

#Fuse settings for ATmega128
ifeq ($(MCU), atmega128)
	BOOTLOADERSTARTADR = 0x1E000
endif

ifeq ($(MCU), atmega640)
	FUSE_BITS = -u -U lfuse:w:0xff:m -U hfuse:w:0xd0:m -U efuse:w:0xfc:m
	BOOTLOADERSTARTADR = 0xE000
endif

ifeq ($(MCU), atmega1280)
	FUSE_BITS = -u -U lfuse:w:0xff:m -U hfuse:w:0xd0:m -U efuse:w:0xfc:m
	BOOTLOADERSTARTADR = 0x1E000
endif

ifeq ($(MCU), atmega1281)
	FUSE_BITS = -u -U lfuse:w:0xff:m -U hfuse:w:0xd0:m -U efuse:w:0xfc:m
	BOOTLOADERSTARTADR = 0x1E000
endif

ifeq ($(MCU), atmega2560)
	FUSE_BITS = -u -U lfuse:w:0xff:m -U hfuse:w:0xd0:m -U efuse:w:0xfc:m
	BOOTLOADERSTARTADR = 0x3E000
endif

ifeq ($(MCU), atmega2561)
	FUSE_BITS = -u -U lfuse:w:0xff:m -U hfuse:w:0xd0:m -U efuse:w:0xfc:m
	BOOTLOADERSTARTADR = 0x3E000
endif

# Output format. (can be srec, ihex, binary)
FORMAT = ihex

# Target file name (without extension).
TARGET = Hexfiles/bootloader_$(HEX_FILE_NAME)

# Optimization level, can be [0, 1, 2, 3, s]. 0 turns off optimization.
# (Note: 3 is not always the best optimization level. See avr-libc FAQ.)
OPT = s

# If there is more than one source file, append them above, or modify and
# uncomment the following:
SRC =  main.c stack.c timer.c para.c console.c

SRC += syslog.c

SRC += usart.c

SRC += networkcard/enc28j60.c networkcard/rtl8019.c

SRC += bootloader/tftp.c bootloader/ihex.c

# List Assembler source files here.
# Make them always end in a capital .S.  Files ending in a lowercase .s
# will not be considered source files but generated files (assembler
# output from the compiler), and will be deleted upon "make clean"!
# Even though the DOS/Win* filesystem matches both .s and .S the same,
# it will preserve the spelling of the filenames, and gcc itself does
# care about how the name is spelled on its command-line.
ASRC =


# List any extra directories to look for include files here.
#     Each directory must be seperated by a space.
EXTRAINCDIRS =


# Optional compiler flags.
#  -g:        generate debugging information (for GDB, or for COFF conversion)
#  -O*:       optimization level
#  -f...:     tuning, see gcc manual and avr-libc documentation
#  -Wall...:  warning level
#  -Wa,...:   tell GCC to pass this to the assembler.
#    -ahlms:  create assembler listing
CFLAGS = -g -O$(OPT) -DF_CPU=$(F_CPU)UL -DBOOTLOADERSTARTADR=$(BOOTLOADERSTARTADR)UL -ffunction-sections\
-funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums \
-Wall -Wstrict-prototypes \
-Wa,-adhlns=$(<:.c=.lst) \
$(patsubst %,-I%,$(EXTRAINCDIRS))


# Set a "language standard" compiler flag.
#   Unremark just one line below to set the language standard to use.
#   gnu99 = C99 + GNU extensions. See GCC manual for more information.
#CFLAGS += -std=c89
#CFLAGS += -std=gnu89
#CFLAGS += -std=c99
CFLAGS += -std=gnu99



# Optional assembler flags.
#  -Wa,...:   tell GCC to pass this to the assembler.
#  -ahlms:    create listing
#  -gstabs:   have the assembler create line number information; note that
#             for use in COFF files, additional information about filenames
#             and function names needs to be present in the assembler source
#             files -- see avr-libc docs [FIXME: not yet described there]
ASFLAGS = -Wa,-adhlns=$(<:.S=.lst),-gstabs



# Optional linker flags.
#  -Wl,...:   tell GCC to pass this to linker.
#  -Map:      create map file
#  --cref:    add cross reference to  map file
LDFLAGS = -Wl,-Map=$(TARGET).map,--cref,--gc-sections,--relax



# Additional libraries

# Minimalistic printf version
#LDFLAGS += -Wl,-u,vfprintf -lprintf_min

# Floating point printf version (requires -lm below)
#LDFLAGS += -Wl,-u,vfprintf -lprintf_flt

# -lm = math library
#LDFLAGS += -lm

#LDFLAGS += -Ttext=E000
LDFLAGS	+= -Wl,--section-start=.text=$(BOOTLOADERSTARTADR)


# Programming support using avrdude. Settings and variables.

# Programming hardware: alf avr910 avrisp bascom bsd
# dt006 pavr picoweb pony-stk200 sp12 stk200 stk500
#
# Type: avrdude -c ?
# to get a full listing.
#

#AVRDUDE_PROGRAMMER = AVR910
#AVRDUDE_PROGRAMMER = stk200
#AVRDUDE_PROGRAMMER = USBasp
AVRDUDE_PROGRAMMER = avrispmkII

AVRDUDE_PORT = usb        # programmer connected to USB port
#AVRDUDE_PORT = com1	   # programmer connected to serial device
#AVRDUDE_PORT = lpt1	       # programmer connected to parallel port

#
#
#AVRDUDE_WRITE_FLASH = -U flash:w:$(TARGET).hex
AVRDUDE_WRITE_FLASH = -U flash:w:$(TARGET).hex $(FUSE_BITS)
#AVRDUDE_WRITE_EEPROM = -U eeprom:w:$(TARGET).eep

#AVRDUDE_FLAGS = -p $(MCU) -P $(AVRDUDE_PORT) -b 115200 -c $(AVRDUDE_PROGRAMMER)
AVRDUDE_FLAGS = -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER)
#AVRDUDE_FLAGS = -p $(MCU) -c $(AVRDUDE_PROGRAMMER)

# Uncomment the following if you want avrdude's erase cycle counter.
# Note that this counter needs to be initialized first using -Yn,
# see avrdude manual.
#AVRDUDE_ERASE += -y

# Uncomment the following if you do /not/ wish a verification to be
# performed after programming the device.
#AVRDUDE_FLAGS += -V

# Increase verbosity level.  Please use this when submitting bug
# reports about avrdude. See <http://savannah.nongnu.org/projects/avrdude>
# to submit bug reports.
#AVRDUDE_FLAGS += -v -v




# ---------------------------------------------------------------------------

# Define directories, if needed.
DIRAVR = c:/programme/winavr-20080610
DIRAVRBIN = $(DIRAVR)/bin
DIRAVRUTILS = $(DIRAVR)/utils/bin
DIRINC = .
DIRLIB = $(DIRAVR)/avr/lib


# Define programs and commands.
SHELL = sh

CC = avr-gcc

OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size


# Programming support using avrdude.
#AVRDUDE = "c:/programme/atmel/avr tools/avrdude/avrdude"
AVRDUDE = $(DIRAVRBIN)/avrdude

REMOVE = rm -f
COPY = cp

HEXSIZE = $(SIZE) --target=$(FORMAT) $(TARGET).hex
ELFSIZE = $(SIZE) -A $(TARGET).elf



# Define Messages
# English
MSG_ERRORS_NONE = Errors: none
MSG_BEGIN = -------- begin --------
MSG_END = --------  end  --------
MSG_SIZE_BEFORE = Size before:
MSG_SIZE_AFTER = Size after:
MSG_COFF = Converting to AVR COFF:
MSG_EXTENDED_COFF = Converting to AVR Extended COFF:
MSG_FLASH = Creating load file for Flash:
MSG_EEPROM = Creating load file for EEPROM:
MSG_EXTENDED_LISTING = Creating Extended Listing:
MSG_SYMBOL_TABLE = Creating Symbol Table:
MSG_LINKING = Linking:
MSG_COMPILING = Compiling:
MSG_ASSEMBLING = Assembling:
MSG_CLEANING = Cleaning project:




# Define all object files.
OBJ = $(SRC:.c=.o) $(ASRC:.S=.o)

# Define all listing files.
LST = $(ASRC:.S=.lst) $(SRC:.c=.lst)

# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS = -mmcu=$(MCU) -I. $(CFLAGS)
ALL_ASFLAGS = -mmcu=$(MCU) -I. -x assembler-with-cpp $(ASFLAGS)



# Default target.
all: begin gccversion sizebefore $(TARGET).elf $(TARGET).hex $(TARGET).eep \
	$(TARGET).lss $(TARGET).sym $(TARGET).bin sizeafter finished end


# Eye candy.
# AVR Studio 3.x does not check make's exit code but relies on
# the following magic strings to be generated by the compile job.
begin:
	@echo
	@echo $(MSG_BEGIN)

finished:
	@echo $(MSG_ERRORS_NONE)

end:
	@echo $(MSG_END)
	@echo


# Display size of file.
sizebefore:
	@if [ -f $(TARGET).elf ]; then echo; echo $(MSG_SIZE_BEFORE); $(ELFSIZE); echo; fi

sizeafter:
	@if [ -f $(TARGET).elf ]; then echo; echo $(MSG_SIZE_AFTER); $(ELFSIZE); echo; fi



# Display compiler version information.
gccversion :
	@$(CC) --version




# Convert ELF to COFF for use in debugging / simulating in
# AVR Studio or VMLAB.
COFFCONVERT=$(OBJCOPY) --debugging \
	--change-section-address .data-0x800000 \
	--change-section-address .bss-0x800000 \
	--change-section-address .noinit-0x800000 \
	--change-section-address .eeprom-0x810000


coff: $(TARGET).elf
	@echo
	@echo $(MSG_COFF) $(TARGET).cof
	$(COFFCONVERT) -O coff-avr $< $(TARGET).cof


extcoff: $(TARGET).elf
	@echo
	@echo $(MSG_EXTENDED_COFF) $(TARGET).cof
	$(COFFCONVERT) -O coff-ext-avr $< $(TARGET).cof




# Program the device.
program: $(TARGET).hex $(TARGET).eep
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH) $(AVRDUDE_WRITE_EEPROM)


# Reset the device (by reading signature).
reset:
	$(AVRDUDE) $(AVRDUDE_FLAGS)



# Create final output files (.hex, .eep) from ELF output file.
%.hex: %.elf
	@echo
	@echo $(MSG_FLASH) $@
	$(OBJCOPY) -O $(FORMAT) -R .eeprom $< $@

%.eep: %.elf
	@echo
	@echo $(MSG_EEPROM) $@
	-$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" \
	--change-section-lma .eeprom=0 -O $(FORMAT) $< $@

# Create extended listing file from ELF output file.
%.lss: %.elf
	@echo
	@echo $(MSG_EXTENDED_LISTING) $@
	$(OBJDUMP) -h -S $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo
	@echo $(MSG_SYMBOL_TABLE) $@
	avr-nm -n $< > $@

%bin: %elf
	@echo
	$(OBJCOPY) -O binary $< $@

	@echo
	avr-size -B -d $(TARGET).elf
	@echo
	avr-size -B -d $(OBJ)
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}.elf

# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(OBJ)
%.elf: $(OBJ)
	@echo
	@echo $(MSG_LINKING) $@
	$(CC) $(ALL_CFLAGS) $(OBJ) --output $@ $(LDFLAGS)


# Compile: create object files from C source files.
%.o : %.c
	@echo
	@echo $(MSG_COMPILING) $<
	$(CC) -c $(ALL_CFLAGS) $< -o $@


# Compile: create assembler files from C source files.
%.s : %.c
	$(CC) -S $(ALL_CFLAGS) $< -o $@


# Assemble: create object files from assembler source files.
%.o : %.S
	@echo
	@echo $(MSG_ASSEMBLING) $<
	$(CC) -c $(ALL_ASFLAGS) $< -o $@


# Target: clean project.
clean: begin clean_list finished end

clean_list :
	@echo
	@echo $(MSG_CLEANING)
	$(REMOVE) $(TARGET).hex
	$(REMOVE) $(TARGET).eep
	$(REMOVE) $(TARGET).obj
	$(REMOVE) $(TARGET).cof
	$(REMOVE) $(TARGET).elf
	$(REMOVE) $(TARGET).map
	$(REMOVE) $(TARGET).obj
	$(REMOVE) $(TARGET).a90
	$(REMOVE) $(TARGET).sym
	$(REMOVE) $(TARGET).lnk
	$(REMOVE) $(TARGET).lss
	$(REMOVE) $(OBJ)
	$(REMOVE) $(LST)
	$(REMOVE) $(SRC:.c=.s)
	$(REMOVE) $(SRC:.c=.d)


# Automatically generate C source code dependencies.
# (Code originally taken from the GNU make user manual and modified
# (See README.txt Credits).)
#
# Note that this will work with sh (bash) and sed that is shipped with WinAVR
# (see the SHELL variable defined above).
# This may not work with other shells or other seds.
#
#%.d: %.c
#	set -e; $(CC) -MM $(ALL_CFLAGS) $< \
#	| sed 's,\(.*\)\.o[ :]*,\1.o \1.d : ,g' > $@; \
#	[ -s $@ ] || rm -f $@


# Remove the '-' if you want to see the dependency files generated.
-include $(SRC:.c=.d)



# Listing of phony targets.
.PHONY : all begin finish end sizebefore sizeafter gccversion coff extcoff \
	clean clean_list program

