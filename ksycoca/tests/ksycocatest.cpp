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
   }
   else
   {
     debug("Not found !");
   }
     
   debug("done");
}
