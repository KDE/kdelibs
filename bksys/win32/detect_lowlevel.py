##
# @file 
# platform-dependent low level configuration

def detect(lenv, dest):

	content ="""
#define LTDL_SHLIB_EXT ".dll"
#define LTDL_OBJDIR ""

/* Distribution Text to append to OS */
#define KDE_DISTRIBUTION_TEXT "KDE Libraries for MS Windows"
"""

	import sys
	wver=sys.getwindowsversion()
	content += """
/* What OS used for compilation */
#define KDE_COMPILING_OS \"MS Windows %d.%d %s\"
""" % (wver[0], wver[1], wver[4])

	dest.write(content)
