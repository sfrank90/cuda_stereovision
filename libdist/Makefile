#!/usr/bin/make
#

BIN := ../lib/libcudadistribstereo.so

INCLUDE += -I../gsoap/import -I../gsoap -I$(JAVA_HOME)/include

CPPFILES += cuda-jni.cpp 

.PHONY: all gen clean

all: $(BIN)

$(BIN): server-list.h gen
	g++ $(INCLUDE) -shared -fPIC -Wno-vla $(CPPFILES) -o $(BIN)
clean:
	rm -f *.o  $(BIN)
