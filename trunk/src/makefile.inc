#directory definations

DIR_COMMON     = ../../common
DIR_INCLUDE    = /usr/local/include
DIR_LIBRARY    = /usr/local/lib
DIR_MYSQL      = /usr/local/mysql

#base library definations
LIB_COMMON      = -L$(DIR_COMMON)/lib -lhoosho_proto -lcommon_protoio -lcommon_util -lwx_api -limage_helper
LIB_LCE        = -L$(DIR_LIBRARY)/lce/ -llce
LIB_MYSQL      = /usr/local/mysql/lib/libmysqlclient.a
LIB_LOGGER     = /usr/local/lib/liblog4cplus.a
LIB_TINYXML    = /usr/local/lib/tinyxml/libtinyxml.a
LIB_MEMCACHE   = /usr/local/lib/libmemcached.a 
LIB_ZLIB       = /usr/local/lib/libz.a
LIB_PB         = /usr/local/lib/libprotobuf.a
LIB_MAGICK	   = -L/usr/local/lib/ -lMagick++ -lMagickWand -lMagickCore -ljpeg -lpng -lgd
LIB_OTHERS     = -lpthread -lm -lrt -ldl -lssl -lcrypto
LIB_WXBizMsgCrypt = /usr/local/lib/libWXBizMsgCrypt.a

#base include definations
INC_LCE    	   = -I$(DIR_INCLUDE)/lce
INC_MYSQL      = -I$(DIR_MYSQL)/include
INC_COMMON     = -I$(DIR_COMMON)/include
INC_MAGICK 	   = -I/usr/local/include/ImageMagick

#command definations
EXTRAFLAGS     = -g -Wall -ftemplate-depth-50 -fpermissive -Wno-deprecated -fPIC -D__LINUX__  -DLOGGER -DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16
INCLUDES       =  $(INC_LCE) $(INC_COMMON) $(INC_MYSQL) $(INC_MAGICK)
CXXFLAGS       = $(INCLUDES) $(EXTRAFLAGS)
BASE_LIBS      = $(LIB_COMMON) $(LIB_MAGICK) $(LIB_LCE) $(LIB_MYSQL) $(LIB_LOGGER) $(LIB_TINYXML) $(LIB_MEMCACHE)  $(LIB_ZLIB) $(LIB_OTHERS) $(LIB_PB)
MAKE           = make
CXX            = g++
LEX            = flex
YACC           = bison
MV             = mv -f
RM             = rm -f
CP             = cp -f
AR             = ar rc

#rule definations
.SUFFIXES:  .cxx
.cpp.o:
	$(CXX) -o $@ -c $< $(CXXFLAGS)
.cxx.o:
	$(CXX) -o $@ -c $< $(CXXFLAGS)
.cc.o:
	$(CXX) -o $@ -c $< $(CXXFLAGS)

