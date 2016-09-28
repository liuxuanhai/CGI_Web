include ../../makefile.inc

EXES = cgi_user_login cgi_user_info_get 

all: $(EXES)

%:%.cpp
	$(CXX)  $(CXXFLAGS) -o $@  $^  $(BASE_LIBS)
	strip $@

install:
	sudo cp $(EXES) /home/dev/hoosho/cgi-bin/pa/huxiaojiaoshi/user_cgi/

clean:
	rm -f $(EXES) *.o

reload:
	sudo /usr/local/apache2/bin/httpd -k restart
