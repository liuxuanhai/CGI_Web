#command definations
MAKE  	   = make
CXX        = g++ -ftemplate-depth-50 -Wall -Wno-deprecated
EXTRAFLAGS = -g -fpermissive -DLOGGER
INCLUDES   = -I./ -I/usr/local/include  -I/usr/local/mysql/include
CXXFLAGS   = -fPIC $(INCLUDES) $(EXTRAFLAGS) $(LIBS)

#rule definations
.SUFFIXES:  .cxx
.cpp.o:
	$(CXX)  $(CXXFLAGS) -o $@ -c $<
.cxx.o:
	$(CXX)  $(CXXFLAGS) -o $@ -c $<
.cc.o:
	$(CXX)  $(CXXFLAGS) -o $@ -c $<


.PHONEY : all clean install unstall

SRCFILES=$(wildcard ./*/*.cpp)
OBJECTS := $(patsubst %.cpp,%.o,$(SRCFILES))

TARGET=liblce.a

$(TARGET): $(OBJECTS)
	ar rc $(TARGET) $(OBJECTS)
	rm $(OBJECTS)

clean:
	rm -rf $(TARGET) $(OBJECTS)

install:
	mkdir -p /usr/local/include/lce
	mkdir -p /usr/local/lib/lce

	cp $(TARGET)  /usr/local/lib/lce/

	mkdir -p /usr/local/include/lce/memory
	mkdir -p /usr/local/include/lce/util
	mkdir -p /usr/local/include/lce/app
	mkdir -p /usr/local/include/lce/thread
	mkdir -p /usr/local/include/lce/net
	mkdir -p /usr/local/include/lce/cgi
	mkdir -p /usr/local/include/lce/jsoncpp

	cp ./memory/*.h  /usr/local/include/lce/memory/
	cp ./util/*.h    /usr/local/include/lce/util/
	cp ./app/*.h     /usr/local/include/lce/app/
	cp ./thread/*.h  /usr/local/include/lce/thread/
	cp ./net/*.h     /usr/local/include/lce/net/
	cp ./cgi/*.h     /usr/local/include/lce/cgi/
	cp ./jsoncpp/*.h     /usr/local/include/lce/jsoncpp/

uninstall:
	rm -rf /usr/local/include/lce
	rm -rf /usr/local/lib/lce
