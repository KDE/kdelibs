/* This file is part of the KDE libraries
    Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>

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

#include "kkeybutton.h"

#include <qcursor.h>
#include <qdrawutil.h>
#include <qpainter.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobalaccel.h>
#include <kkey_x11.h>
#include <klocale.h>

#ifdef Q_WS_X11
#define XK_XKB_KEYS
#define XK_MISCELLANY
#include <X11/Xlib.h>	// For x11Event()
#include <X11/keysymdef.h> // For XK_...

#ifdef KeyPress
const int XFocusOut = FocusOut;
const int XFocusIn = FocusIn;
const int XKeyPress = KeyPress;
const int XKeyRelease = KeyRelease;
#undef KeyRelease
#undef KeyPress
#undef FocusOut
#undef FocusIn
#endif
#endif

/***********************************************************************/
/* KKeyButton                                                          */
/*                                                                     */
/* Initially added by Mark Donohoe <donohoe@kde.org>                   */
/*                                                                     */
/***********************************************************************/

KKeyButton::KKeyButton(QWidget *parent, const char *name)
:	QPushButton( parent, name )
{
	setFocusPolicy( QWidget::StrongFocus );
	m_bEditing = false;
	connect( this, SIGNAL(clicked()), this, SLOT(slotCaptureKey()) );
#ifdef Q_WS_X11
	kapp->installX11EventFilter( this );	// Allow button to capture X Key Events.
#endif
	setKey( KAccelShortcut() );
}

KKeyButton::~KKeyButton ()
{
	if( m_bEditing )
		captureKey( false );
}

void KKeyButton::setKey( KAccelShortcut cut )
{
	m_cut = cut;
	QString keyStr = m_cut.toString();
	setText( keyStr.isEmpty() ? i18n("None") : keyStr );
}

void KKeyButton::setText( const QString& text )
{
	QPushButton::setText( text );
	setFixedSize( sizeHint().width()+12, sizeHint().height()+8 );
}

void KKeyButton::captureKey( bool bCapture )
{
	m_bEditing = bCapture;
	if( m_bEditing == true ) {
		setFocus();
		KGlobalAccel::setKeyEventsEnabled( false );
		grabKeyboard();
		grabMouse( IbeamCursor );
	} else {
		releaseMouse();
		releaseKeyboard();
		KGlobalAccel::setKeyEventsEnabled( true );
	}
	repaint();
}

void KKeyButton::slotCaptureKey()
{
	captureKey( true );
}

#ifdef Q_WS_X11
bool KKeyButton::x11Event( XEvent *pEvent )
{
	if( m_bEditing ) {
		//kdDebug(125) << "x11Event: type: " << pEvent->type << " window: " << pEvent->xany.window << endl;
		switch( pEvent->type ) {
			case XKeyPress:
			case XKeyRelease:
				x11EventKeyPress( pEvent );
				return true;
			case ButtonPress:
				captureKey( false );
				setKey( m_cut );
				return true;
		}
	}
	return QWidget::x11Event( pEvent );
}

void KKeyButton::x11EventKeyPress( XEvent *pEvent )
{
	KKeySequence key = KKeyX11::keyEventXToKey( pEvent );
	uint keyModX = key.m_keyMod, keySymX = key.m_keySym;
	KAccelShortcut cut;
	cut.init( key );

	//kdDebug(125) << QString( "keycode: 0x%1 state: 0x%2\n" )
	//			.arg( pEvent->xkey.keycode, 0, 16 ).arg( pEvent->xkey.state, 0, 16 );

	switch( keySymX ) {
		// Don't allow setting a modifier key as an accelerator.
		// Also, don't release the focus yet.  We'll wait until
		//  we get a 'normal' key.
		case XK_Shift_L:   case XK_Shift_R:	keyModX = KKeyX11::keyModXShift(); break;
		case XK_Control_L: case XK_Control_R:	keyModX = KKeyX11::keyModXCtrl(); break;
		case XK_Alt_L:     case XK_Alt_R:	keyModX = KKeyX11::keyModXAlt(); break;
		case XK_Meta_L:    case XK_Meta_R:	keyModX = KKeyX11::keyModXMeta(); break;
		case XK_Super_L:   case XK_Super_R:
		case XK_Hyper_L:   case XK_Hyper_R:
		case XK_Mode_switch:
			break;
		default:
			if( !key.isNull() ) {
				captureKey( false );
				// The parent must decide whether this is a valid
				//  key, and if so, call setKey(uint) with the new value.
				emit capturedKey( cut );
				setKey( m_cut );
			}
			return;
	}

	if( pEvent->type == XKeyPress )
		keyModX |= pEvent->xkey.state;
	else
		keyModX = pEvent->xkey.state & ~keyModX;

	QString keyModStr;
	if( keyModX & KKeyX11::keyModXMeta() )	keyModStr += i18n("Meta") + "+";
	if( keyModX & KKeyX11::keyModXAlt() )	keyModStr += i18n("Alt") + "+";
	if( keyModX & KKeyX11::keyModXCtrl() )	keyModStr += i18n("Ctrl") + "+";
	if( keyModX & KKeyX11::keyModXShift() )	keyModStr += i18n("Shift") + "+";

	// Display currently selected modifiers, or redisplay old key.
	if( !keyModStr.isEmpty() )
		setText( keyModStr );
	else
		setKey( m_cut );
}
#endif

void KKeyButton::drawButton( QPainter *painter )
{
  QPointArray a( 4 );
  a.setPoint( 0, 0, 0) ;
  a.setPoint( 1, width(), 0 );
  a.setPoint( 2, 0, height() );
  a.setPoint( 3, 0, 0 );

  QRegion r1( a );
  painter->setClipRegion( r1 );
  painter->setBrush( backgroundColor().light() );
  painter->drawRoundRect( 0, 0, width(), height(), 20, 20);

  a.setPoint( 0, width(), height() );
  a.setPoint( 1, width(), 0 );
  a.setPoint( 2, 0, height() );
  a.setPoint( 3, width(), height() );

  QRegion r2( a );
  painter->setClipRegion( r2 );
  painter->setBrush( backgroundColor().dark() );
  painter->drawRoundRect( 0, 0, width(), height(), 20, 20 );

  painter->setClipping( false );
  if( width() > 12 && height() > 8 )
    qDrawShadePanel( painter, 6, 4, width() - 12, height() - 8,
                     colorGroup(), true, 1, 0L );
  if ( m_bEditing )
  {
    painter->setPen( colorGroup().base() );
    painter->setBrush( colorGroup().base() );
  }
  else
  {
    painter->setPen( backgroundColor() );
    painter->setBrush( backgroundColor() );
  }
  if( width() > 14 && height() > 10 )
    painter->drawRect( 7, 5, width() - 14, height() - 10 );

  drawButtonLabel( painter );

  painter->setPen( colorGroup().text() );
  painter->setBrush( NoBrush );
  if( hasFocus() || m_bEditing )
  {
    if( width() > 16 && height() > 12 )
      painter->drawRect( 8, 6, width() - 16, height() - 12 );
  }

}

#include "kkeybutton.moc"
