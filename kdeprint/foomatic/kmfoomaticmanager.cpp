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

#include "kmfoomaticmanager.h"
#include "kpipeprocess.h"
#include "driver.h"

#include <qdom.h>
#include <klocale.h>
#include <kdebug.h>
#include <kprocess.h>

#include <unistd.h>

KMFoomaticManager::KMFoomaticManager(QObject *parent, const char *name)
: KMManager(parent,name)
{
	setHasManagement(getuid() == 0);
	setPrinterOperationMask(KMManager::PrinterConfigure);
}

KMFoomaticManager::~KMFoomaticManager()
{
}

void KMFoomaticManager::listPrinters()
{
	KPipeProcess	proc("foomatic-configure -Q -q -r");
	QDomDocument	doc;

	doc.setContent(&proc);
	QDomElement	docElem = doc.documentElement();
	if (docElem.isNull() || docElem.tagName() != "queues")
		return;

	QDomNode	queueNode = docElem.firstChild();
	while (!queueNode.isNull())
	{
		QDomElement	queueElem = queueNode.toElement();
		if (!queueElem.isNull() && queueElem.tagName() == "queue")
		{
			KMPrinter	*printer = createPrinterFromElement(&queueElem);
			if (printer)
				addPrinter(printer);
		}
		queueNode = queueNode.nextSibling();
	}
}

DrMain* KMFoomaticManager::loadPrinterDriver(KMPrinter *printer, bool)
{
	if (printer->option("foomatic") != "1")
	{
		setErrorMsg(i18n("This is not a Foomatic printer"));
		return NULL;
	}
	else if (printer->option("driver").isEmpty() || printer->option("printer").isEmpty())
	{
		setErrorMsg(i18n("Some printer information are missing"));
		return NULL;
	}

	QString	cmd = "foomatic-combo-xml -p ";
	cmd += KProcess::quote(printer->option("printer"));
	cmd += " -d ";
	cmd += KProcess::quote(printer->option("driver"));
	KPipeProcess	proc(cmd);
	QDomDocument	doc;
	doc.setContent(&proc);
	QDomElement	docElem = doc.documentElement();
	return createDriverFromXML(&docElem);
}

KMPrinter* KMFoomaticManager::createPrinterFromElement(QDomElement *elem)
{
	QDomElement	e = elem->namedItem("name").toElement();
	if (!e.isNull())
	{
		KMPrinter	*printer = new KMPrinter;
		printer->setType(KMPrinter::Printer);
		printer->setName(e.text());
		printer->setPrinterName(e.text());
		printer->setState(KMPrinter::Idle);
		/*if (printer->name().find('/') != -1)
		{
			QString	s(printer->name());
			int	p = s.find('/');
			printer->setPrinterName(s.left(p));
			printer->setInstanceName(s.mid(p+1));
			printer->addType(KMPrinter::Virtual);
		}*/

		if (!(e=elem->namedItem("description").toElement()).isNull())
			printer->setDescription(e.text());
		if (!(e=elem->namedItem("location").toElement()).isNull())
			printer->setLocation(e.text());
		if (!(e=elem->namedItem("connect").toElement()).isNull())
			printer->setDevice(e.text());

		printer->setOption("foomatic", elem->attribute("foomatic"));
		printer->setOption("spooler", elem->attribute("spooler"));
		if (elem->attribute("foomatic") == "1")
		{
			if (!(e=elem->namedItem("printer").toElement()).isNull())
				printer->setOption("printer", e.text());
			if (!(e=elem->namedItem("driver").toElement()).isNull())
				printer->setOption("driver", e.text());
		}

		return printer;
	}
	return NULL;
}

DrMain* KMFoomaticManager::createDriverFromXML(QDomElement *elem)
{
	DrMain	*driver = new DrMain();
	QDomElement	pelem = elem->namedItem("printer").toElement(), delem = elem->namedItem("driver").toElement();
	if (!pelem.isNull() && !delem.isNull())
	{
		driver->set("manufacturer", pelem.namedItem("make").toElement().text());
		driver->set("model", pelem.namedItem("model").toElement().text());
		QString	s = QString::fromLatin1("%1 %2 (%3)").arg(driver->get("manufacturer")).arg(driver->get("model")).arg(delem.namedItem("name").toElement().text());
		driver->set("description", s);
		driver->set("text", s);

		QDomElement	opts = elem->namedItem("options").toElement();
		if (!opts.isNull())
		{
			QDomElement	o = opts.firstChild().toElement();
			while (!o.isNull())
			{
				if (o.tagName() == "option")
				{
					QString	type = o.attribute("type");
					DrBase *dropt(0);

					if (type == "bool" || type == "enum")
					{
						if (type == "bool") dropt = new DrBooleanOption();
						else dropt = new DrListOption();
						QString	defval = o.namedItem("arg_defval").toElement().text(), valuetext;
						QDomNode	val = o.namedItem("enum_vals").firstChild();
						while (!val.isNull())
						{
							DrBase	*choice = new DrBase();
							choice->setName(val.namedItem("ev_shortname").namedItem("en").toElement().text());
							choice->set("text", i18n(val.namedItem("ev_longname").namedItem("en").toElement().text().latin1()));
							static_cast<DrListOption*>(dropt)->addChoice(choice);
							if (val.toElement().attribute("id") == defval)
								valuetext = choice->name();

							val = val.nextSibling();
						}
						dropt->set("default", valuetext);
						dropt->setValueText(valuetext);
					}
					else if (type == "int" || type == "float")
					{
						if (type == "int") dropt = new DrIntegerOption();
						else dropt = new DrFloatOption();
						dropt->set("minval", o.namedItem("arg_min").toElement().text());
						dropt->set("maxval", o.namedItem("arg_max").toElement().text());
						QString	defval = o.namedItem("arg_defval").toElement().text();
						dropt->set("default", defval);
						dropt->setValueText(defval);
					}

					if (dropt)
					{
						dropt->setName(o.namedItem("arg_shortname").namedItem("en").toElement().text());
						dropt->set("text", i18n(o.namedItem("arg_longname").namedItem("en").toElement().text().latin1()));
						driver->addOption(dropt);
					}
				}
				o = o.nextSibling().toElement();
			}
		}
	}
	return driver;
}
