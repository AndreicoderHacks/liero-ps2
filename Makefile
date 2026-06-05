EE_BIN = liero.elf

SRCS = main.c menu.c sfx.c \
       world.c \
       player.c \
       weapon.c \
       projectile.c \
       render.c \
       input.c \
       particles.c

OBJS = $(SRCS:.c=.o)

PS2DEV ?= /usr/local/ps2dev
PS2SDK ?= $(PS2DEV)/ps2sdk
GSKIT  ?= $(PS2DEV)/gsKit

CC     = mips64r5900el-ps2-elf-gcc
CFLAGS = -O2 -G0 -D_EE \
         -I$(PS2SDK)/ee/include \
         -I$(PS2SDK)/common/include \
         -I$(GSKIT)/include \
         -I.

LDFLAGS = -T$(PS2SDK)/ee/startup/linkfile \
          -L$(PS2SDK)/ee/lib \
          -L$(GSKIT)/lib

LIBS = -lgskit -ldmakit -lpad -laudsrv -lcglue -lkernel -lc -lm -lgcc

all: $(EE_BIN)

$(EE_BIN): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(EE_BIN)
