#include "connectToAppServer.h"
#include <memory>
#include <iostream>
#include <exception>
#include <string>
#include <array>
#include "boost_1_55_0/boost/date_time/posix_time/posix_time.hpp"
#include "protobuf/app.dispatch.pb.h"
#include <string>
#include "timer.hpp"
#include "task.hpp"
#include <chrono>
#include <thread>

std::string ConnectToAppServer::get_response()
{
	return recstr;
}

int ConnectToAppServer::get_result()
{
	return result;
}

void ConnectToAppServer::keep_alive(const unsigned long session_id)
{
	create_timer([this,session_id](){
		std::string keep_alive_str = "";
		Get_Keepalive_Request_str(session_id,&keep_alive_str);
		send_request(keep_alive_str);
		
		if(exit)
		{
			std::exception e;
			exit_keep_alive_thread = true;
			std::cout<<"exit_keep_alive_thread"<<std::endl;
			throw e;
		}		
	},5,true);
}

ConnectToAppServer::ConnectToAppServer(asio::io_service &io_service):socket_(io_service),recstr(""),result(0),
																	exit(false),exit_keep_alive_thread(false),exit_read_thread(false)
{
	if(!connect())
	{
		return;
	}

	struct timeval tv;
    tv.tv_sec  = 3; 
    tv.tv_usec = 0;
	if( ::setsockopt(socket_.native_handle(), SOL_SOCKET, SO_RCVTIMEO, 
				(const char*)&tv, sizeof(tv)) != 0)
	{
		LOG(ERROR)<< "RCVTIMEO not set properly."<<errno;
		result = -5;
		return;
	}
	if( ::setsockopt(socket_.native_handle(), SOL_SOCKET, SO_SNDTIMEO, 
				(const char*)&tv, sizeof(tv)) != 0)
	{
		LOG(ERROR)<< "SNDTIMEO not set properly."<<errno;
		result = -5;
		return;
	}

	run_job([this](){
		while(true)
		{
			read_head();
			std::cout<<"333333333333333"<<std::endl;
			if(exit)
			{
				std::exception e;
				exit_read_thread = true;
				std::cout<<"exit_read_thread"<<std::endl;
				throw e;
			}
		}
	});	
}

ConnectToAppServer::~ConnectToAppServer()
{
	std::cout<<"Destructor is called"<<std::endl;
	exit = true;
	do
	{}while(!exit_keep_alive_thread);
	do
	{}while(!exit_read_thread);
}

std::string ConnectToAppServer::packData(std::string msg,int len)
{
	std::tr1::shared_ptr<char> buf(new char[len+4]);
	buf.get()[0] = (len & 0xff000000) >> 24;
	buf.get()[1] = (len & 0x00ff0000) >> 16;
	buf.get()[2] = (len & 0x0000ff00) >> 8;
	buf.get()[3] = (len & 0x000000ff);

	memcpy(buf.get()+4,msg.c_str(),len);
	return std::string(buf.get(),len+4);
}

bool ConnectToAppServer::connect()
{
	asio::ip::tcp::endpoint end_point(asio::ip::address::from_string(SERVERIP), SERVPORT);
	asio::error_code ec;

	socket_.connect(end_point,ec);
	if(ec)
	{
		result = -1;
		return false;
	}
	return true;
} 

void ConnectToAppServer::send_request(std::string requeststr)
{
	asio::error_code ec;
	std::string send_str = packData(requeststr,requeststr.length());
	asio::write(socket_,asio::buffer(send_str,send_str.length()),ec);
	if(ec)
	{
		result = -2;
	}
}

void ConnectToAppServer::read_head() 
{
	asio::error_code ec;
	std::tr1::shared_ptr<char> lenbuf(new char[4]);
    memset(lenbuf.get(),sizeof(lenbuf), 0);

	asio::read(socket_,asio::buffer(lenbuf.get(),4),ec);
	if(ec)
	{
		result = -3;
		return;
	}

	read_body(lenbuf);
}

void ConnectToAppServer::read_body(std::tr1::shared_ptr<char> lenbuf) 
{
	int recvlen_ = 0;
	asio::error_code ec;
	recvlen_ |= ((lenbuf.get()[3])&0x000000ff);
	recvlen_ |= ((lenbuf.get()[2])&0x000000ff) << 8;
	recvlen_ |= ((lenbuf.get()[1])&0x000000ff) << 16;
	recvlen_ |= ((lenbuf.get()[0])&0x000000ff) << 24;

	std::tr1::shared_ptr<char> recvbuf(new char[recvlen_]);
    memset(recvbuf.get(),sizeof(recvbuf), 0);

	asio::read(socket_,asio::buffer(recvbuf.get(),recvlen_),ec);
	if(ec)
	{
		result = -4;
		return;
	}

	read_more_body(recvbuf,recvlen_);
}

void ConnectToAppServer::read_more_body(std::tr1::shared_ptr<char> recvbuf,int recvlen_) 
{
	app::dispatch::Message message;
	std::string tmpstr;
	
	tmpstr = std::string(recvbuf.get(),recvlen_);
	recstr +=tmpstr;
	message.ParseFromString(tmpstr);
	//std::cout<<message.DebugString();
	if(message.sequence() == 0xffffffff)//notification message
	{
		handler_pushed_message();
	}
	else
	{
		if(!message.response().last_response())//后面还有数据需要接受
		{
			read_head();//read 
		}
	}
}
