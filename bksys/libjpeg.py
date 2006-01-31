# Copyright Thomas Nagy 2005
# Copyright Ralf Habacker 2005
# BSD license (see COPYING)
def exists(env):
	return true

def generate(env):

	from SCons.Options import Options
	from SCons.Tool import Tool
	import os

	optionFile = env['CACHEDIR'] + 'libjpeg.cache.py'
	opts = Options(optionFile)
	opts.AddOptions(
		('CACHED_JPEG', 'Whether libjpeg is available'),
		('CXXFLAGS_JPEG',''),
		('CCFLAGS_JPEG',''),
		('LINKFLAGS_JPEG',''),
		('CPPPATH_JPEG',''),
		('INCLUDES_JPEG',''),
		('LIB_JPEG',''),
		('LIBPATH_JPEG',''),
		)
	opts.Update(env)

	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('CACHED_JPEG')):
		pkgs = Tool('pkgconfig', ['./bksys'])
		pkgs.generate(env)

		have_lib = 0
# jpeg has no pkgconfig support
#		if env['HAVE_PKGCONFIG'] == 0:
		if 1:
	 		conf = env.Configure()
			if conf.CheckHeader(['stdio.h','jpeglib.h']):
				if conf.CheckLib('jpeg'):
					conf.env['INCLUDES_JPEG'] = [ 'jpeglib.h' ]
					conf.env['LIB_JPEG']      = [ 'jpeg' ]
					have_lib = 1
				elif conf.CheckLib('jpeg6b'):
					conf.env['INCLUDES_JPEG '] = [ 'jpeglib.h' ]
					conf.env['LIB_JPEG ']      = [ 'jpeg6b' ]
					have_lib = 1
			env = conf.Finish()
		else:
			have_lib = env.pkgConfig_findPackage('JPEG', 'jpeg', '1.0')

		env.write_lib_header( 'libjpeg', have_lib, False )

		opts.Save(optionFile, env)
	
