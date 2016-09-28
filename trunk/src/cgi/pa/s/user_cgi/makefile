include ../../makefile.inc

TARGETS= cgi_user_login \
	 cgi_user_get_vc \
	 cgi_user_info_update_phone \
     cgi_user_info_update \
 	 cgi_user_info_get \
	 cgi_user_identity_info_update \
	 cgi_user_identity_info_get \
	 cgi_user_recv_addr_info_add \
	 cgi_user_recv_addr_info_del \
	 cgi_user_recv_addr_info_update \
	 cgi_user_recv_addr_info_get 

		
INSTALL_PATH = /home/dev/hoosho/cgi-bin/pa/s/user/

all: $(TARGETS);

%:%.cpp cgi_server.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BASE_LIBS)
	strip $@ 

install:
	mkdir -p $(INSTALL_PATH)
	cp $(TARGETS) config.ini logger.properties $(INSTALL_PATH)
clean:
	$(RM) *.o $(TARGETS)


