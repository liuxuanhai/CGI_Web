include ../../makefile.inc

TARGETS=huxiaojiaoshi.user_server

SRCS=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp, %.o, $(SRCS))

all: $(TARGETS) 

huxiaojiaoshi.user_server: $(OBJECTS)
	$(CXX)  $(CXXFLAGS) -o $@  $^  $(BASE_LIBS)
	#strip $@

clean:
	$(RM) *.o $(TARGETS)


