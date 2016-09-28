include ../../makefile.inc

TARGETS= cgi_get_feed_list cgi_get_feed_detail cgi_get_user_feed_list cgi_add_feed \
			cgi_get_feed_follow_list cgi_get_follow_detail cgi_get_user_follow_list cgi_add_follow \
			cgi_get_user_listen_list cgi_listen_follow cgi_add_listen cgi_check_listen\
			cgi_get_user_history_list \
			cgi_comment_follow cgi_get_comment_follow cgi_check_business_id_valid

SRCS=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp, %.o, $(SRCS))

PROTOS=$(wildcard *.cc)
PROTO_OBJECTS=$(patsubst %.cc, %.o, $(PROTOS))
	
INSTALL_PATH=/home/dev/hoosho/cgi-bin/pa/huxiaojiaoshi/feeds

all: $(TARGETS);

%:%.cpp $(PROTO_OBJECTS) cgi_feeds_base.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BASE_LIBS)
	strip $@

install:
	sudo mkdir -p $(INSTALL_PATH)
	sudo cp $(TARGETS) config.ini logger.properties $(INSTALL_PATH)
clean:
	$(RM) *.o $(TARGETS)


