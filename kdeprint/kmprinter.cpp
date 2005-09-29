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

#include "kmprinter.h"
#include "kprinter.h"
#include "driver.h"

#include <klocale.h>
#include <kfiledialog.h>

KMPrinter::KMPrinter()
: KMObject()
{
	m_type = KMPrinter::Printer;
	m_dbentry = 0;
	m_harddefault = m_softdefault = m_ownsoftdefault = false;
	m_driver = 0;
	m_isedited = false;
	m_printercap = 0;
}

KMPrinter::KMPrinter(const KMPrinter& p)
: KMObject()
{
	m_driver = 0;	// don't copy driver structure
	m_harddefault = m_softdefault = m_ownsoftdefault = false;
	m_isedited = false;
	copy(p);
}

KMPrinter::~KMPrinter()
{
	delete m_driver;
}

void KMPrinter::copy(const KMPrinter& p)
{
	m_name = p.m_name;
	m_printername = p.m_printername;
	m_instancename = p.m_instancename;
	m_type = p.m_type;
	m_state = p.m_state;
	m_device = p.m_device;
	m_members = p.m_members;
	m_description = p.m_description;
	m_location = p.m_location;
	m_manufacturer = p.m_manufacturer;
	m_model = p.m_model;
	m_uri = p.m_uri;
	m_driverinfo = p.m_driverinfo;
	m_dbentry = p.m_dbentry;
	m_pixmap = p.m_pixmap;
	//m_harddefault = p.m_harddefault;
	//m_softdefault = p.m_softdefault;
	m_options = p.m_options;
	m_printercap = p.m_printercap;
	setDiscarded(false);
}

void KMPrinter::setDriver(DrMain *driver)
{
	delete m_driver;
	m_driver = driver;
}

DrMain* KMPrinter::takeDriver()
{
	DrMain	*dr = m_driver;
	m_driver = 0;
	return dr;
}

QString KMPrinter::pixmap()
{
	if (!m_pixmap.isEmpty()) return m_pixmap;

	QString	str("kdeprint_printer");
	if (!isValid()) str.append("_defect");
	else
	{
		//if (isHardDefault()) str.append("_default");
		if (isClass(true)) str.append("_class");
		else if (isRemote()) str.append("_remote");
		switch (state())
		{
			case KMPrinter::Stopped:
				str.append("_stopped");
				break;
			case KMPrinter::Processing:
				str.append("_process");
				break;
			default:
				break;
		}
	}
	return str;
}

int KMPrinter::compare(KMPrinter *p1, KMPrinter *p2)
{
	if (p1 && p2)
	{
		bool	s1(p1->isSpecial()), s2(p2->isSpecial());
		if (s1 && s2) return QString::compare(p1->name(),p2->name());
		else if (s1) return 1;
		else if (s2) return -1;
		else
		{
			bool	c1(p1->isClass(false)), c2(p2->isClass(false));
			if (c1 == c2) return QString::compare(p1->name(),p2->name());
			else if (c1 && !c2) return -1;
			else if (!c1 && c2) return 1;
		}
	}
	return 0;
}

QString KMPrinter::stateString() const
{
	QString	s;
	switch (state())
	{
		case KMPrinter::Idle: s = i18n("Idle"); break;
		case KMPrinter::Processing: s = i18n("Processing..."); break;
		case KMPrinter::Stopped: s = i18n("Stopped"); break;
		default: return i18n("Unknown State", "Unknown");
	}
	s += (" " + (m_state & Rejecting ? i18n("(rejecting jobs)") : i18n("(accepting jobs)")));
	return s;
}

bool KMPrinter::autoConfigure(KPrinter *printer, QWidget *parent)
{
	// standard settings
	printer->setPrinterName(printerName());
	printer->setSearchName(name());
	// printer default settings (useful for instances)
	printer->setOptions(defaultOptions());
	// special printer case:
	//	- add command
	//	- ask for output file (if needed) using default extension.
	if (isSpecial())
	{
		if (option("kde-special-file") == "1")
		{
			// build-up default filename/directory
			QString fName = printer->docFileName(), ext = option( "kde-special-extension" );
			if ( fName.isEmpty() )
				fName = ( printer->docName() + "." + ext );
			else
			{
				int p = fName.findRev( '.' );
				if ( p == -1 )
					fName.append( "." ).append( ext );
				else
				{
					fName.truncate( p+1 );
					fName.append( ext );
				}
			}
			fName.prepend( "/" ).prepend( printer->docDirectory() );

			// build-up file dialog
			KFileDialog *dialog = new KFileDialog (fName,
								QString::null,
								parent,
								"filedialog",
								true);
			dialog->setOperationMode (KFileDialog::Saving);

			QString	mimetype = option("kde-special-mimetype");

			if (!mimetype.isEmpty())
			{
				QStringList filter;
				filter << mimetype;
				filter << "all/allfiles";
				dialog->setMimeFilter (filter, mimetype);
			}
			else if (!ext.isEmpty())
				dialog->setFilter ("*." + ext + "\n*|" + i18n ("All Files"));

			if (dialog->exec ())
			{
				printer->setOutputToFile(true);
				printer->setOutputFileName(dialog->selectedFile ());
			}
			else
			{
				// action canceled
				return false;
			}
		}
		printer->setOption( "kde-isspecial", "1" );
		printer->setOption( "kde-special-command", option( "kde-special-command" ) );
	}

	return true;
}

QString KMPrinter::deviceProtocol() const
{
	int p = m_device.find( ':' );
	if ( p != -1 )
		return m_device.left( p );
	else
		return QString::null;
}
