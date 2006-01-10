# Copyright Matt Rogers 2005
# BSD license (see COPYING)
def exists(env):
	return true

def generate(env):

	from SCons.Options import Options
	from SCons.Tool import Tool
	import os
	
	optionFile = env['CACHEDIR'] + 'libtiff.cache.py'
	opts = Options(optionFile)
	opts.AddOptions(
		('CACHED_TIFF', 'Whether libtiff is available'),
		('CXXFLAGS_TIFF',''),
		('CCFLAGS_TIFF',''),
		('LINKFLAGS_TIFF',''),
		('CPPPATH_TIFF',''),
		('INCLUDES_TIFF',''),
		('LIB_TIFF',''),
		('LIBPATH_TIFF','')
		)
	opts.Update(env)

	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('CACHED_TIFF')):
		pkgs = Tool('pkgconfig', ['./bksys'])
		pkgs.generate(env)

		have_lib = 0
# tiff has no pkgconfig support
#		if env['HAVE_PKGCONFIG'] == 0:
		if 1:
			conf = env.Configure()
			if conf.CheckHeader('tiff.h'):
				if conf.CheckLib('tiff'):
					conf.env['INCLUDES_TIFF'] = [ 'tiff.h' ]
					conf.env['LIB_TIFF']      = [ 'tiff' ]
					have_lib = 1
				elif conf.CheckLib('libtiff'):
					conf.env['INCLUDES_TIFF'] = [ 'tiff.h' ]
					conf.env['LIB_TIFF']      = [ 'libtiff' ]
					have_lib = 1
			env = conf.Finish()
		else:
			have_lib = env.pkgConfig_findPackage('TIFF', 'tiff', '1.0')

		env.write_lib_header( 'libtiff', have_lib, False )

	opts.Save(optionFile, env)
