# Copyright David Faure <faure@kde.org> 2005
# BSD license (see COPYING)
def exists(env):
	return true

def generate(env):

	def Check_compiler(context):
		context.Message('Checking for compiler version... ')
		import sys, os
		
		if env['WINDOWS']:
			sys.path.append('bksys'+os.sep+'win32')
			from detect_compiler import detect
		else:
			sys.path.append('bksys'+os.sep+'unix') # works for unix and osx too
			from detect_compiler import detect

		compiler_version = detect(env)

		# Quote backslashes, as we are going to make this a string
		compiler_version.replace('\\', '\\\\')
		
		env['_CONFIG_H_'].append('compiler')
		if compiler_version:
			env['COMPILER_ISCONFIGURED'] = 1;
			dest = open(env.join(env['_BUILDDIR_'], 'config-compiler.h'), 'w')
			dest.write('/* compiler name and version */\n')
			dest.write('#define KDE_COMPILER_VERSION "'+compiler_version+'"\n')
			dest.close();
			context.Result(compiler_version)
		else:
			print 'Compiler not found.'
			context.Result(False)
			env.Exit(1)

		
	from SCons.Options import Options

	optionFile = env['CACHEDIR'] + 'compiler.cache.py'
	opts = Options(optionFile)
	opts.AddOptions(
		('COMPILER_ISCONFIGURED', 'If the compiler is already tested for'),
		)
	opts.Update(env)
	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('COMPILER_ISCONFIGURED')):
		conf = env.Configure(custom_tests =	 { 'Check_compiler' : Check_compiler } )

		conf.Check_compiler()

		env = conf.Finish()
		opts.Save(optionFile, env)
