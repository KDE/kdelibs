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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kjavaappletwidget.h"
#include "kjavaappletserver.h"

#include <kwin.h>
#include <kdebug.h>
#include <klocale.h>

#include <qlabel.h>

#ifndef Q_WS_X11
#define QXEmbed QWidget
#endif

// For future expansion
class KJavaAppletWidgetPrivate
{
friend class KJavaAppletWidget;
private:
    QLabel* tmplabel;
};

int KJavaAppletWidget::appletCount = 0;

KJavaAppletWidget::KJavaAppletWidget( QWidget* parent )
   : QX11EmbedContainer ( parent ),
   d(new KJavaAppletWidgetPrivate)
{
    //setProtocol(QXEmbed::XPLAIN);

    m_applet = new KJavaApplet( this );
#ifdef Q_WS_X11
    m_kwm    = new KWinModule( this );
#endif

    d->tmplabel = new QLabel( this );
    d->tmplabel->setText( KJavaAppletServer::getAppletLabel() );
    d->tmplabel->setAlignment( Qt::AlignCenter | Qt::TextWordWrap );
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
#ifdef Q_WS_X11
    connect( m_kwm, SIGNAL( windowAdded( WId ) ),
	         this,  SLOT( setWindow( WId ) ) );

    m_kwm->doNotManage( m_swallowTitle );

    //Now we send applet info to the applet server
    if ( !m_applet->isCreated() )
        m_applet->create();
#endif
}

void KJavaAppletWidget::setWindow( WId w )
{
#ifdef Q_WS_X11
    //make sure that this window has the right name, if so, embed it...
    KWin::WindowInfo w_info = KWin::windowInfo( w );
    if ( m_swallowTitle == w_info.name() ||
         m_swallowTitle == w_info.visibleName() )
    {
        kdDebug(6100) << "swallowing our window: " << m_swallowTitle
                      << ", window id = " << w << endl;
        delete d->tmplabel;
        d->tmplabel = 0;

        // disconnect from KWM events
        disconnect( m_kwm, SIGNAL( windowAdded( WId ) ),
                    this,  SLOT( setWindow( WId ) ) );

        embedClient( w );
        setFocus();
    }
#else
    //TODO
#endif
}

QSize KJavaAppletWidget::sizeHint() const
{
    kdDebug(6100) << "KJavaAppletWidget::sizeHint()" << endl;
    QSize rval = QX11EmbedContainer::sizeHint();

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
    if( d->tmplabel )
    {
        d->tmplabel->resize( w, h );
        m_applet->setSize( QSize( w, h ) );
    }

    QX11EmbedContainer::resize( w, h );
}

void KJavaAppletWidget::showEvent (QShowEvent * e) {
    QX11EmbedContainer::showEvent(e);
    if (!applet()->isCreated() && !applet()->appletClass().isEmpty()) {
        // delayed showApplet
        if (applet()->size().width() <= 0)
            applet()->setSize (sizeHint());
        showApplet();
    }
}

#include "kjavaappletwidget.moc"
