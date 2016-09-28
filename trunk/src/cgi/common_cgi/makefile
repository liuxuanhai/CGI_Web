include ../../makefile.inc

EXES = cgi_common_wx_pic_upload cgi_common_raw_pic_upload cgi_common_pic_download cgi_common_jsapiticket_signature 

all: $(EXES)

%:%.cpp
	$(CXX)  $(CXXFLAGS) -o $@  $^  $(BASE_LIBS)


clean:
	rm -f $(EXES) *.o

reload:
	sudo /usr/local/apache2/bin/httpd -k restart
