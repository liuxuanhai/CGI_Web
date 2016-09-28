include ../../makefile.inc

TARGETS=cgi_manager_login cgi_manager_logout cgi_feed_add cgi_feed_delete cgi_feed_list\
		cgi_modify_passwd cgi_manager_add cgi_manager_list_get cgi_manager_delete cgi_manager_info_get \
		cgi_forbid_user cgi_report_user_list cgi_report_feed_list cgi_report_feed_detail \
		cgi_pa_info_get cgi_pa_info_update \

SRCS=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp, %.o, $(SRCS))

PROTOS=$(wildcard *.cc)
PROTO_OBJECTS=$(patsubst %.cc, %.o, $(PROTOS))
	
INSTALL_PATH=/home/dev/hoosho/cgi-bin/hp/manager

all: $(TARGETS);

%:%.cpp $(PROTO_OBJECTS) manager_cgi.cpp comm_struct.cpp
	$(CXX) $(CXXFLAGS) -std=c++11 -o $@ $^ $(BASE_LIBS)
	strip $@
	
install:
	mkdir -p $(INSTALL_PATH)
	sudo cp $(TARGETS) config.ini logger.properties $(INSTALL_PATH)
clean:
	$(RM) *.o $(TARGETS)


