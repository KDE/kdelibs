#include <kcmdlineargs.h>
#include <QtCore/QCoreApplication>
#include <kurl.h>

#include <stdio.h>
#include <assert.h>
#include <QtCore/QDir>
#include <kdebug.h>

#if 1
int
main(int argc, char *argv[])
{
   KCmdLineOptions options;
   options.add("test", ki18n("do a short test only, note that\n"
                             "this is rather long comment"));
   options.add("baud <baudrate>", ki18n("set baudrate"), "9600");
   options.add("+file(s)", ki18n("Files to load"));

   KCmdLineArgs::init(argc, argv, "testapp", 0,
                      ki18n("TestApp"), "v0.0.2",
                      ki18n("This is a test program.\n"
                            "1999 (c) Waldo Bastian"));

   KCmdLineArgs::addCmdLineOptions( options ); // Add my own options.

   kDebug() << "allArguments:" << KCmdLineArgs::allArguments();

   // MyWidget::addCmdLineOptions();

   //KApplication app( false );
   QCoreApplication app( KCmdLineArgs::qtArgc(), KCmdLineArgs::qtArgv() );

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
   QString baudrate = args->getOption("baud"); // 9600 is the default value.

   printf("Baudrate = %s\n", baudrate.toLocal8Bit().data());

   printf("Full list of baudrates:\n");
   QStringList result = args->getOptionList("baud");
   Q_FOREACH(const QString& it, result) {
      printf("Baudrate = %s\n", it.toLocal8Bit().data());
   }
   printf("End of list\n");

   for(int i = 0; i < args->count(); i++)
   {
      printf("%d: %s\n", i, args->arg(i).toLocal8Bit().data());
      printf("%d: %s\n", i, args->url(i).url().toLocal8Bit().data());
   }

   // Check how KCmdLineArgs::url() works
   KUrl u = KCmdLineArgs::makeURL("/tmp");
   kDebug() << u;
   assert(u.toLocalFile() == "/tmp");
   u = KCmdLineArgs::makeURL("foo");
   kDebug() << u << "  expected: " << KUrl(QDir::currentPath()+"/foo");
   assert(u.toLocalFile() == QDir::currentPath()+"/foo");
   u = KCmdLineArgs::makeURL("http://www.kde.org");
   kDebug() << u;
   assert(u.url() == "http://www.kde.org");

   QFile file("a:b");
#ifndef Q_WS_WIN
   bool ok = file.open(QIODevice::WriteOnly);
   assert(ok);
#endif
   u = KCmdLineArgs::makeURL("a:b");
   qDebug() << u.toLocalFile();
   assert(u.isLocalFile());
   assert(u.toLocalFile().endsWith("a:b"));

   args->clear(); // Free up memory.


//   return app.exec();
   return 0;
}
#else
int
main(int argc, char *argv[])
{
   KCmdLineArgs::init( argc, argv, "testapp", description, version);

   QApplication app( KCmdLineArgs::qtArgc(), KCmdLineArgs::qtArgv(), false );

   return app.exec();
}
#endif


