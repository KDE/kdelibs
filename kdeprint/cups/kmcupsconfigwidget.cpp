/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kmcupsconfigwidget.h"
#include "cupsinfos.h"

#include <QtGui/QLabel>
#include <QtGui/QGroupBox>
#include <QtGui/QLineEdit>
#include <QtGui/QCheckBox>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>
#include <QtGui/QValidator>

#include <klocale.h>
#include <kcursor.h>
#include <kconfiggroup.h>
#include <kstringhandler.h>

#ifdef __GNUC__
#warning rename class or remove from global namespace
#endif
class PortValidator : public QIntValidator
{
public:
    PortValidator(QWidget *parent, const char *name = 0);
    virtual QValidator::State validate(QString&, int&) const;
};

PortValidator::PortValidator(QWidget *parent, const char *name)
        : QIntValidator(1, 65535, parent)
{
    setObjectName(name);
}

QValidator::State PortValidator::validate(QString& txt, int&) const
{
    bool  ok(false);
    int  p = txt.toInt(&ok);
    if (txt.isEmpty())
        return QValidator::Intermediate;
    else if (ok && p >= bottom() && p <= top())
        return QValidator::Acceptable;
    return QValidator::Invalid;
}

//******************************************************************************************

KMCupsConfigWidget::KMCupsConfigWidget(QWidget *parent)
        : QWidget(parent)
{
    // widget creation
    QGroupBox *m_hostbox = new QGroupBox(i18n("Server Information"), this);
    m_hostbox->setLayout(new QVBoxLayout);
    QGroupBox *m_loginbox = new QGroupBox(i18n("Account Information"), this);
    m_loginbox->setLayout(new QVBoxLayout);
    QLabel *m_hostlabel = new QLabel(i18n("&Host:"), m_hostbox);
    QLabel *m_portlabel = new QLabel(i18n("&Port:"), m_hostbox);
    m_host = new QLineEdit(m_hostbox);
    m_port = new QLineEdit(m_hostbox);
    m_hostlabel->setBuddy(m_host);
    m_portlabel->setBuddy(m_port);
    m_port->setValidator(new PortValidator(m_port));
    m_login = new QLineEdit(m_loginbox);
    QLabel *m_loginlabel = new QLabel(i18n("&User:"), m_loginbox);
    QLabel *m_passwordlabel = new QLabel(i18n("Pass&word:"), m_loginbox);
    m_password = new QLineEdit(m_loginbox);
    m_password->setEchoMode(QLineEdit::Password);
    m_savepwd = new QCheckBox(i18n("&Store password in configuration file"), m_loginbox);
    m_savepwd->setCursor(QCursor(Qt::PointingHandCursor));
    m_anonymous = new QCheckBox(i18n("Use &anonymous access"), m_loginbox);
    m_anonymous->setCursor(QCursor(Qt::PointingHandCursor));
    m_loginlabel->setBuddy(m_login);
    m_passwordlabel->setBuddy(m_password);

    // layout creation
    QVBoxLayout *lay0 = new QVBoxLayout(this);
    lay0->setMargin(0);
    lay0->setSpacing(10);
    lay0->addWidget(m_hostbox, 1);
    lay0->addWidget(m_loginbox, 1);
    QGridLayout *lay2 = new QGridLayout();
    m_hostbox->layout()->addItem(lay2);
    lay2->setMargin(10);
    lay2->setColumnStretch(1, 1);
    lay2->addWidget(m_hostlabel, 0, 0);
    lay2->addWidget(m_portlabel, 1, 0);
    lay2->addWidget(m_host, 0, 1);
    lay2->addWidget(m_port, 1, 1);
    QGridLayout *lay3 = new QGridLayout();
    m_loginbox->layout()->addItem(lay3);
    lay3->setMargin(10);
    lay3->setColumnStretch(1, 1);
    lay3->addWidget(m_loginlabel, 0, 0);
    lay3->addWidget(m_passwordlabel, 1, 0);
    lay3->addWidget(m_login, 0, 1);
    lay3->addWidget(m_password, 1, 1);
    lay3->addWidget(m_savepwd, 2, 0, 1, 2);
    lay3->addWidget(m_anonymous, 3, 0, 1, 2);

    // connections
    connect(m_anonymous, SIGNAL(toggled(bool)), m_login, SLOT(setDisabled(bool)));
    connect(m_anonymous, SIGNAL(toggled(bool)), m_password, SLOT(setDisabled(bool)));
    connect(m_anonymous, SIGNAL(toggled(bool)), m_savepwd, SLOT(setDisabled(bool)));
}

void KMCupsConfigWidget::load()
{
    CupsInfos *inf = CupsInfos::self();
    m_host->setText(inf->host());
    m_port->setText(QString::number(inf->port()));
    if (inf->login().isEmpty())
        m_anonymous->setChecked(true);
    else {
        m_login->setText(inf->login());
        m_password->setText(inf->password());
        m_savepwd->setChecked(inf->savePassword());
    }
}

void KMCupsConfigWidget::save(bool sync)
{
    CupsInfos *inf = CupsInfos::self();
    inf->setHost(m_host->text());
    inf->setPort(m_port->text().toInt());
    if (m_anonymous->isChecked()) {
        inf->setLogin(QString());
        inf->setPassword(QString());
        inf->setSavePassword(false);
    } else {
        inf->setLogin(m_login->text());
        inf->setPassword(m_password->text());
        inf->setSavePassword(m_savepwd->isChecked());
    }
    if (sync) inf->save();
}

void KMCupsConfigWidget::saveConfig(KConfig *conf)
{
    KConfigGroup cg(conf, "CUPS");
    cg.writeEntry("Host", m_host->text());
    cg.writeEntry("Port", m_port->text().toInt());
    cg.writeEntry("Login", (m_anonymous->isChecked() ? QString() : m_login->text()));
    cg.writeEntry("SavePassword", (m_anonymous->isChecked() ? false : m_savepwd->isChecked()));
    if (m_savepwd->isChecked() && !m_anonymous->isChecked())
        cg.writeEntry("Password", (m_anonymous->isChecked() ? QString("") : KStringHandler::obscure(m_password->text())));
    else
        cg.deleteEntry("Password");
    // synchronize CupsInfos object
    save(false);
}
