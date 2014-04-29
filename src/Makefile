FLAGS =--std=c++11 -lglog -lpthread -lprotobuf -DWITH_OPENSSL -lssl -lcrypt -g -DDEBUG -gdwarf-2 -gstrict-dwarf
INCLUDE = -I./ -I./boost_1_55_0 -I/usr/local/protobuf/include -I./asio-1.10.1/include

LIB = -L/usr/local/lib

CC = g++ $(FLAGS) $(INCLUDE) $(LIB)


PROTOBUF_SRC = $(wildcard ./protobuf/*.cc)
PROTOBUF_OBJ = $(patsubst %.cc,%.o,$(PROTOBUF_SRC))
SOURCE = $(wildcard *.cpp)
SOURCE_OBJ = $(patsubst %.cpp,%.o,$(SOURCE))
SERVER_SOURCE = $(SOURCE) $(PROTOBUF_SRC)
SERVER_OBJ=$(PROTOBUF_OBJ) $(SOURCE_OBJ)
.PHONY:all gsoap
all : server
gsoap:
	soapcpp2 -i -S -e webservice.h
server:$(SERVER_OBJ)
	$(CC) -o $@ $^
%.o:%.cpp
	$(CC) -o $@ -c $< $(FLAGS) $(INCLUDE)
%.d:%.cpp
	@set -e; rm -f $@; $(CC) -MM $< $(INCLUDE) > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(OBJS:.o=.d)

.PHONY:clean gsoap_clean
clean:
	rm -f server *.o *.d *.d.* ./protobuf/*.o 
gsoap_clean:
	rm -f xiaofang* soap* ns.xsd
