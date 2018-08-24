#ifndef INTERFACE
#define INTERFACE

#include <iostream>
#include "Python.h"

using namespace std;


extern "C"
{
	void initExVocoder8K();
	static PyObject* ExVocoder8K_synthesizeMemoMemoPCM(PyObject *self, PyObject *args);
}

#endif // INTERFACE

