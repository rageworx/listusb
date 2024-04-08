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
    APPVER = ${shell ./makefiles/verstrip.sh}
    APPVER_C = ${shell ./makefiles/verstrip.sh --compat}
    VERFLAGS += -current_version ${APPVER_C} -compatibility_version ${APPVER_C}
else
    CFLAGS += -s
    ifeq ($(KRNL),Linux)
        CFLAGS += -static-libgcc -static-libstdc++
        OPTLIBS += -ludev -lpthread
    else
        # split kernel names, case of MinGW.
        KERNEL_SS := $(shell echo $(KRNL) | cut -d _ -f1 )
        ifeq ($(KERNEL_SS),MINGW64)
            CFLAGS += -mconsole
            WFLAGS += -Ires
            LFLAGS += -static
            WRC = windres
            WROBJ = $(TARGET_OBJ)/resource.o
        endif
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
CFLAGS += -Ires
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

clean:
	@echo "Cleaning built targets ..."
	@rm -rf $(TARGET_DIR)/$(TARGET_PKG)
	@rm -rf $(TARGET_OBJ)/*.o

$(OBJS): $(TARGET_OBJ)/%.o: $(SRC_PATH)/%.cpp
	@echo "Building $@ ... "
	@$(GPP) $(CFLAGS) -c $< -o $@

$(WROBJ): res/resource.rc
	@echo "Building windows resource ..."
	@$(WRC) -i $< $(WFLAGS) -o $@

$(TARGET_DIR)/$(TARGET_PKG): $(OBJS) $(WROBJ)
	@echo "Linking $@ ..."
	@$(GPP) $^ $(CFLAGS) $(LFLAGS) -o $@
	@echo "Stripping $@ ..."
	@strip -S $@
	@echo "done."

install:
	@echo "Install to $(INSTALLDIR) ... "
	@cp -f $(TARGET_DIR)/$(TARGET_PKG) $(INSTALLDIR)

uninstall:
	@echo "Unintalling ..."
	@rm -f $(INSTALLDIR)/$(TARGET_PKG)
