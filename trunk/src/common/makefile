part:	
	$(MAKE) -C proto M=$(.) love all install;
	$(MAKE) -C util M=$(.) all install;
all:
	$(MAKE) -C proto M=$(.) love;
	$(MAKE) -C proto M=$(.) all install;
	$(MAKE) -C util M=$(.) all install;
	$(MAKE) -C image M=$(.) all install;
	$(MAKE) -C proto_io M=$(.) all install;
	$(MAKE) -C wx_api M=$(.) all install;
	sudo $(MAKE) -C lce M=$(.) liblce.a install;
	$(MAKE) -C ffmpeg_api M=$(.) all install;
	$(MAKE) -C bd_api M=$(.) all install;
	$(MAKE) -C sms_api M=$(.) all install;
	sudo $(MAKE) -C libtinyxml M=$(.) libtinyxml.a install;
	

tags:
	$(ctags -R)

clpart:
	$(MAKE) -C proto M=$(.) clean;
	$(MAKE) -C util M=$(.) clean;
clall:
	$(MAKE) -C proto M=$(.) clean;
	$(MAKE) -C util M=$(.) clean;
	$(MAKE) -C image M=$(.) clean;
	$(MAKE) -C proto_io M=$(.) clean;
	$(MAKE) -C wx_api M=$(.) clean;
	$(MAKE) -C lce M=$(.) clean;
	$(MAKE) -C ffmpeg_api M=$(.) clean;
	$(MAKE) -C bd_api M=$(.) clean;
	$(MAKE) -C sms_api M=$(.) clean;
	$(MAKE) -C libtinyxml M=$(.) clean;
.PHONY: all clean tags

 
