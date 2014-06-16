//gsoap ns service name: xiaofang 
//gsoap ns service protocol: SOAP 
//gsoap ns service style: rpc 
//gsoap ns service encoding: encoded 
//gsoap ns schema namespace: urn:xiaofang
//gsoap ns service location: http://192.168.146.128:18881 
#pragma once
#import "import/stllist.h"
#ifndef WITH_OPENSSL
#define WITH_OPENSSL
#endif

enum ns__RegisterStatus
{
	OffLine,
	OnLine,
};

enum ns__ListModifyType
{
	Increased,//������members�����ݺͷ��������еĳ�Ա���кϲ�
	Decrease,//����ָ����members��Ա
	Replace,//�滻
};

enum ns__AccountType
{
	Dispatcher,//ר�ŷ��������Ա���˺ţ�������Operator���͵�User���а󶨵��˺�
	Terminal,//��ͨ�ն��˺�
	Record,// ΪȺ��¼����Ԥ�����˺�
	MediaGateway,//Ϊý�����ض�Ԥ�����˺�
};
//ÿ��PoC�˺ţ������д�������Ϊȫ�����ã����˺ż����ض���Ⱥ��ʱ��Ҳ������Բ�ͬ��Ⱥ��������
enum ns__CallPrivilege
{
	CallForbid,//��ֹ��PoC�˺Ž�ֹ�����κ�Ⱥ�飬��ʹ��Ⱥ��ĳ�ԱҲ����ֹ
	CallFreedom,//����ģʽ��PoC�˺ſ��Ժ���Ⱥ�飬�������Լ�ΪȺ��ĳ�Ա
	CallPolicy,// ���������ߣ����������к󣬿��Ժ���Ⱥ��
};

enum ns__TokenPrivilege
{
	TokenForbid,//��ֹ����
	TokenFreedom,//�������룬����󱨸�״̬����
	TokenPolicy,//ͨ�����������к�ſ������룬�������Ҫ����״̬
};

//Ҳ���ǲ����ߵ�״̬
enum ns__SessionStatus
{
	IDLE,
	InGroup,
	Talking,
};

enum ns__RecordType
{
	DontRecord,
	AutoRecord,
	ManualRecord,
};

enum ns__RecordStatus
{
	RecordStop,
	RecordStart,
};

enum ns__AlertStatus
{
	AlertActive,
	AlertOver,
};

enum ns__AlertLevel
{
	Exigency,
	Importance,
	Subordination,
	Ordinary,
};

enum ns__EntityNotifyType
{
	Created,
	Modify,//  �����޸�
	Update,// ״̬�ı�
	Deleted
};

enum ns__ProfessionStatus
{
	Standby,
	Duty,
	Turnout,
	Training,
};

enum ns__SubscribeType
{
	Gis,//����λ��
	Status, //���ߣ�����,����Ϊenum ns__RegisterStatus
	Profession,// ������ִ�ڡ�������ѵ��
	Gis_status,
	Gis_profession,
	Status_profession,
	Gis_status_profession,
};

enum ns__EntityType
{
	UNIT = 0,//    ��Ԫ
	ACCOUNT = 1,//  PoC�˺�
	USER = 2,//     �û�
	GROUPW = 3,//  Ⱥ��
//	GATEWAY = 4,//  ý�����أ�¼���������ϲ�һ��
	ORGANIZATION = 5,// ��֯��PoC�˺ŵļ��ϣ�
	ALERT = 6,
};


class ns__Entity
{
public:
	std::string id;
	std::string name;
	std::string parentid;
	std::string entity_type;
};

class ns__Address
{
public:
	std::string ip;
	std::string port;
};


class ns__User
{
public:
	ns__Entity base;//��poc�˻���Ϣ
	ns__Entity account;//poc�˻���Ϣ
	std::string register_tatus;
};

class ns__Account
{
public:
	ns__Entity base;
	//enum ns__AccountType account_type;//PoC�˺�����
	std::string number; //���룬SIPע���˺�
	std::string  short_number; //�̺ţ�Ӧ�÷������ڲ�Ψһ���룬�ڲ�����
	std::string priority;//���ȼ�����Ȩ�����ͻ���ߵ����ݡ�ֵԽ�����ȼ�Խ�ߡ�
	std::string call_privilege;//����Ĭ��Ȩ��,����Ⱥ��ʱ��Ĭ�Ϻ���Ȩ��

	std::string token_privilege;//��ȨĬ��Ȩ��,����Ⱥ���ͨ����Ļ�Ȩ����Ĭ��Ȩ��

	std::string register_tatus;
	std::string sip_status;
};


class ns__Dispatch_Login_Response
{
public:
	std::string session_id;	//�ỰID��������Ϣ������Я����
	std::string id;
	std::string parentid;
	std::string ttl;
	std::string error_describe;
	bool result;
};

class ns__Unit
{
public:
	ns__Entity base; 	//������������Ϣ
	std::list<ns__Entity> members;//   ��Ԫ�ĳ�Ա�б������Ա��������Entity. entity_typeָ��
	std::string size;
};

class ns__Participant
{
public:
	std::string id;
	std::string name;
	std::string parentid;
	std::string priority;
	std::string call_privilege;
	std::string token_privilege;
	std::string session_status;
};


class ns__Group
{
public:
	std::string id;
	std::string name;
	std::string owner_id;//������
	std::string parent_id;//group������Ⱥ��
	std::string number;
	std::string short_number;
	std::list<ns__Participant> participants;
	std::string size;
	bool sealed;//�Ƿ�Ϊ����ʽȺ��        
	std::string record_type;
	std::string record_status;
};

class ns__Dispatch_Append_Group_Response
{
	ns__Group data;
	std::string session_id;
	std::string error_describe;
	bool result;
};

class ns__Dispatch_Modify_Group_Response
{
	ns__Group data;
	std::string session_id;
	std::string error_describe;
	bool result;
};

class ns__Alert
{
public:
	ns__Entity base;
	std::string group_id;//Ŀǰֻ����һ������Ⱥ���ID
	std::string describe;// ��������
	std::string alert_level;//���鼶��
	std::string alram_time;//   ����ʱ��
	std::string use_cars;// ����������
	std::string create_time;//����Ĵ���ʱ��
	//std::string alert_status ;//����״̬
};


class ns__Dispatch_Append_Alert_Request_Response
{
	ns__Alert data;
	std::string session_id;
	std::string error_describe;
	bool result;
};

class ns__Organization
{
public:
	ns__Entity base;
	std::list<ns__Entity> members;
};


class ns__EntityData
{
public:
	ns__Entity  id;// ���� id.entity_type��ֵ�����������ض����ֶΡ�
	ns__Unit    unit;
	ns__Account account;
	ns__User   user;
	ns__Group group;
	ns__Alert  alert;
	ns__Organization  organization;
};

class ns__Dispatch_Entity_Request_Response
{
	ns__EntityData data;
	std::string session_id;
	std::string error_describe;
	bool result;
};

class ns__Dispatch_Entity_Nofitication_Response
{
public:
	std::string notify_type;//
	ns__EntityData data;//ʵ��ļ�����Ϣ���ΪDeletedʱ��dataֻ��Ҫ����id�ֶμ���
	std::string session_id;
	std::string error_describe;
	bool result;
};

class ns__Entity_Status_Notification
{
public:
	ns__Entity id;//״̬�ı��ʵ��,����Ϊ Account,User,Gateway����Ӧ�÷�������ע���ʵ��
	std::string register_status;//��Ӧ�÷�����֮���ע��״̬
	std::string sip_status;//Accountר���ֶΣ���ʾAccount��PoC������֮ǰ��ע��״̬
};

class ns__Modify_Participant
{
public:
	std::string group_id;
	std::string modify_type;
	std::list<ns__Participant> participants;
	std::string size;
};

class ns__Dispatch_Modify_Participants_Response
{
	ns__Modify_Participant data;
	std::string session_id;
	std::string error_describe;
	bool result;
};

class ns__Dispatch_Dispatch_Participants_Notification_Response
{
	ns__Modify_Participant data;
	std::string session_id;
	std::string error_describe;
	bool result;
};

class ns__Dispatch_Join_Group_Request_Nofitication_Response
{
public:
	ns__Entity group_id;//Ҫ�����Ⱥ��
	ns__Entity account_id;//�������Ⱥ����˺�
	std::string session_id;
	std::string error_describe;
	bool result;
};

class ns__Dispatch_Participant_Status_Notification_Response
{
public:
	ns__Entity group_id;//Ⱥ��id
	ns__Entity account_id;//PoC�˺�ID
	std::string session_status;//�Ự״̬
	std::string session_id;
	std::string error_describe;
	bool result;
};

class ns__MediaMessage
{
public:
	std::string id;//��Ϣ��ţ���ϢԽ�緢�ͣ���ֵԽС,�������Զ����ɣ����ͷ���������.
	std::string sender;//��Ϣ�����ߣ�һ��ΪAccout.Entity.name�˺�
	std::string text;//���Ϊ�ı���Ϣʱ����������ֶ�
	std::string picture_uri;//���ΪͼƬ��Ϣʱ�ڣ���������ֶΣ�ͼƬ��uri��ַ
	std::string audio_uri;
	std::string audio_length;
	std::string video_uri;
	std::string video_length;
	std::string timestamp;//���ͷ����������ã��������Զ����÷��������յ���Ϣ��ʱ��yyyy-mm-dd HH::MM::SS��ʽ
};

class ns__Dispatch_Media_Message_Request_Response
{
	std::list<ns__MediaMessage> data;
	std::string size;
	std::string leave_message_count;
	std::string session_id;
	std::string error_describe;
	bool result;
};

class ns__Media_Message
{
public:
	ns__Entity id;//Ⱥ��ID\����id
	ns__MediaMessage messages;//��Ϣ�б����û�а����κ���Ϣʱ��˵��Ⱥ����Ϣ�Ѿ���ˢ�����,�ɻ�ȡ��messages �б��е����һ����Ϣ��ʱ������ٴη���ˢ��
	std::string leave_message_count;//
};

class ns__Dispatch_Media_Message_Notification_Response
{
public:
	std::string id;//���մ���Ϣ��Ⱥ��/���߾���
	ns__MediaMessage messages;
	std::string session_id;
	std::string error_describe;
	bool result;
};

class ns__Dispatch_Participant_Connect_Request_Notification_Response
{
public:
	ns__Entity group_id;
	ns__Entity account_id;
	std::string session_id;
	std::string error_describe;
	bool result;
};

class ns__Dispatch_Participant_Speak_Request_Notification_Response
{
public:
	ns__Entity group_id;
	ns__Entity account_id;
	std::string session_id;
	std::string error_describe;
	bool result;
};


class ns__Dispatch_Session_Status_Notification_Response
{
public:
	ns__Entity group_id;
	std::string session_status;
	std::string session_id;
	std::string error_describe;
	bool result;
};

class ns__Dispatch_Account_Info_Notification_Response
{
public:
	ns__Entity account_id;//Terminal�˺�ID
	double longitude;//����
	double latitude;//ά��
	std::string timestamp;//��λ����Ϣ��ȡ��ʱ��
	std::string profession_status;
	std::string register_status;
	std::string session_id;
	std::string error_describe;
	bool result;
};

class ns__HistoryAlert
{
public:
	std::string id;
	std::string name;
	std::string describe;
	std::string alert_level;
	std::string alram_time;
	std::string use_cars;
	std::string create_time;
	std::string over_time;
};

class ns__Dispatch_Alert_Request_Response
{
	ns__HistoryAlert data;
	std::string session_id;
	std::string error_describe;
	bool result;
};


class ns__Dispatch_History_Alert_Request_Reponse
{
	std::list<ns__HistoryAlert> data;
	std::string size;
	std::string session_id;
	std::string error_describe;
	bool result;
};

class ns__Dispatch_History_Alert_Message_Request_Response
{
public:
	std::string history_alert_id;//��ʷ����id
	std::list<ns__MediaMessage> messages;//��Ϣ�б�
	std::string size;
	std::string leave_message_count;//ʣ����Ϣ����
	std::string error_describe;
	std::string session_id;
	bool result;
};

class ns__Dispatch_Record_Status_Notification_Response
{
public:
	ns__Entity group_id;
	bool recording;//true-����¼�� false-¼����ֹͣ
	std::string error_describe;
	std::string session_id;
	bool result;
};

class ns__Dispatch_Alert_Overed_Notification_Response
{
	std::string alert_id;
	std::string error_describe;
	std::string session_id;
	bool result;
};

class ns__Normal_Response
{
	std::string session_id;
	std::string error_describe;
	bool result;
};

class ns__Dispatch_Move_Unit_Notification_Response
{
	ns__Entity entity;
};