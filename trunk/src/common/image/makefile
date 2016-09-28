include ../../makefile.inc


TARGETS=libimage_helper.a
SRCS=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp, %.o, $(SRCS))

all: $(TARGETS) 
libimage_helper.a: $(OBJECTS)
	ar rc $@ $^

install:
	cp image_helper.h ../include/
	cp $(TARGETS) ../lib/

uninstall:
	rm ../include/image_helper.h
	rm ../lib/$(TARGETS)

clean:
	$(RM) *.o $(TARGETS)
