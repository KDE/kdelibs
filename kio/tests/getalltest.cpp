#include <kservice.h>
#include <kmimetype.h>

#include <kapp.h>

main(int argc, char *argv[])
{
   KApplication k(argc,argv); // KMessageBox needs KApp for makeStdCaption

for (int i = 0 ; i < 2 ; ++i ) { // test twice to see if they got deleted
   debug("All services");
   KService::List services = KService::allServices();
   debug("got %d services", services.count());
   QValueListIterator<KService::Ptr> s = services.begin();
   for ( ; s != services.end() ; ++s )
   {
     debug((*s)->name());
   }
}
   
   debug("All mimeTypes");
   KMimeType::List mimeTypes = KMimeType::allMimeTypes();
   debug("got %d mimeTypes", mimeTypes.count());
   QValueListIterator<KMimeType::Ptr> m = mimeTypes.begin();
   for ( ; m != mimeTypes.end() ; ++m )
   {
     debug((*m)->name());
   }
   
   debug("done");
}
