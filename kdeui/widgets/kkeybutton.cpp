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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kkeybutton.h"
#include "kkeyserver.h"
#include "kshortcutdialog.h"

#include <qcursor.h>
#include <qdrawutil.h>
#include <qpainter.h>
#include <QPolygon>
#include <QStyle>
#include <QKeyEvent>

#include <kdebug.h>
#include <kglobalaccel.h>
#include <klocale.h>

/*#ifdef Q_WS_X11
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
#endif // KeyPress
#endif // Q_WS_X11*/

//static const char* psTemp[] = {
//  I18N_NOOP("Primary"), I18N_NOOP("Alternate"), I18N_NOOP("Multi-Key")
//};

/***********************************************************************/
/* KKeyButton                                                          */
/*                                                                     */
/* Initially added by Mark Donohoe <donohoe@kde.org>                   */
/*                                                                     */
/***********************************************************************/
 
class KKeyButtonPrivate
{
public:
	KKeyButtonPrivate(KKeyButton *q): q(q) {}

	static QKeySequence appendToSequence(const QKeySequence& seq, int keyQt);
	void updateShortcutDisplay();

	//private slots
	void captureShortcut();

	KKeyButton *q;
	QKeySequence keySequence;
	uint nKey;
	uint modifierKeys;
	bool isRecording;
};


KKeyButton::KKeyButton(QWidget *parent)
: QPushButton( parent ), d(new KKeyButtonPrivate(this))
{
	setFocusPolicy( Qt::StrongFocus );
	d->isRecording = false;
	connect(this, SIGNAL(clicked()), this, SLOT(captureShortcut()));
	setShortcut(QKeySequence());
}


KKeyButton::~KKeyButton ()
{
	delete d;
}


//slot
void KKeyButton::captureShortcut()
{
	startRecording();
}


QKeySequence KKeyButton::keySequence() const
{
	return d->keySequence;
}


void KKeyButton::setKeySequence(const QKeySequence &seq)
{
	d->keySequence = seq;
	QString keyStr = d->keySequence.toString();
	keyStr.replace(QLatin1Char('&'), QLatin1String("&&"));
	setText(keyStr.isEmpty() ? i18n("None") : keyStr);
}


void KKeyButton::setText( const QString& text )
{
	QPushButton::setText( text );
	setFixedSize( sizeHint().width()+12, sizeHint().height()+8 );
}


void KKeyButton::startRecording()
{
	d->nKey = 0;
	d->modifierKeys = 0;
	d->isRecording = true;
	grabKeyboard();
	//### it's not clear to me when exactly to repaint. try stuff.
	repaint();
}


void KKeyButton::doneRecording()
{
	//### other things that need to be done
	d->isRecording = false;
	releaseKeyboard();
	emit capturedKeySequence(d->keySequence);
}


void KKeyButton::paintEvent( QPaintEvent* )
{
  QPainter painter(this);
  painter.setRenderHint( QPainter::Antialiasing );

  QPolygon a( 4 );
  a.setPoint( 0, 0, 0) ;
  a.setPoint( 1, width(), 0 );
  a.setPoint( 2, 0, height() );
  a.setPoint( 3, 0, 0 );

  QRegion r1( a );
  painter.setClipRegion( r1 );
  painter.setBrush( palette().color( QPalette::Background ).light() );
  painter.drawRoundRect( 0, 0, width(), height(), 20, 20);

  a.setPoint( 0, width(), height() );
  a.setPoint( 1, width(), 0 );
  a.setPoint( 2, 0, height() );
  a.setPoint( 3, width(), height() );

  QRegion r2( a );
  painter.setClipRegion( r2 );
  painter.setBrush( palette().color( QPalette::Background ).dark() );
  painter.drawRoundRect( 0, 0, width(), height(), 20, 20 );

  painter.setClipping( false );
  if( width() > 12 && height() > 8 )
    qDrawShadePanel( &painter, 6, 4, width() - 12, height() - 8,
                     palette(), true, 1, 0L );
  if (d->isRecording)
  {
    painter.setPen( palette().color( QPalette::Base ) );
    painter.setBrush( palette().color( QPalette::Base ) );
  }
  else
  {
    painter.setPen( palette().color( QPalette::Background ) );
    painter.setBrush( palette().color( QPalette::Background ) );
  }
  if( width() > 14 && height() > 10 )
    painter.drawRect( 7, 5, width() - 14, height() - 10 );

  painter.setPen( palette().color( QPalette::Text ) );

  style()->drawItemText( &painter, rect(), Qt::AlignCenter | Qt::TextShowMnemonic,
                         palette(), isEnabled(), text() );

  painter.setBrush( Qt::NoBrush );
  if( hasFocus() || d->isRecording )
  {
    if( width() > 16 && height() > 12 )
      painter.drawRect( 8, 6, width() - 16, height() - 12 );
  }

}


void KKeyButtonPrivate::updateShortcutDisplay()
{
	//empty string if no non-modifier was pressed
	QString s = keySequence.toString();
	s.replace('&', QLatin1String("&&"));

	//### really needed???
	q->setFocus();
	if (isRecording) {
		// Display modifiers for the first key in the QKeySequence
		if (nKey == 0) {
			if (modifierKeys) {
#if defined(Q_WS_MAC)
				if(modifierKeys & Qt::META)  s += KKeyServer::modToStringUser(Qt::META) + '+';
				if(modifierKeys & Qt::ALT)   s += KKeyServer::modToStringUser(Qt::ALT) + '+';
				if(modifierKeys & Qt::CTRL)  s += KKeyServer::modToStringUser(Qt::CTRL) + '+';
				if(modifierKeys & Qt::SHIFT) s += KKeyServer::modToStringUser(Qt::SHIFT) + '+';
#elif defined(Q_WS_X11)
				if(modifierKeys & Qt::META)  s += KKeyServer::modToStringUser(Qt::META) + '+';
				if(modifierKeys & Qt::CTRL)  s += KKeyServer::modToStringUser(Qt::CTRL) + '+';
				if(modifierKeys & Qt::ALT)   s += KKeyServer::modToStringUser(Qt::ALT) + '+';
				if(modifierKeys & Qt::SHIFT) s += KKeyServer::modToStringUser(Qt::SHIFT) + '+';
#endif
			} else
				s = i18nc("What the user inputs now will be taken as the new shortcut", "Capturing");
		}
	}
	//make it clear that input is still going on
	s.append("...");

	q->setText(s);
}


void KKeyButton::keyPressEvent(QKeyEvent *e)
{
	QPushButton::keyPressEvent(e);

	//if key is a letter, it must be stored as lowercase
	int keyQt = QChar( e->key() & 0xff ).isLetter() ?
		(QChar( e->key() & 0xff ).toLower().toLatin1() | (e->key() & 0xffff00) )
		: e->key();

	uint newModifiers = e->modifiers() & (Qt::SHIFT | Qt::CTRL | Qt::ALT | Qt::META);
	
	if (!d->isRecording) {
		if (keyQt == Qt::Key_Return) {
			startRecording();
			d->modifierKeys = newModifiers;
			d->updateShortcutDisplay();
		}
		return;
	}

	if (d->modifierKeys != newModifiers && d->nKey == 0)
		d->modifierKeys = newModifiers;

	switch(keyQt) {
	case Qt::Key_Shift:
	case Qt::Key_Control:
	case Qt::Key_Alt:
	case Qt::Key_Meta:
	case Qt::Key_Menu: //unused (yes, but why?)
		break;
	default:
		if(keyQt == Qt::Key_Return && d->nKey > 0)
			doneRecording();
		else if (keyQt) {
			if (d->nKey == 0)
				d->keySequence = KKeyButtonPrivate::appendToSequence(d->keySequence, keyQt | d->modifierKeys);
			else
				d->keySequence = KKeyButtonPrivate::appendToSequence(d->keySequence, keyQt);

			d->nKey++;
			d->updateShortcutDisplay();
		}
		return;
	}

	// If we are editing the first key in the sequence,
	// display modifier keys which are held down
	if(d->nKey == 0)
		d->updateShortcutDisplay();
}


void KKeyButton::keyReleaseEvent(QKeyEvent *e)
{
	QPushButton::keyReleaseEvent(e);
	if (!d->isRecording)
		return;

	uint newModifiers = e->modifiers() & (Qt::SHIFT | Qt::CTRL | Qt::ALT | Qt::META);

	//if a modifier that belongs to the shortcut was released...
	if ((newModifiers & d->modifierKeys) < d->modifierKeys) {
		if (d->nKey == 0) {
			d->modifierKeys = newModifiers;
			d->updateShortcutDisplay();
		} else
			doneRecording();
	}
}


QKeySequence KKeyButtonPrivate::appendToSequence(const QKeySequence& seq, int keyQt)
{
	switch (seq.count()) {
	case 0:
		return QKeySequence(keyQt);
	case 1:
		return QKeySequence(seq[0], keyQt);
	case 2:
		return QKeySequence(seq[0], seq[1], keyQt);
	case 3:
		return QKeySequence(seq[0], seq[1], seq[2], keyQt);
	default:
		return seq;
	}
}

#include "kkeybutton.moc"
