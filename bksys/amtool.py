#! /usr/bin/env python

## 
# @file 
# bksys Makefile.am related tools
# 
# note: may be a future base of a Makefile.am to scons converter if someone like to write it 
#
# (@) 2005 Ralf Habacker  - published under the GPL license
#
 
import os, re, types, sys, string, shutil, stat, glob

## print a dictionary 
def printDict(dict,sep=' = ' ):
	for key in dict.keys():
		print key  + sep + dict[key]


## Makefile.am support class
#
# The class provides methods for the following tasks: 
#		- parsing Makefile.am's and collecting targets and defines into class members 
#		- extracting library dependencies and linker flags keyed by the related targets 
#		- collecting detailled lists of libraries dependencies
#
#
class AMFile: 
	def __init__(self):
		self.defines = {}
		self.targets = {}
		self.libadds = {}
		self.ldflags = {}
		
	## read and parse a Makefile.am 
	#
	# The resulting lines are stored in the defines and targets class member.
	# note: Multiple lines in a target are separated by '###'
	# @param path - path for Makefile.am
	# @return 0 if file couldn't be read 
	# 
	def read(self,path):
		try:
			src=open(path, 'r')
		except:
			return 0
		file  = src.read()
		lines = file.replace('\n\t','###')
		lines2 = lines.replace('\\\n',' ')
		list = lines2.split('\n')
		for line in list:
			if line[:1] == '#' or len(line) == 0:
				continue
			var = line.split('=')
			if len(var) == 2:
				self.defines[str(var[0]).strip()] = var[1].strip().replace("'",'').replace('\###',' ')
			else:
				target = line.split(':')
				if len(target) == 2:
					single_target = target[1].strip().replace("'",'')
					# TODO: (rh) split into list 
					self.targets[str(target[0]).strip()] = single_target
		self.getLibraryDeps()
		self.getLinkerFlags()
		return 1
		
	## adds library dependencies from another AMFile instance 
	#
	# This method is mainly used for an instance collecting definitions 
	# from instances from lower levels 
	# @param src - AMFile instance, from which the dependencies are imported 
	#
	def addLibraryDeps(self,src):
		for key in src.libadds.keys():
			self.libadds[key] = src.libadds[key]
				
	## adds linker flags from another AMFile instance 
	#
	# This method is mainly used for an instance collecting definitions 
	# from instances from lower levels 
	# @param src - AMFile instance, from which the flags are imported 
	#
	def addLinkerFlags(self,src):
		for key in src.ldflags.keys():
			self.ldflags[key] = src.ldflags[key]

	## collect all LIBADDS definitions 
	#
	# the function store the definitions in the libadds class member keyed 
	# by the relating target
	# @return definition list 
	#
	def getLibraryDeps(self):
		reg = re.compile("(.*)_(la|a)_LIBADD$")
		# TODO (rh) fix relative library pathes 
		for key in self.defines.keys():
			result=reg.match(key)
			if result:
				self.libadds[str(result.group(1))] = self.defines[key]
		return self.libadds

	## collect all LDFLAGS definitions 
	#
	# the function store the definitions in the ldflags class member keyed 
	# by the relating target 
	# @return definition list 
	#
	def getLinkerFlags(self):
		reg = re.compile("(.*)_(la|a)_LDFLAGS$")
		for key in self.defines.keys():
			result=reg.match(key)
			if result:
				self.ldflags[str(result.group(1))] = self.defines[key]
		return self.ldflags

	## return a reverse usage list of dependencies 
	#
	# The function scannes the recent library definitions and reorganice
	# the resulting list keyed by the used library
	# @return dependency list 
	#
	def getReverseLibraryDeps(self):
		alist = {}
		for key in self.libadds.keys():
			for lib in self.libadds[key].split():
				if lib in alist:
					alist[str(lib)] += " " + key
				else:
					alist[str(lib)] = key
		return alist
	
	def printDefines(self):
		print "### DEFINES:" 
		printDict(self.defines,' = ')
	
	def printTargets(self):
		print "### TARGETS:" 
		printDict(self.targets,' : ')

	def printLibraryDeps(self):
		print "### LIBADD:" 
		printDict(self.libadds,' : ')

	def printLinkerFlags(self):
		print "### LDFLAGS:"
		printDict(self.ldflags,' : ')

uses = 0
libadds = 0
ldflags = 0
defines = 0
targets = 0
if len(sys.argv) == 1:
	print "amtool [options] Makefile.am [Makefile.am] ..."
	print "list Makefile.am content" 
	print "options:" 
	print "    --uses print where a library is used" 
	print "    --libadd print all LIBADD depenencies " 
	print "    --ldflags print all LDFLAGS definitions" 
	print "    --defines print all Makefile variables" 
	print "    --targets print all Makefile tarets" 
else:
	all_ams = AMFile()
	for a in range(1,len(sys.argv)):
		if sys.argv[a][:6] == '--uses':
			uses = 1
		elif sys.argv[a][:8] == '--libadd':
			libadds = 1
		elif sys.argv[a][:9] == '--defines':
			defines = 1
		elif sys.argv[a][:9] == '--targets':
			targets = 1			
		elif sys.argv[a][:9] == '--ldflags':
			ldflags = 1

		if  libadds or defines or targets or ldflags:	
			uses = 2
						
	for a in range(1,len(sys.argv)):
		if sys.argv[a][:2] == '--':
			continue
		am_file = AMFile()

		if not am_file.read(sys.argv[a]): 
			continue

		if uses == 2:
			print "### " + sys.argv[a]

		if defines:
			am_file.printDefines()
		if targets:
			am_file.printTargets()
		if libadds:
			am_file.printLibraryDeps()
		if ldflags:
			am_file.printLinkerFlags()

		all_ams.addLibraryDeps(am_file)

	if uses == 0:
		all_ams.printUsedLibraries()
	elif uses == 1:
		a = all_ams.getReverseLibraryDeps()
		printDict(a)
