include ../../makefile.inc

TARGETS=huxiaojiaoshi.feeds_server
INSTALL_PATH=/home/dev/hoosho/server/pa/huxiaojiaoshi/feeds_server
CTL_SHELL=ctl_huxiaojiaoshi_feeds_server.sh

SRCS=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp, %.o, $(SRCS))

PROTOS=$(wildcard *.cc)
PROTO_OBJECTS=$(patsubst %.cc, %.o, $(PROTOS))

all: $(TARGETS) 

huxiaojiaoshi.feeds_server: $(OBJECTS) $(PROTO_OBJECTS)
	$(CXX)  $(CXXFLAGS) -o $@  $^  $(BASE_LIBS)
#strip $@

install:
	sudo mkdir -p $(INSTALL_PATH)
	sudo mkdir -p $(INSTALL_PATH)/log $(INSTALL_PATH)/pid
	sudo sh $(INSTALL_PATH)/$(CTL_SHELL) stop
	sudo cp $(TARGETS) dist/
	sudo cp -R dist/* $(INSTALL_PATH)/
	sudo sh $(INSTALL_PATH)/$(CTL_SHELL) start

clean:
	$(RM) *.o $(TARGETS)


