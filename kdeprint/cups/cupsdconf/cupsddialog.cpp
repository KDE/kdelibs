/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
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

#include "cupsddialog.h"

#include "cupsdpage.h"
#include "cupsdconf.h"
#include "cupsdserveridentitypage.h"
#include "cupsdserverlogpage.h"
#include "cupsdserverdirpage.h"
#include "cupsdserverhttppage.h"
#include "cupsdserverjobpage.h"
#include "cupsdservermiscpage.h"
#include "cupsdbrowsinggeneralpage.h"
#include "cupsdbrowsingtimeoutpage.h"
#include "cupsdsplash.h"
#include "cupsdbrowsingmaskspage.h"
#include "cupsdbrowsingconnpage.h"
#include "cupsdnetworkgeneralpage.h"
#include "cupsdbrowsingrelaypage.h"
#include "cupsdnetworkclientspage.h"
#include "cupsdserversecuritypage.h"
#include "cupsdserverencryptpage.h"

#include <qdir.h>
#include <qvbox.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <qstringlist.h>

#include <signal.h>

extern "C"
{
	bool restartServer(QString& msg)
	{
		return CupsdDialog::restartServer(msg);
	}
	bool configureServer(const QString& configfile, QWidget *parent)
	{
		CupsdDialog::configure(configfile,parent);
		return true;
	}
}

int getServerPid()
{
	QDir	dir("/proc",QString::null,QDir::Name,QDir::Dirs);
	for (uint i=0;i<dir.count();i++)
	{
		if (dir[i] == "." || dir[i] == ".." || dir[i] == "self") continue;
		QFile	f("/proc/" + dir[i] + "/cmdline");
		if (f.exists() && f.open(IO_ReadOnly))
		{
			QTextStream	t(&f);
			QString	line;
			t >> line;
			f.close();
			if (line.right(5) == "cupsd" ||
			    line.right(6).left(5) == "cupsd")	// second condition for 2.4.x kernels
								// which add a null byte at the end
				return dir[i].toInt();
		}
	}
	return (-1);
}

//---------------------------------------------------

CupsdDialog::CupsdDialog(QWidget *parent, const char *name)
	: MyDialogBase(parent, name)
{
	KGlobal::iconLoader()->addAppDir("kdeprint");
	KGlobal::locale()->insertCatalogue("cupsdconf");

	pagelist_.setAutoDelete(false);
	filename_ = "";
	conf_ = 0;
	constructDialog();

        setCaption(i18n("CUPS Server Configuration"));

        resize(500, 400);
}

CupsdDialog::~CupsdDialog()
{
        delete conf_;
}

void CupsdDialog::constructDialog()
{
	CupsdPage	*cpage = new CupsdSplash(0);
	pagelist_.append(cpage);
        addPage(cpage->pagePath(), cpage->header(), SmallIcon("fileprint"), cpage);

	cpage = new CupsdServerSecurityPage(0);
	pagelist_.append(cpage);
        addPage(cpage->pagePath(), cpage->header(), SmallIcon("folder_locked"), cpage);

	cpage = new CupsdBrowsingGeneralPage(0);
	pagelist_.append(cpage);
        addPage(cpage->pagePath(), cpage->header(), SmallIcon("kdeprint_printer_remote"), cpage);

	cpage = new CupsdBrowsingRelayPage(0);
	pagelist_.append(cpage);
        addPage(cpage->pagePath(), cpage->header(), SmallIcon("connect_established"), cpage);

	cpage = new CupsdBrowsingTimeoutPage(0);
	pagelist_.append(cpage);
        addPage(cpage->pagePath(), cpage->header(), SmallIcon("date"), cpage);

        cpage = new CupsdBrowsingMasksPage(0);
	pagelist_.append(cpage);
        addPage(cpage->pagePath(), cpage->header(), SmallIcon("encrypted"), cpage);

	cpage = new CupsdBrowsingConnPage(0);
	pagelist_.append(cpage);
        addPage(cpage->pagePath(), cpage->header(), SmallIcon("connect_no"), cpage);

	cpage = new CupsdNetworkGeneralPage(0);
	pagelist_.append(cpage);
        addPage(cpage->pagePath(), cpage->header(), SmallIcon("network"), cpage);

	cpage = new CupsdNetworkClientsPage(0);
	pagelist_.append(cpage);
        addPage(cpage->pagePath(), cpage->header(), SmallIcon("openterm"), cpage);

	cpage = new CupsdServerIdentityPage(0);
	pagelist_.append(cpage);
        addPage(cpage->pagePath(), cpage->header(), SmallIcon("gear"), cpage);

	cpage = new CupsdServerMiscPage(0);
	pagelist_.append(cpage);
        addPage(cpage->pagePath(), cpage->header(), SmallIcon("package_utilities"), cpage);

	cpage = new CupsdServerJobPage(0);
	pagelist_.append(cpage);
        addPage(cpage->pagePath(), cpage->header(), SmallIcon("kdeprint_job"), cpage);

	cpage = new CupsdServerEncryptPage(0);
	pagelist_.append(cpage);
        addPage(cpage->pagePath(), cpage->header(), SmallIcon("password"), cpage);

	cpage = new CupsdServerHTTPPage(0);
	pagelist_.append(cpage);
        addPage(cpage->pagePath(), cpage->header(), SmallIcon("konqueror"), cpage);

	cpage = new CupsdServerDirPage(0);
	pagelist_.append(cpage);
        addPage(cpage->pagePath(), cpage->header(), SmallIcon("folder"), cpage);

	cpage = new CupsdServerLogPage(0);
	pagelist_.append(cpage);
        addPage(cpage->pagePath(), cpage->header(), SmallIcon("contents"), cpage);

	conf_ = new CupsdConf();
	for (pagelist_.first();pagelist_.current();pagelist_.next())
        {
		pagelist_.current()->setDefaults();
                pagelist_.current()->setInfos(conf_);
        }
}

bool CupsdDialog::setConfigFile(const QString& filename)
{
	filename_ = filename;
	if (!conf_->loadFromFile(filename_))
	{
		KMessageBox::error(this, i18n("Error while loading configuration file !"), i18n("CUPS configuration error"));
		return false;
	}
	bool	ok(true);
	QString	msg;
	for (pagelist_.first();pagelist_.current() && ok;pagelist_.next())
		ok = pagelist_.current()->loadConfig(conf_, msg);
	if (!ok)
	{
		KMessageBox::error(this, msg, i18n("CUPS configuration error"));
		return false;
	}
	return true;
}

void CupsdDialog::slotOk()
{
        slotApply();
        MyDialogBase::slotOk();
}

bool CupsdDialog::restartServer(QString& msg)
{
	int	serverPid = getServerPid();
        msg.truncate(0);
	if (serverPid <= 0)
	{
		msg = i18n("Unable to find a running CUPS server");
	}
	else
	{
		if (::kill(serverPid, SIGHUP) != 0)
			msg = i18n("Unable to restart CUPS server (pid = %1)").arg(serverPid);
	}
        return (msg.isEmpty());
}

#define	DEFAULT_CONFIGFILE	"/etc/cups/cupsd.conf"
void CupsdDialog::configure(const QString& filename, QWidget *parent)
{
	QString	fn = (filename.isEmpty() ? QString(DEFAULT_CONFIGFILE) : filename);
	QFileInfo	fi(fn);
	QString		errormsg;
	// check existence
	if (!fi.exists()) errormsg = i18n("File \"%1\" does not exist!").arg(fn);
	// check read state
	else if (!fi.isReadable()) errormsg = i18n("Unable to open file \"%1\"!\nCheck file permissions.").arg(fn);
	// check write state
	else if (!fi.isWritable()) errormsg = i18n("You are not allowed to modify file \"%1\"!\nCheck file permissions or contact the system administrator.").arg(fn);

	if (!errormsg.isEmpty())
	{
		KMessageBox::error(parent, errormsg, i18n("CUPS configuration error"));
	}
	else
	{
		CupsdDialog	dlg(parent);
		if (dlg.setConfigFile(fn))
			dlg.exec();
	}
}

void CupsdDialog::slotApply()
{
        if (conf_ && !filename_.isEmpty())
	{ // try to save the file
		bool	ok(true);
		QString	msg;
		CupsdConf	newconf_;
		for (pagelist_.first();pagelist_.current() && ok;pagelist_.next())
			ok = pagelist_.current()->saveConfig(&newconf_, msg);
		if (!ok)
		{
			; // do nothing
		}
		else if (!newconf_.saveToFile(filename_))
		{
			msg = i18n("Unable to write configuration file %1").arg(filename_);
                        ok = false;
		}
		else ok = restartServer(msg);
                if (!ok)
                {
			KMessageBox::error(this, msg, i18n("CUPS configuration error"));
                }
        }
}

int CupsdDialog::serverPid()
{
	return getServerPid();
}

int CupsdDialog::serverOwner()
{
	int	pid = getServerPid();
	if (pid > 0)
	{
		QString	str;
		str.sprintf("/proc/%d/status",pid);
		QFile	f(str);
		if (f.exists() && f.open(IO_ReadOnly))
		{
			QTextStream	t(&f);
			while (!t.eof())
			{
				str = t.readLine();
				if (str.find("Uid:",0,false) == 0)
				{
					QStringList	list = QStringList::split('\t', str, false);
					if (list.count() >= 2)
					{
						bool	ok;
						int	u = list[1].toInt(&ok);
						if (ok) return u;
					}
				}
			}
		}
	}
	return (-1);
}
#include "cupsddialog.moc"
