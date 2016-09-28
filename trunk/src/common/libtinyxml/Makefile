
.PHONEY : all clean install unstall

SRCFILES=$(wildcard ./*.cpp)
OBJECTS := $(patsubst %.cpp,%.o,$(SRCFILES))

TARGET=libtinyxml.a

$(TARGET): $(OBJECTS)
	ar rc $(TARGET) $(OBJECTS)

clean:
	rm -rf $(TARGET) $(OBJECTS)

install:
	mkdir -p /usr/local/include/tinyxml
	mkdir -p /usr/local/lib/tinyxml

	cp $(TARGET)  /usr/local/lib/tinyxml/
	cp *.h   /usr/local/include/tinyxml/

uninstall:
	rm -rf /usr/local/include/tinyxml
	rm -rf /usr/local/lib/tinyxml
