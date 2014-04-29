#include "soapxiaofangProxy.h"
#include "xiaofang.nsmap"
#include "iostream"


#define SERVERIP 192.168.0.228
#define SERVERPORT 18881 
int main()
{
	xiaofangProxy xfp;
	ns__Normal_Response response;
	if(xfp.Dispatch_Logout(9,"test1", "123456",response) == SOAP_OK)
		std::cout<<"login successful\n";
	else
		std::cout<<"login fail\n";
	xfp.destroy();
}
