# Copyright Thomas Nagy 2005
# Copyright Ralf Habacker 2005
# BSD license (see COPYING)
def exists(env):
	return true

def generate(env):

	from SCons.Options import Options
	from SCons.Tool import Tool
	import os

	optionFile = env['CACHEDIR'] + 'libpng.cache.py'
	opts = Options(optionFile)
	opts.AddOptions(
		('CACHED_PNG', 'Whether libpng is available'),
		('CXXFLAGS_PNG',''),
		('CCFLAGS_PNG',''),
		('LINKFLAGS_PNG',''),
		('CPPPATH_PNG',''),
		('INCLUDES_PNG',''),
		('LIB_PNG',''),
		('LIBPATH_PNG',''),
		)
	opts.Update(env)

	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('CACHED_PNG')):
		pkgs = Tool('pkgconfig', ['./bksys'])
		pkgs.generate(env)

		have_lib = 0
		if env['HAVE_PKGCONFIG'] == 0:
			conf = env.Configure()
			if conf.CheckHeader('png.h'):
				if conf.CheckLib('png'):
					conf.env['INCLUDES_Z'] = [ 'png.h' ]
					conf.env['LIB_Z']      = [ 'png' ]
					have_lib = 1
				elif conf.CheckLib('libpng'):
					conf.env['INCLUDES_Z'] = [ 'png.h' ]
					conf.env['LIB_Z']      = [ 'libpng' ]
					have_lib = 1
			env = conf.Finish()
		else:
			have_lib = env.pkgConfig_findPackage('PNG', 'libpng', '1.0')

		env.write_lib_header( 'libpng', have_lib, False )

	opts.Save(optionFile, env)
