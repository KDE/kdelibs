##
# @file 
# low level configuration like build directory and low level related config.h creation

def detect(lenv):
	import os

	if not os.path.exists( lenv['_BUILDDIR_'] ): os.mkdir(lenv['_BUILDDIR_'])
        dest=open(lenv.join(lenv['_BUILDDIR_'], 'config-lowlevel.h'), 'w')
        dest.write('/* lowlevel configuration */\n')

	content="""
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_CTYPE_H 1 
#define HAVE_MALLOC_H 1
#define HAVE_STRING_H 1 
#define LTDL_SHLIB_EXT ".dll"
#define LTDL_OBJDIR ""
#define HAVE_STDIO_H 1
#define HAVE_STDLIB_H 1
"""

	dest.write(content)
	dest.close()
