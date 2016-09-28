include ../../makefile.inc

TARGETS= cgi_common_jsapiticket_signature \
		 cgi_common_get_type_info \
		 cgi_common_get_good_info
		
INSTALL_PATH = /home/dev/hoosho/cgi-bin/pa/s/common/

all: $(TARGETS);

%:%.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BASE_LIBS)
	strip $@ 

install:
	mkdir -p $(INSTALL_PATH)
	cp $(TARGETS) config.ini logger.properties $(INSTALL_PATH)
clean:
	$(RM) *.o $(TARGETS)


