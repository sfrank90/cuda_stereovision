#!/usr/bin/make
#

EXECUTABLE := server

INCLUDE += -I../kernel/ -I../include

CFILES +=  server.cpp  ../kernel/kernel.cu

.PHONY: all gen clean

all: $(EXECUTABLE)


$(EXECUTABLE): gen
	/usr/local/cuda/bin/nvcc -o $(EXECUTABLE) $(INCLUDE) $(CFILES)

clean:
	rm -f *.o *.nsmap *.xml *.gsoap soapH.h soapStub.h soapC.cpp soapServer.cpp soapServerLib.cpp $(EXECUTABLE)
