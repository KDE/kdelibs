# Copyright liucougar 2005
# BSD license (see COPYING)

"""
Find and load the openssl necessary compilation and link flags
"""

def exists(env):
	return true


def generate(env):

	def Check_openssl(context):
		import SCons.Util
		ret = 0

		context.Message('Checking for openssl... ')
		if env['CC'] == 'gcc': 
			if env.find_file('opensslconf.h',['c:\openssl\include\openssl']):
				env['CACHED_OPENSSL']  = 1
				env['CPPPATH_OPENSSL'] = ['c:\\openssl\\include']
				env['LIBPATH_OPENSSL'] = ['c:\\openssl\\lib\\mingw']
				env['LIB_OPENSSL']	   = ['eay32']
				ret = 1
		context.Result(ret)
		return ret

	from SCons.Options import Options

	optionFile = env['CACHEDIR'] + 'openssl.cache.py'
	opts = Options(optionFile)
	opts.AddOptions(
		('CACHED_OPENSSL', 'Whether openssl is available'),
		('CPPPATH_OPENSSL',''),
		('LIBPATH_OPENSSL',''),
		('LIB_OPENSSL',''),
		)
	opts.Update(env)
	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('CACHED_OPENSSL')):

		if env['WINDOWS']:
			conf = env.Configure(custom_tests =	 { 'Check_openssl' : Check_openssl} )
			have_openssl = conf.Check_openssl()
			env = conf.Finish()
	
		else:
			from SCons.Tool import Tool
			import os
			pkgs = Tool('pkgconfig', ['./bksys'])
			pkgs.generate(env)
	
			# TODO this will not work on opensuse 10 (ita) openssl-devel not installed
			have_openssl = env.pkgConfig_findPackage('OPENSSL', 'openssl', '0.9.6')
	
		dest=open(env.join(env['_BUILDDIR_'], 'ksslconfig.h'), 'w')
		dest.write('/* openssl configuration created by bksys */\n')
		if have_openssl: dest.write('#define KSSL_HAVE_SSL 1\n');
		dest.close()
		#env['_CONFIG_H_'].append('openssl')
	
		if not have_openssl: 
				print 'openssl not found.'

		opts.Save(optionFile, env)
