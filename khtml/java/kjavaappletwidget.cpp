#include "kjavaappletwidget.moc"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <qtimer.h>
#include <kwinmodule.h>
#include <kwin.h>
#include <netwm.h>

#include <unistd.h>

// For future expansion
struct KJavaAppletWidgetPrivate
{

};

static unsigned int count = 0;

KJavaAppletWidget::KJavaAppletWidget( KJavaAppletContext *context,
                                      QWidget *parent, const char *name )
   : QXEmbed( parent, name )
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
   : QXEmbed( parent, name )
{
   this->applet = applet;

   kwm = new KWinModule( this );
   CHECK_PTR( kwm );

   uniqueTitle();
   shown = false;
}

KJavaAppletWidget::KJavaAppletWidget( QWidget *parent, const char *name )
   : QXEmbed( parent, name )
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
  XUnmapWindow(qt_xdisplay(), embeddedWinId());

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

    QXEmbed::show();
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
   connect( kwm, SIGNAL( windowAdded( WId ) ),
	    this, SLOT( setWindow( WId ) ) );

   kwm->doNotManage(swallowTitle);

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

   if ( swallowTitle == QString::fromLatin1((char*)titleProperty.value))
       {
         swallowWindow( w );
	
         // disconnect from KWM events
         disconnect( kwm, SIGNAL( windowAdded( WId ) ),
                     this, SLOT( setWindow( WId ) ) );
       }
}


extern Atom qt_wm_state; // defined in qapplication_x11.cpp
static bool wstate_withdrawn( WId winid )
{
    Atom type;
    int format;
    unsigned long length, after;
    unsigned char *data;
    int r = XGetWindowProperty( qt_xdisplay(), winid, qt_wm_state, 0, 2,
				FALSE, AnyPropertyType, &type, &format,
				&length, &after, &data );
    bool withdrawn = TRUE;
    if ( r == Success && data && format == 32 ) {
	Q_UINT32 *wstate = (Q_UINT32*)data;
	withdrawn  = (*wstate == WithdrawnState );
	XFree( (char *)data );
    }
    return withdrawn;
}


void KJavaAppletWidget::swallowWindow( WId w )
{

   window = w;

   XWithdrawWindow(qt_xdisplay(), window, qt_xscreen());   
   QApplication::flushX();

   while (!wstate_withdrawn(window))
     ;

   embed(window);
}

void KJavaAppletWidget::resize( int w, int h)
{
  QXEmbed::resize( w, h );
  applet->setSize( QSize(w, h) );
}


