#include <kjavaappletcontext.h>
#include "kjavaapplet.moc"

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
};

KJavaApplet::KJavaApplet( KJavaAppletContext *context )
    : params()
{
  d = new KJavaAppletPrivate;
  CHECK_PTR( d );

  if ( context != 0 )
    this->context = context;
  else
    this->context = KJavaAppletContext::getDefaultContext();
  
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
    if ( d->reallyExists )
	context->setParameter( this, name, value );
    else
	params.insert( name, value );
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

    ParamMap::Iterator it;

    if ( params.count() != 0 )
    {
        for ( it = params.begin(); it != params.end(); ++it )
        {
            context->setParameter( this, it.key(), it.data() );
        }
    }
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


