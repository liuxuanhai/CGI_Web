include ../../makefile.inc

TARGETS=ffmpeg_example

DIR_FFMPEG     	= /usr/local/ffmpeg
LIB_FFMPEG     	= -L$(DIR_FFMPEG)/lib -lavformat -lavcodec -lavfilter -lavutil -lswresample
INC_FFMPEG     	= -I$(DIR_FFMPEG)/include

CXXFLAGS 	+= $(INC_FFMPEG)
BASE_LIBS 	+= $(LIB_FFMPEG) -L$(DIR_COMMON)/lib -lffmpegTranscode

SRCS=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp, %.o, $(SRCS))

all: $(TARGETS) 

$(TARGETS): $(OBJECTS)
	$(CXX)  $(CXXFLAGS) -o $@  $^  $(BASE_LIBS)
	#strip $@

clean:
	$(RM) -rf *.o $(TARGETS)


