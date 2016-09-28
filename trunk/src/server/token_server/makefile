include ../../makefile.inc

TARGETS=hoosho.token_server
INSTALL_PATH=/home/dev/hoosho/server/token_server
SH_NAME=ctl_hoosho_token_server.sh

SRCS=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp, %.o, $(SRCS))


all: $(TARGETS) 

hoosho.token_server: $(OBJECTS)
	$(CXX)  $(CXXFLAGS) -o $@  $^  $(BASE_LIBS)
	#strip $@

clean:
	$(RM) *.o $(TARGETS)


