#include <kuserprofile.h>
#include <kservice.h>
#include <kmimetype.h>
#include <assert.h>
#include <kstddirs.h>

#include <kapp.h>

main(int argc, char *argv[])
{
   KApplication k(argc,argv); // KMessageBox needs KApp for makeStdCaption

   debug("Trying to look for text/plain");
   KMimeType * s1 = KMimeType::mimeType("text/plain");
   if ( s1 )
   {
     debug("Found it !");
     debug(QString("Comment is %1").arg(s1->comment("",false)));
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
   debug("got %d offers", offers.count());
   KServiceTypeProfile::OfferList::Iterator it = offers.begin();
   for ( ; it != offers.end() ; it++ )
   {
     debug((*it).service()->name());
   }

   debug("Querying userprofile for services associated with KOfficeFilter");
   offers = KServiceTypeProfile::offers("KOfficeFilter");
   debug("got %d offers", offers.count());
   it = offers.begin();
   for ( ; it != offers.end() ; it++ )
   {
     debug((*it).service()->name());
   }

   debug("Trying findByURL for $KDEDIR/bin/kdesktop\n");
   KMimeType* mf  = KMimeType::findByURL( KStandardDirs::findExe( "kdesktop" ), 0,
				 true, false );
   assert( mf );
   debug(QString("Name is %1").arg(mf->name()));
   debug(QString("Comment is %1").arg(mf->comment("",false)));

   debug("Trying findByURL for home.png\n");
   mf  = KMimeType::findByURL( locate( "toolbar", "home.png" ), 0,
				 true, false );
   assert( mf );
   debug(QString("Name is %1").arg(mf->name()));
   debug(QString("Comment is %1").arg(mf->comment("",false)));

   debug("done");
}
