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
			env['LIBPCRE_ISCONFIGURED'] = 1;
			env['CXXFLAGS_LIBPCRE'] = SCons.Util.CLVar( 
					os.popen('pcre-config --cflags').read().strip() );
			env['LINKFLAGS_LIBCPRE'] = SCons.Util.CLVar( 
					os.popen('pcre-config --libs').read().strip() );
		context.Result(ret[0])
		return ret[0]

        from SCons.Options import Options
        import os

        optionFile = env['CACHEDIR'] + 'libpcre.cache.py'
        opts = Options(optionFile)
        opts.AddOptions(
		('LIBPCRE_ISCONFIGURED', 'If libart is already tested for'),
		('CXXFLAGS_LIBPCRE',''),
		('LINKFLAGS_LIBPCRE',''),
		)
        opts.Update(env)
	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('LIBPCRE_ISCONFIGURED')):
	    conf = env.Configure(custom_tests =	 { 'Check_libpcre' : Check_libpcre } )

	    if not conf.Check_libpcre():
				print 'libpcre not found .'

	    env = conf.Finish()
	    opts.Save(optionFile, env)
    
