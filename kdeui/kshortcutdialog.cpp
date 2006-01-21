/* This file is part of the KDE libraries
    Copyright (C) 2002,2003 Ellis Whitehead <ellis@kde.org>

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

#include "kshortcutdialog.h"

#include <qvariant.h>
#include <QKeyEvent>

#ifdef Q_WS_X11
	#define XK_XKB_KEYS
	#define XK_MISCELLANY
	#include <X11/Xlib.h>	// For x11Event()
	#include <X11/keysymdef.h> // For XK_...
	#include <fixx11h.h>

	#ifdef KeyPress
		const int XKeyPress = KeyPress;
		const int XKeyRelease = KeyRelease;
		const int XFocusOut = FocusOut;
		const int XFocusIn = FocusIn;
		#undef KeyRelease
		#undef KeyPress
		#undef FocusOut
		#undef FocusIn
	#endif
#elif defined(Q_WS_WIN)
# include <kkeyserver.h>
#endif

#include <kshortcutdialog_simple.h>
#include <kshortcutdialog_advanced.h>

#include <qcheckbox.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qtimer.h>


#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kkeynative.h>
#include <klocale.h>
#include <kstdguiitem.h>
#include <kpushbutton.h>
#include <kvbox.h>

bool KShortcutDialog::s_showMore = false;

KShortcutDialog::KShortcutDialog( const KShortcut& shortcut, bool bQtShortcut, QWidget* parent )
: KDialog( parent, i18n("Configure Shortcut"),
               KDialog::Details|KDialog::Ok|KDialog::Cancel )
{
  enableButtonSeparator( true );
  setModal(true);

        setButtonText(Details, i18n("Advanced"));
        m_stack = new KVBox(this);
        m_stack->setMinimumWidth(360);
        m_stack->setSpacing(0);
        m_stack->setMargin(0);
        setMainWidget(m_stack);
        
        m_simple = new KShortcutDialogSimple(m_stack);

        m_adv = new KShortcutDialogAdvanced(m_stack);
        m_adv->hide();
        
	m_bQtShortcut = bQtShortcut;

	m_bGrab = false;
	m_iSeq = 0;
	m_iKey = 0;
	m_ptxtCurrent = 0;
	m_bRecording = false;
	m_mod = 0;

	m_simple->m_btnClearShortcut->setPixmap( SmallIcon( "locationbar_erase" ) );
	m_adv->m_btnClearPrimary->setPixmap( SmallIcon( "locationbar_erase" ) );
	m_adv->m_btnClearAlternate->setPixmap( SmallIcon( "locationbar_erase" ) );
	connect(m_simple->m_btnClearShortcut, SIGNAL(clicked()),
	        this, SLOT(slotClearShortcut()));
	connect(m_adv->m_btnClearPrimary, SIGNAL(clicked()),
	        this, SLOT(slotClearPrimary()));
	connect(m_adv->m_btnClearAlternate, SIGNAL(clicked()),
	        this, SLOT(slotClearAlternate()));

	connect(m_adv->m_txtPrimary, SIGNAL(clicked()),
		m_adv->m_btnPrimary, SLOT(animateClick()));
	connect(m_adv->m_txtAlternate, SIGNAL(clicked()),
		m_adv->m_btnAlternate, SLOT(animateClick()));
	connect(m_adv->m_btnPrimary, SIGNAL(clicked()),
		this, SLOT(slotSelectPrimary()));
	connect(m_adv->m_btnAlternate, SIGNAL(clicked()),
		this, SLOT(slotSelectAlternate()));

	KGuiItem ok = KStdGuiItem::ok();
	ok.setText( i18n( "OK" ) );
	setButtonGuiItem( Ok , ok );

	KGuiItem cancel = KStdGuiItem::cancel();
	cancel.setText( i18n( "Cancel" ) );
	setButtonGuiItem( Cancel, cancel );

	setShortcut( shortcut );
	resize( 0, 0 );

	s_showMore = KConfigGroup(KGlobal::config(), "General").readEntry("ShowAlternativeShortcutConfig", s_showMore);
	updateDetails();

	#ifdef Q_WS_X11
	kapp->installX11EventFilter( this );	// Allow button to capture X Key Events.
	#endif
}

KShortcutDialog::~KShortcutDialog()
{
	KConfigGroup group(KGlobal::config(), "General");
	group.writeEntry("ShowAlternativeShortcutConfig", s_showMore);
}

void KShortcutDialog::setShortcut( const KShortcut & shortcut )
{
	m_shortcut = shortcut;
	updateShortcutDisplay();
}

void KShortcutDialog::updateShortcutDisplay()
{
	QString s[2] = { m_shortcut.seq(0).toString(), m_shortcut.seq(1).toString() };

	if( m_bRecording ) {
		m_ptxtCurrent->setDefault( true );
		m_ptxtCurrent->setFocus();

		// Display modifiers for the first key in the KKeySequence
		if( m_iKey == 0 ) {
			if( m_mod ) {
				QString keyModStr;
				if( m_mod & KKey::WIN )   keyModStr += KKey::modFlagLabel(KKey::WIN) + "+";
				if( m_mod & KKey::ALT )   keyModStr += KKey::modFlagLabel(KKey::ALT) + "+";
				if( m_mod & KKey::CTRL )  keyModStr += KKey::modFlagLabel(KKey::CTRL) + "+";
				if( m_mod & KKey::SHIFT ) keyModStr += KKey::modFlagLabel(KKey::SHIFT) + "+";
				s[m_iSeq] = keyModStr;
	}
		}
		// When in the middle of entering multi-key shortcuts,
		//  add a "," to the end of the displayed shortcut.
		else
			s[m_iSeq] += ",";
	}
	else {
		m_adv->m_txtPrimary->setDefault( false );
		m_adv->m_txtAlternate->setDefault( false );
		this->setFocus();
	}
	
	s[0].replace('&', QLatin1String("&&"));
	s[1].replace('&', QLatin1String("&&"));

	m_simple->m_txtShortcut->setText( s[0] );
	m_adv->m_txtPrimary->setText( s[0] );
	m_adv->m_txtAlternate->setText( s[1] );

	// Determine the enable state of the 'Less' button
	bool bLessOk;
	// If there is no shortcut defined,
	if( m_shortcut.count() == 0 )
		bLessOk = true;
	// If there is a single shortcut defined, and it is not a multi-key shortcut,
	else if( m_shortcut.count() == 1 && m_shortcut.seq(0).count() <= 1 )
		bLessOk = true;
	// Otherwise, we have an alternate shortcut or multi-key shortcut(s).
	else
		bLessOk = false;
	enableButton(Details, bLessOk);
}

void KShortcutDialog::slotDetails()
{
	s_showMore = (m_adv->isHidden());
	updateDetails();
}

void KShortcutDialog::updateDetails()
{
	bool showAdvanced = s_showMore || (m_shortcut.count() > 1);
	setDetails(showAdvanced);
	m_bRecording = false;
	m_iSeq = 0;
	m_iKey = 0;

	if (showAdvanced)
	{
		m_simple->hide();
		m_adv->show();
		m_adv->m_btnPrimary->setChecked( true );
		slotSelectPrimary();
	}
	else
	{
		m_ptxtCurrent = m_simple->m_txtShortcut;
		m_adv->hide();
		m_simple->show();
		m_simple->m_txtShortcut->setDefault( true );
		m_simple->m_txtShortcut->setFocus();
		m_adv->m_btnMultiKey->setChecked( false );
	}
	qApp->processEvents();
	adjustSize();
}

void KShortcutDialog::slotSelectPrimary()
{
	m_bRecording = false;
	m_iSeq = 0;
	m_iKey = 0;
	m_ptxtCurrent = m_adv->m_txtPrimary;
	m_ptxtCurrent->setDefault(true);
	m_ptxtCurrent->setFocus();
	updateShortcutDisplay();
}

void KShortcutDialog::slotSelectAlternate()
{
	m_bRecording = false;
	m_iSeq = 1;
	m_iKey = 0;
	m_ptxtCurrent = m_adv->m_txtAlternate;
	m_ptxtCurrent->setDefault(true);
	m_ptxtCurrent->setFocus();
	updateShortcutDisplay();
}

void KShortcutDialog::slotClearShortcut()
{
	m_shortcut.setSeq( 0, KKeySequence() );
	updateShortcutDisplay();
}

void KShortcutDialog::slotClearPrimary()
{
	m_shortcut.setSeq( 0, KKeySequence() );
	m_adv->m_btnPrimary->setChecked( true );
	slotSelectPrimary();
}

void KShortcutDialog::slotClearAlternate()
{
	if( m_shortcut.count() == 2 )
		m_shortcut.init( m_shortcut.seq(0) );
	m_adv->m_btnAlternate->setChecked( true );
	slotSelectAlternate();
}

void KShortcutDialog::slotMultiKeyMode( bool bOn )
{
	// If turning off multi-key mode during a recording,
	if( !bOn && m_bRecording ) {
		m_bRecording = false;
	m_iKey = 0;
		updateShortcutDisplay();
	}
}

#ifdef Q_WS_X11
/* we don't use the generic Qt code on X11 because it allows us 
 to grab the keyboard so that all keypresses are seen
 */
bool KShortcutDialog::x11Event( XEvent *pEvent )
{
	switch( pEvent->type ) {
		case XKeyPress:
			x11KeyPressEvent( pEvent );
			return true;
		case XKeyRelease:
			x11KeyReleaseEvent( pEvent );
				return true;
		case XFocusIn:
			if (!m_bGrab) {
				//kdDebug(125) << "FocusIn and Grab!" << endl;
				grabKeyboard();
				m_bGrab = true;
			}
			//else
			//	kdDebug(125) << "FocusIn" << endl;
			break;
		case XFocusOut:
			if (m_bGrab) {
				//kdDebug(125) << "FocusOut and Ungrab!" << endl;
				releaseKeyboard();
				m_bGrab = false;
			}
			//else
			//	kdDebug(125) << "FocusOut" << endl;
			break;
		default:
			//kdDebug(125) << "x11Event->type = " << pEvent->type << endl;
			break;
	}
	return KDialog::x11Event( pEvent );
}

static uint getModsFromModX( uint keyModX )
{
	uint mod = 0;
	if( keyModX & KKeyNative::modXShift() ) mod += KKey::SHIFT;
	if( keyModX & KKeyNative::modXCtrl() )  mod += KKey::CTRL;
	if( keyModX & KKeyNative::modXAlt() )   mod += KKey::ALT;
	if( keyModX & KKeyNative::modXWin() )   mod += KKey::WIN;
	return mod;
}

static bool convertSymXToMod( uint keySymX, uint* pmod )
{
	switch( keySymX ) {
		// Don't allow setting a modifier key as an accelerator.
		// Also, don't release the focus yet.  We'll wait until
		//  we get a 'normal' key.
		case XK_Shift_L:   case XK_Shift_R:   *pmod = KKey::SHIFT; break;
		case XK_Control_L: case XK_Control_R: *pmod = KKey::CTRL; break;
		case XK_Alt_L:     case XK_Alt_R:     *pmod = KKey::ALT; break;
		// FIXME: check whether the Meta or Super key are for the Win modifier
		case XK_Meta_L:    case XK_Meta_R:
		case XK_Super_L:   case XK_Super_R:   *pmod = KKey::WIN; break;
		case XK_Hyper_L:   case XK_Hyper_R:
		case XK_Mode_switch:
		case XK_Num_Lock:
		case XK_Caps_Lock:
			break;
		default:
			return false;
				}
	return true;
}

void KShortcutDialog::x11KeyPressEvent( XEvent* pEvent )
{
	KKeyNative keyNative( pEvent );
	uint keyModX = keyNative.mod();
	uint keySymX = keyNative.sym();

	m_mod = getModsFromModX( keyModX );

	if( keySymX ) {
		m_bRecording = true;

		uint mod = 0;
		if( convertSymXToMod( keySymX, &mod ) ) {
			if( mod )
				m_mod |= mod;
		}
		else
			keyPressed( KKey(keyNative) );
	}
	updateShortcutDisplay();
}

void KShortcutDialog::x11KeyReleaseEvent( XEvent* pEvent )
{
	// We're only interested in the release of modifier keys,
	//  and then only when it's for the first key in a sequence.
	if( m_bRecording && m_iKey == 0 ) {
		KKeyNative keyNative( pEvent );
		uint keyModX = keyNative.mod();
		uint keySymX = keyNative.sym();

		m_mod = getModsFromModX( keyModX );

		uint mod = 0;
		if( convertSymXToMod( keySymX, &mod ) && mod ) {
			m_mod &= ~mod;
			if( !m_mod )
				m_bRecording = false;
		}
		updateShortcutDisplay();
	}
}
#elif defined(Q_WS_WIN)
void KShortcutDialog::keyPressEvent( QKeyEvent * e )
{
	kdDebug() << e->text() << " " << (int)e->text()[0].toLatin1()<<  " " << (int)e->ascii() << endl;
	//if key is a letter, it must be stored as lowercase
	int keyQt = QChar( e->key() & 0xff ).isLetter() ? 
		(QChar( e->key() & 0xff ).toLower().toLatin1() | (e->key() & 0xffff00) )
		: e->key();
	int modQt = KKeyServer::qtButtonStateToMod( e->modifiers() );
	KKeyNative keyNative( KKey(keyQt, modQt) );
	m_mod = keyNative.mod();
	uint keySym = keyNative.sym();

	switch( keySym ) {
		case Qt::Key_Shift: 
			m_mod |= KKey::SHIFT;
			m_bRecording = true;
			break;
		case Qt::Key_Control:
			m_mod |= KKey::CTRL;
			m_bRecording = true;
			break;
		case Qt::Key_Alt:
			m_mod |= KKey::ALT;
			m_bRecording = true;
			break;
		case Qt::Key_Menu:
		case Qt::Key_Meta: //unused
			break;
		default:
			if( keyNative.sym() == Qt::Key_Return && m_iKey > 0 ) {
				accept();
				return;
			}
			//accept
			if (keyNative.sym()) {
				KKey key = keyNative;
				key.simplify();
				KKeySequence seq;
				if( m_iKey == 0 )
					seq = key;
				else {
					seq = m_shortcut.seq( m_iSeq );
					seq.setKey( m_iKey, key );
				}
				m_shortcut.setSeq( m_iSeq, seq );

				if(m_adv->m_btnMultiKey->isChecked())
					m_iKey++;

				m_bRecording = true;

				updateShortcutDisplay();

				if( !m_adv->m_btnMultiKey->isChecked() )
					QTimer::singleShot(500, this, SLOT(accept()));
			}
			return;
	}

	// If we are editing the first key in the sequence,
	//  display modifier keys which are held down
	if( m_iKey == 0 ) {
		updateShortcutDisplay();
	}
}

bool KShortcutDialog::event ( QEvent * e )
{
	if (e->type()==QEvent::KeyRelease) {
		int modQt = KKeyServer::qtButtonStateToMod( static_cast<QKeyEvent*>(e)->modifiers() );
		KKeyNative keyNative( KKey(static_cast<QKeyEvent*>(e)->key(), modQt) );
		uint keySym = keyNative.sym();

		bool change = true;
		switch( keySym ) {
		case Qt::Key_Shift: 
			if (m_mod & KKey::SHIFT)
				m_mod ^= KKey::SHIFT;
			break;
		case Qt::Key_Control:
			if (m_mod & KKey::CTRL)
				m_mod ^= KKey::CTRL;
			break;
		case Qt::Key_Alt:
			if (m_mod & KKey::ALT)
				m_mod ^= KKey::ALT;
			break;
		default:
			change = false;
		}
		if (change)
			updateShortcutDisplay();
	}
	return KDialog::event(e);
}
#endif

void KShortcutDialog::keyPressed( KKey key )
{
	kdDebug(125) << "keyPressed: " << key.toString() << endl;

	key.simplify();
	if( m_bQtShortcut ) {
		key = key.keyCodeQt();
		if( key.isNull() ) {
			// TODO: message box about key not able to be used as application shortcut
		}
	}

	KKeySequence seq;
	if( m_iKey == 0 )
		seq = key;
	else {
		// Remove modifiers
		key.init( key.sym(), 0 );
		seq = m_shortcut.seq( m_iSeq );
		seq.setKey( m_iKey, key );
	}

	m_shortcut.setSeq( m_iSeq, seq );

	m_mod = 0;
	if( m_adv->m_btnMultiKey->isChecked() && m_iKey < KKeySequence::MAX_KEYS - 1 )
		m_iKey++;
	else {
		m_iKey = 0;
		m_bRecording = false;
	}

	updateShortcutDisplay();

	if( !m_adv->m_btnMultiKey->isChecked() )
		QTimer::singleShot(500, this, SLOT(accept()));
}

#include "kshortcutdialog.moc"
