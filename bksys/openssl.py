# Copyright liucougar 2005
# BSD license (see COPYING)

def exists(env):
	return true

def generate(env):

	from SCons.Options import Options
	from SCons.Tool import Tool
	import os

	optionFile = env['CACHEDIR'] + 'openssl.cache.py'
	opts = Options(optionFile)
	opts.AddOptions(
		('CACHED_OPENSSL', 'Whether openssl is available'),
		('CXXFLAGS_OPENSSL',''),
		('CCFLAGS_OPENSSL',''),
		('LINKFLAGS_OPENSSL',''),
		('CPPPATH_OPENSSL',''),
		('INCLUDES_OPENSSL',''),
		('LIB_OPENSSL',''),
		('LIBPATH_OPENSSL','')
		)
	opts.Update(env)

	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('CACHED_OPENSSL')):
		pkgs = Tool('pkgconfig', ['./bksys'])
		pkgs.generate(env)

		have_lib = 0
		if env['HAVE_PKGCONFIG'] == 0:
			conf = env.Configure()
			if conf.CheckHeader('opensslconf.h'):
				if conf.CheckLib('eay32'):
					conf.env['INCLUDES_OPENSSL'] = ['']
					conf.env['LIB_OPENSSL']      = ['eay32']
					have_lib = 1
				elif conf.CheckLib('libeay32'):
					conf.env['INCLUDES_OPENSSL'] = ['']
					conf.env['LIB_OPENSSL']      = ['libeay32']
					have_lib = 1
			env = conf.Finish()
		else:
			have_lib = env.pkgConfig_findPackage('OPENSSL', 'openssl', '0.9.6')

		env.write_lib_header( 'openssl', have_lib, False, 'ksslconfig.h', '#define KSSL_HAVE_SSL 1\n' )
# using another header name than config-%s.h doesn't work because of env.write_config_h()
		env.write_lib_header( 'openssl', have_lib, False )

	opts.Save(optionFile, env)
