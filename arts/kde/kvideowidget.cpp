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
#include "kvideowidget.h"

#define VPO_ADD_WINDOW		1
#define VPO_DESTROY_NOTIFY	2
#define VPO_DISABLE_NOTIFY	3
#define VPO_ENABLE_NOTIFY	4
#define VPO_ENABLE_WINDOW	5
#define VPO_RELEASE_WINDOW	6
#define VPO_RESIZE_NOTIFY	7

enum VideoZoom { CustomSize, HalfSize, NormalSize, DoubleSize };


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
    KFullscreenVideoWidget();

protected:
    virtual void hideEvent( QHideEvent * );
    virtual void windowActivationChange( bool );
    virtual bool x11Event( XEvent *event );
};

KFullscreenVideoWidget::KFullscreenVideoWidget()
    : KVideoWidget( 0, 0, WType_TopLevel | WStyle_Customize | WStyle_NoBorder )
{
    setEraseColor( black );
}

void KFullscreenVideoWidget::hideEvent( QHideEvent * )
{
    releaseWinId( winId() );

    setEraseColor( black );
}

void KFullscreenVideoWidget::windowActivationChange( bool )
{
    if (!isActiveWindow())
    {
	hide();
    }
}

bool KFullscreenVideoWidget::x11Event( XEvent *event )
{
    Atom atom = XInternAtom( qt_xdisplay(), "VPO_X11_COMM", False );

    if (event->type == ClientMessage &&
	event->xclient.message_type == atom &&
	event->xclient.data.l[0] == VPO_DISABLE_NOTIFY)
    {
	hide();
    }
    return KVideoWidget::x11Event( event );
}


KVideoWidget::KVideoWidget( QWidget *parent, const char *name, WFlags f )
    : QWidget( parent, name, f )
{
    setEraseColor( lightGray );
    setFocusPolicy( ClickFocus );
    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );

    fullscreenWidget = 0;
    embedded	     = false;
    enabled	     = false;
    zoom	     = CustomSize;
    videoWidth	     = 0;
    videoHeight	     = 0;

    // Add fullscreen widget
    if (parent && !(f & WType_TopLevel))
    {
	fullscreenWidget = new KFullscreenVideoWidget();

	// Interconnect right mouse button signals
	connect( fullscreenWidget, SIGNAL(rightButtonPressed(const QPoint &)),
		 this, SIGNAL(rightButtonPressed(const QPoint &)) );

	// Leave fullscreen mode with <Escape> key
	QAccel *a = new QAccel( fullscreenWidget );
	a->connectItem( a->insertItem( Key_Escape ),
			this, SLOT(slotWindowed()) );
    }
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
	    releaseWinId( fullscreenWidget->winId() );

	    setEraseColor( lightGray );
	    embedded = false;
	}

	// Resize GUI
	videoWidth  = 0;
	videoHeight = 0;

	if (zoom != CustomSize)
	    emit adaptSize( 0, 0 );
    }
    else
    {
	embedded = true;

	setBackgroundMode( NoBackground );

	// Don't reset fullscreen mode for video playlists
	if (fullscreenWidget->isVisible())
	{
	    vpo.x11WindowId( fullscreenWidget->winId() );
	}

	vpo.x11WindowId( winId() );

	// Re-enable video widget
	if (fullscreenWidget->isVisible())
	{
	    sendEvent( fullscreenWidget->winId(), VPO_ENABLE_WINDOW );
	}
	else if (enabled)
	{
	    sendEvent( winId(), VPO_ENABLE_WINDOW );
	}
    }
}

bool KVideoWidget::isEmbedded()
{
    return embedded;
}

bool KVideoWidget::isFullscreen()
{
    return fullscreenWidget->isVisible();
}

bool KVideoWidget::isHalfSize()
{
    return (zoom == HalfSize);
}

bool KVideoWidget::isNormalSize()
{
    return (zoom == NormalSize);
}

bool KVideoWidget::isDoubleSize()
{
    return (zoom == DoubleSize);
}

QSize KVideoWidget::sizeHint() const
{
    return QSize( videoWidth, videoHeight );
}

int KVideoWidget::heightForWidth( int w ) const
{
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

    int oldZoom = zoom;

    if (width() > minimumWidth() || height() > minimumHeight())
    {
	if (width() == QMAX( (videoWidth >> 1), minimumWidth() ) &&
	         height() == QMAX( (videoHeight >> 1), minimumHeight() ))
	    zoom = HalfSize;
	else if (width() == QMAX( videoWidth, minimumWidth() ) &&
		 height() == QMAX( videoHeight, minimumHeight() ))
	    zoom = NormalSize;
	else if (width() == QMAX( (videoWidth << 1), minimumWidth() ) &&
		 height() == QMAX( (videoHeight << 1), minimumHeight() ))
	    zoom = DoubleSize;
	else
	    zoom = CustomSize;
    }

    if (oldZoom != zoom)
    {
	emit zoomChanged();
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

	    if (zoom == HalfSize)
		emit adaptSize( (videoWidth >> 1), (videoHeight >> 1) );
	    else if (zoom == NormalSize)
		emit adaptSize( videoWidth, videoHeight );
	    else if (zoom == DoubleSize)
		emit adaptSize( (videoWidth << 1), (videoHeight << 1) );
	    break;
	case VPO_DESTROY_NOTIFY:
	    embedded = false;
	    setEraseColor( black );
	    break;
	case VPO_ENABLE_NOTIFY:
	    enabled = true;
	    setBackgroundMode( NoBackground );
	    break;
	case VPO_DISABLE_NOTIFY:
	    enabled = false;
	    setEraseColor( black );
	    break;
	}
    }
    return false;
}

void KVideoWidget::slotFullscreen()
{
    if (!isFullscreen())
    {
#if 1	// Ugly workaround for Qt-3.x
	fullscreenWidget->showNormal();
#endif
	fullscreenWidget->showFullScreen();
	fullscreenWidget->setFocus();

	// Add fullscreen window
	sendEvent( winId(), VPO_ADD_WINDOW, fullscreenWidget->winId(), true );
    }
}

void KVideoWidget::slotWindowed()
{
    if (isFullscreen())
    {
	fullscreenWidget->hide();
    }
    sendEvent( winId(), VPO_ENABLE_WINDOW );
}

void KVideoWidget::slotHalfSize()
{
    if (zoom != HalfSize)
    {
	zoom = HalfSize;

	if (isEmbedded())
	{
	    emit adaptSize( (videoWidth >> 1), (videoHeight >> 1) );
	}

	emit zoomChanged();
    }
}

void KVideoWidget::slotNormalSize()
{
    if (zoom != NormalSize)
    {
	zoom = NormalSize;

	if (isEmbedded())
	{
	    emit adaptSize( videoWidth, videoHeight );
	}

	emit zoomChanged();
    }
}

void KVideoWidget::slotDoubleSize()
{
    if (zoom != DoubleSize)
    {
	zoom = DoubleSize;

	if (isEmbedded())
	{
	    emit adaptSize( (videoWidth << 1), (videoHeight << 1) );
	}

	emit zoomChanged();
    }
}

#include "kvideowidget.moc"
