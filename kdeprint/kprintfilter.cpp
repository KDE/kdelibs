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

#include "kprintfilter.h"
#include "driver.h"
#include "driverview.h"
#include "kxmlcommand.h"

#include <kstandarddirs.h>
#include <kdialogbase.h>
#include <ksimpleconfig.h>

struct KPrintFilter::KPrintFilterPrivate
{
	QString		m_description;
	KXmlCommand	*m_xmlcmd;
	bool		m_read;
	QStringList	m_mimetypein;
	QString		m_mimetypeout;
	QStringList	m_requirements;
};

//*****************************************************************************************************

KPrintFilter::KPrintFilter(const QString& idname, QObject *parent, const char *name)
: QObject(parent, name)
{
	d = new KPrintFilterPrivate;
	d->m_xmlcmd = 0;
	d->m_read = false;
	m_idname = idname;
	readDesktopFile();
}

KPrintFilter::~KPrintFilter()
{
	clean();
	delete d;
}

void KPrintFilter::clean()
{
	delete d->m_xmlcmd;
	d->m_xmlcmd = 0;
}

bool KPrintFilter::readDesktopFile()
{
	KSimpleConfig	config(locate("data",QString::fromLatin1("kdeprint/filters/%1.desktop").arg(m_idname)));
	config.setGroup("KDE Print Filter Entry");
	d->m_description = config.readEntry("Comment", m_idname);
	d->m_requirements = config.readListEntry("Require");
	d->m_mimetypein = config.readListEntry("MimeTypeIn");
	d->m_mimetypeout = config.readEntry("MimeTypeOut");
	return true;
}

bool KPrintFilter::readXmlTemplate()
{
	if (!d->m_read || !d->m_xmlcmd)
	{
		clean();

		QString	cpath = locate("data",QString::fromLatin1("kdeprint/filters/%1.xml").arg(m_idname));
		if (!cpath.isEmpty())
		{
			d->m_xmlcmd = new KXmlCommand(cpath);
			d->m_read = true;
		}
		else
			return false;
	}
	return (d->m_xmlcmd != 0);
}

QString KPrintFilter::buildCommand(const QMap<QString,QString>& options, bool pipein, bool pipeout)
{
	QString	cmd;
	if (readXmlTemplate())
	{
		cmd = d->m_xmlcmd->buildCommand(options, pipein, pipeout);
	}
	return cmd;
}

void KPrintFilter::setOptions(const QMap<QString,QString>& opts)
{
	if (readXmlTemplate())
		d->m_xmlcmd->setOptions(opts);
}

void KPrintFilter::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	if (readXmlTemplate())
		d->m_xmlcmd->getOptions(opts, incldef);
}

bool KPrintFilter::configure(QWidget *parent)
{
	if (readXmlTemplate())
	{
		KDialogBase	dlg(parent, 0, true, d->m_description, KDialogBase::Ok);
		DriverView	view(&dlg);

		dlg.setMainWidget(&view);
		view.setDriver(d->m_xmlcmd->driver());
		dlg.resize(350,400);
		dlg.exec();

		return true;
	}
	return false;
}

QString KPrintFilter::description() const
{
	return d->m_description;
}

QString KPrintFilter::mimeType() const
{
	return d->m_mimetypeout;
}

bool KPrintFilter::acceptMimeType(const QString& mimetype)
{
	return (d->m_mimetypein.find(mimetype) != d->m_mimetypein.end());
}

QStringList KPrintFilter::inputMimeTypes() const
{
	return d->m_mimetypein;
}

QStringList KPrintFilter::requirements() const
{
	return d->m_requirements;
}
