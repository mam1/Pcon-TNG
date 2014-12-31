
CFLAGS = -Os -Wall -Dprintf=__simple_printf -Xlinker -Map=/Users/mam1/Git/Pcon-TNG/Pcon.map
# CFLAGS = -Os -Wall -Xlinker -Map=/Users/mam1/Git/Pcon-TNG/Pcon.map

#CFLAGS = -Os -Wall 


NM = propeller-elf-nm
MODEL = xmmc
BOARD = C3F

# objects for this program
NAME = Pcon
OBJS = Pcon.o char_fsm.o cmd_fsm.o

all: $(NAME).elf

include common.mk
