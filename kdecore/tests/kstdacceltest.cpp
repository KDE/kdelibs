#include <config.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kstdaccel.h>
#include <stdlib.h> // for exit

static bool check(QString txt, QString a, QString b)
{
  if (a.isEmpty())
     a = QString::null;
  if (b.isEmpty())
     b = QString::null;
  if (a == b) {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "ok" << endl;
  }
  else {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "KO !" << endl;
    exit(1);
  }
  return true;
}

int main(int argc, char *argv[])
{
  KApplication::disableAutoDcopRegistration();
  KApplication app(argc,argv,"kstdacceltest",false,false);

  check( "shortcutDefault FullScreen", KStdAccel::shortcutDefault( KStdAccel::FullScreen ).toString(), "Ctrl+Shift+F" );
  check( "shortcutDefault BeginningOfLine", KStdAccel::shortcutDefault( KStdAccel::BeginningOfLine ).toString(), "Home" );
  check( "shortcutDefault EndOfLine", KStdAccel::shortcutDefault( KStdAccel::EndOfLine ).toString(), "End" );

  check( "name BeginningOfLine", KStdAccel::name( KStdAccel::BeginningOfLine ), "BeginningOfLine" );
  check( "name EndOfLine", KStdAccel::name( KStdAccel::EndOfLine ), "EndOfLine" );

  check( "shortcut method", KStdAccel::shortcut( KStdAccel::ZoomIn ).toString(), KStdAccel::zoomIn().toString() );

  return 0;
}
