/* This file is part of the KDE libraries
    Copyright (C) 2002,2003 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
    Copyright (C) 2007 Roberto Raggi <roberto@kdevelop.org>

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

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kguiitem.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kshortcut.h>
#include <kstandardguiitem.h>
#include <kpushbutton.h>

#include "kkeyserver.h"
#include <kconfiggroup.h>

class KShortcutDialogPrivate
{
public:
	KShortcutDialogPrivate(KShortcutDialog *q): q(q) {}

	void updateShortcutDisplay();
	void updateDetails();
	void setRecording(bool recording);
        
	static QKeySequence appendToSequence(const QKeySequence& seq, int keyQt);

	void slotButtonClicked(KDialog::ButtonCode code);
	void slotSelectPrimary();
	void slotSelectAlternate();
	void slotClearShortcut();
	void slotClearPrimary();
	void slotClearAlternate();
	void slotMultiKeyMode( bool bOn );
          
	KShortcutDialog *q;
	KShortcut shortcut;
	bool bGrab;
	KPushButton* ptxtCurrent;
	uint iSeq;
	uint iKey;
	bool bRecording;
	uint mod;
	QWidget* simpleBase;
	QWidget* advBase;
	Ui::KShortcutDialogSimple simple;
	Ui::KShortcutDialogAdvanced adv;
	QStackedWidget *stack;
        
	static bool s_showMore;
};

bool KShortcutDialogPrivate::s_showMore = false;

KShortcutDialog::KShortcutDialog( const KShortcut& shortcut, QWidget* parent )
	: KDialog( parent )
	, d(new KShortcutDialogPrivate(this))
{
	setCaption( i18n("Configure Shortcut") );
	setButtons( KDialog::Details|KDialog::Ok|KDialog::Cancel );
	showButtonSeparator( true );
	setModal(true);

	setButtonText(Details, i18n("Advanced"));
	d->stack = new QStackedWidget(this);
	d->stack->setMinimumWidth(360);
	setMainWidget(d->stack);

	d->simpleBase = new QWidget(d->stack);
	d->simple.setupUi(d->simpleBase);
	d->stack->addWidget(d->simpleBase);

	d->advBase = new QWidget(d->stack);
	d->adv.setupUi(d->advBase);
	d->stack->addWidget(d->advBase);

	d->stack->setCurrentWidget(d->simpleBase);

	d->bGrab = false;
	d->iSeq = 0;
	d->iKey = 0;
	d->ptxtCurrent = 0;
	d->bRecording = false;
	d->mod = 0;

	d->simple.m_btnClearShortcut->setIcon( SmallIcon( "locationbar_erase" ) );
	d->adv.m_btnClearPrimary->setIcon( SmallIcon( "locationbar_erase" ) );
	d->adv.m_btnClearAlternate->setIcon( SmallIcon( "locationbar_erase" ) );
	connect(d->simple.m_btnClearShortcut, SIGNAL(clicked()),
	        this, SLOT(slotClearShortcut()));
	connect(d->adv.m_btnClearPrimary, SIGNAL(clicked()),
	        this, SLOT(slotClearPrimary()));
	connect(d->adv.m_btnClearAlternate, SIGNAL(clicked()),
	        this, SLOT(slotClearAlternate()));

	connect(d->adv.m_txtPrimary, SIGNAL(clicked()),
		d->adv.m_btnPrimary, SLOT(animateClick()));
	connect(d->adv.m_txtAlternate, SIGNAL(clicked()),
		d->adv.m_btnAlternate, SLOT(animateClick()));
	connect(d->adv.m_btnPrimary, SIGNAL(clicked()),
		this, SLOT(slotSelectPrimary()));
	connect(d->adv.m_btnAlternate, SIGNAL(clicked()),
		this, SLOT(slotSelectAlternate()));

	connect(this, SIGNAL(buttonClicked(KDialog::ButtonCode)), SLOT(slotButtonClicked(KDialog::ButtonCode)));

	KGuiItem ok = KStandardGuiItem::ok();
	ok.setText( i18n( "Ok" ) );
	setButtonGuiItem( Ok , ok );

	KGuiItem cancel = KStandardGuiItem::cancel();
	cancel.setText( i18n( "Cancel" ) );
	setButtonGuiItem( Cancel, cancel );

	setShortcut( shortcut );
	resize( 0, 0 );

        KShortcutDialogPrivate::s_showMore = KConfigGroup(KGlobal::config(), "General")
              .readEntry("ShowAlternativeShortcutConfig", KShortcutDialogPrivate::s_showMore);
	d->updateDetails();
}

KShortcutDialog::~KShortcutDialog()
{
	d->setRecording(false);

	KConfigGroup group(KGlobal::config(), "General");
	group.writeEntry("ShowAlternativeShortcutConfig", KShortcutDialogPrivate::s_showMore);
	
	delete d;
}

void KShortcutDialog::setShortcut( const KShortcut & shortcut )
{
	d->shortcut = shortcut;
	d->updateShortcutDisplay();
}

void KShortcutDialogPrivate::updateShortcutDisplay()
{
	QString s[2];
	s[0] = shortcut.primary().toString();
	s[1] = shortcut.alternate().toString();

	if( bRecording ) {
		ptxtCurrent->setDefault( true );
		ptxtCurrent->setFocus();

		// Display modifiers for the first key in the QKeySequence
		if( iKey == 0 ) {
			if( mod ) {
				QString keyModStr;
#if defined(Q_WS_MAC)
				if( mod & Qt::META )  keyModStr += KKeyServer::modToStringUser(Qt::META) + '+';
				if( mod & Qt::ALT )   keyModStr += KKeyServer::modToStringUser(Qt::ALT) + '+';
				if( mod & Qt::CTRL )  keyModStr += KKeyServer::modToStringUser(Qt::CTRL) + '+';
				if( mod & Qt::SHIFT ) keyModStr += KKeyServer::modToStringUser(Qt::SHIFT) + '+';
#elif defined(Q_WS_X11)
				if( mod & Qt::META )  keyModStr += KKeyServer::modToStringUser(Qt::META) + '+';
				if( mod & Qt::CTRL )  keyModStr += KKeyServer::modToStringUser(Qt::CTRL) + '+';
				if( mod & Qt::ALT )   keyModStr += KKeyServer::modToStringUser(Qt::ALT) + '+';
				if( mod & Qt::SHIFT ) keyModStr += KKeyServer::modToStringUser(Qt::SHIFT) + '+';
#endif
				s[iSeq] = keyModStr;
			}
		}
		// When in the middle of entering multi-key shortcuts,
		//  add a "," to the end of the displayed shortcut.
		else
			s[iSeq] += ',';
	}
	else {
		adv.m_txtPrimary->setDefault( false );
		adv.m_txtAlternate->setDefault( false );
		q->setFocus();
	}

	s[0].replace('&', QLatin1String("&&"));
	s[1].replace('&', QLatin1String("&&"));

	simple.m_txtShortcut->setText( s[0] );
	adv.m_txtPrimary->setText( s[0] );
	adv.m_txtAlternate->setText( s[1] );

	// Determine the enable state of the 'Less' button
	bool btnLessEn;
	// If there is no shortcut defined,
	if( shortcut.isEmpty() )
		btnLessEn = true;
	// If there is a single shortcut defined, and it is not a multi-key shortcut,
	else if( shortcut.alternate().isEmpty() && shortcut.primary().count() <= 1 )
		btnLessEn = true;
	// Otherwise, we have an alternate shortcut or multi-key shortcut(s).
	else
		btnLessEn = false;
	q->enableButton(KDialog::Details, btnLessEn);
}

void KShortcutDialogPrivate::slotButtonClicked(KDialog::ButtonCode code)
{
	if (code == KDialog::Details) {
		KShortcutDialogPrivate::s_showMore = (stack->currentWidget() != advBase);
		updateDetails();
	}
}

void KShortcutDialogPrivate::updateDetails()
{
	bool showAdvanced = s_showMore || (!shortcut.alternate().isEmpty());
	q->setDetailsWidgetVisible(showAdvanced);
	setRecording(false);
	iSeq = 0;
	iKey = 0;

	if (showAdvanced)
	{
		stack->setCurrentWidget(advBase);
		adv.m_btnPrimary->setChecked( true );
		slotSelectPrimary();
	}
	else
	{
		stack->setCurrentWidget(simpleBase);
		ptxtCurrent = simple.m_txtShortcut;
		simple.m_txtShortcut->setDefault( true );
		simple.m_txtShortcut->setFocus();
		adv.m_btnMultiKey->setChecked( false );
	}
	qApp->processEvents();
	q->adjustSize();
}

void KShortcutDialogPrivate::slotSelectPrimary()
{
	setRecording(false);
	iSeq = 0;
	iKey = 0;
	ptxtCurrent = adv.m_txtPrimary;
	ptxtCurrent->setDefault(true);
	ptxtCurrent->setFocus();
	updateShortcutDisplay();
}

void KShortcutDialogPrivate::slotSelectAlternate()
{
	setRecording(false);
	iSeq = 1;
	iKey = 0;
	ptxtCurrent = adv.m_txtAlternate;
	ptxtCurrent->setDefault(true);
	ptxtCurrent->setFocus();
	updateShortcutDisplay();
}

void KShortcutDialogPrivate::slotClearShortcut()
{
	shortcut.clear();
	updateShortcutDisplay();
}

void KShortcutDialogPrivate::slotClearPrimary()
{
	shortcut.setPrimary(QKeySequence());
	adv.m_btnPrimary->setChecked( true );
	slotSelectPrimary();
}

void KShortcutDialogPrivate::slotClearAlternate()
{
	shortcut.setAlternate(QKeySequence());
	adv.m_btnAlternate->setChecked( true );
	slotSelectAlternate();
}

void KShortcutDialogPrivate::slotMultiKeyMode( bool bOn )
{
	// If turning off multi-key mode during a recording,
	if( !bOn && bRecording ) {
		setRecording(false);
		iKey = 0;
		updateShortcutDisplay();
	}
}

QKeySequence KShortcutDialogPrivate::appendToSequence(const QKeySequence& seq, int keyQt)
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

	//if key is a letter, it must be stored as lowercase
	int keyQt = QChar( e->key() & 0xff ).isLetter() ?
		(QChar( e->key() & 0xff ).toLower().toLatin1() | (e->key() & 0xffff00) )
		: e->key();

	switch( keyQt ) {
		case Qt::Key_Shift:
			d->mod |= Qt::SHIFT;
			d->setRecording(true);
			break;
		case Qt::Key_Control:
			d->mod |= Qt::CTRL;
			d->setRecording(true);
			break;
		case Qt::Key_Alt:
			d->mod |= Qt::ALT;
			d->setRecording(true);
			break;
		case Qt::Key_Meta:
			d->mod |= Qt::META;
			d->setRecording(true);
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
				int keyQtMod = keyQt;
				if( d->iKey == 0 )
					keyQtMod |= d->mod;

				if (d->iSeq == 0) {
					QKeySequence seq = KShortcutDialogPrivate::appendToSequence(d->shortcut.primary(), keyQtMod);
					d->shortcut.setPrimary(seq);
				} else if (d->iSeq == 1) {
					QKeySequence seq = KShortcutDialogPrivate::appendToSequence(d->shortcut.alternate(), keyQtMod);
					d->shortcut.setAlternate(seq);
				}

				if(d->adv.m_btnMultiKey->isChecked())
					d->iKey++;

				d->setRecording(true);

				d->updateShortcutDisplay();

				if( !d->adv.m_btnMultiKey->isChecked() )
					QTimer::singleShot(500, this, SLOT(accept()));
			}
			return;
	}

	// If we are editing the first key in the sequence,
	//  display modifier keys which are held down
	if( d->iKey == 0 ) {
		d->updateShortcutDisplay();
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
		d->updateShortcutDisplay();
}

const KShortcut & KShortcutDialog::shortcut( ) const
{
	return d->shortcut;
}

void KShortcutDialogPrivate::setRecording( bool recording )
{
	if (bRecording != recording) {
		bRecording = recording;
		if (bRecording) {
			q->grabKeyboard();
		} else {
			q->releaseKeyboard();
		}
	}
}

#include "kshortcutdialog.moc"
