include ../../makefile.inc

EXES = cgi_common_jsapiticket_signature 

all: $(EXES)

%:%.cpp
	$(CXX)  $(CXXFLAGS) -o $@  $^  $(BASE_LIBS)

install:
	sudo cp $(EXES) /home/dev/hoosho/cgi-bin/pa/huxiaojiaoshi/common/

clean:
	rm -f $(EXES) *.o

reload:
	sudo /usr/local/apache2/bin/httpd -k restart
