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

#define BACKLOG (100)	// Max. request backlog 
static unsigned long sequence = 1;

unsigned long get_sequence()
{
	return sequence++;
}

std::string int2str(int n)
{
    std::stringstream ss;
    std::string s;
    ss << n;
    ss >> s;
 
    return s;
}

int main(int argc,char* argv[])
{
	xiaofangService service;
   //	google::InitGoogleLogging("webservice");
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
			ns__Dispatch_Login_Response &loginResponse)
{
	int errorReturn;
	LOG(INFO)<<"Dispatch login";
	if(name.empty() || passwd.empty())
	{
		loginResponse.result = false;
		loginResponse.error_describe = "No name or password input";
		return SOAP_OK;
	}
	LOG(INFO)<<"Serializa data to protobuf protocol";
	std::string md5passwd = get_md5(passwd);	
	
	app::dispatch::Message message;
	message.set_sequence(get_sequence());
	message.set_msg_type(app::dispatch::MSG::Login_Request);
	message.mutable_request()->mutable_login()->set_username(name);
	message.mutable_request()->mutable_login()->set_password(md5passwd);
	
	std::string str;
	if(!message.SerializeToString(&str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		loginResponse.result = false;
		loginResponse.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}

	ConnectToAppServer connecttoserver;
	errorReturn = connecttoserver.transportData(str,message.ByteSize());
	if(errorReturn == -1)
	{
		loginResponse.result = false;
		loginResponse.error_describe = "Connection refused";
		return SOAP_OK;
	}
	else if(errorReturn == -2)
	{
		loginResponse.result = false;
		loginResponse.error_describe = "Write data to server error";
		return SOAP_OK;
	}
		else if(errorReturn == -3)
	{
		loginResponse.result = false;
		loginResponse.error_describe = "Read data len from server error";
		return SOAP_OK;
	}
		else if(errorReturn == -4)
	{
		loginResponse.result = false;
		loginResponse.error_describe = "Read data from server error";
		return SOAP_OK;
	}


	LOG(INFO)<<"Parse data from protobuf protobuf";
	if (message.ParseFromString(connecttoserver.get_recstr()) )
	{	
		if(message.msg_type() == app::dispatch::MSG::Login_Response)
		{
			if(message.response().result())
			{
				loginResponse.result = true;
				loginResponse.session_id = int2str(message.response().login().session_id());
				std::cout<<"loginResponse.session_id="<<loginResponse.session_id<<std::endl;
				loginResponse.id = int2str(message.response().login().self().account().id());
				std::cout<<"loginResponse.id="<<loginResponse.id<<std::endl;
				loginResponse.parentid = int2str(message.response().login().self().account().parent().id());
				std::cout<<"loginResponse.parentid="<<loginResponse.parentid<<std::endl;
			}
			else
			{
				LOG(ERROR)<<message.response().error_describe();;
				loginResponse.result = false;
				loginResponse.error_describe = message.response().error_describe();
				return SOAP_OK;
			}
		}
	}
	else
	{
		LOG(ERROR)<<message.InitializationErrorString();
		loginResponse.result = false;
		loginResponse.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	return SOAP_OK;
}

int xiaofangService::Dispatch_Logout(std::string session_id,
									std::string name, 
									std::string password, 
									ns__Normal_Response &response)
{
	int errorReturn;
	LOG(INFO)<<"Dispatch logout";
	if(name.empty() || password.empty())
	{
		response.result = false;
		response.error_describe = "No name or password input";
		return SOAP_OK;
	}
	std::string md5passwd = get_md5(password);	
	
	LOG(INFO)<<"Serializa data to protobuf protocol";
	app::dispatch::Message message;
	message.set_msg_type(app::dispatch::MSG::Logout_Request);
	message.set_session_id(atoi(session_id.c_str()));
	message.set_sequence(get_sequence());
	
	std::string str;
	if(!message.SerializeToString(&str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	ConnectToAppServer connecttoserver;
	errorReturn = connecttoserver.transportData(str,message.ByteSize());
	if(errorReturn == -1)
	{
		response.result = false;
		response.error_describe = "Connection refused";
		return SOAP_OK;
	}
	else if(errorReturn == -2)
	{
		response.result = false;
		response.error_describe = "Write data to server error";
		return SOAP_OK;
	}
		else if(errorReturn == -3)
	{
		response.result = false;
		response.error_describe = "Read data len from server error";
		return SOAP_OK;
	}
		else if(errorReturn == -4)
	{
		response.result = false;
		response.error_describe = "Read data from server error";
		return SOAP_OK;
	}
	LOG(INFO)<<"Parse data from protobuf protobuf";
	if (message.ParseFromString(connecttoserver.get_recstr()) )
	{	
		if(message.msg_type() == app::dispatch::MSG::Login_Response)
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
	int errorReturn;
	LOG(INFO)<<"Dispatch_Entity_Request";
	app::dispatch::Message message;
	message.set_msg_type(app::dispatch::MSG::Entity_Request);
	message.mutable_request()->mutable_entity()->mutable_id()->set_id(atoi(id.c_str()));
	message.set_sequence(get_sequence());
	
	std::string str;
	if(!message.SerializeToString(&str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	ConnectToAppServer connecttoserver;
	errorReturn = connecttoserver.transportData(str,message.ByteSize());
	if(errorReturn == -1)
	{
		response.result = false;
		response.error_describe = "Connection refused";
		return SOAP_OK;
	}
	else if(errorReturn == -2)
	{
		response.result = false;
		response.error_describe = "Write data to server error";
		return SOAP_OK;
	}
		else if(errorReturn == -3)
	{
		response.result = false;
		response.error_describe = "Read data len from server error";
		return SOAP_OK;
	}
		else if(errorReturn == -4)
	{
		response.result = false;
		response.error_describe = "Read data from server error";
		return SOAP_OK;
	}
	LOG(INFO)<<"Parse data from protobuf protobuf";
	if (message.ParseFromString(connecttoserver.get_recstr()) )
	{	
		if(message.msg_type() == app::dispatch::MSG::Entity_Response)
		{
			if(message.response().result())
			{
				response.result = true;
				response.data.id.entity_type = (ns__EntityType)message.response().entity().data().id().entity_type();
				response.session_id = int2str(message.session_id());
				if(response.data.id.entity_type == UNIT)
				{
					response.data.unit.base.entity_type = (ns__EntityType)message.response().entity().data().unit().base().entity_type();
					response.data.unit.base.id = int2str(message.response().entity().data().unit().base().id());
					response.data.unit.base.name = message.response().entity().data().unit().base().name();
					response.data.unit.base.parentid = int2str(message.response().entity().data().unit().base().parent().id());

					ns__Entity member;
					response.data.unit.size = int2str(message.response().entity().data().unit().members_size());
					for(int i=0;i<atoi(response.data.unit.size.c_str());i++)
					{
						member.entity_type = (ns__EntityType)message.response().entity().data().unit().members(i).entity_type();
						member.id = int2str(message.response().entity().data().unit().members(i).id());
						member.name = message.response().entity().data().unit().members(i).name();
						member.parentid = int2str(message.response().entity().data().unit().members(i).parent().id());
						response.data.unit.members.push_back(member);
					}
				}
				else if(response.data.id.entity_type == ACCOUNT)
				{
					response.data.account.base.entity_type = (ns__EntityType)message.response().entity().data().account().base().entity_type();
					response.data.account.base.id = int2str(message.response().entity().data().account().base().id());
					response.data.account.base.name = message.response().entity().data().account().base().name();
					response.data.account.base.parentid = int2str(message.response().entity().data().account().base().parent().id());
					response.data.account.number = message.response().entity().data().account().number();
					response.data.account.priority = int2str(message.response().entity().data().account().priority());
					response.data.account.short_number = message.response().entity().data().account().short_number();
					response.data.account.sip_status = (ns__RegisterStatus)message.response().entity().data().account().sip_status();
					response.data.account.status = (ns__RegisterStatus)message.response().entity().data().account().status();
					response.data.account.token_privilege = (ns__TokenPrivilege)message.response().entity().data().account().token_privilege();
				}
				else if(response.data.id.entity_type == USER)
				{
					response.data.user.account.entity_type = (ns__EntityType)message.response().entity().data().user().account().entity_type();
					response.data.user.account.id = int2str(message.response().entity().data().user().account().id());
					response.data.user.account.name = message.response().entity().data().user().account().name();
					response.data.user.account.parentid = int2str(message.response().entity().data().user().account().parent().id());
					response.data.user.base.entity_type = (ns__EntityType)message.response().entity().data().user().base().entity_type();
					response.data.user.base.id = int2str(message.response().entity().data().user().base().id());
					response.data.user.base.name = message.response().entity().data().user().base().name();
					response.data.user.base.parentid = int2str(message.response().entity().data().user().base().parent().id());
					response.data.user.status = (ns__RegisterStatus)message.response().entity().data().user().status();
				}
				else if(response.data.id.entity_type == GROUP)
				{
					ns__Participant participant;
					response.data.group.id = int2str(message.response().entity().data().group().base().id());
					response.data.group.name = message.response().entity().data().group().base().name();
					response.data.group.number = message.response().entity().data().group().number();
					response.data.group.owner_id = int2str(message.response().entity().data().group().owner().id());

					response.data.group.size = int2str(message.response().entity().data().group().participants_size());
					for(int i=0;i<atoi(response.data.group.size.c_str());i++)
					{
						participant.account.entity_type = (ns__EntityType)message.response().entity().data().group().participants(i).account().entity_type();
						participant.account.id = int2str(message.response().entity().data().group().participants(i).account().id());
						participant.account.name = message.response().entity().data().group().participants(i).account().name();
						participant.account.parentid = int2str(message.response().entity().data().group().participants(i).account().parent().id());
						participant.call_privilege = (ns__CallPrivilege)message.response().entity().data().group().participants(i).call_privilege();
						participant.priority = int2str(message.response().entity().data().group().participants(i).priority());
						participant.status = (ns__SessionStatus)message.response().entity().data().group().participants(i).status();
						participant.token_privilege = (ns__TokenPrivilege)message.response().entity().data().group().participants(i).token_privilege();
						response.data.group.participants.push_back(participant);
					}
					response.data.group.record_status = (ns__RecordStatus)message.response().entity().data().group().record_status();
					response.data.group.record_type = (ns__RecordType)message.response().entity().data().group().record_type();
					response.data.group.short_number = message.response().entity().data().group().short_number();
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
	int errorReturn;
	int i = 0;
	LOG(INFO)<<"Dispatch_Append_Group";
	app::dispatch::Message message;
	message.set_sequence(get_sequence());
	message.set_session_id(atoi(session_id.c_str()));
	message.set_msg_type(app::dispatch::MSG::Append_Group_Request);
	message.mutable_request()->mutable_append_group()->mutable_group()->mutable_base()->set_name(group.name);
	message.mutable_request()->mutable_append_group()->mutable_group()->mutable_owner()->set_id(atoi(group.owner_id.c_str()));
	if(atoi(group.size.c_str())!=0)
	{
		for(std::list<ns__Participant>::iterator itor = group.participants.begin();itor!=group.participants.end();itor++)
		{		
			message.mutable_request()->mutable_append_group()->mutable_group()->mutable_participants(i)->set_priority(atoi((itor->priority).c_str()));
			message.mutable_request()->mutable_append_group()->mutable_group()->mutable_participants(i)->set_call_privilege((::pbmsg::CallPrivilege)itor->call_privilege);
			message.mutable_request()->mutable_append_group()->mutable_group()->mutable_participants(i)->set_token_privilege((::pbmsg::TokenPrivilege)itor->token_privilege);
		}	
		i++;
	}
	std::string str;
	if(!message.SerializeToString(&str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	ConnectToAppServer connecttoserver;
	errorReturn = connecttoserver.transportData(str,message.ByteSize());
	if(errorReturn == -1)
	{
		response.result = false;
		response.error_describe = "Connection refused";
		return SOAP_OK;
	}
	else if(errorReturn == -2)
	{
		response.result = false;
		response.error_describe = "Write data to server error";
		return SOAP_OK;
	}
		else if(errorReturn == -3)
	{
		response.result = false;
		response.error_describe = "Read data len from server error";
		return SOAP_OK;
	}
		else if(errorReturn == -4)
	{
		response.result = false;
		response.error_describe = "Read data from server error";
		return SOAP_OK;
	}
	LOG(INFO)<<"Parse data from protobuf protobuf";
	if (message.ParseFromString(connecttoserver.get_recstr()) )
	{	
		if(message.msg_type() == app::dispatch::MSG::Append_Group_Response)
		{
			if(message.response().result())
			{
				response.result = true;
				response.session_id = int2str(message.session_id());
				response.data.id = int2str(message.response().append_group().group().base().id());
				response.data.name = message.response().append_group().group().base().name();
				response.data.number = message.response().append_group().group().number();
				response.data.owner_id = int2str(message.response().append_group().group().owner().id());
				response.data.short_number = message.response().append_group().group().short_number();
				response.data.record_status = (ns__RecordStatus)message.response().append_group().group().record_status();
				response.data.record_type = (ns__RecordType)message.response().append_group().group().record_type();
				response.data.size = int2str(message.response().append_group().group().participants_size());
				ns__Participant participant;
				for(int i=0;i<message.response().append_group().group().participants_size();i++)
				{
					participant.call_privilege = (ns__CallPrivilege)message.response().append_group().group().participants(i).call_privilege();
					participant.priority = int2str(message.response().append_group().group().participants(i).priority());
					participant.status = (ns__SessionStatus)message.response().append_group().group().participants(i).status();
					participant.token_privilege = (ns__TokenPrivilege)message.response().append_group().group().participants(i).token_privilege();
					participant.account.entity_type = (ns__EntityType)message.response().append_group().group().participants(i).account().entity_type();
					participant.account.id = int2str(message.response().append_group().group().participants(i).account().id());
					participant.account.name = message.response().append_group().group().participants(i).account().name();
					participant.account.parentid = int2str(message.response().append_group().group().participants(i).account().parent().id());
					response.data.participants.push_back(participant);
				}
			}
			else
			{
				response.result = false;
				response.error_describe = message.response().error_describe();
			}
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
	int errorReturn;
	int i = 0;
	LOG(INFO)<<"Dispatch_Modify_Group";
	app::dispatch::Message message;
	message.set_sequence(get_sequence());
	message.set_session_id(atoi(session_id.c_str()));
	message.set_msg_type(app::dispatch::MSG::Modify_Group_Request);
	message.mutable_request()->mutable_modify_group()->mutable_group()->mutable_base()->set_name(group.name);
	message.mutable_request()->mutable_modify_group()->mutable_group()->mutable_owner()->set_id(atoi(group.owner_id.c_str()));
	std::string str;
	if(!message.SerializeToString(&str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	ConnectToAppServer connecttoserver;
	errorReturn = connecttoserver.transportData(str,message.ByteSize());
	if(errorReturn == -1)
	{
		response.result = false;
		response.error_describe = "Connection refused";
		return SOAP_OK;
	}
	else if(errorReturn == -2)
	{
		response.result = false;
		response.error_describe = "Write data to server error";
		return SOAP_OK;
	}
		else if(errorReturn == -3)
	{
		response.result = false;
		response.error_describe = "Read data len from server error";
		return SOAP_OK;
	}
		else if(errorReturn == -4)
	{
		response.result = false;
		response.error_describe = "Read data from server error";
		return SOAP_OK;
	}
	LOG(INFO)<<"Parse data from protobuf protobuf";
	if (message.ParseFromString(connecttoserver.get_recstr()) )
	{	
		if(message.msg_type() == app::dispatch::MSG::Modify_Group_Response)
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
			}
			else
			{
				response.result = false;
				response.error_describe = message.response().error_describe();
			}
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
	int errorReturn;
	int i = 0;
	LOG(INFO)<<"Dispatch_Modify_Participants";
	app::dispatch::Message message;
	message.set_sequence(get_sequence());
	message.set_session_id(atoi(session_id.c_str()));
	message.set_msg_type(app::dispatch::MSG::Modify_Participants_Request);
	if(atoi(request.size.c_str())!=0)
	{
		for(std::list<ns__Participant>::iterator itor = request.participants.begin();itor!=request.participants.end();itor++)
		{		
			message.mutable_request()->mutable_append_group()->mutable_group()->mutable_participants(i)->set_priority(atoi((itor->priority).c_str()));
			message.mutable_request()->mutable_append_group()->mutable_group()->mutable_participants(i)->set_call_privilege((::pbmsg::CallPrivilege)itor->call_privilege);
			message.mutable_request()->mutable_append_group()->mutable_group()->mutable_participants(i)->set_token_privilege((::pbmsg::TokenPrivilege)itor->token_privilege);
		}	
		i++;
	}
	std::string str;
	if(!message.SerializeToString(&str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	ConnectToAppServer connecttoserver;
	errorReturn = connecttoserver.transportData(str,message.ByteSize());
	if(errorReturn == -1)
	{
		response.result = false;
		response.error_describe = "Connection refused";
		return SOAP_OK;
	}
	else if(errorReturn == -2)
	{
		response.result = false;
		response.error_describe = "Write data to server error";
		return SOAP_OK;
	}
		else if(errorReturn == -3)
	{
		response.result = false;
		response.error_describe = "Read data len from server error";
		return SOAP_OK;
	}
		else if(errorReturn == -4)
	{
		response.result = false;
		response.error_describe = "Read data from server error";
		return SOAP_OK;
	}
	LOG(INFO)<<"Parse data from protobuf protobuf";
	if (message.ParseFromString(connecttoserver.get_recstr()) )
	{	
		if(message.msg_type() == app::dispatch::MSG::Modify_Participants_Response)
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
					participant.status = (ns__SessionStatus)message.response().append_group().group().participants(i).status();
					participant.token_privilege = (ns__TokenPrivilege)message.response().append_group().group().participants(i).token_privilege();
					participant.account.entity_type = (ns__EntityType)message.response().append_group().group().participants(i).account().entity_type();
					participant.account.id = int2str(message.response().append_group().group().participants(i).account().id());
					participant.account.name = message.response().append_group().group().participants(i).account().name();
					participant.account.parentid = int2str(message.response().append_group().group().participants(i).account().parent().id());
					response.data.participants.push_back(participant);
				}
			}
			else
			{
				response.result = false;
				response.error_describe = message.response().error_describe();
			}
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
	message.set_session_id(atoi(session_id.c_str()));
	message.set_sequence(get_sequence());
	message.set_msg_type(app::dispatch::MSG::Delete_Group_Request);
	message.mutable_request()->mutable_delete_group()->mutable_group_id()->set_id(atoi(group_id.c_str()));
	std::string str;
	int errorReturn;
	if(!message.SerializeToString(&str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	ConnectToAppServer connecttoserver;
	errorReturn = connecttoserver.transportData(str,message.ByteSize());
	if(errorReturn == -1)
	{
		response.result = false;
		response.error_describe = "Connection refused";
		return SOAP_OK;
	}
	else if(errorReturn == -2)
	{
		response.result = false;
		response.error_describe = "Write data to server error";
		return SOAP_OK;
	}
		else if(errorReturn == -3)
	{
		response.result = false;
		response.error_describe = "Read data len from server error";
		return SOAP_OK;
	}
		else if(errorReturn == -4)
	{
		response.result = false;
		response.error_describe = "Read data from server error";
		return SOAP_OK;
	}
	LOG(INFO)<<"Parse data from protobuf protobuf";
	if (message.ParseFromString(connecttoserver.get_recstr()) )
	{	
		if(message.msg_type() == app::dispatch::MSG::Modify_Participants_Response)
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
	message.set_session_id(atoi(session_id.c_str()));
	message.set_sequence(get_sequence());
	message.set_msg_type(app::dispatch::MSG::Media_Message_Request);
	message.mutable_request()->mutable_media_message()->mutable_id()->set_id(atoi(group_id.c_str()));
	message.mutable_request()->mutable_media_message()->set_from_message_id(atoi(from_message_id.c_str()));
	message.mutable_request()->mutable_media_message()->set_from_time(from_time);
	message.mutable_request()->mutable_media_message()->set_max_message_count(atoi(max_message_count.c_str()));
	std::string str;
	int errorReturn;
	if(!message.SerializeToString(&str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	ConnectToAppServer connecttoserver;
	errorReturn = connecttoserver.transportData(str,message.ByteSize());
	if(errorReturn == -1)
	{
		response.result = false;
		response.error_describe = "Connection refused";
		return SOAP_OK;
	}
	else if(errorReturn == -2)
	{
		response.result = false;
		response.error_describe = "Write data to server error";
		return SOAP_OK;
	}
		else if(errorReturn == -3)
	{
		response.result = false;
		response.error_describe = "Read data len from server error";
		return SOAP_OK;
	}
		else if(errorReturn == -4)
	{
		response.result = false;
		response.error_describe = "Read data from server error";
		return SOAP_OK;
	}
	LOG(INFO)<<"Parse data from protobuf protobuf";
	if (message.ParseFromString(connecttoserver.get_recstr()) )
	{	
		if(message.msg_type() == app::dispatch::MSG::Media_Message_Response)
		{
			if(message.response().result())
			{
				response.result = true;
				response.session_id = int2str(message.session_id());
				response.size = int2str(message.response().media_message().messages_size());
				response.leave_message_count = int2str(message.response().media_message().leave_message_count());
				ns__MediaMessage mediamessage;
				for(int i=0;i<message.response().media_message().messages_size();i++)
				{
					mediamessage.audio_length = int2str(message.response().media_message().messages(i).audio_length());
					mediamessage.audio_uri = message.response().media_message().messages(i).audio_uri();
					mediamessage.id = int2str(message.response().media_message().messages(i).id());
					mediamessage.picture_uri = message.response().media_message().messages(i).picture_uri();
					mediamessage.sender = message.response().media_message().messages(i).sender();
					mediamessage.text = message.response().media_message().messages(i).text();
					mediamessage.timestamp = message.response().media_message().messages(i).timestamp();
					mediamessage.video_length = int2str(message.response().media_message().messages(i).video_length());
					mediamessage.video_uri = message.response().media_message().messages(i).video_uri();
					response.data.push_back(mediamessage);
				}
			}
			else
			{
				response.result = false;
				response.error_describe = message.response().error_describe();
			}
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
	return SOAP_OK;
}

int xiaofangService::Dispatch_Drop_Participant_Request(std::string session_id,
														std::string group_id, 
														std::string account_id, 
														ns__Normal_Response &response)
{
	LOG(INFO)<<"Dispatch_Drop_Participant_Request";
	app::dispatch::Message message;
	message.set_session_id(atoi(session_id.c_str()));
	message.set_sequence(get_sequence());
	message.set_msg_type(app::dispatch::MSG::Invite_Participant_Request);
	message.mutable_request()->mutable_invite_participant()->mutable_group_id()->set_id(atoi(group_id.c_str()));
	message.mutable_request()->mutable_invite_participant()->mutable_account_id()->set_id(atoi(account_id.c_str()));
	
	std::string str;
	int errorReturn;
	if(!message.SerializeToString(&str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	ConnectToAppServer connecttoserver;
	errorReturn = connecttoserver.transportData(str,message.ByteSize());
	if(errorReturn == -1)
	{
		response.result = false;
		response.error_describe = "Connection refused";
		return SOAP_OK;
	}
	else if(errorReturn == -2)
	{
		response.result = false;
		response.error_describe = "Write data to server error";
		return SOAP_OK;
	}
		else if(errorReturn == -3)
	{
		response.result = false;
		response.error_describe = "Read data len from server error";
		return SOAP_OK;
	}
		else if(errorReturn == -4)
	{
		response.result = false;
		response.error_describe = "Read data from server error";
		return SOAP_OK;
	}
	LOG(INFO)<<"Parse data from protobuf protobuf";
	if (message.ParseFromString(connecttoserver.get_recstr()) )
	{	
		if(message.msg_type() == app::dispatch::MSG::Invite_Participant_Response)
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
	message.set_session_id(atoi(session_id.c_str()));
	message.set_sequence(get_sequence());
	message.set_msg_type(app::dispatch::MSG::Release_Participant_Token_Request);
	message.mutable_request()->mutable_release_participant_token()->mutable_group_id()->set_id(atoi(group_id.c_str()));
	message.mutable_request()->mutable_release_participant_token()->mutable_account_id()->set_id(atoi(account_id.c_str()));
	
	std::string str;
	int errorReturn;
	if(!message.SerializeToString(&str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	ConnectToAppServer connecttoserver;
	errorReturn = connecttoserver.transportData(str,message.ByteSize());
	if(errorReturn == -1)
	{
		response.result = false;
		response.error_describe = "Connection refused";
		return SOAP_OK;
	}
	else if(errorReturn == -2)
	{
		response.result = false;
		response.error_describe = "Write data to server error";
		return SOAP_OK;
	}
		else if(errorReturn == -3)
	{
		response.result = false;
		response.error_describe = "Read data len from server error";
		return SOAP_OK;
	}
		else if(errorReturn == -4)
	{
		response.result = false;
		response.error_describe = "Read data from server error";
		return SOAP_OK;
	}
	LOG(INFO)<<"Parse data from protobuf protobuf";
	if (message.ParseFromString(connecttoserver.get_recstr()) )
	{	
		if(message.msg_type() == app::dispatch::MSG::Release_Participant_Token_Response)
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
	message.set_session_id(atoi(session_id.c_str()));
	message.set_sequence(get_sequence());
	message.set_msg_type(app::dispatch::MSG::Appoint_Participant_Speak_Request);
	message.mutable_request()->mutable_appoint_participant_speak()->mutable_group_id()->set_id(atoi(group_id.c_str()));
	message.mutable_request()->mutable_appoint_participant_speak()->mutable_account_id()->set_id(atoi(account_id.c_str()));
	
	std::string str;
	int errorReturn;
	if(!message.SerializeToString(&str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	ConnectToAppServer connecttoserver;
	errorReturn = connecttoserver.transportData(str,message.ByteSize());
	if(errorReturn == -1)
	{
		response.result = false;
		response.error_describe = "Connection refused";
		return SOAP_OK;
	}
	else if(errorReturn == -2)
	{
		response.result = false;
		response.error_describe = "Write data to server error";
		return SOAP_OK;
	}
		else if(errorReturn == -3)
	{
		response.result = false;
		response.error_describe = "Read data len from server error";
		return SOAP_OK;
	}
		else if(errorReturn == -4)
	{
		response.result = false;
		response.error_describe = "Read data from server error";
		return SOAP_OK;
	}
	LOG(INFO)<<"Parse data from protobuf protobuf";
	if (message.ParseFromString(connecttoserver.get_recstr()) )
	{	
		if(message.msg_type() == app::dispatch::MSG::Appoint_Participant_Speak_Response)
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
	message.set_session_id(atoi(session_id.c_str()));
	message.set_sequence(get_sequence());
	message.set_msg_type(app::dispatch::MSG::Jion_Group_Request);
	message.mutable_request()->mutable_jion_group()->mutable_group_id()->set_id(atoi(group_id.c_str()));
	
	std::string str;
	int errorReturn;
	if(!message.SerializeToString(&str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	ConnectToAppServer connecttoserver;
	errorReturn = connecttoserver.transportData(str,message.ByteSize());
	if(errorReturn == -1)
	{
		response.result = false;
		response.error_describe = "Connection refused";
		return SOAP_OK;
	}
	else if(errorReturn == -2)
	{
		response.result = false;
		response.error_describe = "Write data to server error";
		return SOAP_OK;
	}
		else if(errorReturn == -3)
	{
		response.result = false;
		response.error_describe = "Read data len from server error";
		return SOAP_OK;
	}
		else if(errorReturn == -4)
	{
		response.result = false;
		response.error_describe = "Read data from server error";
		return SOAP_OK;
	}
	LOG(INFO)<<"Parse data from protobuf protobuf";
	if (message.ParseFromString(connecttoserver.get_recstr()) )
	{	
		if(message.msg_type() == app::dispatch::MSG::Jion_Group_Response)
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
	message.set_session_id(atoi(session_id.c_str()));
	message.set_sequence(get_sequence());
	message.set_msg_type(app::dispatch::MSG::Leave_Group_Request);
	message.mutable_request()->mutable_leave_group()->mutable_group_id()->set_id(atoi(group_id.c_str()));
	
	std::string str;
	int errorReturn;
	if(!message.SerializeToString(&str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	ConnectToAppServer connecttoserver;
	errorReturn = connecttoserver.transportData(str,message.ByteSize());
	if(errorReturn == -1)
	{
		response.result = false;
		response.error_describe = "Connection refused";
		return SOAP_OK;
	}
	else if(errorReturn == -2)
	{
		response.result = false;
		response.error_describe = "Write data to server error";
		return SOAP_OK;
	}
		else if(errorReturn == -3)
	{
		response.result = false;
		response.error_describe = "Read data len from server error";
		return SOAP_OK;
	}
		else if(errorReturn == -4)
	{
		response.result = false;
		response.error_describe = "Read data from server error";
		return SOAP_OK;
	}
	LOG(INFO)<<"Parse data from protobuf protobuf";
	if (message.ParseFromString(connecttoserver.get_recstr()) )
	{	
		if(message.msg_type() == app::dispatch::MSG::Leave_Group_Response)
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
	message.set_session_id(atoi(session_id.c_str()));
	message.set_sequence(get_sequence());
	message.set_msg_type(app::dispatch::MSG::Send_Message_Request);
	message.mutable_request()->mutable_send_message()->mutable_id()->set_id(atoi(group_id.c_str()));
	message.mutable_request()->mutable_send_message()->mutable_msg()->set_sender(mediamessage.sender);
	message.mutable_request()->mutable_send_message()->mutable_msg()->set_text(mediamessage.text);
	message.mutable_request()->mutable_send_message()->mutable_msg()->set_picture_uri(mediamessage.picture_uri);
	message.mutable_request()->mutable_send_message()->mutable_msg()->set_audio_uri(mediamessage.audio_uri);
	message.mutable_request()->mutable_send_message()->mutable_msg()->set_audio_length(atoi(mediamessage.audio_length.c_str()));
	message.mutable_request()->mutable_send_message()->mutable_msg()->set_video_uri(mediamessage.video_uri);
	message.mutable_request()->mutable_send_message()->mutable_msg()->set_video_length(atoi(mediamessage.video_length.c_str()));
	
	std::string str;
	int errorReturn;
	if(!message.SerializeToString(&str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	ConnectToAppServer connecttoserver;
	errorReturn = connecttoserver.transportData(str,message.ByteSize());
	if(errorReturn == -1)
	{
		response.result = false;
		response.error_describe = "Connection refused";
		return SOAP_OK;
	}
	else if(errorReturn == -2)
	{
		response.result = false;
		response.error_describe = "Write data to server error";
		return SOAP_OK;
	}
		else if(errorReturn == -3)
	{
		response.result = false;
		response.error_describe = "Read data len from server error";
		return SOAP_OK;
	}
		else if(errorReturn == -4)
	{
		response.result = false;
		response.error_describe = "Read data from server error";
		return SOAP_OK;
	}
	LOG(INFO)<<"Parse data from protobuf protobuf";
	if (message.ParseFromString(connecttoserver.get_recstr()) )
	{	
		if(message.msg_type() == app::dispatch::MSG::Send_Message_Response)
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
	message.set_session_id(atoi(session_id.c_str()));
	message.set_sequence(get_sequence());
	message.set_msg_type(app::dispatch::MSG::Start_Record_Request);
	message.mutable_request()->mutable_start_record()->mutable_group_id()->set_id(atoi(group_id.c_str()));
	
	std::string str;
	int errorReturn;
	if(!message.SerializeToString(&str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	ConnectToAppServer connecttoserver;
	errorReturn = connecttoserver.transportData(str,message.ByteSize());
	if(errorReturn == -1)
	{
		response.result = false;
		response.error_describe = "Connection refused";
		return SOAP_OK;
	}
	else if(errorReturn == -2)
	{
		response.result = false;
		response.error_describe = "Write data to server error";
		return SOAP_OK;
	}
		else if(errorReturn == -3)
	{
		response.result = false;
		response.error_describe = "Read data len from server error";
		return SOAP_OK;
	}
		else if(errorReturn == -4)
	{
		response.result = false;
		response.error_describe = "Read data from server error";
		return SOAP_OK;
	}
	LOG(INFO)<<"Parse data from protobuf protobuf";
	if (message.ParseFromString(connecttoserver.get_recstr()) )
	{	
		if(message.msg_type() == app::dispatch::MSG::Start_Record_Response)
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
	message.set_session_id(atoi(session_id.c_str()));
	message.set_sequence(get_sequence());
	message.set_msg_type(app::dispatch::MSG::Stop_Record_Request);
	message.mutable_request()->mutable_start_record()->mutable_group_id()->set_id(atoi(group_id.c_str()));
	
	std::string str;
	int errorReturn;
	if(!message.SerializeToString(&str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	ConnectToAppServer connecttoserver;
	errorReturn = connecttoserver.transportData(str,message.ByteSize());
	if(errorReturn == -1)
	{
		response.result = false;
		response.error_describe = "Connection refused";
		return SOAP_OK;
	}
	else if(errorReturn == -2)
	{
		response.result = false;
		response.error_describe = "Write data to server error";
		return SOAP_OK;
	}
		else if(errorReturn == -3)
	{
		response.result = false;
		response.error_describe = "Read data len from server error";
		return SOAP_OK;
	}
		else if(errorReturn == -4)
	{
		response.result = false;
		response.error_describe = "Read data from server error";
		return SOAP_OK;
	}
	LOG(INFO)<<"Parse data from protobuf protobuf";
	if (message.ParseFromString(connecttoserver.get_recstr()) )
	{	
		if(message.msg_type() == app::dispatch::MSG::Stop_Record_Response)
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

int xiaofangService::Dispatch_Subscribe_Account_Info_Request(std::string session_id,
															bool subscribing, 
															std::list<std::string >account_id, 
															enum ns__SubscribeType type, 
															std::string ttl, 
															ns__Normal_Response &response)
{
	return SOAP_OK;
}

int xiaofangService::Dispatch_Account_Info_Notification(std::string session_id, ns__Dispatch_Account_Info_Notification_Response &response)
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
	message.set_session_id(atoi(session_id.c_str()));
	message.set_sequence(get_sequence());
	message.set_msg_type(app::dispatch::MSG::Append_Alert_Request);
	message.mutable_request()->mutable_append_alert()->mutable_alert()->mutable_base()->set_name(alert.base.name);
	message.mutable_request()->mutable_append_alert()->mutable_alert()->set_describe(alert.describe);
	message.mutable_request()->mutable_append_alert()->mutable_alert()->set_level((::pbmsg::AlertLevel)alert.level);
	message.mutable_request()->mutable_append_alert()->mutable_alert()->set_alram_time(alert.alram_time);
	message.mutable_request()->mutable_append_alert()->mutable_alert()->set_use_cars(atoi(alert.use_cars.c_str()));
	message.mutable_request()->mutable_append_alert()->mutable_alert()->set_create_time(alert.create_time);
	message.mutable_request()->mutable_append_alert()->mutable_alert()->mutable_group()->set_id(atoi(alert.group_id.c_str()));

	int i=0;
	for(std::list<ns__Account>::iterator itor = members.begin();itor!=members.end();itor++)
	{
		message.mutable_request()->mutable_append_alert()->mutable_members(i)->set_id(atoi((itor->base.id).c_str()));
		message.mutable_request()->mutable_append_alert()->mutable_members(i)->set_name(itor->base.name);
		message.mutable_request()->mutable_append_alert()->mutable_members(i)->set_entity_type((::pbmsg::EntityType)itor->base.entity_type);
		message.mutable_request()->mutable_append_alert()->mutable_members(i)->mutable_parent()->set_id(atoi(itor->base.parentid.c_str()));
	}
	std::string str;
	int errorReturn;
	if(!message.SerializeToString(&str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	ConnectToAppServer connecttoserver;
	errorReturn = connecttoserver.transportData(str,message.ByteSize());
	if(errorReturn == -1)
	{
		response.result = false;
		response.error_describe = "Connection refused";
		return SOAP_OK;
	}
	else if(errorReturn == -2)
	{
		response.result = false;
		response.error_describe = "Write data to server error";
		return SOAP_OK;
	}
		else if(errorReturn == -3)
	{
		response.result = false;
		response.error_describe = "Read data len from server error";
		return SOAP_OK;
	}
		else if(errorReturn == -4)
	{
		response.result = false;
		response.error_describe = "Read data from server error";
		return SOAP_OK;
	}
	LOG(INFO)<<"Parse data from protobuf protobuf";
	if (message.ParseFromString(connecttoserver.get_recstr()) )
	{	
		if(message.msg_type() == app::dispatch::MSG::Append_Alert_Response)
		{
			if(message.response().result())
			{
				response.result = true;
				response.session_id = int2str(message.session_id());
				response.data.alram_time = message.response().append_alert().alert().alram_time();
				response.data.create_time = message.response().append_alert().alert().create_time();
				response.data.describe = message.response().append_alert().alert().describe();
				response.data.group_id = int2str(message.response().append_alert().alert().group().id());
				response.data.level = (ns__AlertLevel)message.response().append_alert().alert().level();
				response.data.use_cars = message.response().append_alert().alert().use_cars();
				response.data.base.entity_type = (ns__EntityType)message.response().append_alert().alert().base().entity_type();
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
	message.set_session_id(atoi(session_id.c_str()));
	message.set_sequence(get_sequence());
	message.set_msg_type(app::dispatch::MSG::Modify_Alert_Request);
	message.mutable_request()->mutable_modify_alert()->mutable_alert()->mutable_base()->set_name(alert.base.name);
	message.mutable_request()->mutable_modify_alert()->mutable_alert()->set_describe(alert.describe);
	message.mutable_request()->mutable_modify_alert()->mutable_alert()->set_level((::pbmsg::AlertLevel)alert.level);
	message.mutable_request()->mutable_modify_alert()->mutable_alert()->set_alram_time(alert.alram_time);
	message.mutable_request()->mutable_modify_alert()->mutable_alert()->set_use_cars(atoi(alert.use_cars.c_str()));
	message.mutable_request()->mutable_modify_alert()->mutable_alert()->set_create_time(alert.create_time);

	std::string str;
	int errorReturn;
	if(!message.SerializeToString(&str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	ConnectToAppServer connecttoserver;
	errorReturn = connecttoserver.transportData(str,message.ByteSize());
	if(errorReturn == -1)
	{
		response.result = false;
		response.error_describe = "Connection refused";
		return SOAP_OK;
	}
	else if(errorReturn == -2)
	{
		response.result = false;
		response.error_describe = "Write data to server error";
		return SOAP_OK;
	}
		else if(errorReturn == -3)
	{
		response.result = false;
		response.error_describe = "Read data len from server error";
		return SOAP_OK;
	}
		else if(errorReturn == -4)
	{
		response.result = false;
		response.error_describe = "Read data from server error";
		return SOAP_OK;
	}
	LOG(INFO)<<"Parse data from protobuf protobuf";
	if (message.ParseFromString(connecttoserver.get_recstr()) )
	{	
		if(message.msg_type() == app::dispatch::MSG::Modify_Alert_Response)
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
	message.set_session_id(atoi(session_id.c_str()));
	message.set_sequence(get_sequence());
	message.set_msg_type(app::dispatch::MSG::Stop_Alert_Request);
	message.mutable_request()->mutable_stop_alert()->mutable_alert_id()->set_id(atoi(alert_id.c_str()));
	std::string str;
	int errorReturn;
	if(!message.SerializeToString(&str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	ConnectToAppServer connecttoserver;
	errorReturn = connecttoserver.transportData(str,message.ByteSize());
	if(errorReturn == -1)
	{
		response.result = false;
		response.error_describe = "Connection refused";
		return SOAP_OK;
	}
	else if(errorReturn == -2)
	{
		response.result = false;
		response.error_describe = "Write data to server error";
		return SOAP_OK;
	}
		else if(errorReturn == -3)
	{
		response.result = false;
		response.error_describe = "Read data len from server error";
		return SOAP_OK;
	}
		else if(errorReturn == -4)
	{
		response.result = false;
		response.error_describe = "Read data from server error";
		return SOAP_OK;
	}
	LOG(INFO)<<"Parse data from protobuf protobuf";
	if (message.ParseFromString(connecttoserver.get_recstr()) )
	{	
		if(message.msg_type() == app::dispatch::MSG::Stop_Alert_Response)
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
	message.set_session_id(atoi(session_id.c_str()));
	message.set_sequence(get_sequence());
	message.set_msg_type(app::dispatch::MSG::History_Alerts_Request);
	message.mutable_request()->mutable_history_alerts()->set_name(name);
	message.mutable_request()->mutable_history_alerts()->set_create_time_from(create_time_from);
	message.mutable_request()->mutable_history_alerts()->set_create_time_to(create_time_to);
	message.mutable_request()->mutable_history_alerts()->set_alram_time_from(alram_time_from);
	message.mutable_request()->mutable_history_alerts()->set_alram_time_to(alram_time_to);
	message.mutable_request()->mutable_history_alerts()->set_over_time_from(over_time_from);
	message.mutable_request()->mutable_history_alerts()->set_over_time_to(over_time_to);
	std::string str;
	int errorReturn;
	if(!message.SerializeToString(&str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	ConnectToAppServer connecttoserver;
	errorReturn = connecttoserver.transportData(str,message.ByteSize());
	if(errorReturn == -1)
	{
		response.result = false;
		response.error_describe = "Connection refused";
		return SOAP_OK;
	}
	else if(errorReturn == -2)
	{
		response.result = false;
		response.error_describe = "Write data to server error";
		return SOAP_OK;
	}
		else if(errorReturn == -3)
	{
		response.result = false;
		response.error_describe = "Read data len from server error";
		return SOAP_OK;
	}
		else if(errorReturn == -4)
	{
		response.result = false;
		response.error_describe = "Read data from server error";
		return SOAP_OK;
	}
	LOG(INFO)<<"Parse data from protobuf protobuf";
	if (message.ParseFromString(connecttoserver.get_recstr()) )
	{	
		if(message.msg_type() == app::dispatch::MSG::History_Alerts_Response)
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
					historyalert.level = (ns__AlertLevel)message.response().history_alerts().history_alerts(i).level();
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
	message.set_session_id(atoi(session_id.c_str()));
	message.set_sequence(get_sequence());
	message.set_msg_type(app::dispatch::MSG::History_Alert_Request);
	message.mutable_request()->mutable_history_alert()->set_history_alert_id(atoi(alert_id.c_str()));
	std::string str;
	int errorReturn;
	if(!message.SerializeToString(&str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	ConnectToAppServer connecttoserver;
	errorReturn = connecttoserver.transportData(str,message.ByteSize());
	if(errorReturn == -1)
	{
		response.result = false;
		response.error_describe = "Connection refused";
		return SOAP_OK;
	}
	else if(errorReturn == -2)
	{
		response.result = false;
		response.error_describe = "Write data to server error";
		return SOAP_OK;
	}
		else if(errorReturn == -3)
	{
		response.result = false;
		response.error_describe = "Read data len from server error";
		return SOAP_OK;
	}
		else if(errorReturn == -4)
	{
		response.result = false;
		response.error_describe = "Read data from server error";
		return SOAP_OK;
	}
	LOG(INFO)<<"Parse data from protobuf protobuf";
	if (message.ParseFromString(connecttoserver.get_recstr()) )
	{	
		if(message.msg_type() == app::dispatch::MSG::History_Alert_Response)
		{
			if(message.response().result())
			{
				response.result = true;
				response.session_id = int2str(message.session_id());
				response.data.alram_time = message.response().history_alert().history_alert().alram_time();
				response.data.create_time = message.response().history_alert().history_alert().create_time();
				response.data.describe = message.response().history_alert().history_alert().describe();
				response.data.id = int2str(message.response().history_alert().history_alert().id());
				response.data.level = (ns__AlertLevel)message.response().history_alert().history_alert().level();
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
	message.set_session_id(atoi(session_id.c_str()));
	message.set_sequence(get_sequence());
	message.set_msg_type(app::dispatch::MSG::History_Alert_Message_Request);
	message.mutable_request()->mutable_history_alert_message()->set_history_alert_id(atoi(history_alert_id.c_str()));
	message.mutable_request()->mutable_history_alert_message()->set_from_message_id(atoi(from_message_id.c_str()));
	message.mutable_request()->mutable_history_alert_message()->set_from_time(from_time);
	message.mutable_request()->mutable_history_alert_message()->set_max_message_count(atoi(max_message_count.c_str()));
	std::string str;
	int errorReturn;
	if(!message.SerializeToString(&str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	ConnectToAppServer connecttoserver;
	errorReturn = connecttoserver.transportData(str,message.ByteSize());
	if(errorReturn == -1)
	{
		response.result = false;
		response.error_describe = "Connection refused";
		return SOAP_OK;
	}
	else if(errorReturn == -2)
	{
		response.result = false;
		response.error_describe = "Write data to server error";
		return SOAP_OK;
	}
		else if(errorReturn == -3)
	{
		response.result = false;
		response.error_describe = "Read data len from server error";
		return SOAP_OK;
	}
		else if(errorReturn == -4)
	{
		response.result = false;
		response.error_describe = "Read data from server error";
		return SOAP_OK;
	}
	LOG(INFO)<<"Parse data from protobuf protobuf";
	if (message.ParseFromString(connecttoserver.get_recstr()) )
	{	
		if(message.msg_type() == app::dispatch::MSG::History_Alert_Message_Response)
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
	message.set_session_id(atoi(session_id.c_str()));
	message.set_sequence(get_sequence());
	message.set_msg_type(app::dispatch::MSG::Delete_History_Alert_Request);
	message.mutable_request()->mutable_delete_history_alert()->set_history_alert_id(atoi(history_alert_id.c_str()));
	std::string str;
	int errorReturn;
	if(!message.SerializeToString(&str))
	{
		LOG(ERROR)<<message.InitializationErrorString();
		response.result = false;
		response.error_describe = message.InitializationErrorString();
		return SOAP_OK;
	}
	ConnectToAppServer connecttoserver;
	errorReturn = connecttoserver.transportData(str,message.ByteSize());
	if(errorReturn == -1)
	{
		response.result = false;
		response.error_describe = "Connection refused";
		return SOAP_OK;
	}
	else if(errorReturn == -2)
	{
		response.result = false;
		response.error_describe = "Write data to server error";
		return SOAP_OK;
	}
		else if(errorReturn == -3)
	{
		response.result = false;
		response.error_describe = "Read data len from server error";
		return SOAP_OK;
	}
		else if(errorReturn == -4)
	{
		response.result = false;
		response.error_describe = "Read data from server error";
		return SOAP_OK;
	}
	LOG(INFO)<<"Parse data from protobuf protobuf";
	if (message.ParseFromString(connecttoserver.get_recstr()) )
	{	
		if(message.msg_type() == app::dispatch::MSG::Delete_History_Alert_Response)
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
