#include <kjavaappletserver.moc>
#include <kjavaprocess.h>

// For future expansion
struct KJavaAppletServerPrivate
{

};

static KJavaAppletServer *server = 0;

KJavaAppletServer::KJavaAppletServer()
{
  process = new KJavaProcess();
  CHECK_PTR( process );

  process->setMainClass( "org.kde.kjas.server.Main" );
  process->startJava();
}

KJavaAppletServer *KJavaAppletServer::getDefaultServer()
{
  if ( server == 0 ) {
    server = new KJavaAppletServer();
    CHECK_PTR( server );
  }

  return server;
}

void KJavaAppletServer::killDefaultServer()
{
    server->quit();
    server = 0;
}

void KJavaAppletServer::createContext( int contextId )
{
    QString s;
    s.sprintf( "createContext!%d\n", contextId );
    process->send( s );
}

void KJavaAppletServer::destroyContext( int contextId )
{
    QString s;
    s.sprintf( "destroyContext!%d\n", contextId );
    process->send( s );
}

void KJavaAppletServer::createApplet( int contextId, int appletId,
				      const QString name, const QString clazzName,
				      const QString base )
{
    QString s;
    s.sprintf( "createApplet!%d!%d!%s!%s!%s\n",
	       contextId, appletId,
	       name.data(), clazzName.data(),
	       base.data() );
    process->send( s );

}

void KJavaAppletServer::destroyApplet( int contextId, int appletId )
{
    QString s;
    s.sprintf( "destroyApplet!%d!%d\n",
	       contextId, appletId );
    process->send( s );
}


void KJavaAppletServer::setParameter( int contextId, int appletId,
				      const QString name, const QString value )
{
    QString s;
    s.sprintf( "setParameter!%d!%d!%s!%s\n",
	       contextId, appletId,
	       name.data(), value.data() );
    process->send( s );
}

void KJavaAppletServer::showApplet( int contextId, int appletId,
				    const QString title )
{
    QString s;
    s.sprintf( "showApplet!%d!%d!%s\n",
	       contextId, appletId,
	       title.data() );
    process->send( s );
}

void KJavaAppletServer::startApplet( int contextId, int appletId )
{
    QString s;
    s.sprintf( "startApplet!%d!%d\n",
	       contextId, appletId );
    process->send( s );
}

void KJavaAppletServer::stopApplet( int contextId, int appletId )
{
    QString s;
    s.sprintf( "stopApplet!%d!%d\n",
	       contextId, appletId );
    process->send( s );
}

void KJavaAppletServer::quit()
{
    QString s;
    s = "quit\n";
    process->send( s );
}

