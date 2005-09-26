# Copyright Thomas Nagy 2005
# BSD license (see COPYING)
def exists(env):
	return true

def generate(env):

        from SCons.Options import Options
        import os

        optionFile = env['CACHEDIR'] + 'fam.cache.py'
        opts = Options(optionFile)
        opts.AddOptions(
		('FAM_ISCONFIGURED', 'If FAM is already tested for'),
		('CXXFLAGS_FAM',''),
		('LINKFLAGS_FAM',''),
		)
        opts.Update(env)
	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('FAM_ISCONFIGURED')):
	    conf = env.Configure( )
	    if conf.CheckCXXHeader('fam.h') and conf.CheckLib('fam','FAMOpen'):
		env['FAM_ISCONFIGURED'] = 1;
		env['CXXFLAGS_FAM']=' -DHAVE_FAM '
		env['LINKFLAGS_FAM'] = ' -lfam'
	    else: print 'FAM not found.'

	    env = conf.Finish()
	    opts.Save(optionFile, env)
    
