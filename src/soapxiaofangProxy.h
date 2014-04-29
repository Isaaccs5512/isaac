/* soapxiaofangProxy.h
   Generated by gSOAP 2.8.17r from webservice.h

Copyright(C) 2000-2013, Robert van Engelen, Genivia Inc. All Rights Reserved.
The generated code is released under one of the following licenses:
GPL or Genivia's license for commercial use.
This program is released under the GPL with the additional exemption that
compiling, linking, and/or using OpenSSL is allowed.
*/

#ifndef soapxiaofangProxy_H
#define soapxiaofangProxy_H
#include "soapH.h"

class SOAP_CMAC xiaofangProxy : public soap
{ public:
	/// Endpoint URL of service 'xiaofangProxy' (change as needed)
	const char *soap_endpoint;
	/// Constructor
	xiaofangProxy();
	/// Construct from another engine state
	xiaofangProxy(const struct soap&);
	/// Constructor with endpoint URL
	xiaofangProxy(const char *url);
	/// Constructor with engine input+output mode control
	xiaofangProxy(soap_mode iomode);
	/// Constructor with URL and input+output mode control
	xiaofangProxy(const char *url, soap_mode iomode);
	/// Constructor with engine input and output mode control
	xiaofangProxy(soap_mode imode, soap_mode omode);
	/// Destructor frees deserialized data
	virtual	~xiaofangProxy();
	/// Initializer used by constructors
	virtual	void xiaofangProxy_init(soap_mode imode, soap_mode omode);
	/// Delete all deserialized data (with soap_destroy and soap_end)
	virtual	void destroy();
	/// Delete all deserialized data and reset to default
	virtual	void reset();
	/// Disables and removes SOAP Header from message
	virtual	void soap_noheader();
	/// Get SOAP Header structure (NULL when absent)
	virtual	const SOAP_ENV__Header *soap_header();
	/// Get SOAP Fault structure (NULL when absent)
	virtual	const SOAP_ENV__Fault *soap_fault();
	/// Get SOAP Fault string (NULL when absent)
	virtual	const char *soap_fault_string();
	/// Get SOAP Fault detail as string (NULL when absent)
	virtual	const char *soap_fault_detail();
	/// Close connection (normally automatic, except for send_X ops)
	virtual	int soap_close_socket();
	/// Force close connection (can kill a thread blocked on IO)
	virtual	int soap_force_close_socket();
	/// Print fault
	virtual	void soap_print_fault(FILE*);
#ifndef WITH_LEAN
	/// Print fault to stream
#ifndef WITH_COMPAT
	virtual	void soap_stream_fault(std::ostream&);
#endif

	/// Put fault into buffer
	virtual	char *soap_sprint_fault(char *buf, size_t len);
#endif

	/// Web service operation 'Dispatch-Login' (returns error code or SOAP_OK)
	virtual	int Dispatch_Login(std::string username, std::string password, ns__Dispatch_Login_Response &response) { return this->Dispatch_Login(NULL, NULL, username, password, response); }
	virtual	int Dispatch_Login(const char *endpoint, const char *soap_action, std::string username, std::string password, ns__Dispatch_Login_Response &response);

	/// Web service operation 'Dispatch-Logout' (returns error code or SOAP_OK)
	virtual	int Dispatch_Logout(std::string session_id, std::string name, std::string password, ns__Normal_Response &response) { return this->Dispatch_Logout(NULL, NULL, session_id, name, password, response); }
	virtual	int Dispatch_Logout(const char *endpoint, const char *soap_action, std::string session_id, std::string name, std::string password, ns__Normal_Response &response);

	/// Web service operation 'Dispatch-Entity-Request' (returns error code or SOAP_OK)
	virtual	int Dispatch_Entity_Request(std::string session_id, std::string id, ns__Dispatch_Entity_Request_Response &response) { return this->Dispatch_Entity_Request(NULL, NULL, session_id, id, response); }
	virtual	int Dispatch_Entity_Request(const char *endpoint, const char *soap_action, std::string session_id, std::string id, ns__Dispatch_Entity_Request_Response &response);

	/// Web service operation 'Dispatch-Entity-Nofitication' (returns error code or SOAP_OK)
	virtual	int Dispatch_Entity_Nofitication(std::string session_id, ns__Dispatch_Entity_Nofitication_Response &response) { return this->Dispatch_Entity_Nofitication(NULL, NULL, session_id, response); }
	virtual	int Dispatch_Entity_Nofitication(const char *endpoint, const char *soap_action, std::string session_id, ns__Dispatch_Entity_Nofitication_Response &response);

	/// Web service operation 'Dispatch-Append-Group' (returns error code or SOAP_OK)
	virtual	int Dispatch_Append_Group(std::string session_id, ns__Group group, ns__Dispatch_Append_Group_Response &response) { return this->Dispatch_Append_Group(NULL, NULL, session_id, group, response); }
	virtual	int Dispatch_Append_Group(const char *endpoint, const char *soap_action, std::string session_id, ns__Group group, ns__Dispatch_Append_Group_Response &response);

	/// Web service operation 'Dispatch-Modify-Group' (returns error code or SOAP_OK)
	virtual	int Dispatch_Modify_Group(std::string session_id, ns__Group group, ns__Dispatch_Modify_Group_Response &response) { return this->Dispatch_Modify_Group(NULL, NULL, session_id, group, response); }
	virtual	int Dispatch_Modify_Group(const char *endpoint, const char *soap_action, std::string session_id, ns__Group group, ns__Dispatch_Modify_Group_Response &response);

	/// Web service operation 'Dispatch-Modify-Participants' (returns error code or SOAP_OK)
	virtual	int Dispatch_Modify_Participants(std::string session_id, ns__Modify_Participant request, ns__Dispatch_Modify_Participants_Response &response) { return this->Dispatch_Modify_Participants(NULL, NULL, session_id, request, response); }
	virtual	int Dispatch_Modify_Participants(const char *endpoint, const char *soap_action, std::string session_id, ns__Modify_Participant request, ns__Dispatch_Modify_Participants_Response &response);

	/// Web service operation 'Dispatch-Delete-Group' (returns error code or SOAP_OK)
	virtual	int Dispatch_Delete_Group(std::string session_id, std::string group_id, ns__Normal_Response &response) { return this->Dispatch_Delete_Group(NULL, NULL, session_id, group_id, response); }
	virtual	int Dispatch_Delete_Group(const char *endpoint, const char *soap_action, std::string session_id, std::string group_id, ns__Normal_Response &response);

	/// Web service operation 'Dispatch-Dispatch-Participants-Notification' (returns error code or SOAP_OK)
	virtual	int Dispatch_Dispatch_Participants_Notification(std::string session_id, ns__Dispatch_Dispatch_Participants_Notification_Response &response) { return this->Dispatch_Dispatch_Participants_Notification(NULL, NULL, session_id, response); }
	virtual	int Dispatch_Dispatch_Participants_Notification(const char *endpoint, const char *soap_action, std::string session_id, ns__Dispatch_Dispatch_Participants_Notification_Response &response);

	/// Web service operation 'Dispatch-Join-Group-Request-Nofitication' (returns error code or SOAP_OK)
	virtual	int Dispatch_Join_Group_Request_Nofitication(std::string session_id, ns__Dispatch_Join_Group_Request_Nofitication_Response &response) { return this->Dispatch_Join_Group_Request_Nofitication(NULL, NULL, session_id, response); }
	virtual	int Dispatch_Join_Group_Request_Nofitication(const char *endpoint, const char *soap_action, std::string session_id, ns__Dispatch_Join_Group_Request_Nofitication_Response &response);

	/// Web service operation 'Dispatch-Participant-Status-Notification' (returns error code or SOAP_OK)
	virtual	int Dispatch_Participant_Status_Notification(std::string session_id, ns__Dispatch_Participant_Status_Notification_Response &response) { return this->Dispatch_Participant_Status_Notification(NULL, NULL, session_id, response); }
	virtual	int Dispatch_Participant_Status_Notification(const char *endpoint, const char *soap_action, std::string session_id, ns__Dispatch_Participant_Status_Notification_Response &response);

	/// Web service operation 'Dispatch-Media-Message-Request' (returns error code or SOAP_OK)
	virtual	int Dispatch_Media_Message_Request(std::string session_id, std::string group_id, std::string from_message_id, std::string from_time, std::string max_message_count, ns__Dispatch_Media_Message_Request_Response &response) { return this->Dispatch_Media_Message_Request(NULL, NULL, session_id, group_id, from_message_id, from_time, max_message_count, response); }
	virtual	int Dispatch_Media_Message_Request(const char *endpoint, const char *soap_action, std::string session_id, std::string group_id, std::string from_message_id, std::string from_time, std::string max_message_count, ns__Dispatch_Media_Message_Request_Response &response);

	/// Web service operation 'Dispatch-Media-Message-Notification' (returns error code or SOAP_OK)
	virtual	int Dispatch_Media_Message_Notification(std::string session_id, ns__Dispatch_Media_Message_Notification_Response &response) { return this->Dispatch_Media_Message_Notification(NULL, NULL, session_id, response); }
	virtual	int Dispatch_Media_Message_Notification(const char *endpoint, const char *soap_action, std::string session_id, ns__Dispatch_Media_Message_Notification_Response &response);

	/// Web service operation 'Dispatch-Participant-Connect-Request-Notification' (returns error code or SOAP_OK)
	virtual	int Dispatch_Participant_Connect_Request_Notification(std::string session_id, ns__Dispatch_Participant_Connect_Request_Notification_Response &response) { return this->Dispatch_Participant_Connect_Request_Notification(NULL, NULL, session_id, response); }
	virtual	int Dispatch_Participant_Connect_Request_Notification(const char *endpoint, const char *soap_action, std::string session_id, ns__Dispatch_Participant_Connect_Request_Notification_Response &response);

	/// Web service operation 'Dispatch-Participant-Speak-Request-Notification' (returns error code or SOAP_OK)
	virtual	int Dispatch_Participant_Speak_Request_Notification(std::string session_id, ns__Dispatch_Participant_Speak_Request_Notification_Response &response) { return this->Dispatch_Participant_Speak_Request_Notification(NULL, NULL, session_id, response); }
	virtual	int Dispatch_Participant_Speak_Request_Notification(const char *endpoint, const char *soap_action, std::string session_id, ns__Dispatch_Participant_Speak_Request_Notification_Response &response);

	/// Web service operation 'Dispatch-Invite-Participant-Request' (returns error code or SOAP_OK)
	virtual	int Dispatch_Invite_Participant_Request(std::string session_id, std::string group_id, std::string account_id, ns__Normal_Response &response) { return this->Dispatch_Invite_Participant_Request(NULL, NULL, session_id, group_id, account_id, response); }
	virtual	int Dispatch_Invite_Participant_Request(const char *endpoint, const char *soap_action, std::string session_id, std::string group_id, std::string account_id, ns__Normal_Response &response);

	/// Web service operation 'Dispatch-Drop-Participant-Request' (returns error code or SOAP_OK)
	virtual	int Dispatch_Drop_Participant_Request(std::string session_id, std::string group_id, std::string account_id, ns__Normal_Response &response) { return this->Dispatch_Drop_Participant_Request(NULL, NULL, session_id, group_id, account_id, response); }
	virtual	int Dispatch_Drop_Participant_Request(const char *endpoint, const char *soap_action, std::string session_id, std::string group_id, std::string account_id, ns__Normal_Response &response);

	/// Web service operation 'Dispatch-Release-Participant-Token-Request' (returns error code or SOAP_OK)
	virtual	int Dispatch_Release_Participant_Token_Request(std::string session_id, std::string group_id, std::string account_id, ns__Normal_Response &response) { return this->Dispatch_Release_Participant_Token_Request(NULL, NULL, session_id, group_id, account_id, response); }
	virtual	int Dispatch_Release_Participant_Token_Request(const char *endpoint, const char *soap_action, std::string session_id, std::string group_id, std::string account_id, ns__Normal_Response &response);

	/// Web service operation 'Dispatch-Appoint-Participant-Speak-Request' (returns error code or SOAP_OK)
	virtual	int Dispatch_Appoint_Participant_Speak_Request(std::string session_id, std::string group_id, std::string account_id, ns__Normal_Response &response) { return this->Dispatch_Appoint_Participant_Speak_Request(NULL, NULL, session_id, group_id, account_id, response); }
	virtual	int Dispatch_Appoint_Participant_Speak_Request(const char *endpoint, const char *soap_action, std::string session_id, std::string group_id, std::string account_id, ns__Normal_Response &response);

	/// Web service operation 'Dispatch-Jion-Group-Request' (returns error code or SOAP_OK)
	virtual	int Dispatch_Jion_Group_Request(std::string session_id, std::string group_id, ns__Normal_Response &response) { return this->Dispatch_Jion_Group_Request(NULL, NULL, session_id, group_id, response); }
	virtual	int Dispatch_Jion_Group_Request(const char *endpoint, const char *soap_action, std::string session_id, std::string group_id, ns__Normal_Response &response);

	/// Web service operation 'Dispatch-Leave-Group-Request' (returns error code or SOAP_OK)
	virtual	int Dispatch_Leave_Group_Request(std::string session_id, std::string group_id, ns__Normal_Response &response) { return this->Dispatch_Leave_Group_Request(NULL, NULL, session_id, group_id, response); }
	virtual	int Dispatch_Leave_Group_Request(const char *endpoint, const char *soap_action, std::string session_id, std::string group_id, ns__Normal_Response &response);

	/// Web service operation 'Dispatch-Session-Status-Notification' (returns error code or SOAP_OK)
	virtual	int Dispatch_Session_Status_Notification(std::string session_id, ns__Dispatch_Session_Status_Notification_Response &response) { return this->Dispatch_Session_Status_Notification(NULL, NULL, session_id, response); }
	virtual	int Dispatch_Session_Status_Notification(const char *endpoint, const char *soap_action, std::string session_id, ns__Dispatch_Session_Status_Notification_Response &response);

	/// Web service operation 'Dispatch-Send-Message-Request' (returns error code or SOAP_OK)
	virtual	int Dispatch_Send_Message_Request(std::string session_id, std::string group_id, ns__MediaMessage mediamessage, ns__Normal_Response &response) { return this->Dispatch_Send_Message_Request(NULL, NULL, session_id, group_id, mediamessage, response); }
	virtual	int Dispatch_Send_Message_Request(const char *endpoint, const char *soap_action, std::string session_id, std::string group_id, ns__MediaMessage mediamessage, ns__Normal_Response &response);

	/// Web service operation 'Dispatch-Start-Record-Request' (returns error code or SOAP_OK)
	virtual	int Dispatch_Start_Record_Request(std::string session_id, std::string group_id, ns__Normal_Response &response) { return this->Dispatch_Start_Record_Request(NULL, NULL, session_id, group_id, response); }
	virtual	int Dispatch_Start_Record_Request(const char *endpoint, const char *soap_action, std::string session_id, std::string group_id, ns__Normal_Response &response);

	/// Web service operation 'Dispatch-Stop-Record-Request' (returns error code or SOAP_OK)
	virtual	int Dispatch_Stop_Record_Request(std::string session_id, std::string group_id, ns__Normal_Response &response) { return this->Dispatch_Stop_Record_Request(NULL, NULL, session_id, group_id, response); }
	virtual	int Dispatch_Stop_Record_Request(const char *endpoint, const char *soap_action, std::string session_id, std::string group_id, ns__Normal_Response &response);

	/// Web service operation 'Dispatch-Record-Status-Notification' (returns error code or SOAP_OK)
	virtual	int Dispatch_Record_Status_Notification(std::string session_id, ns__Dispatch_Record_Status_Notification_Response &response) { return this->Dispatch_Record_Status_Notification(NULL, NULL, session_id, response); }
	virtual	int Dispatch_Record_Status_Notification(const char *endpoint, const char *soap_action, std::string session_id, ns__Dispatch_Record_Status_Notification_Response &response);

	/// Web service operation 'Dispatch-Subscribe-Account-Info-Request' (returns error code or SOAP_OK)
	virtual	int Dispatch_Subscribe_Account_Info_Request(std::string session_id, bool subscribing, std::list<std::string >account_id, enum ns__SubscribeType type, std::string ttl, ns__Normal_Response &response) { return this->Dispatch_Subscribe_Account_Info_Request(NULL, NULL, session_id, subscribing, account_id, type, ttl, response); }
	virtual	int Dispatch_Subscribe_Account_Info_Request(const char *endpoint, const char *soap_action, std::string session_id, bool subscribing, std::list<std::string >account_id, enum ns__SubscribeType type, std::string ttl, ns__Normal_Response &response);

	/// Web service operation 'Dispatch-Account-Info-Notification' (returns error code or SOAP_OK)
	virtual	int Dispatch_Account_Info_Notification(std::string session_id, ns__Dispatch_Account_Info_Notification_Response &response) { return this->Dispatch_Account_Info_Notification(NULL, NULL, session_id, response); }
	virtual	int Dispatch_Account_Info_Notification(const char *endpoint, const char *soap_action, std::string session_id, ns__Dispatch_Account_Info_Notification_Response &response);

	/// Web service operation 'Dispatch-Append-Alert-Request' (returns error code or SOAP_OK)
	virtual	int Dispatch_Append_Alert_Request(std::string session_id, ns__Alert alert, std::list<ns__Account >members, std::string size, ns__Dispatch_Append_Alert_Request_Response &response) { return this->Dispatch_Append_Alert_Request(NULL, NULL, session_id, alert, members, size, response); }
	virtual	int Dispatch_Append_Alert_Request(const char *endpoint, const char *soap_action, std::string session_id, ns__Alert alert, std::list<ns__Account >members, std::string size, ns__Dispatch_Append_Alert_Request_Response &response);

	/// Web service operation 'Dispatch-Modify-Alert-Request' (returns error code or SOAP_OK)
	virtual	int Dispatch_Modify_Alert_Request(std::string session_id, ns__Alert alert, ns__Normal_Response &response) { return this->Dispatch_Modify_Alert_Request(NULL, NULL, session_id, alert, response); }
	virtual	int Dispatch_Modify_Alert_Request(const char *endpoint, const char *soap_action, std::string session_id, ns__Alert alert, ns__Normal_Response &response);

	/// Web service operation 'Dispatch-Stop-Alert-Request' (returns error code or SOAP_OK)
	virtual	int Dispatch_Stop_Alert_Request(std::string session_id, std::string alert_id, ns__Normal_Response &response) { return this->Dispatch_Stop_Alert_Request(NULL, NULL, session_id, alert_id, response); }
	virtual	int Dispatch_Stop_Alert_Request(const char *endpoint, const char *soap_action, std::string session_id, std::string alert_id, ns__Normal_Response &response);

	/// Web service operation 'Dispatch-Alert-Overed-Notification' (returns error code or SOAP_OK)
	virtual	int Dispatch_Alert_Overed_Notification(std::string session_id, ns__Dispatch_Alert_Overed_Notification_Response &response) { return this->Dispatch_Alert_Overed_Notification(NULL, NULL, session_id, response); }
	virtual	int Dispatch_Alert_Overed_Notification(const char *endpoint, const char *soap_action, std::string session_id, ns__Dispatch_Alert_Overed_Notification_Response &response);

	/// Web service operation 'Dispatch-History-Alert-Request' (returns error code or SOAP_OK)
	virtual	int Dispatch_History_Alert_Request(std::string session_id, std::string name, std::string create_time_from, std::string create_time_to, std::string alram_time_from, std::string alram_time_to, std::string over_time_from, std::string over_time_to, ns__Dispatch_History_Alert_Request_Reponse &response) { return this->Dispatch_History_Alert_Request(NULL, NULL, session_id, name, create_time_from, create_time_to, alram_time_from, alram_time_to, over_time_from, over_time_to, response); }
	virtual	int Dispatch_History_Alert_Request(const char *endpoint, const char *soap_action, std::string session_id, std::string name, std::string create_time_from, std::string create_time_to, std::string alram_time_from, std::string alram_time_to, std::string over_time_from, std::string over_time_to, ns__Dispatch_History_Alert_Request_Reponse &response);

	/// Web service operation 'Dispatch-Alert-Request' (returns error code or SOAP_OK)
	virtual	int Dispatch_Alert_Request(std::string session_id, std::string alert_id, ns__Dispatch_Alert_Request_Response &response) { return this->Dispatch_Alert_Request(NULL, NULL, session_id, alert_id, response); }
	virtual	int Dispatch_Alert_Request(const char *endpoint, const char *soap_action, std::string session_id, std::string alert_id, ns__Dispatch_Alert_Request_Response &response);

	/// Web service operation 'Dispatch-History-Alert-Message-Request' (returns error code or SOAP_OK)
	virtual	int Dispatch_History_Alert_Message_Request(std::string session_id, std::string history_alert_id, std::string from_message_id, std::string from_time, std::string max_message_count, ns__Dispatch_History_Alert_Message_Request_Response &response) { return this->Dispatch_History_Alert_Message_Request(NULL, NULL, session_id, history_alert_id, from_message_id, from_time, max_message_count, response); }
	virtual	int Dispatch_History_Alert_Message_Request(const char *endpoint, const char *soap_action, std::string session_id, std::string history_alert_id, std::string from_message_id, std::string from_time, std::string max_message_count, ns__Dispatch_History_Alert_Message_Request_Response &response);

	/// Web service operation 'Dispatch-Delete-History-Alert-Request' (returns error code or SOAP_OK)
	virtual	int Dispatch_Delete_History_Alert_Request(std::string session_id, std::string history_alert_id, ns__Normal_Response &response) { return this->Dispatch_Delete_History_Alert_Request(NULL, NULL, session_id, history_alert_id, response); }
	virtual	int Dispatch_Delete_History_Alert_Request(const char *endpoint, const char *soap_action, std::string session_id, std::string history_alert_id, ns__Normal_Response &response);
};
#endif
