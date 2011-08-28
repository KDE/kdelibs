#!/usr/bin/python

import os
import sys
import buildinator_common
from buildinator_build_status import BuildStatus

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
Status = BuildStatus()

try:
    Status.revision = int (sys.argv[1])
    Status.svnUrl = sys.argv[2]
    Status.projectFile = sys.argv[3]
    Status.numberOfTestRuns = 100
except:
    print "Usage: " + sys.argv[0] + " <SVN revision to test> <SvnURL> <ProFileName>"
    print "Example: " + sys.argv[0] \
          + ' 452230 svn+ssh://svn.kde.org/home/kde/trunk/kdenonbeta/threadweaver ' \
          + 'ThreadWeaver.pro'
    sys.exit (-1)
else:
    Results = buildinator_common.ExecuteBuildAndTest ( Status, os.getcwd() )
    for Result in Results:
        Result.dumpBuildStatus()

    
