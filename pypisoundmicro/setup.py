#!/usr/bin/env python3

from setuptools import find_packages, setup, Extension
from setuptools.command.build_py import build_py

import sys
import re
import os
import glob
import importlib.util
import shutil
import ast

# Find the library path
lib_paths = []
for search_path in ['../debian/tmp/usr/lib', '../debian/libpisoundmicro/usr/lib', '../']:
	for lib_glob in glob.glob(f"{search_path}/**/libpisoundmicro.*", recursive=True):
		lib_dir = os.path.dirname(lib_glob)
		if lib_dir not in lib_paths:
			lib_paths.append(lib_dir)

if '--owner=root' in sys.argv:
	sys.argv.remove('--owner=root')
if '--group=root' in sys.argv:
	sys.argv.remove('--group=root')

class DocstringInjector(ast.NodeTransformer):
	def __init__(self, psm_module):
		self.psm = psm_module
		self.modified = False
		self.processed_count = 0

	def visit_FunctionDef(self, node):
		# Process decorators
		copy_doc_target = None
		new_decorators = []

		for decorator in node.decorator_list:
			if (isinstance(decorator, ast.Call) and 
				isinstance(decorator.func, ast.Name) and 
				decorator.func.id == 'copy_doc'):
				# Extract the path from copy_doc(psm.X.Y.Z)
				if len(decorator.args) == 1:
					# Extract the full path
					attr_path = self._extract_attribute_path(decorator.args[0])
					if attr_path.startswith('psm.'):
						copy_doc_target = attr_path[4:]  # Remove 'psm.' prefix
						self.modified = True
						continue  # Skip this decorator

			new_decorators.append(decorator)

		# Update decorators list (removing @copy_doc)
		node.decorator_list = new_decorators

		# If we found a copy_doc decorator, apply the docstring
		if copy_doc_target:
			try:
				# Navigate to source object and get its docstring
				obj = self.psm
				for part in copy_doc_target.split('.'):
					obj = getattr(obj, part)

				docstring = obj.__doc__ or ''

				# Set the docstring on the function
				node.body = [ast.Expr(value=ast.Constant(value=docstring))] + node.body
				self.processed_count += 1
				print(f"Copied docstring from {copy_doc_target} to {node.name}")
			except Exception as e:
				print(f"Error copying docstring for {node.name}: {e}")

		return self.generic_visit(node)

	def _extract_attribute_path(self, node):
		"""Extract the full attribute path (e.g., psm.Audio.init)"""
		if isinstance(node, ast.Name):
			return node.id
		elif isinstance(node, ast.Attribute):
			return f"{self._extract_attribute_path(node.value)}.{node.attr}"
		return ""

def preprocess_api_facade(self):
	print("Preprocessing _api_facade.py using AST to inject docstrings...")
	try:
		# Find the built SWIG module
		build_dirs = glob.glob(os.path.join(self.build_lib, 'pypisoundmicro', 'swig', '_pypisoundmicro*.so'))
		if not build_dirs:
			print("Warning: Could not find built SWIG module, skipping docstring injection")
			return

		# Import the SWIG module to access docstrings
		sys.path.insert(0, self.build_lib)
		from pypisoundmicro.swig import pypisoundmicro as psm

		# Source and destination files
		src_file = os.path.join('pypisoundmicro', '_api_facade.py')
		dest_file = os.path.join(self.build_lib, 'pypisoundmicro', '_api_facade.py')
		
		# Read the source file
		with open(src_file, 'r') as f:
			source = f.read()
		
		# Parse the source into an AST
		tree = ast.parse(source)
		
		# Apply our docstring injector
		transformer = DocstringInjector(psm)
		modified_tree = transformer.visit(tree)
		ast.fix_missing_locations(modified_tree)
		
		# Convert modified AST back to source
		modified_source = ast.unparse(modified_tree)

		# If no copy_doc functionality is needed anymore, remove the decorator definition
		if transformer.modified:
			# This pattern removes the copy_doc decorator function definition
			import re
			modified_source = re.sub(
				r"def copy_doc\(from_func\):\n\s+def decorator\(to_func\):.+?return decorator\n\n",
				"",
				modified_source,
				flags=re.DOTALL
			)

		# Write the modified source to destination
		with open(dest_file, 'w') as f:
			f.write(modified_source)

		print(f"Successfully injected {transformer.processed_count} docstrings into {dest_file}")

	except Exception as e:
		print(f"Error preprocessing _api_facade.py with AST: {e}")
		# Ensure the original file is still copied if processing fails
		os.makedirs(os.path.dirname(dest_file), exist_ok=True)
		shutil.copy2(src_file, dest_file)

# Custom build_py command to preprocess _api_facade.py
class CustomBuildPy(build_py):
	def run(self):
		super().run()
		preprocess_api_facade(self)

pisoundmicro_module = Extension(
	'pypisoundmicro.swig._pypisoundmicro',
	sources = [ 'pisoundmicro.i' ],
	swig_opts = [ '-c++', '-py3', '-includeall', '-I/usr/include', '-I../include', '-doxygen', '-outdir', 'pypisoundmicro/swig' ],
	include_dirs = [ '../include' ],
	library_dirs = lib_paths,
	libraries = [ 'pisoundmicro' ]
)

VERSION = '1.0.0-dev'
try:
	with open("VERSION") as f:
		VERSION = f.read().strip()
except:
	pass

setup(
	name='pypisoundmicro',
	version=VERSION,
	packages=find_packages(),
	description='Python bindings for libpisoundmicro',
	author='Giedrius Trainavičius',
	author_email = 'giedrius@blokas.io',
	url='https://blokas.io/',
	license='LGPLv3',
	ext_modules=[pisoundmicro_module],
	py_modules=['pypisoundmicro'],
	cmdclass={
		'build_py': CustomBuildPy,
	},
)
