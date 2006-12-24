This file is intended as entry point for the win32 port of KDELIBS.

Any developer interested into working on the win32 port is invited 
to start at http://www.kdelibs.com for an overview and status of the port. 

BUGS: 
- kio/tests/netaccesstest has dns resolution problems using ftp because
  gethostbyname on win32 isn't able to handle ip addresses as input. 
  In cases where ip addresses is given as source address the resolve 
  could be skipped. 

  

