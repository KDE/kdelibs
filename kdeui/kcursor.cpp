/* This file is part of the KDE libraries
   Copyright (C) 1998 Kurt Granroth (granroth@kde.org)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include <kcursor.h>

#include <qbitmap.h>
#include <qcursor.h>
#include <qevent.h>
#include <qtimer.h>
#include <qwidget.h>

#include <kglobal.h>
#include <kconfig.h>


KCursor::KCursor()
{
}

QCursor KCursor::handCursor()
{
	static QCursor *hand_cursor = 0;

	if (hand_cursor == 0)
	{
		KConfig *config = KGlobal::config();
		KConfigGroupSaver saver( config, "General" );

		if ( config->readEntry("handCursorStyle", "Windows") == "Windows" )
		{
			unsigned char HAND_BITS[] = {
				0x80, 0x01, 0x00, 0x40, 0x02, 0x00, 0x40, 0x02, 0x00, 0x40, 0x02,
				0x00, 0x40, 0x02, 0x00, 0x40, 0x02, 0x00, 0x40, 0x1e, 0x00, 0x40,
				0xf2, 0x00, 0x40, 0x92, 0x01, 0x70, 0x92, 0x02, 0x50, 0x92, 0x04,
				0x48, 0x80, 0x04, 0x48, 0x00, 0x04, 0x48, 0x00, 0x04, 0x08, 0x00,
				0x04, 0x08, 0x00, 0x04, 0x10, 0x00, 0x04, 0x10, 0x00, 0x04, 0x20,
				0x00, 0x02, 0x40, 0x00, 0x02, 0x40, 0x00, 0x01, 0xc0, 0xff, 0x01};
			unsigned char HAND_MASK_BITS[] = {
				0x80, 0x01, 0x00, 0xc0, 0x03, 0x00, 0xc0, 0x03, 0x00, 0xc0, 0x03,
				0x00, 0xc0, 0x03, 0x00, 0xc0, 0x03, 0x00, 0xc0, 0x1f, 0x00, 0xc0,
				0xff, 0x00, 0xc0, 0xff, 0x01, 0xf0, 0xff, 0x03, 0xf0, 0xff, 0x07,
				0xf8, 0xff, 0x07, 0xf8, 0xff, 0x07, 0xf8, 0xff, 0x07, 0xf8, 0xff,
				0x07, 0xf8, 0xff, 0x07, 0xf0, 0xff, 0x07, 0xf0, 0xff, 0x07, 0xe0,
				0xff, 0x03, 0xc0, 0xff, 0x03, 0xc0, 0xff, 0x01, 0xc0, 0xff, 0x01};
			QBitmap hand_bitmap(22, 22, HAND_BITS, true);
			QBitmap hand_mask(22, 22, HAND_MASK_BITS, true);
			hand_cursor = new QCursor(hand_bitmap, hand_mask, 7, 0);
		}
		else
			hand_cursor = new QCursor(PointingHandCursor);
	}

	CHECK_PTR(hand_cursor);
	return *hand_cursor;
}

/**
 * All of the follow functions will return the Qt default for now regardless
 * of the style.  This will change at some later date
 */
QCursor KCursor::arrowCursor()
{
    return Qt::arrowCursor;
}


QCursor KCursor::upArrowCursor()
{
	return Qt::upArrowCursor;
}


QCursor KCursor::crossCursor()
{
	return Qt::crossCursor;
}


QCursor KCursor::waitCursor()
{
	return Qt::waitCursor;
}


QCursor KCursor::ibeamCursor()
{
	return Qt::ibeamCursor;
}


QCursor KCursor::sizeVerCursor()
{
	return Qt::sizeVerCursor;
}


QCursor KCursor::sizeHorCursor()
{
	return Qt::sizeHorCursor;
}


QCursor KCursor::sizeBDiagCursor()
{
	return Qt::sizeBDiagCursor;
}


QCursor KCursor::sizeFDiagCursor()
{
	return Qt::sizeFDiagCursor;
}


QCursor KCursor::sizeAllCursor()
{
	return Qt::sizeAllCursor;
}


QCursor KCursor::blankCursor()
{
	return Qt::blankCursor;
}

// auto-hide cursor stuff

void KCursor::setAutoHideCursor( QWidget *w, bool enable )
{
    if ( !w )
	return;

    KConfig *kc = KGlobal::config();
    KConfigGroupSaver ks( kc, QString::fromLatin1("KDE") );
    if ( !kc->readBoolEntry( QString::fromLatin1("Autohiding cursor enabled"),
                            true ) )
        return;

    // we need an object for a slot and for the eventfilter
    static KCursorPrivate hideHelper;

    // nice Qt "feature": installing an eventfilter twice, gives us...
    // ... two eventfilters => two events arriving here
    // so we better remove an existing eventFilter
    // Arnt said it will be fixed, in 2.2.
    w->removeEventFilter( &hideHelper );

    if ( enable ) {
	hideHelper.start();
	w->setMouseTracking( true );

        // nice Qt "feature": installing an eventfilter twice, gives us...
        // ... two eventfilters => two events arriving here
        // so we better remove an existing eventFilter before installing one
        // Arnt said it will be fixed, in 2.2.
        w->removeEventFilter( &hideHelper );
	w->installEventFilter( &hideHelper );
    }

    else {
	w->setMouseTracking( false );
        w->removeEventFilter( &hideHelper );
	hideHelper.stop();
    }
}

// **************************************************************************
// **************************************************************************

QTimer * KCursorPrivate::s_autoHideTimer = 0L;
int KCursorPrivate::s_hideCursorDelay = 5000; // 5s default value
int KCursorPrivate::s_count = 0;
bool KCursorPrivate::s_isCursorHidden = false;
bool KCursorPrivate::s_isOwnCursor    = false;
// grr, Bug in gcc 2.95-2 (segfault when calling QCursor::operator=(QCursor))
QCursor * KCursorPrivate::s_oldCursor = 0L;

void KCursorPrivate::start()
{
    if ( !s_autoHideTimer ) {
	s_autoHideTimer = new QTimer;
	QObject::connect( s_autoHideTimer, SIGNAL( timeout() ),
			  this, SLOT( slotHideCursor() ));
    }
    s_count++;
}

void KCursorPrivate::stop()
{
    s_count--;
    if ( s_count <= 0 ) {
	delete s_autoHideTimer;
	s_autoHideTimer = 0L;
	s_count = 0;
    }
}

void KCursorPrivate::hideCursor( QWidget *w )
{
    if ( !w )
	return;

    s_isCursorHidden = true;
    // FIXME (when Qt 2.2 is out, QWidget::hasOwnCursor())
    s_isOwnCursor = w->testWState( WState_OwnCursor );
    if ( s_isOwnCursor ) {
	delete s_oldCursor;
	s_oldCursor = new QCursor( w->cursor() );
    }

    w->setCursor( KCursor::blankCursor() );
}

void KCursorPrivate::unhideCursor( QWidget *w )
{
    s_isCursorHidden = false;
    if ( w ) {
	if ( s_isOwnCursor ) {
	    w->setCursor( *s_oldCursor );
	    delete s_oldCursor;
	    s_oldCursor = 0L;
	}
	else
	    w->unsetCursor();
    }
}


// what a mess :-/
bool KCursorPrivate::eventFilter( QObject *o, QEvent *e )
{
    if ( !o->isWidgetType() ) // should never happen, actually
	return false;

    int t = e->type();
    QWidget *w = static_cast<QWidget *>( o );

    if ( t == QEvent::Leave || t == QEvent::FocusOut ) {
	s_autoHideTimer->stop();
	if ( s_isCursorHidden )
	    unhideCursor( w );
	
	s_isCursorHidden = false;
	return false;
    }
    
    // don't process events not coming from the focus-widget
    if ( w != qApp->focusWidget() )
	 return false;

    else if ( t == QEvent::Enter ) {
	if ( s_isCursorHidden )
	    unhideCursor( w );
	s_isCursorHidden = false;
	s_autoHideTimer->start( s_hideCursorDelay, true );
    }
	
    else { // no enter/leave/focus events
	if ( s_isCursorHidden ) {
	    if ( t == QEvent::MouseButtonPress ||
		 t ==QEvent::MouseButtonRelease ||
		 t == QEvent::MouseButtonDblClick || t == QEvent::MouseMove ||
		 t == QEvent::Show || t == QEvent::Hide )
		unhideCursor( w );
	}
	else { // cursor not hidden yet
	    if ( t == QEvent::KeyPress ) { //t == QEvent::KeyRelease ) {
		if ( insideWidget( QCursor::pos(), w ))
		    hideCursor( w );
	    }
	    else {
		if ( insideWidget( QCursor::pos(), w ))
		    s_autoHideTimer->start( s_hideCursorDelay, true );
	    }
	}
    }
    return false;
}

void KCursorPrivate::slotHideCursor()
{
    if ( !s_isCursorHidden )
	hideCursor( kapp->focusWidget() );
}

bool KCursorPrivate::insideWidget( const QPoint &p, QWidget *w )
{
    return w->geometry().contains( w->mapFromGlobal( p ) );
}

#include "kcursor.moc"
