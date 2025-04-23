#!/usr/bin/env python3

from setuptools import find_packages, setup, Extension

import sys

import os
import glob

# Find the library path
lib_paths = []
for search_path in ['../debian/tmp/usr/lib', '../debian/libpisoundmicro/usr/lib']:
	for lib_glob in glob.glob(f"{search_path}/**/libpisoundmicro.*", recursive=True):
		lib_dir = os.path.dirname(lib_glob)
		if lib_dir not in lib_paths:
			lib_paths.append(lib_dir)

if '--owner=root' in sys.argv:
	sys.argv.remove('--owner=root')
if '--group=root' in sys.argv:
	sys.argv.remove('--group=root')

pisoundmicro_module = Extension(
	'_pypisoundmicro',
	sources = [ 'pisoundmicro.i' ],
	swig_opts = [ '-c++', '-py3', '-includeall', '-I/usr/include', '-I../include', '-doxygen' ],
	include_dirs = [ '../include' ],
	library_dirs = lib_paths,
	libraries = [ 'pisoundmicro' ]
)

with open("VERSION") as f:
	VERSION = f.read().strip()

setup(
	name='pisoundmicro',
	version=VERSION,
	packages=find_packages(),
	description='Python bindings for libpisoundmicro',
	author='Giedrius Trainavičius',
	author_email = 'giedrius@blokas.io',
	url='https://blokas.io/',
	license='LGPLv3',
	ext_modules=[pisoundmicro_module],
	py_modules=['pypisoundmicro']
)
