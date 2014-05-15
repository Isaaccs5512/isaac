#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <glog/logging.h>
#include "memory.hpp"
#include "asio-1.10.1/include/asio.hpp"
#include "boost_1_55_0/boost/bind.hpp"
#include <condition_variable>
#include <atomic>


#pragma once

#define SERVERIP "192.168.0.43"
#define SERVPORT 38881    /*服务器监听端口号 */

class ConnectToAppServer{
public:
    ConnectToAppServer(asio::io_service &io_service,
		std::tr1::shared_ptr<asio::ip::tcp::socket> socket,
		std::string msg,int len,
		std::tr1::weak_ptr<std::condition_variable> cv,
		std::tr1::weak_ptr<std::atomic<bool> > flag)
        : socket_(socket),
        recvlen_(0),
        sendstr_(msg),
        sendlen_(len),
        recstr(""),
        result_(0),
        timer_write(io_service),
        timer_read_head(io_service),
        timer_read_body(io_service),
        timer_start(io_service),
        cv_(cv),
        flag_(flag)
    { 
    	//socket_=socket;
    	cv_shared_ = cv_.lock();
		flag_shared_ = flag_.lock();
    } 

    void start(); 
	std::string get_recstr();
	int get_result();
private:
    void write(); 

    void read_head();

    void read_body();

    void read_more_body();
	
	std::string packData(std::string msg,int len);

	void read_more();

	void write_with_tcp();

	void handler_timerout_error(const asio::error_code &error);

private:
    std::tr1::shared_ptr<asio::ip::tcp::socket> socket_;
	std::string sendstr_;//传递进来的需要发送的数据
	int sendlen_;//sendstr的长度
	std::string str_;//sendstr经过重新封装后的数据
	int recvlen_;
	std::string recstr;//返回接受到的数据
	int result_;
	std::tr1::shared_ptr<char> lenbuf;
	std::tr1::shared_ptr<char> recvbuf;
	asio::deadline_timer timer_start;
	asio::deadline_timer timer_read_head;
	asio::deadline_timer timer_read_body;
	asio::deadline_timer timer_write;
    std::tr1::weak_ptr<std::condition_variable> cv_;
	std::tr1::weak_ptr<std::atomic<bool> > flag_;
	std::tr1::shared_ptr<std::condition_variable> cv_shared_;
	std::tr1::shared_ptr<std::atomic<bool> > flag_shared_;
};
