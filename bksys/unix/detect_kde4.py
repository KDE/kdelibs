# Thomas Nagy, 2005

import os

def bootstrap(env):

	#print "bootstrapping kdelibs"

	########### KDE4
	env['LIBPATH_KDE4'] = env['LIBPATH_QT']

	## not very portable but that's the best i have at the moment (ITA)
	includes=['.','dcop','kio','kio/kio','kio/kfile','kdeui','kdecore','libltdl','kdefx']
	if env['WINDOWS']:
		includes.append('win')
	env['INCLUDES_KDE4']=['#build']
	for dir in includes:
		env['INCLUDES_KDE4'].append('#'+dir)
		env['INCLUDES_KDE4'].append('#build/'+dir)

	# prefix/lib and prefix/lib/kde4 (ita)
	env['RPATH_KDE4']= [
		env['QTLIBPATH'], env.join(env['PREFIX'], 'lib'), env.join(env['PREFIX'], 'lib', 'kde4')
	]

	#######################################
	## install paths
	env['KDEBIN']    = env.join( env['PREFIX'], 'bin')
	env['KDEDATA']   = env.join( env['PREFIX'], 'share')
	env['KDEAPPS']   = env.join( env['PREFIX'], 'share/apps')
	env['KDEKCFG']   = env.join( env['PREFIX'], 'share/config.kcfg')
	env['KDEICONS']   = env.join( env['PREFIX'], 'share/icons')
	env['KDEMIME']   = env.join( env['PREFIX'], 'share/mimelnk')
	env['KDEDOC']   = env.join( env['PREFIX'], 'share/doc')
	env['KDESERV']   = env.join( env['PREFIX'], 'share/services')
	env['KDELOCALE']   = env.join( env['PREFIX'], 'share/locale')
	env['KDESERVTYPES']   = env.join( env['PREFIX'], 'share/servicetypes')
	
	env['KDELIB']    = env.join( env['PREFIX'], 'lib')
	env['KDECONF']    = env.join( env['PREFIX'], 'share/config')
	env['KDEMODULE'] = env.join( env['PREFIX'], 'lib/kde4')

	env['KCONFIGCOMPILER']= env.join(os.getcwd(), env['_BUILDDIR_'], 'kdecore/kconfig_compiler/kconfig_compiler')
	env['DCOPIDL']        = env.join(os.getcwd(), 'dcop/dcopidlng/dcopidl --srcdir dcop/dcopidlng')
	env['DCOPIDL2CPP']    = env.join(os.getcwd(), env['_BUILDDIR_'], 'dcop/dcopidl2cpp/dcopidl2cpp')

	env['LIB_KDECORE'] = ['kdecore']
	env['LIB_KIO']     = ['kio']
	env['LIB_KDEUI']   = ['kdeui']

	env['LIBPATH_KDECORE'] = [ env.join(os.getcwd(), env['_BUILDDIR_'], 'kdecore') ]
	env['LIBPATH_KIO']     = [ env.join(os.getcwd(), env['_BUILDDIR_'], 'kio') ]
	env['LIBPATH_KDEUI']   = [ env.join(os.getcwd(), env['_BUILDDIR_'], 'kdeui') ]

	env['CPPPATH_KDECORE'] = [ env.join(os.getcwd(), env['_BUILDDIR_'], 'kdecore'), '#kdecore' ]
	env['CPPPATH_KIO']     = [ env.join(os.getcwd(), env['_BUILDDIR_'], 'kio'), '#kio' ]
	env['CPPPATH_KDEUI']   = [ env.join(os.getcwd(), env['_BUILDDIR_'], 'kdeui'), '#kdeui' ]

def detect(env):
	"""
	def getpath(varname):
		if not env.has_key('ARGS'): return None
		v=env['ARGS'].get(varname, None)
		if v : v=os.path.abspath(v)
		return v

	prefix		= getpath('prefix')
	execprefix	= getpath('execprefix')
	datadir		= getpath('datadir')
	libdir		= getpath('libdir')
	qtincludes	= getpath('qtincludes')
	qtlibs		= getpath('qtlibs')
	libsuffix	= ''
	if env.has_key('ARGS'): libsuffix=env['ARGS'].get('libsuffix', '')
	"""

	if env.has_key('BOOTSTRAP_KDE'):
		bootstrap(env)
		return

	# normal detection functions go here

