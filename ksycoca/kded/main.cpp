
#include "kbuildsycoca.h"
#include <kservicetypefactory.h>
#include <kservicefactory.h>

#include <kapp.h>
#include <qfile.h>
#include <qdatastream.h>

#include <unistd.h>

main(int argc, char *argv[])
{
  KApplication k(argc,argv); // KMessageBox needs KApp for makeStdCaption

     KBuildSycoca *sycoca= new KBuildSycoca; // Build data base
     
     KServiceTypeFactory *factory = new KServiceTypeFactory;
     sycoca->addFactory(factory);
     KServiceFactory *sfactory = new KServiceFactory;
     sycoca->addFactory(sfactory);

     sycoca->build(); // Parse dirs
     sycoca->save(); // Save database

#ifndef NOFORK     // define NOFORK to debug kded in gdb
     switch(fork()) {
     case -1:
	  fprintf(stderr, "kded: fork() failed!");
	  break;
     case 0:
	  // ignore in child
	  break;
     default:
	  // parent: exit immediatly
	  _exit(0);
     }
#endif

     k.exec(); // keep running
}
