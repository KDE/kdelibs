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
				if env.has_key('LIB_QTCORE'):
					env['LIB_Z'] = env['LIB_QTCORE']
				else:
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
		('LIBPATH_Z','')
		)
	opts.Update(env)
	opts.Save(optionFile, env)

	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('CACHED_Z')):
		conf = env.Configure(custom_tests =	 { 'Check_libz' : Check_libz} )

		if env['WINDOWS'] and not conf.Check_libz():
			print 'libz not found (mandatory).'
		else:
			env['CACHED_Z'] = 0
			if conf.CheckHeader('zlib.h') and conf.CheckLib('z'):
				env['LINKFLAGS_Z'] = ['-lz']
				env['CACHED_Z'] = 1

			dest=open(env.join(env['_BUILDDIR_'], 'config-z.h'), 'w')
			dest.write('/* libz configuration created by bksys */\n')

			if env['CACHED_Z']:
				dest.write('#define HAVE_LIBZ 1\n');
			else:
				print 'libz not found (mandatory).'

			dest.close()
			env['_CONFIG_H_'].append('z')

			env = conf.Finish()

			opts.Save(optionFile, env)
			
		opts.Save(optionFile, env)
