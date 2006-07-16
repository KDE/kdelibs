#!/usr/bin/python

import buildinator_common

# "main":

# parse command line args to find the revision to test:
print """Build and test a subversion revision of a module. (C) Mirko Boehm, 2006
This script is licensed as a part of the ThreadWeaver multithreading suite
under the LGPL.
Run this script in an empty directory. It will create folders and files in
this folder, assuming nothing in it needs to be preserved.
"""

Module = ''
ProFileName = ''

Revision = 0
MinimumTestRevision = -1
try:
    Revision = int (sys.argv[1])
    Module = sys.argv[2]
    ProFileName = sys.argv[3]
except:
    print "Usage: " + sys.argv[0] + " <SVN revision to test>"
    print "Example: " + sys.argv[0] \
          + ' 452230 svn+ssh://svn.kde.org/home/kde/trunk/kdenonbeta/threadweaver ' \
          + 'ThreadWeaver.pro'
    sys.exit (-1)
else:
    ExecuteBuildAndTest ( Revision, Module, ProFileName)
