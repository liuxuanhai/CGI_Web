include ../../makefile.inc

TARGETS = cgi_manager_get_vc \
		  cgi_manager_login \
 		  cgi_manager_logout \
		  cgi_manager_add_good_type \
		  cgi_manager_add_good \
		  cgi_manager_add_consume_type \
		  cgi_manager_get_type_info \
		  cgi_manager_get_good_info \
		  cgi_manager_get_feed_list \
		  cgi_manager_add_feed \
		  cgi_manager_get_ticket_list \
 		  cgi_manager_check_ticket \
		  cgi_manager_get_user_identity_info_list \
 		  cgi_manager_check_user_identity_info \
		  cgi_manager_add_banner \
		  cgi_manager_del_banner \
		  cgi_manager_get_banner_list \
		  cgi_manager_pic_download \
		  cgi_manager_pic_upload 

INSTALL_PATH = /home/dev/hoosho/cgi-bin/pa/s/manager

all: $(TARGETS);

%:%.cpp cgi_manager_base.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BASE_LIBS)
	strip $@

install:
	mkdir -p $(INSTALL_PATH)
	sudo cp $(TARGETS) config.ini logger.properties create_memcache.sh $(INSTALL_PATH)

clean:
	$(RM) *.o $(TARGETS)
