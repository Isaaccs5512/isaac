#include <iostream>

#ifdef DEBUG
#define debug() {std::cout<<"Debug mesage:"<<__FILE__<<":"<<__LINE__<<"\n";}
#endif