/*
 *   copyright            : (C) 2001-2002 by Richard Moore
 *   email                : rich@kde.org
 */

#include <qpixmap.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <ksystemtray.h>

#include "kwindowinfo.h"
#include "kwindowinfo.moc"

static const int UNSPECIFIED_TIMEOUT = -1;
static const int DEFAULT_MESSAGE_TIMEOUT = 3000;

KWindowInfo::KWindowInfo( QWidget *parent, const char *name )
    : QObject( parent, name ), win( parent ), autoDel( false )
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
    save();
    display( text, pix );

    if ( timeout < 0 )
	timeout = DEFAULT_MESSAGE_TIMEOUT;
    if ( timeout == 0 )
	return;
    QTimer::singleShot( timeout, this, SLOT( restore() ) );
}

void KWindowInfo::display( const QString &text, const QPixmap &pix )
{
    QPixmap icon;
    if ( pix.isNull() )
	icon.load( "bell.png" );
    else
	icon = pix;

    if ( win->inherits( "KSystemTray" ) ) {
	KSystemTray *tray = static_cast<KSystemTray *>( win );
	tray->setPixmap( icon );
	QToolTip::add( tray, text );
	return;
    }

    win->setCaption( text );
    win->setIcon( icon );
}

void KWindowInfo::save()
{
    if ( win->inherits( "KSystemTray" ) ) {
	KSystemTray *tray = static_cast<KSystemTray *>( win );
	oldIcon = *(tray->pixmap());
	oldText = QToolTip::textFor( tray );
	return;
    }

    oldText = win->caption();

    const QPixmap *px = win->icon();
    if ( px )
	oldIcon = *px;
    else
	oldIcon.resize( 0, 0 );
}

void KWindowInfo::restore()
{
    if ( win->inherits( "KSystemTray" ) ) {
	KSystemTray *tray = static_cast<KSystemTray *>( win );
	tray->setPixmap( oldIcon );
	QToolTip::add( tray, oldText );
	return;
    }

    win->setCaption( oldText );
    win->setIcon( oldIcon );

    if ( autoDel )
	delete this;
}





