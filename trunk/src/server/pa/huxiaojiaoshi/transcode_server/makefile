include ../../makefile.inc

TARGETS=huxiaojiaoshi.transcode_server
INSTALL_PATH=/home/dev/hoosho/server/pa/huxiaojiaoshi/transcode_server
SH_NAME=ctl_huxiaojiaoshi_transcode_server.sh

DIR_FFMPEG     	= /usr/local/ffmpeg
LIB_FFMPEG     	= -L$(DIR_FFMPEG)/lib -lavformat -lavcodec -lavfilter -lavutil -lswresample
INC_FFMPEG     	= -I$(DIR_FFMPEG)/include

CXXFLAGS 	+= $(INC_FFMPEG)
BASE_LIBS 	+= $(LIB_FFMPEG) -L$(DIR_COMMON)/lib -lffmpeg_api

SRCS=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp, %.o, $(SRCS))

PROTOS=$(wildcard *.cc)
PROTO_OBJECTS=$(patsubst %.cc, %.o, $(PROTOS))

all: $(TARGETS) 

huxiaojiaoshi.transcode_server: $(OBJECTS) $(PROTO_OBJECTS)
	$(CXX)  $(CXXFLAGS) -o $@  $^  $(BASE_LIBS)
#	strip $@
	
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


