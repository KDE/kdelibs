#include <kcmdlineargs.h>
#include <klocale.h>
#include <kapp.h>

#include <stdio.h>

static const char *version = "v0.0.2 1999 (c) Waldo Bastian";
static const char *description = I18N_NOOP("This is a test program.");

static KCmdLineOptions options[] =
{
 { "test",		I18N_NOOP("do a short test only, note that\n"
				  "this is rather long comment"), 0 },
 { "baud <baudrate>",	I18N_NOOP("set baudrate"), "9600" },
 { "+file(s)",		I18N_NOOP("Files to load"), 0 },
 { 0,0,0 }
};

#if 1
int
main(int argc, char *argv[])
{
   KCmdLineArgs::init( argc, argv, "testapp", description, version);

   KCmdLineArgs::addCmdLineOptions( options ); // Add my own options.

   // MyWidget::addCmdLineOptions();

   KApplication k( true, true );

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
   QCString baudrate = args->getOption("baud"); // 9600 is the default value.
   
   printf("Baudrate = %s\n", baudrate.data());

   for(int i = 0; i < args->count(); i++)
   {
      printf("%d: %s\n", i, args->arg(i));
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


