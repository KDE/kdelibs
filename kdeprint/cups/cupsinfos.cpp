#include "cupsinfos.h"

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

void CupsInfos::setHost(const QString& s)
{
	host_ = s;
	cupsSetServer(s.local8Bit());
	save();
}

void CupsInfos::setPort(int p)
{
	port_ = p;
	ippSetPort(p);
	save();
}

void CupsInfos::setLogin(const QString& s)
{
	login_ = s;
	cupsSetUser(s.local8Bit());
	save();
}

void CupsInfos::setPassword(const QString& s)
{
	password_ = s;
}

const char* CupsInfos::getPasswordCB()
{
	if (count_ == 0 && !password_.isEmpty()) return password_.local8Bit();
	else
	{
		QString	msg = i18n("<p>The access to the requested resource on the CUPS server running on <b>%1</b> (port <b>%2</b>) requires a password.</p>").arg(host_).arg(port_);
		KIO::PasswordDialog	dlg(msg,login_);
		count_++;
		if (dlg.exec())
		{
			setLogin(dlg.username());
			setPassword(dlg.password());
			return password_.local8Bit();
		}
		else
			return NULL;
	}
	return NULL;
}

void CupsInfos::load()
{
	KConfig	conf_("kdeprintrc");
	conf_.setGroup("CUPS");
	host_ = conf_.readEntry("Host",QString::fromLocal8Bit(cupsServer()));
	port_ = conf_.readNumEntry("Port",ippPort());
	login_ = conf_.readEntry("Login",QString::fromLocal8Bit(cupsUser()));
	password_ = QString::null;
	if (login_.isEmpty()) login_ = QString::null;

	// synchronize with CUPS
	cupsSetServer(host_.local8Bit());
	cupsSetUser(login_.local8Bit());
	ippSetPort(port_);
}

void CupsInfos::save()
{
	KConfig	conf_("kdeprintrc");
	conf_.setGroup("CUPS");
	conf_.writeEntry("Host",host_);
	conf_.writeEntry("Port",port_);
	conf_.writeEntry("Login",login_);
	// don't write password for obvious security...
}
