#include <kcmdlineargs.h>
#include <kapp.h>

#include <stdio.h>

static const char *version = "v0.0.2 1999 (c) Waldo Bastian";
static const char *description ="This is a test program.";

static KCmdLineOptions options[] =
{
 { "-test", "do a short test only", 0 },
 { "-baud <baudrate>", "set baudrate", "9600" },
 { "file(s)", "Files to load", 0 },
 { 0,0,0 }
};

int
main(int argc, char *argv[])
{
   KCmdLineArgs::init( argc, argv, "testapp", description, version);
   KApplication::addCmdLineOptions(); // Add KDE and Qt specific options.
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
   
   // Read the value of an option. 
   QCString baudrate = args->getOption("baud"); // 9600 is the default value.
   
   printf("Baudrate = %s\n", baudrate.data());

   delete args; // Free up memory.
  
   
   k.exec();
   return 0;
}



