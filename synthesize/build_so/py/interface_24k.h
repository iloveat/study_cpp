#ifndef INTERFACE
#define INTERFACE

#include <iostream>
#include "Python.h"

using namespace std;


extern "C"
{
	void initExVocoder24K();
	static PyObject* ExVocoder24K_synthesizeMemoMemoPCM(PyObject *self, PyObject *args);
}

#endif // INTERFACE

