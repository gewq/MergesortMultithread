/**
\file NullptrArray_Exception.h
\brief Класс NullptrArray_Exception - класс-обработчик исключения "Data array ptr = nullptr"
*/

#pragma once

#include <string>
#include <exception>

class NullptrArray_Exception : public std::exception {
  public:
    NullptrArray_Exception();

    virtual const char* what() const noexcept override;
};