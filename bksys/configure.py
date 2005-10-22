#! /usr/bin/env python

#
# call from package root for example 
# 
#    cd kdelibs 
#    python bksys/configure.py    
#

import os,sys,types

# replace through python installation dir 
#sys.path.append('c:\Python24' + os.sep + 'scons')

#from SCons.Script.SConscript import SConsEnvironment
#from SCons.Options import Options, PathOption

env = {}
env['CACHEDIR'] = 'cache'

#cachefile=env['CACHEDIR']+'test.cache.py'
#opts = Options(cachefile)


class PackageConfiguration:
	def __init__(self):
		self.cfunctions = ''
		self.cheaders = ''
		self.optional_cfunctions = ''
		self.optional_cheaders = ''
		self.options = {}
		self.uselibs = {}
		self.modules = {}

	def __getattr__(self, item):
		"""Maps values to attributes."""
		try:
			return self.__dict__[item]
		except KeyError:
			raise AttributeError(item)

	def __setattr__(self, item, value):
		"""Maps attributes to values."""
		if not self.__dict__.has_key(item):
			self.__dict__[item] = value
		elif type(value) is types.ListType:
			self.__dict__[item] += value
		elif type(value) is types.StringType:
			self.__dict__[item] += value + ' '
		else:
			print "unknown type"


conf = PackageConfiguration()
verbose = 1

# walk through available directory an see if there are any SConfigure files 
if verbose: 
	print 'SConfigure(s) found in '
for root, dirs, files in os.walk('.'):

	# skip svn or cvs directories 
	if '.svn' in root or 'CVS' in root:
		continue

	# got one
	if 'SConfigure' in files:
		if verbose:
			print root
		_file_ = open(root + os.sep + 'SConfigure', "r")
		# conf is given as global object 
		exec _file_

# print results 		
print conf.cheaders
print conf.cfunctions
print conf.optional_cheaders
print conf.optional_cfunctions
print conf.options
print conf.uselibs
print conf.modules


		
