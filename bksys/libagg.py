# Copyright Thomas Nagy 2005
# Copyright Ralf Habacker 2005
# BSD license (see COPYING)
def exists(env):
	return true

def generate(env):
	pkg='agg'
	pkgu=pkg.upper()

	def Check_libagg(context):
		import SCons.Util
		context.Message('Checking for lib'+pkg+'... ')
		ret = 0

		if env['WINDOWS']:
			if env['ARGS'] and env['ARGS'].has_key('with-lib'+pkg):
				pkgdir = env['ARGS']['with-lib'+pkg]
			else:
				# TODO set predefined value
				pkgdir = 'c:\Programme\agg23'

			# TODO check if agg is in any of given path 
			if os.path.isdir(pkgdir): 
				env['CPPPATH_' + pkgu] = [pkgdir + "\\include"]
				env['LIBPATH_' + pkgu] = [pkgdir + "\\src"]
				env['LIB_' + pkgu] = ['agg'] 
				env['CACHED_' + pkgu] = 1
				ret = 1
		
		#else:
		# TODO include pkgconfig check from ../kdecore/configure.in.in
			
		context.Result(ret)
		return ret

	from SCons.Options import Options
	import os

	optionFile = env['CACHEDIR'] + 'lib'+pkg+'.cache.py'
	opts = Options(optionFile)
	opts.AddOptions(
		('CACHED_' + pkgu, 'Whether lib'+pkg+' is available'),
		('CXXFLAGS_' + pkgu,''),
		('CCFLAGS_' + pkgu,''),
		('LINKFLAGS_' + pkgu,''),
		('CPPPATH_' + pkgu,''),
		('LIB_' + pkgu,''),
		('LIBPATH_' + pkgu,''),
		)
	opts.Update(env)
	opts.Save(optionFile, env)

	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('CACHED_' + pkgu)):
		conf = env.Configure(custom_tests =	 { 'Check_libagg' : Check_libagg} )

		conf.Check_libagg()

		env = conf.Finish()
		opts.Save(optionFile, env)
