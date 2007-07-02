/*
 testqtargs -- is there really a bug in KCmdLineArgs or am I on crack?

  I used the following compile options:

  g++ -g -Wall -o testqtargs testqtargs.cpp  -I/usr/X11R6/include \
  -I/opt/qt3/include -I/opt/kde3/include -L/usr/X11R6/lib -L/opt/qt3/lib \
  -L/opt/kde3/lib -lqt -lkdecore

  if invoked like this ./testqtargs --bg blue --caption something --hello hi

  The program should list argv[] then produce output like this:
  
  qt arg[0] = background
  qt arg[1] = blue
  arg bg = blue
  arg caption = something
  arg hello = hi

  Instead for me it prints:

  qt arg[0] = -background
  qt arg[1] = blue
  arg caption = something
  arg hello = hi

  See the extra dash in qt arg[0]?  I believe that is the cause of the problem.
  --bg is aliased to --background but If you try it with --background or 
  -background, you get the same thing.

  in kdecore/kapplication.cpp, KCmdLineOption qt_options is defined and used 
  by the static method Kapplication::addCmdLineOptions to add the Qt options
  but its' entries look like this:

  { "background <color>", I18N_NOOP("sets the default background color and an\n
application palette (light and dark shades are\ncalculated)."), 0},

  it looks for "background"  instead of "-background" so never find the arg.

  Software:  g++ 2.95, kdelibs from CVS Jan 28, Qt 3.01
  OS: Debian GNU/Linux 3.0 (sid)


*/

#include <QtGui/QWidget>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>

int main(int argc, char *argv[])
{
  for (int i = 0; i < argc; i++)
  {
    qDebug("argv[%d] = %s", i, argv[i]);
  }
  KAboutData aboutData( "testqtargs", 0, ki18n("testqtargs"),
    "1.0", ki18n("testqtargs"), KAboutData::License_GPL);
	
  KCmdLineOptions options;
  options.add("hello ", ki18n("Says hello"));

  KCmdLineArgs::init(argc, argv, &aboutData);
  KCmdLineArgs::addCmdLineOptions(options);

  KCmdLineArgs *qtargs = KCmdLineArgs::parsedArgs("qt");
  for (int i = 0; i < qtargs->count(); i++)
  {
    qDebug("qt arg[%d] = %s", i, qtargs->arg(i).toLocal8Bit().data());
  }

  KApplication app;

  KCmdLineArgs *kdeargs = KCmdLineArgs::parsedArgs("kde");
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  // An arg set by Qt
  if(qtargs->isSet("background"))
  {
    qDebug("arg bg = %s", qtargs->getOption("background").toLocal8Bit().data());
  }
  // An arg set by KDE
  if(kdeargs->isSet("caption"))
  {
    qDebug("arg caption = %s", kdeargs->getOption("caption").toLocal8Bit().data());
  }
  // An arg set by us.
  if(args->isSet("hello"))
  {
    qDebug("arg hello = %s", args->getOption("hello").toLocal8Bit().data());
  }
  args->clear();

  QWidget *w = new QWidget();
  w->show();

  return app.exec();
}

