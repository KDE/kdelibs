# Copyright Benjamin Reed 2005
# BSD license (see COPYING)
def exists(env):
	return true

def generate(env):

	from SCons.Options import Options
	import os

	optionFile = env['CACHEDIR'] + 'libbz2.cache.py'
	opts = Options(optionFile)
	opts.AddOptions(
		('CACHED_LIBBZ2', 'Whether the bzip2 library is available'),
		('CXXFLAGS_LIBBZ2',''),
		('LINKFLAGS_LIBBZ2',''),
		)
	opts.Update(env)

	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('CACHED_LIBBZ2')):
		conf = env.Configure()

		env['CACHED_LIBBZ2'] = 0
		if conf.CheckHeader('bzlib.h') and conf.CheckLib('bz2'):
			env['CXXFLAGS_LIBBZ2'] = ['-DHAVE_LIBBZ2']
			env['LINKFLAGS_LIBBZ2'] = ['-lbz2']
			env['CACHED_LIBBZ2'] = 1

		dest=open(env.join(env['_BUILDDIR_'], 'config-bzip2.h'), 'w')
		dest.write('/* bzip2 configuration created by bksys */\n')

		if env['CACHED_LIBBZ2']:
			dest.write('#define HAVE_BZIP2_SUPPORT 1\n');
		else:
			print 'libbz2 not found.'

		dest.close()
		env['_CONFIG_H_'].append('bzip2')

		env = conf.Finish()

		opts.Save(optionFile, env)

