#include "kjavaappletwidget.h"
#include "kjavaappletcontext.h"


typedef QMap<QString, QString> ParamMap;

struct KJavaAppletPrivate
{
   bool reallyExists;
   QString clazzName;
   QString appName;
   QString jar;
   QString baseURL;
   QString codeBase;
   QSize size;
   KJavaAppletWidget* UIwidget;
};

KJavaApplet::KJavaApplet( KJavaAppletContext* _context )
    : params()
{
    d = new KJavaAppletPrivate;
    CHECK_PTR( d );

    if ( context != 0 )
        context = _context;
    else
        context = KJavaAppletContext::getDefaultContext();
  
    d->reallyExists = false;
}

KJavaApplet::KJavaApplet( KJavaAppletWidget* _parent,
                          KJavaAppletContext* _context )
{
    d = new KJavaAppletPrivate;

    d->UIwidget = _parent;

    if( _context )
        context = _context;
    else
        context = KJavaAppletContext::getDefaultContext();

    d->reallyExists = false;
}

KJavaApplet::~KJavaApplet()
{
    if ( d->reallyExists )
        context->destroy( this );

    delete d;
}

bool KJavaApplet::isCreated()
{
    return d->reallyExists;
}

void KJavaApplet::setAppletClass( const QString &clazzName )
{
    d->clazzName = clazzName;
}

QString &KJavaApplet::appletClass()
{
    return d->clazzName;
}

void KJavaApplet::setJARFile( const QString &jar )
{
    d->jar = jar;
}

QString &KJavaApplet::jarFile()
{
    return d->jar;
}

QString &KJavaApplet::parameter( const QString &name )
{
    return params[ name ];
}

void KJavaApplet::setParameter( const QString &name, const QString &value )
{
    params.insert( name, value );
}

QMap< QString, QString >& KJavaApplet::getParams()
{
    return params;
}

void KJavaApplet::setBaseURL( const QString &baseURL )
{
    d->baseURL = baseURL;
}

QString &KJavaApplet::baseURL()
{
    return d->baseURL;
}

void KJavaApplet::setCodeBase( const QString &codeBase )
{
    d->codeBase = codeBase;
}

QString &KJavaApplet::codeBase()
{
    return d->codeBase;
}

void KJavaApplet::setSize( QSize size )
{
    d->size = size;
}

QSize KJavaApplet::size()
{
    return d->size;
}

void KJavaApplet::resizeAppletWidget( int width, int height )
{
    if( d->UIwidget )
        d->UIwidget->resize( width, height );
}

void KJavaApplet::setAppletName( const QString &name )
{
    d->appName = name;
}

QString &KJavaApplet::appletName()
{
    return d->appName;
}

void KJavaApplet::create( )
{
    context->create( this );
    d->reallyExists = true;
}

void KJavaApplet::show( const QString &title )
{
    context->show( this, title );
}

void KJavaApplet::start()
{
    context->start( this );
}

void KJavaApplet::stop()
{
    context->stop( this );
}

int KJavaApplet::appletId()
{
    return id;
}

void KJavaApplet::setAppletId( int id )
{
    this->id = id;
}

#include "kjavaapplet.moc"
