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

#include "matichandler.h"
#include "printcapentry.h"
#include "kmprinter.h"
#include "matic.h"
#include "matichelper.h"

#include <klocale.h>
#include <kstringhandler.h>
#include <kdebug.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>

MaticHandler::MaticHandler(KMManager *mgr)
: LprHandler("foomatic", mgr)
{
}

bool MaticHandler::validate(PrintcapEntry *entry)
{
	if (entry)
		return (entry->field("if").right(9) == "lpdomatic");
	return false;
}

KMPrinter* MaticHandler::createPrinter(PrintcapEntry *entry)
{
	if (entry && validate(entry))
	{
		KMPrinter	*prt = new KMPrinter;
		prt->setName(entry->name);
		prt->setPrinterName(entry->name);
		prt->setType(KMPrinter::Printer);
		//if (entry->field("lp") == "/dev/null" || entry->field("lp").isEmpty())
		//	prt->addType(KMPrinter::Remote);
		return prt;
	}
	return NULL;
}

bool MaticHandler::completePrinter(KMPrinter *prt, PrintcapEntry *entry, bool shortmode)
{
	QString	val = entry->field("lp");
	if (val == "/dev/null" || val.isEmpty())
	{
		prt->setLocation(i18n("Network printer"));
	}
	else
	{
		prt->setLocation(i18n("Local printer on %1").arg(val));
		KURL	url(val);
		if (val.find("usb") != -1)
			url.setProtocol("usb");
		else
			url.setProtocol("parallel");
		prt->setDevice(url);
	}
	prt->setDescription(entry->aliases.join(", "));

	if (!shortmode)
	{
		MaticBlock	*blk = loadMaticDriver(entry);
		if (blk)
		{
			QString	postpipe = blk->arg("$postpipe");
			if (!postpipe.isEmpty())
			{
				KURL	url = parsePostpipe(postpipe);
				if (!url.isEmpty())
				{
					QString	ds = QString::fromLatin1("%1 (%2)").arg(prt->location()).arg(url.protocol());
					prt->setDevice(url);
					prt->setLocation(ds);
				}
			}

			MaticBlock	*varblk = blk->block("$VAR1");
			if (varblk)
			{
				prt->setManufacturer(varblk->arg("make"));
				prt->setModel(varblk->arg("model"));
				prt->setDriverInfo(QString::fromLatin1("%1 %2 (%3)").arg(prt->manufacturer()).arg(prt->model()).arg(varblk->arg("driver")));
			}
		}
	}

	return true;
}

MaticBlock* MaticHandler::loadMaticDriver(PrintcapEntry *entry)
{
	QString	s(entry->field("af"));
	if (s.isEmpty())
	{
		s = entry->field("filter_options");
		if (!s.isEmpty())
		{
			int	p = s.findRev(' ');
			if (p != -1)
				s = s.right(s.length()-p-1);
		}
	}
	QFile	f(s);
	if (!f.exists() || !f.open(IO_ReadOnly))
	{
		kdDebug() << "unable to load matic data: " << s << endl;
		return NULL;
	}

	QCString	buffer(f.size()+1);
	f.readBlock(buffer.data(), f.size());
	f.close();

	MaticBlock	*blk = loadMaticData(buffer.data());
	return blk;
}

KURL MaticHandler::parsePostpipe(const QString& s)
{
	KURL	url;
	int	p = s.findRev('|');
	QStringList	args = QStringList::split(" ", s.right(s.length()-p-1));

	if (args.count() != 0)
	{
		// socket printer
		if (args[0].right(3) == "/nc")
		{
			url.setProtocol("socket");
			url.setHost(args[1]);
			if (args.count() > 2)
				url.setPort(args[2].toInt());
			else
				url.setPort(9100);
		}
		// smb printer
		else if (args[0].right(10) == "/smbclient")
		{
			url.setProtocol("smb");
			QStringList	host_components = QStringList::split(QRegExp("/|\\\\\""), args[1], false);
			QString	workgrp, user, pass;
			for (int i=2; i<args.count(); i++)
			{
				if (args[i] == "-U")
					user = args[++i];
				else if (args[i] == "-W")
					workgrp = args[++i];
				else if (args[i][0] != '-' && i == 2)
					pass = args[i];
			}
			if (!workgrp.isEmpty())
			{
				url.setHost(workgrp);
				url.setPath("/"+host_components[0]+"/"+host_components[1]);
			}
			else
			{
				url.setHost(host_components[0]);
				url.setPath("/"+host_components[1]);
			}
			if (!user.isEmpty())
				url.setUser(user);
			if (!pass.isEmpty())
				url.setPass(pass);
		}
		// remote printer
		else if (args[0].right(5) == "/rlpr")
		{
			int	i=1;
			while (i < args.count())
			{
				if (args[i].left(2) != "-P")
					i++;
				else
				{
					QString	host = (args[i].length() == 2 ? args[i+1] : args[i].right(args[i].length()-2));
					int	p = host.find("\\@");
					if (p != -1)
					{
						url.setProtocol("lpd");
						url.setHost(host.right(host.length()-p-2));
						url.setPath("/"+host.left(p));
					}
					break;
				}
			}
		}
	}

	return url;
}

DrMain* MaticHandler::loadDriver(KMPrinter*, PrintcapEntry *entry)
{
	MaticBlock	*blk = loadMaticDriver(entry);
	if (blk)
		return maticToDriver(blk);
	return NULL;
}
