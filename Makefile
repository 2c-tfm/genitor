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

PREFIX := /
DESTDIR ?=

SBIN_DIR    := $(DESTDIR)$(PREFIX)sbin
ETC_GENITOR := $(DESTDIR)$(PREFIX)etc/genitor
SV_DIR      := $(ETC_GENITOR)/sv
RUN_DIR     := $(DESTDIR)$(PREFIX)run/genitor/service
SYSLOG_DIR     := $(DESTDIR)$(PREFIX)var/log/syslog

TTY_COUNT = "6"

.PHONY: all clean fclean re install install-init install-services test

all: build $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -Iinc/ $(OBJS) -o $(TARGET)

build:
	mkdir -p $@

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/%.s | build
	$(CC) $(ASFLAGS) -c $< -o $@

clean:
	rm -rf build

fclean: clean
	rm -rf $(TARGET)

re: fclean all

install:  install-init install-services

install-init: $(TARGET)
	install -d -m755 "$(SBIN_DIR)"
	install -m755 "$(TARGET)" "$(SBIN_DIR)/init"

install-services:
	install -d -m755 "$(SV_DIR)" "$(RUN_DIR)" "$(SYSLOG_DIR)"
	@set -e; \
	n=1; \
	while [ "$$n" -le "$(TTY_COUNT)" ]; do \
		svc="$(SV_DIR)/tty$$n"; \
		install -d -m755 "$$svc"; \
		printf '%s\n' \
			'#!/bin/sh' \
			'' \
			'tty=$$(basename "$$(pwd)")' \
			'PATH=$${PATH}:/sbin' \
			'BAUD_RATE=38400' \
			'TERM_NAME=linux' \
			'if [ "$${tty}" = "tty1" ]; then' \
			'	GETTY_ARGS="--noclear"' \
			'fi' \
			'' \
			'[ -r conf ] && . ./conf' \
			'' \
			'if [ -x /sbin/agetty ] || [ -x /bin/agetty ]; then' \
			'    GETTY=agetty' \
			'elif [ -x /sbin/getty ] || [ -x /bin/getty ]; then' \
			'    GETTY=getty' \
			'fi' \
			'' \
			'exec setsid "$${GETTY}" $${GETTY_ARGS} "$${tty}" "$${BAUD_RATE}" "$${TERM_NAME}"' \
			> "$$svc/run"; \
		chmod 755 "$$svc/run"; \
		ln -sfn "/etc/genitor/sv/tty$$n" "$(RUN_DIR)/tty$$n"; \
		n=$$((n + 1)); \
	done

test : $(TARGET)
	mv genitor env/initramfs/

