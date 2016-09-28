include ../../makefile.inc


TARGETS=libwx_api.a
SRC_H_S=$(wildcard *.h)
SRCS=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp, %.o, $(SRCS))

all: $(TARGETS) 
libwx_api.a: $(OBJECTS)
	ar rc $@ $^

install:
	cp $(SRC_H_S) ../include
	cp $(TARGETS) ../lib/

uninstall:
	rm -f $(foreach x, $(SRC_H_S), ../include/$x)
	rm ../lib/$(TARGETS)

clean:
	$(RM) *.o $(TARGETS)
