#include "kjavaappletcontext.h"

#include <kjavaappletserver.h>
#include <kjavaapplet.h>
#include <kdebug.h>
#include <qmap.h>
#include <qguardedptr.h>

// For future expansion
struct KJavaAppletContextPrivate
{
    QMap< int, QGuardedPtr<KJavaApplet> > applets;
};


/*  Static Factory Functions
 *
 */
KJavaAppletContext* KJavaAppletContext::getDefaultContext()
{
   static KJavaAppletContext* context = 0;

    if ( context == 0 )
    {
        context = new KJavaAppletContext();
        CHECK_PTR( context );
    }

    return context;
}


/*  Class Implementation
 */
KJavaAppletContext::KJavaAppletContext()
    : QObject()
{
    d = new KJavaAppletContextPrivate;
    server = KJavaAppletServer::allocateJavaServer();

    static int contextIdSource = 0;

    id = contextIdSource;
    server->createContext( contextIdSource, this );

    contextIdSource++;
}

KJavaAppletContext::~KJavaAppletContext()
{
    server->destroyContext( id );
    KJavaAppletServer::freeJavaServer();
    delete d;
}

int KJavaAppletContext::contextId()
{
    return id;
}

void KJavaAppletContext::setContextId( int _id )
{
    id = _id;
}

void KJavaAppletContext::create( KJavaApplet* applet )
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

void KJavaAppletContext::destroy( KJavaApplet* applet )
{
    int appletId = applet->appletId();
    d->applets.remove( appletId );

    server->destroyApplet( id, appletId );
}

void KJavaAppletContext::setParameter( KJavaApplet* applet,
                                       const QString& name, const QString& value )
{
    server->setParameter( id, applet->appletId(),
                          name, value );
}

void KJavaAppletContext::show( KJavaApplet* applet, const QString& title )
{
    server->showApplet( id, applet->appletId(), title );
}

void KJavaAppletContext::start( KJavaApplet* applet )
{
    server->startApplet( id, applet->appletId() );
}

void KJavaAppletContext::stop( KJavaApplet* applet )
{
    server->stopApplet( id, applet->appletId() );
}

void KJavaAppletContext::processCmd( QString cmd, QStringList args )
{
    received( cmd, args );
}

void KJavaAppletContext::received( const QString& cmd, const QStringList& arg )
{
    kdDebug() << "KJavaAppletContext::received, cmd = >>" << cmd << "<<" << endl;
    kdDebug() << "arg count = " << arg.count() << endl;;

    if ( cmd=="showstatus" && arg.count()>0 )
    {
        kdDebug() << "status message = " << arg[0] << endl;
        emit showStatus( arg[0] );
    }
    else if ( cmd=="showurlinframe" && arg.count()>1 )
    {
        kdDebug() << "url = " << arg[0] << ", frame = " << arg[1] << endl;
        emit showDocument( arg[0], arg[1] );
    }
    else if ( cmd=="showdocument" && arg.count()>0 )
    {
        kdDebug() << "url = " << arg[0] << endl;
        emit showDocument( arg[0], "_top" );
    }
    else if ( cmd=="resizeapplet" )
    {
        //arg[1] should be appletID
        //arg[2] should be new width
        //arg[3] should be new height
        bool ok;
        int appletID = arg[0].toInt( &ok );
        int width = arg[1].toInt( &ok );
        int height = arg[2].toInt( &ok );

        if( !ok )
        {
            kdError() << "could not parse out parameters for resize" << endl;
        }
        else
        {
            KJavaApplet* tmp = d->applets[appletID];
            tmp->resizeAppletWidget( width, height );
        }

    }
}

#include <kjavaappletcontext.moc>
