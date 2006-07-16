#!/usr/bin/python

import os
import sys
import buildinator_common

# "main":

# parse command line args to find the revision to test:
print """Build and test a subversion revision of a module. (C) Mirko Boehm, 2006
This script is licensed as a part of the ThreadWeaver multithreading suite
under the LGPL.
Run this script in an empty directory.
"""

Module = ''
ProFileName = ''
Revision = 0

try:
    Revision = int (sys.argv[1])
    Module = sys.argv[2]
    ProFileName = sys.argv[3]
except:
    print "Usage: " + sys.argv[0] + " <SVN revision to test> <SvnURL> <ProFileName>"
    print "Example: " + sys.argv[0] \
          + ' 452230 svn+ssh://svn.kde.org/home/kde/trunk/kdenonbeta/threadweaver ' \
          + 'ThreadWeaver.pro'
    sys.exit (-1)
else:
#    if (Revision < 452230):
#        raise ("Only revisions starting at 452230 are supported (Qt4 based versions)")
    buildinator_common.ExecuteBuildAndTest ( os.getcwd(), Revision, Module, ProFileName)
    
