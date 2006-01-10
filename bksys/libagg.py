# Copyright Thomas Nagy 2005
# Copyright Ralf Habacker 2005
# BSD license (see COPYING)
def exists(env):
	return true

def generate(env):

	from SCons.Options import Options
	from SCons.Tool import Tool
	import os

	optionFile = env['CACHEDIR'] + 'libagg.cache.py'
	opts = Options(optionFile)
	opts.AddOptions(
		('CACHED_AGG', 'Whether libagg is available'),
		('CXXFLAGS_AGG',''),
		('CCFLAGS_AGG',''),
		('LINKFLAGS_AGG',''),
		('CPPPATH_AGG',''),
		('INCLUDES_AGG',''),
		('LIB_AGG',''),
		('LIBPATH_AGG',''),
		)
	opts.Update(env)

	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('CACHED_AGG')):
		pkgs = Tool('pkgconfig', ['./bksys'])
		pkgs.generate(env)

		have_lib = 0
		if env['HAVE_PKGCONFIG'] == 0:
			conf = env.Configure()
			if conf.CheckLib('agg'):
				conf.env['INCLUDES_AGG'] = [ '' ]
				conf.env['LIB_AGG']      = [ 'agg' ]
				have_lib = 1
			env = conf.Finish()
		else:
			have_lib = env.pkgConfig_findPackage('AGG', 'libagg', '2.3')

		env.write_lib_header( 'libagg', have_lib, False )

	opts.Save(optionFile, env)
