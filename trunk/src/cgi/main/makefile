
.SUFFIXES:

CXX = g++ -g -DLOGGER  # -ggdb -gdwarf-2
CXXFLAGS = -g -Wall -Werror -fPIC -z defs -std=c++11 -DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16  #-DHAVE_CONFIG_H

CGI_MAIN = cgi_user_register cgi_user_login  cgi_user_logout cgi_user_order_get cgi_send_vc \
		cgi_user_info_get cgi_user_info_mod cgi_dev_apply cgi_dev_mod cgi_oper_apply cgi_oper_mod \
		cgi_wxpa_add cgi_wxpa_mod cgi_wxpa_del cgi_wxpa_feedback cgi_wxpa_detail_get\
		cgi_pic_upload cgi_pic_upload_private cgi_pic_download cgi_pic_download_private cgi_doc_upload cgi_doc_download \
		cgi_dev_ranklist cgi_wxpa_filter cgi_dev_detail_get cgi_oper_detail_get  cgi_recommend_list_get\
		cgi_dev_order_add cgi_oper_order_add cgi_open_order_add cgi_test_302

		
EXES = $(CGI_MAIN)

MY_INC = \
		 -I/usr/local/include \
		 -I/usr/local/include/lce \
		 -I/usr/local/mysql/include \
		 -I/usr/local/include/ImageMagick
		 

MY_LIB = \
		 -L/usr/local/lib/lce -llce \
		 /usr/local/lib/liblog4cplus.a \
		 /usr/local/lib/libmemcached.a \
		 /usr/local/lib/tinyxml/libtinyxml.a \
		 /usr/local/mysql/lib/libmysqlclient.a \
		 /usr/local/lib/libz.a \
		 -lpthread -lm -lrt -ldl -lssl -lcrypto -lgd \
		

all: $(EXES);
	mkdir -p ../../../cgi-bin/main
	mv $(CGI_MAIN) ../../../cgi-bin/main/
	cp config.ini logger.properties ../../../cgi-bin/main/

cgi_pic_upload: cgi_pic_upload.o main_cgi.o comm_struct.o image_helper.o
	$(CXX) $(CXXFLAGS) -fopenmp -o $@ $^  -lMagick++ -lMagickWand -lMagickCore -ljpeg -lpng $(MY_LIB)
	strip $@
	
%: %.o  main_cgi.o comm_struct.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(MY_LIB)
	strip $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $(MY_INC) -c $<
	
clean:
	rm -f $(EXES) *.o

reload:
	sudo /usr/local/apache2/bin/httpd -k restart
