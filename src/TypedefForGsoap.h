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
//AccountStop 状态时，系统不提供对账号的任何服务，包括注册。
enum ns__AccountServiceStatus
{
	AccountStop,
	AccountActive,
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

enum ns__GpsReportMode
{
	GpsManualPull,// 拉模式，服务器请求一次，终端应答一次
	GpsManualPush ,// 手动推模式，在服务器请求开始和结束之间，间隔的发送位置信息
	GpsAutoPush ,// 自动推送模式，终端注册上应用服务器以后，间隔发送位置信息
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
	enum ns__AccountType account_type;//PoC账号类型
	std::string number; //号码，SIP注册账号
	std::string  short_number; //短号，应用服务器内部唯一号码，内部号码
	std::string  password; 
	enum ns__AccountServiceStatus service_status;
	uint32 priority;//优先级，话权申请冲突决策的依据。值越大，优先级越高。
	enum ns__CallPrivilege call_privilege;//呼叫默认权限,加入群组时的默认呼叫权限

	enum ns__TokenPrivilege token_privilege;//话权默认权限,加入群组的通话后的话权申请默认权限

	enum ns__GpsReportMode gps_report_mode;//GPS位置信息上报模式
	uint32 ttl;
	enum ns__RegisterStatus status;
	enum ns__RegisterStatus sip_status;
	ns__Address address; 
};


class ns__Login_Response
{
public:
	uint32 session_id;	//会话ID，后续消息都必须携带。
	ns__User user;		//用户在系统中的信息
	uint32 ttl;			//保活间隔,单位s
};

class ns__Unit
{
public:
	ns__Entity base; 	//包含基础的信息
	bool include_members;//是否包含members字段，如果此值为false时，则忽略members字段
	ns__Entity members;//   单元的成员列表，具体成员的类型由Entity. entity_type指定
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
	ns__Address address;// 地址

	bool include_allocate_counts;
	ns__Entity allocate_counts;//     分配给在媒体网关的账号
	bool include_active_counts;
	ns__Entity active_counts;//  处于活动的账号	        
	enum ns__RegisterStatus status;//  注册状态
};


class ns__Alert
{
public:
	ns__Entity base;
	ns__Entity group;//关联群组的ID
	std::string describe;// 警情描述
	enum ns__AlertLevel level;//警情级别
	std::string alram_time;//   报警时间
	uint32 use_cars;// 出动车辆数
	std::string create_time;//警情的创建时间
	enum ns__AlertStatus status ;//警情状态
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
	ns__Entity  id;// 根据 id.entity_type的值，包含下面特定的字段。
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
	ns__EntityData data;//实体的集合信息如果为Deleted时，data只需要包含id字段即可
};

class ns__Entity_Status_Notification
{
public:
	ns__Entity id;//状态改变的实体,可以为 Account,User,Gateway等与应用服务器有注册的实体
	enum ns__RegisterStatus status;//与应用服务器之间的注册状态
	enum ns__RegisterStatus sip_status;//Account专有字段，表示Account和PoC服务器之前的注册状态
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
	ns__Entity group_id;//要加入的群组
	ns__Entity account_id;//请求加入群组的账号
};

class ns__Participant_Status_Notification
{
public:
	ns__Entity group_id;//群组id
	ns__Entity account_id;//PoC账号ID
	enum ns__SessionStatus status;//会话状态
};

class ns__MediaMessage
{
public:
	uint32 id;//消息编号，消息越早发送，则值越小,服务器自动生成，发送方无需设置.
	std::string sender;//消息发送者，一般为Accout.Entity.name账号
	std::string text;//如果为文本消息时，则包含此字段
	std::string picture_uri;//如果为图片消息时在，则包含此字段，图片的uri地址
	std::string audio_uri;
	uint32 audio_length;
	std::string video_uri;
	uint32 video_length;
	std::string timestamp;//发送方，无需设置，服务器自动设置服务器接收到消息的时间yyyy-mm-dd HH::MM::SS格式
};

class ns__Media_Message
{
public:
	ns__Entity id;//群组ID\警情id
	ns__MediaMessage messages;//消息列表，如果没有包含任何消息时，说明群组消息已经推刷新完成。
								//可获取到messages 列表中的最后一个消息的时间戳，再次发起刷新
	uint32 leave_message_count;//
};

class ns__Media_Message_Notification
{
public:
	uint32 id;//接收此消息的群组/或者警情
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
	ns__Entity account_id;//Terminal账号ID
	double longitude;//经度
	double latitude;//维度
	std::string timestamp;//此位置信息获取的时间
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
	uint32 history_alert_id;//历史警情id
	std::list<ns__MediaMessage> messages;//消息列表
	uint32 leave_message_count;//剩余消息总数
};

class ns__Dispatch_Record_Status
{
public:
	ns__Entity group_id;
	bool recording;//true-正在录音 false-录音已停止
};
