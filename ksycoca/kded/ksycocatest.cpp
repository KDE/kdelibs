#include <ksycoca.h>
#include <kservicetypefactory.h>
#include <kservicefactory.h>

#include <kapp.h>

main(int argc, char *argv[])
{
   // KApplication k(argc,argv); // KMessageBox needs KApp for makeStdCaption

   debug("Trying to look for text/plain");
   KServiceType * s = KServiceTypeFactory::findServiceTypeByName("text/plain");
   if ( s )
   {
     debug("Found it !");
   }
   else
   {
     debug("Not found !");
   }
     
   debug("done");
}
