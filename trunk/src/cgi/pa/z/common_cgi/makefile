include ../../makefile.inc

TARGETS= cgi_common_jsapiticket_signature \
	cgi_qixi_share_page_statistics \
	cgi_common_redirect
		
INSTALL_PATH = /home/dev/hoosho/cgi-bin/pa/z/common/

all: $(TARGETS);

%:%.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BASE_LIBS)
	strip $@ 

install:
	mkdir -p $(INSTALL_PATH)
	cp $(TARGETS) config.ini logger.properties $(INSTALL_PATH)
clean:
	$(RM) *.o $(TARGETS)


