# Thomas Nagy, 2005

import os
import sys

def bootstrap(env):

	#print "bootstrapping kdelibs"

	########### KDE4
	env['LIBPATH_KDE4'] = env['LIBPATH_QT']

	## not very portable but that's the best i have at the moment (ITA)
	if env['_IN_KDELIBS_']:
		includes=['.','dcop','kio','kio\\kio','kio\\kfile','kdeui','kdecore','libltdl','kdefx']

		env['INCLUDES_KDE4']=['#build']
		for dir in includes:
			env['INCLUDES_KDE4'].append('#'+dir)
			env['INCLUDES_KDE4'].append('#build\\'+dir)

		# prefix/lib and prefix/lib/kde4 (ita)
	#	env['RPATH_KDE4']= [
	#	env['QTLIBPATH'], env.join(env['PREFIX'], 'lib'), env.join(env['PREFIX'], 'lib', 'kde4')
	#]

	#######################################
	## install paths
	env['KDEBIN']    = env.join( env['PREFIX'], 'bin')
	env['KDEDATA']   = env.join( env['PREFIX'], 'share')
	env['KDEAPPS']   = env.join( env['PREFIX'], 'share\\apps')
	env['KDEKCFG']   = env.join( env['PREFIX'], 'share\\config.kcfg')
	env['KDEICONS']  = env.join( env['PREFIX'], 'share\\icons')
	env['KDEMIME']   = env.join( env['PREFIX'], 'share\\mimelnk')
	env['KDEDOC']    = env.join( env['PREFIX'], 'share\\doc')
	env['KDESERV']   = env.join( env['PREFIX'], 'share\\services')
	env['KDELOCALE'] = env.join( env['PREFIX'], 'share\\locale')
	env['KDESERVTYPES'] = env.join( env['PREFIX'], 'share\\servicetypes')
	env['KDEINCLUDE']= env.join( env['PREFIX'], 'include')
	
	env['KDELIB']    = env.join( env['PREFIX'], 'lib')
	env['KDECONF']    = env.join( env['PREFIX'], 'share\\config')
	env['KDEMODULE'] = env.join( env['PREFIX'], 'lib\\kde4')

	if env['_IN_KDELIBS_']:
		env['KCONFIGCOMPILER']= env.join(os.getcwd(), env['_BUILDDIR_'], 'kdecore\\kconfig_compiler\\kconfig_compiler.exe')
		env['DCOPIDL']        = env.join(os.getcwd(), 'dcop\\dcopidlng\\dcopidl --srcdir dcop\\dcopidlng')
		env['DCOPIDL2CPP']    = env.join(os.getcwd(), env['_BUILDDIR_'], 'dcop\\dcopidl2cpp\\dcopidl2cpp.exe')
	else:
		env['KCONFIGCOMPILER']= env.join(env['_KDEDIR_'],'bin','kconfig_compiler.exe')
		env['DCOPIDL']        = env.join(env['_KDEDIR_'],'bin','dcopidl.bat')  + ' --srcdir ' + env.join(env['_KDEDIR_'],'share','dcopidl')
		env['DCOPIDL2CPP']    = env.join(env['_KDEDIR_'],'bin','copidl2cpp.exe')
	
	env['LIB_KDECORE'] = ['kdecore']
	env['LIB_KIO']     = ['kio', 'kwalletclient']  # TODO: kdesu isn't supported yet
	env['LIB_KDEUI']   = ['kdeui']

	if env['_IN_KDELIBS_']:
		env['LIBPATH_KDECORE'] = [ env.join(os.getcwd(), env['_BUILDDIR_'], 'kdecore') ]
		env['LIBPATH_KIO']     = [ env.join(os.getcwd(), env['_BUILDDIR_'], 'kio') ]
		env['LIBPATH_KIO'].append( env.join(os.getcwd(), env['_BUILDDIR_'], 'kdesu'))
		env['LIBPATH_KIO'].append( env.join(os.getcwd(), env['_BUILDDIR_'], 'kwallet', 'client'))
		env['LIBPATH_KDEUI']   = [ env.join(os.getcwd(), env['_BUILDDIR_'], 'kdeui') ]
		env['CPPPATH_KDECORE'] = [ env.join(os.getcwd(), env['_BUILDDIR_'], 'kdecore'), '#kdecore' ]
		env['CPPPATH_KIO']     = [ env.join(os.getcwd(), env['_BUILDDIR_'], 'kio'), '#kio' ]
		env['CPPPATH_KDEUI']   = [ env.join(os.getcwd(), env['_BUILDDIR_'], 'kdeui'), '#kdeui' ]
	else:
		env['LIBPATH_KDE4']   += [ env.join(env['_KDEDIR_'], 'lib') ]
		env['LIBPATH_KDECORE'] = []
		env['LIBPATH_KIO']     = []
		env['LIBPATH_KDEUI']   = []
		env['INCLUDES_KDE4']   = [ env.join(env['_KDEDIR_'], 'include') ]
		env['CPPPATH_KDECORE'] = []
		env['CPPPATH_KIO']     = []
		env['CPPPATH_KDEUI']   = []
	
## detect kde4 configuration
def detect(env):
	def getpath(varname):
		if not env.has_key('ARGS'): return None
		v=env['ARGS'].get(varname, None)
		if v : v=os.path.abspath(v)
		return v

	kdedir		= getpath('kdedir')
	if kdedir:
		env['_KDEDIR_'] = kdedir
		env['_IN_KDELIBS_'] = 0
	else:
		env['_IN_KDELIBS_'] = 1

	"""
	prefix		= getpath('prefix')
	execprefix	= getpath('execprefix')
	datadir		= getpath('datadir')
	libdir		= getpath('libdir')
	qtincludes	= getpath('qtincludes')
	qtlibs		= getpath('qtlibs')
	libsuffix	= ''
	if env.has_key('ARGS'): libsuffix=env['ARGS'].get('libsuffix', '')
	"""

	# normal detection functions go here
	if env['CC'] == 'cl':
		if env['_IN_KDELIBS_']:
			env.AppendUnique(KDECCFLAGS = ['/FI./win/include/kdelibs_global_win.h'])
			env.AppendUnique(KDECXXFLAGS = ['/FI./win/include/kdelibs_global_win.h'])
		env.AppendUnique(KDECCFLAGS = ['-DKDE_FULL_TEMPLATE_EXPORT_INSTANTIATION'])
		env.AppendUnique(KDECXXFLAGS = ['-DKDE_FULL_TEMPLATE_EXPORT_INSTANTIATION'])

	if env.has_key('BOOTSTRAP_KDE'):
		bootstrap(env)
		return
