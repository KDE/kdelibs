def detect(lenv):
	import os

	if not os.path.exists( lenv['_BUILDDIR_'] ): os.mkdir(lenv['_BUILDDIR_'])
        dest=open(lenv.join(lenv['_BUILDDIR_'], 'config-lowlevel.h'), 'w')
        dest.write('/* lowlevel configuration */\n')

	content="""
"""

	dest.write(content)
	dest.close()
