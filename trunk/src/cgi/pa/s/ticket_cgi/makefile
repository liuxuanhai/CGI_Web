include ../../makefile.inc

TARGETS= cgi_add_ticket \
		 cgi_get_user_feed_list \
		 cgi_get_user_ticket_list \
		 cgi_get_feed_detail \
		 cgi_get_feed_list \
		 cgi_get_feed_join_user_list \
		 cgi_get_user_show_list \
		 cgi_get_show_list \
		 cgi_get_show_detail \
		 cgi_add_show \
		 cgi_get_user_collect_list \
		 cgi_get_user_luck_list \
		 cgi_add_collect \
		 cgi_get_banner_list \
		 cgi_del_collect \
		 cgi_check_user_collect \
		 cgi_pic_download \
		 cgi_get_good_info_list
		
INSTALL_PATH = /home/dev/hoosho/cgi-bin/pa/s/ticket/

all: $(TARGETS);

%:%.cpp cgi_server.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BASE_LIBS)
	strip $@ 

install:
	mkdir -p $(INSTALL_PATH)
	cp $(TARGETS) config.ini logger.properties $(INSTALL_PATH)
clean:
	$(RM) *.o $(TARGETS)


