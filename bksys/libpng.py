# Copyright Thomas Nagy 2005
# Copyright Ralf Habacker 2005
# BSD license (see COPYING)
def exists(env):
	return true

def generate(env):

	def Check_libpng(context):
		import SCons.Util
		context.Message('Checking for libpng... ')
		ret = 0

		if env['WINDOWS']:
			qtdir = os.getenv("QTDIR")
			if qtdir: 
				# TODO: check if libpng is really in QTCore lib 
				# TODO: png lib requires zlib, so add it by default, this should be better solved by specifing a dependency 
				env['CPPPATH_PNG'] = [qtdir + "\\src\\3rdparty\\libpng",qtdir + "\\src\\3rdparty\\zlib"]
				env['LIBPATH_PNG'] = [qtdir + "\\lib"]
				env['LIB_PNG'] = ['QtCore4'] # TODO: check if debug version is used 
				env['CACHED_PNG'] = 1
				ret = 1

		context.Result(ret)
		return ret

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
		('LIB_PNG',''),
		('LIBPATH_PNG',''),
		)
	opts.Update(env)
	opts.Save(optionFile, env)

	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('CACHED_PNG')):
		if env['WINDOWS']:
			conf = env.Configure(custom_tests =	 { 'Check_libpng' : Check_libpng} )
			have_png = conf.Check_libpng()
			if not have_png:
				print 'libpng not found (mandatory).'
			env = conf.Finish()
		else:
			env['CACHED_PNG'] = 0
			pkgs = Tool('pkgconfig', ['./bksys'])
			pkgs.generate(env)

			have_png = env.pkgConfig_findPackage('PNG', 'libpng', '1.0')

    		dest=open(env.join(env['_BUILDDIR_'], 'config-libpng.h'), 'w')
    		dest.write('/* libpng configuration created by bksys */\n')
		if have_png:
			dest.write('#define HAVE_LIBPNG_H 1\n');
			env['CACHED_PNG'] = 1
		dest.close()
		env['_CONFIG_H_'].append('libpng')
		opts.Save(optionFile, env)
