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

#include "kio_print.h"
#include "kmprinter.h"
#include "kmmanager.h"
#include "kmfactory.h"

#include <qfile.h>
#include <qtextstream.h>
#include <klocale.h>
#include <kdebug.h>
#include <kinstance.h>
#include <kio/global.h>
#include <kstddirs.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define	PRINT_DEBUG	kdDebug(7019) << "kio_print: "

extern "C"
{
	int kdemain(int argc, char **argv);
}

int kdemain(int argc, char **argv)
{
	KInstance	instance("kio_print");

	PRINT_DEBUG << "starting ioslave" << endl;
	if (argc != 4)
	{
		fprintf(stderr, "Usage: kio_print protocol domain-socket1 domain-socket2\n");
		exit(-1);
	}

	KIO_Print	 slave(argv[2], argv[3]);
	slave.dispatchLoop();

	PRINT_DEBUG << "done" << endl;
	return 0;
}

KIO_Print::KIO_Print(const QCString& pool, const QCString& app)
: KIO::SlaveBase("print", pool, app)
{
}

void KIO_Print::listDir(const KURL& url)
{
	QString	path( url.path(-1) );

	PRINT_DEBUG << "listing " << path << endl;
	if (path == "/")
		listRoot();
	else if (path.find('/', 1) == -1)
	{
		PRINT_DEBUG << "listing group " << path << endl;

		int	mask;
		QString	mimeType;
		if (path == "/printers")
		{
			mask = KMPrinter::Printer;
			mimeType = "print/printer";
		}
		else if (path == "/classes")
		{
			mask = KMPrinter::Class | KMPrinter::Implicit;
			mimeType = "print/class";
		}
		else if (path == "/specials")
		{
			mask = KMPrinter::Special;
			mimeType = "print/printer";
		}
		else
			error(KIO::ERR_DOES_NOT_EXIST, url.url());
		QListIterator<KMPrinter>	it(*(KMFactory::self()->manager()->printerList()));
		for (;it.current();++it)
		{
			if (!(it.current()->type() & mask) || !it.current()->instanceName().isEmpty())
			{
				PRINT_DEBUG << "rejecting " << it.current()->name() << endl;
				continue;
			}

			KIO::UDSEntry	entry;
			KIO::UDSAtom	atom;
			atom.m_uds = KIO::UDS_NAME;
			atom.m_str = it.current()->name();
			entry.append(atom);
			atom.m_uds = KIO::UDS_FILE_TYPE;
			atom.m_long = S_IFREG;
			entry.append(atom);
			atom.m_uds = KIO::UDS_URL;
			atom.m_str = QString::fromLatin1("print:%1/%2").arg(path).arg(it.current()->name());
			entry.append(atom);
			atom.m_uds = KIO::UDS_ACCESS;
			atom.m_long = S_IRWXO | S_IRWXG | S_IRWXU;
			entry.append(atom);
			atom.m_uds = KIO::UDS_MIME_TYPE;
			atom.m_str = mimeType;
			entry.append(atom);
			atom.m_uds = KIO::UDS_GUESSED_MIME_TYPE;
			atom.m_str = "text/html";
			entry.append(atom);

			PRINT_DEBUG << "accepting " << it.current()->name() << endl;
			listEntry(entry, false);
		}

		listEntry(KIO::UDSEntry(), true);
		finished();
	}
	else
	{
		error(KIO::ERR_UNSUPPORTED_ACTION, i18n("Unsupported path %1").arg(path));
	}
}

void KIO_Print::listRoot()
{
	PRINT_DEBUG << "listing root entry" << endl;

	KIO::UDSAtom		atom;
	KIO::UDSEntry	entry;

	// Classes entry
	createRootEntry(entry);
	atom.m_uds = KIO::UDS_NAME;
	atom.m_str = i18n("Classes");
	entry.append(atom);
	atom.m_uds = KIO::UDS_URL;
	atom.m_str = QString::fromLatin1("print:/classes/");
	entry.append(atom);
	listEntry(entry, false);

	// Printers entry
	createRootEntry(entry);
	atom.m_uds = KIO::UDS_NAME;
	atom.m_str = i18n("Printers");
	entry.append(atom);
	atom.m_uds = KIO::UDS_URL;
	atom.m_str = QString::fromLatin1("print:/printers/");
	entry.append(atom);
	listEntry(entry, false);

	// Specials entry
	createRootEntry(entry);
	atom.m_uds = KIO::UDS_NAME;
	atom.m_str = i18n("Specials");
	entry.append(atom);
	atom.m_uds = KIO::UDS_URL;
	atom.m_str = QString::fromLatin1("print:/specials/");
	entry.append(atom);
	listEntry(entry, false);

	// finish
	listEntry(entry, true);
	finished();
}

void KIO_Print::createRootEntry(KIO::UDSEntry& entry)
{
	KIO::UDSAtom	atom;

	entry.clear();
	atom.m_uds = KIO::UDS_FILE_TYPE;
	atom.m_long = S_IFDIR;
	entry.append(atom);
	atom.m_uds = KIO::UDS_ACCESS;
	atom.m_long = S_IRWXO | S_IRWXG | S_IRWXU;
	entry.append(atom);
	atom.m_uds = KIO::UDS_MIME_TYPE;
	atom.m_str = QString::fromLatin1("print/folder");
	entry.append(atom);
	atom.m_uds = KIO::UDS_GUESSED_MIME_TYPE;
	atom.m_str = QString::fromLatin1("inode/directory");
	entry.append(atom);
}

void KIO_Print::get(const KURL& url)
{
	QStringList	elems = QStringList::split('/', url.path(), false);
	QString		group(elems[0]), printer(elems[1]);
	KMPrinter	*mprinter(0);

	KMFactory::self()->manager()->printerList(false);
	if (elems.count() != 2 || (group != "printers" && group != "classes" && group != "specials")
	    || (mprinter = KMFactory::self()->manager()->findPrinter(printer)) == 0)
	{
		error(KIO::ERR_DOES_NOT_EXIST, url.path());
	}
	else if (group == "printers" && mprinter->isPrinter())
		showPrinterInfo(mprinter);
	else if (group == "classes" && mprinter->isClass(true))
		showClassInfo(mprinter);
	else if (group == "specials" && mprinter->isSpecial())
		showSpecialInfo(mprinter);
	else
		error(KIO::ERR_INTERNAL, i18n("Can't determine printer type"));
}

void KIO_Print::showPrinterInfo(KMPrinter *printer)
{
	if (!KMFactory::self()->manager()->completePrinter(printer))
		error(KIO::ERR_INTERNAL, i18n("Unable to retrieve class informations for %1.").arg(printer->name()));
	else
	{
		mimeType("text/html");

		QString	content;
		if (!loadTemplate(QString::fromLatin1("printer.template"), content))
		{
			error(KIO::ERR_INTERNAL, i18n("Unable to load template %1").arg("printer.template"));
			return;
		}

		content = content.arg(printer->name())
				 .arg(printer->isRemote() ? i18n("Remote") : i18n("Local"))
				 .arg(printer->stateString())
				 .arg(printer->location())
				 .arg(printer->description())
				 .arg(printer->uri().prettyURL())
				 .arg(printer->manufacturer())
				 .arg(printer->model())
				 .arg(printer->driverInfo())
				 .arg(printer->device().prettyURL());

		data(content.local8Bit());
		finished();
	}
}

void KIO_Print::showClassInfo(KMPrinter *printer)
{
	if (!KMFactory::self()->manager()->completePrinter(printer))
		error(KIO::ERR_INTERNAL, i18n("Unable to retrieve class informations for %1.").arg(printer->name()));
	else
	{
		mimeType("text/html");

		QString	content;
		if (!loadTemplate(QString::fromLatin1("class.template"), content))
		{
			error(KIO::ERR_INTERNAL, i18n("Unable to load template %1").arg("class.template"));
			return;
		}

		QString		memberContent;
		QStringList	members(printer->members());
		for (QStringList::ConstIterator it=members.begin(); it!=members.end(); ++it)
		{
			memberContent.append(QString::fromLatin1("<LI><A href=\"print:/printers/%1\">%2</A></LI>\n").arg(*it).arg(*it));
		}

		QString		typeContent = (printer->isImplicit() ? i18n("Implicit") : (printer->isRemote() ? i18n("Remote") : i18n("Local")));

		content = content.arg(printer->name())
				 .arg(typeContent)
				 .arg(printer->stateString())
				 .arg(printer->location())
				 .arg(printer->description())
				 .arg(printer->uri().prettyURL())
				 .arg(memberContent);

		data(content.local8Bit());
		finished();
	}
}

void KIO_Print::showSpecialInfo(KMPrinter *printer)
{
	mimeType("text/html");
	QCString	myContent;
	myContent.sprintf("<HTML>\n"
			  "<BODY>%s: %s</BODY>\n"
			  "</HTML>\n", i18n("Special printer").local8Bit().data(), printer->name().latin1());
	data(myContent);
	finished();
}

bool KIO_Print::loadTemplate(const QString& filename, QString& buffer)
{
	QFile	f(locate("data", QString::fromLatin1("kdeprint/template/")+filename));
	if (f.exists() && f.open(IO_ReadOnly))
	{
		QTextStream	t(&f);
		buffer = t.read();
		return true;
	}
	else
	{
		buffer = QString::null;
		return false;
	}
}
