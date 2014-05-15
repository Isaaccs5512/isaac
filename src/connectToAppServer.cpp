#include "connectToAppServer.h"
#include <memory>
#include <iostream>
#include <exception>
#include <string>
#include <array>
#include "boost_1_55_0/boost/date_time/posix_time/posix_time.hpp"
#include "protobuf/app.dispatch.pb.h"


#define TIME_OUT 5

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

void ConnectToAppServer::handler_timerout_error()
{	
	std::cout<<"1111111111111"<<std::endl;
	result_ = -5;
	if(!flag_.expired())
		flag_shared_->store(true);
	if(!cv_.expired())
		cv_shared_->notify_one();
	socket_->cancel();
}

void ConnectToAppServer::start()
{
	asio::ip::tcp::endpoint end_point(asio::ip::address::from_string(SERVERIP), SERVPORT);
	if(socket_->is_open())
	{
		write();
	}
	else
	{
		socket_->async_connect(end_point,
			[this](asio::error_code ec){
			if(!ec)
			{
				try{
					timer_start.cancel();
				}catch(asio::system_error se)
				{
					LOG(ERROR)<<"cancel timer error";
				}
				write();
			}
			else
			{
				try{
					timer_start.cancel();
				}catch(asio::system_error se)
				{
					LOG(ERROR)<<"cancel timer error";
				}
				result_ = -1;
				if(!flag_.expired())
					flag_shared_->store(true);
				if(!cv_.expired())
					cv_shared_->notify_one();
			}
		});
		timer_start.expires_from_now(boost::posix_time::seconds(TIME_OUT));
		timer_start.async_wait([this](asio::error_code ec){
			if(ec!=asio::error::operation_aborted)
			{
				handler_timerout_error();
			}
		});
	}
} 

void ConnectToAppServer::write()
{
	packData(sendstr_,sendlen_);
	asio::async_write(*socket_.get(),
        asio::buffer(str_,sendlen_+4),
        [this](asio::error_code ec,std::size_t length){
			if(!ec)
			{
				try{
					timer_write.cancel();
				}catch(asio::system_error se)
				{
					LOG(ERROR)<<"cancel timer error";
				}
				read_head();
			}
			else
			{
				try{
					timer_write.cancel();
				}catch(asio::system_error se)
				{
					LOG(ERROR)<<"cancel timer error";
				}
				result_ = -2;
				if(!flag_.expired())
					flag_shared_->store(true);
				if(!cv_.expired())
					cv_shared_->notify_one();
			}
	});
	timer_write.expires_from_now(boost::posix_time::seconds(TIME_OUT));
		timer_write.async_wait([this](asio::error_code ec){
			if(ec!=asio::error::operation_aborted)
			{
				handler_timerout_error();
			}
		});
} 

void ConnectToAppServer::read_head() 
{
	std::tr1::shared_ptr<char> tmplenbuf(new char[4]);
	lenbuf = tmplenbuf;
    memset(lenbuf.get(),sizeof(lenbuf), 0);
	asio::async_read(*socket_.get(),
        asio::buffer(lenbuf.get(),4),
        [this](asio::error_code ec,std::size_t length){
			if(!ec)
			{
				try{
					timer_read_head.cancel();
				}catch(asio::system_error se)
				{
					LOG(ERROR)<<"cancel timer error";
				}
				read_body();
			}
			else
			{
				try{
					timer_read_head.cancel();
				}catch(asio::system_error se)
				{
					LOG(ERROR)<<"cancel timer error";
				}
				result_ = -3;
				if(!flag_.expired())
					flag_shared_->store(true);
				if(!cv_.expired())
					cv_shared_->notify_one();
			}
	});
	timer_read_head.expires_from_now(boost::posix_time::seconds(TIME_OUT));
		timer_read_head.async_wait([this](asio::error_code ec){
			if((ec!=asio::error::operation_aborted) && (ec.value()!=0))
			{
				LOG(ERROR)<<ec.message();
				std::cout<<"444444444444"<<std::endl;
				handler_timerout_error();
			}
		});
}

void ConnectToAppServer::read_body() 
{
	recvlen_ = 0;
	recvlen_ |= ((lenbuf.get()[3])&0x000000ff);
	recvlen_ |= ((lenbuf.get()[2])&0x000000ff) << 8;
	recvlen_ |= ((lenbuf.get()[1])&0x000000ff) << 16;
	recvlen_ |= ((lenbuf.get()[0])&0x000000ff) << 24;

	std::tr1::shared_ptr<char> tmprecvbuf(new char[recvlen_]);
	recvbuf = tmprecvbuf;
    memset(recvbuf.get(),sizeof(recvbuf), 0);

	asio::async_read(*socket_.get(),
        asio::buffer(recvbuf.get(),recvlen_),
        [this](asio::error_code ec,std::size_t length){
			if(!ec)
			{
				try{
					timer_read_body.cancel();
				}catch(asio::system_error se)
				{
					LOG(ERROR)<<"cancel timer error";
				}
				read_more_body();
			}
			else
			{
				try{
					timer_read_body.cancel();
				}catch(asio::system_error se)
				{
					LOG(ERROR)<<"cancel timer error";
				}
				result_ = -4;
				if(!flag_.expired())
					flag_shared_->store(true);
				if(!cv_.expired())
					cv_shared_->notify_one();
			}
	});
	timer_read_body.expires_from_now(boost::posix_time::seconds(TIME_OUT));
		timer_read_body.async_wait([this](asio::error_code ec){
			if(ec!=asio::error::operation_aborted)
			{
				handler_timerout_error();
			}
		});
}

void ConnectToAppServer::read_more_body() 
{
	app::dispatch::Message message;
	std::string tmpstr;
	
	tmpstr = std::string(recvbuf.get(),recvlen_);
	recstr +=tmpstr;
	message.ParseFromString(tmpstr);
	std::cout<<message.DebugString();
	if(!message.response().last_response())//后面还有数据需要接受
	{
		read_head();
	}
	else
	{
		if(!flag_.expired())
			flag_shared_->store(true);
		if(!cv_.expired())
			cv_shared_->notify_one();
	}
}

void ConnectToAppServer::read_head_notification() 
{
	std::tr1::shared_ptr<char> tmplenbuf(new char[4]);
	lenbuf = tmplenbuf;
    memset(lenbuf.get(),sizeof(lenbuf), 0);
	asio::async_read(*socket_.get(),
        asio::buffer(lenbuf.get(),4),
        [this](asio::error_code ec,std::size_t length){
			if(!ec)
			{
				try{
					timer_read_head_notification.cancel();
				}catch(asio::system_error se)
				{
					LOG(ERROR)<<"cancel timer error";
				}
				read_body_notification();
			}
			else
			{
				try{
					timer_read_head_notification.cancel();
				}catch(asio::system_error se)
				{
					LOG(ERROR)<<"cancel timer error";
				}
				result_ = -3;
				if(!flag_.expired())
					flag_shared_->store(true);
				if(!cv_.expired())
					cv_shared_->notify_one();
			}
	});
	timer_read_head_notification.expires_from_now(boost::posix_time::seconds(TIME_OUT));
		timer_read_head_notification.async_wait([this](asio::error_code ec){
			if(ec!=asio::error::operation_aborted)
			{
				LOG(ERROR)<<ec.message();
				std::cout<<"444444444444"<<std::endl;
				handler_timerout_error();
			}
		});
}

void ConnectToAppServer::read_body_notification() 
{
	std::cout<<"vvvvvvvvvvvvvvvv"<<std::endl;
	recvlen_ = 0;
	recvlen_ |= ((lenbuf.get()[3])&0x000000ff);
	recvlen_ |= ((lenbuf.get()[2])&0x000000ff) << 8;
	recvlen_ |= ((lenbuf.get()[1])&0x000000ff) << 16;
	recvlen_ |= ((lenbuf.get()[0])&0x000000ff) << 24;

	std::tr1::shared_ptr<char> tmprecvbuf(new char[recvlen_]);
	recvbuf = tmprecvbuf;
    memset(recvbuf.get(),sizeof(recvbuf), 0);

	asio::async_read(*socket_.get(),
        asio::buffer(recvbuf.get(),recvlen_),
        [this](asio::error_code ec,std::size_t length){
			if(!ec)
			{
				try{
					timer_read_body_notification.cancel();
				}catch(asio::system_error se)
				{
					LOG(ERROR)<<"cancel timer error";
				}
				recstr = std::string(recvbuf.get(),recvlen_);
			}
			else
			{
				try{
					timer_read_body_notification.cancel();
				}catch(asio::system_error se)
				{
					LOG(ERROR)<<"cancel timer error";
				}
				result_ = -4;
				if(!flag_.expired())
					flag_shared_->store(true);
				if(!cv_.expired())
					cv_shared_->notify_one();
			}
	});
	timer_read_body_notification.expires_from_now(boost::posix_time::seconds(TIME_OUT));
		timer_read_body_notification.async_wait([this](asio::error_code ec){
			if(ec!=asio::error::operation_aborted)
			{
				std::cout<<"55555555555"<<std::endl;
				handler_timerout_error();
			}
		});
}

