#! /usr/bin/env python

BOLD   ="\033[1m"
RED    ="\033[91m"
GREEN  ="\033[92m"
YELLOW ="\033[1m" #"\033[93m" # unreadable on white backgrounds
CYAN   ="\033[96m"
NORMAL ="\033[0m"

def exists(env):
	return true

import types

def generate(env):
	import SCons.Util, os

	if env['HELP']:
		print """
"""+BOLD+"""*** Sound options ***
-----------------------"""+NORMAL+"""
"""+BOLD+"""* noalsa  """+NORMAL+""": disable alsa
"""+BOLD+"""* nojack """+NORMAL+""": disable jack
ie: """+BOLD+"""scons configure noalsa=1 nojack=1
"""+NORMAL
		return

	# load the options
	from SCons.Options import Options, PathOption
	cachefile = env['CACHEDIR']+'/gtk2.cache.py'
	opts = Options(cachefile)
	opts.AddOptions(
		( 'DATA', 'data directory' ),
		( 'LIBDIR', 'library directory' ),
		( 'BINDIR', 'library directory' )
		)
	opts.Update(env)

	import sys
	from SCons.Tool import Tool

	pkgs = Tool('pkgconfig', ['./bksys'])
	pkgs.generate(env)
	x
		haveglib  = env.pkgConfig_findPackage('GLIB', 'glib-2.0', '2.6')
		havegtk   = env.pkgConfig_findPackage('GTK', 'gtk+-2.0', '2.6')
		haveglade = env.pkgConfig_findPackage('GLADE', 'libglade-2.0', '2.0')

		env['DATA'] = env['PREFIX']+'/share'
		env['LIBDIR'] = env['PREFIX']+'/lib'
		env['BINDIR'] = env['PREFIX']+'/bin'
		
		opts.Save(cachefile, env)


	## load non-cacheable options

        import SCons.Defaults
        import SCons.Tool
        import SCons.Util

        ## libtool file builder
        def la_file(target, source, env):
                dest=open(target[0].path, 'w')
                sname=source[0].name
                dest.write("dlname='%s'\n" % sname)
                dest.write("library_names='%s %s %s'\n" % (sname, sname, sname))
                dest.write("old_library=''\n")
                dest.write("dependency_libs=''\n")
                dest.write("current=0\n")
                dest.write("age=0\n")
                dest.write("revision=0\n")
                dest.write("installed=yes\n")
                dest.write("shouldnotlink=no\n")
                dest.write("dlopen=''\n")
                dest.write("dlpreopen=''\n")
                dest.write("libdir='%s'" % (env['PREFIX']+"/lib") )
                dest.close()
                return 0

		LA_BUILDER = env.Builder(
			action     = la_file,
			suffix     = '.la',
			src_suffix = env['SHLIBSUFFIX'])

		env['BUILDERS']['LaFile']    = LA_BUILDER
	

	if env.has_key('GTK_CCFLAGS'):
		env.AppendUnique(CCFLAGS = env['GTK_CCFLAGS'] )
		#env.AppendUnique(CXXFLAGS = env['GTK_CCFLAGS'] )
	if env.has_key('GTK_LDFLAGS'):
		env.AppendUnique(LINKFLAGS = env['GTK_LDFLAGS'] )

        def make_list(e):
                if type(e) is types.ListType:
                        return e
                else:
                        return e.split()

	def G_files(lenv, target, source):
		return make_list(source)

        import glob
        def G_install(lenv, restype, subdir, files):
                if not env['_INSTALL']:
                        return
                basedir=env['DESTDIR']
                if len(restype)>0:
                        if not lenv.has_key(restype):
                                print RED+"unknown resource type "+restype+NORMAL
                        else:
                                basedir += lenv[restype]+'/'
                #print file # <- useful to trace stuff :)
                install_list =  env.Install(basedir+subdir+'/', make_list(files) )
                env.Alias('install', install_list)
                return install_list

        def G_installas(lenv, restype, destfile, file):
                if not env['_INSTALL']:
                        return
                basedir=env['DESTDIR']
                if len(restype)>0:
                        if not lenv.has_key(restype):
                                print RED+"unknown resource type "+restype+NORMAL
                        else:
                                basedir += lenv[restype]+'/'
                install_list = env.InstallAs(basedir+destfile, file)
                env.Alias('install', install_list)
                return install_list

        def G_program(lenv, target, source):
                """ Makes a kde program
                The program is installed except if one sets env['NOAUTOINSTALL'] """
                src = G_files(lenv, target, source)
                program_list = lenv.Program(target, src)
                if not lenv.has_key('NOAUTOINSTALL'):
                        G_install(lenv, 'BINDIR', '', target)
                return program_list

        def G_shlib(lenv, target, source, kdelib=0, libprefix='lib'):
                """ Makes a shared library for kde (.la file for klibloader)
                The library is installed except if one sets env['NOAUTOINSTALL'] """
                src = G_files(lenv, target, source)
                lenv['LIBPREFIX']=libprefix
                library_list = lenv.SharedLibrary(target, src)
                lafile_list  = lenv.LaFile(target, library_list)
                if not lenv.has_key('NOAUTOINSTALL'):
                        install_dir = 'LIBDIR'
                        if kdelib==1:
                                install_dir = 'LIBDIR'
                        G_install(lenv, install_dir, '', library_list)
                        G_install(lenv, install_dir, '', lafile_list)
                return library_list

        def G_staticlib(lenv, target, source):
                """ Makes a static library for kde - in practice you should not use static libraries
                1. they take more memory than shared ones
                2. makefile.am needed it because of limitations
                (cannot handle sources in separate folders - takes extra processing) """
                src = G_files(lenv, target, source)
                return lenv.StaticLibrary(target, src)
                # do not install static libraries by default

        def G_addflags_c(lenv, fl):
                """ Compilation flags for C programs """
                lenv.AppendUnique(CCFLAGS = make_list(fl))

        def G_addflags_link(lenv, fl):
                """ Add link flags - Use this if G_addlibs below is not enough """
                lenv.AppendUnique(LINKFLAGS = make_list(fl))

        def G_addlibs(lenv, libs):
                """ Helper function """
                lenv.AppendUnique(LIBS = make_list(libs))

        def G_addpaths_includes(lenv, paths):
                """ Add new include paths """
                lenv.AppendUnique(CPPPATH = make_list(paths))

        def G_addpaths_libs(lenv, paths):
                """ Add paths to libraries """
                lenv.AppendUnique(LIBPATH = make_list(paths))

        def subdirs(lenv, folderlist):
                flist=make_list(folderlist)
                for i in flist:
                        lenv.SConscript(i+"/SConscript")

        def Use(lenv, flags):
                _flags=make_list(flags)
                if 'environ' in _flags:
                        ## The scons developers advise against using this but it is mostly innocuous :)
                        import os
                        lenv.AppendUnique( ENV = os.environ )
                if not 'lang_qt' in _flags:
                        ## Use this define if you are using the kde translation scheme (.po files)
                        lenv.Append( CPPFLAGS = '-DQT_NO_TRANSLATION' )
                if 'rpath' in _flags:
                        ## Use this to set rpath - this may cause trouble if folders are moved (chrpath)
                        lenv.Append( RPATH = [lenv['PREFIX']+'/lib'] )
                if 'thread' in _flags:
                        ## Uncomment the following if you need threading support
                        lenv.G_addflags_cxx( ['-DQT_THREAD_SUPPORT', '-D_REENTRANT'] )
                if not 'nohelp' in _flags:
                        if lenv['_CONFIGURE'] or lenv['HELP']:
                                env.Exit(0)

        from SCons.Script.SConscript import SConsEnvironment
        SConsEnvironment.G_program = G_program
        SConsEnvironment.G_shlib = G_shlib
        SConsEnvironment.G_staticlib = G_staticlib
        SConsEnvironment.G_install = G_install
        SConsEnvironment.G_installas = G_installas

        SConsEnvironment.G_addflags_c = G_addflags_c
        SConsEnvironment.G_addflags_link = G_addflags_link
        SConsEnvironment.G_addlibs = G_addlibs
        SConsEnvironment.G_addpaths_includes = G_addpaths_includes
        SConsEnvironment.G_addpaths_libs = G_addpaths_libs	
	
	SConsEnvironment.Use = Use
	SConsEnvironment.subdirs = subdirs
	

	
