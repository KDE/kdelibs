/*
   This file is part of KDE/aRts (Noatun) - xine integration
   Copyright (C) 2002 Ewald Snel <ewald@rambo.its.tudelft.nl>
   Copyright (C) 2002 Neil Stevens <neil@qualityassistant.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License version 2 as published by the Free Software Foundation.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <time.h>
#ifdef HAVE_USLEEP
#include <unistd.h>
#endif
#include <qaccel.h>
#include <qcursor.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <kaction.h>
#include <klocale.h>
#include "kvideowidget.h"


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
    setEraseColor( black );
    setCursor(QCursor(Qt::BlankCursor));
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
    if (event->type == ClientMessage &&
	event->xclient.message_type ==
		XInternAtom( qt_xdisplay(), "VPO_RESIZE_NOTIFY", False ))
    {
	videoWidget->resizeNotify( event->xclient.data.l[0], event->xclient.data.l[1] );
    }
    return false;
}

KVideoWidget::KVideoWidget( KXMLGUIClient *clientParent, QWidget *parent, const char *name, WFlags f )
    : QWidget( parent, name, f )
    , KXMLGUIClient( clientParent )
{
	init();
	// ???
	QString toolbarName = i18n("Video Toolbar");
	setXML(QString("<!DOCTYPE kpartgui>\n<kpartgui name=\"kvideowidget\" version=\"1\"><MenuBar><Menu name=\"edit\"><Separator/><Action name=\"double_size\"/><Action name=\"normal_size\"/><Action name=\"half_size\"/><Separator/><Action name=\"fullscreen_mode\"/></Menu></MenuBar><Toolbar name=\"%1\"><Action name=\"fullscreen_mode\"/></Toolbar></kpartgui>").arg(toolbarName), true);
}

KVideoWidget::KVideoWidget( QWidget *parent, const char *name, WFlags f )
    : QWidget( parent, name, f )
{
	init();
}

void KVideoWidget::init(void)
{
    setMinimumSize(0, 0);
    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    setFocusPolicy( ClickFocus );

    fullscreenWidget = 0;
    poVideo	     = Arts::VideoPlayObject::null();
    videoWidth	     = 0;
    videoHeight	     = 0;

    // Setup actions
    new KToggleAction( i18n("Fullscreen &Mode"), "window_fullscreen",
		       CTRL+SHIFT+Key_F, this, SLOT(fullscreenActivated()),
		       actionCollection(), "fullscreen_mode" );
    new KRadioAction( i18n("&Half Size"), ALT+Key_0,
		      this, SLOT(halfSizeActivated()),
		      actionCollection(), "half_size" );
    new KRadioAction( i18n("&Normal Size"), ALT+Key_1,
		      this, SLOT(normalSizeActivated()),
		      actionCollection(), "normal_size" );
    new KRadioAction( i18n("&Double Size"), ALT+Key_2,
		      this, SLOT(doubleSizeActivated()),
		      actionCollection(), "double_size" );

    ((KToggleAction *)action( "half_size" ))->setExclusiveGroup( "KVideoWidget::zoom" );
    ((KToggleAction *)action( "normal_size" ))->setExclusiveGroup( "KVideoWidget::zoom" );
    ((KToggleAction *)action( "double_size" ))->setExclusiveGroup( "KVideoWidget::zoom" );
}

KVideoWidget::~KVideoWidget()
{
    if (isEmbedded())
    {
	poVideo.x11WindowId( -1 );
	poVideo = Arts::VideoPlayObject::null();
    }

    if (fullscreenWidget)
    {
	delete fullscreenWidget;
    }
}

void KVideoWidget::embed( Arts::VideoPlayObject vpo )
{
    if (vpo.isNull())
    {
	if (isEmbedded())
	{
	    poVideo.x11WindowId( -1 );
	    poVideo = Arts::VideoPlayObject::null();
	}

	setBackgroundMode( PaletteBackground );
	repaint();

	// Resize GUI
	videoWidth  = 0;
	videoHeight = 0;

	if (isHalfSize() || isNormalSize() || isDoubleSize())
	    emit adaptSize( 0, 0 );
    }
    else
    {
	if (isEmbedded())
	{
	    poVideo.x11WindowId( -1 );
	}

	poVideo = vpo;

	// Don't reset fullscreen mode for video playlists
	if (fullscreenWidget)
	{
	    poVideo.x11WindowId( fullscreenWidget->winId() );
	    fullscreenWidget->setBackgroundMode( NoBackground );

	    setEraseColor( black );
	}
	else
	{
	    poVideo.x11WindowId( winId() );
	    setBackgroundMode( NoBackground );
	}
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
    return !poVideo.isNull();
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

void KVideoWidget::resizeNotify( int width, int height )
{
    videoWidth = width;
    videoHeight = height;

    if (isHalfSize())
	emit adaptSize( (videoWidth / 2), (videoHeight / 2) );
    else if (isNormalSize())
	emit adaptSize( videoWidth, videoHeight );
    else if (isDoubleSize())
	emit adaptSize( (2 * videoWidth), (2 * videoHeight) );
}

bool KVideoWidget::x11Event( XEvent *event )
{
    if (event->type == ClientMessage &&
	event->xclient.message_type ==
		XInternAtom( qt_xdisplay(), "VPO_RESIZE_NOTIFY", False ))
    {
	resizeNotify( event->xclient.data.l[0], event->xclient.data.l[1] );
    }
    return false;
}

void KVideoWidget::fullscreenActivated()
{
    if (isFullscreen() == (fullscreenWidget != 0))
	return;

    if (isFullscreen())
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

	if (isEmbedded())
	{
	    poVideo.x11WindowId( fullscreenWidget->winId() );
	    fullscreenWidget->setBackgroundMode( NoBackground );
	}
    }
    else
    {
	if (isEmbedded())
	{
	    poVideo.x11WindowId( winId() );
	    setBackgroundMode( NoBackground );
	}

	delete fullscreenWidget;
	fullscreenWidget = 0;
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
