include ../../makefile.inc


TARGETS=libcommon_protoio.a
SRCS=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp, %.o, $(SRCS))

all: $(TARGETS) 
libcommon_protoio.a: $(OBJECTS)
	ar rc $@ $^

install:
	cp proto_io_tcp_client.h ../include/
	cp $(TARGETS) ../lib/

uninstall:
	rm ../include/proto_io_tcp_client.h
	rm ../lib/$(TARGETS)

clean:
	$(RM) *.o $(TARGETS)