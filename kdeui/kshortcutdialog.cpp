/* This file is part of the KDE libraries
    Copyright (C) 2002,2003 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#include <ui_kshortcutdialog_simple.h>
#include <ui_kshortcutdialog_advanced.h>

#include <qcheckbox.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qtimer.h>
#include <QStackedWidget>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstdguiitem.h>
#include <kpushbutton.h>

#include "kkeyserver.h"

bool KShortcutDialog::s_showMore = false;

class KShortcutDialogPrivate
{
public:
	KShortcut shortcut;
	bool bGrab;
	KPushButton* ptxtCurrent;
	uint iSeq;
	uint iKey;
	bool bRecording;
	uint mod;
	QWidget* simpleBase;
	QWidget* advBase;
	Ui::KShortcutDialogSimple *simple;
	Ui::KShortcutDialogAdvanced *adv;
	QStackedWidget *stack;
};

KShortcutDialog::KShortcutDialog( const KShortcut& shortcut, QWidget* parent )
	: KDialog( parent, i18n("Configure Shortcut"), KDialog::Details|KDialog::Ok|KDialog::Cancel )
	, d(new KShortcutDialogPrivate)
{
	enableButtonSeparator( true );
	setModal(true);

	setButtonText(Details, i18n("Advanced"));
	d->stack = new QStackedWidget(this);
	d->stack->setMinimumWidth(360);
	setMainWidget(d->stack);

	d->simpleBase = new QWidget(d->stack);
	d->simple = new Ui::KShortcutDialogSimple();
	d->simple->setupUi(d->simpleBase);
	d->stack->addWidget(d->simpleBase);

	d->advBase = new QWidget(d->stack);
	d->adv = new Ui::KShortcutDialogAdvanced();
	d->adv->setupUi(d->advBase);
	d->stack->addWidget(d->advBase);

	d->stack->setCurrentWidget(d->simpleBase);

	d->bGrab = false;
	d->iSeq = 0;
	d->iKey = 0;
	d->ptxtCurrent = 0;
	d->bRecording = false;
	d->mod = 0;

	d->simple->m_btnClearShortcut->setIcon( SmallIcon( "locationbar_erase" ) );
	d->adv->m_btnClearPrimary->setIcon( SmallIcon( "locationbar_erase" ) );
	d->adv->m_btnClearAlternate->setIcon( SmallIcon( "locationbar_erase" ) );
	connect(d->simple->m_btnClearShortcut, SIGNAL(clicked()),
	        this, SLOT(slotClearShortcut()));
	connect(d->adv->m_btnClearPrimary, SIGNAL(clicked()),
	        this, SLOT(slotClearPrimary()));
	connect(d->adv->m_btnClearAlternate, SIGNAL(clicked()),
	        this, SLOT(slotClearAlternate()));

	connect(d->adv->m_txtPrimary, SIGNAL(clicked()),
		d->adv->m_btnPrimary, SLOT(animateClick()));
	connect(d->adv->m_txtAlternate, SIGNAL(clicked()),
		d->adv->m_btnAlternate, SLOT(animateClick()));
	connect(d->adv->m_btnPrimary, SIGNAL(clicked()),
		this, SLOT(slotSelectPrimary()));
	connect(d->adv->m_btnAlternate, SIGNAL(clicked()),
		this, SLOT(slotSelectAlternate()));

	connect(this, SIGNAL(buttonClicked(KDialog::ButtonCode)), SLOT(slotButtonClicked(KDialog::ButtonCode)));

	KGuiItem ok = KStdGuiItem::ok();
	ok.setText( i18n( "Ok" ) );
	setButtonGuiItem( Ok , ok );

	KGuiItem cancel = KStdGuiItem::cancel();
	cancel.setText( i18n( "Cancel" ) );
	setButtonGuiItem( Cancel, cancel );

	setShortcut( shortcut );
	resize( 0, 0 );

	s_showMore = KConfigGroup(KGlobal::config(), "General").readEntry("ShowAlternativeShortcutConfig", s_showMore);
	updateDetails();
}

KShortcutDialog::~KShortcutDialog()
{
	setRecording(false);

	KConfigGroup group(KGlobal::config(), "General");
	group.writeEntry("ShowAlternativeShortcutConfig", s_showMore);
	
	delete d;
}

void KShortcutDialog::setShortcut( const KShortcut & shortcut )
{
	d->shortcut = shortcut;
	updateShortcutDisplay();
}

void KShortcutDialog::updateShortcutDisplay()
{
	QString s[2];
	if (d->shortcut.count())
		s[0] = d->shortcut.seq(0).toString();
	if (d->shortcut.count() > 1)
		s[1] = d->shortcut.seq(1).toString();

	if( d->bRecording ) {
		d->ptxtCurrent->setDefault( true );
		d->ptxtCurrent->setFocus();

		// Display modifiers for the first key in the QKeySequence
		if( d->iKey == 0 ) {
			if( d->mod ) {
				QString keyModStr;
#if defined(Q_WS_MAC)
				if( d->mod & Qt::META )  keyModStr += KKeyServer::modToStringUser(Qt::META) + "+";
				if( d->mod & Qt::ALT )   keyModStr += KKeyServer::modToStringUser(Qt::ALT) + "+";
				if( d->mod & Qt::CTRL )  keyModStr += KKeyServer::modToStringUser(Qt::CTRL) + "+";
				if( d->mod & Qt::SHIFT ) keyModStr += KKeyServer::modToStringUser(Qt::SHIFT) + "+";
#elif defined(Q_WS_X11)
				if( d->mod & Qt::META )  keyModStr += KKeyServer::modToStringUser(Qt::META) + "+";
				if( d->mod & Qt::CTRL )  keyModStr += KKeyServer::modToStringUser(Qt::CTRL) + "+";
				if( d->mod & Qt::ALT )   keyModStr += KKeyServer::modToStringUser(Qt::ALT) + "+";
				if( d->mod & Qt::SHIFT ) keyModStr += KKeyServer::modToStringUser(Qt::SHIFT) + "+";
#endif
				s[d->iSeq] = keyModStr;
			}
		}
		// When in the middle of entering multi-key shortcuts,
		//  add a "," to the end of the displayed shortcut.
		else
			s[d->iSeq] += ",";
	}
	else {
		d->adv->m_txtPrimary->setDefault( false );
		d->adv->m_txtAlternate->setDefault( false );
		this->setFocus();
	}

	s[0].replace('&', QLatin1String("&&"));
	s[1].replace('&', QLatin1String("&&"));

	d->simple->m_txtShortcut->setText( s[0] );
	d->adv->m_txtPrimary->setText( s[0] );
	d->adv->m_txtAlternate->setText( s[1] );

	// Determine the enable state of the 'Less' button
	bool bLessOk;
	// If there is no shortcut defined,
	if( d->shortcut.count() == 0 )
		bLessOk = true;
	// If there is a single shortcut defined, and it is not a multi-key shortcut,
	else if( d->shortcut.count() == 1 && d->shortcut.seq(0).count() <= 1 )
		bLessOk = true;
	// Otherwise, we have an alternate shortcut or multi-key shortcut(s).
	else
		bLessOk = false;
	enableButton(Details, bLessOk);
}

void KShortcutDialog::slotButtonClicked(KDialog::ButtonCode code)
{
	if (code == KDialog::Details) {
		s_showMore = (d->stack->currentWidget() != d->advBase);
		updateDetails();
	}
}

void KShortcutDialog::updateDetails()
{
	bool showAdvanced = s_showMore || (d->shortcut.count() > 1);
	setDetails(showAdvanced);
	setRecording(false);
	d->iSeq = 0;
	d->iKey = 0;

	if (showAdvanced)
	{
		d->stack->setCurrentWidget(d->advBase);
		d->adv->m_btnPrimary->setChecked( true );
		slotSelectPrimary();
	}
	else
	{
		d->stack->setCurrentWidget(d->simpleBase);
		d->ptxtCurrent = d->simple->m_txtShortcut;
		d->simple->m_txtShortcut->setDefault( true );
		d->simple->m_txtShortcut->setFocus();
		d->adv->m_btnMultiKey->setChecked( false );
	}
	qApp->processEvents();
	adjustSize();
}

void KShortcutDialog::slotSelectPrimary()
{
	setRecording(false);
	d->iSeq = 0;
	d->iKey = 0;
	d->ptxtCurrent = d->adv->m_txtPrimary;
	d->ptxtCurrent->setDefault(true);
	d->ptxtCurrent->setFocus();
	updateShortcutDisplay();
}

void KShortcutDialog::slotSelectAlternate()
{
	setRecording(false);
	d->iSeq = 1;
	d->iKey = 0;
	d->ptxtCurrent = d->adv->m_txtAlternate;
	d->ptxtCurrent->setDefault(true);
	d->ptxtCurrent->setFocus();
	updateShortcutDisplay();
}

void KShortcutDialog::slotClearShortcut()
{
	d->shortcut.setSeq( 0, QKeySequence() );
	updateShortcutDisplay();
}

void KShortcutDialog::slotClearPrimary()
{
	d->shortcut.setSeq( 0, QKeySequence() );
	d->adv->m_btnPrimary->setChecked( true );
	slotSelectPrimary();
}

void KShortcutDialog::slotClearAlternate()
{
	if( d->shortcut.count() == 2 )
		d->shortcut.init( d->shortcut.seq(0) );
	d->adv->m_btnAlternate->setChecked( true );
	slotSelectAlternate();
}

void KShortcutDialog::slotMultiKeyMode( bool bOn )
{
	// If turning off multi-key mode during a recording,
	if( !bOn && d->bRecording ) {
		setRecording(false);
	d->iKey = 0;
		updateShortcutDisplay();
	}
}

QKeySequence appendToSequence(const QKeySequence& seq, int keyQt)
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

void KShortcutDialog::keyPressEvent( QKeyEvent * e )
{
	KDialog::keyPressEvent(e);

	kDebug() << k_funcinfo << "'" << e->text() << "': " << e->key() << endl;

	//if key is a letter, it must be stored as lowercase
	int keyQt = QChar( e->key() & 0xff ).isLetter() ?
		(QChar( e->key() & 0xff ).toLower().toLatin1() | (e->key() & 0xffff00) )
		: e->key();

	switch( keyQt ) {
		case Qt::Key_Shift:
			d->mod |= Qt::SHIFT;
			setRecording(true);
			break;
		case Qt::Key_Control:
			d->mod |= Qt::CTRL;
			setRecording(true);
			break;
		case Qt::Key_Alt:
			d->mod |= Qt::ALT;
			setRecording(true);
			break;
		case Qt::Key_Meta:
			d->mod |= Qt::META;
			setRecording(true);
			break;
		case Qt::Key_Menu: //unused
			break;
		default:
			if( keyQt == Qt::Key_Return && d->iKey > 0 ) {
				accept();
				return;
			}
			//accept
			if (keyQt) {
				QKeySequence seq;
				if( d->iKey == 0 )
					seq = keyQt | d->mod;
				else
					seq = appendToSequence(d->shortcut.seq( d->iSeq ), keyQt );

				d->shortcut.setSeq( d->iSeq, seq );

				if(d->adv->m_btnMultiKey->isChecked())
					d->iKey++;

				setRecording(true);

				updateShortcutDisplay();

				if( !d->adv->m_btnMultiKey->isChecked() )
					QTimer::singleShot(500, this, SLOT(accept()));
			}
			return;
	}

	// If we are editing the first key in the sequence,
	//  display modifier keys which are held down
	if( d->iKey == 0 ) {
		updateShortcutDisplay();
	}
}

void KShortcutDialog::keyReleaseEvent( QKeyEvent * e )
{
	KDialog::keyReleaseEvent(e);

	bool change = true;
	switch( e->key() ) {
		case Qt::Key_Shift:
			if (d->mod & Qt::SHIFT)
				d->mod ^= Qt::SHIFT;
			break;
		case Qt::Key_Control:
			if (d->mod & Qt::CTRL)
				d->mod ^= Qt::CTRL;
			break;
		case Qt::Key_Alt:
			if (d->mod & Qt::ALT)
				d->mod ^= Qt::ALT;
			break;
		case Qt::Key_Meta:
			if (d->mod & Qt::META)
				d->mod ^= Qt::META;
			break;
		default:
			change = false;
	}
	
	if (change)
		updateShortcutDisplay();
}

const KShortcut & KShortcutDialog::shortcut( ) const
{
  return d->shortcut;
}

void KShortcutDialog::setRecording( bool recording )
{
	if (d->bRecording != recording) {
		d->bRecording = recording;
		if (d->bRecording) {
			grabKeyboard();
		} else {
			releaseKeyboard();
		}
	}
}

#include "kshortcutdialog.moc"
