/**
\file Mergesort.h
\brief Модуль "Сортировка слиянием"

Функция сортировки слиянием - шаблонная. Позволяет сортировать массивы объектов,
поддерживающих операции:
- < (меньше)
- = (присваивание)
*/

#pragma once
#include <iostream>


namespace mergesort{
  /**
  Отсортировать массив
  \param[in] data Указатель на начало массива
  \param[in] size Количество элементов массива
  \param[in] isMultithread Признак использовать многопоточность
  */
  template <typename T>
  void sort(T* data, size_t size, bool isMultithread = true);

  /**
  Протестировать функцию сортировки
  */
  void test();
}

//Реализация
#include "Mergesort.hpp"