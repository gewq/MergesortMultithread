/**
\file IncorrectSizeArray_Exception.h
\brief Класс IncorrectSizeArray_Exception - класс-обработчик исключения "Size array = 0"
*/

#pragma once

#include <string>
#include <exception>

class IncorrectSizeArray_Exception : public std::exception {
  public:
    IncorrectSizeArray_Exception();

    virtual const char* what() const noexcept override;
};