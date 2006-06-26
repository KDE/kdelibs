#!/usr/bin/python

import os
import sys
import time

def TimeIt(f):
    """ Annotate a function with its elapsed execution time. """

    def timed_f(*args, **kwargs):
        t1 = time.time()

        try:
            f(*args, **kwargs)
        finally:
            t2 = time.time()
        
        timed_f.func_time = ((t2 - t1) / 60.0, t2 - t1, (t2 - t1) * 1000.0)
        
        sys.stdout.write ("%s: %0.3fms (%0.3fs)\n" % ( f.func_name, timed_f.func_time[2], timed_f.func_time[1] ))
            
    return timed_f()

def RunTests():
    for count in range(10):
        print "Test run #", count + 1
        os.system ("LD_LIBRARY_PATH=../Weaver:../Experimental ./JobTests >/dev/null")


TimeIt(RunTests)


