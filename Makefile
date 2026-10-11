TARGET := genitor
CC := gcc
CFLAGS := -ggdb -Wall -Wextra -Werror -O3 -flto \
	  -ffast-math -pedantic -fPIE -Iinc/    \
	  -fno-stack-protector -fno-builtin \
	  -nostdlib -static
ASFLAGS := -fPIE 
LDFLAGS := -nostdlib -pie -flto -Wl,-e,_start -static

CSRC := $(wildcard src/*.c)
SSRC := $(wildcard src/*.s)

OBJS := $(patsubst src/%.c,build/%.o,$(CSRC)) \
        $(patsubst src/%.s,build/%.o,$(SSRC))

.PHONY: all clear

all: build $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -Iinc/ $(OBJS) -o $(TARGET)

build:
	mkdir -p $@

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/%.s
	$(CC) $(ASFLAGS) -c $< -o $@

clean:
	rm -rf build $(TARGET)

test : $(TARGET)
	mv genitor env/initramfs/init

