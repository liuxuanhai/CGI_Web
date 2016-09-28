include ../../makefile.inc

TARGETS =  cgi_wx_msg_callback 
		
INSTALL_PATH = /home/dev/hoosho/cgi-bin/pa/huxiaojiaoshi/wx_callback

all: $(TARGETS)

%:%.cpp $(PROTO_OBJECTS) cgi_wx_callback.cpp
	$(CXX)  $(CXXFLAGS) -o $@  $^  $(BASE_LIBS) $(LIB_WXBizMsgCrypt) $(LIB_CRYPTO)
	strip $@
	
install:
	sudo mkdir -p $(INSTALL_PATH)
	sudo cp $(TARGETS) config.ini logger.properties $(INSTALL_PATH)
clean:
	rm -f $(TARGETS) *.o

reload:
	sudo /usr/local/apache2/bin/httpd -k restart
