
#include "kbuildsycoca.h"
#include "kservicetypefactory.h"
#include "kservicefactory.h"

#include <kapp.h>
#include <qfile.h>
#include <qdatastream.h>


main(int argc, char *argv[])
{
   // KApplication k(argc,argv); // KMessageBox needs KApp for makeStdCaption

   bool build = false;

   for(int i = 1; i < argc; i++)
   {
      if (argv[i] == QString("-build"))
          build = true;
   }

   if (build)
   {
     KBuildSycoca *sycoca= new KBuildSycoca; // Build data base
     
     KServiceTypeFactory *factory = new KServiceTypeFactory;
     sycoca->addFactory(factory);
     KServiceFactory *sfactory = new KServiceFactory;
     sycoca->addFactory(sfactory);

     sycoca->build(); // Parse dirs
     sycoca->save(); // Save database
     delete sycoca;
     delete factory;
     delete sfactory;
   }

}
