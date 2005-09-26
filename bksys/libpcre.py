# Copyright Thomas Nagy 2005
# BSD license (see COPYING)
def exists(env):
	return true

def generate(env):

	def Check_libpcre(context):
		import SCons.Util
	    	context.Message('Checking for libpcre ... ')
		ret = context.TryAction('pcre-config --version')
		if ret[0]: 
			env['PCRE_ISCONFIGURED'] = 1;
			env['CXXFLAGS_PCRE'] = SCons.Util.CLVar( 
					os.popen('pcre-config --cflags').read().strip() );
			env['LINKFLAGS_PCPRE'] = SCons.Util.CLVar( 
					os.popen('pcre-config --libs').read().strip() );
		context.Result(ret[0])
		return ret[0]

        from SCons.Options import Options
        import os

        optionFile = env['CACHEDIR'] + 'libpcre.cache.py'
        opts = Options(optionFile)
        opts.AddOptions(
		('PCRE_ISCONFIGURED', 'If libpcre is already tested for'),
		('CXXFLAGS_PCRE',''),
		('LINKFLAGS_PCRE',''),
		)
        opts.Update(env)
	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('PCRE_ISCONFIGURED')):
	    conf = env.Configure(custom_tests =	 { 'Check_libpcre' : Check_libpcre } )

	    if not conf.Check_libpcre():
				print 'libpcre not found .'

	    env = conf.Finish()
	    opts.Save(optionFile, env)
    
