
#include "ksycoca.h"
#include "kservicetypefactory.h"

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
     KSycoca *sycoca= new KSycoca(true); // Build data base
     
     KServiceTypeFactory *factory = new KServiceTypeFactory(true); // Build data base
     sycoca->addFactory(factory);
     sycoca->save(); // Save database
     delete sycoca;
     delete factory;
   }

}
