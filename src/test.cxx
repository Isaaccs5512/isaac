#include "soapxiaofangProxy.h"
#include "xiaofang.nsmap"
#include "iostream"


#define SERVERIP 192.168.146.128
#define SERVERPORT 18881 
int main()
{
	xiaofangProxy xfp;
	ns__Login_Response response;
	if(xfp.Dispatch_Login( "test1", "123456",response) == SOAP_OK)
		std::cout<<"login successful\n";
	else
		std::cout<<"login fail\n";
	xfp.destroy();
}
