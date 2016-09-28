include ../../makefile.inc


TARGETS=libhoosho_proto.a
SRCS=$(wildcard *.pb.cc ./*/*.pb.cc)
OBJECTS=$(patsubst %.pb.cc, %.pb.o, $(SRCS))

all: $(TARGETS) 
libhoosho_proto.a: $(OBJECTS)
	ar rc $@ $^

love:
	protoc -I. --cpp_out=./ *.proto
	
install:
	cp *.pb.h ../include/
	cp $(TARGETS) ../lib/

uninstall:
	rm ../include/*.pb.h
	rm ../lib/$(TARGETS)

clean:
	$(RM) *.o *.pb.h *.pb.cc  $(TARGETS)


