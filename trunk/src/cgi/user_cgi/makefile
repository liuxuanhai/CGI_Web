include ../../makefile.inc

TARGETS = cgi_user_login \
	cgi_user_info_get \
	cgi_user_info_update \
	cgi_user_fans_get \
	cgi_user_fans_num_get \
	cgi_user_follows_get \
	cgi_user_follow \
	cgi_user_power_get \
	cgi_user_zombie_login \
	cgi_user_get_zombie_list \
	cgi_user_get_pa_list \
	cgi_user_get_zomanager_list
		
INSTALL_PATH = /home/dev/hoosho/cgi-bin/hp/user

all: $(TARGETS)

%:%.cpp
	$(CXX)  $(CXXFLAGS) -o $@  $^  $(BASE_LIBS)
	strip $@
	
install:
	mkdir -p $(INSTALL_PATH)
	cp $(TARGETS) config.ini logger.properties $(INSTALL_PATH)
clean:
	rm -f $(TARGETS) *.o

reload:
	sudo /usr/local/apache2/bin/httpd -k restart
