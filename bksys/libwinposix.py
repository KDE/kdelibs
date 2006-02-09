# Copyright 2006 Christian Ehrlicher
# BSD license (see COPYING)

"""
What to do here?
- check for Qt to get qmake.exe
- call qmake to create correct Makefile
- execute Makefile
- add winposix-lib to the buildsystem

Notes
- could be used in uselib attribute 
- include/library settings are added for the following configure process and the build process
"""


def exists(env):
	return true

def execute(env, command):
	ret = env.Execute( command )
	if ret:
		env.pprint('RED', 'Error executing \'' + command + ' (ret: ' + str(ret) + ')'  )
		env.Exit(1)

def generate(env):
	import os
	from SCons.Options import Options

	if not env['WINDOWS']:
	    return

	optionFile = env['CACHEDIR'] + 'libwinposix.cache.py'
	opts = Options(optionFile)
	opts.AddOptions(
		('CACHED_WINPOSIX', 'Whether libwinposix is available'),
		('CPPPATH_WINPOSIX',''),
		('LIBPATH_WINPOSIX',''),
		('LIB_WINPOSIX',''),
		)
	opts.Update(env)

	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('CACHED_WINPOSIX')):
		p=env.pprint
		winposixdir = 'win'
		winposixpro = 'win.pro'
		# we need Qt4
		from SCons.Tool import Tool
		Tool('qt4', ['./bksys']).generate(env)

		if not env.has_key ('QT_QMAKE'):
		    p('RED', "qmake.exe wasn't found - can't compile winposix lib!")
		    env.Exit(1)

		#could maybe optimized
		if env['CC'] == 'cl':
		    make = 'nmake'
		    spec = '-spec win32-msvc2005'
		elif env['CC'] == 'gcc':
		    make = 'mingw32-make'
		    spec = '-spec win32-g++'
		else:
			p('RED', 'Wrong compiler ' + env['COMPILERTOOL'] )
			env.Exit(1)
		if os.environ.has_key('QMAKESPEC'):
			spec = ''
		# create Makefile
		execute( env, env['QT_QMAKE'] + ' ' + winposixdir + '/' + winposixpro + ' ' + spec )
		# create libwinposix
		execute( env, 'cd ' + winposixdir + ' && ' + make + ' -f Makefile' )
		# 'install' it
		execute( env, 'cd ' + winposixdir + ' && ' + make + ' -f Makefile install' )

		# add the lib & include path to the standard libs
		if env['ARGS'].get('debug', None) != None:
			winposixlib = 'kdewin32d'
		else:
			winposixlib = 'kdewin32'

		if env['CC'] == 'cl':
			env['CPPPATH_WINPOSIX']  = [ env.join(os.getcwd(),winposixdir,'include'),env.join(os.getcwd(),winposixdir,'include','msvc')]
			env['LIBPATH_WINPOSIX']   = [ env.join(os.getcwd(),winposixdir,'lib')]
			env['LIB_WINPOSIX']       = [ winposixlib,'ws2_32' ]
		elif env['CC'] == 'gcc':
			env['CPPPATH_WINPOSIX']  = [ env.join(os.getcwd(),winposixdir,'include'),env.join(os.getcwd(),winposixdir,'include','mingw')]
			env['LIBPATH_WINPOSIX']   = [ env.join(os.getcwd(),winposixdir,'lib')]
			env['LIB_WINPOSIX']       = [ winposixlib,'ws2_32']

		env['CACHED_WINPOSIX'] = 1
		opts.Save(optionFile, env)

	# add winposix to environment by default 
	if env.has_key('CACHED_WINPOSIX') and env['CACHED_WINPOSIX']:
		env.AppendUnique( CPPPATH = env['CPPPATH_WINPOSIX'] )
		env.AppendUnique( LIBPATH = env['LIBPATH_WINPOSIX'] )
		env.AppendUnique( LIBS = env['LIB_WINPOSIX'] )