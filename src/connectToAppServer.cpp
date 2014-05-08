#include "connectToAppServer.h"
#include <memory>
#include <iostream>
#include <exception>
#include <string>
#include <array>
#include "boost_1_55_0/boost/date_time/posix_time/posix_time.hpp"
#define TIME_OUT 5

void handler_timerout_error(asio::ip::tcp::socket &socket,const asio::error_code &error)
{	
	if(error != asio::error::operation_aborted)
	{
		LOG(ERROR)<<error.message();
		socket.close();
	}
}

std::string ConnectToAppServer::get_recstr()
{
	return recstr;
}

int ConnectToAppServer::get_result()
{
	return result_;
}

std::string ConnectToAppServer::packData(std::string msg,int len)
{
	std::tr1::shared_ptr<char> buf(new char[len+4]);
	buf.get()[0] = (len & 0xff000000) >> 24;
	buf.get()[1] = (len & 0x00ff0000) >> 16;
	buf.get()[2] = (len & 0x0000ff00) >> 8;
	buf.get()[3] = (len & 0x000000ff);

	memcpy(buf.get()+4,msg.c_str(),len);
	str_ = std::string(buf.get(),len+4);
	return str_;
}

void ConnectToAppServer::start()
{
	timer->expires_from_now(boost::posix_time::seconds(TIME_OUT));
	asio::ip::tcp::endpoint end_point(asio::ip::address::from_string(SERVERIP), SERVPORT);
    socket_.async_connect(end_point,
            boost::bind(&ConnectToAppServer::handle_connect,
                this,
                asio::placeholders::error));
	timer->async_wait(boost::bind(&handler_timerout_error,std::ref(socket_),asio::placeholders::error));
} 

void ConnectToAppServer::handle_connect(const asio::error_code &error)
{
	//timer->cancel();
	timer->expires_from_now(boost::posix_time::seconds(TIME_OUT));
    if (error){
		LOG(ERROR)<<error.message();
        socket_.close();
		result_ = -1;
        return;
    } 
	packData(sendstr_,sendlen_);
    asio::async_write(socket_,
        asio::buffer(str_,sendlen_+4),
        boost::bind(&ConnectToAppServer::handle_write,
            this,
            asio::placeholders::error));
	timer->async_wait(boost::bind(&handler_timerout_error,std::ref(socket_),asio::placeholders::error));
} 

void ConnectToAppServer::handle_write(const asio::error_code&error) 
{
	//timer->cancel();
	timer->expires_from_now(boost::posix_time::seconds(TIME_OUT));
    if (error){
		LOG(ERROR)<<error.message();
        socket_.close();
		result_ = -2;
        return;
    } 
	std::tr1::shared_ptr<char> tmplenbuf(new char[4]);
	lenbuf = tmplenbuf;
    memset(lenbuf.get(),sizeof(lenbuf), 0);
    asio::async_read(socket_,
        asio::buffer(lenbuf.get(),4),
        boost::bind(&ConnectToAppServer::handle_read_len,
            this,
            asio::placeholders::error));
	timer->async_wait(boost::bind(&handler_timerout_error,std::ref(socket_),asio::placeholders::error));
}

void ConnectToAppServer::handle_read_len(const asio::error_code&error) 
{
	//timer->cancel();
	timer->expires_from_now(boost::posix_time::seconds(TIME_OUT));
    if (error){
		LOG(ERROR)<<error.message();
        socket_.close();
		result_ = -3;
        return;
    } 
	recvlen_ |= ((lenbuf.get()[3])&0x000000ff);
	recvlen_ |= ((lenbuf.get()[2])&0x000000ff) << 8;
	recvlen_ |= ((lenbuf.get()[1])&0x000000ff) << 16;
	recvlen_ |= ((lenbuf.get()[0])&0x000000ff) << 24;
	
	std::tr1::shared_ptr<char> tmprecvbuf(new char[recvlen_]);
	recvbuf = tmprecvbuf;
    memset(recvbuf.get(),sizeof(recvbuf), 0);
    asio::async_read(socket_,
        asio::buffer(recvbuf.get(),recvlen_),
        boost::bind(&ConnectToAppServer::handle_read_data,
            this,
            asio::placeholders::error));
	timer->async_wait(boost::bind(&handler_timerout_error,std::ref(socket_),asio::placeholders::error));
}

void ConnectToAppServer::handle_read_data(const asio::error_code&error) 
{
	timer->cancel();
	//timer->expires_from_now(boost::posix_time::seconds(TIME_OUT));
	//timer->async_wait(boost::bind(&handler_timerout_error,std::ref(socket_)));
    if (error){
		LOG(ERROR)<<error.message();
        socket_.close();
		result_ = -4;
        return;
    } 
	recstr = std::string(recvbuf.get(),recvlen_);
}
