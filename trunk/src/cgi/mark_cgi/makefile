include ../../makefile.inc

TARGETS=cgi_mark_query \
		cgi_mark_add \
		cgi_mark_del
		
INSTALL_PATH = /home/dev/hoosho/cgi-bin/hp/mark

all: $(TARGETS);

%:%.cpp cgi_mark_server.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BASE_LIBS)

install:
	mkdir -p $(INSTALL_PATH)
	cp $(TARGETS) config.ini logger.properties $(INSTALL_PATH)
clean:
	$(RM) *.o $(TARGETS)


