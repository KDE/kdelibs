#include "kjavaappletwidget.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <qtimer.h>
#include <kwmmapp.h>

KJavaAppletWidget::KJavaAppletWidget( KJavaAppletContext *context,
                                      QWidget *parent=0, const char *name=0 )
   : QWidget( parent, name )
{
   applet = new KJavaApplet( context );
   CHECK_PTR( applet );

   uniqueTitle();
   shown = false;
}

KJavaAppletWidget::KJavaAppletWidget( KJavaApplet *applet,
                                      QWidget *parent=0, const char *name=0 )
   : QWidget( parent, name )
{
   this->applet = applet;

   uniqueTitle();
   shown = false;
}

KJavaAppletWidget::KJavaAppletWidget( QWidget *parent, const char *name )
   : QWidget( parent, name )
{
   KJavaAppletContext *context = KJavaAppletContext::getDefaultContext();

   applet = new KJavaApplet( context );
   CHECK_PTR( applet );

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

void KJavaAppletWidget::setAppletClass( QString clazzName )
{
   applet->setAppletClass( clazzName );
}

QString KJavaAppletWidget::appletClass()
{
   return applet->appletClass();
}

void KJavaAppletWidget::setAppletName( QString name )
{
    applet->setAppletName( name );
}

QString KJavaAppletWidget::appletName()
{
    return applet->appletName();
}

void KJavaAppletWidget::setJARFile( QString jar )
{
   applet->setJARFile( jar );
}

QString KJavaAppletWidget::jarFile()
{
   return applet->jarFile();
}

void KJavaAppletWidget::setParameter( QString name, QString value )
{
   applet->setParameter( name, value );
}

QString KJavaAppletWidget::parameter( QString name )
{
    //   return applet->parameter( name );
}

void KJavaAppletWidget::setBaseURL( QString base )
{
   applet->setBaseURL( base );
}

QString KJavaAppletWidget::baseURL()
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

    // connect to KWM events to get notification if window appears
 connect( kapp, SIGNAL( windowAdd( Window ) ),
	  this, SLOT( setWindow( Window ) ) );

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

void KJavaAppletWidget::setWindow( Window w )
{
   XTextProperty titleProperty;

   XGetWMName( qt_xdisplay(), w, &titleProperty );

   if ( strcmp( swallowTitle.data(), (char *) titleProperty.value ) == 0 )
       {
	   swallowWindow( w );
         //setFocus(); //workaround (ettrich)
	   
         // disconnect from KWM events
         disconnect( kapp, SIGNAL( windowAdd( Window ) ),
                     this, SLOT( setWindow( Window ) ) );
         //QTimer *t = new QTimer(this);
         //connect(t, SIGNAL(timeout()), SLOT(slotSave()));
         //t->start(5000);
       }
}

void KJavaAppletWidget::swallowWindow( Window w )
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


