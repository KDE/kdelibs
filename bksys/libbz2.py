# Copyright Benjamin Reed 2005
# BSD license (see COPYING)
def exists(env):
	return true

def generate(env):

	from SCons.Options import Options
	from SCons.Tool import Tool
	import os

	optionFile = env['CACHEDIR'] + 'libbz2.cache.py'
	opts = Options(optionFile)
	opts.AddOptions(
		('CACHED_BZ2', 'Whether libbzip2 is available'),
		('CXXFLAGS_BZ2',''),
		('CCFLAGS_BZ2',''),
		('LINKFLAGS_BZ2',''),
		('CPPPATH_BZ2',''),
		('INCLUDES_BZ2',''),
		('LIB_BZ2',''),
		('LIBPATH_BZ2',''),
		)
	opts.Update(env)

	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('CACHED_BZ2')):
		pkgs = Tool('pkgconfig', ['./bksys'])
		pkgs.generate(env)

		have_lib = 0
# bzip2 has no pkgconfig support
#		if env['HAVE_PKGCONFIG'] == 0:
		if 1:
			conf = env.Configure()
			if conf.CheckHeader('bzlib.h'):
				if conf.CheckLib('bz2'):
					conf.env['INCLUDES_BZ2'] = [ 'bzlib.h' ]
					conf.env['LIB_BZ2']      = [ 'bz2' ]
					have_lib = 1
				if conf.CheckLib('bzip2'):
					conf.env['INCLUDES_BZ2'] = [ 'bzlib.h' ]
					conf.env['LIB_BZ2']      = [ 'bzip2' ]
					have_lib = 1
			env = conf.Finish()
		else:
			have_lib = env.pkgConfig_findPackage('BZ2', 'bz2', '1.0')

		env.write_lib_header( 'libbz2', have_lib, False )

	opts.Save(optionFile, env)

