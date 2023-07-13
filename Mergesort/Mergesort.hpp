#include <future>
#include <atomic>
#include <cstring>
#include <chrono>
#include <algorithm>
#include <cassert>

//Обработчики исключений
#include "Exceptions/IncorrectSizeArray_Exception.h"
#include "Exceptions/NullptrArray_Exception.h"


namespace {
  //Максимальное количество одновременно выполняемых потоков
  //Если hardware_concurrency() не сможет опеределить кол-во потоков - вернёт 0
  //В этом случае считаем, что поток может быть только 1
  const unsigned int MAX_THREADS = std::max<unsigned int>(1,
                                          std::thread::hardware_concurrency());

  //Минимальное количество элементов, обрабатываемых одним потоком
  const size_t MIN_ELEMENTS_PER_THREAD = 1'000;
}


/**
Соединить два участка массива в один отсортированный участок
\param[in] data Указатель на начало массива
\param[in] leftID Индекс начала левого участка массива
\param[in] middleID Индекс конца левого участка массива / начало правого
\param[in] rightID Индекс конца правого участка массива
*/
template <typename T>
static void merge(T* data,
                  size_t leftID,
                  size_t middleID,
                  size_t rightID)
{
  //Вычислить размеры левой и правой частей
  size_t leftArraySize = middleID - leftID + 1;
  size_t rightArraySize = rightID - middleID;
  //Создать временные массивы левой и правой частей
  T* leftArray = new T[leftArraySize];
  T* rightArray = new T[rightArraySize];
  //Скопировать данные из входного массива во временные массивы
  for (size_t i = 0; i < leftArraySize; ++i) {
    leftArray[i] = data[leftID + i];
  }
  for (size_t i = 0; i < rightArraySize; ++i) {
    rightArray[i] = data[middleID + i + 1];
  }
  size_t i = 0;
  size_t j = 0;
  //Начало левой части
  size_t k = leftID;
  while ( (i < leftArraySize) && (j < rightArraySize) ) {
    //Поместить минимальные элементы обратно во входной массив
    if (leftArray[i] < rightArray[j]) {
      data[k] = leftArray[i];
      ++i;
    }
    else {
      data[k] = rightArray[j];
      ++j;
    }
    ++k;
  }
  //Записать оставшиеся элементы левой части во входной массив
  while (i < leftArraySize) {
    data[k] = leftArray[i];
    ++i;
    ++k;
  }
  //Записать оставшиеся элементы правой части во входной массив
  while (j < rightArraySize) {
    data[k] = rightArray[j];
    ++j;
    ++k;
  }
}



/**
Отсортировать участок массива между заданными индексами в одном потоке
\param[in] data Указатель на начало массива
\param[in] leftID Индекс начала участка массива
\param[in] rightID Индекс конца участка массива
*/
template <typename T>
static void sortBatchOnethread(T* data, size_t leftID, size_t rightID)
{
  if (leftID >= rightID) {
    return;
  }
  size_t middleID = (leftID + rightID - 1) / 2;
  sortBatchOnethread<T>(data, leftID, middleID);
  sortBatchOnethread<T>(data, middleID+1, rightID);
  merge<T>(data, leftID, middleID, rightID);
}



/**
Отсортировать участок массива между заданными индексами с учётом многопоточности
\param[in] data Указатель на начало массива
\param[in] leftID Индекс начала участка массива
\param[in] rightID Индекс конца участка массива
\param[in] runThreads Количество уже запущенных потоков, сортирующих данных массив
*/
template <typename T>
static void sortBatchMultithread(T* data,
                                  size_t leftID,
                                  size_t rightID,
                                  std::atomic_uint* runThreads)
{
  if (leftID >= rightID) {
    return;
  }
  size_t middleID = (leftID + rightID - 1) / 2;
  //Кол-во элементов половины массива > MIN на один поток И
  //Есть незанятые потоки
  if (((middleID - leftID) > MIN_ELEMENTS_PER_THREAD) &&
        (runThreads->load() < MAX_THREADS)){
    //Запустить дополнительный поток
    auto resultLeft = std::async(std::launch::async, [&](){
      sortBatchMultithread(data, leftID, middleID, runThreads);
    });
    ++(*runThreads);
    //В текущем потоке обрабатываем правую половину
    sortBatchMultithread(data, middleID+1, rightID, runThreads);
    //Ждать завершения потока обработки левой половины
    resultLeft.wait();
  }
  //Выполнять в текущем потоке
  else{
    sortBatchMultithread<T>(data, leftID, middleID, runThreads);
    sortBatchMultithread<T>(data, middleID+1, rightID, runThreads);
  }
  merge<T>(data, leftID, middleID, rightID);
}



template <typename T>
void mergesort::sort(T* data, size_t size, bool isMultithread)
{
  //Обработка ошибок
  if (!data){
    throw NullptrArray_Exception();
  }
  if (size == 0){
    throw IncorrectSizeArray_Exception();
  }

  if (isMultithread){
    //Количество запущенных потоков (текущий поток тоже считается)
    std::atomic_uint runThreads = 1;
    sortBatchMultithread<T>(data, 0, size-1, &runThreads);
  }
  else{
    sortBatchOnethread<T>(data, 0, size-1);
  }
}



//-----------------------------------------------------------------------------
/**
Сгенерировать массив случайных чисел int
\param[in] size Количество элементов массива
\return Указатель на начало массива
*/
static int* generateArrayInt(size_t size);

/**
Сгенерировать массив случайных чисел float
\param[in] size Количество элементов массива
\return Указатель на начало массива
*/
static float* generateArrayFloat(size_t size);

/**
Сгенерировать массив случайных ASCII символов char
\param[in] size Количество элементов массива
\return Указатель на начало массива
*/
static char* generateArrayChar(size_t size);



/**
Протестировать сортировку массива
\param[in] data Указатель на начало массива
\param[in] size Количество элементов массива
\param[in] isMultithread Признак использовать многопоточность
*/
template <typename T>
static void testArray(T* data, size_t size, bool isMultithread)
{
  auto start = std::chrono::high_resolution_clock::now();
  //Сортировать массив
  mergesort::sort<T>(data, size, isMultithread);
  auto end = std::chrono::high_resolution_clock::now();

  std::chrono::duration<double, std::milli> duration = end - start;

  if (isMultithread){
    printf("Multithread ");
  }
  else{
    printf("One thread ");
  }
  printf("duration: %f ms\n", duration.count());

  assert(std::is_sorted(data, &data[size-1]) == true);
}



/**
Протестировать сортировку массива целых чисел int
\param[in] size Количество элементов массива
*/
static void testInt(size_t size);

/**
Протестировать сортировку массива чисел float
\param[in] size Количество элементов массива
*/
static void testFloat(size_t size);

/**
Протестировать сортировку массива символов char
\param[in] size Количество элементов массива
*/
static void testChar(size_t size);



void mergesort::test()
{
  srand(time(0));
  const size_t SIZE = 10'000'000;

  printf("Test one- multithread- mergesort array of int, float, char. Size: %zu\n", SIZE);
  testInt(SIZE);
  testFloat(SIZE);
  testChar(SIZE);

  //Тест выброс исключений
  // int testArray[] = {1};
  // mergesort::sort<int>(nullptr, SIZE);
  // mergesort::sort<int>(testArray, 0);
}



static void testInt(size_t size)
{
  //Тестовые наборы данных
  int* testArray_Onethread = generateArrayInt(size);  //Однопоточная реализация
  int* testArray_Multithreading = new int[size];      //Многопоточная реализация

  std::memcpy(testArray_Multithreading, testArray_Onethread, size*sizeof(int));

  printf("Test array of int...\n");
  testArray<int>(testArray_Multithreading, size, true);
  testArray<int>(testArray_Onethread, size, false);
  delete [] testArray_Multithreading;
  delete [] testArray_Onethread;
}



static void testFloat(size_t size)
{
  //Тестовые наборы данных
  float* testArray_Onethread = generateArrayFloat(size);  //Однопоточная реализация
  float* testArray_Multithreading = new float[size];      //Многопоточная реализация

  std::memcpy(testArray_Multithreading, testArray_Onethread, size*sizeof(float));

  printf("Test array of float...\n");
  testArray<float>(testArray_Multithreading, size, true);
  testArray<float>(testArray_Onethread, size, false);
  delete [] testArray_Multithreading;
  delete [] testArray_Onethread;
}



static void testChar(size_t size)
{
  //Тестовые наборы данных
  char* testArray_Onethread = generateArrayChar(size);  //Однопоточная реализация
  char* testArray_Multithreading = new char[size];      //Многопоточная реализация

  std::memcpy(testArray_Multithreading, testArray_Onethread, size*sizeof(char));

  printf("Test array of char...\n");
  testArray<char>(testArray_Multithreading, size, true);
  testArray<char>(testArray_Onethread, size, false);
  delete [] testArray_Multithreading;
  delete [] testArray_Onethread;
}


static int* generateArrayInt(size_t size)
{
  int* array = new int[size];
  std::generate_n(array, size, [](){return rand() % 10'000 - 5'000;});
  return array;
}



static float* generateArrayFloat(size_t size)
{
  float* array = new float[size];
  std::generate_n(array, size, [](){return static_cast<float> (rand() % 10'000 - 5'000);});
  return array;
}



static char* generateArrayChar(size_t size)
{
  char* array = new char[size];
  std::generate_n(array, size, [](){return static_cast<char> (rand() % 126 + 33);});
  return array;
}