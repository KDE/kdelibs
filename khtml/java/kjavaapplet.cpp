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

void KJavaApplet::setAppletClass( QString clazzName )
{
   this->clazzName = clazzName;
}

QString KJavaApplet::appletClass()
{
   return clazzName;
}

void KJavaApplet::setJARFile( QString jar )
{
   this->jar = jar;
}

QString KJavaApplet::jarFile()
{
   return jar;
}

void KJavaApplet::setParameter( QString name, QString value )
{
  context->setParameter( this, name, value );
}

void KJavaApplet::setBaseURL( QString base )
{
   this->base = base;
}

QString KJavaApplet::baseURL()
{
   return base;
}

void KJavaApplet::setAppletName( QString name )
{
    appName = name;
}

QString KJavaApplet::appletName()
{
    return appName;
}

void KJavaApplet::create( )
{
  context->create( this );
  reallyExists = true;
}

void KJavaApplet::show( QString title )
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


