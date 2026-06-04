EE_BIN = liero.elf
EE_OBJS = main.o

EE_LIBS = -lgs -ldmakit -lkernel

all: $(EE_BIN)

$(EE_BIN): $(EE_OBJS)
	$(EE_CC) $(EE_CFLAGS) -o $@ $^ $(EE_LIBS)

clean:
	rm -f $(EE_BIN) $(EE_OBJS)
