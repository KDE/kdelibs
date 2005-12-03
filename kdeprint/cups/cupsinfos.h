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

#ifndef CUPSINFOS_H
#define CUPSINFOS_H

#include <qstring.h>
#include "kpreloadobject.h"

class CupsInfos : public KPReloadObject
{
public:
	static CupsInfos* self();

	CupsInfos();
	~CupsInfos();

	const QString& host() const;
	int port() const;
	const QString& login() const;
	const QString& password() const;
	const QString& realLogin() const;
	bool savePassword() const;

	void setHost(const QString& s);
	void setPort(int p);
	void setLogin(const QString& s);
	void setPassword(const QString& s);
	void setSavePassword( bool on );

	const char* getPasswordCB();

	void load();
	void save();

protected:
	void reload();
	void configChanged();

private:
	static CupsInfos	*unique_;

	QString	host_;
	int	port_;
	QString	login_;
	QString	password_;
	QString	reallogin_;
	bool savepwd_;

	int	count_;
};

inline const QString& CupsInfos::host() const
{ return host_; }

inline int CupsInfos::port() const
{ return port_; }

inline const QString& CupsInfos::login() const
{ return login_; }

inline const QString& CupsInfos::password() const
{ return password_; }

inline const QString& CupsInfos::realLogin() const
{ return reallogin_; }

inline bool CupsInfos::savePassword() const
{ return savepwd_; }

#endif
