/*
   This file is part of KDE/aRts (Noatun) - xine integration
   Copyright (C) 2002 Ewald Snel <ewald@rambo.its.tudelft.nl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <time.h>
#include <qaccel.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <kaction.h>
#include "kvideowidget.h"

#define VPO_ADD_WINDOW		1
#define VPO_DESTROY_NOTIFY	2
#define VPO_DISABLE_NOTIFY	3
#define VPO_ENABLE_NOTIFY	4
#define VPO_ENABLE_WINDOW	5
#define VPO_RELEASE_WINDOW	6
#define VPO_RESIZE_NOTIFY	7


static void sendEvent( Window window, long message, long arg1 = 0, long arg2 = 0 )
{
    XEvent event;
    Atom atom = XInternAtom( qt_xdisplay(), "VPO_X11_COMM", False );

    // Send X11 client message
    memset( &event, 0, sizeof(event) );

    event.type			= ClientMessage;
    event.xclient.window	= window;
    event.xclient.message_type	= atom;
    event.xclient.format	= 32;
    event.xclient.data.l[0]	= message;
    event.xclient.data.l[1]	= arg1;
    event.xclient.data.l[2]	= arg2;
    XSendEvent( qt_xdisplay(), window, False, ExposureMask, &event );

    XFlush( qt_xdisplay() );
}

static void releaseWinId( Window window )
{
    XTextProperty prop;
    Atom atom = XInternAtom( qt_xdisplay(), "VPO_X11_COMM", False );

    // Wait until the Window is released by the VideoPlayObject
    while (XGetTextProperty( qt_xdisplay(), window, &prop, atom ))
    {
	struct timespec ts;

	sendEvent( window, VPO_RELEASE_WINDOW );

	// Do not consume 100% CPU (wait 50ms)
	ts.tv_sec  = 0;
	ts.tv_nsec = 50000000;
	nanosleep( &ts, NULL );
    }
}


class KFullscreenVideoWidget : public KVideoWidget
{
public:
    KFullscreenVideoWidget( KVideoWidget *parent = 0, const char *name = 0 );

protected:
    virtual void windowActivationChange( bool );
    virtual bool x11Event( XEvent *event );

private:
    KVideoWidget *videoWidget;
};

KFullscreenVideoWidget::KFullscreenVideoWidget( KVideoWidget *parent, const char *name )
    : KVideoWidget( parent, name, WType_TopLevel | WStyle_Customize | WStyle_NoBorder )
{
    this->videoWidget = parent;
}

void KFullscreenVideoWidget::windowActivationChange( bool )
{
    if (!isActiveWindow())
    {
	videoWidget->setWindowed();
    }
}

bool KFullscreenVideoWidget::x11Event( XEvent *event )
{
    Atom atom = XInternAtom( qt_xdisplay(), "VPO_X11_COMM", False );

    if (event->type == ClientMessage && event->xclient.message_type == atom)
    {
	switch (event->xclient.data.l[0])
	{
	case VPO_DESTROY_NOTIFY:
	case VPO_DISABLE_NOTIFY:
	    setBackgroundMode( PaletteBackground );
	    repaint();
	    break;
	case VPO_ENABLE_NOTIFY:
	    setBackgroundMode( NoBackground );
	    break;
	}
    }
    return false;
}

KVideoWidget::KVideoWidget( KXMLGUIClient *clientParent, QWidget *parent, const char *name, WFlags f )
    : QWidget( parent, name, f )
    , KXMLGUIClient( clientParent )
{
	init();
	// ???
	// setXML("<!DOCTYPE kpartgui>\n<kpartgui name=\"kvideowidget\" version=\"1\"><MenuBar><Menu name=\"edit\"><Action name=\"fullscreen_mode\"><Separator/></Menu></MenuBar></kpartgui>");
}

KVideoWidget::KVideoWidget( QWidget *parent, const char *name, WFlags f )
    : QWidget( parent, name, f )
{
	init();
}

void KVideoWidget::init(void)
{
    setMinimumSize(0, 0);
    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum ) );
    emit adaptSize(0, 0);
    setFocusPolicy( ClickFocus );

    fullscreenWidget = 0;
    embedded	     = false;
    enabled	     = false;
    videoWidth	     = 0;
    videoHeight	     = 0;

    // Setup actions
    new KToggleAction( "Fullscreen &Mode", "window_fullscreen",
		       CTRL+SHIFT+Key_F, this, SLOT(fullscreenActivated()),
		       actionCollection(), "fullscreen_mode" );
    new KRadioAction( "&Half Size", ALT+Key_0,
		      this, SLOT(halfSizeActivated()),
		      actionCollection(), "half_size" );
    new KRadioAction( "&Normal Size", ALT+Key_1,
		      this, SLOT(normalSizeActivated()),
		      actionCollection(), "normal_size" );
    new KRadioAction( "&Double Size", ALT+Key_2,
		      this, SLOT(doubleSizeActivated()),
		      actionCollection(), "double_size" );

    ((KToggleAction *)action( "half_size" ))->setExclusiveGroup( "KVideoWidget::zoom" );
    ((KToggleAction *)action( "normal_size" ))->setExclusiveGroup( "KVideoWidget::zoom" );
    ((KToggleAction *)action( "double_size" ))->setExclusiveGroup( "KVideoWidget::zoom" );
}

KVideoWidget::~KVideoWidget()
{
    if (fullscreenWidget)
    {
	delete fullscreenWidget;
    }

    releaseWinId( winId() );
}

void KVideoWidget::embed( Arts::VideoPlayObject vpo )
{
    if (vpo.isNull())
    {
	if (embedded)
	{
	    releaseWinId( winId() );

	    if (fullscreenWidget)
	    {
		releaseWinId( fullscreenWidget->winId() );
	    }

	    embedded = false;

	    setBackgroundMode( PaletteBackground );
	    repaint();
	}

	// Resize GUI
	videoWidth  = 0;
	videoHeight = 0;
	setMinimumSize(0, 0);
	setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum ) );
	repaint();

	if (isHalfSize() || isNormalSize() || isDoubleSize())
	    emit adaptSize( 0, 0 );
    }
    else
    {
	embedded = true;

	// Don't reset fullscreen mode for video playlists
	if (fullscreenWidget)
	{
	    vpo.x11WindowId( fullscreenWidget->winId() );
	}

	vpo.x11WindowId( winId() );

	// Re-enable video widget
	if (fullscreenWidget)
	{
	    sendEvent( fullscreenWidget->winId(), VPO_ENABLE_WINDOW );
	}
	else if (enabled)
	{
	    sendEvent( winId(), VPO_ENABLE_WINDOW );
	}

	setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
	emit adaptSize(videoWidth, videoHeight);
    }
}

QImage KVideoWidget::snapshot( Arts::VideoPlayObject vpo )
{
    Window root;
    Pixmap pixmap;
    XImage *xImage;
    unsigned int width, height, border, depth;
    int x, y;

    if (vpo.isNull() || (long)(pixmap = vpo.x11Snapshot()) == -1)
	return QImage();

    // Get 32bit RGBA image data (stored in 1bpp pixmap)
    XGetGeometry( qt_xdisplay(), pixmap, &root, &x, &y, &width, &height, &border, &depth );

    xImage = XGetImage( qt_xdisplay(), pixmap, 0, 0, width, height, 1, XYPixmap );

    if (xImage == 0)
    {
	XFreePixmap( qt_xdisplay(), pixmap );
	return QImage();
    }

    // Convert 32bit RGBA image data into Qt image
    QImage qImage = QImage( (uchar *)xImage->data, width/32, height, 32, (QRgb *)0, 0, QImage::IgnoreEndian ).copy();

    // Free X11 resources and return Qt image
    XDestroyImage( xImage );
    XFreePixmap( qt_xdisplay(), pixmap );

    return qImage;
}

bool KVideoWidget::isEmbedded()
{
    return embedded;
}

bool KVideoWidget::isFullscreen()
{
    return ((KToggleAction *)action( "fullscreen_mode" ))->isChecked();
}

bool KVideoWidget::isHalfSize()
{
    return ((KToggleAction *)action( "half_size" ))->isChecked();
}

bool KVideoWidget::isNormalSize()
{
    return ((KToggleAction *)action( "normal_size" ))->isChecked();
}

bool KVideoWidget::isDoubleSize()
{
    return ((KToggleAction *)action( "double_size" ))->isChecked();
}

void KVideoWidget::setFullscreen()
{
    if (!isFullscreen())
    {
	((KToggleAction *)action( "fullscreen_mode" ))->setChecked( true );
	fullscreenActivated();
    }
}

void KVideoWidget::setWindowed()
{
    if (isFullscreen())
    {
	((KToggleAction *)action( "fullscreen_mode" ))->setChecked( false );
	fullscreenActivated();
    }
}

void KVideoWidget::setHalfSize()
{
    ((KToggleAction *)action( "half_size" ))->setChecked( true );
    halfSizeActivated();
}

void KVideoWidget::setNormalSize()
{
    ((KToggleAction *)action( "normal_size" ))->setChecked( true );
    normalSizeActivated();
}

void KVideoWidget::setDoubleSize()
{
    ((KToggleAction *)action( "double_size" ))->setChecked( true );
    doubleSizeActivated();
}

QSize KVideoWidget::sizeHint() const
{
    return QSize( videoWidth, videoHeight );
}

int KVideoWidget::heightForWidth( int w ) const
{
	if(videoWidth == 0)
		return 0;
	else
		return int( double(w)*double(videoHeight)/double(videoWidth) );
}

void KVideoWidget::focusInEvent( QFocusEvent * )
{
    sendEvent( winId(), VPO_ENABLE_WINDOW );
}

void KVideoWidget::mousePressEvent( QMouseEvent *event )
{
    if (event->button() == RightButton)
	emit rightButtonPressed( mapToGlobal( event->pos() ) );
}

void KVideoWidget::resizeEvent( QResizeEvent *event )
{
    QWidget::resizeEvent( event );

    if (width() > minimumWidth() || height() > minimumHeight())
    {
	if (width() == QMAX( (videoWidth / 2), minimumWidth() ) &&
	         height() == QMAX( (videoHeight / 2), minimumHeight() ))
	    ((KToggleAction *)action( "half_size" ))->setChecked( true );
	else if (width() == QMAX( videoWidth, minimumWidth() ) &&
		 height() == QMAX( videoHeight, minimumHeight() ))
	    ((KToggleAction *)action( "normal_size" ))->setChecked( true );
	else if (width() == QMAX( (2 * videoWidth), minimumWidth() ) &&
		 height() == QMAX( (2 * videoHeight), minimumHeight() ))
	    ((KToggleAction *)action( "double_size" ))->setChecked( true );
	else
	{
	    ((KToggleAction *)action( "half_size" ))->setChecked( false );
	    ((KToggleAction *)action( "normal_size" ))->setChecked( false );
	    ((KToggleAction *)action( "double_size" ))->setChecked( false );
	}
    }
}

bool KVideoWidget::x11Event( XEvent *event )
{
    Atom atom = XInternAtom( qt_xdisplay(), "VPO_X11_COMM", False );

    if (event->type == ClientMessage && event->xclient.message_type == atom)
    {
	switch (event->xclient.data.l[0])
	{
	case VPO_RESIZE_NOTIFY:
	    videoWidth  = event->xclient.data.l[1];
	    videoHeight = event->xclient.data.l[2];

	    if (isHalfSize())
		emit adaptSize( (videoWidth / 2), (videoHeight / 2) );
	    else if (isNormalSize())
		emit adaptSize( videoWidth, videoHeight );
	    else if (isDoubleSize())
		emit adaptSize( (2 * videoWidth), (2 * videoHeight) );
	    break;
	case VPO_DESTROY_NOTIFY:
	    setEraseColor(palette().active().background());
	    embedded = false;
	    break;
	case VPO_ENABLE_NOTIFY:
	    setEraseColor(black);
	    enabled = true;
	    break;
	case VPO_DISABLE_NOTIFY:
	    setEraseColor(palette().active().background());
	    enabled = false;
	    break;
	}
    }
    return false;
}

void KVideoWidget::fullscreenActivated()
{
    if (isFullscreen() && !fullscreenWidget)
    {
	fullscreenWidget = new KFullscreenVideoWidget( this );

	// Interconnect right mouse button signals
	connect( fullscreenWidget, SIGNAL(rightButtonPressed(const QPoint &)),
		 this, SIGNAL(rightButtonPressed(const QPoint &)) );

	// Leave fullscreen mode with <Escape> key
	QAccel *a = new QAccel( fullscreenWidget );
	a->connectItem( a->insertItem( Key_Escape ),
			this, SLOT(setWindowed()) );

	fullscreenWidget->showFullScreen();
	fullscreenWidget->setFocus();

	// Add fullscreen window
	sendEvent( winId(), VPO_ADD_WINDOW, fullscreenWidget->winId(), true );
    }
    else
    {
	if (fullscreenWidget)
	{
	    delete fullscreenWidget;
	    fullscreenWidget = 0;
	}

	sendEvent( winId(), VPO_ENABLE_WINDOW );
    }
}

void KVideoWidget::halfSizeActivated()
{
    if (isHalfSize())
    {
	emit adaptSize( (videoWidth / 2), (videoHeight / 2) );
	setWindowed();
    }
}

void KVideoWidget::normalSizeActivated()
{
    if (isNormalSize())
    {
	emit adaptSize( videoWidth, videoHeight );
	setWindowed();
    }
}

void KVideoWidget::doubleSizeActivated()
{
    if (isDoubleSize())
    {
	emit adaptSize( (2 * videoWidth), (2 * videoHeight) );
	setWindowed();
    }
}

#include "kvideowidget.moc"
