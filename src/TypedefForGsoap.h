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
	Increased,//新增，members的内容和服务器现有的成员进行合并
	Decrease,//减少指定的members成员
	Replace,//替换
};

enum ns__AccountType
{
	Dispatcher,//专门分配给调度员的账号，用于与Operator类型的User进行绑定的账号
	Terminal,//普通终端账号
	Record,// 为群组录音而预留的账号
	MediaGateway,//为媒体网关而预留的账号
};
//每个PoC账号，可以有此配置作为全局配置，在账号加入特定的群组时，也可以针对不同的群组做设置
enum ns__CallPrivilege
{
	CallForbid,//禁止，PoC账号禁止呼叫任何群组，即使是群组的成员也被禁止
	CallFreedom,//自由模式，PoC账号可以呼叫群组，条件是自己为群组的成员
	CallPolicy,// 服务器决策，服务器运行后，可以呼叫群组
};

enum ns__TokenPrivilege
{
	TokenForbid,//禁止申请
	TokenFreedom,//自由申请，申请后报告状态即可
	TokenPolicy,//通过服务器运行后才可以申请，申请后需要报告状态
};

//也就是参与者的状态
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
	Modify,//  配置修改
	Update,// 状态改变
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
	Gis,//地理位置
	Status, //在线，离线,类行为enum ns__RegisterStatus
	Profession,// 待命、执勤、出车、训练
	Gis_status,
	Gis_profession,
	Status_profession,
	Gis_status_profession,
};

enum ns__EntityType
{
	UNIT = 0,//    单元
	ACCOUNT = 1,//  PoC账号
	USER = 2,//     用户
	GROUPW = 3,//  群组
//	GATEWAY = 4,//  媒体网关（录音服务器合并一起）
	ORGANIZATION = 5,// 组织（PoC账号的集合）
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
	ns__Entity base;//非poc账户信息
	ns__Entity account;//poc账户信息
	std::string register_tatus;
};

class ns__Account
{
public:
	ns__Entity base;
	//enum ns__AccountType account_type;//PoC账号类型
	std::string number; //号码，SIP注册账号
	std::string  short_number; //短号，应用服务器内部唯一号码，内部号码
	std::string priority;//优先级，话权申请冲突决策的依据。值越大，优先级越高。
	std::string call_privilege;//呼叫默认权限,加入群组时的默认呼叫权限

	std::string token_privilege;//话权默认权限,加入群组的通话后的话权申请默认权限

	std::string register_tatus;
	std::string sip_status;
};


class ns__Dispatch_Login_Response
{
public:
	std::string session_id;	//会话ID，后续消息都必须携带。
	std::string id;
	std::string parentid;
	std::string ttl;
	std::string error_describe;
	bool result;
};

class ns__Unit
{
public:
	ns__Entity base; 	//包含基础的信息
	std::list<ns__Entity> members;//   单元的成员列表，具体成员的类型由Entity. entity_type指定
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
	std::string owner_id;//创建者
	std::string parent_id;//group所属的群组
	std::string number;
	std::string short_number;
	std::list<ns__Participant> participants;
	std::string size;
	bool sealed;//是否为开放式群组        
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
	std::string group_id;//目前只能有一个关联群组的ID
	std::string describe;// 警情描述
	std::string alert_level;//警情级别
	std::string alram_time;//   报警时间
	std::string use_cars;// 出动车辆数
	std::string create_time;//警情的创建时间
	//std::string alert_status ;//警情状态
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
	ns__Entity  id;// 根据 id.entity_type的值，包含下面特定的字段。
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
	ns__EntityData data;//实体的集合信息如果为Deleted时，data只需要包含id字段即可
	std::string session_id;
	std::string error_describe;
	bool result;
};

class ns__Entity_Status_Notification
{
public:
	ns__Entity id;//状态改变的实体,可以为 Account,User,Gateway等与应用服务器有注册的实体
	std::string register_status;//与应用服务器之间的注册状态
	std::string sip_status;//Account专有字段，表示Account和PoC服务器之前的注册状态
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
	ns__Entity group_id;//要加入的群组
	ns__Entity account_id;//请求加入群组的账号
	std::string session_id;
	std::string error_describe;
	bool result;
};

class ns__Dispatch_Participant_Status_Notification_Response
{
public:
	ns__Entity group_id;//群组id
	ns__Entity account_id;//PoC账号ID
	std::string session_status;//会话状态
	std::string session_id;
	std::string error_describe;
	bool result;
};

class ns__MediaMessage
{
public:
	std::string id;//消息编号，消息越早发送，则值越小,服务器自动生成，发送方无需设置.
	std::string sender;//消息发送者，一般为Accout.Entity.name账号
	std::string text;//如果为文本消息时，则包含此字段
	std::string picture_uri;//如果为图片消息时在，则包含此字段，图片的uri地址
	std::string audio_uri;
	std::string audio_length;
	std::string video_uri;
	std::string video_length;
	std::string timestamp;//发送方，无需设置，服务器自动设置服务器接收到消息的时间yyyy-mm-dd HH::MM::SS格式
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
	ns__Entity id;//群组ID\警情id
	ns__MediaMessage messages;//消息列表，如果没有包含任何消息时，说明群组消息已经推刷新完成,可获取到messages 列表中的最后一个消息的时间戳，再次发起刷新
	std::string leave_message_count;//
};

class ns__Dispatch_Media_Message_Notification_Response
{
public:
	std::string id;//接收此消息的群组/或者警情
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
	ns__Entity account_id;//Terminal账号ID
	double longitude;//经度
	double latitude;//维度
	std::string timestamp;//此位置信息获取的时间
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
	std::string history_alert_id;//历史警情id
	std::list<ns__MediaMessage> messages;//消息列表
	std::string size;
	std::string leave_message_count;//剩余消息总数
	std::string error_describe;
	std::string session_id;
	bool result;
};

class ns__Dispatch_Record_Status_Notification_Response
{
public:
	ns__Entity group_id;
	bool recording;//true-正在录音 false-录音已停止
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