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
typedef unsigned long uint32;

enum ns__RegisterStatus
{
	OffLine,
	OnLine,
};


enum ns__UserType
{
	Admin,
	Operator,
};

enum ns__EntityType
{
	UNIT,
	ACCOUNT,
	USER,
	GROUP,
	GATEWAY,
	ORGANIZATION,
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
//AccountStop ״̬ʱ��ϵͳ���ṩ���˺ŵ��κη��񣬰���ע�ᡣ
enum ns__AccountServiceStatus
{
	AccountStop,
	AccountActive,
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

enum ns__GpsReportMode
{
	GpsManualPull,// ��ģʽ������������һ�Σ��ն�Ӧ��һ��
	GpsManualPush ,// �ֶ���ģʽ���ڷ���������ʼ�ͽ���֮�䣬����ķ���λ����Ϣ
	GpsAutoPush ,// �Զ�����ģʽ���ն�ע����Ӧ�÷������Ժ󣬼������λ����Ϣ
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

class ns__Entity
{
public:
	uint32 id;
	std::string name;
	enum ns__EntityType entity_type;
	uint32 parentid;
};

class ns__Address
{
public:
	std::string ip;
	uint32 port;
};


class ns__User
{
public:
	ns__Entity base;
	std::string password;
	enum ns__UserType user_type;
	ns__Entity account;
	enum ns__RegisterStatus status;
	ns__Address address;
};

class ns__Account
{
public:
	ns__Entity base;
	enum ns__AccountType account_type;//PoC�˺�����
	std::string number; //���룬SIPע���˺�
	std::string  short_number; //�̺ţ�Ӧ�÷������ڲ�Ψһ���룬�ڲ�����
	std::string  password; 
	enum ns__AccountServiceStatus service_status;
	uint32 priority;//���ȼ�����Ȩ�����ͻ���ߵ����ݡ�ֵԽ�����ȼ�Խ�ߡ�
	enum ns__CallPrivilege call_privilege;//����Ĭ��Ȩ��,����Ⱥ��ʱ��Ĭ�Ϻ���Ȩ��

	enum ns__TokenPrivilege token_privilege;//��ȨĬ��Ȩ��,����Ⱥ���ͨ����Ļ�Ȩ����Ĭ��Ȩ��

	enum ns__GpsReportMode gps_report_mode;//GPSλ����Ϣ�ϱ�ģʽ
	uint32 ttl;
	enum ns__RegisterStatus status;
	enum ns__RegisterStatus sip_status;
	ns__Address address; 
};


class ns__Login_Response
{
public:
	uint32 session_id;	//�ỰID��������Ϣ������Я����
	ns__User user;		//�û���ϵͳ�е���Ϣ
	uint32 ttl;			//������,��λs
};

class ns__Unit
{
public:
	ns__Entity base; 	//������������Ϣ
	bool include_members;//�Ƿ����members�ֶΣ������ֵΪfalseʱ�������members�ֶ�
	ns__Entity members;//   ��Ԫ�ĳ�Ա�б������Ա��������Entity. entity_typeָ��
};

class ns__Participant
{
public:
	uint32 id ;
	ns__Entity group;
	ns__Entity account;
	uint32 priority;
	enum ns__CallPrivilege call_privilege;
	enum ns__TokenPrivilege toke_privilege;
	enum ns__SessionStatus status;
	bool sync_added;
};


class ns__Group
{
public:
	ns__Entity base;
	ns__Entity owner;
	std::string number;
	std::string short_number;
	bool sealed;

	bool include_participants;
	ns__Participant participants;
	        
	enum ns__RecordType record_type;
	ns__Participant recorder;
	enum ns__RecordStatus record_status;
};

class ns__Gateway
{
public:
	ns__Entity base;
	ns__Address address;// ��ַ

	bool include_allocate_counts;
	ns__Entity allocate_counts;//     �������ý�����ص��˺�
	bool include_active_counts;
	ns__Entity active_counts;//  ���ڻ���˺�	        
	enum ns__RegisterStatus status;//  ע��״̬
};


class ns__Alert
{
public:
	ns__Entity base;
	ns__Entity group;//����Ⱥ���ID
	std::string describe;// ��������
	enum ns__AlertLevel level;//���鼶��
	std::string alram_time;//   ����ʱ��
	uint32 use_cars;// ����������
	std::string create_time;//����Ĵ���ʱ��
	enum ns__AlertStatus status ;//����״̬
};

class ns__Organization
{
public:
	ns__Entity base;

	bool include_members;
	ns__Entity members;
};


class ns__EntityData
{
public:
	ns__Entity  id;// ���� id.entity_type��ֵ�����������ض����ֶΡ�
	ns__Unit    unit;
	ns__Account accout;
	ns__User   user;
	ns__Group group ;
	ns__Alert  alert ;
	ns__Gateway gateway ;
	ns__Organization  organization ;
};

class ns__Entity_Nofitication_Response
{
public:
	enum ns__EntityNotifyType notify_type;//
	ns__EntityData data;//ʵ��ļ�����Ϣ���ΪDeletedʱ��dataֻ��Ҫ����id�ֶμ���
};

class ns__Entity_Status_Notification
{
public:
	ns__Entity id;//״̬�ı��ʵ��,����Ϊ Account,User,Gateway����Ӧ�÷�������ע���ʵ��
	enum ns__RegisterStatus status;//��Ӧ�÷�����֮���ע��״̬
	enum ns__RegisterStatus sip_status;//Accountר���ֶΣ���ʾAccount��PoC������֮ǰ��ע��״̬
};

class ns__Modify_Participant
{
public:
	ns__Entity group_id;
	enum ns__ListModifyType modify_type;
	std::list<ns__Participant> participants;
};

class ns__Join_Group_Request_Nofitication
{
public:
	ns__Entity group_id;//Ҫ�����Ⱥ��
	ns__Entity account_id;//�������Ⱥ����˺�
};

class ns__Participant_Status_Notification
{
public:
	ns__Entity group_id;//Ⱥ��id
	ns__Entity account_id;//PoC�˺�ID
	enum ns__SessionStatus status;//�Ự״̬
};

class ns__MediaMessage
{
public:
	uint32 id;//��Ϣ��ţ���ϢԽ�緢�ͣ���ֵԽС,�������Զ����ɣ����ͷ���������.
	std::string sender;//��Ϣ�����ߣ�һ��ΪAccout.Entity.name�˺�
	std::string text;//���Ϊ�ı���Ϣʱ����������ֶ�
	std::string picture_uri;//���ΪͼƬ��Ϣʱ�ڣ���������ֶΣ�ͼƬ��uri��ַ
	std::string audio_uri;
	uint32 audio_length;
	std::string video_uri;
	uint32 video_length;
	std::string timestamp;//���ͷ����������ã��������Զ����÷��������յ���Ϣ��ʱ��yyyy-mm-dd HH::MM::SS��ʽ
};

class ns__Media_Message
{
public:
	ns__Entity id;//Ⱥ��ID\����id
	ns__MediaMessage messages;//��Ϣ�б����û�а����κ���Ϣʱ��˵��Ⱥ����Ϣ�Ѿ���ˢ����ɡ�
								//�ɻ�ȡ��messages �б��е����һ����Ϣ��ʱ������ٴη���ˢ��
	uint32 leave_message_count;//
};

class ns__Media_Message_Notification
{
public:
	uint32 id;//���մ���Ϣ��Ⱥ��/���߾���
	ns__MediaMessage messages;
};

class ns__Participant_Connect_Request_Notification
{
public:
	ns__Entity group_id;
	ns__Entity account_id;
};

class ns__Session_Status_Notification
{
public:
	ns__Entity group_id;
	enum ns__SessionStatus status;
};

class ns__Dispatch_Account_Location_Notification_Response
{
public:
	ns__Entity account_id;//Terminal�˺�ID
	double longitude;//����
	double latitude;//ά��
	std::string timestamp;//��λ����Ϣ��ȡ��ʱ��
};

class ns__HistoryAlert
{
public:
	uint32 id;
	std::string name;
	std::string describe;
	enum ns__AlertLevel level;
	std::string alram_time;
	uint32 use_cars;
	std::string create_time;
	std::string over_time;
};

class ns__Dispatch_History_Alert_Message
{
public:
	uint32 history_alert_id;//��ʷ����id
	std::list<ns__MediaMessage> messages;//��Ϣ�б�
	uint32 leave_message_count;//ʣ����Ϣ����
};

class ns__Dispatch_Record_Status
{
public:
	ns__Entity group_id;
	bool recording;//true-����¼�� false-¼����ֹͣ
};
