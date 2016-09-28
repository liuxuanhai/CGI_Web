include ../../makefile.inc

TARGETS = cgi_manager_get_vc \
		  cgi_manager_login \
 		  cgi_manager_logout \
		  cgi_manager_get_invoice_list \
 		  cgi_manager_invoice_pass \
		  cgi_manager_finish_outcome \
		  cgi_manager_get_outcome_list \
		  cgi_manager_get_cash_flow_list

INSTALL_PATH = /home/dev/hoosho/cgi-bin/pa/z/manager

all: $(TARGETS);

%:%.cpp cgi_manager_base.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BASE_LIBS)
	strip $@

install:
	mkdir -p $(INSTALL_PATH)
	sudo cp $(TARGETS) config.ini logger.properties create_memcache.sh $(INSTALL_PATH)

clean:
	$(RM) *.o $(TARGETS)
