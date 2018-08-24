#include <iostream>
#include <assert.h>
#include "../../vocoder/common.h"
#include "../../vocoder/world.h"
#include "interface_16k.h"
#include "Python.h"
#include "numpy/ndarrayobject.h"

using namespace std;


// example: add
static PyObject*
ExVocoder16K_add(PyObject *self, PyObject *args)  
{
	int a;
	int b;
	if(!PyArg_ParseTuple(args, "ii", &a, &b)) {
		std::cout<<"invalid parameters"<<std::endl;
		return Py_None;
	}
	return (PyObject*)Py_BuildValue("i", a+b);
}

/// example: ndarray input and list output
static PyObject*
ExVocoder16K_array(PyObject *self, PyObject *args)
{
	PyObject *o;
	if(!PyArg_ParseTuple(args, "O", &o)) {
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
		double *p = new double[rows * cols];
		if(p == NULL) {
			std::cout<<"new p failed"<<std::endl;
			Py_DECREF(ao);
			return Py_None;
		}
		for(int i = 0; i < rows * cols; i++) {
			p[i] = -*(double*)(pai->data+8*i);
		}

		PyObject *result = PyList_New(rows * cols);
		for(int i = 0; i < rows * cols; i++) {
			PyList_SetItem(result, i, PyFloat_FromDouble(p[i]));
		}
		delete []p;

		Py_DECREF(ao);
		Py_INCREF(o);
		return (PyObject*)Py_BuildValue("O", result);
	}
}

void writeWaveHead(FILE *fp, const int &nWaveLength)
{
	char *pHead = (char*)malloc(sizeof(char)*44);

	strcpy((char*)pHead, "RIFF");
	*(int *)(pHead+4) = nWaveLength*2+32;
	strcpy((char*)(pHead+8),"WAVEfmt ");

	*(int *)(pHead+16) 		= 16;
	*(short *)(pHead+20) 	= 1;					/*WAVE_FORMAT_PCM*/
	*(short *)(pHead+22) 	= 1;					/*single channel wave*/
	*(int *)(pHead+24) 		= 16000;				/*16k sample rate*/
	*(int *)(pHead+28) 		= 16000*2; 				/*average bytes per second*/
	*(short *)(pHead+32) 	= 16/8;
	*(short *)(pHead+34) 	= 16;					/*16 bits per sample*/

	strcpy((char*)(pHead+36), "data");
	*(int *)(pHead+40) = nWaveLength*2-44;

	fwrite(pHead, sizeof(char)*44, 1, fp);

	free(pHead);
}

int synthesizeFileFileWAV(const int &offline,			//offline:1, online:0
						  const int &num_spectrum,		//dimension of mgc, offline:19, online:59
						  const int &num_skip,			//skipped points
						  const double &world_alpha,	//parameter alpha of world
						  const char *pinput,			//(input) parameter file name
						  const char *poutput)			//(output) pcm file name
{
	assert(offline == 1 || offline == 0);
	int num_column = num_spectrum+1+1;  //mgc+lf0+bap

	FILE *fp_all = fopen(pinput, "rb");
	fseek(fp_all, 0, SEEK_END);
	unsigned int len_all = ftell(fp_all);
	assert(len_all % (sizeof(float)*num_column) == 0);
	rewind(fp_all);

	int num_frame = len_all/(sizeof(float)*num_column);  // calculate frame number

	float **world_input_param = (float**)malloc(num_frame*sizeof(float*));
	for(int i = 0; i < num_frame; i++)
	{
		world_input_param[i] = (float*)malloc(num_column*sizeof(float));
		fread(world_input_param[i], sizeof(float), num_spectrum, fp_all);  //mgc
		fread(world_input_param[i]+num_spectrum, sizeof(float), 1, fp_all);  //lf0
		fread(world_input_param[i]+num_spectrum+1, sizeof(float), 1, fp_all);  //bap
	}
	fclose(fp_all);

	int y_length = (int)((num_frame-num_skip*2)*5.0/1000*16000);
	double *y = (double *)malloc(sizeof(double)*y_length);

	printf("params in file_file: %f,%d,%d,%d,%d,%d,%d,%d\n", world_alpha, num_spectrum, num_frame, num_skip, num_frame-num_skip, num_skip, offline, y_length);
	for(int i = 0; i < 3; i++)
	{
		for(int j = 0; j < num_column; j++)
		{
			printf("%f ",world_input_param[i][j]);
		}
		printf("\n");
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

	for(int i = 0; i < num_frame; i++)
	{
		free(world_input_param[i]);
	}
	free(world_input_param);

	//write pcm file
	FILE *fout = fopen(poutput, "wb+");
	writeWaveHead(fout, y_length);
	for(int i = 0; i < y_length; i++)
	{
		short tmp_signal = (short)(MyMaxInt(-32768, MyMinInt(32767, (int)(y[i]*32767))));
		fwrite(&tmp_signal, sizeof(short), 1, fout);
	}
	fclose(fout);
	free(y);
	return 1;
}

int synthesizeMemoFileWAV(const int &offline,			//offline:1, online:0
						  const int &num_spectrum,		//dimension of mgc, offline:19, online:59
						  const int &num_skip,			//skipped points
						  const double &world_alpha,	//parameter alpha of world
						  const char *poutput,
						  double *params, const int &rows, const int &cols)
{
	assert(offline == 1 || offline == 0);
	int num_column = num_spectrum+1+1;  //mgc+lf0+bap
	assert(num_column == cols);

	unsigned int len_all = rows * cols * 8;
	assert(len_all % (sizeof(double)*num_column) == 0);

	int num_frame = len_all/(sizeof(double)*num_column);  // calculate frame number
	assert(num_frame == rows);

	float **world_input_param = (float**)malloc(num_frame*sizeof(float*));
	for(int i = 0; i < num_frame; i++)
	{
		world_input_param[i] = (float*)malloc(num_column*sizeof(float));
		for(int j = 0; j < num_column; j++)
		{
			// Note: column first storage !!!!!
			world_input_param[i][j] = *(params+j*num_frame+i);
		}
	}

	int y_length = (int)((num_frame-num_skip*2)*5.0/1000*16000);
	double *y = (double *)malloc(sizeof(double)*y_length);

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

	for(int i = 0; i < num_frame; i++)
	{
		free(world_input_param[i]);
	}
	free(world_input_param);

	//write pcm file
	FILE *fout = fopen(poutput, "wb+");
	writeWaveHead(fout, y_length);
	for(int i = 0; i < y_length; i++)
	{
		short tmp_signal = (short)(MyMaxInt(-32768, MyMinInt(32767, (int)(y[i]*32767))));
		fwrite(&tmp_signal, sizeof(short), 1, fout);
	}
	fclose(fout);
	free(y);
	return 1;
}

short* synthesizeMemoMemoPCM(const int &offline,		//offline:1, online:0
							 const int &num_spectrum,	//dimension of mgc, offline:19, online:59
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

	int num_column = num_spectrum+1+1;  //mgc+lf0+bap
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

	int y_length = (int)((num_frame-num_skip*2)*5.0/1000*16000);
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
ExVocoder16K_synthesizeFileFileWAV(PyObject *self, PyObject *args)  
{
	int offline;
	int num_spectrum;
	int num_skip;
	double world_alpha;
	char *pinput;
	char *poutput;

	if(!PyArg_ParseTuple(args, "iiidss", &offline, &num_spectrum, &num_skip, &world_alpha, &pinput, &poutput)) {
		std::cout<<"invalid parameters"<<std::endl;
		return (PyObject*)Py_BuildValue("i", -1);
	}

	int ret = synthesizeFileFileWAV(offline, num_spectrum, num_skip, world_alpha, pinput, poutput);
	if(ret < 0) {
		std::cout<<"synthesize wav file failed"<<std::endl;
		return (PyObject*)Py_BuildValue("i", -1);
	}

	return (PyObject*)Py_BuildValue("i", 1);
}

static PyObject* 
ExVocoder16K_synthesizeMemoFileWAV(PyObject *self, PyObject *args)
{
	int offline;
	int num_spectrum;
	int num_skip;
	double world_alpha;
	char *poutput;
	PyObject *o;

	if(!PyArg_ParseTuple(args, "iiidsO", &offline, &num_spectrum, &num_skip, &world_alpha, &poutput, &o)) {
		std::cout<<"invalid parameters"<<std::endl;
		return (PyObject*)Py_BuildValue("i", -1);
	}

	PyObject *ao = PyObject_GetAttrString(o, "__array_struct__");
	if((ao == NULL) || !PyCObject_Check(ao)) {
		std::cout<<"object does not have array interface"<<std::endl;
		return (PyObject*)Py_BuildValue("i", -1);
	}

	PyArrayInterface *pai = (PyArrayInterface*)PyCObject_AsVoidPtr(ao);
	if(pai->two != 2) {
		std::cout<<"object does not have array interface"<<std::endl;
		Py_DECREF(ao);
		return (PyObject*)Py_BuildValue("i", -1);
	}

	if(pai->typekind == 'f' && pai->itemsize == 8) {
		//std::cout<<pai->typekind<<","<<pai->itemsize<<std::endl;
	} else {
		std::cout<<"array type should be float64"<<std::endl;
		Py_DECREF(ao);
		return (PyObject*)Py_BuildValue("i", -1);
	}

	if(pai->nd != 2) {
		std::cout<<"array can be 2D only"<<std::endl;
		Py_DECREF(ao);
		return (PyObject*)Py_BuildValue("i", -1);
	} else {
		int rows = (int)pai->shape[0];
		int cols = (int)pai->shape[1];
		int ret = synthesizeMemoFileWAV(offline, num_spectrum, num_skip, world_alpha, poutput, (double*)pai->data, rows, cols);
		if(ret < 0) {
			std::cout<<"synthesize wav file failed"<<std::endl;
			Py_DECREF(ao);
			return (PyObject*)Py_BuildValue("i", -1);
		}

		Py_DECREF(ao);
		Py_INCREF(o);
		return (PyObject*)Py_BuildValue("i", 1);
	}
}

static PyObject* 
ExVocoder16K_synthesizeMemoMemoPCM(PyObject *self, PyObject *args)
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
ExVocoder16KMethods[] =
{
	{ "add", ExVocoder16K_add, METH_VARARGS },
	{ "array", ExVocoder16K_array, METH_VARARGS },
	{ "synthesize_file_file_wav", ExVocoder16K_synthesizeFileFileWAV, METH_VARARGS },
	{ "synthesize_memo_file_wav", ExVocoder16K_synthesizeMemoFileWAV, METH_VARARGS },
	{ "synthesize_memo_memo_pcm", ExVocoder16K_synthesizeMemoMemoPCM, METH_VARARGS },
	{ NULL, NULL },  
};

void initExVocoder16K()
{
	Py_InitModule("ExVocoder16K", ExVocoder16KMethods);
}




