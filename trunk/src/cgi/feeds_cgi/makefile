include ../../makefile.inc

TARGETS=cgi_feed_detail cgi_feed_list cgi_feed_add cgi_feed_delete \
		cgi_follow_detail cgi_follow_comment_detail cgi_follow_comment_list cgi_follow_add cgi_follow_delete  \
		cgi_feed_list_favorite_check cgi_favorite_list cgi_favorite_mod  \
		cgi_feed_list_collect_check cgi_collect_list cgi_collect_mod	\
		cgi_gen_expert cgi_report_feed\

SRCS=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp, %.o, $(SRCS))

PROTOS=$(wildcard *.cc)
PROTO_OBJECTS=$(patsubst %.cc, %.o, $(PROTOS))
	
INSTALL_PATH=/home/dev/hoosho/cgi-bin/hp/feeds

all: $(TARGETS);

%:%.cpp $(PROTO_OBJECTS) cgi_feeds_server.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BASE_LIBS)
	strip $@

install:
	mkdir -p $(INSTALL_PATH)
	sudo cp $(TARGETS) config.ini logger.properties $(INSTALL_PATH)
clean:
	$(RM) *.o $(TARGETS)


