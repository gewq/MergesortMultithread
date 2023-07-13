#include "NullptrArray_Exception.h"



NullptrArray_Exception::NullptrArray_Exception() : std::exception()
{
}



const char* NullptrArray_Exception::what() const noexcept
{
	return "Error: Data array ptr = nullptr";
}