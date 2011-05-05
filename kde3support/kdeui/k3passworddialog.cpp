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

#include "k3passworddialog.h"

#include <sys/time.h>
#include <sys/resource.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QRegExp>
#include <QtCore/QSize>
#include <QtCore/QString>
#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QKeyEvent>
#include <QtGui/QProgressBar>
#include <QtGui/QWidget>

#include <Q3PtrDict>

#include <kconfig.h>
#include <kglobal.h>
#include <khbox.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <kconfiggroup.h>

/*
 * Password line editor.
 */

// BCI: Add a real d-pointer and put the int into that

static Q3PtrDict<int>* d_ptr = 0;

static void cleanup_d_ptr() {
	delete d_ptr;
}

static int * ourMaxLength( const K3PasswordEdit* const e ) {
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

static void delete_d( const K3PasswordEdit* const e ) {
	if ( d_ptr )
		d_ptr->remove( (void*) e );
}

const int K3PasswordEdit::PassLen = 200;

class K3PasswordDialog::K3PasswordDialogPrivate
{
    public:
	K3PasswordDialogPrivate()
	 : m_MatchLabel( 0 ), allowEmptyPasswords( false ),
	   minimumPasswordLength(0), maximumPasswordLength(K3PasswordEdit::PassLen - 1),
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
};


K3PasswordEdit::K3PasswordEdit(QWidget *parent) : QLineEdit(parent)
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

K3PasswordEdit::K3PasswordEdit(EchoModes echoMode, QWidget *parent)
    : QLineEdit(parent), m_EchoMode(echoMode)
{
    init();
}

K3PasswordEdit::K3PasswordEdit(EchoMode echoMode, QWidget *parent)
    : QLineEdit(parent)
    , m_EchoMode( echoMode == QLineEdit::NoEcho ? NoEcho : OneStar )
{
    init();
}

void K3PasswordEdit::init()
{
    setEchoMode(QLineEdit::Password); // Just in case
    setAcceptDrops(false);
    int* t = ourMaxLength(this);
    *t = (PassLen - 1); // the internal max length
    m_Password = new char[PassLen];
    m_Password[0] = '\000';
    m_Length = 0;
}

K3PasswordEdit::~K3PasswordEdit()
{
    memset(m_Password, 0, PassLen * sizeof(char));
    delete[] m_Password;
    delete_d(this);
}

const char *K3PasswordEdit::password() const
{
    return m_Password;
}

void K3PasswordEdit::insert(const QString &txt)
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

void K3PasswordEdit::erase()
{
    m_Length = 0;
    memset(m_Password, 0, PassLen * sizeof(char));
    setText("");
}

void K3PasswordEdit::focusInEvent(QFocusEvent *e)
{
    const QString txt = text();
    setUpdatesEnabled(false);
    QLineEdit::focusInEvent(e);
    setUpdatesEnabled(true);
    setText(txt);
}


void K3PasswordEdit::keyPressEvent(QKeyEvent *e)
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

bool K3PasswordEdit::event(QEvent *e) {
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

void K3PasswordEdit::showPass()
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

void K3PasswordEdit::setMaxPasswordLength(int newLength)
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

int K3PasswordEdit::maxPasswordLength() const
{
    return *(ourMaxLength(this));
}
/*
 * Password dialog.
 */

K3PasswordDialog::K3PasswordDialog(Types type, bool enableKeep, ButtonCodes extraBttn,
                                 QWidget *parent)
    : KDialog(parent, Qt::Dialog)
      , m_Keep(enableKeep? 1 : 0), m_Type(type), d(new K3PasswordDialogPrivate)
{
    setButtons( Ok|Cancel|extraBttn );
    setModal( true );
    setDefaultButton( Ok );
    d->iconName = "password";
    init();
}

K3PasswordDialog::K3PasswordDialog(Types type, bool enableKeep, ButtonCodes extraBttn, const QString& icon,
				  QWidget *parent)
    : KDialog(parent, Qt::Dialog)
      , m_Keep(enableKeep? 1 : 0), m_Type(type), d(new K3PasswordDialogPrivate)
{
    setButtons( Ok|Cancel|extraBttn );
    setModal( true );
    setDefaultButton( Ok );
    if ( icon.trimmed().isEmpty() )
	d->iconName = "password";
    else
	d->iconName = icon;
    init();
}


void K3PasswordDialog::init()
{
    m_Row = 0;

    KConfigGroup cg(KGlobal::config(), "Passwords");
    if (m_Keep && cg.readEntry("Keep", false))
	++m_Keep;

    m_pMain = new QWidget(this);
    setMainWidget(m_pMain);
    m_pGrid = new QGridLayout(m_pMain);
    m_pGrid->setMargin(0);
    m_pGrid->setSpacing(0);

    // Row 1: pixmap + prompt
    QLabel *lbl;
    const QPixmap pix( KIconLoader::global()->loadIcon( d->iconName, KIconLoader::NoGroup, KIconLoader::SizeHuge, 0, QStringList(), 0, true));
    if (!pix.isNull()) {
	lbl = new QLabel(m_pMain);
	lbl->setPixmap(pix);
	lbl->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
	lbl->setFixedSize(lbl->sizeHint());
	m_pGrid->addWidget(lbl, 0, 0, Qt::AlignCenter);
    }

    m_pHelpLbl = new QLabel(m_pMain);
    m_pHelpLbl->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    m_pHelpLbl->setWordWrap(true);
    m_pGrid->addWidget(m_pHelpLbl, 0, 2, Qt::AlignLeft);
    m_pGrid->setRowStretch(1, 12);

    // Row 2+: space for 4 extra info lines
    m_pGrid->setRowStretch(6, 12);

    // Row 3: Password editor #1
    lbl = new QLabel(m_pMain);
    lbl->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    lbl->setText(i18n("&Password:"));
    lbl->setFixedSize(lbl->sizeHint());
    m_pGrid->addWidget(lbl, 7, 0, Qt::AlignLeft);

    QHBoxLayout *h_lay = new QHBoxLayout();
    m_pGrid->addLayout(h_lay, 7, 2);
    m_pEdit = new K3PasswordEdit(m_pMain);
    m_pEdit2 = 0;
    lbl->setBuddy(m_pEdit);
    QSize size = m_pEdit->sizeHint();
    m_pEdit->setFixedHeight(size.height());
    m_pEdit->setMinimumWidth(size.width());
    h_lay->addWidget(m_pEdit);

    // Row 4: Password editor #2 or keep password checkbox

    if ((m_Type == Password) && m_Keep) {
	m_pGrid->setRowStretch(8, 12);
	QCheckBox* const cb = new QCheckBox(i18n("&Keep password"), m_pMain);
	cb->setFixedSize(cb->sizeHint());
	if (m_Keep > 1)
	    cb->setChecked(true);
	else
	    m_Keep = 0;
	connect(cb, SIGNAL(toggled(bool)), SLOT(slotKeep(bool)));
	m_pGrid->addWidget(cb, 9, 2, Qt::AlignLeft|Qt::AlignVCenter);
    } else if (m_Type == NewPassword) {
	lbl = new QLabel(m_pMain);
	lbl->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
	lbl->setText(i18n("&Verify:"));
	lbl->setFixedSize(lbl->sizeHint());
	m_pGrid->addWidget(lbl, 9, 0, Qt::AlignLeft);

	h_lay = new QHBoxLayout();
	m_pGrid->addLayout(h_lay, 9, 2);
	m_pEdit2 = new K3PasswordEdit(m_pMain);
	lbl->setBuddy(m_pEdit2);
	size = m_pEdit2->sizeHint();
	m_pEdit2->setFixedHeight(size.height());
	m_pEdit2->setMinimumWidth(size.width());
	h_lay->addWidget(m_pEdit2);

        // Row 6: Password strength meter
        m_pGrid->setRowStretch(10, 12);

        KHBox* const strengthBox = new KHBox(m_pMain);
        strengthBox->setSpacing(10);
        m_pGrid->addWidget(strengthBox, 11, 0, 1, 3);
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
        m_pGrid->setRowStretch(12, 12);

        d->m_MatchLabel = new QLabel(m_pMain);
        d->m_MatchLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
        d->m_MatchLabel->setWordWrap(true);
        m_pGrid->addWidget(d->m_MatchLabel, 13, 0, 1, 3);
        d->m_MatchLabel->setText(i18n("Passwords do not match"));


        connect( m_pEdit, SIGNAL(textChanged(const QString&)), SLOT(enableOkBtn()) );
        connect( m_pEdit2, SIGNAL(textChanged(const QString&)), SLOT(enableOkBtn()) );
        enableOkBtn();
    }

    erase();
}


K3PasswordDialog::~K3PasswordDialog()
{
	delete d;
}


void K3PasswordDialog::clearPassword()
{
    m_pEdit->erase();
}

void K3PasswordDialog::setPrompt(const QString &prompt)
{
    m_pHelpLbl->setText(prompt);
    m_pHelpLbl->setFixedSize(275, m_pHelpLbl->heightForWidth(275));
}


QString K3PasswordDialog::prompt() const

{
    return m_pHelpLbl->text();
}


void K3PasswordDialog::addLine(const QString &key, const QString &value)
{
    if (m_Row > 3)
	return;

    QLabel *lbl = new QLabel(key, m_pMain);
    lbl->setAlignment(Qt::AlignLeft|Qt::AlignTop);
    lbl->setFixedSize(lbl->sizeHint());
    m_pGrid->addWidget(lbl, m_Row+2, 0, Qt::AlignLeft);

    lbl = new QLabel(value, m_pMain);
    lbl->setAlignment(Qt::AlignTop);
    lbl->setWordWrap(true);
    lbl->setFixedSize(275, lbl->heightForWidth(275));
    m_pGrid->addWidget(lbl, m_Row+2, 2, Qt::AlignLeft);
    ++m_Row;
}


void K3PasswordDialog::erase()
{
    m_pEdit->erase();
    m_pEdit->setFocus();
    if (m_Type == NewPassword)
	m_pEdit2->erase();
}


void K3PasswordDialog::accept()
{
    if (m_Type == NewPassword) {
	if (strcmp(m_pEdit->password(), m_pEdit2->password())) {
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
    if (!checkPassword(m_pEdit->password())) {
	erase();
	return;
    }
	KDialog::accept();
}




void K3PasswordDialog::slotKeep(bool keep)
{
    m_Keep = keep;
}

bool K3PasswordDialog::checkPassword(const char *)
{
    return true;
}


int K3PasswordDialog::getPassword(QWidget *parent, QByteArray &password, const QString &caption,
    const QString &prompt, bool *keep)
{
    const bool enableKeep = (keep && *keep);
    K3PasswordDialog* const dlg = new K3PasswordDialog(Password, enableKeep,KDialog::None,parent);
    dlg->setWindowTitle(caption);
    dlg->setPrompt(prompt);
    const int ret = dlg->exec();
    if (ret == Accepted) {
	password = dlg->password();
	if (enableKeep)
	    *keep = dlg->keep();
    }
    delete dlg;
    return ret;
}

int K3PasswordDialog::getPassword(QWidget *parent, QByteArray &password, const QString &prompt,
	int *keep)
{
    int res = K3PasswordDialog::Rejected;
    if (keep) {
        bool boolkeep = *keep;
        res = getPassword(parent, password, i18n("Password Input"), prompt, &boolkeep);
        *keep = boolkeep;
    }
    else {
        res = getPassword(parent, password, i18n("Password Input"), prompt);
    }
    return res;
}


int K3PasswordDialog::getNewPassword(QWidget *parent, QByteArray &password, const QString &caption,
    const QString &prompt)
{
    K3PasswordDialog* const dlg = new K3PasswordDialog(NewPassword, false,KDialog::None,parent);
    dlg->setWindowTitle(caption);
    dlg->setPrompt(prompt);
    const int ret = dlg->exec();
    if (ret == Accepted)
	password = dlg->password();
    delete dlg;
    return ret;
}

int K3PasswordDialog::getNewPassword(QWidget *parent, QByteArray &password, const QString &prompt)
{
    return getNewPassword(parent, password, i18n("Password Input"), prompt);
}


// static
void K3PasswordDialog::disableCoreDumps()
{
    struct rlimit rlim;
    rlim.rlim_cur = rlim.rlim_max = 0;
    setrlimit(RLIMIT_CORE, &rlim);
}


void K3PasswordDialog::enableOkBtn()
{
    if (m_Type == NewPassword) {
      const bool match = strcmp(m_pEdit->password(), m_pEdit2->password()) == 0
                   && (d->allowEmptyPasswords || m_pEdit->password()[0]);

      const QString pass(m_pEdit->password());

      const int minPasswordLength = minimumPasswordLength();

      if ((int) pass.length() < minPasswordLength) {
          enableButtonOk(false);
      } else {
          enableButtonOk( match );
      }

      if ( match && d->allowEmptyPasswords && m_pEdit->password()[0] == 0 ) {
          d->m_MatchLabel->setText( i18n("Password is empty") );
      } else {
          if ((int) pass.length() < minPasswordLength) {
              d->m_MatchLabel->setText(i18np("Password must be at least 1 character long", "Password must be at least %1 characters long", minPasswordLength));
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

      
      int pwlength = (int) (pass.length() / lengthFactor);
      if (pwlength > 5) pwlength = 5;

      const QRegExp numRxp("[0-9]", Qt::CaseSensitive, QRegExp::RegExp);
      int numeric = (int) (pass.count(numRxp) / lengthFactor);
      if (numeric > 3) numeric = 3;

      const QRegExp symbRxp("\\W", Qt::CaseInsensitive, QRegExp::RegExp);
      int numsymbols = (int) (pass.count(symbRxp) / lengthFactor);
      if (numsymbols > 3) numsymbols = 3;

      const QRegExp upperRxp("[A-Z]", Qt::CaseSensitive, QRegExp::RegExp);
      int upper = (int) (pass.count(upperRxp) / lengthFactor);
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


void K3PasswordDialog::setAllowEmptyPasswords(bool allowed) {
    d->allowEmptyPasswords = allowed;
    enableOkBtn();
}


bool K3PasswordDialog::allowEmptyPasswords() const {
    return d->allowEmptyPasswords;
}

void K3PasswordDialog::setMinimumPasswordLength(int minLength) {
    d->minimumPasswordLength = minLength;
    enableOkBtn();
}

int K3PasswordDialog::minimumPasswordLength() const {
    return d->minimumPasswordLength;
}

void K3PasswordDialog::setMaximumPasswordLength(int maxLength) {

    if (maxLength < 0) maxLength = 0;
    if (maxLength >= K3PasswordEdit::PassLen) maxLength = K3PasswordEdit::PassLen - 1;

    d->maximumPasswordLength = maxLength;

    m_pEdit->setMaxPasswordLength(maxLength);
    if (m_pEdit2) m_pEdit2->setMaxPasswordLength(maxLength);

}

int K3PasswordDialog::maximumPasswordLength() const {
    return d->maximumPasswordLength;
}

// reasonable password length code contributed by Steffen Mthing

void K3PasswordDialog::setReasonablePasswordLength(int reasonableLength) {

    if (reasonableLength < 1) reasonableLength = 1;
    if (reasonableLength >= maximumPasswordLength()) reasonableLength = maximumPasswordLength();

    d->reasonablePasswordLength = reasonableLength;

}

int K3PasswordDialog::reasonablePasswordLength() const {
  return d->reasonablePasswordLength;
}


void K3PasswordDialog::setPasswordStrengthWarningLevel(int warningLevel) {
    if (warningLevel < 0) warningLevel = 0;
    if (warningLevel > 99) warningLevel = 99;
    d->passwordStrengthWarningLevel = warningLevel;
}

int K3PasswordDialog::passwordStrengthWarningLevel() const {
    return d->passwordStrengthWarningLevel;
}

const char *K3PasswordDialog::password() const
{
    return m_pEdit->password();
}

bool K3PasswordDialog::keep() const
{
    return m_Keep;
}

#include "k3passworddialog.moc"
