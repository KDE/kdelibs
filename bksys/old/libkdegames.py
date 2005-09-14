import os.path

RED    ="\033[91m"
GREEN  ="\033[92m"
NORMAL ="\033[0m"

def exists(env):
	return true

def generate(env):
        from SCons.Options import Options

	optionfile = env['CACHEDIR']+'/libkdegames.cache.py'

        opts = Options(optionfile)
        opts.AddOptions(
		( 'LIBKDEGAMESINCLUDEPATH', 'path to libkdegames headers' ),
		( 'LIBKDEGAMESLIBPATH', 'path to libkdegames library' ),
		( 'ALREADY_CONFIGURED', 'configuration succeeded' )
        )
        opts.Update(env)

	if 'configure' in env['TARGS'] or not env.has_key('ALREADY_CONFIGURED'):
		print "Checking for libkdegames          : ",

		if not env.has_key('LIBKDEGAMESINCLUDEPATH') or \
		   not os.path.isfile( env['LIBKDEGAMESINCLUDEPATH'] + "/kstdgameaction.h" ):
			env['LIBKDEGAMESINCLUDEPATH'] = env['KDEINCLUDEPATH']
			if not os.path.isfile( env['LIBKDEGAMESINCLUDEPATH'] + "/kstdgameaction.h" ):
				print RED+"could not find headers!"+NORMAL
				import sys
				sys.exit( 1 )

		if not env.has_key('LIBKDEGAMESLIBPATH') or \
		   not os.path.isfile( env['LIBKDEGAMESLIBPATH'] + "/libkdegames.so" ):
			env['LIBKDEGAMESLIBPATH'] = env['KDELIBPATH']
			if not os.path.isfile( env['LIBKDEGAMESLIBPATH'] + "/libkdegames.so" ):
				print RED+"could not find library!"+NORMAL
				import sys
				sys.exit( 1 )

		env['ALREADY_CONFIGURED'] = 1

		opts.Save(optionfile, env)

		print GREEN+"found"+NORMAL

		env.AppendUnique(CPPPATH = env['LIBKDEGAMESINCLUDEPATH'])
		env.AppendUnique(LIBS = env['LIBKDEGAMESLIBPATH'])

#env.AppendUnique( LIBPATH = "-lkdegames" )

