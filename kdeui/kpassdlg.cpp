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

#include <kglobal.h>
#include <kapp.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kaboutdialog.h>
#include <kconfig.h>
#include <kstddirs.h>

#include <sys/time.h>
#include <sys/resource.h>

#include "kpassdlg.h"

/*
 * Password line editor.
 */

const int KPasswordEdit::PassLen = 100;

KPasswordEdit::KPasswordEdit(QWidget *parent, const char *name)
    : QLineEdit(parent, name), m_EchoMode(OneStar)
{
    setAcceptDrops(false);
    m_Password = new char[PassLen];
    m_Password[0] = '\000';
    m_Length = 0;

    KConfig *cfg = KGlobal::config();
    KConfigGroupSaver saver(cfg, "Passwords");

    QString val = cfg->readEntry("EchoMode", "OneStar");
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
    m_Password = new char[PassLen];
    m_Password[0] = '\000';
    m_Length = 0;
}

KPasswordEdit::~KPasswordEdit()
{
    for (int i=0; i<PassLen; i++)
	m_Password[i] = '\000';
    delete[] m_Password;
}


void KPasswordEdit::erase()
{
    m_Length = 0;
    for (int i=0; i<PassLen; i++)
	m_Password[i] = '\000';
    setText("");
}

void KPasswordEdit::mousePressEvent(QMouseEvent *)
{
    // Do nothing.
}

void KPasswordEdit::mouseMoveEvent(QMouseEvent *)
{
    // Do nothing.
}

void KPasswordEdit::mouseReleaseEvent(QMouseEvent *)
{
    // Do nothing.
}

void KPasswordEdit::focusInEvent(QFocusEvent *e)
{
    QString txt = text();
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
	unsigned char ke = e->text().local8Bit()[0];
	if (ke >= 32) {
	    if (m_Length < (PassLen - 1)) {
		m_Password[m_Length] = ke;
		m_Password[++m_Length] = '\000';
		showPass();
	    }
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
        return TRUE; //Ignore

      case QEvent::AccelOverride:
      {
        QKeyEvent *k = (QKeyEvent*) e;
        switch (k->key()) {
            case Key_U:
                if (k->state() & ControlButton) {
                    m_Length = 0;
                    m_Password[m_Length] = '\000';
                    showPass();
                }
        }
        return TRUE; // stop bubbling
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
	break;
    }
}


/*
 * Password dialog.
 */

KPasswordDialog::KPasswordDialog(int type, QString prompt, bool enableKeep,
	int extraBttn)
    : KDialogBase(0L, "Password Dialog", true, "", Ok|Cancel|extraBttn,
	    Ok, true)
{
    m_Keep = enableKeep ? 1 : 0;
    m_Type = type;
    m_Row = 0;

    KConfig *cfg = KGlobal::config();
    KConfigGroupSaver saver(cfg, "Passwords");
    if (m_Keep && cfg->readBoolEntry("Keep", false))
	m_Keep++;

    m_pMain = new QWidget(this);
    setMainWidget(m_pMain);
    m_pGrid = new QGridLayout(m_pMain, 10, 3, 10, 0);
    m_pGrid->addColSpacing(1, 10);

    // Row 1: pixmap + prompt
    QLabel *lbl;
    QPixmap pix(locate("data", QString::fromLatin1("kdeui/pics/keys.png")));
    if (!pix.isNull()) {
	lbl = new QLabel(m_pMain);
	lbl->setPixmap(pix);
	lbl->setAlignment(AlignLeft|AlignVCenter);
	lbl->setFixedSize(lbl->sizeHint());
	m_pGrid->addWidget(lbl, 0, 0, AlignLeft);
    }

    m_pHelpLbl = new QLabel(m_pMain);
    m_pHelpLbl->setAlignment(AlignLeft|AlignVCenter|WordBreak);
    m_pGrid->addWidget(m_pHelpLbl, 0, 2, AlignLeft);
    setPrompt(prompt);
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
    h_lay->addWidget(m_pEdit, 12);
    h_lay->addStretch(4);

    // Row 4: Password editor #2 or keep password checkbox

    if ((m_Type == Password) && m_Keep) {
	m_pGrid->addRowSpacing(8, 10);
	m_pGrid->setRowStretch(8, 12);
	QCheckBox *cb = new QCheckBox(i18n("&Keep Password"), m_pMain);
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
	h_lay->addWidget(m_pEdit2, 12);
	h_lay->addStretch(4);
    }

    erase();
}


KPasswordDialog::~KPasswordDialog()
{
}


void KPasswordDialog::setPrompt(QString prompt)
{
    m_pHelpLbl->setText(prompt);
    m_pHelpLbl->setFixedSize(275, m_pHelpLbl->heightForWidth(275));
}


void KPasswordDialog::addLine(QString key, QString value)
{
    if (m_Row > 3)
	return;

    QLabel *lbl = new QLabel(key, m_pMain);
    lbl->setAlignment(AlignTop);
    lbl->setIndent(5);
    lbl->setFixedSize(lbl->sizeHint());
    m_pGrid->addWidget(lbl, m_Row+2, 0, AlignLeft);

    lbl = new QLabel(value, m_pMain);
    lbl->setAlignment(AlignTop|WordBreak);
    lbl->setIndent(5);
    lbl->setFixedSize(275, lbl->heightForWidth(275));
    m_pGrid->addWidget(lbl, m_Row+2, 2, AlignLeft);
    m_Row++;
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


// static
int KPasswordDialog::getPassword(QCString &password, QString prompt,
	int *keep)
{
    bool enableKeep = keep && *keep;
    KPasswordDialog *dlg = new KPasswordDialog(Password, prompt, enableKeep);
    int ret = dlg->exec();
    if (ret == Accepted) {
	password = dlg->password();
	if (enableKeep)
	    *keep = dlg->keep();
    }
    delete dlg;
    return ret;
}


// static
int KPasswordDialog::getNewPassword(QCString &password, QString prompt)
{
    KPasswordDialog *dlg = new KPasswordDialog(NewPassword, prompt);
    int ret = dlg->exec();
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

#include "kpassdlg.moc"
