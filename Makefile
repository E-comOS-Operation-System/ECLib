# Makefile for ECLib
# Builds libeclib.a (static) and optionally libeclib.so (shared)

CC ?= gcc
AR ?= ar
RM ?= rm -rf

PREFIX ?= /usr/local
LIB_INSTALL_DIR ?= $(PREFIX)/lib
INCLUDE_INSTALL_DIR ?= $(PREFIX)/include/eclib

BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
LIB_DIR := lib

CFLAGS ?= -O2 -Wall -Wextra -fPIC -Iinclude
LDFLAGS ?=
LDLIBS ?= -pthread
CFLAGS += -pthread

# set DEBUG=1 to build with -g -O0
ifeq ($(DEBUG),1)
CFLAGS := -g -O0 -Wall -Iinclude
endif

SRCS := $(shell find src -type f -name '*.c' 2>/dev/null)
OBJS := $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SRCS))

TARGET_STATIC := $(LIB_DIR)/libeclib.a
TARGET_SHARED := $(LIB_DIR)/libeclib.so

.PHONY: all static shared clean install uninstall

all: static

static: $(TARGET_STATIC)

$(TARGET_STATIC): $(OBJS) | $(LIB_DIR)
	@echo "Creating static library $@"
	$(AR) rcs $@ $(OBJS)

shared: CFLAGS += -fPIC
shared: $(TARGET_SHARED)

$(TARGET_SHARED): $(OBJS) | $(LIB_DIR)
	@echo "Creating shared library $@"
	$(CC) -shared -o $@ $(OBJS) $(LDLIBS)

# compile rule: create necessary dirs automatically
$(OBJ_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB_DIR):
	@mkdir -p $@

install: all
	@echo "Installing to $(DESTDIR)$(LIB_INSTALL_DIR) and $(DESTDIR)$(INCLUDE_INSTALL_DIR)"
	@mkdir -p $(DESTDIR)$(LIB_INSTALL_DIR)
	@mkdir -p $(DESTDIR)$(INCLUDE_INSTALL_DIR)
	@cp -a $(TARGET_STATIC) $(DESTDIR)$(LIB_INSTALL_DIR)/
	@cp -a include/* $(DESTDIR)$(INCLUDE_INSTALL_DIR)/

uninstall:
	@echo "Removing installed files from $(DESTDIR)$(LIB_INSTALL_DIR) and $(DESTDIR)$(INCLUDE_INSTALL_DIR)"
	@rm -f $(DESTDIR)$(LIB_INSTALL_DIR)/libeclib.a
	@rm -rf $(DESTDIR)$(INCLUDE_INSTALL_DIR)

clean:
	$(RM) $(BUILD_DIR) $(LIB_DIR)