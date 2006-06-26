#!/usr/bin/python

import os
import sys
import time

def RunTests( Number ):
    for count in range( Number ):
        print "Test run #", count + 1
        os.system ("LD_LIBRARY_PATH=../Weaver:../Experimental ./JobTests >/dev/null")

Number = 10

try:
    Number = int ( sys.argv[1] )
except:
    print "No number given, using default of ", Number

t1 = time.time()
RunTests ( Number )
t2 = time.time()


elapsed = ((t2 - t1) / 60.0, t2 - t1, (t2 - t1) * 1000.0)

print "RunTests: %0.3fms (%0.3fs)" % ( elapsed[2], elapsed[1] )
print "Averages: %0.3fms per test run (%0.3fs)" % ( elapsed[2] / Number, elapsed[1] / Number )





