include ../../makefile.inc

TARGETS=cgi_pay_user_cash_get \
	cgi_pay_unifiedorder \
	cgi_pay_unifiedorder_callback \
	cgi_pay_transfer \
	cgi_pay_cash_flow_query_by_user \
	cgi_pay_cash_flow_query_by_manager \
	cgi_pay_sendredpack \
	cgi_pay_user_outcome_req_query_by_user \
	cgi_pay_user_outcome_req_query_by_manager \
	cgi_pay_user_outcome_req_commit \
	cgi_pay_check_out_trade_no \
		
INSTALL_PATH = /home/dev/hoosho/cgi-bin/pa/huxiaojiaoshi/pay/

all: $(TARGETS);

%:%.cpp cgi_pay_server.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BASE_LIBS)
	strip $@ 

install:
	mkdir -p $(INSTALL_PATH)
	cp $(TARGETS) config.ini logger.properties $(INSTALL_PATH)
clean:
	$(RM) *.o $(TARGETS)


