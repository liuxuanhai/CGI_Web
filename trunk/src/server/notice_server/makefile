include ../../makefile.inc

TARGETS=hoosho.notice_server
INSTALL_PATH=/home/dev/hoosho/server/notice_server
SH_NAME=ctl_hoosho_notice_server.sh

SRCS=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp, %.o, $(SRCS))

PROTOS=$(wildcard *.cc)
PROTO_OBJECTS=$(patsubst %.cc, %.o, $(PROTOS))

all: $(TARGETS)
	
hoosho.notice_server: $(OBJECTS) $(PROTO_OBJECTS)
	$(CXX)  $(CXXFLAGS) -o $@  $^  $(BASE_LIBS)
	#strip $@
	
install:
	sudo mkdir -p $(INSTALL_PATH)/log $(INSTALL_PATH)/pid
	sudo cp -R dist/* $(INSTALL_PATH)
	sudo sh $(INSTALL_PATH)/$(SH_NAME) stop
	sudo cp $(TARGETS) $(INSTALL_PATH)

run:	
	sudo sh $(INSTALL_PATH)/$(SH_NAME) start

clean:
	$(RM) *.o $(TARGETS)


