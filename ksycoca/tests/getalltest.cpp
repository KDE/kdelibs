#include <kservice.h>
#include <kmimetype.h>

//#include <kapp.h>

main(int argc, char *argv[])
{
   // KApplication k(argc,argv); // KMessageBox needs KApp for makeStdCaption

   debug("All services");
   KServiceList * services = KService::allServices();
   debug("got %d services", services->count());
   QListIterator<KService> s ( *services );
   for ( ; s.current() ; ++s )
   {
     debug(s.current()->name());
   }
   
   debug("All mimeTypes");
   KMimeTypeList * mimeTypes = KMimeType::allMimeTypes();
   debug("got %d mimeTypes", mimeTypes->count());
   QListIterator<KMimeType> m ( *mimeTypes );
   for ( ; m.current() ; ++m )
   {
     debug(m.current()->name());
   }
   
   debug("done");
}
