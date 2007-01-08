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


#include <QApplication>
#include <QLabel>
#include <QLayout>
#include <QProgressBar>
#include <QRegExp>
#include <QSize>
#include <QString>
#include <QWidget>

#include <kapplication.h>
#include <kglobal.h>
#include <khbox.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <klineedit.h>

#include "knewpassworddialog.h"


class KNewPasswordDialog::KNewPasswordDialogPrivate
{
    public:
        KNewPasswordDialogPrivate() : minimumPasswordLength(0),  passwordStrengthWarningLevel(1),  reasonablePasswordLength(8)  {}

        int minimumPasswordLength;
        int passwordStrengthWarningLevel;
        int reasonablePasswordLength;

        QLabel *promptLabel;
        QLabel *iconLabel;
        KLineEdit *pEdit, *pEdit2;
        QProgressBar* strengthBar;
        QLabel *matchLabel;
        QString pass;
};


/*
 * Password dialog.
 */

KNewPasswordDialog::KNewPasswordDialog( QWidget *parent)
    : KDialog(parent), d(new KNewPasswordDialogPrivate)
{
    init();
}

void KNewPasswordDialog::init()
{
    setButtons( Ok|Cancel );
    showButtonSeparator( true );
    setDefaultButton( Ok );
    

    QWidget *pMain = mainWidget();
    QGridLayout *layout = new QGridLayout(pMain);
    layout->setMargin(0);
    layout->setSpacing(0);

    // Row 1: pixmap + prompt
    d->iconLabel = new QLabel(pMain);
    d->iconLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    layout->addWidget(d->iconLabel, 0, 0, Qt::AlignCenter);

    const QPixmap pix( kapp->iconLoader()->loadIcon( "password", K3Icon::NoGroup, K3Icon::SizeHuge, 0, 0, true));
    d->promptLabel = new QLabel(pMain);
    d->promptLabel->setAlignment(Qt::AlignVCenter);
    d->promptLabel->setWordWrap(true);
//    d->promptLabel->setText( i18n("Enter a password") );
    layout->addWidget(d->promptLabel, 0, 2);
    //layout->setRowStretch(1, 12);

    // Row 2: Password editor #1
    QLabel *lbl = new QLabel(pMain);
    lbl->setText(i18n("&Password:"));
    layout->addWidget(lbl, 2, 0);


    d->pEdit = new KLineEdit(pMain);
    d->pEdit->setPasswordMode(true);
    layout->addWidget(d->pEdit, 2, 2);
    lbl->setBuddy(d->pEdit);    


    // Row 3: Password editor #2 
    lbl = new QLabel(pMain);
    lbl->setText(i18n("&Verify:"));
    layout->addWidget(lbl, 4, 0);

    d->pEdit2 = new KLineEdit(pMain);
    d->pEdit2->setPasswordMode(true);
    layout->addWidget(d->pEdit2, 4, 2);
    lbl->setBuddy(d->pEdit);

    // Row 4: Password strength meter
    //layout->setRowStretch(10, 12);

    KHBox* const strengthBox = new KHBox(pMain);
    strengthBox->setSpacing(10);
    layout->addWidget(strengthBox, 6, 0, 1, 3);
    QLabel* const passStrengthLabel = new QLabel(strengthBox);
    //passStrengthLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    passStrengthLabel->setText(i18n("Password strength meter:"));
    d->strengthBar = new QProgressBar(strengthBox);
    d->strengthBar->setRange(0, 100);
    d->strengthBar->setTextVisible(false);

    const QString strengthBarWhatsThis(i18n("The password strength meter gives an indication of the security "
            "of the password you have entered.  To improve the strength of "
            "the password, try:\n"
            " - using a longer password;\n"
            " - using a mixture of upper- and lower-case letters;\n"
            " - using numbers or symbols, such as #, as well as letters."));
    passStrengthLabel->setWhatsThis(strengthBarWhatsThis);
    d->strengthBar->setWhatsThis(strengthBarWhatsThis);

    // Row 5: Label saying whether the passwords match
    layout->setRowStretch(12, 12);

    d->matchLabel = new QLabel(pMain);
    d->matchLabel->setWordWrap(true);
    layout->addWidget(d->matchLabel, 8, 0, 1, 3);
    d->matchLabel->setText(i18n("Passwords do not match"));

    connect( d->pEdit, SIGNAL(textChanged(const QString&)), SLOT(slotTextChanged()) );
    connect( d->pEdit2, SIGNAL(textChanged(const QString&)), SLOT(slotTextChanged()) );
    slotTextChanged();
}


KNewPasswordDialog::~KNewPasswordDialog()
{
    delete d;
}


void KNewPasswordDialog::setPrompt(const QString &prompt)
{
    d->promptLabel->setText(prompt);
//    d->promptLabel->setFixedSize(275, d->promptLabel->heightForWidth(275));
}


QString KNewPasswordDialog::prompt() const
{
    return d->promptLabel->text();
}


void KNewPasswordDialog::setPixmap(const QPixmap &pixmap)
{
    d->iconLabel->setPixmap(pixmap);
    d->iconLabel->setFixedSize(d->iconLabel->sizeHint());
}


QPixmap KNewPasswordDialog::pixmap() const
{
    return *d->iconLabel->pixmap();
}



void KNewPasswordDialog::accept()
{

    if (d->pEdit->text() != d->pEdit2->text()) {
        KMessageBox::sorry(this, i18n("You entered two different "
                "passwords. Please try again."));
        d->pEdit->clear();
        d->pEdit2->clear();
        return;
    }
    if (d->strengthBar && d->strengthBar->value() < d->passwordStrengthWarningLevel) {
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
    if (!checkPassword(d->pEdit->text())) {
        return;
    }
    d->pass=d->pEdit->text();
    emit newPassword( d->pass );
    KDialog::accept();
}



void KNewPasswordDialog::slotTextChanged()
{
    const bool match = d->pEdit->text() == d->pEdit2->text();

    const int minPasswordLength = minimumPasswordLength();

    if ( d->pEdit->text().length() < minPasswordLength) {
        enableButtonOk(false);
    } else {
        enableButtonOk( match );
    }

    if ( match && !allowEmptyPasswords() && d->pEdit->text().isEmpty()) {
        d->matchLabel->setText( i18n("Password is empty") );
    } else {
        if ( d->pEdit->text().length() < minPasswordLength ) {
            d->matchLabel->setText(i18np("Password must be at least 1 character long", "Password must be at least %n characters long", minPasswordLength));
        } else {
            d->matchLabel->setText( match? i18n("Passwords match")
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
    d->strengthBar->setValue(pwstrength);
}


void KNewPasswordDialog::setAllowEmptyPasswords(bool allowed) {
    setMinimumPasswordLength( allowed ? 0 : 1 );
    slotTextChanged();
}


bool KNewPasswordDialog::allowEmptyPasswords() const {
    return d->minimumPasswordLength == 0;
}

void KNewPasswordDialog::setMinimumPasswordLength(int minLength) {
    d->minimumPasswordLength = minLength;
    slotTextChanged();
}

int KNewPasswordDialog::minimumPasswordLength() const {
    return d->minimumPasswordLength;
}

void KNewPasswordDialog::setMaximumPasswordLength(int maxLength) {
    d->pEdit->setMaxLength(maxLength);
    d->pEdit2->setMaxLength(maxLength);
}

int KNewPasswordDialog::maximumPasswordLength() const {
    return d->pEdit->maxLength();
}

// reasonable password length code contributed by Steffen Mthing

void KNewPasswordDialog::setReasonablePasswordLength(int reasonableLength) {

    if (reasonableLength < 1) reasonableLength = 1;
    if (reasonableLength >= maximumPasswordLength()) reasonableLength = maximumPasswordLength();

    d->reasonablePasswordLength = reasonableLength;

}

int KNewPasswordDialog::reasonablePasswordLength() const {
    return d->reasonablePasswordLength;
}


void KNewPasswordDialog::setPasswordStrengthWarningLevel(int warningLevel) {
    if (warningLevel < 0) warningLevel = 0;
    if (warningLevel > 99) warningLevel = 99;
    d->passwordStrengthWarningLevel = warningLevel;
}

int KNewPasswordDialog::passwordStrengthWarningLevel() const {
    return d->passwordStrengthWarningLevel;
}

QString KNewPasswordDialog::password() const 
{
    return d->pass;
}

bool KNewPasswordDialog::checkPassword(const QString &)
{
    return true;
}
#include "knewpassworddialog.moc"
