#include "kjavaappletwidget.moc"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <qtimer.h>
#include <kwinmodule.h>

// For future expansion
struct KJavaAppletWidgetPrivate
{

};

static unsigned int count = 0;

KJavaAppletWidget::KJavaAppletWidget( KJavaAppletContext *context,
                                      QWidget *parent, const char *name )
   : QWidget( parent, name )
{
   applet = new KJavaApplet( context );
   CHECK_PTR( applet );

   kwm = new KWinModule( this );
   CHECK_PTR( kwm );

   uniqueTitle();
   shown = false;
}

KJavaAppletWidget::KJavaAppletWidget( KJavaApplet *applet,
                                      QWidget *parent, const char *name )
   : QWidget( parent, name )
{
   this->applet = applet;

   kwm = new KWinModule( this );
   CHECK_PTR( kwm );

   uniqueTitle();
   shown = false;
}

KJavaAppletWidget::KJavaAppletWidget( QWidget *parent, const char *name )
   : QWidget( parent, name )
{
   KJavaAppletContext *context = KJavaAppletContext::getDefaultContext();

   applet = new KJavaApplet( context );
   CHECK_PTR( applet );

   kwm = new KWinModule( this );
   CHECK_PTR( kwm );

   uniqueTitle();
   shown = false;
}

KJavaAppletWidget::~KJavaAppletWidget()
{
  delete applet;
}

void KJavaAppletWidget::create()
{
    applet->create();
}

void KJavaAppletWidget::show()
{
    if (!shown) {
	if ( !applet->isCreated() )
	    applet->create();

	showApplet();
	shown = true;
    }

    QWidget::show();
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
   // Avoid flickering a la kwm! (ettrich)
   KWM::doNotManage( swallowTitle );

   connect( kwm, SIGNAL( windowAdd( WId ) ),
	    this, SLOT( setWindow( WId ) ) ); 

   applet->show( swallowTitle );
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
   XTextProperty titleProperty;
   
   XGetWMName( qt_xdisplay(), w, &titleProperty );

   if ( strcmp( swallowTitle.data(), (char *) titleProperty.value ) == 0 )
       {
         swallowWindow( w );
	   
         // disconnect from KWM events
         disconnect( kwm, SIGNAL( windowAdd( WId ) ),
                     this, SLOT( setWindow( WId ) ) );
       }
}

void KJavaAppletWidget::swallowWindow( WId w )
{
   window = w;
   
   KWM::prepareForSwallowing( w );
   
   XReparentWindow( qt_xdisplay(), w, winId(), 0, 0 );
   XMapRaised( qt_xdisplay(), w );
   XResizeWindow( qt_xdisplay(), window, width(), height() );
}

void KJavaAppletWidget::resizeEvent( QResizeEvent * )
{
   if ( window != 0 )
      XResizeWindow( qt_xdisplay(), window, width(), height() );
}

void KJavaAppletWidget::resize( int w, int h)
{
  QWidget::resize( w, h );
  applet->setSize( QSize(w, h) );
}

void KJavaAppletWidget::closeEvent( QCloseEvent *e )
{
   if ( window )
      KWM::close( window );
   else
      e->accept();
}


