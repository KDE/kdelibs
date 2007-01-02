// vi: ts=8 sts=4 sw=4
/* This file is part of the KDE libraries
   Copyright (C) 1998 Pietro Iglio <iglio@fub.it>
   Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
   Copyright (C) 2004,2005 Andrew Coles <andrew_coles@yahoo.co.uk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <sys/time.h>
#include <sys/resource.h>

#include <QApplication>
#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QKeyEvent>
#include <QProgressBar>
#include <QRegExp>
#include <QSize>
#include <QString>
#include <QWidget>

#include <Q3PtrDict>

#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <khbox.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <klineedit.h>

#include "kpassworddialog.h"

/*
 * Password line editor.
 */

// BCI: Add a real d-pointer and put the int into that

static Q3PtrDict<int>* d_ptr = 0;

static void cleanup_d_ptr() {
	delete d_ptr;
}

static int * ourMaxLength( const KPasswordEdit* const e ) {
	if ( !d_ptr ) {
		d_ptr = new Q3PtrDict<int>;
		d_ptr->setAutoDelete(true);
		qAddPostRoutine( cleanup_d_ptr );
	}
	int* ret = d_ptr->find( (void*) e );
	if ( ! ret ) {
		ret = new int;
		d_ptr->replace( (void*) e, ret );
	}
	return ret;
}

static void delete_d( const KPasswordEdit* const e ) {
	if ( d_ptr )
		d_ptr->remove( (void*) e );
}

const int KPasswordEdit::PassLen = 200;

class KPasswordDialog::KPasswordDialogPrivate
{
    public:
	KPasswordDialogPrivate()
	 : m_MatchLabel( 0 ), iconName( 0 ), allowEmptyPasswords( false ),
	   minimumPasswordLength(0), maximumPasswordLength(KPasswordEdit::PassLen - 1),
	   passwordStrengthWarningLevel(1), m_strengthBar(0),
	   reasonablePasswordLength(8)
	    {}
	QLabel *m_MatchLabel;
	QString iconName;
	bool allowEmptyPasswords;
	int minimumPasswordLength;
	int maximumPasswordLength;
	int passwordStrengthWarningLevel;
	QProgressBar* m_strengthBar;
	int reasonablePasswordLength;


	bool keep;
	int type, row;
	QLabel *pHelpLbl;
	QGridLayout *pGrid;
	QWidget *pMain;
	KLineEdit *pEdit, *pEdit2;
};


KPasswordEdit::KPasswordEdit(QWidget *parent) : QLineEdit(parent)
{
    init();

    KConfigGroup cg(KGlobal::config(), "Passwords");

    const QString val = cg.readEntry("EchoMode", "OneStar");
    if (val == "ThreeStars")
	m_EchoMode = ThreeStars;
    else if (val == "NoEcho")
	m_EchoMode = NoEcho;
    else
	m_EchoMode = OneStar;

}

KPasswordEdit::KPasswordEdit(EchoModes echoMode, QWidget *parent)
    : QLineEdit(parent), m_EchoMode(echoMode)
{
    init();
}

KPasswordEdit::KPasswordEdit(EchoMode echoMode, QWidget *parent)
    : QLineEdit(parent)
    , m_EchoMode( echoMode == QLineEdit::NoEcho ? NoEcho : OneStar )
{
    init();
}

void KPasswordEdit::init()
{
    setEchoMode(QLineEdit::Password); // Just in case
    setAcceptDrops(false);
    int* t = ourMaxLength(this);
    *t = (PassLen - 1); // the internal max length
    m_Password = new char[PassLen];
    m_Password[0] = '\000';
    m_Length = 0;
}

KPasswordEdit::~KPasswordEdit()
{
    memset(m_Password, 0, PassLen * sizeof(char));
    delete[] m_Password;
    delete_d(this);
}

void KPasswordEdit::insert(const QString &txt)
{
    const QByteArray localTxt = txt.toLocal8Bit();
    const unsigned int lim = localTxt.length();
    const int m_MaxLength = maxPasswordLength();
    for(unsigned int i=0; i < lim; ++i)
    {
        const unsigned char ke = localTxt[i];
        if (m_Length < m_MaxLength)
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
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Escape:
	e->ignore();
	break;
    case Qt::Key_Backspace:
    case Qt::Key_Delete:
    case 0x7f: // Delete
	if (e->modifiers() & (Qt::ControlModifier | Qt::AltModifier))
	    e->ignore();
	else if (m_Length) {
	    m_Password[--m_Length] = '\000';
	    showPass();
	}
	break;
    default:
	const unsigned char ke = e->text().toLocal8Bit()[0];
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
        return true; //Ignore
      case QEvent::InputMethod:
      {
        QInputMethodEvent* const ie = (QInputMethodEvent*) e;
        if (!ie->commitString().isNull())
            insert( ie->commitString() );
        return true;
      }

      case QEvent::ShortcutOverride:
      {
        QKeyEvent* const k = (QKeyEvent*) e;
        switch (k->key()) {
            case Qt::Key_U:
                if (k->modifiers() & Qt::ControlModifier) {
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
	emit textChanged(QString()); //To update the password comparison if need be.
	break;
    }
}

void KPasswordEdit::setMaxPasswordLength(int newLength)
{
    if (newLength >= PassLen) newLength = PassLen - 1; // belt and braces
    if (newLength < 0) newLength = 0;
    int* t = ourMaxLength(this);
    *t = newLength; 
    while (m_Length > newLength) {
        m_Password[m_Length] = '\000';
        --m_Length;
    }
    showPass();
}

int KPasswordEdit::maxPasswordLength() const
{
    return *(ourMaxLength(this));
}
/*
 * Password dialog.
 */

KPasswordDialog::KPasswordDialog(Types type, bool enableKeep, ButtonCodes extraBttn,
                                 QWidget *parent)
    : KDialog(parent, Qt::Dialog), d(new KPasswordDialogPrivate)
{
    d->keep = enableKeep ? 1 : 0;
    d->type = type; 
    setButtons( Ok|Cancel|extraBttn );
    setModal( true );
    showButtonSeparator( true );
    setDefaultButton( Ok );
    d->iconName = "password";
    init();
}

KPasswordDialog::KPasswordDialog(Types type, bool enableKeep, ButtonCodes extraBttn, const QString& icon,
				  QWidget *parent)
    : KDialog(parent, Qt::Dialog), d(new KPasswordDialogPrivate)
{
    d->keep = enableKeep ? 1 : 0;
    d->type = type; 
    setButtons( Ok|Cancel|extraBttn );
    setModal( true );
    showButtonSeparator( true );
    setDefaultButton( Ok );
    if ( icon.trimmed().isEmpty() )
	d->iconName = "password";
    else
	d->iconName = icon;
    init();
}


void KPasswordDialog::init()
{
    d->row = 0;

    KConfigGroup cg(KGlobal::config(), "Passwords");
    if (d->keep && cg.readEntry("Keep", false))
	++d->keep;

    d->pMain = new QWidget(this);
    setMainWidget(d->pMain);
    d->pGrid = new QGridLayout(d->pMain);
    d->pGrid->setMargin(0);
    d->pGrid->setSpacing(0);

    // Row 1: pixmap + prompt
    QLabel *lbl;
    const QPixmap pix( kapp->iconLoader()->loadIcon( d->iconName, K3Icon::NoGroup, K3Icon::SizeHuge, 0, 0, true));
    if (!pix.isNull()) {
	lbl = new QLabel(d->pMain);
	lbl->setPixmap(pix);
	lbl->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
	lbl->setFixedSize(lbl->sizeHint());
	d->pGrid->addWidget(lbl, 0, 0, Qt::AlignCenter);
    }

    d->pHelpLbl = new QLabel(d->pMain);
    d->pHelpLbl->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    d->pHelpLbl->setWordWrap(true);
    d->pGrid->addWidget(d->pHelpLbl, 0, 2, Qt::AlignLeft);
    d->pGrid->setRowStretch(1, 12);

    // Row 2+: space for 4 extra info lines
    d->pGrid->setRowStretch(6, 12);

    // Row 3: Password editor #1
    lbl = new QLabel(d->pMain);
    lbl->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    lbl->setText(i18n("&Password:"));
    lbl->setFixedSize(lbl->sizeHint());
    d->pGrid->addWidget(lbl, 7, 0, Qt::AlignLeft);

    QHBoxLayout *h_lay = new QHBoxLayout();
    d->pGrid->addLayout(h_lay, 7, 2);
    d->pEdit = new KLineEdit(d->pMain);
    d->pEdit->setPasswordMode(true);
    d->pEdit2 = 0;
    lbl->setBuddy(d->pEdit);
    QSize size = d->pEdit->sizeHint();
    d->pEdit->setFixedHeight(size.height());
    d->pEdit->setMinimumWidth(size.width());
    h_lay->addWidget(d->pEdit);

    // Row 4: Password editor #2 or keep password checkbox

    if ((d->type == Password) && d->keep) {
	d->pGrid->setRowStretch(8, 12);
	QCheckBox* const cb = new QCheckBox(i18n("&Keep password"), d->pMain);
	cb->setFixedSize(cb->sizeHint());
	if (d->keep > 1)
	    cb->setChecked(true);
	else
	    d->keep = 0;
	connect(cb, SIGNAL(toggled(bool)), SLOT(slotKeep(bool)));
	d->pGrid->addWidget(cb, 9, 2, Qt::AlignLeft|Qt::AlignVCenter);
    } else if (d->type == NewPassword) {
	lbl = new QLabel(d->pMain);
	lbl->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
	lbl->setText(i18n("&Verify:"));
	lbl->setFixedSize(lbl->sizeHint());
	d->pGrid->addWidget(lbl, 9, 0, Qt::AlignLeft);

	h_lay = new QHBoxLayout();
	d->pGrid->addLayout(h_lay, 9, 2);
	d->pEdit2 = new KLineEdit(d->pMain);
	d->pEdit2->setPasswordMode(true);
	lbl->setBuddy(d->pEdit2);
	size = d->pEdit2->sizeHint();
	d->pEdit2->setFixedHeight(size.height());
	d->pEdit2->setMinimumWidth(size.width());
	h_lay->addWidget(d->pEdit2);

        // Row 6: Password strength meter
        d->pGrid->setRowStretch(10, 12);

        KHBox* const strengthBox = new KHBox(d->pMain);
        strengthBox->setSpacing(10);
        d->pGrid->addWidget(strengthBox, 11, 0, 1, 3);
        QLabel* const passStrengthLabel = new QLabel(strengthBox);
        passStrengthLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
        passStrengthLabel->setText(i18n("Password strength meter:"));
        d->m_strengthBar = new QProgressBar(strengthBox);
        d->m_strengthBar->setObjectName("PasswordStrengthMeter");
        d->m_strengthBar->setRange(0, 100);
        d->m_strengthBar->setTextVisible(false);

        const QString strengthBarWhatsThis(i18n("The password strength meter gives an indication of the security "
                                                "of the password you have entered.  To improve the strength of "
                                                "the password, try:\n"
                                                " - using a longer password;\n"
                                                " - using a mixture of upper- and lower-case letters;\n"
                                                " - using numbers or symbols, such as #, as well as letters."));
        passStrengthLabel->setWhatsThis(strengthBarWhatsThis);
        d->m_strengthBar->setWhatsThis(strengthBarWhatsThis);

        // Row 6: Label saying whether the passwords match
        d->pGrid->setRowStretch(12, 12);

        d->m_MatchLabel = new QLabel(d->pMain);
        d->m_MatchLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
        d->m_MatchLabel->setWordWrap(true);
        d->pGrid->addWidget(d->m_MatchLabel, 13, 0, 1, 3);
        d->m_MatchLabel->setText(i18n("Passwords do not match"));


        connect( d->pEdit, SIGNAL(textChanged(const QString&)), SLOT(enableOkBtn()) );
        connect( d->pEdit2, SIGNAL(textChanged(const QString&)), SLOT(enableOkBtn()) );
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
    d->pEdit->clear();
}

void KPasswordDialog::setPrompt(const QString &prompt)
{
    d->pHelpLbl->setText(prompt);
    d->pHelpLbl->setFixedSize(275, d->pHelpLbl->heightForWidth(275));
}


QString KPasswordDialog::prompt() const

{
    return d->pHelpLbl->text();
}


void KPasswordDialog::addLine(const QString &key, const QString &value)
{
    if (d->row > 3)
	return;

    QLabel *lbl = new QLabel(key, d->pMain);
    lbl->setAlignment(Qt::AlignLeft|Qt::AlignTop);
    lbl->setFixedSize(lbl->sizeHint());
    d->pGrid->addWidget(lbl, d->row+2, 0, Qt::AlignLeft);

    lbl = new QLabel(value, d->pMain);
    lbl->setAlignment(Qt::AlignTop);
    lbl->setWordWrap(true);
    lbl->setFixedSize(275, lbl->heightForWidth(275));
    d->pGrid->addWidget(lbl, d->row+2, 2, Qt::AlignLeft);
    ++d->row;
}


void KPasswordDialog::erase()
{
    d->pEdit->clear();
    d->pEdit->setFocus();
    if (d->type == NewPassword)
	d->pEdit2->clear();
}


void KPasswordDialog::accept()
{
    if (d->type == NewPassword) {
	if (d->pEdit->text() != d->pEdit2->text()) {
	    KMessageBox::sorry(this, i18n("You entered two different "
		    "passwords. Please try again."));
	    erase();
	    return;
	}
	if (d->m_strengthBar && d->m_strengthBar->value() < d->passwordStrengthWarningLevel) {
	    int retVal = KMessageBox::warningContinueCancel(this,
		i18n(   "The password you have entered has a low strength. "
			"To improve the strength of "
			"the password, try:\n"
			" - using a longer password;\n"
			" - using a mixture of upper- and lower-case letters;\n"
			" - using numbers or symbols as well as letters.\n"
			"\n"
			"Would you like to use this password anyway?"),
		i18n("Low Password Strength"));
	    if (retVal == KMessageBox::Cancel) return;
	}
    }
    if (!checkPassword(d->pEdit->text())) {
	erase();
	return;
    }
	KDialog::accept();
}




void KPasswordDialog::slotKeep(bool keep)
{
    d->keep = keep;
}


QString KPasswordDialog::getPassword(const QString &prompt,
    const QString &caption, bool *keep, QWidget *parent)
{
    KPasswordDialog* const dlg = new KPasswordDialog(Password, keep,false,parent);
    if( !caption.isNull() )
        dlg->setWindowTitle(caption);
    dlg->setPrompt(prompt);
    QString password;
    if ( dlg->exec() == Accepted) {
	password = dlg->password();
	if (keep)
	    *keep = dlg->keep();
    }
    delete dlg;
    return password;
}

int KPasswordDialog::getPassword(QWidget *parent, QByteArray &password, const QString &caption,
    const QString &prompt, bool *keep)
{
	QString p= getPassword(prompt,caption , keep, parent);
	password=p.toUtf8();
	return p.isNull() ? Rejected : Accepted;
}


int KPasswordDialog::getPassword(QWidget *parent, QByteArray &password, const QString &prompt,
	bool *keep)
{
	QString p= getPassword(prompt,QString() , keep, parent);
	password=p.toUtf8();
	return p.isNull() ? Rejected : Accepted;
}


QString KPasswordDialog::getNewPassword(const QString &prompt,const QString &caption,QWidget *parent)
{
    KPasswordDialog* const dlg = new KPasswordDialog(NewPassword, false,false,parent);
    dlg->setWindowTitle(caption);
    dlg->setPrompt(prompt);
    const int ret = dlg->exec();
    QString password;
    if (ret == Accepted)
	password = dlg->password();
    delete dlg;
    return password;
}


int KPasswordDialog::getNewPassword(QWidget *parent, QByteArray &password, const QString &caption,
    const QString &prompt)
{
	QString p= getNewPassword(prompt,caption , parent);
	password=p.toUtf8();
	return p.isNull() ? Rejected : Accepted;
}


int KPasswordDialog::getNewPassword(QWidget *parent, QByteArray &password, const QString &prompt)
{
	QString p= getNewPassword(prompt,QString(), parent);
	password=p.toUtf8();
	return p.isNull() ? Rejected : Accepted;
}


// static
void KPasswordDialog::disableCoreDumps()
{
    struct rlimit rlim;
    rlim.rlim_cur = rlim.rlim_max = 0;
    setrlimit(RLIMIT_CORE, &rlim);
}


void KPasswordDialog::enableOkBtn()
{
    if (d->type == NewPassword) {
      const bool match = d->pEdit->text() == d->pEdit2->text();

      const int minPasswordLength = minimumPasswordLength();

      if ( d->pEdit->text().length() < minPasswordLength) {
          enableButtonOk(false);
      } else {
          enableButtonOk( match &&  (d->allowEmptyPasswords || !d->pEdit->text().isEmpty()) );
      }

      if ( match && d->allowEmptyPasswords && d->pEdit->text().isEmpty()) {
          d->m_MatchLabel->setText( i18n("Password is empty") );
      } else {
          if ((int) d->pEdit->text().length() < minPasswordLength) {
              d->m_MatchLabel->setText(i18np("Password must be at least 1 character long", "Password must be at least %n characters long", minPasswordLength));
          } else {
              d->m_MatchLabel->setText( match? i18n("Passwords match")
                                              :i18n("Passwords do not match") );
          }
      }

      // Password strength calculator
      // Based on code in the Master Password dialog in Firefox
      // (pref-masterpass.js)
      // Original code triple-licensed under the MPL, GPL, and LGPL
      // so is license-compatible with this file

      const double lengthFactor = d->reasonablePasswordLength / 8.0;

      
      int pwlength = (int) ( d->pEdit->text().length()/ lengthFactor);
      if (pwlength > 5) pwlength = 5;

      const QRegExp numRxp("[0-9]", Qt::CaseSensitive, QRegExp::RegExp);
      int numeric = (int) (d->pEdit->text().count(numRxp) / lengthFactor);
      if (numeric > 3) numeric = 3;

      const QRegExp symbRxp("\\W", Qt::CaseInsensitive, QRegExp::RegExp);
      int numsymbols = (int) (d->pEdit->text().count(symbRxp) / lengthFactor);
      if (numsymbols > 3) numsymbols = 3;

      const QRegExp upperRxp("[A-Z]", Qt::CaseSensitive, QRegExp::RegExp);
      int upper = (int) (d->pEdit->text().count(upperRxp) / lengthFactor);
      if (upper > 3) upper = 3;

      int pwstrength=((pwlength*10)-20) + (numeric*10) + (numsymbols*15) + (upper*10);

      if ( pwstrength < 0 ) {
	      pwstrength = 0;
      }
  
      if ( pwstrength > 100 ) {
	      pwstrength = 100;
      }
      d->m_strengthBar->setValue(pwstrength);

   }
}


void KPasswordDialog::setAllowEmptyPasswords(bool allowed) {
    d->allowEmptyPasswords = allowed;
    enableOkBtn();
}


bool KPasswordDialog::allowEmptyPasswords() const {
    return d->allowEmptyPasswords;
}

void KPasswordDialog::setMinimumPasswordLength(int minLength) {
    d->minimumPasswordLength = minLength;
    enableOkBtn();
}

int KPasswordDialog::minimumPasswordLength() const {
    return d->minimumPasswordLength;
}

void KPasswordDialog::setMaximumPasswordLength(int maxLength) {

    if (maxLength < 0) maxLength = 0;
    if (maxLength >= KPasswordEdit::PassLen) maxLength = KPasswordEdit::PassLen - 1;

    d->maximumPasswordLength = maxLength;

    d->pEdit->setMaxLength(maxLength);
    if (d->pEdit2) d->pEdit2->setMaxLength(maxLength);

}

int KPasswordDialog::maximumPasswordLength() const {
    return d->maximumPasswordLength;
}

// reasonable password length code contributed by Steffen Mthing

void KPasswordDialog::setReasonablePasswordLength(int reasonableLength) {

    if (reasonableLength < 1) reasonableLength = 1;
    if (reasonableLength >= maximumPasswordLength()) reasonableLength = maximumPasswordLength();

    d->reasonablePasswordLength = reasonableLength;

}

int KPasswordDialog::reasonablePasswordLength() const {
  return d->reasonablePasswordLength;
}


void KPasswordDialog::setPasswordStrengthWarningLevel(int warningLevel) {
    if (warningLevel < 0) warningLevel = 0;
    if (warningLevel > 99) warningLevel = 99;
    d->passwordStrengthWarningLevel = warningLevel;
}

int KPasswordDialog::passwordStrengthWarningLevel() const {
    return d->passwordStrengthWarningLevel;
}

QString KPasswordDialog::password() const {
    return d->pEdit->text();
}

bool KPasswordDialog::keep() const
{
	return d->keep;
}

#include "kpassworddialog.moc"
