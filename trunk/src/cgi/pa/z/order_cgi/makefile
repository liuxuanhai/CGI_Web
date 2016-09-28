include ../../makefile.inc

DIR_FFMPEG      = /usr/local/ffmpeg
LIB_FFMPEG      = -L$(DIR_FFMPEG)/lib -lavformat -lavcodec -lavfilter -lavutil -lswresample
INC_FFMPEG      = -I$(DIR_FFMPEG)/include

CXXFLAGS    += $(INC_FFMPEG)
BASE_LIBS   += $(LIB_FFMPEG) -L$(DIR_COMMON)/lib -lffmpeg_api


TARGETS = cgi_order_search cgi_order_create cgi_order_fetch cgi_order_detail_get \
		  cgi_order_get_user_fetch_order_list cgi_order_get_user_create_order_list \
		  cgi_order_finish cgi_order_cancel cgi_order_comment cgi_order_reward \
		  cgi_order_num_get cgi_order_fetch_cancel cgi_order_fetch_accept \
		  cgi_order_media_get


INSTALL_PATH = /home/dev/hoosho/cgi-bin/pa/z/order

all: $(TARGETS);

%:%.cpp cgi_order_base.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BASE_LIBS)
	strip $@

install:
	mkdir -p $(INSTALL_PATH)
	sudo cp $(TARGETS) config.ini logger.properties $(INSTALL_PATH)

clean:
	$(RM) *.o $(TARGETS)
