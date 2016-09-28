include ../../makefile.inc

TARGETS=hoosho.feeds_server
INSTALL_PATH=/home/dev/hoosho/server/feeds_server

SRCS=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp, %.o, $(SRCS))

PROTOS=$(wildcard *.cc)
PROTO_OBJECTS=$(patsubst %.cc, %.o, $(PROTOS))

all: $(TARGETS) 

hoosho.feeds_server: $(OBJECTS) $(PROTO_OBJECTS)
	$(CXX)  $(CXXFLAGS) -o $@  $^  $(BASE_LIBS)
	#strip $@

install:
	sudo mkdir -p $(INSTALL_PATH)/log $(INSTALL_PATH)/pid
	sudo cp $(TARGETS) dist/
	sudo sh /home/dev/hoosho/server/feeds_server/ctl_hoosho_feeds_server.sh stop
	sudo cp -R dist/* $(INSTALL_PATH)/
	sudo sh /home/dev/hoosho/server/feeds_server/ctl_hoosho_feeds_server.sh start

clean:
	$(RM) *.o $(TARGETS)


