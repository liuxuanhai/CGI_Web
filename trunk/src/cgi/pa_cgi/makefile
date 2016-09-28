include ../../makefile.inc

TARGETS=cgi_pa_expert_get \
	cgi_pa_tab_get
		
INSTALL_PATH = /home/dev/hoosho/cgi-bin/hp/pa

all: $(TARGETS);

%:%.cpp cgi_pa_server.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BASE_LIBS)

install:
	mkdir -p $(INSTALL_PATH)
	cp $(TARGETS) config.ini logger.properties $(INSTALL_PATH)
clean:
	$(RM) *.o $(TARGETS)


