#include "IncorrectSizeArray_Exception.h"



IncorrectSizeArray_Exception::IncorrectSizeArray_Exception() : std::exception()
{
}



const char* IncorrectSizeArray_Exception::what() const noexcept
{
	return "Error: Size array = 0";
}