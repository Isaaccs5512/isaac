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

#pragma once

#define SERVERIP "192.168.0.43"
#define SERVPORT 38881    /*�����������˿ں� */


class ConnectToAppServer{
public:
    ConnectToAppServer(asio::io_service&io_service,std::string msg,int len)
        : socket_(io_service),recvlen_(0),sendstr_(msg),sendlen_(len)
    { 
    	timer = new asio::deadline_timer(io_service, boost::posix_time::seconds(5));
    } 

    void start(); 
	std::string get_recstr();
	int get_result();
private:
    void handle_connect(const asio::error_code &error); 

    void handle_write(const asio::error_code&error);

    void handle_read_len(const asio::error_code&error);

    void handle_read_data(const asio::error_code&error);
	
	std::string packData(std::string msg,int len);

private:
    asio::ip::tcp::socket socket_;
	std::string sendstr_;//���ݽ�������Ҫ���͵�����
	int sendlen_;//sendstr�ĳ���
	std::string str_;//sendstr�������·�װ�������
	int recvlen_;
	std::string recstr;//���ؽ��ܵ�������
	int result_;
	std::tr1::shared_ptr<char> lenbuf;
	std::tr1::shared_ptr<char> recvbuf;
	asio::deadline_timer *timer;
};
