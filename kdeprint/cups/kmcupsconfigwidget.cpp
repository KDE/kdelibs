/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  $Id$
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "kmcupsconfigwidget.h"
#include "cupsinfos.h"

#include <qlabel.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qvalidator.h>

#include <klocale.h>
#include <kcursor.h>
#include <kconfig.h>

class PortValidator : public QIntValidator
{
public:
	PortValidator(QWidget *parent, const char *name = 0);
	virtual QValidator::State validate(QString&, int&) const;
};

PortValidator::PortValidator(QWidget *parent, const char *name)
: QIntValidator(1, 9999, parent, name)
{
}

QValidator::State PortValidator::validate(QString& txt, int&) const
{
	bool 	ok(false);
	int 	p = txt.toInt(&ok);
	if (txt.isEmpty())
		return QValidator::Intermediate;
	else if (ok && p >= bottom() && p <= top())
		return QValidator::Acceptable;
	return QValidator::Invalid;
}

//******************************************************************************************

KMCupsConfigWidget::KMCupsConfigWidget(QWidget *parent, const char *name)
: QWidget(parent,name)
{
	// widget creation
	QGroupBox	*m_hostbox = new QGroupBox(0, Qt::Vertical, i18n("Server information"), this);
	QGroupBox	*m_loginbox = new QGroupBox(0, Qt::Vertical, i18n("Account information"), this);
	QLabel	*m_hostlabel = new QLabel(i18n("&Host:"), m_hostbox);
	QLabel	*m_portlabel = new QLabel(i18n("&Port:"), m_hostbox);
	m_host = new QLineEdit(m_hostbox);
	m_port = new QLineEdit(m_hostbox);
	m_hostlabel->setBuddy(m_host);
	m_portlabel->setBuddy(m_port);
	m_port->setValidator(new PortValidator(m_port));
	m_login = new QLineEdit(m_loginbox);
	QLabel	*m_loginlabel = new QLabel(i18n("&User:"), m_loginbox);
	QLabel	*m_passwordlabel = new QLabel(i18n("Pass&word:"), m_loginbox);
	m_password = new QLineEdit(m_loginbox);
	m_password->setEchoMode(QLineEdit::Password);
	m_anonymous = new QCheckBox(i18n("Use &anonymous access"), m_loginbox);
	m_anonymous->setCursor(KCursor::handCursor());
	m_loginlabel->setBuddy(m_login);
	m_passwordlabel->setBuddy(m_password);

	// layout creation
	QVBoxLayout	*lay0 = new QVBoxLayout(this, 0, 10);
	lay0->addWidget(m_hostbox,1);
	lay0->addWidget(m_loginbox,1);
	QGridLayout	*lay2 = new QGridLayout(m_hostbox->layout(), 2, 2, 10);
	lay2->setColStretch(1,1);
	lay2->addWidget(m_hostlabel,0,0);
	lay2->addWidget(m_portlabel,1,0);
	lay2->addWidget(m_host,0,1);
	lay2->addWidget(m_port,1,1);
	QGridLayout	*lay3 = new QGridLayout(m_loginbox->layout(), 3, 2, 10);
	lay3->setColStretch(1,1);
	lay3->addWidget(m_loginlabel,0,0);
	lay3->addWidget(m_passwordlabel,1,0);
	lay3->addWidget(m_login,0,1);
	lay3->addWidget(m_password,1,1);
	lay3->addMultiCellWidget(m_anonymous,2,2,0,1);

	// connections
	connect(m_anonymous,SIGNAL(toggled(bool)),m_login,SLOT(setDisabled(bool)));
	connect(m_anonymous,SIGNAL(toggled(bool)),m_password,SLOT(setDisabled(bool)));
}

void KMCupsConfigWidget::load()
{
	CupsInfos	*inf = CupsInfos::self();
	m_host->setText(inf->host());
	m_port->setText(QString::number(inf->port()));
	if (inf->login().isEmpty())
		m_anonymous->setChecked(true);
	else
	{
		m_login->setText(inf->login());
		m_password->setText(inf->password());
	}
}

void KMCupsConfigWidget::save(bool sync)
{
	CupsInfos	*inf = CupsInfos::self();
	inf->setHost(m_host->text());
	inf->setPort(m_port->text().toInt());
	if (m_anonymous->isChecked())
	{
		inf->setLogin(QString::null);
		inf->setPassword(QString::null);
	}
	else
	{
		inf->setLogin(m_login->text());
		inf->setPassword(m_password->text());
	}
	if (sync) inf->save();
}

void KMCupsConfigWidget::saveConfig(KConfig *conf)
{
	conf->setGroup("CUPS");
	conf->writeEntry("Host",m_host->text());
	conf->writeEntry("Port",m_port->text().toInt());
	conf->writeEntry("Login",(m_anonymous->isChecked() ? QString::null : m_login->text()));
	// synchronize CupsInfos object
	save(false);
}
