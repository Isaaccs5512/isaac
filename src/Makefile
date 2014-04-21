FLAGS = --std=c++11 -lglog -lpthread -lprotobuf -DWITH_OPENSSL -lssl -lcrypt -g -DDEBUG
INCLUDE = -I./ -I./boost_1_55_0 -I/usr/local/protobuf/include -I./asio-1.10.1/include

LIB = -L/usr/local/lib

CC = g++ $(FLAGS) $(INCLUDE) $(LIB)


PROTOBUF_SRC = $(wildcard ./protobuf/*.cc)
SOURCE = $(wildcard *.cpp)
SERVER_SOURCE = $(SOURCE) $(PROTOBUF_SRC)
CLIENT_SOURCE = soapxiaofangProxy.cpp test.cxx stdsoap2.cpp soapC.cpp
.PHONY:server clean gsoap
server:
	$(CC) -o $@ $(SERVER_SOURCE)
gsoap:
	soapcpp2 -i -S -e webservice.h
client:
	$(CC) -o $@  $(CLIENT_SOURCE)
clean:
	-rm -f server *.o *.d
