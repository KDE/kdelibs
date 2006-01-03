# Copyright Thomas Nagy 2005
# Copyright Ralf Habacker 2005
# BSD license (see COPYING)
def exists(env):
	return true

def generate(env):

	def Check_libjpeg(context):
		import SCons.Util

		context.Message('Checking for libjpeg...\n')

		ret = 0

		header = env.find_file('jpeglib.h', env['CPPPATH'])
# CE: Doesn't work for msvc currently
#		if conf.CheckHeader('jpeglib.h'):
		if header != '':
			if conf.CheckLib('jpeg'):
				conf.env['INCLUDES_JPEG'] = [ 'jpeglib.h' ]
				conf.env['LIB_JPEG']      = [ 'jpeg' ]
				ret = 1
			elif conf.CheckLib('jpeg6b'):
				conf.env['INCLUDES_JPEG '] = [ 'jpeglib.h' ]
				conf.env['LIB_JPEG ']      = [ 'jpeg6b' ]
				ret = 1

		context.Result(ret)
		return ret

	from SCons.Options import Options
	import os

	optionFile = env['CACHEDIR'] + 'libjpeg.cache.py'
	opts = Options(optionFile)
	opts.AddOptions(
		('CACHED_JPEG', 'Whether libjpeg is available'),
		('CXXFLAGS_JPEG',''),
		('CCFLAGS_JPEG',''),
		('LINKFLAGS_JPEG',''),
		('INCLUDES_JPEG',''),
		('LIB_JPEG',''),
		('LIBPATH_JPEG',''),
		)
	opts.Update(env)

	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('CACHED_JPEG')):

 		conf = env.Configure(custom_tests =	 { 'Check_libjpeg' : Check_libjpeg} )
		have_jpeg = conf.Check_libjpeg()
		env = conf.Finish()

		env.write_lib_header( 'libjpeg', have_jpeg, True )

	opts.Save(optionFile, env)
	
