#include <kjavaappletcontext.moc>
#include <kjavaappletserver.h>
#include <kjavaapplet.h>
#include <kdebug.h>
#include <qmap.h>

// For future expansion
struct KJavaAppletContextPrivate
{
    QMap<int,KJavaApplet*> applets;
};

KJavaAppletContext::KJavaAppletContext()
  : QObject()
{
   d = new KJavaAppletContextPrivate;

   server = KJavaAppletServer::allocateJavaServer();
   connect( server, SIGNAL(receivedCommand(const QString&,const QStringList&)),
            this, SLOT(received(const QString&,const QStringList&)) );
   static int contextIdSource = 0;

   setContextId( contextIdSource );
   server->createContext( contextIdSource );

   contextIdSource++;
}

KJavaAppletContext::~KJavaAppletContext()
{
   server->destroyContext( id );
   KJavaAppletServer::freeJavaServer();
   delete d;
}

KJavaAppletContext *KJavaAppletContext::getDefaultContext()
{
   static KJavaAppletContext *context = 0;

   if ( context == 0 ) {
      context = new KJavaAppletContext();
      CHECK_PTR( context );
   }

   return context;
}

int KJavaAppletContext::contextId()
{
   return id;
}

void KJavaAppletContext::setContextId( int id )
{
   this->id = id;
}

void KJavaAppletContext::create( KJavaApplet *applet )
{
    static int appletId = 0;

    server->createApplet( id, appletId,
                          applet->appletName(),
                          applet->appletClass(),
                          applet->baseURL(),
                          applet->codeBase(),
                          applet->jarFile(),
                          applet->size() );

    applet->setAppletId( appletId );
    d->applets.insert( appletId, applet );
    appletId++;
}

void KJavaAppletContext::destroy( KJavaApplet *applet )
{
  int appletId = applet->appletId();
  d->applets.remove( appletId );
  server->destroyApplet( id, appletId );
}

void KJavaAppletContext::setParameter( KJavaApplet *applet,
                                       const QString &name, const QString &value )
{
    server->setParameter( id, applet->appletId(),
                          name, value );
}

void KJavaAppletContext::show( KJavaApplet *applet, const QString &title )
{
    server->showApplet( id, applet->appletId(), title );
}

void KJavaAppletContext::start( KJavaApplet *applet )
{
    server->startApplet( id, applet->appletId() );
}

void KJavaAppletContext::stop (KJavaApplet *applet )
{
    server->stopApplet( id, applet->appletId() );
}


void KJavaAppletContext::received( const QString &cmd, const QStringList &arg )
{
    if ( cmd=="showstatus" && arg.count()>0 )
        emit showStatus( arg[0] );
    else if ( cmd=="showurlinframe" && arg.count()>1 )
        emit showDocument( arg[0], arg[1] );
    else if ( cmd=="showdocument" && arg.count()>0 )
        emit showDocument( arg[0], "_top" );
}
