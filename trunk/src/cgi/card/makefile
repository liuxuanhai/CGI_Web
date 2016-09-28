
.SUFFIXES:

CXX = g++ -g -DLOGGER  # -ggdb -gdwarf-2
CXXFLAGS = -g -Wall -Werror -fPIC -z defs -std=c++11

CGI_MAIN =  cgi_user_login_card cgi_user_logout_card \
			cgi_user_register_card \
			cgi_user_info_get_card \
		    cgi_verify_captcha_card cgi_create_captcha_card \
		    cgi_forget_password_card cgi_reset_password_card \
			cgi_wx_create_landing_page \
			cgi_get_wx_api_data \
			cgi_wx_card_create cgi_wx_card_delete cgi_wx_card_update  \
			cgi_wx_card_list_get cgi_wx_card_detail_get  \
			cgi_wx_card_consume cgi_wx_card_statistic_get \
		    cgi_wx_card_qrcode_get cgi_wx_card_code_query cgi_wx_card_logo_upload\
			cgi_wx_card_member_create cgi_wx_card_member_activateform_create cgi_wx_card_set_paysell cgi_wx_membercard_userinfo_query \
			cgi_wx_entityshop_create cgi_wx_entityshop_delete cgi_wx_entityshop_list_get_from_wx\
			cgi_wx_entityshop_update cgi_wx_entityshop_list_get cgi_wx_entityshop_detail_get \
			cgi_wx_media_image_upload cgi_wx_media_video_upload cgi_wx_media_voice_upload cgi_wx_media_thumb_upload \
			cgi_wx_media_image_download \
			cgi_wx_upload_news cgi_wx_upload_video cgi_wx_send_all_preview cgi_wx_send_all \
			cgi_wx_submerchant_create cgi_wx_submerchant_delete cgi_wx_submerchant_update  \
			cgi_wx_submerchant_list_get cgi_wx_submerchant_detail_get \
			cgi_wx_applyprotocol_get\
			cgi_pic_upload_card cgi_pic_download_card \
			cgi_wx_pic_download \
			cgi_wx_category_query \
			cgi_wx_entityshop_list_get_by_id \
			cgi_get_pre_auth_code \
			cgi_wx_ticket_get \
			cgi_wx_menubar \
			
CGI_CALLBACK =	cgi_wx_auth_event cgi_wx_auth_result \
				cgi_wx_account_msg \
				cgi_wx_msg_pushed \
		
EXES = $(CGI_MAIN) $(CGI_CALLBACK)

#test

MY_INC = \
		 -I/usr/local/include \
		 -I/usr/local/include/lce \
		 -I/usr/local/mysql/include

MY_LIB = \
		 -L/usr/local/lib/lce -llce \
		 /usr/local/lib/liblog4cplus.a \
		 /usr/local/lib/libmemcached.a \
		 /usr/local/lib/tinyxml/libtinyxml.a \
		 /usr/local/lib/libqrencode.a \
		 /usr/local/mysql/lib/libmysqlclient.a \
		 /usr/local/lib/libWXBizMsgCrypt.a \
		 /usr/local/lib/libz.a \
		 -lpthread -lm -lrt -ldl -lssl -lcrypto -lgd -lpng 

BUILD_CGI = $(CXX) $(CXXFLAGS) -o $@ $^ $(MY_LIB);
all: $(EXES);
	mkdir -p ../../../cgi-bin/card;
	mv $(CGI_MAIN) ../../../cgi-bin/card/;
	cp config_card.ini logger.properties_card cmr10.ttf ../../../cgi-bin/card/;
	@if [ ! -d "../../../cgi-bin/callback" ]; \
	then \
		 mkdir -p ../../../cgi-bin/callback;  \
	fi  
	mv $(CGI_CALLBACK) ../../../cgi-bin/callback/;
	cp config_card.ini logger.properties_card ../../../cgi-bin/callback/;	
%: %.o  hoosho_cgi_card.o comm_struct_card.o wx_https_req.o qr_png.o sms_https_req.o comm_struct_relation_card.o
	$(BUILD_CGI)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $(MY_INC) -c $<
	
clean:
	rm -f $(EXES) *.o	

set:
	cp $(CGI_MAIN) config.ini logger.properties cmr10.ttf ../cgi-bin/card/
	rm -f $(EXES)
dist:
	sz $(EXES) config.ini logger.properties
