/* This file is part of the KDE libraries
   Copyright (C) 2004 George Staikos <staikos@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "kwalletwizard.h"
#include "kwalletwizard.moc"

#include "ui_kwalletwizardpageexplanation.h"
#include "ui_kwalletwizardpageintro.h"
#include "ui_kwalletwizardpageoptions.h"
#include "ui_kwalletwizardpagepassword.h"

#include <QtGui/QButtonGroup>

#include <klocale.h>

class PageIntro : public QWizardPage
{
public:
    PageIntro(QWidget *parent)
        : QWizardPage(parent)
    {
        ui.setupUi(this);

        bg = new QButtonGroup(this);
        bg->setExclusive(true);
        bg->addButton(ui._basic, 0);
        bg->addButton(ui._advanced, 1);

        // force the "basic" button to be selected
        ui._basic->setChecked(true);
    }

    QButtonGroup *bg;

private:
    Ui::KWalletWizardPageIntro ui;
};


class PagePassword : public QWizardPage
{
public:
    PagePassword(QWidget *parent)
        : QWizardPage(parent)
    {
        ui.setupUi(this);

        registerField("useWallet", ui._useWallet);
        registerField("pass1", ui._pass1);
        registerField("pass2", ui._pass2);

        connect(ui._useWallet, SIGNAL(clicked()), parent, SLOT(passwordPageUpdate()));
        connect(ui._pass1, SIGNAL(textChanged(QString)), parent, SLOT(passwordPageUpdate()));
        connect(ui._pass2, SIGNAL(textChanged(QString)), parent, SLOT(passwordPageUpdate()));
    }

    virtual int nextId() const
    {
        return static_cast<KWalletWizard*>(wizard())->wizardType() == KWalletWizard::Basic ? -1 : KWalletWizard::PageOptionsId;
    }

    void setMatchLabelText(const QString &text)
    {
        ui._matchLabel->setText(text);
    }

private:
    Ui::KWalletWizardPagePassword ui;
};


class PageOptions : public QWizardPage
{
public:
    PageOptions(QWidget *parent)
        : QWizardPage(parent)
    {
        ui.setupUi(this);

        registerField("closeWhenIdle", ui._closeIdle);
        registerField("networkWallet", ui._networkWallet);
    }

private:
    Ui::KWalletWizardPageOptions ui;
};


class PageExplanation : public QWizardPage
{
public:
    PageExplanation(QWidget *parent)
        : QWizardPage(parent)
    {
        ui.setupUi(this);
        setFinalPage(true);
    }

private:
    Ui::KWalletWizardPageExplanation ui;
};



KWalletWizard::KWalletWizard( QWidget *parent )
    : QWizard(parent)
{
    setOption(HaveFinishButtonOnEarlyPages);

    m_pageIntro = new PageIntro(this);
    setPage(PageIntroId, m_pageIntro);
    m_pagePasswd = new PagePassword(this);
    setPage(PagePasswordId, m_pagePasswd);
    setPage(PageOptionsId, new PageOptions(this));
    setPage(PageExplanationId, new PageExplanation(this));
}

void KWalletWizard::passwordPageUpdate()
{
    bool complete = true;
    if (field("useWallet").toBool()) {
        if (field("pass1").toString() == field("pass2").toString()) {
            if (field("pass1").toString().isEmpty()) {
                m_pagePasswd->setMatchLabelText(i18n("<qt>Password is empty.  <b>(WARNING: Insecure)</qt>"));
            } else {
                m_pagePasswd->setMatchLabelText(i18n("Passwords match."));
            }
        } else {
            m_pagePasswd->setMatchLabelText(i18n("Passwords do not match."));
            complete = false;
        }
    } else {
        m_pagePasswd->setMatchLabelText(QString());
    }
    button(wizardType() == Basic ? FinishButton : NextButton)->setEnabled(complete);
}

KWalletWizard::WizardType KWalletWizard::wizardType() const
{
    return (KWalletWizard::WizardType)m_pageIntro->bg->checkedId();
}

void KWalletWizard::initializePage(int id)
{
    switch (id) {
    case PagePasswordId:
    {
        bool islast = m_pageIntro->bg->checkedId() == 0;
        m_pagePasswd->setFinalPage(islast);
        button(NextButton)->setVisible(!islast);
        break;
    }
    }
}

