
#ifndef _ERROR_EXCEPTION_H_
#define _ERROR_EXCEPTION_H_
#include <exception>
#include <string>

class error_exception : public std::exception
{
public:
	error_exception( const std::string & desc);
	virtual const char * what() const  noexcept (true);
	~error_exception();
private:
	std::string what_;
};

#endif
