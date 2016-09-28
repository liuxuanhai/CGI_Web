include ../../makefile.inc

TARGETS = cgi_order_check_favor cgi_order_update_favor cgi_order_get_favor_list cgi_order_add_comment  \
		  cgi_order_add_reply cgi_order_get_comment_list cgi_order_get_comment_detail cgi_order_del_follow


INSTALL_PATH = /home/dev/hoosho/cgi-bin/pa/z/feeds

all: $(TARGETS);

%:%.cpp cgi_feeds_base.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BASE_LIBS)
	strip $@

install:
	mkdir -p $(INSTALL_PATH)
	sudo cp $(TARGETS) config.ini logger.properties $(INSTALL_PATH)

clean:
	$(RM) *.o $(TARGETS)
