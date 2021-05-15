# Makefile for NymphCast-MediaServer (NC-MS)


# Set platform-specific parameters, either from an external file, or native platform defaults.
ifdef PLATFORM
include platforms/$(PLATFORM).mk
TARGET := $(PLATFORM)
else
# Get the compiler's (GCC or Clang) target triplet and use that as platform.
TARGET := $(shell g++ -dumpmachine)
$(info TARGET: $(TARGET))
endif

ifdef TOOLCHAIN
#include Makefile.$(TARGET)
include toolchain/$(TOOLCHAIN).mk
else
GPP = g++
GCC = gcc
STRIP = strip
MAKEDIR = mkdir -p
RM = rm
endif

TARGET_BIN := $(TARGET)/


# Include the file with the versioning information ('VERSION' variable).
include version
VERSIONINFO = -D__VERSION="\"$(VERSION)\""

OUTPUT := nymphcast_mediaserver
INCLUDE := -Isrc
LIB := -lnymphcast -lnymphrpc -lPocoNet -lPocoUtil -lPocoFoundation -lPocoJSON -lstdc++fs
CFLAGS := $(INCLUDE) -g3 -std=c++17 $(VERSIONINFO)

# Check for MinGW and patch up POCO
# The OS variable is only set on Windows.
ifdef OS
	CFLAGS := $(CFLAGS) -U__STRICT_ANSI__
	LIB += -lws2_32
	OUTPUT := $(OUTPUT).exe
else
	LIB += -pthread
endif

SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(addprefix obj/$(TARGET_BIN),$(notdir) $(SOURCES:.cpp=.o))

all: makedir bin/$(TARGET_BIN)$(OUTPUT)

makedir:
	$(MAKEDIR) obj/$(TARGET_BIN)src
	$(MAKEDIR) bin/$(TARGET)

obj/$(TARGET_BIN)%.o: %.cpp
	$(GPP) -c -o $@ $< $(CFLAGS)
	
bin/$(TARGET_BIN)$(OUTPUT): $(OBJECTS)
	$(GPP) -o $@ $(OBJECTS) $(LIB)
	cp $@ $@.debug
	$(STRIP) -S --strip-unneeded $@
	
PREFIX ?= /usr/local

ifeq ($(PREFIX),/usr/local)
	CONFDIR := $(PREFIX)/etc
else
	CONFDIR := /etc
endif
	
install:
	install -d $(DESTDIR)$(PREFIX)/bin/ \
			-d $(DESTDIR)$(CONFDIR)/nymphcast/
	install -m 755 bin/$(TARGET_BIN)$(OUTPUT) $(DESTDIR)$(PREFIX)/bin/
	install -m 644 *.ini $(DESTDIR)$(CONFDIR)/nymphcast/

clean:
	$(RM) $(OBJECTS)
