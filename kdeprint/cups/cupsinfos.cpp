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

#include "cupsinfos.h"
#include "kmfactory.h"
#include "kmtimer.h"
#include "messagewindow.h"

#include <kio/passdlg.h>
#include <klocale.h>
#include <kconfig.h>

#include <cups/cups.h>
#include <cups/ipp.h>

const char* cupsGetPasswordCB(const char*)
{
	return CupsInfos::self()->getPasswordCB();
}

CupsInfos* CupsInfos::unique_ = 0;

CupsInfos* CupsInfos::self()
{
	if (!unique_)
	{
		unique_ = new CupsInfos();
	}
	return unique_;
}

CupsInfos::CupsInfos()
: KPReloadObject(true)
{
	count_ = 0;

	load();
/*	host_ = cupsServer();
	login_ = cupsUser();
	if (login_.isEmpty()) login_ = QString::null;
	port_ = ippPort();
	password_ = QString::null;*/

	cupsSetPasswordCB(cupsGetPasswordCB);
}

CupsInfos::~CupsInfos()
{
}

void CupsInfos::setHost(const QString& s)
{
	host_ = s;
	cupsSetServer(s.latin1());
}

void CupsInfos::setPort(int p)
{
	port_ = p;
	ippSetPort(p);
}

void CupsInfos::setLogin(const QString& s)
{
	login_ = s;
	cupsSetUser(s.latin1());
}

void CupsInfos::setPassword(const QString& s)
{
	password_ = s;
}

const char* CupsInfos::getPasswordCB()
{
	if (count_ == 0 && !password_.isEmpty()) return password_.latin1();
	else
	{
		// to avoid focus/modality problem, message windows must be removed
		MessageWindow::removeAll();

		QString	msg = i18n("<p>The access to the requested resource on the CUPS server running on <b>%1</b> (port <b>%2</b>) requires a password.</p>").arg(host_).arg(port_);
		bool ok(false);;
		KIO::PasswordDialog	dlg(msg,login_);
		count_++;
		KMTimer::self()->hold();
		if (dlg.exec())
		{
			setLogin(dlg.username());
			setPassword(dlg.password());
			ok = true;
		}
		KMTimer::self()->release();
		return (ok ? password_.latin1() : NULL);
	}
	return NULL;
}

void CupsInfos::load()
{
	KConfig	*conf_ = KMFactory::self()->printConfig();
	conf_->setGroup("CUPS");
	host_ = conf_->readEntry("Host",QString::fromLatin1(cupsServer()));
	port_ = conf_->readNumEntry("Port",ippPort());
	login_ = conf_->readEntry("Login",QString::fromLatin1(cupsUser()));
	password_ = QString::null;
	if (login_.isEmpty()) login_ = QString::null;
	reallogin_ = cupsUser();

	// synchronize with CUPS
	cupsSetServer(host_.latin1());
	cupsSetUser(login_.latin1());
	ippSetPort(port_);
}

void CupsInfos::save()
{
	KConfig	*conf_ = KMFactory::self()->printConfig();
	conf_->setGroup("CUPS");
	conf_->writeEntry("Host",host_);
	conf_->writeEntry("Port",port_);
	conf_->writeEntry("Login",login_);
	// don't write password for obvious security...
	conf_->sync();
}

void CupsInfos::reload()
{
	// do nothing, but needs to be implemented
}

void CupsInfos::configChanged()
{
	// we need to reload settings
	load();
}
