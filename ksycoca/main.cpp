
#include "kbuildsycoca.h"
#include "kservicetypefactory.h"
#include "kservicefactory.h"

#include <qfile.h>
#include <qdatastream.h>


main(int argc, char *argv[])
{
   bool build = false;

   for(int i = 1; i < argc; i++)
   {
      if (argv[i] == QString("-build"))
          build = true;
   }

   if (build)
   {
     KBuildSycoca *sycoca= new KBuildSycoca(); // Build data base
     
     KServiceTypeFactory *factory = new KServiceTypeFactory(true); // Build data base
     sycoca->addFactory(factory);
     KServiceFactory *sfactory = new KServiceFactory(true); // Build data base
     sycoca->addFactory(sfactory);
     sycoca->save(); // Save database
     delete sycoca;
     delete factory;
   }

}
