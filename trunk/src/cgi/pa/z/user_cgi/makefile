include ../../makefile.inc

TARGETS= cgi_user_login \
	 cgi_user_get_vc \
	 cgi_user_info_update_phone \
     cgi_user_info_update \
 	 cgi_user_info_get \
	 cgi_user_cash_get \
	 cgi_user_get_recommend_list \
	 cgi_user_trust \
	 cgi_user_trust_sb_check \
	 cgi_user_get_my_trust_list \
	 cgi_user_get_trust_me_list \
	 cgi_user_activity_info_update \
	 cgi_user_activity_info_get \
	 cgi_user_get_lottery_record_list \
	 cgi_user_get_available_lottery_time \
	 cgi_user_do_lottery \
	 cgi_user_get_ranking_list
		
INSTALL_PATH = /home/dev/hoosho/cgi-bin/pa/z/user/

all: $(TARGETS);

%:%.cpp cgi_server.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BASE_LIBS)
	strip $@ 

install:
	mkdir -p $(INSTALL_PATH)
	cp $(TARGETS) config.ini logger.properties $(INSTALL_PATH)
clean:
	$(RM) *.o $(TARGETS)


