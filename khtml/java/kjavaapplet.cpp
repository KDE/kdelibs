#include "kjavaapplet.moc"

KJavaApplet::KJavaApplet( KJavaAppletContext *context )
{
    if ( context != 0 )
	this->context = context;
    else
	this->context = KJavaAppletContext::getDefaultContext();

    reallyExists = false;
}

bool KJavaApplet::isCreated()
{
    return reallyExists;
}

void KJavaApplet::setAppletClass( const QString clazzName )
{
    this->clazzName = clazzName;
}

const QString KJavaApplet::appletClass()
{
    return clazzName;
}

void KJavaApplet::setJARFile( const QString jar )
{
    this->jar = jar;
}

const QString KJavaApplet::jarFile()
{
    return jar;
}

void KJavaApplet::setParameter( const QString name, const QString value )
{
    context->setParameter( this, name, value );
}

void KJavaApplet::setBaseURL( const QString base )
{
    this->base = base;
}

const QString KJavaApplet::baseURL()
{
    return base;
}

void KJavaApplet::setAppletName( const QString name )
{
    appName = name;
}

const QString KJavaApplet::appletName()
{
    return appName;
}

void KJavaApplet::create( )
{
  context->create( this );
  reallyExists = true;
}

void KJavaApplet::show( const QString title )
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


