#include <kjavaappletserver.h>
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
    s.sprintf( "createContext!%d\n", contextId );
    process->send( s );
}

void KJavaAppletServer::createApplet( int contextId, int appletId,
				      QString name, QString clazzName,
				      QString base )
{
    QString s;
    s.sprintf( "createApplet!%d!%d!%s!%s!%s\n",
	       contextId, appletId,
	       name.data(), clazzName.data(),
	       base.data() );
    process->send( s );

}

void KJavaAppletServer::setParameter( int contextId, int appletId,
				      QString name, QString value )
{
    QString s;
    s.sprintf( "setParameter!%d!%d!%s!%s\n",
	       contextId, appletId,
	       name.data(), value.data() );
    process->send( s );
}

void KJavaAppletServer::showApplet( int contextId, int appletId,
				    QString title )
{
    QString s;
    s.sprintf( "showApplet!%d!%d!%s\n",
	       contextId, appletId,
	       title.data() );
    process->send( s );
}

void KJavaAppletServer::quit()
{
    QString s;
    s = "quit\n";
    process->send( s );
}

