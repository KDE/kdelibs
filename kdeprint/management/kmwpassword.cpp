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

#include "kmwpassword.h"
#include "kmwizard.h"
#include "kmprinter.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <klocale.h>
#include <kcursor.h>

#include <stdlib.h>

KMWPassword::KMWPassword(QWidget *parent, const char *name)
: KMWizardPage(parent,name)
{
	m_title = i18n("User Identification");
	m_ID = KMWizard::Password;
	m_nextpage = KMWizard::SMB;

	// create widgets
	QLabel	*infotext_ = new QLabel(this);
	infotext_->setText(i18n("<p>This backend may require a login/password to work properly. "
				"Fill in the required entries or enable <b>Guest account</b> to "
				"use anonymous access.</p>"));
	m_login = new QLineEdit(this);
	m_login->setText(QString::fromLocal8Bit(getenv("USER")));
	m_password = new QLineEdit(this);
	m_password->setEchoMode(QLineEdit::Password);
	QLabel	*loginlabel_ = new QLabel(i18n("&Login:"),this);
	QLabel	*passwdlabel_ = new QLabel(i18n("&Password:"),this);
	m_guest = new QCheckBox(i18n("&Guest account"),this);
	m_guest->setCursor(KCursor::handCursor());

	loginlabel_->setBuddy(m_login);
	passwdlabel_->setBuddy(m_password);

	m_guest->setChecked(true);
	m_login->setEnabled(false);
	m_password->setEnabled(false);
	connect(m_guest,SIGNAL(toggled(bool)),m_login,SLOT(setDisabled(bool)));
	connect(m_guest,SIGNAL(toggled(bool)),m_password,SLOT(setDisabled(bool)));

	// layout
	QGridLayout	*main_ = new QGridLayout(this, 7, 2, 0, 5);
	main_->setRowStretch(6,1);
	main_->setColStretch(1,1);
	main_->addRowSpacing(1,20);
	main_->addRowSpacing(4,20);
	main_->addMultiCellWidget(infotext_, 0, 0, 0, 1);
	main_->addWidget(loginlabel_, 2, 0);
	main_->addWidget(passwdlabel_, 3, 0);
	main_->addWidget(m_login, 2, 1);
	main_->addWidget(m_password, 3, 1);
	main_->addMultiCellWidget(m_guest, 5, 5, 0, 1);
}

bool KMWPassword::isValid(QString& msg)
{
	if (!m_guest->isChecked() && m_login->text().isEmpty())
	{
		msg = i18n("User name is empty!");
		return false;
	}
	return true;
}

void KMWPassword::updatePrinter(KMPrinter *p)
{
	QString	s = p->option("kde-backend");
	if (!s.isEmpty()) setNextPage(s.toInt());
	else setNextPage(KMWizard::Error);
	if (m_guest->isChecked())
	{
		p->setOption("kde-login",QString::null);
		p->setOption("kde-password",QString::null);
	}
	else
	{
		p->setOption("kde-login",m_login->text());
		p->setOption("kde-password",m_password->text());
	}
}

