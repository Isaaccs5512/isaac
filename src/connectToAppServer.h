#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <glog/logging.h>
#include "memory.hpp"
#include "asio-1.10.1/include/asio.hpp"

#pragma once

#define SERVERIP "192.168.1.244"
#define SERVPORT 38881    /*·þÎñÆ÷¼àÌý¶Ë¿ÚºÅ */

class ConnectToAppServer
{
private:
	std::string str;
	std::string packData(std::string msg,int len);
	std::string recstr;
public:
	ConnectToAppServer():str(""),recstr(""){}
	int transportData(std::string msg,int len);
	std::string get_recstr();
};
