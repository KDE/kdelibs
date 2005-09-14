# Julien Antille, Thomas Nagy, 2005
# BSD license (see COPYING)

BOLD   ="\033[1m"
RED    ="\033[91m"
GREEN  ="\033[92m"
YELLOW ="\033[93m"
CYAN   ="\033[96m"
NORMAL ="\033[0m"

def exists(env):
	return true

def generate(env):
	""" Detect the cg library (nvidia) """

	from SCons.Options import Options
	cachefile = env['CACHEDIR']+'/libkdegames.cache.py'
	opts = Options(cachefile)
	opts.AddOptions(
		( 'HASCG', 'has the cg library' )
        )
	opts.Update(env)

	if 'configure' in env['TARGS'] or not env.has_key('HASCG'):
		print "Checking for cg nvidia            : ",
		
		#import SCons.SConf
		#conf = SCons.SConf.SConf( env )
		#if not conf.CheckCHeader( 'Cg/cg.h' ):
		import os
		if not os.path.isfile( '/usr/include/Cg/cg.h' ):
			print RED+'We really need the cg library !'+NORMAL
			print RED+'Get ftp://download.nvidia.com/developer/cg/Cg_1.3/Linux/Cg-1.3.0501-0700.i386.tar.gz and unpack it in your root directory'+NORMAL
			import sys
			sys.exit(1)
		else:
			print GREEN+'looks good'+NORMAL
		env['HASCG']=1
		#env = conf.Finish()
		opts.Save(cachefile, env)
