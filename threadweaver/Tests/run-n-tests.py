#!/usr/bin/python

import re
import os
import sys
import time
import platform

def RunTests( Number ):
    # use Linux as the default:
    SuccessCount = 0
    Platform = platform.platform()
    Command = "LD_LIBRARY_PATH=../Weaver:../Experimental ./JobTests >/dev/null"
#    if re.search( "Darwin", Platform ):
#        Command = "DYLD_LIBRARY_PATH=../Weaver:../Experimental ./JobTests.app/Contents/MacOs/JobTests >/dev/null"
    for count in range( Number ):
	retValue = os.system ( Command)
	resultString = " (FAILED)"
	if retValue == 0:
	    resultString = " (succeeded)"
            SuccessCount = SuccessCount + 1
        print "Test run #" + str(count + 1) + resultString
    return SuccessCount

Number = 10

try:
    Number = int ( sys.argv[1] )
except:
    print "No number given, using default of ", Number

FailedCount = 0
SuccessCount = 0

t1 = time.time()
SuccessCount = RunTests ( Number )
t2 = time.time()


elapsed = ((t2 - t1) / 60.0, t2 - t1, (t2 - t1) * 1000.0)

print "RunTests: " + str(Number) + " test runs (" + str(SuccessCount) + " successful, " + str(Number - SuccessCount) + " failed)"
print "RunTests: %0.3fms (%0.3fs)" % ( elapsed[2], elapsed[1] )
print "Averages: %0.3fms per test run (%0.3fs)" % ( elapsed[2] / Number, elapsed[1] / Number )



