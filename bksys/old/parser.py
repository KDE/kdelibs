#!/usr/bin/python

from xml.sax import make_parser 
from xml.sax.handler import ContentHandler 

import SCons.Util

def exists(env):
	return True

class SconsHandler(ContentHandler): 

	def __init__ (self, envi, builddir):
		self.envname = ""
		self.env = envi
		self.builddir="" #envi['_BUILDDIR_']

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

	def adrl(self, file):
		if self.builddir:
			dir=self.env.join(self.builddir,file).lstrip('/')
		else:
			dir=file.lstrip('/')
		return dir
    
	def dump_commands(self, str):
		if self.dump:
			print str
   
	def startElement(self, name, attrs): 

		if name == 'icondirent':
			dir = attrs.get('dir', '')
			sbdir = attrs.get('subdir', '')
			if dir:
				#if self.env.has_key("DUMPCONFIG"):
				#	print "env.KDEicon('"+dir+")'"
				self.env.KDEicon('*', self.adrl(dir), subdir=sbdir)
		elif name == 'subdirent':
			dir = attrs.get('dir', None)
			if dir:
				#if self.env.has_key("DUMPCONFIG"):
				#	print "env.SConscript('"+dir+"/SConscript')"
				self.env.SConscript(self.env.join(self.adrl(dir),"SConscript"))
		elif name == 'docdir':
			self.appname = self.adrl( attrs.get('name', None) )
		elif name == 'docdirent':
			dir = attrs.get('dir', None)
			lang = attrs.get('lang', None)
			if dir and lang:
				#if self.env.has_key("DUMPCONFIG"):
				#	print "env.docfolder('"+dir+"', '"+lang+"', '"+self.appname+"')"
				self.env.docfolder(self.adrl(dir), lang, self.appname)
		elif name == 'podir':
			dir = attrs.get('dir', None)
			appname = attrs.get('name', None)
			if dir and appname:
				if self.env.has_key('_BUILDDIR_'): dir=self.env.join(self.env['_BUILDDIR_'], dir)
				self.env.KDElang(dir, appname)
		elif name == 'install':
			self.type   = attrs.get('type', None)
			self.subdir = attrs.get('subdir', None)

		elif name == 'file':
			name = attrs.get('name', None)
			if self.type:
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

			self.obj.chdir      = self.adrl(str(attrs.get('chdir', '')))
			self.obj.dirprefix  = self.adrl(str(attrs.get('dirprefix', './')))
			if not self.obj.dirprefix: self.obj.dirprefix='./' # avoid silly errors

		elif name == 'source':
			file = attrs.get('file', None)
			condition = attrs.get('condition', "");
			lst=condition.split(':')
			for c in lst:
				if self.env.has_key(c):
					self.obj.source.append( file )
					break
			if file and not condition: self.obj.source.append( file )

	def endElement(self, name): 
		if name == 'compile':
			self.obj.execute()

def generate(env):
	
	def xmlfile(env, file, builddir=''):
		parser = make_parser()
		curHandler = SconsHandler(env, builddir)
		parser.setContentHandler(curHandler)
		parser.parse(open(file))

	from SCons.Script.SConscript import SConsEnvironment
        SConsEnvironment.xmlfile = xmlfile

