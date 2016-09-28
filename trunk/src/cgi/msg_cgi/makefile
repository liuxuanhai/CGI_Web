include ../../makefile.inc

TARGETS=cgi_msg_get_newmsg_status \
		cgi_msg_get_msg_list_by_session \
		cgi_msg_get_session_list \
		cgi_msg_add \
		cgi_msg_del_by_id \
		cgi_msg_del_session \
		cgi_msg_system_red_point_mod \
		cgi_msg_system_red_point_get \
		cgi_msg_notice_record_get \
		cgi_msg_system_message_get
		
INSTALL_PATH = /home/dev/hoosho/cgi-bin/hp/msg

all: $(TARGETS);

%:%.cpp cgi_msg_server.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BASE_LIBS)
	strip $@
	
install:
	mkdir -p $(INSTALL_PATH)
	cp $(TARGETS) config.ini logger.properties $(INSTALL_PATH)
clean:
	$(RM) *.o $(TARGETS)


