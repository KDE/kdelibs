#! /usr/bin/env python

Import('env')

obj = env.genobj('shlib',env)
obj.vnum     = '6.0.2'
obj.target   = 'libkdewin32'
obj.source   = 'pwd.c win32_utils.c time.c unistd.c signal.c fcntl.c'

if env['CC'] == 'gcc': 
	obj.includes = 'include include/mingw'
elif env['CC'] == 'cl':
	obj.includes = 'include include/msvc'
	
obj.execute()
