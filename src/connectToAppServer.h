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
#define SERVPORT 38881    /*�����������˿ں� */

extern void handler_pushed_message();
extern int Get_Keepalive_Request_str(const unsigned long session_id,std::string *out_str);


class ConnectToAppServer{
public:
    ConnectToAppServer();

	~ConnectToAppServer();

	std::string get_response();

	void send_request(std::string requeststr);

	void set_cancel_thread();

	void keep_alive(const unsigned long session_id);
private:
    void connect(); 

    void read_head();

    void read_body(std::tr1::shared_ptr<char> lenbuf);

    void read_more_body(std::tr1::shared_ptr<char> recvbuf,int recvlen_) ;
	
	std::string packData(std::string msg,int len);
private:
	bool cancel_thread;
	asio::io_service io_service;
    asio::ip::tcp::socket socket_;
	std::string recstr;
};
