# Copyright Thomas Nagy 2005
# Copyright Ralf Habacker 2005
# BSD license (see COPYING)
def exists(env):
	return true

def generate(env):

	from SCons.Options import Options
	from SCons.Tool import Tool
	import os
	
	optionFile = env['CACHEDIR'] + 'libz.cache.py'
	opts = Options(optionFile)
	opts.AddOptions(
		('CACHED_Z', 'Whether libz is available'),
		('CXXFLAGS_Z',''),
		('CCFLAGS_Z',''),
		('LINKFLAGS_Z',''),
		('CPPPATH_Z',''),
		('INCLUDES_Z',''),
		('LIB_Z',''),
		('LIBPATH_Z','')
		)
	opts.Update(env)

	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('CACHED_Z')):
		pkgs = Tool('pkgconfig', ['./bksys'])
		pkgs.generate(env)

		have_lib = 0
# zlib has no pkgconfig support
#		if env['HAVE_PKGCONFIG'] == 0:
		if 1:
			conf = env.Configure()
			if conf.CheckHeader('zlib.h'):
				if conf.CheckLib('z'):
					conf.env['INCLUDES_Z'] = ['zlib.h']
					conf.env['LIB_Z']      = ['z']
					have_lib = 1
				elif conf.CheckLib('zlib'):
					conf.env['INCLUDES_Z'] = ['zlib.h']
					conf.env['LIB_Z']      = ['zlib']
					have_lib = 1
			env = conf.Finish()
		else:
			have_lib = env.pkgConfig_findPackage('Z', 'zlib', '1.0')

		env.write_lib_header( 'libz', have_lib, False )

		opts.Save(optionFile, env)
