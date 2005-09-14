#! /usr/bin/env python

def exists(env):
	return True

def generate(env):

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

