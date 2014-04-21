#include "error_exception.hpp"


error_exception ::error_exception ( const std::string & desc):what_(desc){}
const char * error_exception ::what() const  noexcept (true){ return what_.c_str();}
error_exception ::~error_exception (){ }
