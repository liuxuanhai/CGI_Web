include ../../makefile.inc


TARGETS=libcommon_util.a
SRCS=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp, %.o, $(SRCS))

all: $(TARGETS) 
libcommon_util.a: $(OBJECTS)
	ar rc $@ $^

install:
	cp common_util.h ../include/
	cp $(TARGETS) ../lib/

uninstall:
	rm ../include/common_util.h
	rm ../lib/$(TARGETS)

clean:
	$(RM) *.o $(TARGETS)
