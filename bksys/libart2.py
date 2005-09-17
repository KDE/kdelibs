# Copyright Thomas Nagy 2005
# BSD license (see COPYING)
def exists(env):
	return true

def generate(env):

	def Check_libart2(context):
		import SCons.Util
		ret = context.TryAction('libart2-config --version')
		if ret[0]: 
    			ver = os.popen('libart2-config --version').read().strip().split('.')
		    	context.Message('Checking for libart2 ... ')
			if (int(ver[0])*1000000 + int(ver[1])*1000 + int(ver[2]))<2003008: 
			    context.Result(False)
			    return False
			env['LIBART_ISCONFIGURED'] = 1;
			env['CXXFLAGS_LIBART'] = SCons.Util.CLVar( 
					os.popen('libart2-config --cflags').read().strip() );
			env['LINKFLAGS_LIBART'] = SCons.Util.CLVar( 
					os.popen('libart2-config --libs').read().strip() );
		context.Result(ret[0])
		return ret[0]

        from SCons.Options import Options
        import os

        optionFile = env['CACHEDIR'] + 'libart.cache.py'
        opts = Options(optionFile)
        opts.AddOptions(
		('LIBART_ISCONFIGURED', 'If libart is already tested for'),
		('CXXFLAGS_LIBART',''),
		('LINKFLAGS_LIBART',''),
		)
        opts.Update(env)
	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('LIBART_ISCONFIGURED')):
	    conf = env.Configure(custom_tests =	 { 'Check_libart2' : Check_libart2 } )

	    if not conf.Check_libart2():
				print 'libart2 >= 2.3.8 not found.'
				env.Exit(1)

	    env = conf.Finish()
	    opts.Save(optionFile, env)
	    
