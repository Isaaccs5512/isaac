#include "connectToAppServer.h"
#include <memory>
#include <iostream>
#include <exception>
#include <string>
#include <array>

std::string ConnectToAppServer::get_recstr()
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

int ConnectToAppServer::transportData(std::string msg,int len)
{
	asio::io_service io_service;
	char *buf;
	char *lenbuf = new char[4];
	int reclen=0;
	asio::error_code ec;
	asio::ip::tcp::endpoint end_point(asio::ip::address::from_string(SERVERIP), SERVPORT);

	asio::ip::tcp::socket socket(io_service);
	
	socket.connect(end_point,ec);
	if(ec)
	{
		LOG(ERROR)<<ec.message();
		return -1;
	}
	LOG(INFO)<<"Send request to server";	
	//socket.write_some(asio::buffer(packData(msg,len)),ec);
	packData(msg,len);
	asio::write(socket,asio::buffer(str,len+4),ec);
	if(ec)
	{
		LOG(ERROR)<<ec.message();
		return -2;
	}

	LOG(INFO)<<"Recive response data from server";
	asio::read(socket,asio::buffer(lenbuf,4),ec);
	if(ec)
	{
		LOG(ERROR)<<ec.message();
		return -3;
	}
	reclen |= ((lenbuf[3])&0x000000ff);
	reclen |= ((lenbuf[2])&0x000000ff) << 8;
	reclen |= ((lenbuf[1])&0x000000ff) << 16;
	reclen |= ((lenbuf[0])&0x000000ff) << 24;

	LOG(INFO)<<"Datalen is:"<<reclen;
	buf = new char[reclen];
	asio::read(socket,asio::buffer(buf,reclen),ec);

	if(ec)
	{
		LOG(ERROR)<<ec.message();
		return -4;
	}
	recstr = std::string(buf,reclen);
	delete []buf;
	delete []lenbuf;
	return 0;
}
