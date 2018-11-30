import setuptools
import sys
from distutils.core import Extension

if sys.version_info < (3,0):
    sys.exit('Sorry, Python < 3.0 is not supported')

with open("README.md", "r") as fh:
    long_description = fh.read()

setuptools.setup(
	name="ledpixelmath",
	version="1.0.3",
	description = 'Does heavy LED pixel calculations.',
	long_description=long_description,
	long_description_content_type="text/markdown",
	author="Tim Greve",
	author_email="tgreve@stakotec.de",
	url="https://github.com/tgreve85/ledpixelmath",
	keywords = ['led', 'pixel', 'math', 'calculation'],
	ext_modules=[
		Extension("ledpixelmath", ["ledpixelmath.cpp"],
		extra_compile_args=['-std=c++17'])
	],
	include_package_data=True,
	classifiers=(
		"Programming Language :: C++",
		"License :: OSI Approved :: GNU Lesser General Public License v3 (LGPLv3)",
		"Operating System :: POSIX"
	)
)
