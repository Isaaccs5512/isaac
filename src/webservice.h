#import "TypedefForGsoap.h"
/*
*brief		:����̨��¼�ӿ�
*param in	:name
*param in	:password
*param out	:response
*/
int ns__Dispatch_Login(std::string username,
						std::string password,
						ns__Dispatch_Login_Response &response);

/*
brief		:����̨ע����¼�ӿ�
param in	:name
param in	:
param out	:
*/
int ns__Dispatch_Logout(std::string session_id,
						std::string name,
						std::string password,
						ns__Normal_Response &response);


/*
brief		:��������
param in	:
param in	:
param out	:
*/
int ns__Dispatch_Keepalive_Request(std::string session_id,
						ns__Normal_Response &response);


/*
brief		:��ȡ����ϵͳ�ĵ�Ԫ�ܹ���������Ԫ�����Ⱥ�顢PoC�˺ŵȣ���Ҫͨ���ݹ�ķ�ʽ������Response�����ͣ�
			����ǵ�Ԫ����Ԫ�����˳�Ա����Ҫ�Ե�Ԫ�ĳ�Ա���еݹ��ȡ��Ϣ.
param in	:id //Ҫ��ȡ��ʵ���id(��¼ʱ���᷵��parent_id��һ��Ӵ�ֵ��ʼ���ݹ��ȡ��������)
param in	:
param out	:response
*/
int ns__Dispatch_Entity_Request(std::string session_id,
								std::string id,
								ns__Dispatch_Entity_Request_Response &response);

/*
brief		:�������������ͣ���ʵ������ݱ��������޸ġ�״̬���¡�ɾ��ʱ��ͬ��֪ͨ��Ȩ�޵ĵ���̨��
param in	:
param in	:
param out	:response
*/
int ns__Dispatch_Entity_Nofitication(std::string session_id,
			ns__Dispatch_Entity_Nofitication_Response &response);

/*
brief		:����Ⱥ��
param in	:group  //base.id�ֶο��Բ���������Ϊ0
param in	:
param out	:response  //base.idΪϵͳ�����Ψһ���
*/
int ns__Dispatch_Append_Group(std::string session_id,
								ns__Group group,
								ns__Dispatch_Append_Group_Response &response);

/*
brief		:����Ⱥ�����Ա�б��������Ϣ
param in	:group  //base.id ָ��Ҫ�޸ĵ�Ⱥ��group.include_members�ֶ�Ϊ����Ϊfalseʱ��
						�˴���Ҫ�޸ĳ�Ա������������.
param in	:
param out	:response
*/
int ns__Dispatch_Modify_Group(std::string session_id,
							ns__Group group,
							ns__Dispatch_Modify_Group_Response &response);
/*
brief		:�޸�Ⱥ���Ա�б�
param in	:group_id // ��Ҫ�޸ĵ�Ⱥ���id
param in	:request   
param out	:response
*/
int ns__Dispatch_Modify_Participants(std::string session_id,
									ns__Modify_Participant request,
									ns__Dispatch_Modify_Participants_Response &response);

/*
brief		:ɾ��Ⱥ�飬ɾ��Ⱥ�飬��ͬ��ɾ����Ⱥ���������Ϣ
param in	:group_id // ��Ҫ�޸ĵ�Ⱥ���id
param in	:request   
param out	:
*/
int ns__Dispatch_Delete_Group(std::string session_id,
							std::string group_id,
							ns__Normal_Response &response);

/*
brief		:Ⱥ�鴴���������޸ģ�ɾ��֪ͨʹ��
param in	:
param in	:  
param out	:response
*/
int ns__Dispatch_Dispatch_Participants_Notification(std::string session_id,
			ns__Dispatch_Dispatch_Participants_Notification_Response &response);

/*
brief		:ĳ������Ⱥ���е��նˣ���Ӧ����ĳ����յ�Ⱥ��ʱ����̨�޷����ߣ�������ʹ���Ϣ��֪����̨��
				����ն˶Է��ʽȺ�����δ˹��ܣ��߷�����Ҳ�����ṩ��
param in	:
param in	:  
param out	:response
*/
int ns__Dispatch_Join_Group_Request_Nofitication(std::string session_id,
			ns__Dispatch_Join_Group_Request_Nofitication_Response &response);

/*
brief		: Ⱥ���Ա״̬֪ͨ,
param in	:
param in	:  
param out	:response
*/
int ns__Dispatch_Participant_Status_Notification(std::string session_id,
			ns__Dispatch_Participant_Status_Notification_Response &response);

/*
brief		: ����ˢ��ָ��Ⱥ�����Ϣ�� ˢ������Ⱥ����Ϣ������Ҫͨ��������ϡ�
param in	:id,//Ⱥ��ID/���߾���id
param in	:from_message_id,//��ѯ����ϢIDֵ������from_messge_id
param in	:from_time,//��ѯ����Ϣ�ķ���ʱ�䣬����from_time
param in	:max_message_count,//default = 64��Ӧ������������������Ϣ������Ⱥ�����Ϣ����̫�࣬����ͻ�����������ݹ��󣬷��������ܷ��ص���Ϣ��ָ����Ҫ�١�
param out	:response
*/
int ns__Dispatch_Media_Message_Request(std::string session_id,
										std::string group_id,
										std::string from_message_id,
										std::string from_time,
										std::string max_message_count,
										ns__Dispatch_Media_Message_Request_Response &response);

/*
brief		:Ӧ�÷�����ת��Ⱥ����Ϣ������̨
param in	:
param in	:  
param out	:response
*/
int ns__Dispatch_Media_Message_Notification(std::string session_id,
			ns__Dispatch_Media_Message_Notification_Response &response);

/*
brief		:Ⱥ���Ա������Ⱥ�齨��ͨ��֪ͨ,����ΪȺ���Ա,
			//���ն���PoC�������������еĺ���Ȩ�ޣ�CallPrivilege��ΪCallPolicy���������޷�����ʱ��
			//ͨ������Ϣת��������̨������Ա����󣬿���ʹ�������û�����(ns__Dispatch_Appoint_Participant_Speak_Request)������
param in	:
param in	:  
param out	:
*/
int ns__Dispatch_Participant_Connect_Request_Notification(std::string session_id,
			ns__Dispatch_Participant_Connect_Request_Notification_Response &response);

/*
brief		:Ӧ�÷�����ת��Ⱥ����Ϣ������̨,��Ա��Ⱥ���е�״̬ΪInGroupʱ��Ҫ����ʱ��
			//���һ�ȨȨ�ޣ�TokenPrivilege��ΪTokenPolicyʱ��������ת�����󵽵���̨������̨�����,
			//����ִ��ָ�����Ե�������ns__Dispatch_Appoint_Participant_Speak_Request
param in	:
param in	:  
param out	:
*/
int ns__Dispatch_Participant_Speak_Request_Notification(std::string session_id,
			ns__Dispatch_Participant_Speak_Request_Notification_Response &response);

/*
brief		:����̨�������������ĳ���ն���Ⱥ�齨��SIP�Ự����������һ���ն˽�����Ե���С�
param in	:group_id
param in	:account_id //���û�а���account_id��Ⱥ������״̬ΪIDLE��Ⱥ���Ա������ֻ����ָ���ĳ�Ա.
param out	:����Ϣ�壬�ɹ�Ӧ�𣬱����������Ѿ��������ָ����ն��˺ţ�
			//����״̬ͨ��Ⱥ���Ա״̬֪ͨ��ȡ; ���벻�ڳ�Ա�б��е��˺ţ����յ�Ӧ�����.
			//���벻�ڳ�Ա�б��еĲ�����ʹ�ü���Ự
*/
int ns__Dispatch_Invite_Participant_Request(std::string session_id,
											std::string group_id,
											std::string account_id,
											ns__Normal_Response &response);

/*
brief		:����̨����������Ҷ�ָ���ն���ĳ��Ⱥ���SIP�Ự��
param in	:group_id //Ⱥ��id
param in	:account_id  //account_id������Ⱥ��ĳ�Ա,���û�а���account_id�� �Ҷ����еĳ�Ա�ĻỰ������ֻ�Ҷ�ָ���ĳ�Ա
param out	:����Ϣ�壬�ɹ�Ӧ��˵���������Ѿ����ܲ���ִ��������
			//����״̬ͨ��Ⱥ���Ա״̬֪ͨ��ȡ.���ָ���ĳ�Ա�����б��л��߲�����InGroup��Talking״̬��
			//���յ�Ӧ�����.
*/
int ns__Dispatch_Drop_Participant_Request(std::string session_id,
											std::string group_id,
											std::string account_id,
											ns__Normal_Response &response);

/*
brief		:ָ���ͷ�ĳ��Ⱥ������ߵĻ�Ȩ��
param in	:group_id
param in	:account_id //account_id������Ⱥ��ĳ�Ա  
param out	: account_id��ָ��Ⱥ���У��������talking״̬���򷵻�Ӧ�����.
*/
int ns__Dispatch_Release_Participant_Token_Request(std::string session_id,
													std::string group_id,
													std::string account_id,
													ns__Normal_Response &response);

/*
brief		:����ָ̨��ĳ�������߷��ԡ�
param in	:group_id
param in	:account_id  
param out	:
*/
int ns__Dispatch_Appoint_Participant_Speak_Request(std::string session_id,
													std::string group_id,
													std::string account_id,
													ns__Normal_Response &response);

/*
brief		:����Ա����ָ��Ⱥ��ĻỰ��ָ��Ⱥ��ĳ�Ա��������Ա����һ��������ڼ�������
param in	:group_id
param in	:  
param out	:
*/
int ns__Dispatch_Jion_Group_Request(std::string session_id,
									std::string group_id,
									ns__Normal_Response &response);

/*
brief		:����Ա�뿪ָ��Ⱥ��ĻỰ��ָ��Ⱥ��ĳ�Ա��������Ա����һ��������ڼ�������
param in	:
param in	:  
param out	:
*/
int ns__Dispatch_Leave_Group_Request(std::string session_id,
									std::string group_id,
									ns__Normal_Response &response);

/*
brief		:����Ⱥ��ʱ���Ự״̬�ı���
param in	:
param in	:  
param out	:
*/
int ns__Dispatch_Session_Status_Notification(std::string session_id,
			ns__Dispatch_Session_Status_Notification_Response &response);

/*
brief		:���Է���������Ϣ��ͼƬ��Ϣ��ָ����Ⱥ���Ա������ȫ����
param in	:id//������Ϣ��Ⱥ��ID���߾���;��Ϣʵ��������Ⱥ�������
param in	:message
param out	:
*/
int ns__Dispatch_Send_Message_Request(std::string session_id,
									std::string group_id,
									ns__MediaMessage mediamessage,
									ns__Normal_Response &response);

/*
brief		:��ʼ¼��
param in	:group_id//ָ��¼����Ⱥ��
param in	:  
param out	:
*/
int ns__Dispatch_Start_Record_Request(std::string session_id,
									std::string group_id,
									ns__Normal_Response &response);

/*
brief		:ֹͣ¼��
param in	:group_id//ָ��¼����Ⱥ��
param in	:  
param out	:
*/
int ns__Dispatch_Stop_Record_Request(std::string session_id,
									std::string group_id,
									ns__Normal_Response &response);

/*
brief		:¼��״̬֪ͨ
param in	:
param in	:  
param out	:response
*/
int ns__Dispatch_Record_Status_Notification(std::string session_id,
			ns__Dispatch_Record_Status_Notification_Response &response);

/*
brief		:������Ϣ
param in	:subscribing,//true-���� false-ȡ������
param in	:account_id,//���Ļ���ȡ�����˺�һ��ֻ���Զ����˺�����ΪTerminal�ġ�
param in	:ttl,//�������ڣ���λs,subscribing==trueʱ��Ҫ,����Ĭ��Ϊ60s
param out	:
*/
int ns__Dispatch_Subscribe_Account_Location_Request(std::string session_id,
												bool subscribing,
												std::string account_id,
												std::string ttl,
												ns__Normal_Response &response);

/*
brief		:����̨�������ն���Ϣ������Ϣ�ϱ�ʱ֪ͨ������̨
param in	:
param in	:  
param out	:response
*/
int ns__Dispatch_Account_Location_Notification(std::string session_id,
			ns__Dispatch_Account_Info_Notification_Response &response);

/*
brief		:��������
param in	:alert//base.id == 0 ���߲�����base.id.name ��������group ���Բ�����
param in	:acount//������صĳ�Ա�����alert������groupʱ������Ҫ�������ֶΣ�
					//alert.group������ʱ��ͨ��members����һ��Ⱥ����þ������
param out	:response
*/
int ns__Dispatch_Append_Alert_Request(std::string session_id,
										ns__Alert alert,
										std::list<ns__Account> members,
										std::string size,
										ns__Dispatch_Append_Alert_Request_Response &response);

/*
brief		:�˴�ֻ�漰���޸ľ����������Ϣ������Ǿ���ĳ�Ա�޸ģ�תΪʹ��Ⱥ��ĳ�Ա�޸���ؽӿ�
param in	:alert
param in	:  
param out	:
*/
int ns__Dispatch_Modify_Alert_Request(std::string session_id,
									ns__Alert alert,
									ns__Normal_Response &response);

/*
brief		:�������飬����ɾ����
param in	:alert_id
param in	:  
param out	:
*/
int ns__Dispatch_Stop_Alert_Request(std::string session_id,
									std::string alert_id,
									ns__Normal_Response &response);

/*
brief		:�������֪ͨ
param in	:alert_id
param in	:  
param out	:
*/
int ns__Dispatch_Alert_Overed_Notification(std::string session_id,
			ns__Dispatch_Alert_Overed_Notification_Response &response);

/*
brief		:����ָ��ʱ�䷶Χ��ģ�����ƵȲ�ѯ��������ɵľ���
param in	:name//���ݾ������ν��в�ѯ
param in	:create_time_from,create_time_to//���ݾ���Ĵ���ʱ����в�ѯ
param in	:alram_time_from,alram_time_to//���ݱ���ʱ����в�ѯ
param in	:over_time_from,over_time_to//���ݾ������ʱ�䣬���в�ѯ
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
brief		:��ѯĳ���������ϸ��Ϣ,����ֻ�Ƿ���������Ϣ
param in	:alert_id
param in	:  
param out	:response
*/
int ns__Dispatch_Alert_Request(std::string session_id,
								std::string alert_id,
								ns__Dispatch_Alert_Request_Response &response);
/*
brief		:������Ϣ��ѯ
param in	:history_alert_id//��ʷ����id
param in	:from_message_id//���Ǹ���Ϣid��ʼ,0���߲���������ӵ�һ����Ϣ��ʼ
param in	:from_time//����Ϣ���յ���ʱ�俪ʼ��
param in	:max_message_count//Ӧ���У����������ٸ���Ϣ����С�ڷ�������Ĭ��ֵ
param out	:response
*/
int ns__Dispatch_History_Alert_Message_Request(std::string session_id,
												std::string history_alert_id,
												std::string from_message_id,
												std::string from_time,
												std::string max_message_count,
												ns__Dispatch_History_Alert_Message_Request_Response &response);

/*
brief		:ɾ��ָ������ʷ���飬��ͬ��ɾ������������Ϣ��¼��ɾ�����ڵ�Ⱥ��
param in	:history_alert_id
param in	:  
param out	:
*/
int ns__Dispatch_Delete_History_Alert_Request(std::string session_id,
											std::string history_alert_id,
											ns__Normal_Response &response);

/*
brief		:��ָ����id�߳�Ⱥ�飬���û���ƶ�id����ô���Ⱥ��
param in	:history_alert_id
param in	:  
param out	:
*/
int ns__Dispatch_Kick_Participant_Request(std::string session_id,
											std::string group_id,
											std::string account_id,
											ns__Normal_Response &response);
