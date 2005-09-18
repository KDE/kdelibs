# Thomas Nagy, 2005

#import os

def bootstrap(env):

	#print "bootstrapping kdelibs"

	########### KDE4
	env['LIBPATH_KDE4'] = env['LIBPATH_QT']

	## not very portable but that's the best i have at the moment (ITA)
	includes=['.','dcop','kio','kio/kio','kio/kfile','kdeui','kdecore','libltdl','kdefx']
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
	env['KDEBIN']='/usr/bin'
	env['KDEDATA']='/usr/share'
	env['KDEMIME']='/usr/share/mimelnk'

	env['DCOPIDL']='build/dcop/dcopidl/dcopidlold'
	env['DCOPIDL2CPP']='build/dcop/dcopidl2cpp/dcopidl2cpp'

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

