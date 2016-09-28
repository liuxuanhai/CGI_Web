CXX 		= g++ 
EXTRAFLAGS	= -g -Wall -DLOGGER -D__STDC_CONSTANT_MACROS
INCLUDES	= -I/usr/local/ffmpeg/include -I/usr/local/include
CXXFLAGS	= $(INCLUDES) $(EXTRAFLAGS)

#rule definations
.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

TARGETS=libffmpeg_api.a
SRC_H_S=$(wildcard *.h)
SRCS=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp, %.o, $(SRCS))

all: $(TARGETS)
$(TARGETS): $(OBJECTS)
	ar rc $@ $^


install:
	mkdir -p ../include ../lib
	cp *.h ../include
	cp $(TARGETS) ../lib

uninstall:
	rm -f $(foreach x, $(SRC_H_S), ../include/$x)
	rm ../lib/$(TARGETS)

clean:
	rm -rf *.o $(TARGETS)

