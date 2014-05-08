#import "TypedefForGsoap.h"
/*
*brief		:控制台登录接口
*param in	:name
*param in	:password
*param out	:response
*/
int ns__Dispatch_Login(std::string username,
						std::string password,
						ns__Dispatch_Login_Response &response);

/*
brief		:控制台注销登录接口
param in	:name
param in	:
param out	:
*/
int ns__Dispatch_Logout(std::string session_id,
						std::string name,
						std::string password,
						ns__Normal_Response &response);


/*
brief		:保活请求
param in	:
param in	:
param out	:
*/
int ns__Dispatch_Keepalive_Request(std::string session_id,
						ns__Normal_Response &response);


/*
brief		:获取整个系统的单元架构（包含单元下面的群组、PoC账号等，需要通过递归的方式）根据Response的类型，
			如果是单元，单元包含了成员，需要对单元的成员进行递归获取信息.
param in	:id //要获取的实体的id(登录时，会返回parent_id，一般从此值开始，递归获取所有数据)
param in	:
param out	:response
*/
int ns__Dispatch_Entity_Request(std::string session_id,
								std::string id,
								ns__Dispatch_Entity_Request_Response &response);

/*
brief		:服务器主动发送，当实体的内容被新增、修改、状态更新、删除时，同步通知有权限的调度台。
param in	:
param in	:
param out	:response
*/
int ns__Dispatch_Entity_Nofitication(std::string session_id,
			ns__Dispatch_Entity_Nofitication_Response &response);

/*
brief		:创建群组
param in	:group  //base.id字段可以不包含或者为0
param in	:
param out	:response  //base.id为系统分配的唯一编号
*/
int ns__Dispatch_Append_Group(std::string session_id,
								ns__Group group,
								ns__Dispatch_Append_Group_Response &response);

/*
brief		:创建群组除成员列表以外的信息
param in	:group  //base.id 指定要修改的群组group.include_members字段为设置为false时，
						此处不要修改成员，服务器忽略.
param in	:
param out	:response
*/
int ns__Dispatch_Modify_Group(std::string session_id,
							ns__Group group,
							ns__Dispatch_Modify_Group_Response &response);
/*
brief		:修改群组成员列表
param in	:group_id // 需要修改的群组的id
param in	:request   
param out	:response
*/
int ns__Dispatch_Modify_Participants(std::string session_id,
									ns__Modify_Participant request,
									ns__Dispatch_Modify_Participants_Response &response);

/*
brief		:删除群组，删除群组，会同步删除与群组关联的消息
param in	:group_id // 需要修改的群组的id
param in	:request   
param out	:
*/
int ns__Dispatch_Delete_Group(std::string session_id,
							std::string group_id,
							ns__Normal_Response &response);

/*
brief		:群组创建，配置修改，删除通知使用
param in	:
param in	:  
param out	:response
*/
int ns__Dispatch_Dispatch_Participants_Notification(std::string session_id,
			ns__Dispatch_Dispatch_Participants_Notification_Response &response);

/*
brief		:某个不在群组中的终端，相应加入某个封闭的群组时，后台无法决策，因此推送此消息告知调度台。
				如果终端对封闭式群组屏蔽此功能，者服务器也无需提供。
param in	:
param in	:  
param out	:response
*/
int ns__Dispatch_Join_Group_Request_Nofitication(std::string session_id,
			ns__Dispatch_Join_Group_Request_Nofitication_Response &response);

/*
brief		: 群组成员状态通知,
param in	:
param in	:  
param out	:response
*/
int ns__Dispatch_Participant_Status_Notification(std::string session_id,
			ns__Dispatch_Participant_Status_Notification_Response &response);

/*
brief		: 可以刷新指定群组的消息， 刷新所有群组消息可能需要通过几次组合。
param in	:id,//群组ID/或者警情id
param in	:from_message_id,//查询的消息ID值，大于from_messge_id
param in	:from_time,//查询的消息的发送时间，大于from_time
param in	:max_message_count,//default = 64；应答中最多包含多少条消息。由于群组的消息可能太多，如果客户端请求的数据过大，服务器可能返回的消息比指定的要少。
param out	:response
*/
int ns__Dispatch_Media_Message_Request(std::string session_id,
										std::string group_id,
										std::string from_message_id,
										std::string from_time,
										std::string max_message_count,
										ns__Dispatch_Media_Message_Request_Response &response);

/*
brief		:应用服务器转发群组消息到调度台
param in	:
param in	:  
param out	:response
*/
int ns__Dispatch_Media_Message_Notification(std::string session_id,
			ns__Dispatch_Media_Message_Notification_Response &response);

/*
brief		:群组成员请求与群组建立通话通知,必须为群组成员,
			//当终端与PoC服务器建立呼叫的呼叫权限（CallPrivilege）为CallPolicy，服务器无法决策时，
			//通过此消息转发给调度台，调度员允许后，可以使用邀请用户命令(ns__Dispatch_Appoint_Participant_Speak_Request)来允许。
param in	:
param in	:  
param out	:
*/
int ns__Dispatch_Participant_Connect_Request_Notification(std::string session_id,
			ns__Dispatch_Participant_Connect_Request_Notification_Response &response);

/*
brief		:应用服务器转发群组消息到调度台,成员在群组中的状态为InGroup时，要发言时，
			//并且话权权限（TokenPrivilege）为TokenPolicy时，服务器转发请求到调度台。调度台允许后,
			//可以执行指定发言调度命令ns__Dispatch_Appoint_Participant_Speak_Request
param in	:
param in	:  
param out	:
*/
int ns__Dispatch_Participant_Speak_Request_Notification(std::string session_id,
			ns__Dispatch_Participant_Speak_Request_Notification_Response &response);

/*
brief		:调度台请求服务器邀请某个终端与群组建立SIP会话。或者与另一个终端建立点对点呼叫。
param in	:group_id
param in	:account_id //如果没有包含account_id则群邀所有状态为IDLE的群组成员。否则只邀请指定的成员.
param out	:无消息体，成功应答，表名服务器已经发送相关指令给终端账号，
			//后续状态通过群组成员状态通知获取; 邀请不在成员列表中的账号，会收到应答错误.
			//邀请不在成员列表中的操作请使用加入会话
*/
int ns__Dispatch_Invite_Participant_Request(std::string session_id,
											std::string group_id,
											std::string account_id,
											ns__Normal_Response &response);

/*
brief		:调度台请求服务器挂断指定终端与某个群组的SIP会话。
param in	:group_id //群组id
param in	:account_id  //account_id必须是群组的成员,如果没有包含account_id则 挂断所有的成员的会话，否则只挂断指定的成员
param out	:无消息体，成功应答，说明服务器已经接受并且执行了请求，
			//后续状态通过群组成员状态通知获取.如果指定的成员不在列表中或者不处于InGroup、Talking状态，
			//会收到应答错误.
*/
int ns__Dispatch_Drop_Participant_Request(std::string session_id,
											std::string group_id,
											std::string account_id,
											ns__Normal_Response &response);

/*
brief		:指定释放某个群组参与者的话权。
param in	:group_id
param in	:account_id //account_id必须是群组的成员  
param out	: account_id在指定群组中，如果不是talking状态，则返回应答错误.
*/
int ns__Dispatch_Release_Participant_Token_Request(std::string session_id,
													std::string group_id,
													std::string account_id,
													ns__Normal_Response &response);

/*
brief		:调度台指定某个发言者发言。
param in	:group_id
param in	:account_id  
param out	:
*/
int ns__Dispatch_Appoint_Participant_Speak_Request(std::string session_id,
													std::string group_id,
													std::string account_id,
													ns__Normal_Response &response);

/*
brief		:操作员加入指定群组的会话（指定群组的成员新增操作员），一般可以用于监听命令
param in	:group_id
param in	:  
param out	:
*/
int ns__Dispatch_Jion_Group_Request(std::string session_id,
									std::string group_id,
									ns__Normal_Response &response);

/*
brief		:操作员离开指定群组的会话（指定群组的成员新增操作员），一般可以用于监听命令
param in	:
param in	:  
param out	:
*/
int ns__Dispatch_Leave_Group_Request(std::string session_id,
									std::string group_id,
									ns__Normal_Response &response);

/*
brief		:监听群组时，会话状态的报告
param in	:
param in	:  
param out	:
*/
int ns__Dispatch_Session_Status_Notification(std::string session_id,
			ns__Dispatch_Session_Status_Notification_Response &response);

/*
brief		:可以发送文字消息和图片消息到指定的群组成员，或者全部。
param in	:id//接收消息的群组ID或者警情;消息实际上是与群组关联的
param in	:message
param out	:
*/
int ns__Dispatch_Send_Message_Request(std::string session_id,
									std::string group_id,
									ns__MediaMessage mediamessage,
									ns__Normal_Response &response);

/*
brief		:开始录音
param in	:group_id//指定录音的群组
param in	:  
param out	:
*/
int ns__Dispatch_Start_Record_Request(std::string session_id,
									std::string group_id,
									ns__Normal_Response &response);

/*
brief		:停止录音
param in	:group_id//指定录音的群组
param in	:  
param out	:
*/
int ns__Dispatch_Stop_Record_Request(std::string session_id,
									std::string group_id,
									ns__Normal_Response &response);

/*
brief		:录音状态通知
param in	:
param in	:  
param out	:response
*/
int ns__Dispatch_Record_Status_Notification(std::string session_id,
			ns__Dispatch_Record_Status_Notification_Response &response);

/*
brief		:订阅信息
param in	:subscribing,//true-订阅 false-取消订阅
param in	:account_id,//订阅或者取消的账号一般只可以订阅账号类型为Terminal的。
param in	:ttl,//推送周期，单位s,subscribing==true时需要,否则默认为60s
param out	:
*/
int ns__Dispatch_Subscribe_Account_Location_Request(std::string session_id,
												bool subscribing,
												std::string account_id,
												std::string ttl,
												ns__Normal_Response &response);

/*
brief		:调度台订阅了终端信息后，有信息上报时通知给调度台
param in	:
param in	:  
param out	:response
*/
int ns__Dispatch_Account_Location_Notification(std::string session_id,
			ns__Dispatch_Account_Info_Notification_Response &response);

/*
brief		:创建警情
param in	:alert//base.id == 0 或者不包含base.id.name 警情名次group 可以不包含
param in	:acount//警情相关的成员，如果alert关联了group时，则不需要包含此字段，
					//alert.group不存在时，通过members创建一个群组与该警情管理。
param out	:response
*/
int ns__Dispatch_Append_Alert_Request(std::string session_id,
										ns__Alert alert,
										std::list<ns__Account> members,
										std::string size,
										ns__Dispatch_Append_Alert_Request_Response &response);

/*
brief		:此处只涉及到修改警情的配置信息。如果是警情的成员修改，转为使用群组的成员修改相关接口
param in	:alert
param in	:  
param out	:
*/
int ns__Dispatch_Modify_Alert_Request(std::string session_id,
									ns__Alert alert,
									ns__Normal_Response &response);

/*
brief		:结束警情，不可删除；
param in	:alert_id
param in	:  
param out	:
*/
int ns__Dispatch_Stop_Alert_Request(std::string session_id,
									std::string alert_id,
									ns__Normal_Response &response);

/*
brief		:警情结束通知
param in	:alert_id
param in	:  
param out	:
*/
int ns__Dispatch_Alert_Overed_Notification(std::string session_id,
			ns__Dispatch_Alert_Overed_Notification_Response &response);

/*
brief		:可以指定时间范围，模糊名称等查询警情已完成的警情
param in	:name//根据警情名次进行查询
param in	:create_time_from,create_time_to//根据警情的创建时间进行查询
param in	:alram_time_from,alram_time_to//根据报警时间进行查询
param in	:over_time_from,over_time_to//根据警情结束时间，进行查询
param out	:response
*/
int ns__Dispatch_History_Alert_Request(std::string session_id,
										std::string name,
										std::string create_time_from,
										std::string create_time_to,
										std::string alram_time_from,
										std::string alram_time_to,
										std::string over_time_from,
										std::string over_time_to,
										ns__Dispatch_History_Alert_Request_Reponse &response);

/*
brief		:查询某个警情的明细信息,这里只是返回配置信息
param in	:alert_id
param in	:  
param out	:response
*/
int ns__Dispatch_Alert_Request(std::string session_id,
								std::string alert_id,
								ns__Dispatch_Alert_Request_Response &response);
/*
brief		:警情消息查询
param in	:history_alert_id//历史警情id
param in	:from_message_id//从那个消息id开始,0或者不包含，则从第一个消息开始
param in	:from_time//从消息接收到的时间开始…
param in	:max_message_count//应答中，最多包含多少个消息必须小于服务器的默认值
param out	:response
*/
int ns__Dispatch_History_Alert_Message_Request(std::string session_id,
												std::string history_alert_id,
												std::string from_message_id,
												std::string from_time,
												std::string max_message_count,
												ns__Dispatch_History_Alert_Message_Request_Response &response);

/*
brief		:删除指定的历史警情，会同步删除警情的相关消息记录。删除对于的群组
param in	:history_alert_id
param in	:  
param out	:
*/
int ns__Dispatch_Delete_History_Alert_Request(std::string session_id,
											std::string history_alert_id,
											ns__Normal_Response &response);

/*
brief		:将指定的id踢出群组，如果没有制定id，那么清空群组
param in	:history_alert_id
param in	:  
param out	:
*/
int ns__Dispatch_Kick_Participant_Request(std::string session_id,
											std::string group_id,
											std::string account_id,
											ns__Normal_Response &response);
