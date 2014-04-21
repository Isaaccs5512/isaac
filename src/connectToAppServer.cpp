#include "connectToAppServer.h"
#include <memory>
#include <iostream>
#include "debug.h"


std::string get_recstr()
{
	return recstr;
}

std::string ConnectToAppServer::packData(std::string msg,int len)
{
	char *buf = new char[len+4];
	buf[0] = (len & 0xff000000) >> 24;
	buf[1] = (len & 0x00ff0000) >> 16;
	buf[2] = (len & 0x0000ff00) >> 8;
	buf[3] = (len & 0x000000ff);

	memcpy(buf+4,msg.c_str(),len);
	str = std::string(buf,len+4);
	delete []buf;
	return str;
}

void ConnectToAppServer::transportData(std::string msg,int len)
{
	asio::io_service io_service;
	debug();
	asio::ip::tcp::endpoint end_point(asio::ip::address::from_string(SERVERIP), SERVPORT);
	debug();

	asio::ip::tcp::socket socket(io_service);
	debug();
	try{
		socket.connect(end_point);
	}catch(...){
		debug();
		throw;
	}
	debug();
	try{
		socket.write_some(asio::buffer(packData(msg,len)));
	}catch(...){
		debug();
		throw;
	}

	try{
		socket.read_some(asio::buffer(recstr))>0);
	}catch(...){
		debug();
		throw;
	}
debug();
}
