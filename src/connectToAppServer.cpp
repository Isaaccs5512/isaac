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

#define TIME_OUT 5

std::string ConnectToAppServer::get_response(unsigned long sequence)
{
	std::string response_str;
	if(response_message_map.empty())
	{
		return std::string("");
	}
	else
	{
		std::map<unsigned long,std::string>::iterator itor_tcp = response_message_map.find(sequence);
		response_str=itor_tcp->second;
		response_message_map.erase(itor_tcp);
		return response_str;
	}
}

void ConnectToAppServer::set_cancel_thread()
{
	cancel_thread = true;
}

ConnectToAppServer::ConnectToAppServer():socket_(io_service),timer_read_body(io_service),
										timer_read_head(io_service),cancel_thread(false)
{
	connect();

	run_job(
	[this](){
		while(!cancel_thread)
		{
			write();
		}
	});

	run_job(
	[this](){
		while(!cancel_thread)
		{
			read_head();
		}
	});
}

std::string ConnectToAppServer::get_notification()
{
	std::string notification_str;
	if(notification_message_queue.empty())
	{
		return std::string("");
	}
	else
	{
		notification_str = notification_message_queue.front();
		notification_message_queue.pop();
		return notification_str;
	}
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

void ConnectToAppServer::connect()
{
	asio::ip::tcp::endpoint end_point(asio::ip::address::from_string(SERVERIP), SERVPORT);
	asio::error_code ec;

	socket_.connect(end_point,ec);
} 
void ConnectToAppServer::send_request(std::string requeststr)
{
	request_queue.push(packData(requeststr,requeststr.length()));
}

void ConnectToAppServer::write()
{
	while(!request_queue.empty())
	{
		std::string send_str = request_queue.front();

		asio::write(socket_,asio::buffer(send_str,send_str.length()));
		request_queue.pop();
	}
} 

void ConnectToAppServer::read_head() 
{
	std::tr1::shared_ptr<char> lenbuf(new char[4]);
    memset(lenbuf.get(),sizeof(lenbuf), 0);

	timer_read_head.expires_from_now(boost::posix_time::seconds(5));
	timer_read_head.async_wait([this](const asio::error_code& error){
		if(error != asio::error::operation_aborted)
		{
			LOG(ERROR)<<"read head fail";
			try
			{
				socket_.cancel();
			}
			catch(asio::system_error se)
			{
				LOG(ERROR)<<se.what();
			}
		}
	});
	asio::read(socket_,asio::buffer(lenbuf.get(),4));
	try{
		timer_read_head.cancel();
	}
	catch(asio::error_code ec)
	{
		LOG(ERROR)<<ec.message();
	}
	read_body(lenbuf);
}

void ConnectToAppServer::read_body(std::tr1::shared_ptr<char> lenbuf) 
{
	int recvlen_ = 0;
	recvlen_ |= ((lenbuf.get()[3])&0x000000ff);
	recvlen_ |= ((lenbuf.get()[2])&0x000000ff) << 8;
	recvlen_ |= ((lenbuf.get()[1])&0x000000ff) << 16;
	recvlen_ |= ((lenbuf.get()[0])&0x000000ff) << 24;

	std::tr1::shared_ptr<char> recvbuf(new char[recvlen_]);
    memset(recvbuf.get(),sizeof(recvbuf), 0);

	timer_read_body.expires_from_now(boost::posix_time::seconds(5));
	timer_read_body.async_wait([this](const asio::error_code& error){
		if(error != asio::error::operation_aborted)
		{
			LOG(ERROR)<<"read head fail";
			try
			{
				socket_.cancel();
			}
			catch(asio::system_error se)
			{
				LOG(ERROR)<<se.what();
			}
		}
	});
	asio::read(socket_,asio::buffer(recvbuf.get(),recvlen_));
	try{
		timer_read_body.cancel();
	}
	catch(asio::error_code ec)
	{
		LOG(ERROR)<<ec.message();
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
		notification_message_queue.push(recstr);
	}
	else
	{
		if(!message.response().last_response())//后面还有数据需要接受
		{
			read_head();//read 
		}
		else
		{
			response_message_map.insert(std::pair<unsigned long,std::string>(message.sequence(),recstr));
		}
	}

}
