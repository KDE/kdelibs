This file is intended as entry point for the win32 port of KDELIBS.

Any developer interested into working on the win32 port is invited 
to start at http://www.kdelibs.com for an overview and status of the port. 

BUGS: 


TODO:
- kio/kio/kfileitem.cpp: win32 implementation of user/group/permissions handling required.
qt's QFileInfo class has methods for platform independent user/group/permission retrieving 
but they are not implemented complete in the win32 port.
There may be more places where such handling is required, I haven't checked yet







