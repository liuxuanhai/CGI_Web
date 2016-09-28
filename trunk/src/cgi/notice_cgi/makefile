include ../../makefile.inc

TARGETS=cgi_sys_notice_get_for_pa \
		cgi_sys_notice_new_get_for_user \
		cgi_notice_notify_sys_notice_get \
		cgi_notice_notify_get \
		cgi_notice_notify_typelist_get \
		cgi_notice_record_get \
		cgi_notice_record_read \
		cgi_notice_record_delete \
#		cgi_sys_notice_all_get_for_user \ 	cgi_sys_notice_delete \ cgi_notice_record_update \ 		cgi_sys_notice_add \ cgi_sys_notice_update cgi_sys_notice_delete 
	
INSTALL_PATH = /home/dev/hoosho/cgi-bin/hp/notice

all: $(TARGETS);

%:%.cpp notice_server_cgi.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BASE_LIBS)

install:
	mkdir -p $(INSTALL_PATH)
	cp $(TARGETS) config.ini logger.properties $(INSTALL_PATH)
clean:
	$(RM) *.o $(TARGETS)


