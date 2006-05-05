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

#include "kvisiblebuttongroup.h"
#include "kmwpassword.h"
#include "kmwizard.h"
#include "kmprinter.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <klocale.h>

#include <stdlib.h>

KMWPassword::KMWPassword(QWidget *parent)
    : KMWizardPage(parent)
{
	m_title = i18n("User Identification");
	m_ID = KMWizard::Password;
	m_nextpage = KMWizard::SMB;

	// create widgets
	QLabel	*infotext_ = new QLabel(this);
	infotext_->setWordWrap(true);
	infotext_->setText(i18n("<p>This backend may require a login/password to work properly. "
				"Select the type of access to use and fill in the login and password entries if needed.</p>"));
	m_login = new QLineEdit(this);
	m_login->setText(QString::fromLocal8Bit(getenv("USER")));
	m_password = new QLineEdit(this);
	m_password->setEchoMode(QLineEdit::Password);
	QLabel	*loginlabel_ = new QLabel(i18n("&Login:"),this);
	QLabel	*passwdlabel_ = new QLabel(i18n("&Password:"),this);

	m_btngroup = new KVisibleButtonGroup( this );
	m_btngroup->addButton(new KRadioButtonWithHandOver( i18n( "&Anonymous (no login/password)" )), 0);
	m_btngroup->addButton(new KRadioButtonWithHandOver( i18n( "&Guest account (login=\"guest\")" )), 1);
	m_btngroup->addButton(new KRadioButtonWithHandOver( i18n( "Nor&mal account" )), 2);
	m_btngroup->button( 0 )->click();

	loginlabel_->setBuddy(m_login);
	passwdlabel_->setBuddy(m_password);

	m_login->setEnabled(false);
	m_password->setEnabled(false);
	connect(m_btngroup->button( 2 ),SIGNAL(toggled(bool)),m_login,SLOT(setEnabled(bool)));
	connect(m_btngroup->button( 2 ),SIGNAL(toggled(bool)),m_password,SLOT(setEnabled(bool)));

	// layout
	QVBoxLayout *main_ = new QVBoxLayout( this );
	main_->setMargin( 0 );
	main_->setSpacing( 0 );
	main_->addWidget( infotext_ );
	main_->addSpacing( 10 );
	main_->addWidget( m_btngroup );
	QGridLayout *l1 = new QGridLayout( );
	main_->addLayout( l1 );
	main_->addStretch( 1 );
	l1->setColumnMinimumWidth( 0, 35 );
	l1->setColumnStretch( 2, 1 );
	l1->addWidget( loginlabel_, 0, 1 );
	l1->addWidget( passwdlabel_, 1, 1 );
	l1->addWidget( m_login, 0, 2 );
	l1->addWidget( m_password, 1, 2 );
}

bool KMWPassword::isValid(QString& msg)
{
	if ( !m_btngroup->checkedButton() )
		msg = i18n( "Select one option" );
	else if (m_btngroup->checkedId() == 2 && m_login->text().isEmpty())
		msg = i18n("User name is empty.");
	else
		return true;
	return false;
}

void KMWPassword::initPrinter( KMPrinter* p )
{
	/* guest account only for SMB backend */
	if ( p->option( "kde-backend" ).toInt() != KMWizard::SMB )
	{
		int ID = m_btngroup->checkedId();
		m_btngroup->button( 1 )->hide();
		if ( ID == 1 )
			m_btngroup->button( 0 )->click();
	}
	else
		m_btngroup->button( 1 )->show();
}

void KMWPassword::updatePrinter(KMPrinter *p)
{
	QString	s = p->option("kde-backend");
	if (!s.isEmpty())
		setNextPage(s.toInt());
	else
		setNextPage(KMWizard::Error);
	switch ( m_btngroup->checkedId() )
	{
		case 0:
			p->setOption( "kde-login", QString() );
			p->setOption( "kde-password", QString() );
			break;
		case 1:
			p->setOption( "kde-login", QLatin1String( "guest" ) );
			p->setOption( "kde-password", QString() );
			break;
		case 2:
			p->setOption( "kde-login", m_login->text() );
			p->setOption( "kde-password", m_password->text() );
			break;
	}
}

