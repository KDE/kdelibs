#include <kuserprofile.h>
#include <kservice.h>
#include <kmimetype.h>

//#include <kapp.h>

main(int argc, char *argv[])
{
   // KApplication k(argc,argv); // KMessageBox needs KApp for makeStdCaption

   debug("Trying to look for text/plain");
   KMimeType * s = KMimeType::mimeType("text/plain");
   if ( s )
   {
     debug("Found it !");
     debug(QString("Comment is %1").arg(s->comment("",false)));
   }
   else
   {
     debug("Not found !");
   }
     
   debug("Trying to look for Konqueror");
   KService * se = KService::service("Konqueror");
   if ( se )
   {
     debug("Found it !");
     debug(QString("Comment is %1").arg(se->comment()));
   }
   else
   {
     debug("Not found !");
   }
   
   debug("Querying userprofile for services associated with text/plain");
   KServiceTypeProfile::OfferList offers = KServiceTypeProfile::offers("text/plain");
   KServiceTypeProfile::OfferList::Iterator it = offers.begin();
   for ( ; it != offers.end() ; it++ )
   {
     debug((*it).service()->name());
   }

   debug("done");
}
