# Copyright Thomas Nagy 2005
# Copyright Ralf Habacker 2005
# BSD license (see COPYING)
def exists(env):
	return true

def generate(env):

	from SCons.Options import Options
	import os

	optionFile = env['CACHEDIR'] + 'libagg.cache.py'
	opts = Options(optionFile)
	opts.AddOptions(
		('CACHED_AGG','Whether libagg is available'),
		('LINKFLAGS_AGG',''),
		('LIB_AGG','')
		# other options are added by the pkgConfig_findPackage() call
		# apparently not, because the above LINKFLAGS_AGG line had to be added
		# to make things work when attempting to use agg on linux. (mattr)
		)
	opts.Update(env)
	opts.Save(optionFile, env)

	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('CACHED_AGG')):

		env['CACHED_AGG'] = 0

		if env['WINDOWS']:
			conf = env.Configure()
			have_agg = conf.CheckLib('agg')
			if have_agg:
				env['LIB_AGG'] = ['agg']
				env['CACHED_AGG'] = 1
			env = conf.Finish()
		else:
			from SCons.Tool import Tool
			pkgs = Tool('pkgconfig', ['./bksys'])
			pkgs.generate(env)
			have_agg = env.pkgConfig_findPackage('AGG', 'libagg', '2.3')
			# env['xxx_AGG'] options are set now

		if have_agg:
			dest=open(env.join(env['_BUILDDIR_'], 'config-libagg.h'), 'w')
			dest.write('/* libagg configuration created by bksys */\n')
			dest.write('#define HAVE_LIBAGG 1\n');
			env['CACHED_AGG'] = 1
			dest.close()
			env['_CONFIG_H_'].append('libagg')
		else:
			env.pprint('RED', 'libagg >= 2.3 not found.')

	
	opts.Update(env)
	opts.Save(optionFile, env)
