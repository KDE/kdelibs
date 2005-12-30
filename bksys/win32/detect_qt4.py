##
# @file 
# win32 related QT 4 detection 

import os, re, types, sys

# check if qt was build with rtti and exceptions and add correct flgas from mkspec
def check_qtconfig(env):
	p=env.pprint

	if env['ARGS'] and env['ARGS'].has_key('platform'):
		# first check for argument 'platform'
		platform = env['ARGS']['platform']
	elif os.environ.has_key('QMAKESPEC'):
		# the look for QMAKESPEC
		platform = os.environ['QMAKESPEC']
	else:
		print 'Using default values for Qt checks as QMAKESPEC is not defined, please define it to the proper value'
		return '','4'
	if env['ARGS'] and env['ARGS'].has_key('exceptions'):
		exceptions = 1
	else:
		exceptions = 0

	qtdir		= env['QTDIR']
	qmakecache	= env.join(qtdir, '.qmake.cache')
	qmakeconf	= env.join(qtdir, 'mkspecs', platform, 'qmake.conf' )

	debug = 0
	
	# read .qmake.cache to 
	# - check if QMAKESPEC == platform
	# - check if no-rtti isn't set (we need it for dynamic_cast)
	# - get some defines
	# - wheter to use debug or release libs

	kde_debug = env['ARGS'].get('debug', None) != None
	qt_debug = 0
	
	cxxflags = ''
	file = open( qmakecache )
	for line in file:
		line = line.rstrip()
		val = line.split('=')[0].rstrip('+* ')
		val = val.rstrip()
		if val == 'CONFIG':
			for word in line.split():
				if word == 'no-rtti':
					p('RED', 'Qt isn\'t compiled with rtti support' )
					env.Exit(1)
				elif word == 'debug':
					qt_debug = 1
					if not kde_debug:
						p('YELLOW', 'Qt is compiled with debug support but you won\'t compile KDE in debug mode.')
				elif word == 'release' and kde_debug:
					qt_debug = 0
		elif val == 'QMAKESPEC':
			# problem: compiled with qt4/free and msvc/borland we get 'win32-g++' here ...
			# so only a warning
			plat = line.split('=')[1]
			if ( plat != platform ):
				p('YELLOW', 'Qt is compiled for ' + plat + ' but you want to compile KDE for ' + platform + '!')
		elif val == 'DEFINES':
			defines = line.split('*=')[1]
			for d in defines.split():
				cxxflags += '-D' + d + ' '
	file.close()
	
	file = open( qmakeconf )
	# fixme - what if an option is defined this way: QMAKE_CXXFLAGS_THREAD	= $$QMAKE_CFLAGS_THREAD
	for line in file:
		line = line.rstrip()
		val = line.split('=')[0].rstrip('+*')
		val = val.rstrip()
		cxxflag = ''
		if kde_debug:
			if val == 'QMAKE_CFLAGS_WARN_ON':
				cxxflag =  line.split('=')[1]
			elif val == 'QMAKE_CFLAGS_DEBUG':
				cxxflag =  line.split('=')[1]
		else:
			if val == 'QMAKE_CFLAGS_WARN_OFF':
				cxxflag =  line.split('=')[1]
			elif val == 'QMAKE_CFLAGS_RELEASE':
				cxxflag =  line.split('=')[1]

		if val == 'DEFINES':
			defines = line.split('=')[1]
			for d in defines.split():
				cxxflags += '-D' + d + ' '
		elif val == 'QMAKE_CXXFLAGS_RTTI_ON':
			cxxflag =  line.split('=')[1]
		elif val == 'QMAKE_CXXFLAGS_EXCEPTIONS_ON':
			if exceptions == 1:
				cxxflag =  line.split('=')[1]
		if ( cxxflag != '' ):
			cxxflags += cxxflag + ' '
	file.close()

	if kde_debug:
		cxxflags += '-DQT_DEBUG '
	else:
		cxxflags += '-DQT_NO_DEBUG '
	env['CXXFLAGS_QT'] = cxxflags
	return qt_debug

def detect(env):
	import re

	def getpath(varname):
		if not env.has_key('ARGS'): return None
		v=env['ARGS'].get(varname, None)
		if v : v=os.path.abspath(v)
		return v

	# TODO (rh) libsuffix and lib_addon may be overlap in some areas, potential for cleanup 
	libsuffix	= ''
	if env.has_key('ARGS'): libsuffix=env['ARGS'].get('libsuffix', '')

	p=env.pprint
	# do our best to find the QTDIR
	qtdir       = os.getenv("QTDIR")
	qtincludes	= os.getenv("QTINCLUDES")
	qtlibs		= os.getenv("QTLIBS")
	if not qtdir:
		qtdir=env.find_path('include/', [ # lets find the qt include directory
				'c:/Qt/4.0.3/', # one never knows
				'c:/Qt/4.0.2/',
				'c:/Qt/4.0.1/',
				'c:/Qt/4.0.0/'])
		if qtdir:
			p('YELLOW', 'The qtdir was found as '+qtdir)
		else:
			p('YELLOW', 'There is no QTDIR set')
			return

	env['QTDIR'] = qtdir.strip()

	# if we have the QTDIR, finding the qtlibs and qtincludes is easy
	if qtdir:
		if not qtlibs:     qtlibs     = os.path.join(qtdir, 'lib' + libsuffix)
		if not qtincludes: qtincludes = os.path.join(qtdir, 'include')
		#os.putenv('PATH', os.path.join(qtdir , 'bin') + ":" + os.getenv("PATH")) # TODO ita 

	# Check for uic, uic-qt3, moc, rcc, ..
	def find_qt_bin(progs):
		# first use the qtdir
		path=''
		for prog in progs:
			path=env.find_program(prog, [env.join(qtdir, 'bin')])
			if path:
				p('GREEN',"%s was found as %s" % (prog, path))
				return path

		# everything failed
		p('RED',"%s was not found - make sure Qt4-devel is installed, or set $QTDIR or $PATH" % prog)
		env.Exit(1)
	
	print "Checking for uic               :",
	env['QT_UIC']  = find_qt_bin(['uic', 'uic-qt4'])

	print "Checking for uic-qt3           :",
	env['QT_UIC3'] = find_qt_bin(['uic3', 'uic-qt3'])

	print "Checking for moc               :",
	env['QT_MOC'] = find_qt_bin(['moc', 'moc-qt4'])

	print "Checking for rcc               :",
	env['QT_RCC'] = find_qt_bin(['rcc'])

	# TODO is this really needed now ?
	print "Checking for uic3 version      :",
	version = os.popen(env['QT_UIC'] + " -version 2>&1").read().strip()
	ver_str = re.match( '.*([0-9]\.[0-9]\.[0-9].*)\.', version ).group(1)
	ver_major = int(ver_str[0:1])
	ver_minor = int(ver_str[2:3])
	ver_patch = int(ver_str[4:5])
	ver_add   = ver_str[6:]
	print str(ver_major)+'.'+str(ver_minor)+'.'+str(ver_patch)+'-'+ver_add
	if ver_major != 4:
		p('RED', ver_str + " (too old)")
		env.Exit(1)
	p('GREEN', "fine - %s" % version)
	
	if ( ver_major == 4 and ver_minor < 1 ):
		p('RED', "At least Qt version 4.1.0rc1 is needed! You've only "+ ver_str )
		env.Exit(1)

	#if os.environ.has_key('PKG_CONFIG_PATH'):
	#	os.environ['PKG_CONFIG_PATH'] = os.environ['PKG_CONFIG_PATH'] + ':' + qtlibs
	#else:
	#	os.environ['PKG_CONFIG_PATH'] = qtlibs

	## check for the Qt4 includes
	print "Checking for the Qt4 includes  :",
	if qtincludes and os.path.isfile(qtincludes + "/QtGui/QFont"):
		# The user told where to look for and it looks valid
		p('GREEN','ok '+qtincludes)
	else:
		if os.path.isfile(qtdir+'/include/QtGui/QFont'):
			# Automatic detection
			p('GREEN','ok '+qtdir+"/include/")
			qtincludes = qtdir + "/include/"
		else:
			p('RED',"the qt headers were not found")
			env.Exit(1)


	#env['QTPLUGINS']=os.popen('kde-config --expandvars --install qtplugins').read().strip()

	## qt libs and includes
	env['QTINCLUDEPATH']=qtincludes
	if not qtlibs: qtlibs=env.join(qtdir, 'lib', libsuffix)
	env['QTLIBPATH']=qtlibs

	########## QT
	# QTLIBPATH is a special var used in the qt4 module - has to be changed (ita)

	debug = check_qtconfig(env)
	if debug:
		lib_addon = 'd'+str(ver_major)
		test_addon = '_debug'+str(ver_major)	# inconsistent naming sheme
	else:
		lib_addon = str(ver_major)
		test_addon = str(ver_major)

	# may be required by msvc too 
	if env['CC'] == 'gcc':
		env['CXXFLAGS_QT']         = ['-DQT_DLL','-DQT_THREAD_SUPPORT','-DQT_NEEDS_QMAIN']
	env['CPPPATH_QT']          = [ env.join(env['QTINCLUDEPATH'], 'Qt'), env['QTINCLUDEPATH'] ] # TODO QTINCLUDEPATH (ita)
	env['LIBPATH_QT']          = [env['QTLIBPATH']]
	env['LIB_QT']              = ['QtGui'+lib_addon]
	env['RPATH_QT']            = [env['QTLIBPATH']]

	env['CXXFLAGS_QT3SUPPORT'] = ['-DQT3_SUPPORT']
	env['CPPPATH_QT3SUPPORT']  = [ env.join(env['QTINCLUDEPATH'], 'Qt3Support') ]
	env['LIB_QT3SUPPORT']      = ['Qt3Support'+lib_addon]
	env['RPATH_QT3SUPPORT']    = env['RPATH_QT']
	
	env['CXXFLAGS_QTCORE']     = ['-DQT_CORE_LIB']
	env['CPPPATH_QTCORE']      = [ env.join(env['QTINCLUDEPATH'], 'QtCore') ]
	env['LIB_QTCORE']          = ['QtCore'+lib_addon]
	env['RPATH_QTCORE']        = env['RPATH_QT']
	
	env['CXXFLAGS_QTGUI']      = ['-DQT_GUI_LIB']
	env['CPPPATH_QTGUI']       = [ env.join(env['QTINCLUDEPATH'], 'QtGui') ]
	env['LIB_QTGUI']           = ['QtCore'+lib_addon, 'QtGui'+lib_addon]
	env['RPATH_QTGUI']         = env['RPATH_QT']
	
	env['CXXFLAGS_QTNETWORK']  = ['-DQT_NETWORK_LIB']
	env['CPPPATH_QTNETWORK']   = [ env.join(env['QTINCLUDEPATH'], 'QtNetwork') ]
	env['LIB_QTNETWORK']       = ['QtNetwork'+lib_addon]
	env['RPATH_QTNETWORK']     = env['RPATH_QT']
	
	env['CPPPATH_OPENGL']      = [ env.join(env['QTINCLUDEPATH'], 'QtOpengl') ]
	env['LIB_QTOPENGL']        = ['QtOpenGL'+lib_addon]
	env['RPATH_QTOPENGL']      = env['RPATH_QT']
	
	env['CPPPATH_QTSQL']       = [ env.join(env['QTINCLUDEPATH'], 'QtSql') ]
	env['LIB_QTSQL']           = ['QtSql'+lib_addon]
	env['RPATH_QTSQL']         = env['RPATH_QT']
	
	env['CPPPATH_QTXML']       = [ env.join(env['QTINCLUDEPATH'], 'QtXml') ]
	env['LIB_QTXML']           = ['QtXml'+lib_addon]
	env['RPATH_QTXML']         = env['RPATH_QT']
	
	env['CPPPATH_QTEST']       = [ env.join(env['QTINCLUDEPATH'], 'QtCore') ]
	env['LIB_QTEST']           = ['QtTest'+test_addon]
	env['RPATH_QTEST']         = env['RPATH_QT']
	
	env['CPPPATH_QTASSISTANT'] = [ env.join(env['QTINCLUDEPATH'], 'QtAssistant') ]
	env['LIB_QTASSISTANT']     = ['QtAssistant'+lib_addon]
	
	env['CPPPATH_QTDESIGNER']  = [ env.join(env['QTINCLUDEPATH'], 'QtDesigner') ]
	env['LIB_QTDESIGNER']      = ['QtDesigner'+lib_addon]
	
	env['QTLOCALE']=env.join(env['PREFIX'], 'share', 'locale')

	if env['CC'] == 'gcc':    
		qtmingwflags = '-DQT_DLL -DQT_THREAD_SUPPORT'
		env['GENCXXFLAGS']  += qtmingwflags.split()
	'-DQT_EDITION=QT_EDITION_DESKTOP -DUNICODE -DQT_LARGEFILE_SUPPORT  -w  -O2  -frtti -DQT_NO_DEBUG '