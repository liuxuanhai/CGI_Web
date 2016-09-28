include ../../makefile.inc

TARGETS=cgi_pay_unifiedorder_callback cgi_pay_commit_outcome cgi_pay_get_outcome_list
		
INSTALL_PATH = /home/dev/hoosho/cgi-bin/pa/z/pay/

all: $(TARGETS);

%:%.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BASE_LIBS)
	strip $@ 

install:
	mkdir -p $(INSTALL_PATH)
	cp $(TARGETS) config.ini logger.properties $(INSTALL_PATH)
clean:
	$(RM) *.o $(TARGETS)


