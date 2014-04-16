 
CFLAGS += -fPIC -Wall -Werror -std=gnu99 -g -O3 -Ibuild -I.
LDFLAGS_EXTRA = -shared

ARCH = $(shell uname -m | sed 's/^i.86$$/x86/')

BUILD_DIR ?= build
HEADER = $(BUILD_DIR)/gnu_io_SerialDriver.h
CLASSE = bin/gnu/io/serial/SerialDriver.class
OBJECTS = $(BUILD_DIR)/gnuioserial.o
OUT_DIR ?= bin/lib
LIB_DIR ?= $(OUT_DIR)/$(ARCH)
LIB = $(LIB_DIR)/libgnuioserial.so
STRIP ?= strip

default: all

clean :
	-rm -f *.o $(LIB) $(HEADER)

all: $(LIB_DIR) $(BUILD_DIR) $(LIB)
	cd bin && zip -r ../bin/gnuioserial.jar gnu

$(HEADER): bin/gnu/io/serial/SerialDriver.class
	javah -jni -classpath bin/ -d $(BUILD_DIR) gnu.io.serial.SerialDriver

$(CLASSE): src/gnu/io/serial/SerialDriver.java
	javac -s src -d bin $^

$(LIB_DIR) :
	mkdir -p $@

$(BUILD_DIR) :
	mkdir -p $@

$(OBJECTS) :$(BUILD_DIR)/%.o: src/%.c $(HEADER)
	$(CC) $(CFLAGS) -c -o $@ $<

$(LIB): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDFLAGS_EXTRA)
	$(STRIP) $@
