#include "kjavaappletwidget.h"

#include <kwinmodule.h>
#include <kwin.h>
#include <kdebug.h>

#include <qtimer.h>
#include <qapplication.h>


// For future expansion
struct KJavaAppletWidgetPrivate
{
};

static unsigned int count = 0;

KJavaAppletWidget::KJavaAppletWidget( KJavaAppletContext* context,
                                      QWidget* parent, const char* name )
   : JavaEmbed( parent, name )
{
    applet = new KJavaApplet( this, context );
    init();
}

KJavaAppletWidget::KJavaAppletWidget( KJavaApplet* _applet,
                                      QWidget* parent, const char* name )
   : JavaEmbed( parent, name )
{
    applet = _applet;
    init();
}

KJavaAppletWidget::KJavaAppletWidget( QWidget* parent, const char* name )
   : JavaEmbed( parent, name )
{
    KJavaAppletContext* context = KJavaAppletContext::getDefaultContext();
    applet = new KJavaApplet( this, context );
    init();
}

KJavaAppletWidget::~KJavaAppletWidget()
{
    delete applet;
}

void KJavaAppletWidget::init()
{
    kwm = new KWinModule( this );

    uniqueTitle();
    shown = false;
}

void KJavaAppletWidget::create()
{
//    applet->create();
}

void KJavaAppletWidget::setAppletClass( const QString &clazzName )
{
   applet->setAppletClass( clazzName );
}

QString &KJavaAppletWidget::appletClass()
{
   return applet->appletClass();
}

void KJavaAppletWidget::setAppletName( const QString &name )
{
    applet->setAppletName( name );
}

QString &KJavaAppletWidget::appletName()
{
    return applet->appletName();
}

void KJavaAppletWidget::setJARFile( const QString &jar )
{
   applet->setJARFile( jar );
}

QString &KJavaAppletWidget::jarFile()
{
   return applet->jarFile();
}

void KJavaAppletWidget::setParameter( const QString &name, const QString &value )
{
   applet->setParameter( name, value );
}

QString &KJavaAppletWidget::parameter( const QString &name )
{
    return applet->parameter( name );
}

void KJavaAppletWidget::setBaseURL( const QString &base )
{
   applet->setBaseURL( base );
}

QString &KJavaAppletWidget::baseURL()
{
   return applet->baseURL();
}

void KJavaAppletWidget::setCodeBase( const QString &codeBase )
{
   applet->setCodeBase( codeBase );
}

QString &KJavaAppletWidget::codeBase()
{
   return applet->codeBase();
}

void KJavaAppletWidget::uniqueTitle()
{
   swallowTitle.sprintf( "KJAS Applet - Ticket number %u", count );
   count++;
}

void KJavaAppletWidget::showApplet()
{
    //Now we send applet info to the applet server

    if ( !applet->isCreated() )
        applet->create();

    connect( kwm, SIGNAL( windowAdded( WId ) ),
	         this, SLOT( setWindow( WId ) ) );
   
    kwm->doNotManage(swallowTitle);
    applet->show( swallowTitle );
   
    shown = true;
}

void KJavaAppletWidget::start()
{
    applet->start();
}

void KJavaAppletWidget::stop()
{
    applet->stop();
}

void KJavaAppletWidget::setWindow( WId w )
{
    //make sure that this window has the right name, if so, embed it...
    KWin::Info w_info = KWin::info( w );

    if ( swallowTitle == w_info.name ||
         swallowTitle == w_info.visibleName )
    {
        kdDebug(6100) << "swallowing our window: " << swallowTitle
                      << ", window id = " << w << endl;

        // disconnect from KWM events
        disconnect( kwm, SIGNAL( windowAdded( WId ) ),
                    this, SLOT( setWindow( WId ) ) );

        embed( w );
    }
}

void KJavaAppletWidget::resize( int w, int h)
{
    JavaEmbed::resize( w, h );
    applet->setSize( QSize(w, h) );
}

#include "kjavaappletwidget.moc"
