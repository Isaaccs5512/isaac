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
#include <queue>
#include <map>


#pragma once

#define SERVERIP "192.168.0.43"
#define SERVPORT 38881    /*·þÎñÆ÷¼àÌý¶Ë¿ÚºÅ */

class ConnectToAppServer{
public:
    ConnectToAppServer();

	std::string get_response(unsigned long sequence);

	std::string get_notification();

	void send_request(std::string requeststr);

private:
    void connect(); 

    void write(); 

    void read_head();

    void read_body(std::tr1::shared_ptr<char> lenbuf);

    void read_more_body(std::tr1::shared_ptr<char> recvbuf,int recvlen_) ;
	
	std::string packData(std::string msg,int len);

	void handler_timerout_error();

private:
	asio::io_service io_service_;
    asio::ip::tcp::socket socket_;
	std::string recstr;
	asio::deadline_timer timer_connect;
	asio::deadline_timer timer_read_head;
	asio::deadline_timer timer_read_body;
	asio::deadline_timer timer_write;
	std::map<unsigned long,std::string> response_message_map;
	std::queue<std::string> notification_message_queue;
	std::queue<std::string> request_queue;
};
