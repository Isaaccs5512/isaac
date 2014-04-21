// md5.h: interface for the Cmd5 class.
//
//////////////////////////////////////////////////////////////////////

#ifndef MD5_H__20060525
#define MD5_H__20060525 "MD5_H__20060525(76098()*&()*Q#&(*Y(*S^D*(&S^*&%^)&(TXSC&*)%)&Q*#$"

#include <string.h>
#include <string>


#define MD5_DIGEST_LENGTH   16


std::string get_md5( const std::string & input);
void MD5(unsigned char *output, unsigned char *input , unsigned int inputlen);
void HMAC_MD5(unsigned char * text, int textLen, unsigned char * key, int keyLen, unsigned char * digest);


#endif // !defined(AFX_MD5_H__3451190C_EF0D_49C3_8C85_7FAAE60C331F__INCLUDED_)



