/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdeui.
 * Copyright (C) 1998 Pietro Iglio <iglio@fub.it>
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 * 
 * kpassdlg.cpp: Password input dialog.
 */

#include <config.h>
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

KPasswordEdit::KPasswordEdit(QWidget *parent, const char *name)
    : QLineEdit(parent, name), m_EchoMode(OneStar)
{
    m_Password = new char[PassLen];
    m_Password[0] = '\000';
    m_Length = 0;

    KConfig *cfg = KGlobal::config();
    KConfigGroupSaver(cfg, "Passwords");

    QString val = cfg->readEntry("EchoMode", "OneStar");
    if (val == "ThreeStars")
	m_EchoMode = ThreeStars;
    else if (val == "NoEcho")
	m_EchoMode = NoEcho;
    else
	m_EchoMode = OneStar;
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


void KPasswordEdit::keyPressEvent(QKeyEvent *e)
{    
    switch (e->key()) {
    case Key_Return:
    case Key_Escape:
	e->ignore();
	break;
    case Key_Backspace:
	if (m_Length) {
	    m_Password[--m_Length] = '\000';
	    showPass();
	}
	break;
    default:
	if ((m_Length < PassLen) && e->ascii()) {
	    m_Password[m_Length] = (char) e->ascii();
	    m_Password[++m_Length] = '\000';
	    showPass();
	}
	break;
    }
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

KPasswordDialog::KPasswordDialog(int type, QString prompt, QString command, 
	bool enableKeep, int extraBttn)
    : KDialogBase(0L, "Password Dialog", true, "", Ok|Cancel|extraBttn,
	    Ok, true)
{
    m_Command = command;
    m_Keep = enableKeep ? 1 : 0;
    m_Type = type;

    KConfig *cfg = KGlobal::config();
    KConfigGroupSaver(cfg, "Passwords");
    if (m_Keep && cfg->readBoolEntry("Keep", false))
	m_Keep++;
    
    QWidget *main = new QWidget(this);
    setMainWidget(main);
    QGridLayout *grid = new QGridLayout(main, 4, 3, 10, 10);

    // pixmap + prompt
    QLabel *lbl;
    QPixmap pix(locate("data", QString::fromLatin1("kdeui/pics/keys.png")));
    if (!pix.isNull()) {
	lbl = new QLabel(main);
	lbl->setPixmap(pix);
	lbl->setFixedSize(lbl->sizeHint());
	grid->addWidget(lbl, 0, 0, AlignCenter);
    }

    m_pHelpLbl = new QLabel(prompt, main);
    m_pHelpLbl->setAlignment(AlignLeft|AlignVCenter|WordBreak);
    grid->addWidget(m_pHelpLbl, 0, 2, AlignLeft);
    setPrompt(prompt);

    QHBoxLayout *h_lay;
    QSize size;

    // Row 2: Command or password
    if ((m_Type == Password) && !m_Command.isEmpty()) {
	lbl = new QLabel(main);
	lbl->setText(i18n("Command:"));
	lbl->setFixedSize(lbl->sizeHint());
	grid->addWidget(lbl, 1, 0, AlignLeft);

	lbl = new QLabel(main);
	lbl->setAlignment(AlignLeft|AlignVCenter|WordBreak);
	lbl->setText(m_Command);
	lbl->setFixedSize(275, lbl->heightForWidth(275));
	grid->addWidget(lbl, 1, 2, AlignLeft);
    } else if (m_Type == newPassword) {
	lbl = new QLabel(main);
	lbl->setAlignment(AlignLeft|AlignVCenter);
	lbl->setText(i18n("&Password:"));
	lbl->setFixedSize(lbl->sizeHint());
	grid->addWidget(lbl, 1, 0, AlignLeft);

	QHBoxLayout *h_lay = new QHBoxLayout();
	grid->addLayout(h_lay, 1, 2);
	m_pEdit2 = new KPasswordEdit(main);
	lbl->setBuddy(m_pEdit2);
	size = m_pEdit2->sizeHint();
	m_pEdit2->setFixedHeight(size.height());
	m_pEdit2->setMinimumWidth(size.width());
	h_lay->addWidget(m_pEdit2, 12);
	h_lay->addStretch(6);
    }
    
    // Row 3: Password or verify
    lbl = new QLabel(main);
    lbl->setAlignment(AlignLeft|AlignVCenter);
    if (m_Type == Password)
	lbl->setText(i18n("&Password:"));
    else
	lbl->setText(i18n("&Verify:"));
    lbl->setFixedSize(lbl->sizeHint());
    grid->addWidget(lbl, 2, 0, AlignLeft);
    
    h_lay = new QHBoxLayout();
    grid->addLayout(h_lay, 2, 2);
    m_pEdit1 = new KPasswordEdit(main);
    lbl->setBuddy(m_pEdit1);
    size = m_pEdit1->sizeHint();
    m_pEdit1->setFixedHeight(size.height());
    m_pEdit1->setMinimumWidth(size.width());
    h_lay->addWidget(m_pEdit1, 12);
    h_lay->addStretch(6);

    // Row 4: Keep password checkbox
    if (m_Keep) {
	QCheckBox *cb = new QCheckBox(i18n("&Keep Password"), main);
	cb->setFixedSize(cb->sizeHint());
	if (m_Keep > 1)
	    cb->setChecked(true);
	else
	    m_Keep = 0;
	connect(cb, SIGNAL(toggled(bool)), SLOT(slotKeep(bool)));
	grid->addMultiCellWidget(cb, 3, 3, 0, 2, AlignLeft|AlignVCenter);
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


void KPasswordDialog::erase()
{
    if (m_Type == newPassword) {
	m_pEdit1->erase();
	m_pEdit2->erase();
	m_pEdit2->setFocus();
    } else {
	m_pEdit1->erase();
	m_pEdit1->setFocus();
    }
}


void KPasswordDialog::slotOk()
{
    if (m_Type == newPassword) {
	if (strcmp(m_pEdit1->password(), m_pEdit2->password())) {
	    KMessageBox::sorry(this, i18n("You entered two different "
		    "passwords. Please try again."));
	    erase(); 
	    return;
	}
    }
    if (!checkPassword(m_pEdit1->password())) {
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
	QString command, int *keep)
{
    bool enableKeep = keep && *keep;
    KPasswordDialog *dlg = new KPasswordDialog(Password, prompt, 
	    command, enableKeep);
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
    KPasswordDialog *dlg = new KPasswordDialog(newPassword, prompt);
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
