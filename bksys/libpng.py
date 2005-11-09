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
				env['CPPPATH_PNG'] = [qtdir + "\\src\\3rdparty\\libpng"]
				env['LIBPATH_PNG'] = [qtdir + "\\lib"]
				env['LIB_PNG'] = ['QtCore4'] # TODO: check if debug version is used 
				env['CACHED_PNG'] = 1
				ret = 1

		context.Result(ret)
		return ret

	from SCons.Options import Options
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
		conf = env.Configure(custom_tests =	 { 'Check_libpng' : Check_libpng} )

		if not conf.Check_libpng():
			print 'libpng not found (mandatory).'

		env = conf.Finish()
		opts.Save(optionFile, env)
