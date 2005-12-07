/*
 *   copyright            : (C) 2001-2002 by Richard Moore
 *   License              : This file is released under the terms of the LGPL, version 2.
 *   email                : rich@kde.org
 */

#include <qobject.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <ksystemtray.h>
#include <kwin.h>

#include "kwindowinfo.h"
#include "kwindowinfo.moc"

static const int UNSPECIFIED_TIMEOUT = -1;
static const int DEFAULT_MESSAGE_TIMEOUT = 3000;

KWindowInfo::KWindowInfo( QWidget *parent )
    : QObject( parent ), win( parent ), autoDel( false )
{
}

KWindowInfo::~KWindowInfo()
{
}

void KWindowInfo::showMessage( QWidget *window, const QString &text, int timeout )
{
    KWindowInfo *info = new KWindowInfo( window );
    info->autoDel = true;
    info->message( text, timeout );
    if ( timeout == 0 )
	delete info;
}

void KWindowInfo::showMessage( QWidget *window, const QString &text, const QPixmap &pix, int timeout )
{
    KWindowInfo *info = new KWindowInfo( window );
    info->autoDel = true;
    info->message( text, pix, timeout );
}

void KWindowInfo::message( const QString &text )
{
    message( text, QPixmap(), UNSPECIFIED_TIMEOUT );
}

void KWindowInfo::message( const QString &text, const QPixmap &pix )
{
    message( text, pix, UNSPECIFIED_TIMEOUT );
}

void KWindowInfo::message( const QString &text, int timeout )
{
    message( text, QPixmap(), timeout );
}

void KWindowInfo::message( const QString &text, const QPixmap &pix, int timeout )
{
    if ( timeout != 0 )
	save();

    display( text, pix );

    if ( timeout < 0 )
	timeout = DEFAULT_MESSAGE_TIMEOUT;
    if ( timeout != 0 )
	QTimer::singleShot( timeout, this, SLOT( restore() ) );
}

void KWindowInfo::permanent( const QString &text )
{
#ifdef Q_WS_X11
    oldMiniIcon = KWin::icon( win->winId(), 16, 16, true );
    oldIcon = KWin::icon( win->winId(), 34, 34, false );
    if ( oldIcon.isNull() )
	oldIcon = KWin::icon( win->winId(), 32, 32, true );
#endif

    permanent( text, oldIcon );
}

void KWindowInfo::permanent( const QString &text, const QPixmap &pix )
{
    if ( !oldText.isNull() ) {
	QList<QTimer *> l = findChildren<QTimer *>();
		foreach ( QObject*o , l ) {
			delete o;
		}
	}
    oldText = QString::null;
    display( text, pix );
}

void KWindowInfo::display( const QString &text, const QPixmap &pix )
{
    QPixmap icon;
    if ( pix.isNull() )
	icon.load( "bell.png" );
    else
	icon = pix;

    if ( KSystemTray *tray = qobject_cast<KSystemTray*>(win) ) {
	tray->setPixmap( icon );
	tray->setToolTip( text );
	return;
    }

    win->setCaption( text );
    win->setIcon( icon );
#ifdef Q_WS_X11
    KWin::setIcons( win->winId(), icon, icon );
#endif
}

void KWindowInfo::save()
{
    if ( !oldText.isNull() )
	return;

    if ( KSystemTray *tray = qobject_cast<KSystemTray*>(win) ) {
	oldIcon = *(tray->pixmap());
	oldText = tray->toolTip();
	return;
    }

    oldText = win->caption();
#ifdef Q_WS_X11
    oldMiniIcon = KWin::icon( win->winId(), 16, 16, true );
    oldIcon = KWin::icon( win->winId(), 34, 34, false );
    if ( oldIcon.isNull() )
	oldIcon = KWin::icon( win->winId(), 32, 32, true );
#endif

    if ( oldIcon.isNull() ) {
	const QPixmap *px = win->icon();
	if ( px )
	    oldIcon = *px;
	else
	    oldIcon.resize( 0, 0 );
    }
}

void KWindowInfo::restore()
{
    if ( KSystemTray *tray = qobject_cast<KSystemTray*>(win) ) {
	tray->setPixmap( oldIcon );
	tray->setToolTip( oldText );
	oldText = QString::null;
	return;
    }

    win->setIcon( oldIcon );
#ifdef Q_WS_X11
    KWin::setIcons( win->winId(), oldIcon, oldMiniIcon );
#endif
    win->setCaption( oldText );
    oldText = QString::null;

    if ( autoDel )
	delete this;
}





