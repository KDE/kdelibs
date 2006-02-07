# Copyright Benjamin Reed 2005
# BSD license (see COPYING)
def exists(env):
	return true

def generate(env):

	from SCons.Options import Options
	from SCons.Tool import Tool
	import os

	optionFile = env['CACHEDIR'] + 'libgif.cache.py'
	opts = Options(optionFile)
	opts.AddOptions(
		('CACHED_GIF', 'Whether libgif is available'),
		('CXXFLAGS_GIF',''),
		('CCFLAGS_GIF',''),
		('LINKFLAGS_GIF',''),
		('CPPPATH_GIF',''),
		('INCLUDES_GIF',''),
		('LIB_GIF',''),
		('LIBPATH_GIF',''),
		)
	opts.Update(env)

	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('CACHED_GIF')):
		pkgs = Tool('pkgconfig', ['./bksys'])
		pkgs.generate(env)

		have_lib = 0
		if env['HAVE_PKGCONFIG'] == 0:
			conf = env.Configure()
			if conf.CheckHeader('gif_lib.h'):
				if conf.CheckLib('gif'):
					conf.env['INCLUDES_GIF'] = [ 'gif_lib.h' ]
					conf.env['LIB_GIF']      = [ 'gif' ]
					have_lib = 1
				elif conf.CheckLib('giflib'):
					conf.env['INCLUDES_GIF'] = [ 'gif_lib.h' ]
					conf.env['LIB_GIF']      = [ 'giflib' ]
					have_lib = 1
				if conf.CheckLib('ungif'):
					conf.env['INCLUDES_GIF'] = [ 'gif_lib.h' ]
					conf.env['LIB_GIF']      = [ 'ungif' ]
					have_lib = 1
				elif conf.CheckLib('libungif'):
					conf.env['INCLUDES_GIF'] = [ 'gif_lib.h' ]
					conf.env['LIB_GIF']      = [ 'libungif' ]
					have_lib = 1
			env = conf.Finish()
		else:
			have_lib = env.pkgConfig_findPackage('GIF', 'libgif', '4.0')
			if not have_lib:
				have_lib = env.pkgConfig_findPackage('GIF', 'libungif', '4.0')

		env.write_lib_header( 'libgif', have_lib, False )

		opts.Save(optionFile, env)

