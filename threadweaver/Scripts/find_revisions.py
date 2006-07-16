#!/usr/bin/python

import buildinator_common

# get Subversion Url from command line arg 1:
Url = '.'
try:
    Url = sys.argv[1]
except:
    pass

Lines = buildinator_common.GetSvnLog( Url )
Revisions = buildinator_common.GetRevisionList( Lines )
print Revisions
