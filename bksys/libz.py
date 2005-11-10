# Copyright Thomas Nagy 2005
# Copyright Ralf Habacker 2005
# BSD license (see COPYING)
def exists(env):
	return true

def generate(env):

	def Check_libz(context):
		import SCons.Util
		context.Message('Checking for libz... ')
		ret = 0

		if env['WINDOWS']:
			qtdir = os.getenv("QTDIR")
			if qtdir: 
				# TODO: check if libz is really in QTCore lib 
				env['CPPPATH_Z'] = [qtdir + "\\src\\3rdparty\\zlib"]
				env['LIBPATH_Z'] = [qtdir + "\\lib"]
				env['LIB_Z'] = ['QtCore4'] # TODO: check if debug version is used 
				env['CACHED_Z'] = 1
				ret = 1

		context.Result(ret)
		return ret

	from SCons.Options import Options
	import os

	optionFile = env['CACHEDIR'] + 'libz.cache.py'
	opts = Options(optionFile)
	opts.AddOptions(
		('CACHED_Z', 'Whether libz is available'),
		('CXXFLAGS_Z',''),
		('CCFLAGS_Z',''),
		('LINKFLAGS_Z',''),
		('CPPPATH_Z',''),
		('LIB_Z',''),
		('LIBPATH_Z',''),
		)
	opts.Update(env)
	opts.Save(optionFile, env)

	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('CACHED_Z')):
		conf = env.Configure(custom_tests =	 { 'Check_libz' : Check_libz} )

		if not conf.Check_libz():
			print 'libz not found (mandatory).'

		env = conf.Finish()
		opts.Save(optionFile, env)
