/* This file is part of the KDE project
 *
 * Copyright (C) 2000 Richard Moore <rich@kde.org>
 *               2000 Wynn Wilkes <wynnw@caldera.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "kjavaappletwidget.h"
#include "kjavaappletserver.h"

#include <kwin.h>
#include <kdebug.h>
#include <klocale.h>

#include <qlabel.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

const int XFocusOut = FocusOut;
const int XFocusIn = FocusIn;
#undef FocusOut
#undef FocusIn

typedef int (*QX11EventFilter) (XEvent*);
extern QX11EventFilter qt_set_x11_event_filter (QX11EventFilter filter);
static QX11EventFilter oldFilter = 0;

static int qxembed_x11_event_filter( XEvent* e)
{
    switch ( e->type ) {
    case LeaveNotify: {
        QWidget* w = QWidget::find( e->xkey.window );
        if (w && w->metaObject()->inherits("KJavaAppletWidget"))
            XUngrabButton( qt_xdisplay(), AnyButton, AnyModifier, e->xany.window );
        break;
    }
    case EnterNotify: {
        QWidget* w = QWidget::find( e->xkey.window );
        if (w && w->metaObject()->inherits("KJavaAppletWidget") && !w->hasFocus())
            XGrabButton(qt_xdisplay(), AnyButton, AnyModifier, e->xany.window,
                    FALSE, ButtonPressMask, GrabModeSync, GrabModeAsync,
                    None, None );
        break;
    }
    }
    if ( oldFilter )
        return oldFilter( e );
    return FALSE;
}


// For future expansion
class KJavaAppletWidgetPrivate
{
friend class KJavaAppletWidget;
private:
    QLabel* tmplabel;
};

int KJavaAppletWidget::appletCount = 0;

KJavaAppletWidget::KJavaAppletWidget( KJavaAppletContext* context,
                                      QWidget* parent, const char* name )
   : QXEmbed ( parent, name)
{
    static bool initialized = false;
    if (!initialized) {
        oldFilter = qt_set_x11_event_filter( qxembed_x11_event_filter );
        initialized = true;
    }
    m_applet = new KJavaApplet( this, context );
    d        = new KJavaAppletWidgetPrivate;
    m_kwm    = new KWinModule( this );

    d->tmplabel = new QLabel( this );
    d->tmplabel->setText( KJavaAppletServer::getAppletLabel() );
    d->tmplabel->setAlignment( Qt::AlignCenter | Qt::WordBreak );
    d->tmplabel->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    d->tmplabel->show();

    m_swallowTitle.sprintf( "KJAS Applet - Ticket number %u", appletCount++ );
    m_applet->setWindowName( m_swallowTitle );
}

KJavaAppletWidget::~KJavaAppletWidget()
{
    delete m_applet;
    delete d;
}

void KJavaAppletWidget::showApplet()
{
    connect( m_kwm, SIGNAL( windowAdded( WId ) ),
	         this,  SLOT( setWindow( WId ) ) );

    m_kwm->doNotManage( m_swallowTitle );

    //Now we send applet info to the applet server
    if ( !m_applet->isCreated() )
        m_applet->create();
}

void KJavaAppletWidget::setWindow( WId w )
{
    //make sure that this window has the right name, if so, embed it...
    KWin::Info w_info = KWin::info( w );
    if ( m_swallowTitle == w_info.name ||
         m_swallowTitle == w_info.visibleName )
    {
        kdDebug(6100) << "swallowing our window: " << m_swallowTitle
                      << ", window id = " << w << endl;
        delete d->tmplabel;
        d->tmplabel = 0;

        // disconnect from KWM events
        disconnect( m_kwm, SIGNAL( windowAdded( WId ) ),
                    this,  SLOT( setWindow( WId ) ) );


        embed( w );
        setFocus();
    }
}

QSize KJavaAppletWidget::sizeHint() const
{
    kdDebug(6100) << "KJavaAppletWidget::sizeHint()" << endl;
    QSize rval = QXEmbed::sizeHint();

    if( rval.width() == 0 || rval.height() == 0 )
    {
        if( width() != 0 && height() != 0 )
        {
            rval = QSize( width(), height() );
        }
    }

    kdDebug(6100) << "returning: (" << rval.width() << ", " << rval.height() << ")" << endl;

    return rval;
}

void KJavaAppletWidget::resize( int w, int h )
{
    kdDebug(6100) << "KJavaAppletWidget, id = " << m_applet->appletId() << ", ::resize to: " << w << ", " << h << endl;

    if( d->tmplabel )
    {
        d->tmplabel->resize( w, h );
        m_applet->setSize( QSize( w, h ) );
    }

    QXEmbed::resize( w, h );
}

void KJavaAppletWidget::focusInEvent( QFocusEvent * e ){
    WId window = embeddedWinId();
    if (!window)
        return;
    XUngrabButton( qt_xdisplay(), AnyButton, AnyModifier, window );
    XFocusInEvent inev = { XFocusIn, 0, TRUE, qt_xdisplay(), window, 
                           NotifyNormal, NotifyPointer };
    XSendEvent(qt_xdisplay(), window, TRUE, FocusChangeMask, (XEvent*) &inev);
    QXEmbed::focusInEvent(e);
}

/*!\reimp
 */
void KJavaAppletWidget::focusOutEvent( QFocusEvent * e ) {
    if (!embeddedWinId())
        return;
    XFocusOutEvent outev = { XFocusOut, 0, TRUE, qt_xdisplay(), embeddedWinId(),
                             NotifyNormal, NotifyPointer };
    XSendEvent(qt_xdisplay(), embeddedWinId(), TRUE, FocusChangeMask, (XEvent*) &outev);
    QXEmbed::focusOutEvent(e);
}

bool KJavaAppletWidget::x11Event( XEvent* e )
{
    switch ( e->type ) {

    case ButtonPress:
        QFocusEvent::setReason( QFocusEvent::Mouse );
        setFocus();
        QFocusEvent::resetReason();
        XAllowEvents(qt_xdisplay(), ReplayPointer, CurrentTime);
        return TRUE;
    case ButtonRelease:
        XAllowEvents(qt_xdisplay(), SyncPointer, CurrentTime);
        break;
    }
    return QXEmbed::x11Event(e);
}

#include "kjavaappletwidget.moc"
