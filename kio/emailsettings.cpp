/*
 * Copyright (c) 2000 Alex Zepeda <jazepeda@pacbell.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$Id$
 */

#include "emailsettings.h"

#include <kconfig.h>

class KEMailSettingsPrivate {
public:
	KConfig *m_pConfig;
	QStringList profiles;
	QString m_sDefaultProfile, m_sCurrentProfile;
};

QString KEMailSettings::defaultProfileName()
{
	return p->m_sDefaultProfile.copy();
}

QString KEMailSettings::getSetting(KEMailSettings::Setting s)
{
	p->m_pConfig->setGroup(QString("PROFILE_")+p->m_sCurrentProfile);
	switch (s) {
		case ClientProgram: {
			return p->m_pConfig->readEntry("EmailClient");
			break;
		}
		case ClientTerminal: {
			return ((p->m_pConfig->readBoolEntry("TerminalClient")) ? QString("true") : QString("false") );
			break;
		}
		case RealName: {
			return p->m_pConfig->readEntry("FullName");
			break;
		}
		case EmailAddress: {
			return p->m_pConfig->readEntry("EmailAddress");
			break;
		}
		case ReplyToAddress: {
			return p->m_pConfig->readEntry("ReplyAddr");
			break;
		}
		case Organization: {
			return p->m_pConfig->readEntry("Organization");
			break;
		}
		case OutServer: {
			return p->m_pConfig->readEntry("Outgoing");
			break;
		}
		case InServer: {
			return p->m_pConfig->readEntry("Incoming");
			break;
		}
		case InServerLogin: {
			return p->m_pConfig->readEntry("UserName");
			break;
		}
		case InServerPass: {
			return p->m_pConfig->readEntry("Password");
			break;
		}
		case InServerType: {
			return p->m_pConfig->readEntry("ServerType");
			break;
		}
	};
	return QString::null;
}
void KEMailSettings::setSetting(KEMailSettings::Setting s, const QString  &v)
{
	p->m_pConfig->setGroup(QString("PROFILE_")+p->m_sCurrentProfile);
	switch (s) {
		case ClientProgram: {
			p->m_pConfig->writeEntry("EmailClient", v);
			break;
		}
		case ClientTerminal: {
			p->m_pConfig->writeEntry("TerminalClient", (v == "true") ? true : false );
			break;
		}
		case RealName: {
			p->m_pConfig->writeEntry("FullName", v);
			break;
		}
		case EmailAddress: {
			p->m_pConfig->writeEntry("EmailAddress", v);
			break;
		}
		case ReplyToAddress: {
			p->m_pConfig->writeEntry("ReplyAddr", v);
			break;
		}
		case Organization: {
			p->m_pConfig->writeEntry("Organization", v);
			break;
		}
		case OutServer: {
			p->m_pConfig->writeEntry("Outgoing", v);
			break;
		}
		case InServer: {
			p->m_pConfig->writeEntry("Incoming", v);
			break;
		}
		case InServerLogin: {
			p->m_pConfig->writeEntry("UserName", v);
			break;
		}
		case InServerPass: {
			p->m_pConfig->writeEntry("Password", v);
			break;
		}
		case InServerType: {
			p->m_pConfig->writeEntry("ServerType", v);
			break;
		}
	};
	p->m_pConfig->sync();
}

void KEMailSettings::setDefault(const QString &s)
{
	p->m_pConfig->setGroup("Defaults");
	p->m_pConfig->writeEntry("Profile", s);
	p->m_pConfig->sync();
}

void KEMailSettings::setProfile (const QString &s)
{
	p->m_sCurrentProfile=s.copy();
}

QString KEMailSettings::currentProfileName()
{
	return p->m_sCurrentProfile;
}

QStringList KEMailSettings::profiles()
{
	return p->profiles;
}

KEMailSettings::KEMailSettings()
{
	p = new KEMailSettingsPrivate();
	p->m_sCurrentProfile=QString::null;

	p->m_pConfig = new KConfig("emaildefaults");

	QStringList groups = p->m_pConfig->groupList();
	for (QStringList::Iterator it = groups.begin(); it != groups.end(); ++it) {
		if ( (*it).left(8) == "PROFILE_" )
			p->profiles+= (*it).mid(8, (*it).length());
	}

	p->m_pConfig->setGroup("Defaults");
	p->m_sDefaultProfile=p->m_pConfig->readEntry("Profile");
	if (p->m_sDefaultProfile != QString::null) {
		if (!p->m_pConfig->hasGroup(QString("PROFILE_")+p->m_sDefaultProfile))
			p->m_sDefaultProfile=QString::null;
	} else if (p->profiles.count()) {
		p->m_pConfig->setGroup("Defaults");
		p->m_pConfig->writeEntry("Profile", p->profiles[0]);
		p->m_pConfig->sync();
		p->m_sDefaultProfile=p->profiles[0];
	}
	setProfile(p->m_sDefaultProfile);
}

KEMailSettings::~KEMailSettings()
{
}
