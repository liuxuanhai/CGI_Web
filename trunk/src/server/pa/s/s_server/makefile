include ../../makefile.inc

TARGETS=s.s_server
INSTALL_PATH=/home/dev/hoosho/server/pa/s/s_server
SH_NAME=ctl_s_s_server.sh

SRCS=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp, %.o, $(SRCS))

PROTOS=$(wildcard *.cc)
PROTO_OBJECTS=$(patsubst %.cc, %.o, $(PROTOS))

all: $(TARGETS) 

$(TARGETS): $(OBJECTS) $(PROTO_OBJECTS)
	$(CXX)  $(CXXFLAGS) -o $@  $^  $(BASE_LIBS)
	strip $@
	
install:
	sudo mkdir -p $(INSTALL_PATH)/log $(INSTALL_PATH)/pid
	sudo cp -R dist/* $(INSTALL_PATH)
	@if [ "$(shell ps -ef|grep $(TARGETS)|grep -v grep|awk '{print $$2}')" != "" ]; \
	then \
		sudo sh $(INSTALL_PATH)/$(SH_NAME) stop; \
	fi
	sudo cp $(TARGETS) $(INSTALL_PATH)

run:	
	sudo sh $(INSTALL_PATH)/$(SH_NAME) start

clean:
	$(RM) *.o $(TARGETS)


