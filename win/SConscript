#! /usr/bin/env python

Import('env')

env.subdirs('iceauth')

obj = env.qt4obj('shlib',env)
obj.vnum     = '6.0.2'
obj.target   = 'kdewin32'
obj.uselib   = 'QT QTCORE QT3SUPPORT'
obj.source   = """bootstrap.cpp qeventloopex.cpp fcntl.c getenv.c kde_file_win.c net.c readdir.c 
resource.c strndup.c time.c unistd.c fsync.c grp.c mmap.c pwd.c 
realpath.c signal.c syslog.c uname.c  win32_utils.c win32_utils2.cpp
"""

obj.ccflags += ' -DMAKE_KDEWIN32_LIB '

if env['CC'] == 'cl':
	obj.includes = 'include include/msvc'
	obj.ccflags += ' /FI./include/kdelibs_export_win.h /FI./include/kdemacros.h '
	obj.linkflags += ' advapi32.lib shell32.lib '

obj.execute()
