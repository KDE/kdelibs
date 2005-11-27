# Copyright Matt Rogers 2005
# BSD license (see COPYING)
def exists(env):
	return true

def generate(env):

	def Check_libtiff(context):
		import SCons.Util
		#context.Message('Checking for libtiff... ')
		ret = 0
		context.Result(ret)
		return ret

	from SCons.Options import Options
	import os
	
	optionFile = env['CACHEDIR'] + 'libtiff.cache.py'
	opts = Options(optionFile)
	opts.AddOptions(
		('CACHED_TIFF', 'Whether libtiff is available'),
		('CXXFLAGS_TIFF',''),
		('CCFLAGS_TIFF',''),
		('LINKFLAGS_TIFF',''),
		('CPPPATH_TIFF',''),
		('LIB_TIFF',''),
		('LIBPATH_TIFF','')
		)
	opts.Update(env)
	opts.Save(optionFile, env)

	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('CACHED_TIFF')):
		conf = env.Configure(custom_tests =	 { 'Check_libtiff' : Check_libtiff } )
		env['CACHED_TIFF'] = 0
		if env['WINDOWS']:
			conf.Check_libtiff()
		else:	
			if conf.CheckHeader('tiff.h') and conf.CheckLib('tiff'):
				env['LINKFLAGS_TIFF'] = ['-ltiff']
				env['CACHED_TIFF'] = 1
				ret = 1
			
			dest = open(env.join(env['_BUILDDIR_'], 'config-tiff.h'), 'w')
			dest.write('/* libtiff configuration created by bksys */\n')
			if env['CACHED_TIFF']:
				dest.write('#define HAVE_LIBTIFF 1\n')
			dest.close()

			env['_CONFIG_H_'].append('tiff')

		conf.Finish()
			
		opts.Save(optionFile, env)
