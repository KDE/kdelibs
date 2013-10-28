#include <kcmdlineargs.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QUrl>

#include <stdio.h>
#include <assert.h>
#include <QtCore/QDir>
#include <QtCore/QDebug>

#if 1
int
main(int argc, char *argv[])
{
   KCmdLineOptions options;
   options.add("test", ki18n("do a short test only, note that\n"
                             "this is rather long comment"));
   options.add("b");
   options.add("baud <baudrate>", ki18n("set baudrate"), "9600");
   options.add("+file(s)", ki18n("Files to load"));

   KCmdLineArgs::init(argc, argv, "testapp", 0,
                      ki18n("TestApp"), "v0.0.2",
                      ki18n("This is a test program.\n"
                            "1999 (c) Waldo Bastian"));

   KCmdLineArgs::addCmdLineOptions( options ); // Add my own options.

   // MyWidget::addCmdLineOptions();

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

   qDebug() << "allArguments:" << KCmdLineArgs::allArguments();

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
      printf("%d: %s\n", i, args->url(i).toEncoded().data());
   }

   // Check how KCmdLineArgs::url() works
   QUrl u = KCmdLineArgs::makeURL(QByteArray("/tmp"));
   qDebug() << u;
   assert(u.toLocalFile() == QLatin1String("/tmp"));
   u = KCmdLineArgs::makeURL(QByteArray("foo"));
   qDebug() << u << "  expected: " << QUrl(QDir::currentPath()+QLatin1String("/foo"));
   assert(u.toLocalFile() == QDir::currentPath()+QLatin1String("/foo"));
   u = KCmdLineArgs::makeURL(QByteArray("http://www.kde.org"));
   qDebug() << u;
   assert(u.toString() == QLatin1String("http://www.kde.org"));

   QFile file(QLatin1String("a:b"));
#ifndef Q_OS_WIN
   bool ok = file.open(QIODevice::WriteOnly);
   Q_UNUSED(ok) // silence warnings
   assert(ok);
#endif
   u = KCmdLineArgs::makeURL(QByteArray("a:b"));
   qDebug() << u.toLocalFile();
   assert(u.isLocalFile());
   assert(u.toLocalFile().endsWith(QLatin1String("a:b")));

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


