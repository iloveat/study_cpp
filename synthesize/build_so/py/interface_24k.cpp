#include <iostream>
#include <assert.h>
#include "../../vocoder/common.h"
#include "../../vocoder/world.h"
#include "interface_24k.h"
#include "Python.h"
#include "numpy/ndarrayobject.h"

using namespace std;


short* synthesizeMemoMemoPCM(const int &offline,		//offline:1, online:0
							 const int &num_spectrum,	//dimension of mgc, offline:19, online:60
							 const int &num_skip,		//skipped points
							 const double &world_alpha,	//parameter alpha of world
							 const double *params, 
							 const int &rows, 
							 const int &cols,
							 int &pcm_length)
{
	if(offline != 1 && offline != 0) {
		printf("offline must be 0 or 1\n");
		return NULL;
	}

	int num_column = num_spectrum+1+3;  //note: when in 24k, bap=3
	if(num_column != cols) {
		printf("column number does not match\n");
		return NULL;
	}

	long long len_all = rows*cols*sizeof(double);
	if(len_all % (sizeof(double)*num_column) != 0) {
		printf("check your data format\n");
		return NULL;
	}

	int num_frame = len_all/(sizeof(double)*num_column);  // calculate frame number
	if(num_frame != rows) {
		printf("row number does not match\n");
		return NULL;
	}

	float **world_input_param = (float**)malloc(num_frame*sizeof(float*));
	if(world_input_param == NULL) {
		printf("malloc world_input_param failed\n");
		return NULL;
	}

/*
	for(int i = 0; i < num_frame; i++) {
		world_input_param[i] = (float*)malloc(num_column*sizeof(float));
		if(world_input_param[i] == NULL) {
			printf("malloc world_input_param[i] failed\n");
			return NULL;
		}
		for(int j = 0; j < num_column; j++) {
			// Note: column first storage !!!!!
			world_input_param[i][j] = *(params+j*num_frame+i);
		}
	}
*/

	for(int i = 0; i < num_frame; i++) {
		world_input_param[i] = (float*)malloc(num_column*sizeof(float));
		if(world_input_param[i] == NULL) {
			printf("malloc world_input_param[i] failed\n");
			return NULL;
		}
		for(int j = 0; j < num_column; j++) {
			world_input_param[i][j] = *(params+i*num_column+j);
		}
	}

	int y_length = (int)((num_frame-num_skip*2)*5.0/1000*24000);  // 24000
	double *y = (double *)malloc(sizeof(double)*y_length);
	if(y == NULL) {
		for(int i = 0; i < num_frame; i++) {
			if(world_input_param[i]) {
				free(world_input_param[i]);
			}
		}
		if(world_input_param) {
			free(world_input_param);
		}
		printf("malloc y failed\n");
		return NULL;
	}

	HTS_World_Synthesize(world_input_param,
						 world_alpha,
						 num_spectrum,
						 num_frame,
						 0+num_skip,
						 num_frame-num_skip,
						 num_skip,
						 offline,
						 y_length,
						 y);

	for(int i = 0; i < num_frame; i++) {
		if(world_input_param[i]) {
			free(world_input_param[i]);
		}
	}
	if(world_input_param) {
		free(world_input_param);
	}
	
	short *pcm = (short *)malloc(sizeof(short)*y_length);
	if(pcm == NULL) {
		printf("malloc pcm failed\n");
		return NULL;
	}
	for(int i = 0; i < y_length; i++) {
		pcm[i] = (short)(MyMaxInt(-32768, MyMinInt(32767, (int)(y[i]*32767))));
	}
	pcm_length = y_length;

	if(y) {
		free(y);
	}
	
	return pcm;
}

static PyObject* 
ExVocoder24K_synthesizeMemoMemoPCM(PyObject *self, PyObject *args)
{
	int offline;
	int num_spectrum;
	int num_skip;
	double world_alpha;
	PyObject *o;

	if(!PyArg_ParseTuple(args, "iiidO", &offline, &num_spectrum, &num_skip, &world_alpha, &o)) {
		std::cout<<"invalid parameters"<<std::endl;
		return Py_None;
	}

	PyObject *ao = PyObject_GetAttrString(o, "__array_struct__");
	if((ao == NULL) || !PyCObject_Check(ao)) {
		std::cout<<"object does not have array interface"<<std::endl;
		return Py_None;
	}

	PyArrayInterface *pai = (PyArrayInterface*)PyCObject_AsVoidPtr(ao);
	if(pai->two != 2) {
		std::cout<<"object does not have array interface"<<std::endl;
		Py_DECREF(ao);
		return Py_None;
	}

	if(pai->typekind == 'f' && pai->itemsize == 8) {
		//std::cout<<pai->typekind<<","<<pai->itemsize<<std::endl;
	} else {
		std::cout<<"array type should be float64"<<std::endl;
		Py_DECREF(ao);
		return Py_None;
	}

	if(pai->nd != 2) {
		std::cout<<"array can be 2D only"<<std::endl;
		Py_DECREF(ao);
		return Py_None;
	} else {
		int rows = (int)pai->shape[0];
		int cols = (int)pai->shape[1];
		int pcm_length = 0;

		short *pcm = synthesizeMemoMemoPCM(offline, num_spectrum, num_skip, world_alpha, (double*)pai->data, rows, cols, pcm_length);
		if(pcm == NULL) {
			std::cout<<"synthesize pcm buffer failed"<<std::endl;
			return Py_None;
		}
		
		PyObject *result = PyList_New(pcm_length);
		for(int i = 0; i < pcm_length; i++) {
			PyList_SetItem(result, i, PyInt_FromLong(pcm[i]));
		}

		if(pcm) {
			free(pcm);
		}

		Py_DECREF(ao);
		Py_INCREF(o);
		return (PyObject*)Py_BuildValue("iO", pcm_length, result);
	}
}

static PyMethodDef
ExVocoder24KMethods[] =
{
	{ "synthesize_memo_memo_pcm", ExVocoder24K_synthesizeMemoMemoPCM, METH_VARARGS },
	{ NULL, NULL },  
};

void initExVocoder24K()
{
	Py_InitModule("ExVocoder24K", ExVocoder24KMethods);
}




