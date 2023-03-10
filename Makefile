# listusb for macOS
# ----------------------------------------------------------------------
# Written by Raph.K.
#

GCC      = gcc
GPP      = g++
AR       = ar
WRC      =
LIBARCH  = 
OPTARCH  =
BENDFIX  = unknown
OPTLIBS  =
PUBADDF  =
UNSUPPORTED = 0

# Automatic detecting architecture.
KRNL := $(shell uname -s)
KVER := $(shell uname -r | cut -d . -f1)
ARCH := $(shell uname -m)

ifeq ($(KRNL),Darwin)
    GCC = llvm-gcc
    GPP = llvm-g++
    ifeq ($(shell test $(KVER) -gt 19; echo $$?),0)
        OPTARCH += -arch x86_64 -arch arm64
        OPTARCH += -mmacosx-version-min=11.0
        LIBARCH = macos11
        BENDFIX = macos_uni
    else
        LIBARCH = macos10
        BENDFIX = macos_x8664
    endif
    PUBADDF = macpub
    # --- prevent to pthreadd stack error by xcode ---
	# OPTARCH += -fno-stack-check
    # --- Apple frameworks ---
    OPTLIBS += -framework Cocoa -framework Foundation -framework IOKit
    OPTLIBS += -framework CoreFoundation
    OPTLIBS += -framework Security
else
    ifeq ($(KRNL),Linux)
        OPTLIBS += -ludev -lpthread
    else
	# currently no plan to support other OS.
    endif
endif

# Base PATH
BASE_PATH = .
SRC_PATH  = $(BASE_PATH)/src
INSTALLDIR = /usr/local/bin

# TARGET settings
#TARGET_PKG = listusb-$(BENDFIX)
TARGET_PKG = listusb
TARGET_DIR = ./bin
TARGET_OBJ = ./obj
TARGET_ASM = ./asm

# .config required
include .config

# libusb config
LIBUSB_INC = $(LIBUSBDIR)/libusb
LIBUSB_LIB = $(LIBUSB_INC)/.libs

# Compiler optiops 
COPTS += -std=c++11
COPTS += -fomit-frame-pointer -O2
#COPTS += -g3 -DDEBUG
COPTS += $(OPTARCH)

# CC FLAGS
CFLAGS += -I$(LIBUSB_INC)
CFLAGS += -I$(SRC_PATH)
CFLAGS += $(DEFS)
CFLAGS += $(COPTS)

# LINK FLAG
LFLAGS += $(LIBUSB_LIB)/libusb-1.0.a
LFLAGS += $(OPTLIBS)

# Sources
SRCS = $(wildcard $(SRC_PATH)/*.cpp)

# Make object targets from SRCS.
OBJS = $(SRCS:$(SRC_PATH)/%.cpp=$(TARGET_OBJ)/%.o)

.PHONY: prepare clean

all: prepare continue
cleanall: clean

continue: $(TARGET_DIR)/$(TARGET_PKG)

prepare:
	@mkdir -p $(TARGET_DIR)
	@mkdir -p $(TARGET_OBJ)
	@mkdir -p $(TARGET_ASM)

clean:
	@echo "Cleaning built targets ..."
	@rm -rf $(TARGET_DIR)/$(TARGET_PKG)
	@rm -rf $(TARGET_OBJ)/*.o
	@rm -rf asm/*.s

$(OBJS): $(TARGET_OBJ)/%.o: $(SRC_PATH)/%.cpp
	@echo "Building $@ ... "
	@$(GPP) $(CFLAGS) -c $< -o $@

$(TARGET_DIR)/$(TARGET_PKG): $(OBJS)
	@echo "Generating $@ ..."
	@$(GPP) $^ $(CFLAGS) $(LFLAGS)  -o $@
	@echo "done."

install:
	@echo "Install to $(INSTALLDIR) ... "
	@cp -f $(TARGET_DIR)/$(TARGET_PKG) $(INSTALLDIR)

uninstall:
	@echo "Unintalling ..."
	@rm -f $(INSTALLDIR)/$(TARGET_PKG)

