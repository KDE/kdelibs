#!/usr/bin/python

from buildinator_build_status import BuildStatus

# main:
status = BuildStatus()
status.revision = 123456
status.platform = 'Linux boombox 2.6.13-15.7-default #1 Tue Nov 29 14:32:29 UTC 2005 x86_64 x86_64 x86_64 GNU/Linux'
status.options = 'CONFIG+=debug'
status.prefix = 'Debug'
status.checkoutStatus = 1
status.configureStatus = 1
status.compileStatus = 2
status.initialTestStatus = 4
status.committer = 'mirko'

status.dumpBuildStatus()

    


