#include "soapxiaofangService.h"
#include "xiaofang.nsmap"
#include "memory.hpp"
#include "task.hpp"
#include <iostream>
#include "protobuf/app.dispatch.pb.h"
#include <glog/logging.h>
#include <google/protobuf/message.h>
#include "connectToAppServer.h"
#include <iostream>
#include "md5.h"
#include "debug.h"

#define BACKLOG (100)	// Max. request backlog 

int main(int argc,char* argv[])
{
	xiaofangService service;
   	google::InitGoogleLogging("webservice");
	FLAGS_log_dir = "/mnt/hgfs/centos_share/xiaofang/src/log";
	LOG(INFO)<<"Service start\n";
   	task_pool_init();
   	if (argc < 2) // no args: assume this is a CGI application 
   	{ 
		LOG(INFO)<<"Run service as CGI application\n";
   		service.serve();
		service.destroy();
   	} 
   	else 
   	{ 
		void *process_request(std::tr1::shared_ptr<xiaofangService>); 
		//XiaoFangService* tservice;
		std::tr1::shared_ptr<xiaofangService> tservice;
		int port = atoi(argv[1]); // first command-line arg is port 
		SOAP_SOCKET m, s; 
		m=service.bind(NULL,port,BACKLOG);
		if (!soap_valid_socket(m)) 
		{	
			LOG(ERROR)<<"bind port to socket error\n";
		 	exit(1);
		}
		LOG(INFO)<<"Socket connection successful\n"; 
		for (;;) 
		{ 
			s = service.accept();
			if (!soap_valid_socket(s)) 
			{ 
			    if(service.errnum)
			    {
			    	debug();
					LOG(ERROR)<<"Server error:"<<service.errnum<<"\n";
					service.soap_stream_fault( std::cerr);
					exit(1);
				}
				LOG(ERROR)<<"Server timed out\n";
				debug();
			    break; 
			 } 
			 tservice =std::tr1::shared_ptr<xiaofangService>(service.copy());
			 debug();
			 if (!tservice.get()) 
			    break; 
			 debug();
			 auto pservice = [&tservice]()mutable{process_request(tservice);};
			 run_job(pservice);
		} 
	} 
   return 0; 
}

void process_request(std::tr1::shared_ptr<xiaofangService> tservice) 
{ 
	tservice->serve();
}

int xiaofangService::Dispatch_Login(std::string name,
			std::string passwd,
			ns__Login_Response &loginResponse)
{
	if(name.empty() || passwd.empty())
	{
		LOG(ERROR)<<"No name or password input\n";
		return SOAP_TYPE;
	}
	debug();
	app::dispatch::LoginRequest LocalLoginRequest;
	std::string md5passwd = get_md5(passwd);
	LocalLoginRequest.set_allocated_password(&md5passwd);
	LocalLoginRequest.set_allocated_username(&name);
	debug();

	app::dispatch::Request request;
	request.set_allocated_login(&LocalLoginRequest);
	debug();
	
	app::dispatch::Message message;
	message.set_msg_type(app::dispatch::MSG::Login_Request);
	message.set_sequence(1);
	message.set_allocated_request(&request);
	debug();
	
	
	std::string str;
	message.SerializeToString(&str);
	ConnectToAppServer connecttoserver;
	try{
		connecttoserver.transportData(str,message.ByteSize());
	}catch(...){
		return SOAP_ERR;
	}
	debug();
	return SOAP_OK;
}

int xiaofangService::Dispatch_Logout(std::string name, struct ns__LogOutResponse *out)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Keepalive(void *_, struct ns__Dispatch_Keepalive_Request_Response *out)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Entity_Request(ns__Entity id, ns__EntityData &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Entity_Nofitication(void *_, ns__Entity_Nofitication_Response &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Entity_Status_Notification(void *_, ns__Entity_Status_Notification &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Append_Group(ns__Group group, ns__Group &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Modify_Group(ns__Group group, ns__Group &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Modify_Participants(ns__Modify_Participant request, ns__Group &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Delete_Group(ns__Entity group_id, struct ns__Delete_Group_Response *out)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Dispatch_Participants_Notification(void *_, ns__Modify_Participant &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Join_Group_Request_Nofitication(void *_, ns__Join_Group_Request_Nofitication &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Participant_Status_Notification(void *_, ns__Participant_Status_Notification &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Media_Message_Request(ns__Entity id, unsigned long from_message_id, std::string from_time, unsigned long max_message_count, ns__Media_Message &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Media_Message_Notification(void *_, ns__Media_Message_Notification &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Participant_Connect_Request_Notification(void *_, ns__Participant_Connect_Request_Notification &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Participant_Speak_Request_Notification(void *_, ns__Participant_Connect_Request_Notification &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Invite_Participant_Request(ns__Entity group_id, ns__Entity account_id, struct ns__Dispatch_Invite_Participant_Request_Response *out)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Drop_Participant_Request(ns__Entity group_id, ns__Entity account_id, struct ns__Dispatch_Drop_Participant_Request_Response *out)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Release_Participant_Token_Request(ns__Entity group_id, ns__Entity account_id, struct ns__Dispatch_Release_Participant_Token_Request_Response *out)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Appoint_Participant_Speak_Request(ns__Entity group_id, ns__Entity account_id, struct ns__Dispatch_Appoint_Participant_Speak_Request_Response *out)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Jion_Group_Request(ns__Entity group_id, struct ns__Dispatch_Jion_Group_Request_Response *out)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Leave_Group_Request(ns__Entity group_id, struct ns__Dispatch_Leave_Group_Request_Response *out)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Session_Status_Notification(void *_, ns__Session_Status_Notification &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Send_Message_Request(unsigned long id, ns__MediaMessage message, struct ns__Dispatch_Send_Message_Request_Response *out)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Kick_Participant_Request(ns__Entity group_id, ns__Entity account_id, struct ns__Dispatch_Kick_Participant_Request_Response *out)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Start_Record_Request(ns__Entity group_id, struct ns__Dispatch_Start_Record_Request_Response *out)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Stop_Record_Request(ns__Entity group_id, struct ns__Dispatch_Stop_Record_Request_Response *out)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Record_Status_Notification(void *_, ns__Dispatch_Record_Status &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Subscribe_Account_Location_Request(bool subscribing, ns__Entity account_id, unsigned long ttl, struct ns__Dispatch_Subscribe_Account_Location_Request_Response *out)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Account_Location_Notification(void *_, ns__Dispatch_Account_Location_Notification_Response &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Append_Alert_Request(ns__Alert alert, std::list<ns__Account >acount, ns__Alert &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Modify_Alert_Request(ns__Alert alert, struct ns__Dispatch_Modify_Alert_Request_Response *out)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Stop_Alert_Request(ns__Entity alert_id, struct ns__Dispatch_Stop_Alert_Request_Response *out)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Alert_Overed_Notification(ns__Entity alert_id, struct ns__Dispatch_Alert_Overed_Notification_Response *out)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_History_Alert_Request(std::string name, std::string create_time_from, std::string create_time_to, std::string alram_time_from, std::string alram_time_to, std::string over_time_from, std::string over_time_to, std::list<ns__HistoryAlert >&response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Alert_Request(unsigned long alert_id, ns__Alert &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_History_Alert_Message_Request(unsigned long history_alert_id, unsigned long from_message_id, std::string from_time, unsigned long max_message_count, ns__Dispatch_History_Alert_Message &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Delete_History_Alert_Request(unsigned long history_alert_id, struct ns__Dispatch_Delete_History_Alert_Request_Response *out)
{
	return SOAP_OK;
}
