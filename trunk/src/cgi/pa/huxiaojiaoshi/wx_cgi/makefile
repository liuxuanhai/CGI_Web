include ../../makefile.inc

TARGETS = 	cgi_wx_create_menu cgi_wx_get_menu cgi_wx_delete_menu cgi_wx_send_template_message \
			cgi_wx_send_message 
		
INSTALL_PATH = /home/dev/hoosho/cgi-bin/pa/huxiaojiaoshi/wx_cgi

all: $(TARGETS)

%:%.cpp $(PROTO_OBJECTS) cgi_wx_base.cpp
	$(CXX)  $(CXXFLAGS) -o $@  $^  $(BASE_LIBS)
	strip $@
	
install:
	sudo mkdir -p $(INSTALL_PATH)
	sudo cp $(TARGETS) config.ini logger.properties $(INSTALL_PATH)
clean:
	rm -f $(TARGETS) *.o

reload:
	sudo /usr/local/apache2/bin/httpd -k restart
