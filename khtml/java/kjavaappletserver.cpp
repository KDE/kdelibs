#include <kjavaappletserver.moc>
#include <kjavaprocess.h>

KJavaAppletServer::KJavaAppletServer()
{
  process = new KJavaProcess();
  CHECK_PTR( process );

  process->setMainClass( "org.kde.kjas.server.Main" );
  process->startJava();
}

KJavaAppletServer *KJavaAppletServer::getDefaultServer()
{
  static KJavaAppletServer *server = 0;

  if ( server == 0 ) {
    server = new KJavaAppletServer();
    CHECK_PTR( server );
  }

  return server;
}

void KJavaAppletServer::createContext( int contextId )
{
    QString s;
    s = "createContext!%1\n";
    s.arg(contextId);

    process->send( s );
}

void KJavaAppletServer::createApplet( int contextId, int appletId,
				      const QString name, const QString clazzName,
				      const QString base )
{
    QString s;
    s = "createApplet!%1!%2!%3!%4!%5\n";
    s.arg(contextId).arg(appletId).arg(name).arg(clazzName).arg(base);

    process->send( s );

}

void KJavaAppletServer::setParameter( int contextId, int appletId,
				      const QString name, const QString value )
{
    QString s;
    s = "setParameter!%1!%2!%3!%4\n";
    s.arg(contextId).arg(appletId).arg(name).arg(value);

    process->send( s );
}

void KJavaAppletServer::showApplet( int contextId, int appletId,
				    const QString title )
{
    QString s;
    s = "showApplet!%1!%2!%3\n";
    s.arg(contextId).arg(appletId).arg(title);

    process->send( s );
}

void KJavaAppletServer::quit()
{
    QString s;
    s = "quit\n";
    process->send( s );
}

