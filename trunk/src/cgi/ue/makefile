include ../../makefile.inc

EXES = cgi_ue_resource_download  cgi_ue_resource_upload \
	cgi_ue_content_download  cgi_ue_content_upload 

all: $(EXES)

%:%.cpp
	$(CXX)  $(CXXFLAGS) -o $@  $^  $(BASE_LIBS)


clean:
	rm -f $(EXES) *.o

reload:
	sudo /usr/local/apache2/bin/httpd -k restart
