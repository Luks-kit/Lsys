CC=gcc
AS=gcc                # or use `as` if you prefer
CFLAGS=-Wall -Wextra -g -ffreestanding -fno-stack-protector
INCFLAGS=-Iinc
LDFLAGS=-nostdlib -static
TARGET=build/lsysc
ASM=src/start.s       # your assembly stub file
SRC=$(wildcard src/*.c)
OBJ=$(SRC:.c=.o) $(ASM:.s=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $(TARGET)

src/%.o: src/%.c
	$(CC) -c $< -o $@ $(CFLAGS) $(INCFLAGS)

src/%.o: src/%.s
	$(AS) -c $< -o $@

clean:
	rm -f $(TARGET)
	rm -f src/*.o

run: all
	$(TARGET)

test: TESTERFILE
	$(TARGET) < $<

