include ../../makefile.inc

TARGETS=cgi_msg_add \
	cgi_msg_del \
	cgi_msg_del_session \
	cgi_msg_get_newmsg_status \
	cgi_msg_get_msg_list \
	cgi_msg_get_session_list \
	cgi_msg_get_red_point \
	cgi_msg_get_notice_list
		
INSTALL_PATH = /home/dev/hoosho/cgi-bin/pa/z_test/msg

all: $(TARGETS);

%:%.cpp cgi_msg_server.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BASE_LIBS)
	strip $@
	
install:
	mkdir -p $(INSTALL_PATH)
	cp $(TARGETS) config.ini logger.properties $(INSTALL_PATH)
clean:
	$(RM) *.o $(TARGETS)


