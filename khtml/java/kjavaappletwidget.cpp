#include "kjavaappletwidget.moc"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <qtimer.h>
#include <kwinmodule.h>

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

void KJavaAppletWidget::create()
{
    applet->create();
}

void KJavaAppletWidget::show()
{
    if (!shown) {
	if ( !applet->isCreated() )
	    applet->create();

// 	QTimer *t = new QTimer( this );
// 	connect( t, SIGNAL( timeout() ),
// 		 this, SLOT( showApplet() ) );
// 	t->start(250, true);
	showApplet();
	shown = true;
    }

    QWidget::show();
}

void KJavaAppletWidget::setAppletClass( const QString clazzName )
{
   applet->setAppletClass( clazzName );
}

const QString KJavaAppletWidget::appletClass()
{
   return applet->appletClass();
}

void KJavaAppletWidget::setAppletName( const QString name )
{
    applet->setAppletName( name );
}

const QString KJavaAppletWidget::appletName()
{
    return applet->appletName();
}

void KJavaAppletWidget::setJARFile( const QString jar )
{
   applet->setJARFile( jar );
}

const QString KJavaAppletWidget::jarFile()
{
   return applet->jarFile();
}

void KJavaAppletWidget::setParameter( const QString name, const QString value )
{
   applet->setParameter( name, value );
}

const QString KJavaAppletWidget::parameter( const QString name )
{
    //   return applet->parameter( name );
}

void KJavaAppletWidget::setBaseURL( const QString base )
{
   applet->setBaseURL( base );
}

const QString KJavaAppletWidget::baseURL()
{
   return applet->baseURL();
}

void KJavaAppletWidget::uniqueTitle()
{
   static unsigned int count = 0;
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
         //setFocus(); //workaround (ettrich)
	   
         // disconnect from KWM events
         disconnect( kwm, SIGNAL( windowAdd( WId ) ),
                     this, SLOT( setWindow( WId ) ) );
         //QTimer *t = new QTimer(this);
         //connect(t, SIGNAL(timeout()), SLOT(slotSave()));
         //t->start(5000);
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

void KJavaAppletWidget::focusInEvent( QFocusEvent * )
{
   // workarund: put the focus onto the swallowed widget (ettrich)
   // TODO: When we switch to a newer qt than qt-1.33 this hack should
   // be replaced with my new kswallow widget!
   if ( isActiveWindow() && isVisible() ) { // isActiveWindow is important here!
      // verify wether the window still belongs to us
      unsigned int nwins;
      Window dw1, dw2, *wins;
      XQueryTree( qt_xdisplay(), winId(), 
                  &dw1, &dw2, &wins, &nwins );
      if ( nwins )
         XSetInputFocus( qt_xdisplay(), window, RevertToParent, CurrentTime );
   }
}

void KJavaAppletWidget::resizeEvent( QResizeEvent * )
{
   if ( window != 0 )
      XResizeWindow( qt_xdisplay(), window, width(), height() );
}

void KJavaAppletWidget::closeEvent( QCloseEvent *e )
{
   if ( window )
      KWM::close( window );
   else
      e->accept();
}


