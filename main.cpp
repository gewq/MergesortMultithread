#include "Mergesort/Mergesort.h"



int main()
{
	try{
		mergesort::test();
	}
	catch (std::exception& error) {
		std::cerr << error.what() << std::endl;
	}
	catch (...) {
		std::cerr << "Undefined exception" << std::endl;
	}
	return EXIT_SUCCESS;
}