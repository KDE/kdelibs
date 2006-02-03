# Copyright 2006 Christian Ehrlicher
# BSD license (see COPYING)

"""
What to do here?
- check for Qt to get qmake.exe
- call qmake to create correct Makefile
- execute Makefile
- add winposix-lib to the buildsystem
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
		('CACHED_LIBWINPOSIX', 'Whether libwinposix is available'),
		('CCFLAGS_LIBWINPOSIX',''),
		('CXXFLAGS_LIBWINPOSIX',''),
		('LFLAGS_LIBWINPOSIX',''),
		)
	opts.Update(env)

	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('CACHED_LIBWINPOSIX')):
		p=env.pprint
		winposixdir = 'win'
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
		execute( env, env['QT_QMAKE'] + ' ' + winposixdir + '/winposix.pro ' + spec )
		# create libwinposix
		execute( env, 'cd ' + winposixdir + ' && ' + make + ' -f Makefile' )
		# 'install' it
		execute( env, 'cd ' + winposixdir + ' && ' + make + ' -f Makefile install' )

		# add the lib & include path to the standard libs
		if env['ARGS'].get('debug', None) != None:
			winposixlib = 'kdewin32d.lib'
		else:
			winposixlib = 'kdewin32.lib'

		if env['CC'] == 'cl':
			env['CCFLAGS_LIBWINPOSIX']   = [ '/I'+winposixdir+'\\include','/I'+winposixdir+'\\include\\msvc' ]
			env['CXXFLAGS_LIBWINPOSIX']  = [ '/I'+winposixdir+'\\include','/I'+winposixdir+'\\include\\msvc' ]
			env['LFLAGS_LIBWINPOSIX']    = [ '/LIBPATH:'+winposixdir+'\\lib',winposixlib ]
		elif env['CC'] == 'gcc':
			env['CCFLAGS_LIBWINPOSIX']   = [ '-I'+winposixdir+'\\include','-I'+winposixdir+'\\include\\mingw' ]
			env['CXXFLAGS_LIBWINPOSIX']  = [ '-I'+winposixdir+'\\include','-I'+winposixdir+'\\include\\mingw' ]
			env['LFLAGS_LIBWINPOSIX']    = [ '-l'+winposixdir+'\\lib\\'+winposixlib ]

		env['CACHED_LIBWINPOSIX'] = 1
		opts.Save(optionFile, env)

	if env.has_key('CCFLAGS_LIBWINPOSIX'):
		env.AppendUnique( CPPFLAGS  = env['CCFLAGS_LIBWINPOSIX'] )
	if env.has_key('CXXFLAGS_LIBWINPOSIX'):
		env.AppendUnique( CCFLAGS   = env['CXXFLAGS_LIBWINPOSIX'] )
	if env.has_key('LFLAGS_LIBWINPOSIX'):
		env.AppendUnique( LINKFLAGS = env['LFLAGS_LIBWINPOSIX'] )
