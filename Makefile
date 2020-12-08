# Makefile for NymphCast-MediaServer (NC-MS)

GCC = g++
MAKEDIR = mkdir -p
RM = rm

# Include the file with the versioning information ('VERSION' variable).
include version
VERSIONINFO = -D__VERSION="\"$(VERSION)\""

OUTPUT := nc_mediaserver
INCLUDE := -Isrc
LIB := -lnymphcast -lnymphrpc -lPocoNet -lPocoUtil -lPocoFoundation -lPocoJSON -lstdc++fs
CFLAGS := $(INCLUDE) -g3 -std=c++17 $(VERSIONINFO)

# Check for MinGW and patch up POCO
# The OS variable is only set on Windows.
ifdef OS
	CFLAGS := $(CFLAGS) -U__STRICT_ANSI__
	LIB += -lws2_32
endif

SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(addprefix obj/,$(notdir) $(SOURCES:.cpp=.o))

all: makedir bin/$(OUTPUT)

makedir:
	$(MAKEDIR) obj/src
	$(MAKEDIR) bin

obj/%.o: %.cpp
	$(GCC) -c -o $@ $< $(CFLAGS)
	
bin/$(OUTPUT): $(OBJECTS)
	$(GCC) -o $@ $(OBJECTS) $(LIB)

clean:
	$(RM) $(OBJECTS)
