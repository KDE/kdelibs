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
#include "kshortcutdialog.h"

#include <qcursor.h>
#include <qdrawutil.h>
#include <qpainter.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobalaccel.h>
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

const char* psTemp[] = { I18N_NOOP("Primary"), I18N_NOOP("Alternate"), I18N_NOOP("Multi-Key") };

class KKeyButtonPrivate
{
 public:
	bool bQtShortcut;
};

/***********************************************************************/
/* KKeyButton                                                          */
/*                                                                     */
/* Initially added by Mark Donohoe <donohoe@kde.org>                   */
/*                                                                     */
/***********************************************************************/

KKeyButton::KKeyButton(QWidget *parent, const char *name)
:	QPushButton( parent, name )
{
	d = new KKeyButtonPrivate;
	setFocusPolicy( QWidget::StrongFocus );
	m_bEditing = false;
	connect( this, SIGNAL(clicked()), this, SLOT(captureShortcut()) );
	setShortcut( KShortcut(), true );
}

KKeyButton::~KKeyButton ()
{
	delete d;
}

void KKeyButton::setShortcut( const KShortcut& cut, bool bQtShortcut )
{
	d->bQtShortcut = bQtShortcut;
	m_cut = cut;
	QString keyStr = m_cut.toString();
	setText( keyStr.isEmpty() ? i18n("None") : keyStr );
}

// deprecated //
void KKeyButton::setShortcut( const KShortcut& cut )
{
	setShortcut( cut, false );
}

void KKeyButton::setText( const QString& text )
{
	QPushButton::setText( text );
	setFixedSize( sizeHint().width()+12, sizeHint().height()+8 );
}

void KKeyButton::captureShortcut()
{
	KShortcut cut;

	m_bEditing = true;
	repaint();

	KShortcutDialog dlg( m_cut, d->bQtShortcut, this );
	if( dlg.exec() == KDialog::Accepted )
		emit capturedShortcut( dlg.cut() );

	m_bEditing = false;
	repaint();
}

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

void KKeyButton::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kkeybutton.moc"
