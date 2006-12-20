#!/usr/bin/python

import buildinator_common
from buildinator_build_status import BuildStatus

# get Subversion Url from command line arg 1:
Url = '.'
try:
    Url = sys.argv[1]
except:
    pass

Revisions = buildinator_common.GetRevisionList( Url )
for Revision in Revisions:
    print 'Revision ' + str(Revision.revision) + ' by ' + Revision.committer

