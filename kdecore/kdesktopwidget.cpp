/* This file is part of the KDE libraries
    Copyright (C) 2001 Balaji Ramani (balaji@yablibli.com)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <config.h>

#include "kdesktopwidget.h"

#include <X11/Xlib.h>
#ifdef HAVE_XINERAMA
#ifndef Bool
/* happens only in --enable-final mode */
#define Bool X11Bool
#endif
extern "C" {
  #include <X11/extensions/Xinerama.h>
}
#endif

class KDesktopWidgetPrivate
{
public:
    KDesktopWidgetPrivate();
    ~KDesktopWidgetPrivate();

    bool use_xinerama;
    int defaultScreen;
    int screenCount;

    QWidget **screens;
    QRect *rects;
};

KDesktopWidgetPrivate::KDesktopWidgetPrivate()
    : use_xinerama(FALSE)
{

#ifdef HAVE_XINERAMA
    XineramaScreenInfo *xinerama_screeninfo = 0;
    int unused;
    use_xinerama = (XineramaQueryExtension(qt_xdisplay(),
					   &unused, &unused) &&
		    XineramaIsActive(qt_xdisplay()));

    if (use_xinerama) {
	xinerama_screeninfo =
	    XineramaQueryScreens(qt_xdisplay(), &screenCount);
	defaultScreen = 0;
    } else
#endif
    {
	defaultScreen = DefaultScreen(QPaintDevice::x11AppDisplay());
	screenCount = ScreenCount(QPaintDevice::x11AppDisplay());
    }

    // get the geometry of each screen
    rects = new QRect[screenCount];
    int i, x, y, w, h;
    for (i = 0; i < screenCount; i++) {

#ifdef HAVE_XINERAMA
	if (use_xinerama) {
	    x = xinerama_screeninfo[i].x_org;
	    y = xinerama_screeninfo[i].y_org;
	    w = xinerama_screeninfo[i].width;
	    h = xinerama_screeninfo[i].height;
	} else
#endif

	    {
		x = 0;
		y = 0;
		w = WidthOfScreen(ScreenOfDisplay(qt_xdisplay(), i));
		h = HeightOfScreen(ScreenOfDisplay(qt_xdisplay(), i));
	    }

	rects[i].setRect(x, y, w, h);
    }

    screens = 0;

#ifdef HAVE_XINERAMA
    if (xinerama_screeninfo)
	XFree(xinerama_screeninfo);
#endif // QT_NO_XINERAMA

};

KDesktopWidgetPrivate::~KDesktopWidgetPrivate()
{
    delete [] rects;

    if (! screens)
	return;

    for ( int i = 0; i < screenCount; ++i ) {
	if (i == defaultScreen) continue;

	delete screens[ i ];
	screens[i] = 0;
    }

    delete [] screens;
}

KDesktopWidget::KDesktopWidget()
: QWidget( 0, "desktop", WType_Desktop )
{
    d = new KDesktopWidgetPrivate;
}

KDesktopWidget::~KDesktopWidget()
{
    delete d;
}

bool KDesktopWidget::isVirtualDesktop() const
{
    return d->use_xinerama;
}

int KDesktopWidget::primaryScreen() const
{
    return d->defaultScreen;
}

int KDesktopWidget::numScreens() const
{
    return d->screenCount;
}

QWidget *KDesktopWidget::screen( int screen )
{
    if (d->use_xinerama)
	return this;

    // This is for not using xinerama (multi head
    // This code is supposed to create a QWidget that
    // represents a screen.  Since QT < 3 does not
    // have this support, we just create on the
    // same screen.
    if ( screen < 0 || screen >= d->screenCount )
	screen = d->defaultScreen;

    if ( ! d->screens ) {
	memset( (d->screens = new QWidget*[d->screenCount] ), 0,
		d->screenCount * sizeof( QWidget*) );
	d->screens[d->defaultScreen] = this;
    }

    if ( ! d->screens[screen] ||               // not created yet
	 ! d->screens[screen]->isDesktop() ) { // reparented away
	d->screens[screen] = new QWidget( 0, "desktop", WType_Desktop );
    }

    return d->screens[screen];
}

const QRect& KDesktopWidget::screenGeometry( int screen ) const
{
    if ( screen < 0 || screen >= d->screenCount )
	screen = d->defaultScreen;

    return d->rects[ screen ];
}

int KDesktopWidget::screenNumber( QWidget *widget ) const
{
    if ( !widget )
	return d->defaultScreen;
    QRect frame = widget->frameGeometry();
    if ( !widget->isTopLevel() )
	frame.moveTopLeft( widget->mapToGlobal( frame.topLeft() ) );

    int maxSize = -1;
    int maxScreen = -1;

    for ( int i = 0; i < d->screenCount; ++i ) {
	QRect sect = d->rects[i].intersect( frame );
	int size = sect.width() * sect.height();
	if ( size > maxSize && sect.width() > 0 && sect.height() > 0 ) {
	    maxSize = size;
	    maxScreen = i;
	}
    }
    return maxScreen;
}

int KDesktopWidget::screenNumber( const QPoint &point ) const
{
    for ( int i = 0; i < d->screenCount; ++i ) {
	if ( d->rects[i].contains( point ) )
	    return i;
    }
    return -1;
}
