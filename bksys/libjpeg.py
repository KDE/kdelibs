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
		jpeg_include_path = env['CPPPATH']
		jpeg_lib_path = env['LIBPATH']
		
		if env['WINDOWS']:
			jpeg_include_path += ['c:\\libjpeg\\include']
			jpeg_lib_path += ['c:\\libjpeg\\lib']
			ext='.lib'
		else:
			jpeg_include_path += ['/usr/include','/usr/local/include']
			jpeg_lib_path += ['/usr/lib','/usr/local/lib']
			ext='.so'

		#we need to check for libjpeg6b then normal libjpeg to
		#be compatible with the current autoconf check
		header = env.find_file_ext('jpeglib.h', jpeg_include_path)
		libjpeg = env.find_file_ext('jpeg'+ext, jpeg_lib_path)
		libjpeg6b = env.find_file_ext('jpeg6b'+ext, jpeg_lib_path)

		if header != '':
			if libjpeg != '':
				conf.env.AppendUnique( LIBPATH = [ libjpeg ] )
				if conf.CheckLib('jpeg'):
					conf.env['INCLUDES_JPEG'] = [ header ]
					conf.env['LIBPATH_JPEG']  = [ libjpeg ]
					conf.env['LIB_JPEG']      = [ 'jpeg' ]
					ret = 1
			if ( ret == 0 and libjpeg6b != ''):
				conf.env.AppendUnique( LIBPATH = [ libjpeg6b ] )
				if conf.CheckLib('jpeg6b'):
					conf.env['INCLUDES_JPEG '] = [ header ]
					conf.env['LIBPATH_JPEG ']  = [ libjpeg6b ]
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
	
