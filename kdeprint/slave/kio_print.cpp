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
#include <kiconloader.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define	PRINT_DEBUG	kdDebug(7019) << "kio_print: "

extern "C"
{
	int kdemain(int argc, char **argv);
}

void addAtom(KIO::UDSEntry& entry, unsigned int ID, long l, const QString& s = QString::null)
{
	KIO::UDSAtom	atom;
	atom.m_uds = ID;
	atom.m_long = l;
	atom.m_str = s;
	entry.append(atom);
}

int kdemain(int argc, char **argv)
{
	KLocale::setMainCatalogue("kdelibs");
	KInstance	instance("kio_print");
        ( void ) KGlobal::locale();


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
			addAtom(entry, KIO::UDS_NAME, 0, it.current()->name());
			addAtom(entry, KIO::UDS_FILE_TYPE, S_IFREG);
			addAtom(entry, KIO::UDS_URL, 0, QString::fromLatin1("print:%1/%2").arg(path).arg(it.current()->name()));
			addAtom(entry, KIO::UDS_ACCESS, S_IRWXO | S_IRWXG | S_IRWXU);
			addAtom(entry, KIO::UDS_MIME_TYPE, 0, mimeType);
			addAtom(entry, KIO::UDS_GUESSED_MIME_TYPE, 0, QString::fromLatin1("text/html"));

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

	KIO::UDSEntry	entry;

	// Classes entry
	createRootEntry(entry);
	addAtom(entry, KIO::UDS_NAME, 0, i18n("Classes"));
	addAtom(entry, KIO::UDS_URL, 0, QString::fromLatin1("print:/classes/"));
	listEntry(entry, false);

	// Printers entry
	createRootEntry(entry);
	addAtom(entry, KIO::UDS_NAME, 0, i18n("Printers"));
	addAtom(entry, KIO::UDS_URL, 0, QString::fromLatin1("print:/printers/"));
	listEntry(entry, false);

	// Specials entry
	createRootEntry(entry);
	addAtom(entry, KIO::UDS_NAME, 0, i18n("Specials"));
	addAtom(entry, KIO::UDS_URL, 0, QString::fromLatin1("print:/specials/"));
	listEntry(entry, false);

	// Management entry
	entry.clear();
	addAtom(entry, KIO::UDS_FILE_TYPE, S_IFREG);
	addAtom(entry, KIO::UDS_ACCESS, S_IRUSR | S_IRGRP | S_IROTH);
	addAtom(entry, KIO::UDS_MIME_TYPE, 0, QString::fromLatin1("print/manager"));
	addAtom(entry, KIO::UDS_NAME, 0, i18n("Manager"));
	addAtom(entry, KIO::UDS_URL, 0, QString::fromLatin1("print:/manager"));
	listEntry(entry, false);

	// finish
	listEntry(entry, true);
	finished();
}

void KIO_Print::createRootEntry(KIO::UDSEntry& entry)
{
	entry.clear();
	addAtom(entry, KIO::UDS_FILE_TYPE, S_IFDIR);
	addAtom(entry, KIO::UDS_ACCESS, S_IRWXO | S_IRWXG | S_IRWXU);
	addAtom(entry, KIO::UDS_MIME_TYPE, 0, QString::fromLatin1("print/folder"));
	addAtom(entry, KIO::UDS_GUESSED_MIME_TYPE, 0, QString::fromLatin1("inode/directory"));
}

void KIO_Print::get(const KURL& url)
{
	QStringList	elems = QStringList::split('/', url.path(), false);
	QString		group(elems[0]), printer(elems[1]);
	KMPrinter	*mprinter(0);

	if (group == "manager")
	{
		PRINT_DEBUG << "opening print management part" << endl;

		mimeType("print/manager");
		finished();
		return;
	}

	PRINT_DEBUG << "opening " << url.url() << endl;

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

		content = content.arg(QString::fromLatin1("file:")+KGlobal::iconLoader()->iconPath("print_printer", KIcon::Desktop))
				 .arg(printer->name())
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
