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
#include "cstdlib"
#include "timer.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>
#include <set>

#define BACKLOG (100000)	// Max. request backlog 
static unsigned long sequence = 1;
std::mutex  keep_tcp_connection_mutex;
static std::map<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> > keep_tcp_connection;
thread_local asio::io_service io_service;

unsigned long get_sequence()
{
	sequence = sequence++%0xffffffff;
	return sequence;
}

std::string int2str(int n)
{
    std::stringstream ss;
    std::string s;
    ss << n;
    ss >> s;
 
    return s;
}

int Get_Keepalive_Request_str(const unsigned long session_id,std::string *out_str)
{
	
	//LOG(INFO)<<"Keepalive_Request";
	
	app::dispatch::Message message;
	message.set_msg_type(app::dispatch::MSG::Keepalive_Request);
	message.set_session_id(session_id);
	unsigned long sequence_keepalive = get_sequence();
	message.set_sequence(sequence_keepalive);
	
	if(!message.IsInitialized())
	{		
		return 1;
	}
	if(!message.SerializeToString(out_str))
	{
		return 1;
	}
	return 0;
}


void handler_pushed_message()
{
	
}


int main(int argc,char* argv[])
{
	xiaofangService service;
	timer_pool_init([](){},10);
	//create_timer(handler_keep_alive,8,true);
	//create_timer(handler_pushed_message,8, true);
   //	google::InitGoogleLogging("webservice");
	FLAGS_log_dir = "/mnt/hgfs/centos_share/xiaofang/src/log";
	LOG(INFO)<<"Service start\n";
	task_pool_init();
   	if (argc < 2) // no args: assume this is a CGI application 
   	{ 
		LOG(INFO)<<"No IP address input\n";
   		return -1;
   	} 
   	else 
   	{ 
		service.send_timeout = 60; // 60 seconds 
		service.recv_timeout = 60; // 60 seconds 
		//service.accept_timeout = 3600; // server stops after 1 hour of inactivity 
		service.max_keep_alive = 3600; // max keep-alive sequence 

	  	void *process_request(std::tr1::shared_ptr<xiaofangService>&); 
		std::tr1::shared_ptr<xiaofangService> tservice;
		int port = atoi(argv[1]); // first command-line arg is port 
		SOAP_SOCKET m, s;
		service.bind_flags = SO_REUSEADDR;
		m=service.bind(NULL,port,BACKLOG);
		if (!soap_valid_socket(m)) 
		{	
			LOG(ERROR)<<"bind port to socket error\n";
		 	exit(1);
		}
		run_job(
		[](){
			asio::io_service::work work(io_service);
			try{
				io_service.run();
			}
			catch(asio::system_error &error)
			{
				std::exception e;
				throw e;
			}
		});

		
		for (;;) 
		{ 
			s = service.accept();
			if (!soap_valid_socket(s)) 
			{ 
			    if(service.errnum)
			    {
					LOG(ERROR)<<"Server error:"<<service.errnum<<"\n";
					service.soap_stream_fault( std::cerr);
					exit(1);
				}
				LOG(ERROR)<<"Server timed out\n";
			    break; 
			 } 
			 tservice =std::tr1::shared_ptr<xiaofangService>(service.copy());
			 if (!tservice.get()) 
			    break; 
			 auto pservice = [tservice]()mutable{process_request(tservice);};
			 run_job(pservice);
		} 
	} 
   return 0; 
}

void *process_request(std::tr1::shared_ptr<xiaofangService> & tservice) 
{ 
	tservice->serve();	
	tservice->destroy();
	return NULL;
}

int xiaofangService::Dispatch_Login(std::string name,
			std::string passwd,
			ns__Dispatch_Login_Response &response)
{
	LOG(INFO)<<"Dispatch login";
	response.result = true;
	if(name.empty() || passwd.empty())
	{
		response.result = false;
		response.error_describe = "No name or password input";
		return SOAP_OK;
	}
	LOG(INFO)<<"Serializa data to protobuf protocol";
	std::string md5passwd = get_md5(passwd);	
	
	app::dispatch::Message message;
	unsigned long sequence_login = get_sequence();
	message.set_sequence(sequence_login);
	message.set_msg_type(app::dispatch::MSG::Login_Request);
	message.mutable_request()->mutable_login()->set_username(name);
	message.mutable_request()->mutable_login()->set_password(md5passwd);
	std::string send_str;
	std::string recev_str;
	if(!message.IsInitialized())
	{
		response.result = false;
		response.error_describe = "Message can't be initialized";
		return SOAP_OK;
	}
	if(!message.SerializeToString(&send_str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	std::tr1::shared_ptr<ConnectToAppServer> connecttoserver_ptr(new ConnectToAppServer(io_service));
	
	if(connecttoserver_ptr->get_result() == -5)
	{
		LOG(ERROR)<<"Time out config error";
		response.result = false;
		response.error_describe = "Time out config error";
		return SOAP_OK;
	}
	else if(connecttoserver_ptr->get_result() == -1)
	{
		LOG(ERROR)<<"Connect refulsed";
		response.result = false;
		response.error_describe = "Connect refulsed";
		return SOAP_OK;
	}

	
	connecttoserver_ptr->send_request(send_str);

	LOG(INFO)<<"Parse data from protobuf protocol";
	message.Clear();


	bool break_while = false;

	create_timer([&break_while](){break_while=true;}, 5, false);
	do{
		recev_str = connecttoserver_ptr->get_response();
		if(connecttoserver_ptr->get_result() != 0)
			break_while = true;
	}while((recev_str=="") && (!break_while));
	//}while((!break_while));
	
	if(break_while)
	{
		LOG(ERROR)<<"socket write and read error";
		response.result = false;
		response.error_describe = "socket write and read error";
		return SOAP_OK;
	}

	
	if(message.ParseFromString(recev_str))
	{	
		if(message.response().result())
		{
			response.result = true;
			response.session_id = int2str(message.response().login().session_id());
			response.id = int2str(message.response().login().self().account().id());
			response.parentid = int2str(message.response().login().self().account().parent().id());
			response.ttl = int2str(message.response().login().ttl());
		}
		else
		{
			LOG(ERROR)<<message.response().error_describe();
			response.result = false;
			response.error_describe = message.response().error_describe();
			return SOAP_OK;
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}

	connecttoserver_ptr->keep_alive(message.response().login().session_id());
	std::lock_guard<std::mutex> lock(keep_tcp_connection_mutex);
	keep_tcp_connection.insert(std::pair<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> >(std::stoul(response.session_id,nullptr,0),connecttoserver_ptr));	
	return SOAP_OK;
}

int xiaofangService::Dispatch_Logout(std::string session_id,
									ns__Normal_Response &response)
{
	
	LOG(INFO)<<"Dispatch logout";
	
	LOG(INFO)<<"Serializa data to protobuf protocol";
	app::dispatch::Message message;
	message.set_msg_type(app::dispatch::MSG::Logout_Request);
	message.set_session_id(std::stoul(session_id,nullptr,0));
	unsigned long sequence_logout = get_sequence();
	message.set_sequence(sequence_logout);
	
	std::string send_str;
	std::string recev_str;
	if(!message.IsInitialized())
	{
		response.result = false;
		response.error_describe = "Message can't be initialized";
		return SOAP_OK;
	}
	if(!message.SerializeToString(&send_str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}

	std::lock_guard<std::mutex> lock(keep_tcp_connection_mutex);
	std::map<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> >::iterator itr = keep_tcp_connection.find(std::stoul(session_id,nullptr,0));
	if(itr == keep_tcp_connection.end())
	{
		response.result = false;
		response.error_describe = "session id not exist";
		return SOAP_OK;
	}
	std::tr1::shared_ptr<ConnectToAppServer> connecttoserver_ptr = itr->second;
	connecttoserver_ptr->send_request(send_str);
	message.Clear();

	bool break_while = false;
	create_timer([&break_while](){break_while=true;}, 5, false);
	do{
		recev_str = connecttoserver_ptr->get_response();
	}while((recev_str=="") && (!break_while));
	if(break_while)
	{
		LOG(ERROR)<<"socket write and read error";
		
		response.result = false;
		response.error_describe = "socket write and read error";
		return SOAP_OK;
	}
	keep_tcp_connection.erase(itr);
		
	LOG(INFO)<<"Parse data from protobuf protocol";
	if (message.ParseFromString(recev_str))
	{	
		if(message.response().result())
		{
			response.session_id = int2str(message.session_id());
			response.result = true;
		}
		else
		{
			LOG(ERROR)<<message.response().error_describe();
			response.result = false;
			response.error_describe = message.response().error_describe();
			return SOAP_OK;
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	
	return SOAP_OK;
}


int xiaofangService::Dispatch_Entity_Request(std::string session_id,
											std::string id, 
											ns__Dispatch_Entity_Request_Response& response)
{
	
	LOG(INFO)<<"Dispatch_Entity_Request";
	app::dispatch::Message message;
	message.set_msg_type(app::dispatch::MSG::Entity_Request);
	message.set_session_id(std::stoul(session_id,nullptr,0));
	if(id.empty())
	{
		response.result = false;
		response.error_describe  = "id is empty";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_entity()->mutable_id()->set_id(std::stoul(id,nullptr,0));
	unsigned long sequence_entity_request = get_sequence();
	message.set_sequence(sequence_entity_request);
	std::string send_str;
	std::string recev_str;
	if(!message.IsInitialized())
	{
		response.result = false;
		response.error_describe = "Message can't be initialized";
		return SOAP_OK;
	}
	if(!message.SerializeToString(&send_str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	{
		std::lock_guard<std::mutex> lock(keep_tcp_connection_mutex);
		std::map<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> >::iterator itr = keep_tcp_connection.find(std::stoul(session_id,nullptr,0));
		if(itr == keep_tcp_connection.end())
		{
			response.result = false;
			response.error_describe = "session id not exist";
			return SOAP_OK;
		}
		std::tr1::shared_ptr<ConnectToAppServer> connecttoserver_ptr = itr->second;

		connecttoserver_ptr->send_request(send_str);

		message.Clear();

		bool break_while = false;
	create_timer([&break_while](){break_while=true;}, 5, false);
		
		do{
			recev_str = connecttoserver_ptr->get_response();
		}while((recev_str=="") && (!break_while));

	if(break_while)
	{
		LOG(ERROR)<<"socket write and read error";
		
		response.result = false;
		response.error_describe = "socket write and read error";
		return SOAP_OK;
	}
	}//unlock keep_tcp_connection_mutex
	LOG(INFO)<<"Parse data from protobuf protocol";
	if (message.ParseFromString(recev_str))
	{	
		if(message.response().result())
		{
			response.result = true;
			response.data.id.entity_type = int2str(message.response().entity().data().id().entity_type());
			response.data.id.id = int2str(message.response().entity().data().id().id());
			response.data.id.name = message.response().entity().data().id().name();
			response.session_id = int2str(message.session_id());
			if((ns__EntityType)message.response().entity().data().id().entity_type() == UNIT)					
			{
				response.data.unit.base.entity_type = int2str(message.response().entity().data().unit().base().entity_type());
				response.data.unit.base.id = int2str(message.response().entity().data().unit().base().id());
				response.data.unit.base.name = message.response().entity().data().unit().base().name();
				response.data.unit.base.parentid = int2str(message.response().entity().data().unit().base().parent().id());

				ns__Entity member;
				response.data.unit.size = int2str(message.response().entity().data().unit().members_size());
				for(int i=0;i<message.response().entity().data().unit().members_size();i++)
				{
					member.entity_type = int2str(message.response().entity().data().unit().members(i).entity_type());
					member.id = int2str(message.response().entity().data().unit().members(i).id());
					member.name = message.response().entity().data().unit().members(i).name();
					member.parentid = int2str(message.response().entity().data().unit().members(i).parent().id());
					response.data.unit.members.push_back(member);
				}
			}
			else if((ns__EntityType)message.response().entity().data().id().entity_type() == ACCOUNT)
			{
				response.data.account.base.entity_type = int2str(message.response().entity().data().account().base().entity_type());
				response.data.account.base.id = int2str(message.response().entity().data().account().base().id());
				response.data.account.base.name = message.response().entity().data().account().base().name();
				response.data.account.base.parentid = int2str(message.response().entity().data().account().base().parent().id());
				response.data.account.number = message.response().entity().data().account().number();
				response.data.account.priority = int2str(message.response().entity().data().account().priority());
				response.data.account.short_number = message.response().entity().data().account().short_number();
				response.data.account.sip_status = int2str(message.response().entity().data().account().sip_status());
				response.data.account.register_tatus = int2str(message.response().entity().data().account().status());
				response.data.account.token_privilege = int2str(message.response().entity().data().account().token_privilege());
				response.data.account.call_privilege = int2str(message.response().entity().data().account().call_privilege());
			}
			else if((ns__EntityType)message.response().entity().data().id().entity_type() == USER)
			{
				response.data.user.account.entity_type = int2str(message.response().entity().data().user().account().entity_type());
				response.data.user.account.id = int2str(message.response().entity().data().user().account().id());
				response.data.user.account.name = message.response().entity().data().user().account().name();
				response.data.user.account.parentid = int2str(message.response().entity().data().user().account().parent().id());
				response.data.user.base.entity_type = int2str(message.response().entity().data().user().base().entity_type());
				response.data.user.base.id = int2str(message.response().entity().data().user().base().id());
				response.data.user.base.name = message.response().entity().data().user().base().name();
				response.data.user.base.parentid = int2str(message.response().entity().data().user().base().parent().id());
				response.data.user.register_tatus = int2str(message.response().entity().data().user().status());
			}
			else if((ns__EntityType)message.response().entity().data().id().entity_type() == GROUPW)
			{
				ns__Participant participant;
				response.data.group.id = int2str(message.response().entity().data().group().base().id());
				response.data.group.name = message.response().entity().data().group().base().name();
				response.data.group.number = message.response().entity().data().group().number();
				response.data.group.owner_id = int2str(message.response().entity().data().group().owner().id());
				response.data.group.parent_id = int2str(message.response().entity().data().group().base().parent().id());
				response.data.group.size = int2str(message.response().entity().data().group().participants_size());
				for(int i=0;i<message.response().entity().data().group().participants_size();i++)
				{
					participant.id = int2str(message.response().entity().data().group().participants(i).account().id());
					participant.name = message.response().entity().data().group().participants(i).account().name();
					participant.parentid = int2str(message.response().entity().data().group().participants(i).account().parent().id());
					participant.call_privilege = int2str(message.response().entity().data().group().participants(i).call_privilege());
					participant.priority = int2str(message.response().entity().data().group().participants(i).priority());
					participant.session_status = int2str(message.response().entity().data().group().participants(i).status());
					participant.token_privilege = int2str(message.response().entity().data().group().participants(i).token_privilege());
					response.data.group.participants.push_back(participant);
				}
				response.data.group.record_status = int2str(message.response().entity().data().group().record_status());
				response.data.group.record_type = int2str(message.response().entity().data().group().record_type());
				response.data.group.short_number = message.response().entity().data().group().short_number();
			}
			else if((ns__EntityType)message.response().entity().data().id().entity_type() == ORGANIZATION)
			{
				response.data.organization.base.entity_type = int2str(message.response().entity().data().organization().base().entity_type());
				response.data.organization.base.id = int2str(message.response().entity().data().organization().base().id());
				response.data.organization.base.name = message.response().entity().data().organization().base().name();
				response.data.organization.base.parentid = int2str(message.response().entity().data().organization().base().parent().id());

				ns__Entity entity;
				for(int i=0;i<message.response().entity().data().organization().members_size();i++)
				{
					entity.entity_type = int2str(message.response().entity().data().organization().members(i).entity_type());
					entity.id = int2str(message.response().entity().data().organization().members(i).id());
					entity.name = message.response().entity().data().organization().members(i).name();
					entity.parentid = int2str(message.response().entity().data().organization().members(i).parent().id());
					response.data.organization.members.push_back(entity);
				}
			}
			else if((ns__EntityType)message.response().entity().data().id().entity_type() == ALERT)
			{
				response.data.alert.alert_level = int2str(message.response().entity().data().alert().level());
				//response.data.alert.alert_status = int2str(message.response().entity().data().alert().level());
				response.data.alert.alram_time = message.response().entity().data().alert().alram_time();
				response.data.alert.base.entity_type = int2str(message.response().entity().data().alert().base().entity_type());
				response.data.alert.base.id = int2str(message.response().entity().data().alert().base().id());
				response.data.alert.base.name = message.response().entity().data().alert().base().name();
				response.data.alert.base.parentid = int2str(message.response().entity().data().alert().base().parent().id());
				response.data.alert.create_time = message.response().entity().data().alert().create_time();
				response.data.alert.describe = message.response().entity().data().alert().describe();
				response.data.alert.use_cars = int2str(message.response().entity().data().alert().use_cars());
				response.data.alert.group_id = int2str(message.response().entity().data().alert().group().id());
			}
		}
		else
		{
			LOG(ERROR)<<message.response().error_describe();
			response.result = false;
			response.error_describe = message.response().error_describe();
			return SOAP_OK;
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	
	return SOAP_OK;
}

int xiaofangService::Dispatch_Entity_Nofitication(std::string session_id, 
													ns__Dispatch_Entity_Nofitication_Response &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Append_Group(std::string session_id,
											ns__Group group, 
											ns__Dispatch_Append_Group_Response &response)
{
	LOG(INFO)<<"Dispatch_Append_Group";
	app::dispatch::Message message;
	unsigned long sequence_append_group = get_sequence();
	message.set_sequence(sequence_append_group);
	try{
		message.set_session_id(std::stoul(session_id,nullptr,0));
	}catch(...)
	{
		response.result = false;
		response.error_describe = "no session id";
		return SOAP_OK;
	}
	message.set_msg_type(app::dispatch::MSG::Append_Group_Request);
	if(group.name.empty())
	{
		response.result = false;
		response.error_describe = "name is empty";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_append_group()->mutable_group()->mutable_base()->set_name(group.name);
	if(group.parent_id.empty())
	{
		response.result = false;
		response.error_describe = "no parent id";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_append_group()->mutable_group()->mutable_base()->mutable_parent()->set_id(std::stoul(group.parent_id,nullptr,0));
	message.mutable_request()->mutable_append_group()->mutable_group()->mutable_base()->set_entity_type((::pbmsg::EntityType)GROUPW);
	if(!group.sealed)
		message.mutable_request()->mutable_append_group()->mutable_group()->set_sealed(false);
	else
		message.mutable_request()->mutable_append_group()->mutable_group()->set_sealed(true);
	if(!group.short_number.empty())
		message.mutable_request()->mutable_append_group()->mutable_group()->set_short_number(group.short_number);
	if(std::stoul(group.size,nullptr,0)!=0)
	{
		::pbmsg::Participant *participant;
		message.mutable_request()->mutable_append_group()->mutable_group()->set_include_participants(true);
		for(std::list<ns__Participant>::iterator itor = group.participants.begin();itor!=group.participants.end();itor++)
		{	
			participant = message.mutable_request()->mutable_append_group()->mutable_group()->add_participants();
			participant->mutable_account()->set_id(std::stoul(itor->id,nullptr,0));
			participant->set_priority(std::stoul(itor->priority,nullptr,0));
			participant->set_call_privilege((::pbmsg::CallPrivilege)std::stoul(itor->call_privilege,nullptr,0));
			participant->set_token_privilege((::pbmsg::TokenPrivilege)std::stoul(itor->token_privilege,nullptr,0));
		}	
	}
	else
	{
		message.mutable_request()->mutable_append_group()->mutable_group()->set_include_participants(false);
	}
	std::string send_str;
	std::string recev_str;
	if(!message.IsInitialized())
	{
		response.result = false;
		response.error_describe = "Message can't be initialized";
		return SOAP_OK;
	}
	if(!message.SerializeToString(&send_str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	{
		std::lock_guard<std::mutex> lock(keep_tcp_connection_mutex);
		std::map<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> >::iterator itr = keep_tcp_connection.find(std::stoul(session_id,nullptr,0));
		if(itr == keep_tcp_connection.end())
		{
			response.result = false;
			response.error_describe = "session id not exist";
			return SOAP_OK;
		}
		std::tr1::shared_ptr<ConnectToAppServer> connecttoserver_ptr = itr->second;

		connecttoserver_ptr->send_request(send_str);

		message.Clear();

		bool break_while = false;
	create_timer([&break_while](){break_while=true;}, 5, false);
		
		do{
			recev_str = connecttoserver_ptr->get_response();
		}while((recev_str=="") && (!break_while));

	if(break_while)
	{
		LOG(ERROR)<<"socket write and read error";
		
		response.result = false;
		response.error_describe = "socket write and read error";
		return SOAP_OK;
	}

	}//unlock keep_tcp_connection_mutex
	LOG(INFO)<<"Parse data from protobuf protocol";
	if (message.ParseFromString(recev_str))
	{	
		if(message.response().result())
		{
			response.result = true;
			response.session_id = int2str(message.session_id());
			response.data.id = int2str(message.response().append_group().group().base().id());
			response.data.name = message.response().append_group().group().base().name();
			response.data.sealed = message.response().append_group().group().sealed();
			response.data.number = message.response().append_group().group().number();
			response.data.owner_id = int2str(message.response().append_group().group().owner().id());
			response.data.parent_id = int2str(message.response().append_group().group().owner().parent().id());
			response.data.short_number = message.response().append_group().group().short_number();
			response.data.record_status = (ns__RecordStatus)message.response().append_group().group().record_status();
			response.data.record_type = (ns__RecordType)message.response().append_group().group().record_type();
			response.data.size = int2str(message.response().append_group().group().participants_size());
			ns__Participant participant;
			for(int i=0;i<message.response().append_group().group().participants_size();i++)
			{
				participant.call_privilege = (ns__CallPrivilege)message.response().append_group().group().participants(i).call_privilege();
				participant.priority = int2str(message.response().append_group().group().participants(i).priority());
				participant.session_status = int2str(message.response().append_group().group().participants(i).status());
				participant.token_privilege = (ns__TokenPrivilege)message.response().append_group().group().participants(i).token_privilege();
				participant.id = int2str(message.response().append_group().group().participants(i).account().id());
				participant.name = message.response().append_group().group().participants(i).account().name();
				participant.parentid = int2str(message.response().append_group().group().participants(i).account().parent().id());
				response.data.participants.push_back(participant);
			}
		}
		else
		{
			response.result = false;
			response.error_describe = message.response().error_describe();
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	return SOAP_OK;
}

int xiaofangService::Dispatch_Modify_Group(std::string session_id,
											ns__Group group, 
											ns__Dispatch_Modify_Group_Response &response)
{
	
	LOG(INFO)<<"Dispatch_Modify_Group";
	
	app::dispatch::Message message;
	unsigned long sequence_modify_group = get_sequence();
	message.set_sequence(sequence_modify_group);
	message.set_session_id(std::stoul(session_id,nullptr,0));
	message.set_msg_type(app::dispatch::MSG::Modify_Group_Request);
	if(group.id.empty())
	{
		response.result = false;
		response.error_describe = "group id is empty";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_modify_group()->mutable_group()->mutable_base()->set_id(std::stoul(group.id,nullptr,0));
	if(!group.sealed)
		message.mutable_request()->mutable_modify_group()->mutable_group()->set_sealed(false);
	else
		message.mutable_request()->mutable_modify_group()->mutable_group()->set_sealed(true);
	if(!group.short_number.empty())
		message.mutable_request()->mutable_modify_group()->mutable_group()->set_short_number(group.short_number);
	std::string send_str;
	std::string recev_str;
	if(!message.IsInitialized())
	{
		response.result = false;
		response.error_describe = "Message can't be initialized";
		return SOAP_OK;
	}
	if(!message.SerializeToString(&send_str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	{
		std::lock_guard<std::mutex> lock(keep_tcp_connection_mutex);
		std::map<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> >::iterator itr = keep_tcp_connection.find(std::stoul(session_id,nullptr,0));
		if(itr == keep_tcp_connection.end())
		{
			response.result = false;
			response.error_describe = "session id not exist";
			return SOAP_OK;
		}
		std::tr1::shared_ptr<ConnectToAppServer> connecttoserver_ptr = itr->second;

		connecttoserver_ptr->send_request(send_str);

		message.Clear();

		bool break_while = false;
		
	create_timer([&break_while](){break_while=true;}, 5, false);
		
		do{
			recev_str = connecttoserver_ptr->get_response();
		}while((recev_str=="") && (!break_while));

	if(break_while)
	{
		LOG(ERROR)<<"socket write and read error";
		
		response.result = false;
		response.error_describe = "socket write and read error";
		return SOAP_OK;
	}

	}//unlock keep_tcp_connection_mutex
	LOG(INFO)<<"Parse data from protobuf protocol";
	if (message.ParseFromString(recev_str))
	{	
		if(message.response().result())
		{
			response.result = true;
			response.session_id = int2str(message.session_id());
			response.data.id = int2str(message.response().modify_group().group().base().id());
			response.data.name = message.response().modify_group().group().base().name();
			response.data.number = message.response().modify_group().group().number();
			response.data.owner_id = int2str(message.response().modify_group().group().owner().id());
			response.data.short_number = message.response().modify_group().group().short_number();
			response.data.record_status = (ns__RecordStatus)message.response().modify_group().group().record_status();
			response.data.record_type = (ns__RecordType)message.response().modify_group().group().record_type();
			response.data.sealed = message.response().modify_group().group().sealed();
			response.data.parent_id = int2str(message.response().modify_group().group().base().parent().id());
		}
		else
		{
			response.result = false;
			response.error_describe = message.response().error_describe();
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	return SOAP_OK;
}

int xiaofangService::Dispatch_Modify_Participants(std::string session_id,
												ns__Modify_Participant request, 
												ns__Dispatch_Modify_Participants_Response &response)
{
	LOG(INFO)<<"Dispatch_Modify_Participants";
	
	app::dispatch::Message message;
	unsigned long sequence_modify_participants = get_sequence();
	message.set_sequence(sequence_modify_participants);
	message.set_session_id(std::stoul(session_id,nullptr,0));
	message.set_msg_type(app::dispatch::MSG::Modify_Participants_Request);
	if(request.group_id.empty())
	{
		response.result = false;
		response.error_describe = "no group id input";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_modify_participants()->mutable_group_id()->set_id(std::stoul(request.group_id,nullptr,0));
	if(request.modify_type.empty())
	{
		response.result = false;
		response.error_describe = "no modify type input";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_modify_participants()->set_modify_type((::pbmsg::ListModifyType)std::stoul(request.modify_type,nullptr,0));
	if(std::stoul(request.size,nullptr,0)!=0)
	{
		::pbmsg::Participant *participant;
		for(std::list<ns__Participant>::iterator itor = request.participants.begin();itor!=request.participants.end();itor++)
		{		
			participant = message.mutable_request()->mutable_modify_participants()->add_particiapnts();
			participant->mutable_account()->set_id(std::stoul(itor->id,nullptr,0));
			participant->set_priority(std::stoul(itor->priority,nullptr,0));
			participant->set_call_privilege((::pbmsg::CallPrivilege)std::stoul(itor->call_privilege,nullptr,0));
			participant->set_token_privilege((::pbmsg::TokenPrivilege)std::stoul(itor->token_privilege,nullptr,0));
		}	
	}
	else
	{
		response.result = false;
		response.error_describe = "no participants input";
		return SOAP_OK;
	}
	std::string send_str;
	std::string recev_str;
	if(!message.IsInitialized())
	{
		response.result = false;
		response.error_describe = "Message can't be initialized";
		return SOAP_OK;
	}
	if(!message.SerializeToString(&send_str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	{
		std::lock_guard<std::mutex> lock(keep_tcp_connection_mutex);
		std::map<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> >::iterator itr = keep_tcp_connection.find(std::stoul(session_id,nullptr,0));
		if(itr == keep_tcp_connection.end())
		{
			response.result = false;
			response.error_describe = "session id not exist";
			return SOAP_OK;
		}
		std::tr1::shared_ptr<ConnectToAppServer> connecttoserver_ptr = itr->second;

		connecttoserver_ptr->send_request(send_str);

		message.Clear();

		bool break_while = false;
		
	create_timer([&break_while](){break_while=true;}, 5, false);
		
		do{
			recev_str = connecttoserver_ptr->get_response();
		}while((recev_str=="") && (!break_while));

	if(break_while)
	{
		LOG(ERROR)<<"socket write and read error";
		
		response.result = false;
		response.error_describe = "socket write and read error";
		return SOAP_OK;
	}

	}//unlock keep_tcp_connection_mutex
	LOG(INFO)<<"Parse data from protobuf protocol";
	if (message.ParseFromString(recev_str))
	{	
		if(message.response().result())
		{
			response.result = true;
			response.session_id = int2str(message.session_id());
			response.data.size = int2str(message.response().append_group().group().participants_size());
			ns__Participant participant;
			for(int i=0;i<message.response().append_group().group().participants_size();i++)
			{
				participant.call_privilege = (ns__CallPrivilege)message.response().append_group().group().participants(i).call_privilege();
				participant.priority = int2str(message.response().append_group().group().participants(i).priority());
				participant.session_status = int2str(message.response().append_group().group().participants(i).status());
				participant.token_privilege = (ns__TokenPrivilege)message.response().append_group().group().participants(i).token_privilege();
				participant.id = int2str(message.response().append_group().group().participants(i).account().id());
				participant.name = message.response().append_group().group().participants(i).account().name();
				participant.parentid = int2str(message.response().append_group().group().participants(i).account().parent().id());
				response.data.participants.push_back(participant);
			}
		}
		else
		{
			response.result = false;
			response.error_describe = message.response().error_describe();
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	return SOAP_OK;
}

int xiaofangService::Dispatch_Delete_Group(std::string session_id,
											std::string group_id, 
											ns__Normal_Response &response)
{
	LOG(INFO)<<"Dispatch_Delete_Group";
	
	app::dispatch::Message message;
	message.set_session_id(std::stoul(session_id,nullptr,0));
	unsigned long sequence_delete_group = get_sequence();
	message.set_sequence(sequence_delete_group);
	message.set_msg_type(app::dispatch::MSG::Delete_Group_Request);
	if(group_id.empty())
	{
		response.result = false;
		response.error_describe = "no group id input";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_delete_group()->mutable_group_id()->set_id(std::stoul(group_id,nullptr,0));
	std::string send_str;
	std::string recev_str;
	if(!message.IsInitialized())
	{
		response.result = false;
		response.error_describe = "Message can't be initialized";
		return SOAP_OK;
	}
	if(!message.SerializeToString(&send_str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	{
		std::lock_guard<std::mutex> lock(keep_tcp_connection_mutex);
		std::map<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> >::iterator itr = keep_tcp_connection.find(std::stoul(session_id,nullptr,0));
		if(itr == keep_tcp_connection.end())
		{
			response.result = false;
			response.error_describe = "session id not exist";
			return SOAP_OK;
		}
		std::tr1::shared_ptr<ConnectToAppServer> connecttoserver_ptr = itr->second;

		connecttoserver_ptr->send_request(send_str);

		message.Clear();

		bool break_while = false;
		
	create_timer([&break_while](){break_while=true;}, 5, false);
		
		do{
			recev_str = connecttoserver_ptr->get_response();
		}while((recev_str=="") && (!break_while));

	if(break_while)
	{
		LOG(ERROR)<<"socket write and read error";
		
		response.result = false;
		response.error_describe = "socket write and read error";
		return SOAP_OK;
	}

	}//unlock keep_tcp_connection_mutex
	LOG(INFO)<<"Parse data from protobuf protocol";
	if (message.ParseFromString(recev_str))
	{	
		if(message.response().result())
		{
			response.result = true;
			response.session_id = int2str(message.session_id());
		}
		else
		{
			response.result = false;
			response.error_describe = message.response().error_describe();
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	return SOAP_OK;
}

int xiaofangService::Dispatch_Dispatch_Participants_Notification(std::string session_id, ns__Dispatch_Dispatch_Participants_Notification_Response &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Join_Group_Request_Nofitication(std::string session_id, ns__Dispatch_Join_Group_Request_Nofitication_Response &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Participant_Status_Notification(std::string session_id, ns__Dispatch_Participant_Status_Notification_Response &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Media_Message_Request(std::string session_id,
													std::string group_id, 
													std::string from_message_id, 
													std::string from_time, 
													std::string max_message_count, 
													ns__Dispatch_Media_Message_Request_Response &response)
{
	LOG(INFO)<<"Dispatch_Media_Message_Request";
	
	app::dispatch::Message message;
	message.set_session_id(std::stoul(session_id,nullptr,0));
	unsigned long sequence_media_message = get_sequence();
	message.set_sequence(sequence_media_message);
	message.set_msg_type(app::dispatch::MSG::Media_Message_Request);
	if(group_id.empty())
	{
		response.result = false;
		response.error_describe = "no group id input";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_group_message()->mutable_id()->set_id(std::stoul(group_id,nullptr,0));
	if(!from_message_id.empty())
		message.mutable_request()->mutable_group_message()->set_from_message_id(std::stoul(from_message_id,nullptr,0));
	if(!from_time.empty())
		message.mutable_request()->mutable_group_message()->set_from_timestamp(from_time);
	if(max_message_count.empty())
	{
		response.result = false;
		response.error_describe = "no max message count input";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_group_message()->set_max_message_count(std::stoul(max_message_count,nullptr,0));
	std::string send_str;
	std::string recev_str;
	if(!message.IsInitialized())
	{
		response.result = false;
		response.error_describe = "Message can't be initialized";
		return SOAP_OK;
	}
	if(!message.SerializeToString(&send_str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	{
		std::lock_guard<std::mutex> lock(keep_tcp_connection_mutex);
		std::map<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> >::iterator itr = keep_tcp_connection.find(std::stoul(session_id,nullptr,0));
		if(itr == keep_tcp_connection.end())
		{
			response.result = false;
			response.error_describe = "session id not exist";
			return SOAP_OK;
		}
		std::tr1::shared_ptr<ConnectToAppServer> connecttoserver_ptr = itr->second;

		connecttoserver_ptr->send_request(send_str);

		message.Clear();

		bool break_while = false;
		
	create_timer([&break_while](){break_while=true;}, 5, false);
		
		do{
			recev_str = connecttoserver_ptr->get_response();
		}while((recev_str=="") && (!break_while));

	if(break_while)
	{
		LOG(ERROR)<<"socket write and read error";
		
		response.result = false;
		response.error_describe = "socket write and read error";
		return SOAP_OK;
	}

	}//unlock keep_tcp_connection_mutex
	LOG(INFO)<<"Parse data from protobuf protocol";
	if (message.ParseFromString(recev_str))
	{	
		if(message.response().result())
		{
			response.result = true;
			response.session_id = int2str(message.session_id());
			response.size = int2str(message.response().group_message().messages_size());
			response.leave_message_count = int2str(message.response().group_message().leave_message_count());
			ns__MediaMessage mediamessage;
			for(int i=0;i<message.response().group_message().messages_size();i++)
			{
				mediamessage.audio_length = int2str(message.response().group_message().messages(i).audio_length());
				mediamessage.audio_uri = message.response().group_message().messages(i).audio_uri();
				mediamessage.id = int2str(message.response().group_message().messages(i).id());
				mediamessage.picture_uri = message.response().group_message().messages(i).picture_uri();
				mediamessage.sender = message.response().group_message().messages(i).sender();
				mediamessage.text = message.response().group_message().messages(i).text();
				mediamessage.timestamp = message.response().group_message().messages(i).timestamp();
				mediamessage.video_length = int2str(message.response().group_message().messages(i).video_length());
				mediamessage.video_uri = message.response().group_message().messages(i).video_uri();
				response.data.push_back(mediamessage);
			}
		}
		else
		{
			response.result = false;
			response.error_describe = message.response().error_describe();
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	return SOAP_OK;
}

int xiaofangService::Dispatch_Media_Message_Notification(std::string session_id, ns__Dispatch_Media_Message_Notification_Response &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Participant_Connect_Request_Notification(std::string session_id, ns__Dispatch_Participant_Connect_Request_Notification_Response &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Participant_Speak_Request_Notification(std::string session_id, ns__Dispatch_Participant_Speak_Request_Notification_Response &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Invite_Participant_Request(std::string session_id,
														std::string group_id, 
														std::string account_id, 
														ns__Normal_Response &response)
{
	LOG(INFO)<<"Dispatch_Invite_Participant_Request";
	
	app::dispatch::Message message;
	message.set_session_id(std::stoul(session_id,nullptr,0));
	unsigned long sequence_invite_participant = get_sequence();
	message.set_sequence(sequence_invite_participant);
	message.set_msg_type(app::dispatch::MSG::Invite_Participant_Request);
	if(group_id.empty())
	{
		response.result = false;
		response.error_describe = "no group id input";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_invite_participant()->mutable_group_id()->set_id(std::stoul(group_id,nullptr,0));
	if(account_id.empty())
	{
		response.result = false;
		response.error_describe = "no account id input";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_invite_participant()->mutable_account_id()->set_id(std::stoul(account_id,nullptr,0));
	
	std::string send_str;
	std::string recev_str;
	if(!message.IsInitialized())
	{
		response.result = false;
		response.error_describe = "Message can't be initialized";
		return SOAP_OK;
	}
	if(!message.SerializeToString(&send_str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	{
		std::lock_guard<std::mutex> lock(keep_tcp_connection_mutex);
		std::map<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> >::iterator itr = keep_tcp_connection.find(std::stoul(session_id,nullptr,0));
		if(itr == keep_tcp_connection.end())
		{
			response.result = false;
			response.error_describe = "session id not exist";
			return SOAP_OK;
		}
		std::tr1::shared_ptr<ConnectToAppServer> connecttoserver_ptr = itr->second;

		connecttoserver_ptr->send_request(send_str);

		message.Clear();

		bool break_while = false;
		
	create_timer([&break_while](){break_while=true;}, 5, false);
		
		do{
			recev_str = connecttoserver_ptr->get_response();
		}while((recev_str=="") && (!break_while));

	if(break_while)
	{
		LOG(ERROR)<<"socket write and read error";
		
		response.result = false;
		response.error_describe = "socket write and read error";
		return SOAP_OK;
	}

	}//unlock keep_tcp_connection_mutex
	LOG(INFO)<<"Parse data from protobuf protocol";
	if (message.ParseFromString(recev_str))
	{	
		if(message.response().result())
		{
			response.result = true;
			response.session_id = int2str(message.session_id());
		}
		else
		{
			response.result = false;
			response.error_describe = message.response().error_describe();
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	return SOAP_OK;
}

int xiaofangService::Dispatch_Drop_Participant_Request(std::string session_id,
														std::string group_id, 
														std::string account_id, 
														ns__Normal_Response &response)
{
	LOG(INFO)<<"Dispatch_Drop_Participant_Request";
	
	app::dispatch::Message message;
	message.set_session_id(std::stoul(session_id,nullptr,0));
	unsigned long sequence_drop_participant = get_sequence();
	message.set_sequence(sequence_drop_participant);
	message.set_msg_type(app::dispatch::MSG::Drop_Participant_Request);
	if(group_id.empty())
	{
		response.result = false;
		response.error_describe = "no group id input";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_drop_participant()->mutable_group_id()->set_id(std::stoul(group_id,nullptr,0));
	if(account_id.empty())
	{
		response.result = false;
		response.error_describe = "no account id input";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_drop_participant()->mutable_account_id()->set_id(std::stoul(account_id,nullptr,0));
	
	std::string send_str;
	std::string recev_str;
	if(!message.IsInitialized())
	{
		response.result = false;
		response.error_describe = "Message can't be initialized";
		return SOAP_OK;
	}
	if(!message.SerializeToString(&send_str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	{
		std::lock_guard<std::mutex> lock(keep_tcp_connection_mutex);
		std::map<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> >::iterator itr = keep_tcp_connection.find(std::stoul(session_id,nullptr,0));
		if(itr == keep_tcp_connection.end())
		{
			response.result = false;
			response.error_describe = "session id not exist";
			return SOAP_OK;
		}
		std::tr1::shared_ptr<ConnectToAppServer> connecttoserver_ptr = itr->second;

		connecttoserver_ptr->send_request(send_str);

		message.Clear();

		bool break_while = false;
		
	create_timer([&break_while](){break_while=true;}, 5, false);
		
		do{
			recev_str = connecttoserver_ptr->get_response();
		}while((recev_str=="") && (!break_while));

	if(break_while)
	{
		LOG(ERROR)<<"socket write and read error";
		
		response.result = false;
		response.error_describe = "socket write and read error";
		return SOAP_OK;
	}

	}//unlock keep_tcp_connection_mutex
	LOG(INFO)<<"Parse data from protobuf protocol";
	if (message.ParseFromString(recev_str ))
	{	
		if(message.response().result())
		{
			response.result = true;
			response.session_id = int2str(message.session_id());
		}
		else
		{
			response.result = false;
			response.error_describe = message.response().error_describe();
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	return SOAP_OK;
}

int xiaofangService::Dispatch_Release_Participant_Token_Request(std::string session_id,
																std::string group_id, 
																std::string account_id, 
																ns__Normal_Response &response)
{
	LOG(INFO)<<"Dispatch_Release_Participant_Token_Request";
	
	app::dispatch::Message message;
	message.set_session_id(std::stoul(session_id,nullptr,0));
	unsigned long sequence_release_participant_token = get_sequence();
	message.set_sequence(sequence_release_participant_token);
	message.set_msg_type(app::dispatch::MSG::Release_Participant_Token_Request);
	if(group_id.empty())
	{
		response.result = false;
		response.error_describe = "no group id input";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_release_participant_token()->mutable_group_id()->set_id(std::stoul(group_id,nullptr,0));
	if(account_id.empty())
	{
		response.result = false;
		response.error_describe = "no account id input";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_release_participant_token()->mutable_account_id()->set_id(std::stoul(account_id,nullptr,0));
	
	std::string send_str;
	std::string recev_str;
	if(!message.IsInitialized())
	{
		response.result = false;
		response.error_describe = "Message can't be initialized";
		return SOAP_OK;
	}
	if(!message.SerializeToString(&send_str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	{
		std::lock_guard<std::mutex> lock(keep_tcp_connection_mutex);
		std::map<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> >::iterator itr = keep_tcp_connection.find(std::stoul(session_id,nullptr,0));
		if(itr == keep_tcp_connection.end())
		{
			response.result = false;
			response.error_describe = "session id not exist";
			return SOAP_OK;
		}
		std::tr1::shared_ptr<ConnectToAppServer> connecttoserver_ptr = itr->second;

		connecttoserver_ptr->send_request(send_str);

		message.Clear();

		bool break_while = false;
		
	create_timer([&break_while](){break_while=true;}, 5, false);
		
		do{
			recev_str = connecttoserver_ptr->get_response();
		}while((recev_str=="") && (!break_while));

	if(break_while)
	{
		LOG(ERROR)<<"socket write and read error";
		
		response.result = false;
		response.error_describe = "socket write and read error";
		return SOAP_OK;
	}

	}//unlock keep_tcp_connection_mutex
	LOG(INFO)<<"Parse data from protobuf protocol";
	if (message.ParseFromString(recev_str ))
	{	
		if(message.response().result())
		{
			response.result = true;
			response.session_id = int2str(message.session_id());
		}
		else
		{
			response.result = false;
			response.error_describe = message.response().error_describe();
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	return SOAP_OK;
}

int xiaofangService::Dispatch_Appoint_Participant_Speak_Request(std::string session_id,
																std::string group_id, 
																std::string account_id, 
																ns__Normal_Response &response)
{
	LOG(INFO)<<"Dispatch_Appoint_Participant_Speak_Request";
	
	app::dispatch::Message message;
	message.set_session_id(std::stoul(session_id,nullptr,0));
	unsigned long sequence_appoint_participant_speak = get_sequence();
	message.set_sequence(sequence_appoint_participant_speak);
	message.set_msg_type(app::dispatch::MSG::Appoint_Participant_Speak_Request);
	if(group_id.empty())
	{
		response.result = false;
		response.error_describe = "no group id input";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_appoint_participant_speak()->mutable_group_id()->set_id(std::stoul(group_id,nullptr,0));
	if(account_id.empty())
	{
		response.result = false;
		response.error_describe = "no account id input";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_appoint_participant_speak()->mutable_account_id()->set_id(std::stoul(account_id,nullptr,0));
	
	std::string send_str;
	std::string recev_str;
	if(!message.IsInitialized())
	{
		response.result = false;
		response.error_describe = "Message can't be initialized";
		return SOAP_OK;
	}
	if(!message.SerializeToString(&send_str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	{
		std::lock_guard<std::mutex> lock(keep_tcp_connection_mutex);
		std::map<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> >::iterator itr = keep_tcp_connection.find(std::stoul(session_id,nullptr,0));
		if(itr == keep_tcp_connection.end())
		{
			response.result = false;
			response.error_describe = "session id not exist";
			return SOAP_OK;
		}
		std::tr1::shared_ptr<ConnectToAppServer> connecttoserver_ptr = itr->second;

		connecttoserver_ptr->send_request(send_str);

		message.Clear();

		bool break_while = false;
		
	create_timer([&break_while](){break_while=true;}, 5, false);
		
		do{
			recev_str = connecttoserver_ptr->get_response();
		}while((recev_str=="") && (!break_while));

	if(break_while)
	{
		LOG(ERROR)<<"socket write and read error";
		
		response.result = false;
		response.error_describe = "socket write and read error";
		return SOAP_OK;
	}

	}//unlock keep_tcp_connection_mutex
	LOG(INFO)<<"Parse data from protobuf protocol";
	if (message.ParseFromString(recev_str ))
	{	
		if(message.response().result())
		{
			response.result = true;
			response.session_id = int2str(message.session_id());
		}
		else
		{
			response.result = false;
			response.error_describe = message.response().error_describe();
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	return SOAP_OK;
}

int xiaofangService::Dispatch_Jion_Group_Request(std::string session_id,
												std::string group_id, 
												ns__Normal_Response &response)
{
	LOG(INFO)<<"Dispatch_Jion_Group_Request";
	
	app::dispatch::Message message;
	message.set_session_id(std::stoul(session_id,nullptr,0));
	unsigned long sequence_jion_group = get_sequence();
	message.set_sequence(sequence_jion_group);
	message.set_msg_type(app::dispatch::MSG::Jion_Group_Request);
	if(group_id.empty())
	{
		response.result = false;
		response.error_describe = "no group id input";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_jion_group()->mutable_group_id()->set_id(std::stoul(group_id,nullptr,0));
	
	std::string send_str;
	std::string recev_str;
	if(!message.IsInitialized())
	{
		response.result = false;
		response.error_describe = "Message can't be initialized";
		return SOAP_OK;
	}
	if(!message.SerializeToString(&send_str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	{
		std::lock_guard<std::mutex> lock(keep_tcp_connection_mutex);
		std::map<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> >::iterator itr = keep_tcp_connection.find(std::stoul(session_id,nullptr,0));
		if(itr == keep_tcp_connection.end())
		{
			response.result = false;
			response.error_describe = "session id not exist";
			return SOAP_OK;
		}
		std::tr1::shared_ptr<ConnectToAppServer> connecttoserver_ptr = itr->second;

		connecttoserver_ptr->send_request(send_str);

		message.Clear();

		bool break_while = false;
		
	create_timer([&break_while](){break_while=true;}, 5, false);
		
		do{
			recev_str = connecttoserver_ptr->get_response();
		}while((recev_str=="") && (!break_while));

	if(break_while)
	{
		LOG(ERROR)<<"socket write and read error";
		
		response.result = false;
		response.error_describe = "socket write and read error";
		return SOAP_OK;
	}

	}//unlock keep_tcp_connection_mutex
	LOG(INFO)<<"Parse data from protobuf protocol";
	if (message.ParseFromString(recev_str ))
	{	
		if(message.response().result())
		{
			response.result = true;
			response.session_id = int2str(message.session_id());
		}
		else
		{
			response.result = false;
			response.error_describe = message.response().error_describe();
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	return SOAP_OK;
}

int xiaofangService::Dispatch_Leave_Group_Request(std::string session_id,
													std::string group_id, 
													ns__Normal_Response &response)
{
	LOG(INFO)<<"Dispatch_Jion_Group_Request";
	
	app::dispatch::Message message;
	message.set_session_id(std::stoul(session_id,nullptr,0));
	unsigned long sequence_leave_group = get_sequence();
	message.set_sequence(sequence_leave_group);
	message.set_msg_type(app::dispatch::MSG::Leave_Group_Request);
	if(group_id.empty())
	{
		response.result = false;
		response.error_describe = "no group id input";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_leave_group()->mutable_group_id()->set_id(std::stoul(group_id,nullptr,0));
	
	std::string send_str;
	std::string recev_str;
	if(!message.IsInitialized())
	{
		response.result = false;
		response.error_describe = "Message can't be initialized";
		return SOAP_OK;
	}
	if(!message.SerializeToString(&send_str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	{
		std::lock_guard<std::mutex> lock(keep_tcp_connection_mutex);
		std::map<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> >::iterator itr = keep_tcp_connection.find(std::stoul(session_id,nullptr,0));
		if(itr == keep_tcp_connection.end())
		{
			response.result = false;
			response.error_describe = "session id not exist";
			return SOAP_OK;
		}
		std::tr1::shared_ptr<ConnectToAppServer> connecttoserver_ptr = itr->second;

		connecttoserver_ptr->send_request(send_str);

		message.Clear();

		bool break_while = false;
		
	create_timer([&break_while](){break_while=true;}, 5, false);
		
		do{
			recev_str = connecttoserver_ptr->get_response();
		}while((recev_str=="") && (!break_while));

	if(break_while)
	{
		LOG(ERROR)<<"socket write and read error";
		
		response.result = false;
		response.error_describe = "socket write and read error";
		return SOAP_OK;
	}

	}//unlock keep_tcp_connection_mutex
	LOG(INFO)<<"Parse data from protobuf protocol";
	if (message.ParseFromString(recev_str ))
	{	
		if(message.response().result())
		{
			response.result = true;
			response.session_id = int2str(message.session_id());
		}
		else
		{
			response.result = false;
			response.error_describe = message.response().error_describe();
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	return SOAP_OK;
}

int xiaofangService::Dispatch_Session_Status_Notification(std::string session_id, ns__Dispatch_Session_Status_Notification_Response &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Send_Message_Request(std::string session_id,
													std::string group_id, 
													ns__MediaMessage mediamessage, 
													ns__Normal_Response &response)
{
	LOG(INFO)<<"Dispatch_Send_Message_Request";
	
	app::dispatch::Message message;
	message.set_session_id(std::stoul(session_id,nullptr,0));
	unsigned long sequence_send_message = get_sequence();
	message.set_sequence(sequence_send_message);
	message.set_msg_type(app::dispatch::MSG::Send_Message_Request);
	if(group_id.empty())
	{
		response.result = false;
		response.error_describe = "no group id input";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_send_message()->mutable_id()->set_id(std::stoul(group_id,nullptr,0));
	if(!mediamessage.sender.empty())
		message.mutable_request()->mutable_send_message()->mutable_msg()->set_sender(mediamessage.sender);
	if(!mediamessage.text.empty())
		message.mutable_request()->mutable_send_message()->mutable_msg()->set_text(mediamessage.text);
	if(!mediamessage.picture_uri.empty())
		message.mutable_request()->mutable_send_message()->mutable_msg()->set_picture_uri(mediamessage.picture_uri);
	if(!mediamessage.audio_uri.empty())
		message.mutable_request()->mutable_send_message()->mutable_msg()->set_audio_uri(mediamessage.audio_uri);
	if(!mediamessage.audio_length.empty())
		message.mutable_request()->mutable_send_message()->mutable_msg()->set_audio_length(std::stoul(mediamessage.audio_length,nullptr,0));
	if(!mediamessage.video_uri.empty())
		message.mutable_request()->mutable_send_message()->mutable_msg()->set_video_uri(mediamessage.video_uri);
	if(!mediamessage.video_length.empty())
		message.mutable_request()->mutable_send_message()->mutable_msg()->set_video_length(std::stoul(mediamessage.video_length,nullptr,0));
	
	std::string send_str;
	std::string recev_str;
	if(!message.IsInitialized())
	{
		response.result = false;
		response.error_describe = "Message can't be initialized";
		return SOAP_OK;
	}
	if(!message.SerializeToString(&send_str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	{
		std::lock_guard<std::mutex> lock(keep_tcp_connection_mutex);
		std::map<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> >::iterator itr = keep_tcp_connection.find(std::stoul(session_id,nullptr,0));
		if(itr == keep_tcp_connection.end())
		{
			response.result = false;
			response.error_describe = "session id not exist";
			return SOAP_OK;
		}
		std::tr1::shared_ptr<ConnectToAppServer> connecttoserver_ptr = itr->second;

		connecttoserver_ptr->send_request(send_str);

		message.Clear();

		bool break_while = false;
		
	create_timer([&break_while](){break_while=true;}, 5, false);
		
		do{
			recev_str = connecttoserver_ptr->get_response();
		}while((recev_str=="") && (!break_while));

	if(break_while)
	{
		LOG(ERROR)<<"socket write and read error";
		
		response.result = false;
		response.error_describe = "socket write and read error";
		return SOAP_OK;
	}

	}//unlock keep_tcp_connection_mutex
	LOG(INFO)<<"Parse data from protobuf protocol";
	if (message.ParseFromString(recev_str ))
	{	
		if(message.response().result())
		{
			response.result = true;
			response.session_id = int2str(message.session_id());
		}
		else
		{
			response.result = false;
			response.error_describe = message.response().error_describe();
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	return SOAP_OK;
}

int xiaofangService::Dispatch_Start_Record_Request(std::string session_id,
													std::string group_id, 
													ns__Normal_Response &response)
{
	LOG(INFO)<<"Dispatch_Start_Record_Request";
	
	app::dispatch::Message message;
	message.set_session_id(std::stoul(session_id,nullptr,0));
	unsigned long sequence_start_recode = get_sequence();
	message.set_sequence(sequence_start_recode);
	message.set_msg_type(app::dispatch::MSG::Start_Record_Request);
	if(group_id.empty())
	{
		response.result = false;
		response.error_describe = "no group id input";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_start_record()->mutable_group_id()->set_id(std::stoul(group_id,nullptr,0));
	
	std::string send_str;
	std::string recev_str;
	if(!message.IsInitialized())
	{
		response.result = false;
		response.error_describe = "Message can't be initialized";
		return SOAP_OK;
	}
	if(!message.SerializeToString(&send_str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	{
		std::lock_guard<std::mutex> lock(keep_tcp_connection_mutex);
		std::map<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> >::iterator itr = keep_tcp_connection.find(std::stoul(session_id,nullptr,0));
		if(itr == keep_tcp_connection.end())
		{
			response.result = false;
			response.error_describe = "session id not exist";
			return SOAP_OK;
		}
		std::tr1::shared_ptr<ConnectToAppServer> connecttoserver_ptr = itr->second;

		connecttoserver_ptr->send_request(send_str);

		message.Clear();

		bool break_while = false;
		
	create_timer([&break_while](){break_while=true;}, 5, false);
		
		do{
			recev_str = connecttoserver_ptr->get_response();
		}while((recev_str=="") && (!break_while));

	if(break_while)
	{
		LOG(ERROR)<<"socket write and read error";
		
		response.result = false;
		response.error_describe = "socket write and read error";
		return SOAP_OK;
	}

	}//unlock keep_tcp_connection_mutex
	LOG(INFO)<<"Parse data from protobuf protocol";
	if (message.ParseFromString(recev_str ))
	{	
		if(message.response().result())
		{
			response.result = true;
			response.session_id = int2str(message.session_id());
		}
		else
		{
			response.result = false;
			response.error_describe = message.response().error_describe();
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	return SOAP_OK;
}

int xiaofangService::Dispatch_Stop_Record_Request(std::string session_id,
												std::string group_id, 
												ns__Normal_Response &response)
{
	LOG(INFO)<<"Dispatch_Stop_Record_Request";
	
	app::dispatch::Message message;
	message.set_session_id(std::stoul(session_id,nullptr,0));
	unsigned long sequence_stop_record = get_sequence();
	message.set_sequence(sequence_stop_record);
	message.set_msg_type(app::dispatch::MSG::Stop_Record_Request);
	if(group_id.empty())
	{
		response.result = false;
		response.error_describe = "no group id input";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_stop_record()->mutable_group_id()->set_id(std::stoul(group_id,nullptr,0));
	
	std::string send_str;
	std::string recev_str;
	if(!message.IsInitialized())
	{
		response.result = false;
		response.error_describe = "Message can't be initialized";
		return SOAP_OK;
	}
	if(!message.SerializeToString(&send_str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	{
		std::lock_guard<std::mutex> lock(keep_tcp_connection_mutex);
		std::map<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> >::iterator itr = keep_tcp_connection.find(std::stoul(session_id,nullptr,0));
		if(itr == keep_tcp_connection.end())
		{
			response.result = false;
			response.error_describe = "session id not exist";
			return SOAP_OK;
		}
		std::tr1::shared_ptr<ConnectToAppServer> connecttoserver_ptr = itr->second;

		connecttoserver_ptr->send_request(send_str);

		message.Clear();

		bool break_while = false;
		
	create_timer([&break_while](){break_while=true;}, 5, false);
		
		do{
			recev_str = connecttoserver_ptr->get_response();
		}while((recev_str=="") && (!break_while));

	if(break_while)
	{
		LOG(ERROR)<<"socket write and read error";
		
		response.result = false;
		response.error_describe = "socket write and read error";
		return SOAP_OK;
	}

	}//unlock keep_tcp_connection_mutex
	LOG(INFO)<<"Parse data from protobuf protocol";
	if (message.ParseFromString(recev_str ))
	{	
		if(message.response().result())
		{
			response.result = true;
			response.session_id = int2str(message.session_id());
		}
		else
		{
			response.result = false;
			response.error_describe = message.response().error_describe();
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	return SOAP_OK;
}

int xiaofangService::Dispatch_Record_Status_Notification(std::string session_id, ns__Dispatch_Record_Status_Notification_Response &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Subscribe_Account_Location_Request(std::string session_id,
															bool subscribing, 
															std::string account_id, 
															std::string ttl, 
															ns__Normal_Response &response)
{
	LOG(INFO)<<"Dispatch_Subscribe_Account_Location_Request";
	
	app::dispatch::Message message;
	message.set_session_id(std::stoul(session_id,nullptr,0));
	unsigned long sequence_subcribe_account_location = get_sequence();
	message.set_sequence(sequence_subcribe_account_location);
	message.set_msg_type(app::dispatch::MSG::Subscribe_Account_Location_Request);
	if(account_id.empty())
	{
		response.result = false;
		response.error_describe = "account id is empty";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_subscribe_account_location()->mutable_account_id()->set_id(std::stoul(account_id,nullptr,0));
	if(ttl.empty())
	{
		response.result = false;
		response.error_describe = "ttl is empty";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_subscribe_account_location()->set_ttl(std::stoul(ttl,nullptr,0));
	if(!subscribing)
		message.mutable_request()->mutable_subscribe_account_location()->set_subscribing(false);
	else
		message.mutable_request()->mutable_subscribe_account_location()->set_subscribing(true);
	
	std::string send_str;
	std::string recev_str;
	if(!message.IsInitialized())
	{
		response.result = false;
		response.error_describe = "Message can't be initialized";
		return SOAP_OK;
	}
	if(!message.SerializeToString(&send_str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	{
		std::lock_guard<std::mutex> lock(keep_tcp_connection_mutex);
		std::map<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> >::iterator itr = keep_tcp_connection.find(std::stoul(session_id,nullptr,0));
		if(itr == keep_tcp_connection.end())
		{
			response.result = false;
			response.error_describe = "session id not exist";
			return SOAP_OK;
		}
		std::tr1::shared_ptr<ConnectToAppServer> connecttoserver_ptr = itr->second;

		connecttoserver_ptr->send_request(send_str);

		message.Clear();

		bool break_while = false;
		
	create_timer([&break_while](){break_while=true;}, 5, false);
		
		do{
			recev_str = connecttoserver_ptr->get_response();
		}while((recev_str=="") && (!break_while));

	if(break_while)
	{
		LOG(ERROR)<<"socket write and read error";
		
		response.result = false;
		response.error_describe = "socket write and read error";
		return SOAP_OK;
	}

	}//unlock keep_tcp_connection_mutex
	LOG(INFO)<<"Parse data from protobuf protocol";
	if (message.ParseFromString(recev_str ))
	{	
		if(message.response().result())
		{
			response.result = true;
			response.session_id = int2str(message.session_id());
		}
		else
		{
			response.result = false;
			response.error_describe = message.response().error_describe();
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	return SOAP_OK;
}

int xiaofangService::Dispatch_Account_Location_Notification(std::string session_id, ns__Dispatch_Account_Info_Notification_Response &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Append_Alert_Request(std::string session_id,
													ns__Alert alert, 
													std::list<ns__Account >members,
													std::string size,
													ns__Dispatch_Append_Alert_Request_Response &response)
{
	LOG(INFO)<<"Dispatch_Append_Alert_Request";
	
	app::dispatch::Message message;
	message.set_session_id(std::stoul(session_id,nullptr,0));
	unsigned long sequence_append_alert = get_sequence();
	message.set_sequence(sequence_append_alert);
	message.set_msg_type(app::dispatch::MSG::Append_Alert_Request);
	if(alert.base.parentid.empty())
	{
		response.result = false;
		response.error_describe = "parent id is empty";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_append_alert()->mutable_alert()->mutable_base()->mutable_parent()->set_id(std::stoul(alert.base.parentid,nullptr,0));
	message.mutable_request()->mutable_append_alert()->mutable_alert()->mutable_base()->set_entity_type(::pbmsg::EntityType(ALERT));
	if(!alert.base.name.empty())
		message.mutable_request()->mutable_append_alert()->mutable_alert()->mutable_base()->set_name(alert.base.name);
	if(!alert.describe.empty())
		message.mutable_request()->mutable_append_alert()->mutable_alert()->set_describe(alert.describe);
	if(!alert.alert_level.empty())
		message.mutable_request()->mutable_append_alert()->mutable_alert()->set_level((::pbmsg::AlertLevel)std::stoul(alert.alert_level,nullptr,0));
	if(!alert.alram_time.empty())
		message.mutable_request()->mutable_append_alert()->mutable_alert()->set_alram_time(alert.alram_time);
	if(!alert.use_cars.empty())
		message.mutable_request()->mutable_append_alert()->mutable_alert()->set_use_cars(std::stoul(alert.use_cars,nullptr,0));
	if(!alert.create_time.empty())
		message.mutable_request()->mutable_append_alert()->mutable_alert()->set_create_time(alert.create_time);
	if(!alert.group_id.empty())
		message.mutable_request()->mutable_append_alert()->mutable_alert()->mutable_group()->set_id(std::stoul(alert.group_id,nullptr,0));
	::pbmsg::Entity *entiry;
	for(std::list<ns__Account>::iterator itor = members.begin();itor!=members.end();itor++)
	{
		entiry = message.mutable_request()->mutable_append_alert()->add_members();
		entiry->set_id(std::stoul(itor->base.id,nullptr,0));
		entiry->set_name(itor->base.name);
		entiry->set_entity_type((::pbmsg::EntityType)std::stoul(itor->base.entity_type,nullptr,0));
		entiry->set_id(std::stoul(itor->base.parentid,nullptr,0));
	}
	std::string send_str;
	std::string recev_str;
	if(!message.IsInitialized())
	{
		response.result = false;
		response.error_describe = "Message can't be initialized";
		return SOAP_OK;
	}
	if(!message.SerializeToString(&send_str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	{
		std::lock_guard<std::mutex> lock(keep_tcp_connection_mutex);
		std::map<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> >::iterator itr = keep_tcp_connection.find(std::stoul(session_id,nullptr,0));
		if(itr == keep_tcp_connection.end())
		{
			response.result = false;
			response.error_describe = "session id not exist";
			return SOAP_OK;
		}
		std::tr1::shared_ptr<ConnectToAppServer> connecttoserver_ptr = itr->second;

		connecttoserver_ptr->send_request(send_str);

		message.Clear();

		bool break_while = false;
		
	create_timer([&break_while](){break_while=true;}, 5, false);
		
		do{
			recev_str = connecttoserver_ptr->get_response();
		}while((recev_str=="") && (!break_while));

	if(break_while)
	{
		LOG(ERROR)<<"socket write and read error";
		
		response.result = false;
		response.error_describe = "socket write and read error";
		return SOAP_OK;
	}

	}//unlock keep_tcp_connection_mutex
	LOG(INFO)<<"Parse data from protobuf protocol";
	if (message.ParseFromString(recev_str ))
	{	
		if(message.response().result())
		{
			response.result = true;
			response.session_id = int2str(message.session_id());
			response.data.alram_time = message.response().append_alert().alert().alram_time();
			response.data.create_time = message.response().append_alert().alert().create_time();
			response.data.describe = message.response().append_alert().alert().describe();
			response.data.group_id = int2str(message.response().append_alert().alert().group().id());
			response.data.alert_level = int2str(message.response().append_alert().alert().level());
			response.data.use_cars = int2str(message.response().append_alert().alert().use_cars());
			response.data.base.entity_type = int2str(message.response().append_alert().alert().base().entity_type());
			response.data.base.id = int2str(message.response().append_alert().alert().base().id());
			response.data.base.name = message.response().append_alert().alert().base().name();
			response.data.base.parentid = int2str(message.response().append_alert().alert().base().parent().id());
		}
		else
		{
			response.result = false;
			response.error_describe = message.response().error_describe();
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	return SOAP_OK;
}

int xiaofangService::Dispatch_Modify_Alert_Request(std::string session_id,
													ns__Alert alert, 
													ns__Normal_Response &response)
{
	LOG(INFO)<<"Dispatch_Modify_Alert_Request";
	
	app::dispatch::Message message;
	message.set_session_id(std::stoul(session_id,nullptr,0));
	unsigned long sequence_modify_alert = get_sequence();
	message.set_sequence(sequence_modify_alert);
	message.set_msg_type(app::dispatch::MSG::Modify_Alert_Request);
	if(!alert.base.name.empty())
		message.mutable_request()->mutable_modify_alert()->mutable_alert()->mutable_base()->set_name(alert.base.name);
	if(!alert.describe.empty())
		message.mutable_request()->mutable_modify_alert()->mutable_alert()->set_describe(alert.describe);
	if(!alert.alert_level.empty())
		message.mutable_request()->mutable_modify_alert()->mutable_alert()->set_level((::pbmsg::AlertLevel)std::stoul(alert.alert_level,nullptr,0));
	if(!alert.alram_time.empty())
		message.mutable_request()->mutable_modify_alert()->mutable_alert()->set_alram_time(alert.alram_time);
	if(!alert.use_cars.empty())
		message.mutable_request()->mutable_modify_alert()->mutable_alert()->set_use_cars(std::stoul(alert.use_cars,nullptr,0));
	if(!alert.create_time.empty())
		message.mutable_request()->mutable_modify_alert()->mutable_alert()->set_create_time(alert.create_time);

	std::string send_str;
	std::string recev_str;
	if(!message.IsInitialized())
	{
		response.result = false;
		response.error_describe = "Message can't be initialized";
		return SOAP_OK;
	}
	if(!message.SerializeToString(&send_str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	{
		std::lock_guard<std::mutex> lock(keep_tcp_connection_mutex);
		std::map<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> >::iterator itr = keep_tcp_connection.find(std::stoul(session_id,nullptr,0));
		if(itr == keep_tcp_connection.end())
		{
			response.result = false;
			response.error_describe = "session id not exist";
			return SOAP_OK;
		}
		std::tr1::shared_ptr<ConnectToAppServer> connecttoserver_ptr = itr->second;

		connecttoserver_ptr->send_request(send_str);

		message.Clear();

		bool break_while = false;
		
	create_timer([&break_while](){break_while=true;}, 5, false);
		
		do{
			recev_str = connecttoserver_ptr->get_response();
		}while((recev_str=="") && (!break_while));

	if(break_while)
	{
		LOG(ERROR)<<"socket write and read error";
		
		response.result = false;
		response.error_describe = "socket write and read error";
		return SOAP_OK;
	}

	}//unlock keep_tcp_connection_mutex
	LOG(INFO)<<"Parse data from protobuf protocol";
	if (message.ParseFromString(recev_str ))
	{	
		if(message.response().result())
		{
			response.result = true;
			response.session_id = int2str(message.session_id());
		}
		else
		{
			response.result = false;
			response.error_describe = message.response().error_describe();
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	return SOAP_OK;
}

int xiaofangService::Dispatch_Stop_Alert_Request(std::string session_id,
												std::string alert_id, 
												ns__Normal_Response &response)
{
	LOG(INFO)<<"Dispatch_Modify_Alert_Request";
	
	app::dispatch::Message message;
	message.set_session_id(std::stoul(session_id,nullptr,0));
	unsigned long sequence_stop_alert = get_sequence();
	message.set_sequence(sequence_stop_alert);
	message.set_msg_type(app::dispatch::MSG::Stop_Alert_Request);
	if(alert_id.empty())
	{
		response.result = false;
		response.error_describe = "no alert id input";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_stop_alert()->mutable_alert_id()->set_id(std::stoul(alert_id,nullptr,0));
	std::string send_str;
	std::string recev_str;
	if(!message.IsInitialized())
	{
		response.result = false;
		response.error_describe = "Message can't be initialized";
		return SOAP_OK;
	}
	if(!message.SerializeToString(&send_str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	{
		std::lock_guard<std::mutex> lock(keep_tcp_connection_mutex);
		std::map<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> >::iterator itr = keep_tcp_connection.find(std::stoul(session_id,nullptr,0));
		if(itr == keep_tcp_connection.end())
		{
			response.result = false;
			response.error_describe = "session id not exist";
			return SOAP_OK;
		}
		std::tr1::shared_ptr<ConnectToAppServer> connecttoserver_ptr = itr->second;

		connecttoserver_ptr->send_request(send_str);

		message.Clear();

		bool break_while = false;
		
	create_timer([&break_while](){break_while=true;}, 5, false);
		
		do{
			recev_str = connecttoserver_ptr->get_response();
		}while((recev_str=="") && (!break_while));

	if(break_while)
	{
		LOG(ERROR)<<"socket write and read error";
		
		response.result = false;
		response.error_describe = "socket write and read error";
		return SOAP_OK;
	}

	}//unlock keep_tcp_connection_mutex
	LOG(INFO)<<"Parse data from protobuf protocol";
	if (message.ParseFromString(recev_str ))
	{	
		if(message.response().result())
		{
			response.result = true;
			response.session_id = int2str(message.session_id());
		}
		else
		{
			response.result = false;
			response.error_describe = message.response().error_describe();
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	return SOAP_OK;
}

int xiaofangService::Dispatch_Alert_Overed_Notification(std::string session_id,
			ns__Dispatch_Alert_Overed_Notification_Response &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_History_Alert_Request(std::string session_id,
													std::string name, 
													std::string create_time_from, 
													std::string create_time_to, 
													std::string alram_time_from, 
													std::string alram_time_to, 
													std::string over_time_from, 
													std::string over_time_to, 
													ns__Dispatch_History_Alert_Request_Reponse &response)
{
	LOG(INFO)<<"Dispatch_History_Alert_Request";
	
	app::dispatch::Message message;
	message.set_session_id(std::stoul(session_id,nullptr,0));
	unsigned long sequence_history_alert = get_sequence();
	message.set_sequence(sequence_history_alert);
	message.set_msg_type(app::dispatch::MSG::History_Alerts_Request);
	message.mutable_request()->mutable_history_alerts();
	if(!name.empty())
		message.mutable_request()->mutable_history_alerts()->set_name(name);
	if(!create_time_from.empty())
		message.mutable_request()->mutable_history_alerts()->set_create_time_from(create_time_from);
	if(!create_time_to.empty())
		message.mutable_request()->mutable_history_alerts()->set_create_time_to(create_time_to);
	if(!alram_time_from.empty())
		message.mutable_request()->mutable_history_alerts()->set_alram_time_from(alram_time_from);
	if(!alram_time_to.empty())
		message.mutable_request()->mutable_history_alerts()->set_alram_time_to(alram_time_to);
	if(!over_time_from.empty())
		message.mutable_request()->mutable_history_alerts()->set_over_time_from(over_time_from);
	if(!over_time_to.empty())
		message.mutable_request()->mutable_history_alerts()->set_over_time_to(over_time_to);
	std::string send_str;
	std::string recev_str;
	if(!message.IsInitialized())
	{
		response.result = false;
		response.error_describe = "Message can't be initialized";
		return SOAP_OK;
	}
	if(!message.SerializeToString(&send_str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	{
		std::lock_guard<std::mutex> lock(keep_tcp_connection_mutex);
		std::map<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> >::iterator itr = keep_tcp_connection.find(std::stoul(session_id,nullptr,0));
		if(itr == keep_tcp_connection.end())
		{
			response.result = false;
			response.error_describe = "session id not exist";
			return SOAP_OK;
		}
		std::tr1::shared_ptr<ConnectToAppServer> connecttoserver_ptr = itr->second;

		connecttoserver_ptr->send_request(send_str);

		message.Clear();

		bool break_while = false;
		
	create_timer([&break_while](){break_while=true;}, 5, false);
		
		do{
			recev_str = connecttoserver_ptr->get_response();
		}while((recev_str=="") && (!break_while));

	if(break_while)
	{
		LOG(ERROR)<<"socket write and read error";
		
		response.result = false;
		response.error_describe = "socket write and read error";
		return SOAP_OK;
	}

	}//unlock keep_tcp_connection_mutex
	LOG(INFO)<<"Parse data from protobuf protocol";
	if (message.ParseFromString(recev_str ))
	{	
		if(message.response().result())
		{
			response.result = true;
			response.session_id = int2str(message.session_id());

			ns__HistoryAlert historyalert;
			response.size = int2str(message.response().history_alerts().history_alerts_size());
			for(int i = 0;i<message.response().history_alerts().history_alerts_size();i++)
			{
				historyalert.alram_time = message.response().history_alerts().history_alerts(i).alram_time();
				historyalert.create_time = message.response().history_alerts().history_alerts(i).create_time();
				historyalert.describe = message.response().history_alerts().history_alerts(i).describe();
				historyalert.id = int2str(message.response().history_alerts().history_alerts(i).id());
				historyalert.alert_level = int2str(message.response().history_alerts().history_alerts(i).level());
				historyalert.name = message.response().history_alerts().history_alerts(i).name();
				historyalert.over_time = message.response().history_alerts().history_alerts(i).over_time();
				historyalert.use_cars = int2str(message.response().history_alerts().history_alerts(i).use_cars());
				response.data.push_back(historyalert);
			}
		}
		else
		{
			response.result = false;
			response.error_describe = message.response().error_describe();
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	return SOAP_OK;
}

int xiaofangService::Dispatch_Alert_Request(std::string session_id,
											std::string alert_id, 
											ns__Dispatch_Alert_Request_Response &response)
{
	LOG(INFO)<<"Dispatch_Modify_Alert_Request";
	
	app::dispatch::Message message;
	message.set_session_id(std::stoul(session_id,nullptr,0));
	unsigned long sequence_alert = get_sequence();
	message.set_sequence(sequence_alert);
	message.set_msg_type(app::dispatch::MSG::History_Alert_Request);
	if(alert_id.empty())
	{
		response.result = false;
		response.error_describe = "no alert id input";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_history_alert()->set_history_alert_id(std::stoul(alert_id,nullptr,0));
	std::string send_str;
	std::string recev_str;
	if(!message.IsInitialized())
	{
		response.result = false;
		response.error_describe = "Message can't be initialized";
		return SOAP_OK;
	}
	if(!message.SerializeToString(&send_str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	{
		std::lock_guard<std::mutex> lock(keep_tcp_connection_mutex);
		std::map<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> >::iterator itr = keep_tcp_connection.find(std::stoul(session_id,nullptr,0));
		if(itr == keep_tcp_connection.end())
		{
			response.result = false;
			response.error_describe = "session id not exist";
			return SOAP_OK;
		}
		std::tr1::shared_ptr<ConnectToAppServer> connecttoserver_ptr = itr->second;

		connecttoserver_ptr->send_request(send_str);

		message.Clear();

		bool break_while = false;
		
	create_timer([&break_while](){break_while=true;}, 5, false);
		
		do{
			recev_str = connecttoserver_ptr->get_response();
		}while((recev_str=="") && (!break_while));

	if(break_while)
	{
		LOG(ERROR)<<"socket write and read error";
		
		response.result = false;
		response.error_describe = "socket write and read error";
		return SOAP_OK;
	}

	}//unlock keep_tcp_connection_mutex
	LOG(INFO)<<"Parse data from protobuf protocol";
	if (message.ParseFromString(recev_str ))
	{	
		if(message.response().result())
		{
			response.result = true;
			response.session_id = int2str(message.session_id());
			response.data.alram_time = message.response().history_alert().history_alert().alram_time();
			response.data.create_time = message.response().history_alert().history_alert().create_time();
			response.data.describe = message.response().history_alert().history_alert().describe();
			response.data.id = int2str(message.response().history_alert().history_alert().id());
			response.data.alert_level = int2str(message.response().history_alert().history_alert().level());
			response.data.name = message.response().history_alert().history_alert().name();
			response.data.over_time = message.response().history_alert().history_alert().over_time();
			response.data.use_cars = int2str(message.response().history_alert().history_alert().use_cars());
		}
		else
		{
			response.result = false;
			response.error_describe = message.response().error_describe();
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	return SOAP_OK;
}

int xiaofangService::Dispatch_History_Alert_Message_Request(std::string session_id,
															std::string history_alert_id, 
															std::string from_message_id, 
															std::string from_time, 
															std::string max_message_count, 
															ns__Dispatch_History_Alert_Message_Request_Response &response)
{
	LOG(INFO)<<"Dispatch_History_Alert_Message_Request";
	
	app::dispatch::Message message;
	message.set_session_id(std::stoul(session_id,nullptr,0));
	unsigned long sequence_history_alert_message = get_sequence();
	message.set_sequence(sequence_history_alert_message);
	message.set_msg_type(app::dispatch::MSG::History_Alert_Message_Request);
	if(!history_alert_id.empty())
		message.mutable_request()->mutable_history_alert_message()->set_history_alert_id(std::stoul(history_alert_id,nullptr,0));
	if(!from_message_id.empty())
		message.mutable_request()->mutable_history_alert_message()->set_from_message_id(std::stoul(from_message_id,nullptr,0));
	if(!from_time.empty())
		message.mutable_request()->mutable_history_alert_message()->set_from_time(from_time);
	if(!max_message_count.empty())
		message.mutable_request()->mutable_history_alert_message()->set_max_message_count(std::stoul(max_message_count,nullptr,0));
	std::string send_str;
	std::string recev_str;
	if(!message.IsInitialized())
	{
		response.result = false;
		response.error_describe = "Message can't be initialized";
		return SOAP_OK;
	}
	if(!message.SerializeToString(&send_str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	{
		std::lock_guard<std::mutex> lock(keep_tcp_connection_mutex);
		std::map<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> >::iterator itr = keep_tcp_connection.find(std::stoul(session_id,nullptr,0));
		if(itr == keep_tcp_connection.end())
		{
			response.result = false;
			response.error_describe = "session id not exist";
			return SOAP_OK;
		}
		std::tr1::shared_ptr<ConnectToAppServer> connecttoserver_ptr = itr->second;

		connecttoserver_ptr->send_request(send_str);

		message.Clear();

		bool break_while = false;
		
	create_timer([&break_while](){break_while=true;}, 5, false);
		
		do{
			recev_str = connecttoserver_ptr->get_response();
		}while((recev_str=="") && (!break_while));

	if(break_while)
	{
		LOG(ERROR)<<"socket write and read error";
		
		response.result = false;
		response.error_describe = "socket write and read error";
		return SOAP_OK;
	}

	}//unlock keep_tcp_connection_mutex
	LOG(INFO)<<"Parse data from protobuf protocol";
	if (message.ParseFromString(recev_str ))
	{	
		if(message.response().result())
		{
			response.result = true;
			response.session_id = int2str(message.session_id());

			response.history_alert_id = int2str(message.response().history_alert_message().history_alert_id());
			response.leave_message_count = int2str(message.response().history_alert_message().leave_message_count());
			response.size = int2str(message.response().history_alert_message().messages_size());

			ns__MediaMessage mediamessage;
			for(int i = 0;i<message.response().history_alert_message().messages_size();i++)
			{
				mediamessage.audio_length = int2str(message.response().history_alert_message().messages(i).audio_length());
				mediamessage.audio_uri = message.response().history_alert_message().messages(i).audio_uri();
				mediamessage.id = int2str(message.response().history_alert_message().messages(i).id());
				mediamessage.picture_uri = message.response().history_alert_message().messages(i).picture_uri();
				mediamessage.sender = message.response().history_alert_message().messages(i).sender();
				mediamessage.text = message.response().history_alert_message().messages(i).text();
				mediamessage.timestamp = message.response().history_alert_message().messages(i).timestamp();
				mediamessage.video_length = int2str(message.response().history_alert_message().messages(i).video_length());
				mediamessage.video_uri = message.response().history_alert_message().messages(i).video_uri();
				response.messages.push_back(mediamessage);
			}
		}
		else
		{
			response.result = false;
			response.error_describe = message.response().error_describe();
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	return SOAP_OK;
}

int xiaofangService::Dispatch_Delete_History_Alert_Request(std::string session_id,
															std::string history_alert_id, 
															ns__Normal_Response &response)
{
	LOG(INFO)<<"Dispatch_Delete_Group";
	
	app::dispatch::Message message;
	message.set_session_id(std::stoul(session_id,nullptr,0));
	unsigned long sequence_delete_history_alert = get_sequence();
	message.set_sequence(sequence_delete_history_alert);
	message.set_msg_type(app::dispatch::MSG::Delete_History_Alert_Request);
	if(history_alert_id.empty())
	{
		response.result = false;
		response.error_describe = "no history alert id input";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_delete_history_alert()->set_history_alert_id(std::stoul(history_alert_id,nullptr,0));
	std::string send_str;
	std::string recev_str;
	if(!message.IsInitialized())
	{
		response.result = false;
		response.error_describe = "Message can't be initialized";
		return SOAP_OK;
	}
	if(!message.SerializeToString(&send_str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	{
		std::lock_guard<std::mutex> lock(keep_tcp_connection_mutex);
		std::map<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> >::iterator itr = keep_tcp_connection.find(std::stoul(session_id,nullptr,0));
		if(itr == keep_tcp_connection.end())
		{
			response.result = false;
			response.error_describe = "session id not exist";
			return SOAP_OK;
		}
		std::tr1::shared_ptr<ConnectToAppServer> connecttoserver_ptr = itr->second;

		connecttoserver_ptr->send_request(send_str);

		message.Clear();

		bool break_while = false;
		
	create_timer([&break_while](){break_while=true;}, 5, false);
		
		do{
			recev_str = connecttoserver_ptr->get_response();
		}while((recev_str=="") && (!break_while));

	if(break_while)
	{
		LOG(ERROR)<<"socket write and read error";
		
		response.result = false;
		response.error_describe = "socket write and read error";
		return SOAP_OK;
	}

	}//unlock keep_tcp_connection_mutex
	LOG(INFO)<<"Parse data from protobuf protocol";
	if (message.ParseFromString(recev_str ))
	{	
		if(message.response().result())
		{
			response.result = true;
			response.session_id = int2str(message.session_id());
		}
		else
		{
			response.result = false;
			response.error_describe = message.response().error_describe();
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	return SOAP_OK;
}

int xiaofangService::Dispatch_Kick_Participant_Request(std::string session_id,
														std::string group_id,
							
														std::string account_id,
							
														ns__Normal_Response &response)
{
	LOG(INFO)<<"Dispatch_Delete_Group";
	
	app::dispatch::Message message;
	message.set_session_id(std::stoul(session_id,nullptr,0));
	unsigned long sequence_kick_participant = get_sequence(); 
	message.set_sequence(sequence_kick_participant);
	message.set_msg_type(app::dispatch::MSG::Kick_Participant_Request);
	if(group_id.empty())
	{
		response.result = false;
		response.error_describe = "no group id input";
		return SOAP_OK;
	}
	message.mutable_request()->mutable_kick_participant()->mutable_group_id()->set_id(std::stoul(group_id,nullptr,0));
	if(!account_id.empty())
		message.mutable_request()->mutable_kick_participant()->mutable_account_id()->set_id(std::stoul(account_id,nullptr,0));
	std::string send_str;
	std::string recev_str;
	if(!message.IsInitialized())
	{
		response.result = false;
		response.error_describe = "Message can't be initialized";
		return SOAP_OK;
	}
	if(!message.SerializeToString(&send_str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	{
		std::lock_guard<std::mutex> lock(keep_tcp_connection_mutex);
		std::map<unsigned long,std::tr1::shared_ptr<ConnectToAppServer> >::iterator itr = keep_tcp_connection.find(std::stoul(session_id,nullptr,0));
		if(itr == keep_tcp_connection.end())
		{
			response.result = false;
			response.error_describe = "session id not exist";
			return SOAP_OK;
		}
		std::tr1::shared_ptr<ConnectToAppServer> connecttoserver_ptr = itr->second;

		connecttoserver_ptr->send_request(send_str);

		message.Clear();

		bool break_while = false;
		
	create_timer([&break_while](){break_while=true;}, 5, false);
		
		do{
			recev_str = connecttoserver_ptr->get_response();
		}while((recev_str=="") && (!break_while));

	if(break_while)
	{
		LOG(ERROR)<<"socket write and read error";
		
		response.result = false;
		response.error_describe = "socket write and read error";
		return SOAP_OK;
	}

	}//unlock keep_tcp_connection_mutex
	LOG(INFO)<<"Parse data from protobuf protocol";
	if (message.ParseFromString(recev_str ))
	{	
		if(message.response().result())
		{
			response.result = true;
			response.session_id = int2str(message.session_id());
		}
		else
		{
			response.result = false;
			response.error_describe = message.response().error_describe();
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	return SOAP_OK;
}

