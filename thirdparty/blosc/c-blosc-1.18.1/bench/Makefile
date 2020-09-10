CC = gcc  # clang++, g++ or just gcc if not compiling Snappy (C++ code)
CFLAGS = -O3 -g -msse2 -Wall
LDFLAGS = -lpthread  # for UNIX or for Windows with pthread installed
#LDFLAGS = -static  # for mingw
SOURCES = $(wildcard ../blosc/*.c)
EXECUTABLE = bench

# Support for internal LZ4 and LZ4HC
LZ4_DIR = ../internal-complibs/lz4-1.7.0
CFLAGS += -DHAVE_LZ4 -I$(LZ4_DIR)
SOURCES += $(wildcard $(LZ4_DIR)/*.c)

# Support for external LZ4 and LZ4HC
#LDFLAGS += -DHAVE_LZ4 -llz4

# Support for internal Snappy
#SNAPPY_DIR = ../internal-complibs/snappy-1.1.1
#CFLAGS += -DHAVE_SNAPPY -I$(SNAPPY_DIR)
#SOURCES += $(wildcard $(SNAPPY_DIR)/*.cc)

# Support for external Snappy
LDFLAGS += -DHAVE_SNAPPY -lsnappy

# Support for external Zlib
LDFLAGS += -DHAVE_ZLIB -lz

# Support for internal Zlib
#ZLIB_DIR = ../internal-complibs/zlib-1.2.8
#CFLAGS += -DHAVE_ZLIB -I$(ZLIB_DIR)
#SOURCES += $(wildcard $(ZLIB_DIR)/*.c)

SOURCES += bench.c

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LDFLAGS)

clean:
	rm -rf $(EXECUTABLE)
