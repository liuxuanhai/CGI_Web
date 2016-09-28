include ../../makefile.inc


TARGETS=libcoroutine.a
SRC_H_S=$(wildcard *.h)
SRCS=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp, %.o, $(SRCS))

all: $(TARGETS) 
libcoroutine.a: $(OBJECTS)
	ar rc $@ $^

install:
	cp $(SRC_H_S) ../include
	cp $(TARGETS) ../lib/

uninstall:
#	echo $(foreach x, $(SRC_H_S), ../include/$x)
	rm -f $(foreach x, $(SRC_H_S), ../include/$x)
	rm -f ../lib/$(TARGETS)

clean:
	$(RM) *.o $(TARGETS)
