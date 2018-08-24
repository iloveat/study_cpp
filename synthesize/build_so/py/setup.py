from distutils.core import setup, Extension
import numpy

module_name = 'ExVocoder8K'

source_file = [
	'../../vocoder/common.c', 
	'../../vocoder/matlabfuns.c', 
	'../../vocoder/fft.c', 
	'../../vocoder/sptk.c', 
	'../../vocoder/synthesis.c', 
	'../../vocoder/world.c', 
	'interface_8k.cpp'
	#'interface_24k.cpp'
]


modules = [Extension(module_name, source_file, extra_compile_args=['-std=c99'], include_dirs=[numpy.get_include()])]

setup(name=module_name, ext_modules=modules)


from shutil import copyfile
# copyfile("./build/lib.linux-x86_64-2.7/ExVocoder24K.so", "/home/pallas/merlin-cntk/refactor/PyExt/ExVocoder24K.so")






