#include <kapp.h>
#include <kjavaprocess.h>
#include <qstring.h>
#include <stdio.h>

int main(int argc, char **argv)
{
  KJavaProcess *kjp = new KJavaProcess();
  kjp->setMainClass( "org.kde.kjas.server.Main" );
  kjp->startJava();

  QString s;
  
  s = "createContext!0\n";
  kjp->send( s );

  //  s = "createApplet!0!0!fred!Lake.class!http://127.0.0.1/applets/\n";
  s = "createApplet!0!0!fred!Lake.class!file:/dos/pcplus/java/lake/\n";
  kjp->send( s );

  s = "setParameter!0!0!image!arch.jpg\n";
  kjp->send( s );

  s = "showApplet!0!0!unique_title_one\n";
  kjp->send( s );

  KApplication app( argc, argv );
  app.exec();
}
