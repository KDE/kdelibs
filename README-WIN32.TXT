This file is intended as entry point for the win32 port of KDELIBS.

Any developer interested into working on the win32 port is invited 
to start at http://www.kdelibs.com for an overview and status of the port. 

BUGS: 

- kde4-config must not print the compile time installation dir, which may completly others as the real installation path. Instead it should 
  use one level up directory where it is running as we do in dbus.
 

