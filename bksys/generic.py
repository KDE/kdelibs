## Thomas Nagy, 2005
""" Run scons -h to display the associated help, or look below """

import os, re, types, sys, string, shutil, stat, glob
import SCons.Defaults
import SCons.Tool
import SCons.Util
from SCons.Script.SConscript import SConsEnvironment
from SCons.Options import Options, PathOption

## CONFIGURATION: write the config.h files
def write_config_h(lenv):
	# put the files into the builddir
	if not os.path.exists( lenv['_BUILDDIR_'] ): os.mkdir(lenv['_BUILDDIR_'])
	dest=open(lenv.join(lenv['_BUILDDIR_'], 'config.h'), 'w')
	dest.write('/* defines are added below */\n')

	# write the config.h including all others
	if lenv.has_key('_CONFIG_H_'):
		for file in lenv['_CONFIG_H_']: dest.write('#include "config-%s.h"\n' % file)
	dest.write('\n')
	dest.close()

	lenv.pprint('GREEN','configuration done - run scons to compile now')
	lenv.Exit(0)

## CONFIGURATION: configure the project - this is the entry point
def configure(dict):
	from SCons import Environment
	import os

	cp_method  = 'soft-copy'
	tool_path  = ['bksys']
	build_dir  = '.'
	cache_dir  = 'cache'+os.sep
	config_h   = 0
	bootstrap  = 0
	want_rpath = 1
	use_colors = 0

	## process the options
	for key in dict.keys():
		if   key == 'modules':   mytools    = dict[key].split()
		elif key == 'builddir':  build_dir  = dict[key]
		elif key == 'config.h':  config_h   = 1
		elif key == 'cp_method': cp_method  = dict[key]
		elif key == 'rpath':     want_rpath = dict[key]
		elif key == 'bootstrap': bootstrap  = 1
		elif key == 'cachedir':  cache_dir  = dict[key]
		elif key == 'colorfull': use_colors = 1
		else: print 'unknown key: '+key

	## now build the environment
	if bootstrap:
		env = Environment.Environment( ENV=os.environ, BOOTSTRAP_KDE=1,
			_BUILDDIR_=build_dir, CACHEDIR=cache_dir, _USECOLORS_=use_colors,
			tools=mytools, toolpath=tool_path )
	else:
		env = Environment.Environment( ENV=os.environ, _BUILDDIR_=build_dir,
			_USECOLORS_=use_colors, CACHEDIR=cache_dir,
			tools=mytools, toolpath=tool_path )

	## at this point the help was displayed if asked to, then quit
	if env['HELP']: env.Exit(0)

	## disable the rpath in scripts if asked to
	env['_WANT_RPATH_']=want_rpath

	env.Append(CPPPATH=['#']) # ugly hack, remove when possible (ita)
	env.AppendUnique(LIBPATH=['#', build_dir]) # ugly hack, remove when possible (ita)

	## we want symlinks by default
	env.SetOption('duplicate', cp_method)

	## now the post-configuration
	if env['_CONFIGURE_']:
		# write the config.h if we were asked to, then quit
		if config_h: write_config_h(env)
		env.Exit(0)

	return env

### To make a tarball of the project use 'scons dist'
def dist(env, appname, version=None):
	import os
	if 'dist' in sys.argv:
                if sys.platform == 'darwin':
                        #sys.path.append('bksys'+os.sep+'osx')
                        #from detect_lowlevel import detect
                        env.pprint('RED', 'Not implemented, see bksys/osx/detect_generic.py')
                        env.Exit(1)
                else:
                        sys.path.append('bksys'+os.sep+'unix')
                        from detect_generic import dist
                dist(env)

	if 'distclean' in sys.argv:
                if sys.platform == 'darwin':
                        #sys.path.append('bksys'+os.sep+'osx')
                        #from detect_lowlevel import detect
                        env.pprint('RED', 'Not implemented, see bksys/osx/detect_generic.py')
                        env.Exit(1)
                else:
                        sys.path.append('bksys'+os.sep+'unix')
                        from detect_generic import distclean
                distclean(env)

colors= {
	'BOLD'  :"\033[1m",
	'RED'   :"\033[91m",
	'GREEN' :"\033[92m",
	'YELLOW':"\033[93m", # unreadable on white backgrounds - fix konsole ?
	'BLUE'  :"\033[94m",
	'CYAN'  :"\033[96m",
	'NORMAL':"\033[0m", }

def pprint(env, col, str, label=''):
	if not env['_USECOLORS_']:
		print "%s %s" % (str, label)
		return
	try: mycol=colors[col]
	except: mycol=''
	print "%s%s%s %s" % (mycol, str, colors['NORMAL'], label)

class genobj:
	def __init__(self, val, env):
		if not val in ["program", "shlib", "kioslave", "staticlib", "module", "convenience"]:
			env.pprint('RED', 'unknown object type given to genobj: '+val)
			env.Exit(1)

		self.type  = val
		self.orenv = env
		self.env   = None
		self.executed = 0

		self.source=''
		self.target=''

		self.cxxflags  =''
		self.ccflags   =''
		self.includes  =''

		self.linkflags =''
		self.libpaths  =''
		self.libs      =''

		# warning: uber-cool feature
		# self.uselib='KIO XML' and all linkflags, etc are added
		self.uselib=''

		# vars used by shlibs
		self.vnum=''
		self.libprefix=''

		# a directory where to install the targets (optional)
		self.instdir=''

		# change the working directory before reading the targets (do not use)
		self.chdir=''

		# unix permissions
		self.perms=''

		# these members are private
		self.chdir_lock=None
		self.dirprefix='./'
		self.old_os_dir=''
		self.old_fs_dir=''
		self.p_local_shlibs=[]
		self.p_local_staticlibs=[]
		self.p_global_shlibs=[]

		self.p_localsource=None
		self.p_localtarget=None

		# work directory
		self.workdir_lock=None
		self.orig_fs_dir=SCons.Node.FS.default_fs.getcwd()
		self.not_orig_fs_dir=''
		self.not_orig_os_dir=''

		# with this it is not mandatory to call execute() on objects created ...
		if not env.has_key('USE_THE_FORCE_LUKE'): env['USE_THE_FORCE_LUKE']=[self]
		else: env['USE_THE_FORCE_LUKE'].append(self)

	# called by subclasses to set the sources to compile
	def setsource(self, src): self.p_localsource=self.orenv.make_list(src)

	# join the builddir to a path
	def joinpath(self, val):
		if len(self.dirprefix)<3: return val # TODO - there can be a bug here ..
		dir=self.dirprefix
		thing=self.orenv.make_list(val)
		files=[]
		bdir='./'
		if self.orenv.has_key('_BUILDDIR_'): bdir=self.orenv['_BUILDDIR_']
		for v in thing: files.append( self.orenv.join(bdir, dir, v) )
		return files

	## FIXME : this scheme is ugly (ita)
	# a list of paths, with absolute and relative ones, useful when giving include dirs
	def fixpath(self, val):
		def reldir(dir):
			ndir    = SCons.Node.FS.default_fs.Dir(dir).srcnode().abspath
			rootdir = SCons.Node.FS.default_fs.Dir('#').abspath
			return ndir.replace(rootdir, '').lstrip('/')

		dir=self.dirprefix
		if not len(dir)>2: dir=reldir('.')

		thing=self.orenv.make_list(val)
		ret=[]
		bdir="./"
		if self.orenv.has_key('_BUILDDIR_'): bdir=self.orenv['_BUILDDIR_']
		for v in thing:
			if v[:1] == "#" or v[:1] == "/":
				ret.append(v)
			else:
				ret.append( self.orenv.join('#', bdir, dir, v) )
				ret.append( self.orenv.join('#', dir, v) )

		return ret

	def lockworkdir(self):
		if self.workdir_lock: return
		self.workdir_lock=1
		self.not_orig_fs_dir=SCons.Node.FS.default_fs.getcwd()
		self.not_orig_os_dir=os.getcwd()
		SCons.Node.FS.default_fs.chdir( self.orig_fs_dir, change_os_dir=1)

	def unlockworkdir(self):
		if not self.workdir_lock: return
		SCons.Node.FS.default_fs.chdir( self.not_orig_fs_dir, change_os_dir=0)
		os.chdir(self.not_orig_os_dir)
		self.workdir_lock=None

	# When an object is created and the sources, targets, etc are given
	# the execute command calls the SCons functions like Program, Library, etc
	def execute(self):
		if self.executed: return

		# dump the configuration to xml when asked to
		if self.orenv.has_key('DUMPCONFIG'):
			self.xml()
			self.executed=1
			return

		# copy the environment if a subclass has not already done it
		if not self.env: self.env = self.orenv.Copy()

		if not self.p_localtarget: self.p_localtarget = self.joinpath(self.target)
		if not self.p_localsource: self.p_localsource = self.joinpath(self.env.make_list(self.source))

		if (not self.source or len(self.source) == 0) and not self.p_localsource:
			self.env.pprint('RED',"no source file given to object - self.source for "+self.target)
			self.env.Exit(1)

		if not self.target:
			self.env.pprint('RED',"no target given to object - self.target")
			self.env.Exit(1)
		if not self.env.has_key('nosmart_includes'):
			self.env.AppendUnique(CPPPATH=['.', self.env.join('#',self.env['_BUILDDIR_']) ])
		if self.type == "kioslave": self.libprefix=''

		if len(self.includes)>0: self.env.AppendUnique(CPPPATH=self.fixpath(self.includes))
		if len(self.cxxflags)>0: self.env.AppendUnique(CXXFLAGS=self.env.make_list(self.cxxflags))
		if len(self.ccflags)>0: self.env.AppendUnique(CCFLAGS=self.env.make_list(self.ccflags))

		if self.type=='convenience':
			self.env.AppendUnique(CCFLAGS=self.env['CONVENIENCE'],CXXFLAGS=self.env['CONVENIENCE'])

		# add the libraries given directly
		llist=self.env.make_list(self.libs)
		lext=['.so', '.la']
		sext=['.a']
		for lib in llist:
			sal=SCons.Util.splitext(lib)
			if len(sal)>1:
				if sal[1] in lext: self.p_local_shlibs.append(self.fixpath(sal[0]+'.so')[0])
				elif sal[1] in sext: self.p_local_staticlibs.append(self.fixpath(sal[0]+'.a')[0])
				else: self.p_global_shlibs.append(lib)

		# and now add the libraries from uselib
		if self.uselib:
			#libs=self.env.make_list(self.uselib)
			libs=SCons.Util.CLVar(self.uselib) # self.env.Split(self.uselib)
			for lib in libs:
				if self.env.has_key('LIB_'+lib):
					self.env.AppendUnique(LIBS=self.env['LIB_'+lib])
				if self.env.has_key('LIBPATH_'+lib):
					self.env.AppendUnique(LIBPATH=self.env['LIBPATH_'+lib])
				if self.env.has_key('LINKFLAGS_'+lib):
					self.env.AppendUnique(LINKFLAGS=self.env['LINKFLAGS_'+lib])
				if self.env.has_key('INCLUDES_'+lib):
					self.env.AppendUnique(CPPPATH=self.env['INCLUDES_'+lib])
				if self.env.has_key('CXXFLAGS_'+lib):
					self.env.AppendUnique(CXXFLAGS=self.env['CXXFLAGS_'+lib])
				if self.env.has_key('CCFLAGS_'+lib):
					self.env.AppendUnique(CCFLAGS=self.env['CCFLAGS_'+lib])
				if self.env.has_key('CPPPATH_'+lib):
					self.env.AppendUnique(CPPPATH=self.env['CPPPATH_'+lib])
				if self.env.has_key('RPATH_'+lib) and self.env['_WANT_RPATH_']:
					self.env.AppendUnique(RPATH=self.env['RPATH_'+lib])

		# Settings for static and shared libraries
		if len(self.p_global_shlibs)>0:    self.env.AppendUnique(LIBS=self.p_global_shlibs)
		if len(self.libpaths)>0:           self.env.PrependUnique(LIBPATH=self.fixpath(self.libpaths))
		if len(self.linkflags)>0:          self.env.PrependUnique(LINKFLAGS=self.env.make_list(self.linkflags))
		if len(self.p_local_shlibs)>0:     self.env.link_local_shlib(self.p_local_shlibs)
		if len(self.p_local_staticlibs)>0: self.env.link_local_staticlib(self.p_local_staticlibs)

		# The target to return - IMPORTANT no more self.env modification is possible after this part
		ret=None
		if self.type=='shlib' or self.type=='kioslave' or self.type=='module':
			ret=self.env.bksys_shlib(self.p_localtarget, self.p_localsource, self.instdir, 
				self.libprefix, self.vnum)
		elif self.type=='program':
			ret=self.env.Program(self.p_localtarget, self.p_localsource)
			if not self.env.has_key('NOAUTOINSTALL'):
				ins=self.env.bksys_install(self.instdir, ret)
				if self.perms: self.env.AddPostAction(ins, self.env.Chmod(ins, self.perms))
		elif self.type=='staticlib' or self.type=='convenience':
			ret=self.env.StaticLibrary(self.p_localtarget, self.p_localsource)

		# If we link the program against a shared library made locally, add the dependency
		if len(self.p_local_shlibs)>0:
			if ret: self.env.Depends( ret, self.p_local_shlibs )
		if len(self.p_local_staticlibs)>0:
			if ret: self.env.Depends( ret, self.p_local_staticlibs )

		self.executed=1

## HELPER Expands strings given and make sure to return a list type
def make_list(env, s):
	if type(s) is types.ListType: return s
	else:
		try: return s.split()
		except AttributeError: return s

## HELPER 
def join(lenv, s1, s2, s3=None, s4=None):
	if s4 and s3: return lenv.join(s1, s2, lenv.join(s3, s4))
	if s3 and s2: return lenv.join(s1, lenv.join(s2, s3))
	elif not s2:  return s1
	# having s1, s2
	#print "path1 is "+s1+" path2 is "+s2+" "+os.path.join(s1,string.lstrip(s2,'/'))
	if not s1: s1='/' # TODO on win32 this will not work (for js)
	return os.path.join(s1,string.lstrip(s2,'/'))

## HELPER export the data to xml
bks_dump='<?xml version="1.0" encoding="UTF-8"?>\n<bksys version="1">\n'
def add_dump(nenv, str):
	global bks_dump
	if str: bks_dump+=str
def get_dump(nenv):
	if not nenv.has_key('DUMPCONFIG'):
		nenv.pprint('RED','WARNING: trying to get a dump while DUMPCONFIG is not set - this will not work')
	global bks_dump
	return bks_dump+"</bksys>\n"

## HELPER - is used by the xml stuff in pmanager (ignore for now)
def getreldir(lenv):
	cwd=os.getcwd()
	root=SCons.Node.FS.default_fs.Dir('#').abspath
	return cwd.replace(root,'').lstrip('/')

## HELPER - find programs and headers
def find_path(lenv, file, path_list):
	for dir in path_list:
		if os.path.exists( lenv.join(dir, file) ):
			return dir
	return ''

def find_file(lenv, file, path_list):
	for dir in path_list:
                if os.path.exists( lenv.join(dir, file) ):
                        return lenv.join(dir, file)
        return ''

def find_program(lenv, prog):
	return os.popen("which %s 2>/dev/null" % prog).read().strip()

## Scons-specific function, do not remove
def exists(env):
	return true

## Entry point of the module generic
def generate(env):

	## (added by Coolo apparently)
	## i see what it does now - however importing default tools causes performance issues (TODO ita)
	from SCons.Tool import Tool;
	Tool('default').generate(env)

	if env['PLATFORM'] in ('cygwin', 'win32', 'win64'):
		env['WINDOWS']=1
	else:
		env['WINDOWS']=0

	## Bksys requires scons >= 0.96
	env.EnsureSConsVersion(0, 96)

	## attach the helper functions to "env"
	SConsEnvironment.pprint = pprint
	SConsEnvironment.make_list = make_list
	SConsEnvironment.join = join
	SConsEnvironment.dist = dist
	SConsEnvironment.getreldir = getreldir
	SConsEnvironment.add_dump = add_dump
	SConsEnvironment.get_dump = get_dump

	# list of the modules which provide a config.h
	env['_CONFIG_H_']=[]

	import sys
	env['HELP']=0
	if '--help' in sys.argv or '-h' in sys.argv or 'help' in sys.argv: env['HELP']=1
	if env['HELP']:
		p=env.pprint
		p('BOLD','*** Instructions ***')
		p('BOLD','--------------------')
		p('BOLD','* scons           ','to compile')
		p('BOLD','* scons -j4       ','to compile with several instances')
		p('BOLD','* scons install   ','to compile and install')
		p('BOLD','* scons -c install','to uninstall')
		p('BOLD','\n*** Generic options ***')
		p('BOLD','--------------------')
		p('BOLD','* debug        ','debug=1 (-g) or debug=full (-g3, slower) else use environment CXXFLAGS, or -O2 by default')
		p('BOLD','* prefix       ','the installation path')
		p('BOLD','* extraincludes','a list of paths separated by ":"')
		p('BOLD','* scons configure debug=full prefix=/usr/local extraincludes=/tmp/include:/usr/local')
		p('BOLD','* scons install prefix=/opt/local DESTDIR=/tmp/blah\n')
		return

	## Global cache directory
	# Put all project files in it so a rm -rf cache will clean up the config
	if not env.has_key('CACHEDIR'): env['CACHEDIR'] = env.join(os.getcwd(),'cache')+os.sep
	if not os.path.isdir(env['CACHEDIR']): os.mkdir(env['CACHEDIR'])
	
	## SCons cache directory
	# This avoids recompiling the same files over and over again: 
	# very handy when working with cvs
	# TODO: not portable so add a win32 ifdef
	if os.getuid() != 0: env.CacheDir( env.join(os.getcwd(),'cache','objects') )

	#  Avoid spreading .sconsign files everywhere - keep this line
	env.SConsignFile( env['CACHEDIR']+'scons_signatures' )
	
	# TODO: this parser is silly, fix scons command-line handling instead
	def makeHashTable(args):
		table = { }
		for arg in args:
			if len(arg) > 1:
				lst=arg.split('=')
				if len(lst) < 2: continue
				key=lst[0]
				value=lst[1]
				if len(key) > 0 and len(value) >0: table[key] = value
 		return table

	env['ARGS']=makeHashTable(sys.argv)

	# Another helper, very handy
	SConsEnvironment.Chmod = SCons.Action.ActionFactory(os.chmod, lambda dest, mode: 'Chmod("%s", 0%o)' % (dest, mode))
	#SConsEnvironment.Symlink = SCons.Action.ActionFactory(os.symlink, lambda dest, link: 'symlink("%s", "%s")' % (dest, link))

	## Special trick for installing rpms ...
	env['DESTDIR']=''
	if 'install' in sys.argv:
		dd=''
		if os.environ.has_key('DESTDIR'): dd=os.environ['DESTDIR']
		if not dd:
			if env['ARGS'] and env['ARGS'].has_key('DESTDIR'): dd=env['ARGS']['DESTDIR']
		if dd:
			env['DESTDIR']=dd
			env.pprint('CYAN','** Enabling DESTDIR for the project ** ',env['DESTDIR'])

	## Use the same extension .o for all object files
	env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME'] = 1

	## load the options
	cachefile=env['CACHEDIR']+'generic.cache.py'
	opts = Options(cachefile)
	opts.AddOptions(
		('PREFIX', 'prefix for installation' ),
		('GENCCFLAGS', 'C flags' ),
                ('GENCXXFLAGS', 'additional cxx flags for the project' ),
		('GENLINKFLAGS', 'additional link flags' ),
		('EXTRAINCLUDES', 'extra include paths for the project' ),
		('BKS_DEBUG', 'debug level: full, trace, or just something' ),
		('_USECOLORS_', 'colors for developers'),
		('GENERIC_ISCONFIGURED', 'is the project configured' ),
	)
	opts.Update(env)
	
	# Use this to avoid an error message 'how to make target configure ?'
	env.Alias('configure', None)

	# Check if the following command line arguments have been given
	# and set a flag in the environment to show whether or not it was
	# given.
	if 'install' in sys.argv: env['_INSTALL_']=1
	else: env['_INSTALL_']=0
	if 'configure' in sys.argv: env['_CONFIGURE_']=1
	else: env['_CONFIGURE_']=0

	# Configure the environment if needed
	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('GENERIC_ISCONFIGURED')):
		env['_CONFIGURE_']=1

		import sys
		if sys.platform == 'darwin':
			#sys.path.append('bksys'+os.sep+'osx')
			#from detect_lowlevel import detect
			env.pprint('RED', 'Not implemented, see bksys/osx/lowlevel.py')
			env.Exit(1)
		elif env['WINDOWS']:
			sys.path.append('bksys'+os.sep+'win32')
			from detect_generic import detect
			detect(env)
		else:
			sys.path.append('bksys'+os.sep+'unix')
			from detect_generic import detect
			detect(env)

		env['GENERIC_ISCONFIGURED']=1

		# And finally save the options in the cache
		opts.Save(cachefile, env)

	if env['_USECOLORS_']:
		building_obj='%scompiling%s $TARGET' % (colors['GREEN'], colors['NORMAL'])
		env['CCCOMSTR']=building_obj
		env['SHCCCOMSTR']=building_obj
		env['CXXCOMSTR']=building_obj
		env['SHCXXCOMSTR']=building_obj
		link_obj='%slinking%s $TARGET' % (colors['YELLOW'], colors['NORMAL'])
		env['LINKCOMSTR']=link_obj
		env['SHLINKCOMSTR']=link_obj

	def bksys_install(lenv, subdir, files, destfile=None, perms=None):
		""" Install files on 'scons install' """
		if not env['_INSTALL_']: return
		basedir = env['DESTDIR']
		install_list=None
		if not destfile: install_list = env.Install(lenv.join(basedir,subdir), lenv.make_list(files))
		elif subdir:     install_list = env.InstallAs(lenv.join(basedir,subdir,destfile), lenv.make_list(files))
		else:            install_list = env.InstallAs(lenv.join(basedir,destfile), lenv.make_list(files))
		# FIXME: this will not work with a list of files
		#if perms and install_list: lenv.AddPostAction(install_list, lenv.Chmod(install_list, perms))
		env.Alias('install', install_list)
		return install_list

	def bksys_insttype(lenv, type, subdir, files, perms=None):
		if not lenv.has_key(type):
			lenv.pprint('RED', 'unknown file type to install '+type)
			lenv.Exit(1)
		lenv.bksys_install( lenv.join(lenv[type],subdir), files, destfile=None, perms=perms)

	def build_la_file(target, source, env):
		""" Writes a .la file, used by libtool """
		dest=open(target[0].path, 'w')
		sname=source[0].name
		dest.write("# Generated by ltmain.sh - GNU libtool 1.5.18 - (pwn3d by bksys)\n#\n#\n")
		if len(env['BKSYS_VNUM'])>0:
			vnum=env['BKSYS_VNUM']
			nums=vnum.split('.')
			src=source[0].name
			name = src.split('so.')[0] + 'so'
			strn = src+" "+name+"."+str(nums[0])+" "+name
			dest.write("dlname='%s'\n" % (name+'.'+str(nums[0])) )
			dest.write("library_names='%s'\n" % (strn) )
		else:
			dest.write("dlname='%s'\n" % sname)
			dest.write("library_names='%s %s %s'\n" % (sname, sname, sname) )
		dest.write("old_library=''\ndependency_libs=''\ncurrent=0\n")
		dest.write("age=0\nrevision=0\ninstalled=yes\nshouldnotlink=no\n")
		dest.write("dlopen=''\ndlpreopen=''\n")
		dest.write("libdir='%s'" % env['BKSYS_DESTDIR'])
		dest.close()
		return 0

	def string_la_file(target, source, env):
		print "building '%s' from '%s'" % (target[0].name, source[0].name)
	la_file = env.Action(build_la_file, string_la_file, ['BKSYS_VNUM', 'BKSYS_DESTDIR'])
	env['BUILDERS']['LaFile'] = env.Builder(action=la_file,suffix='.la',src_suffix=env['SHLIBSUFFIX'])

	## Build symlinks
	def symlink_command(target, source, env):
		os.symlink( str(source[0].name), target[0].path)
	def symlink_string(target, source, env):
		if env['_USECOLORS_']:
			print colors['GREEN']+"symlinking"+colors['NORMAL']+" %s (-> %s)" % (target[0].path, source[0].name)
		else:
			print "symlinking %s (-> %s)" % (target[0].path, source[0].name)
	symlink = env.Action(symlink_command, symlink_string)
	env['BUILDERS']['SymLink'] = env.Builder(action=symlink)

	## Function for building shared libraries
	def bksys_shlib(lenv, ntarget, source, libdir, libprefix='lib', vnum='', noinst=None):
		"""Installs a shared library, with or without a version number, and create a
		.la file for use by libtool.
		
		If library version numbering is to be used, the version number
		should be passed as a period-delimited version number (e.g.
		vnum = '1.2.3').  This causes the library to be installed
		with its full version number, and with symlinks pointing to it.
		
		For example, for libfoo version 1.2.3, install the file
		libfoo.so.1.2.3, and create symlinks libfoo.so and
		libfoo.so.1 that point to it.
		"""
		# parameter can be a list
		if type(ntarget) is types.ListType: target=ntarget[0]
		else: target=ntarget

		thisenv = lenv.Copy() # copying an existing environment is +/- cheap
		thisenv['BKSYS_DESTDIR']=libdir
		thisenv['BKSYS_VNUM']=vnum
		thisenv['SHLIBPREFIX']=libprefix

		if len(vnum)>0:
			thisenv['SHLIBSUFFIX']='.so.'+vnum
			thisenv.Depends(target, thisenv.Value(vnum))
			num=vnum.split('.')[0]
			lst=target.split('/')
			tname=lst[len(lst)-1]
			libname=tname.split('.')[0]
			thisenv.AppendUnique(LINKFLAGS = ["-Wl,--soname=%s.so.%s" % (libname, num)] )

		# Fix against a scons bug - shared libs and ordinal out of range(128)
		if type(source) is types.ListType:
			src2=[]
			for i in source: src2.append( str(i) )
			source=src2

		library_list = thisenv.SharedLibrary(target, source)
		lafile_list  = thisenv.LaFile(target, library_list)

		## Install the libraries automatically
		if not thisenv.has_key('NOAUTOINSTALL') and not noinst and libdir:
			inst_lib=thisenv.bksys_install(libdir, library_list)
			thisenv.bksys_install(libdir, lafile_list)	

		## Handle the versioning
		if len(vnum)>0:
			nums=vnum.split('.')
			symlinkcom = ('cd $SOURCE.dir && rm -f $TARGET.name && ln -s $SOURCE.name $TARGET.name')
			tg = target+'.so.'+vnum
			nm1 = target+'.so'
			nm2 = target+'.so.'+nums[0]

			thisenv.SymLink(target=nm1, source=library_list)
			thisenv.SymLink(target=nm2, source=library_list)

			if env['_INSTALL_'] and libdir:
				link1 = env.join(str(inst_lib[0].dir), nm1)
				link2 = env.join(str(inst_lib[0].dir), nm2)
				src   = str(inst_lib[0].name)
				env.Alias('install', env.SymLink(target=link1, source=src))
				env.Alias('install', env.SymLink(target=link2, source=src))
		return library_list

	# Declare scons scripts to process
	def subdirs(lenv, folderlist):
		flist=lenv.make_list(folderlist)
		for i in flist:
			lenv.SConscript(lenv.join(i, 'SConscript'))
		# take all objects - warn those who are not already executed
		if lenv.has_key('USE_THE_FORCE_LUKE'):
			for ke in lenv['USE_THE_FORCE_LUKE']:
				if ke.executed: continue
				lenv.pprint('GREEN',"you forgot to execute object "+ke.target)
				ke.lockworkdir()
				ke.execute()
				ke.unlockworkdir()

        def link_local_shlib(lenv, str):
                """ Links against a shared library made in the project """
                lst = lenv.make_list(str)
		for file in lst:
			import re
			reg=re.compile("(.*)/lib(.*).(la|so)$")
			result=reg.match(file)
			if not result:
				reg = re.compile("(.*)/lib(.*).(la|so)\.(.)")
				result=reg.match(file)
				if not result:
					print "Unknown la file given "+file
					continue
				dir  = result.group(1)
				link = result.group(2)
			else:
				dir  = result.group(1)
				link = result.group(2)

			lenv.AppendUnique(LIBS = [link])
			lenv.PrependUnique(LIBPATH = [dir])

        def link_local_staticlib(lenv, str):
                """ Links against a shared library made in the project """
                lst = lenv.make_list(str)
		for file in lst:
			import re
			reg = re.compile("(.*)/(lib.*.a)")
			result = reg.match(file)
			if not result:
				print "Unknown archive file given "+file
				continue
			f=SCons.Node.FS.default_fs.File(file)
			lenv.Append(LIBPATH=[f.dir])
			lenv.Append(LIBS=[f.name])

        SConsEnvironment.bksys_install = bksys_install
	SConsEnvironment.bksys_insttype = bksys_insttype
	SConsEnvironment.bksys_shlib   = bksys_shlib
	SConsEnvironment.subdirs       = subdirs
	SConsEnvironment.link_local_shlib = link_local_shlib
	SConsEnvironment.link_local_staticlib = link_local_staticlib
	SConsEnvironment.genobj=genobj
	SConsEnvironment.find_path=find_path
	SConsEnvironment.find_file=find_file
	SConsEnvironment.find_program=find_program

	if env.has_key('GENCXXFLAGS'):   env.AppendUnique( CPPFLAGS  = env['GENCXXFLAGS'] )
	if env.has_key('GENCCFLAGS'):    env.AppendUnique( CCFLAGS   = env['GENCCFLAGS'] )
	if env.has_key('GENLINKFLAGS'):  env.AppendUnique( LINKFLAGS = env['GENLINKFLAGS'] )
	if env.has_key('EXTRAINCLUDES'): env.AppendUnique( CPPPATH   = env['EXTRAINCLUDES'])

        if env.has_key('BKS_DEBUG'):
                if (env['BKS_DEBUG'] == "full"):
                        env.AppendUnique(CXXFLAGS = ['-DDEBUG', '-Wall'])
                        if env['WINDOWS']:
                                pass
                        else:
                                env.AppendUnique(CXXFLAGS = ['-g3'])
                elif (env['BKS_DEBUG'] == "trace"): # i cannot remember who wanted this (TODO ita)
                        env.AppendUnique(
                                LINKFLAGS=env.Split("-lmrwlog4cxxconfiguration -lmrwautofunctiontracelog4cxx -finstrument-functions"),
                                CXXFLAGS=env.Split("-DDEBUG -Wall -finstrument-functions -g3 -O0"))
                else:
                        env.AppendUnique(CXXFLAGS = ['-DDEBUG', '-g', '-Wall'])

	env.Export('env')

