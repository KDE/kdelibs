// vi: ts=8 sts=4 sw=4
/* This file is part of the KDE libraries
   Copyright (C) 1998 Pietro Iglio <iglio@fub.it>
   Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>

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
#include <unistd.h>

#include <qwidget.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qsize.h>
#include <qevent.h>
#include <qkeycode.h>
#include <qcheckbox.h>
#include <qregexp.h>
#include <qhbox.h>
#include <qwhatsthis.h>

#include <kglobal.h>
#include <kdebug.h>
#include <kapplication.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kaboutdialog.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kprogress.h>

#include <sys/time.h>
#include <sys/resource.h>

#include "kpassdlg.h"

/*
 * Password line editor.
 */

class KPasswordDialog::KPasswordDialogPrivate
{
    public:
	KPasswordDialogPrivate()
	    : m_MatchLabel( 0 ), iconName( 0 ), allowEmptyPasswords( false )
	    {}
	QLabel *m_MatchLabel;
	QString iconName;
	bool allowEmptyPasswords;
	KProgress* m_strengthBar;
};

const int KPasswordEdit::PassLen = 200;

KPasswordEdit::KPasswordEdit(QWidget *parent, const char *name)
    : QLineEdit(parent, name)
{
    init();

    KConfig* const cfg = KGlobal::config();
    KConfigGroupSaver saver(cfg, "Passwords");

    const QString val = cfg->readEntry("EchoMode", "OneStar");
    if (val == "ThreeStars")
	m_EchoMode = ThreeStars;
    else if (val == "NoEcho")
	m_EchoMode = NoEcho;
    else
	m_EchoMode = OneStar;
}

KPasswordEdit::KPasswordEdit(QWidget *parent, const char *name, int echoMode)
    : QLineEdit(parent, name), m_EchoMode(echoMode)
{
    init();
}

KPasswordEdit::KPasswordEdit(EchoModes echoMode, QWidget *parent, const char *name)
    : QLineEdit(parent, name), m_EchoMode(echoMode)
{
    init();
}

KPasswordEdit::KPasswordEdit(EchoMode echoMode, QWidget *parent, const char *name)
    : QLineEdit(parent, name)
    , m_EchoMode( echoMode == QLineEdit::NoEcho ? NoEcho : OneStar )
{
    init();
}

void KPasswordEdit::init()
{
    setEchoMode(QLineEdit::Password); // Just in case
    setAcceptDrops(false);
    m_Password = new char[PassLen];
    m_Password[0] = '\000';
    m_Length = 0;
}

KPasswordEdit::~KPasswordEdit()
{
    memset(m_Password, 0, PassLen * sizeof(char));
    delete[] m_Password;
}

void KPasswordEdit::insert(const QString &txt)
{
    const QCString localTxt = txt.local8Bit();
    const unsigned int lim = localTxt.length();
    for(unsigned int i=0; i < lim; ++i)
    {
        const unsigned char ke = localTxt[i];
        if (m_Length < (PassLen - 1))
        {
            m_Password[m_Length] = ke;
            m_Password[++m_Length] = '\000';
        }
    }
    showPass();
}

void KPasswordEdit::erase()
{
    m_Length = 0;
    memset(m_Password, 0, PassLen * sizeof(char));
    setText("");
}

void KPasswordEdit::focusInEvent(QFocusEvent *e)
{
    const QString txt = text();
    setUpdatesEnabled(false);
    QLineEdit::focusInEvent(e);
    setUpdatesEnabled(true);
    setText(txt);
}


void KPasswordEdit::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Key_Return:
    case Key_Enter:
    case Key_Escape:
	e->ignore();
	break;
    case Key_Backspace:
    case Key_Delete:
    case 0x7f: // Delete
	if (e->state() & (ControlButton | AltButton))
	    e->ignore();
	else if (m_Length) {
	    m_Password[--m_Length] = '\000';
	    showPass();
	}
	break;
    default:
	const unsigned char ke = e->text().local8Bit()[0];
	if (ke >= 32) {
	    insert(e->text());
	} else
	    e->ignore();
	break;
    }
}

bool KPasswordEdit::event(QEvent *e) {
    switch(e->type()) {

      case QEvent::MouseButtonPress:
      case QEvent::MouseButtonRelease:
      case QEvent::MouseButtonDblClick:
      case QEvent::MouseMove:
      case QEvent::IMStart:
      case QEvent::IMCompose:
        return true; //Ignore

      case QEvent::IMEnd:
      {
        QIMEvent* const ie = (QIMEvent*) e;
        insert( ie->text() );
        return true;
      }

      case QEvent::AccelOverride:
      {
        QKeyEvent* const k = (QKeyEvent*) e;
        switch (k->key()) {
            case Key_U:
                if (k->state() & ControlButton) {
                    m_Length = 0;
                    m_Password[m_Length] = '\000';
                    showPass();
                }
        }
        return true; // stop bubbling
      }

      default:
        // Do nothing
        break;
    }
    return QLineEdit::event(e);
}

void KPasswordEdit::showPass()
{
    QString tmp;

    switch (m_EchoMode) {
    case OneStar:
	tmp.fill('*', m_Length);
	setText(tmp);
	break;
    case ThreeStars:
	tmp.fill('*', m_Length*3);
	setText(tmp);
	break;
    case NoEcho: default:
	emit textChanged(QString::null); //To update the password comparison if need be.
	break;
    }
}


/*
 * Password dialog.
 */

KPasswordDialog::KPasswordDialog(Types type, bool enableKeep, int extraBttn,
                                 QWidget *parent, const char *name)
    : KDialogBase(parent, name, true, "", Ok|Cancel|extraBttn,
                  Ok, true), m_Keep(enableKeep? 1 : 0), m_Type(type), d(new KPasswordDialogPrivate)
{
    d->iconName = "password";
    init();
}

KPasswordDialog::KPasswordDialog(Types type, bool enableKeep, int extraBttn, const QString& icon,
				  QWidget *parent, const char *name )
    : KDialogBase(parent, name, true, "", Ok|Cancel|extraBttn,
                  Ok, true), m_Keep(enableKeep? 1 : 0), m_Type(type), d(new KPasswordDialogPrivate)
{
    if ( icon.stripWhiteSpace().isEmpty() )
	d->iconName = "password";
    else
	d->iconName = icon;
    init();
}

KPasswordDialog::KPasswordDialog(int type, QString prompt, bool enableKeep,
                                 int extraBttn)
    : KDialogBase(0L, "Password Dialog", true, "", Ok|Cancel|extraBttn,
                  Ok, true), m_Keep(enableKeep? 1 : 0), m_Type(type), d(new KPasswordDialogPrivate)
{
    d->iconName = "password";
    init();
    setPrompt(prompt);
}

void KPasswordDialog::init()
{
    m_Row = 0;

    KConfig* const cfg = KGlobal::config();
    const KConfigGroupSaver saver(cfg, "Passwords");
    if (m_Keep && cfg->readBoolEntry("Keep", false))
	++m_Keep;

    m_pMain = new QWidget(this);
    setMainWidget(m_pMain);
    m_pGrid = new QGridLayout(m_pMain, 10, 3, 0, 0);
    m_pGrid->addColSpacing(1, 10);

    // Row 1: pixmap + prompt
    QLabel *lbl;
    const QPixmap pix( KGlobal::iconLoader()->loadIcon( d->iconName, KIcon::NoGroup, KIcon::SizeHuge, 0, 0, true));
    if (!pix.isNull()) {
	lbl = new QLabel(m_pMain);
	lbl->setPixmap(pix);
	lbl->setAlignment(AlignHCenter|AlignVCenter);
	lbl->setFixedSize(lbl->sizeHint());
	m_pGrid->addWidget(lbl, 0, 0, AlignCenter);
    }

    m_pHelpLbl = new QLabel(m_pMain);
    m_pHelpLbl->setAlignment(AlignLeft|AlignVCenter|WordBreak);
    m_pGrid->addWidget(m_pHelpLbl, 0, 2, AlignLeft);
    m_pGrid->addRowSpacing(1, 10);
    m_pGrid->setRowStretch(1, 12);

    // Row 2+: space for 4 extra info lines
    m_pGrid->addRowSpacing(6, 5);
    m_pGrid->setRowStretch(6, 12);

    // Row 3: Password editor #1
    lbl = new QLabel(m_pMain);
    lbl->setAlignment(AlignLeft|AlignVCenter);
    lbl->setText(i18n("&Password:"));
    lbl->setFixedSize(lbl->sizeHint());
    m_pGrid->addWidget(lbl, 7, 0, AlignLeft);

    QHBoxLayout *h_lay = new QHBoxLayout();
    m_pGrid->addLayout(h_lay, 7, 2);
    m_pEdit = new KPasswordEdit(m_pMain);
    lbl->setBuddy(m_pEdit);
    QSize size = m_pEdit->sizeHint();
    m_pEdit->setFixedHeight(size.height());
    m_pEdit->setMinimumWidth(size.width());
    h_lay->addWidget(m_pEdit);

    // Row 4: Password editor #2 or keep password checkbox

    if ((m_Type == Password) && m_Keep) {
	m_pGrid->addRowSpacing(8, 10);
	m_pGrid->setRowStretch(8, 12);
	QCheckBox* const cb = new QCheckBox(i18n("&Keep password"), m_pMain);
	cb->setFixedSize(cb->sizeHint());
	if (m_Keep > 1)
	    cb->setChecked(true);
	else
	    m_Keep = 0;
	connect(cb, SIGNAL(toggled(bool)), SLOT(slotKeep(bool)));
	m_pGrid->addWidget(cb, 9, 2, AlignLeft|AlignVCenter);
    } else if (m_Type == NewPassword) {
	m_pGrid->addRowSpacing(8, 10);
	lbl = new QLabel(m_pMain);
	lbl->setAlignment(AlignLeft|AlignVCenter);
	lbl->setText(i18n("&Verify:"));
	lbl->setFixedSize(lbl->sizeHint());
	m_pGrid->addWidget(lbl, 9, 0, AlignLeft);

	h_lay = new QHBoxLayout();
	m_pGrid->addLayout(h_lay, 9, 2);
	m_pEdit2 = new KPasswordEdit(m_pMain);
	lbl->setBuddy(m_pEdit2);
	size = m_pEdit2->sizeHint();
	m_pEdit2->setFixedHeight(size.height());
	m_pEdit2->setMinimumWidth(size.width());
	h_lay->addWidget(m_pEdit2);

        // Row 6: Password strength meter
        m_pGrid->addRowSpacing(10, 10);
        m_pGrid->setRowStretch(10, 12);

        QHBox* const strengthBox = new QHBox(m_pMain);
        strengthBox->setSpacing(10);
        m_pGrid->addMultiCellWidget(strengthBox, 11, 11, 0, 2);
        QLabel* const passStrengthLabel = new QLabel(strengthBox);
        passStrengthLabel->setAlignment(AlignLeft|AlignVCenter);
        passStrengthLabel->setText(i18n("Password strength meter:"));
        d->m_strengthBar = new KProgress(100, strengthBox, "PasswordStrengthMeter");
        d->m_strengthBar->setPercentageVisible(false);

        const QString strengthBarWhatsThis(i18n("The password strength bar gives an indication of the security "
                                                "of the password you have entered.  To improve the strength of "
                                                "the password, try:\n"
                                                " - using a longer password;\n"
                                                " - using a mixture of upper- and lower-case letters;\n"
                                                " - using numbers or symbols, such as #, as well as letters."));
        QWhatsThis::add(passStrengthLabel, strengthBarWhatsThis);
        QWhatsThis::add(d->m_strengthBar, strengthBarWhatsThis);

        // Row 6: Label saying whether the passwords match
        m_pGrid->addRowSpacing(12, 10);
        m_pGrid->setRowStretch(12, 12);

        d->m_MatchLabel = new QLabel(m_pMain);
        d->m_MatchLabel->setAlignment(AlignLeft|AlignVCenter|WordBreak);
        m_pGrid->addMultiCellWidget(d->m_MatchLabel, 13, 13, 0, 2);
        d->m_MatchLabel->setText(i18n("Passwords do not match"));


        connect( m_pEdit, SIGNAL(textChanged(const QString&)), SLOT(enableOkBtn()) );
        connect( m_pEdit2, SIGNAL(textChanged(const QString&)), SLOT(enableOkBtn()) );
        enableOkBtn();
    }

    erase();
}


KPasswordDialog::~KPasswordDialog()
{
	delete d;
}


void KPasswordDialog::clearPassword()
{
    m_pEdit->erase();
}

/* KDE 4: Make it const QString & */
void KPasswordDialog::setPrompt(QString prompt)
{
    m_pHelpLbl->setText(prompt);
    m_pHelpLbl->setFixedSize(275, m_pHelpLbl->heightForWidth(275));
}


QString KPasswordDialog::prompt() const

{
    return m_pHelpLbl->text();
}


/* KDE 4: Make them const QString & */
void KPasswordDialog::addLine(QString key, QString value)
{
    if (m_Row > 3)
	return;

    QLabel *lbl = new QLabel(key, m_pMain);
    lbl->setAlignment(AlignLeft|AlignTop);
    lbl->setFixedSize(lbl->sizeHint());
    m_pGrid->addWidget(lbl, m_Row+2, 0, AlignLeft);

    lbl = new QLabel(value, m_pMain);
    lbl->setAlignment(AlignTop|WordBreak);
    lbl->setFixedSize(275, lbl->heightForWidth(275));
    m_pGrid->addWidget(lbl, m_Row+2, 2, AlignLeft);
    ++m_Row;
}


void KPasswordDialog::erase()
{
    m_pEdit->erase();
    m_pEdit->setFocus();
    if (m_Type == NewPassword)
	m_pEdit2->erase();
}


void KPasswordDialog::slotOk()
{
    if (m_Type == NewPassword) {
	if (strcmp(m_pEdit->password(), m_pEdit2->password())) {
	    KMessageBox::sorry(this, i18n("You entered two different "
		    "passwords. Please try again."));
	    erase();
	    return;
	}
    }
    if (!checkPassword(m_pEdit->password())) {
	erase();
	return;
    }
    accept();
}


void KPasswordDialog::slotCancel()
{
    reject();
}


void KPasswordDialog::slotKeep(bool keep)
{
    m_Keep = keep;
}


// static . antlarr: KDE 4: Make it const QString & prompt
int KPasswordDialog::getPassword(QCString &password, QString prompt,
	int *keep)
{
    const bool enableKeep = (keep && *keep);
    KPasswordDialog* const dlg = new KPasswordDialog(int(Password), prompt, enableKeep);
    const int ret = dlg->exec();
    if (ret == Accepted) {
	password = dlg->password();
	if (enableKeep)
	    *keep = dlg->keep();
    }
    delete dlg;
    return ret;
}


// static . antlarr: KDE 4: Make it const QString & prompt
int KPasswordDialog::getNewPassword(QCString &password, QString prompt)
{
    KPasswordDialog* const dlg = new KPasswordDialog(NewPassword, prompt);
    const int ret = dlg->exec();
    if (ret == Accepted)
	password = dlg->password();
    delete dlg;
    return ret;
}


// static
void KPasswordDialog::disableCoreDumps()
{
    struct rlimit rlim;
    rlim.rlim_cur = rlim.rlim_max = 0;
    setrlimit(RLIMIT_CORE, &rlim);
}

void KPasswordDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

void KPasswordDialog::enableOkBtn()
{
    if (m_Type == NewPassword) {
      const bool match = strcmp(m_pEdit->password(), m_pEdit2->password()) == 0
                   && (d->allowEmptyPasswords || m_pEdit->password()[0]);

      enableButtonOK( match );
      if ( match && d->allowEmptyPasswords && m_pEdit->password()[0] == 0 ) {
          d->m_MatchLabel->setText( i18n("Password is empty") );
      } else {
          d->m_MatchLabel->setText( match? i18n("Passwords match")
                                          :i18n("Passwords do not match") );
      }

      // Password strength calculator
      // Based on code in the Master Password dialog in Firefox
      // (pref-masterpass.js)
      // Original code triple-licensed under the MPL, GPL, and LGPL
      // so is license-compatible with this file

      const QString pass(m_pEdit->password());
      int pwlength = pass.length();
      if (pwlength > 5) pwlength = 5;

      const QRegExp numRxp("[0-9]", true, false);
      int numeric = pass.contains(numRxp);
      if (numeric > 3) numeric = 3;

      const QRegExp symbRxp("\\W", false, false);
      int numsymbols = pass.contains(symbRxp);
      if (numsymbols > 3) numsymbols = 3;

      const QRegExp upperRxp("[A-Z]", true, false);
      int upper = pass.contains(upperRxp);
      if (upper > 3) upper = 3;

      int pwstrength=((pwlength*10)-20) + (numeric*10) + (numsymbols*15) + (upper*10);

      if ( pwstrength < 0 ) {
	      pwstrength = 0;
      }
  
      if ( pwstrength > 100 ) {
	      pwstrength = 100;
      }
      d->m_strengthBar->setProgress(pwstrength);

   }
}


void KPasswordDialog::setAllowEmptyPasswords(bool allowed) {
    d->allowEmptyPasswords = allowed;
    enableOkBtn();
}


bool KPasswordDialog::allowEmptyPasswords() const {
    return d->allowEmptyPasswords;
}

#include "kpassdlg.moc"
