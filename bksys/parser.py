#!/usr/bin/python

from xml.sax import make_parser 
from xml.sax.handler import ContentHandler 

import SCons.Util

def exists(env):
	return True

class SconsHandler(ContentHandler): 

	def __init__ (self, envi):
		self.envname = ""
		self.env = envi

		#self.dump = True
		self.dump = False
		self.count = 0
		self.dir = ""
		self.autoinstall = False
		self.appname=""

		self.obj = ""

		self.subdir =""
		self.type   =""

		self.isgloballib=""
		self.target=""
		
		self._includes=""
		self.cxxflags=""
		self.globallibs=""
		self.locallibs=""
		self.linkflags=""
    
		self.srclist=[]

    
	def dump_commands(self, str):
		if self.dump:
			print str
   
	# helper
	def _add_dir(self, var):
		#ladd_dir = lambda x: self.dir+"/"+x
		#return map( ladd_dir, var )
		lst = []
		for file in var:
			lst.append( str("./"+self.dir+"/"+file) )

		#print "-"
		#for file in lst:
		#	print file+" "
		return lst
			
    
	def startElement(self, name, attrs): 

		if name == 'icondirent':
			dir = attrs.get('dir', None)
			if dir:
				#if self.env.has_key("DUMPCONFIG"):
				#	print "env.KDEicon('"+dir+")'"
				self.env.KDEicon('*', dir)
		elif name == 'subdirent':
			dir = attrs.get('dir', None)
			if dir:
				#if self.env.has_key("DUMPCONFIG"):
				#	print "env.SConscript('"+dir+"/SConscript')"
				self.env.SConscript(dir+"/SConscript")
		elif name == 'docdir':
			self.appname = attrs.get('name', None)
		elif name == 'docdirent':
			dir = attrs.get('dir', None)
			lang = attrs.get('lang', None)
			if dir and lang:
				#if self.env.has_key("DUMPCONFIG"):
				#	print "env.docfolder('"+dir+"', '"+lang+"', '"+self.appname+"')"
				self.env.docfolder(dir, lang, self.appname)

		elif name == 'install':
			self.type   = attrs.get('type', None)
			self.subdir = attrs.get('subdir', None)

		elif name == 'file':
			name = attrs.get('name', None)
			if self.type and self.subdir:
				#if self.env.has_key("DUMPCONFIG"):
				#	print "env.KDEinstall('"+self.type+"', '"+self.subdir+"', '"+name+"')"
				self.env.KDEinstall(self.type, self.subdir, name)

		elif name == 'compile':
		
			type = attrs.get('type', None)
			if not type: self.env.Exit(1)
			self.obj = self.env.kobject(type)

			self.obj.target     = str(attrs.get('target', ''))
			self.obj.source     = []

			self.obj.includes   = str(attrs.get('includes', ''))
			self.obj.cflags     = str(attrs.get('cflags', ''))
			self.obj.cxxflags   = str(attrs.get('cxxflags', ''))

			self.obj.libs       = str(attrs.get('libs', ''))
			self.obj.linkflags  = str(attrs.get('linkflags', ''))
			self.obj.libpath    = str(attrs.get('libpath', ''))

			self.obj.vnum       = str(attrs.get('vnum', ''))
			self.obj.iskdelib   = str(attrs.get('iskdelib', 0))
			self.obj.libprefix  = str(attrs.get('libprefix', ''))

			self.obj.chdir      = str(attrs.get('chdir', ''))

		elif name == 'source':
			file = attrs.get('file', None)
			if file: self.obj.source.append( file )
			#self.obj.source="settings.kcfgc pmanager_part.cpp preader.cpp"

	def endElement(self, name): 
		if name == 'compile':
			self.obj.execute()

def generate(env):
	
	def xmlfile(env, file):
		parser = make_parser()
		curHandler = SconsHandler(env)
		parser.setContentHandler(curHandler)
		parser.parse(open(file))

	from SCons.Script.SConscript import SConsEnvironment
        SConsEnvironment.xmlfile = xmlfile

