#include <kcmdlineargs.h>
#include <klocale.h>
#include <kapplication.h>
#include <kurl.h>

#include <stdio.h>

// we use our own macro to not bother translators
// but still demonstrate the use. You would use I18N_NOOP
#define I18N_NOP(x) x

static const char version[] = "v0.0.2 1999 (c) Waldo Bastian";
static const char description[] = I18N_NOP("This is a test program.");

static KCmdLineOptions options[] =
{
 { "test",		I18N_NOP("do a short test only, note that\n"
				  "this is rather long comment"), 0 },
 { "baud <baudrate>",	I18N_NOP("set baudrate"), "9600" },
 { "+file(s)",		I18N_NOP("Files to load"), 0 },
 KCmdLineLastOption
};

#if 1
int
main(int argc, char *argv[])
{
   KLocale::setMainCatalog("kdelibs");
   KCmdLineArgs::init( argc, argv, "testapp", "TestApp", description, version);

   KCmdLineArgs::addCmdLineOptions( options ); // Add my own options.

   // MyWidget::addCmdLineOptions();

   KApplication k( false );

   // Get application specific arguments
   KCmdLineArgs *args = KCmdLineArgs::parsedArgs(); 
   // Check if an option is set
   if (args->isSet("test"))
   {
      // Do stuff
      printf("Option 'test' is set.\n");
   }   

   if (args->isSet("baud"))
   {
      // Do stuff
      printf("Option 'baud' is set.\n");
   }   
   
   // Read the value of an option. 
   QByteArray baudrate = args->getOption("baud"); // 9600 is the default value.
   
   printf("Baudrate = %s\n", baudrate.data());

   printf("Full list of baudrates:\n");
   QByteArrayList result = args->getOptionList("baud");
   for(QByteArrayList::ConstIterator it=result.begin();
       it != result.end();
       ++it)
   {
      printf("Baudrate = %s\n", (*it).data());
   }
   printf("End of list\n");

   for(int i = 0; i < args->count(); i++)
   {
      printf("%d: %s\n", i, args->arg(i));
      printf("%d: %s\n", i, args->url(i).url().ascii());
   }

   args->clear(); // Free up memory.
  
   
//   k.exec();
   return 0;
}
#else
int
main(int argc, char *argv[])
{
   KCmdLineArgs::init( argc, argv, "testapp", description, version);

   KApplication k( true, true );

   k.exec();
   return 0;
}
#endif


