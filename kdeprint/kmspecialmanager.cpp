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

#include "kmspecialmanager.h"
#include "kmmanager.h"
#include "kmprinter.h"

#include <qdom.h>
#include <qfile.h>
#include <qtextstream.h>
#include <kstddirs.h>

KMSpecialManager::KMSpecialManager(KMManager *parent, const char *name)
: QObject(parent,name), m_mgr(parent), m_loaded(false)
{
}

bool KMSpecialManager::savePrinters()
{
	QFile	f(locate("data","kdeprint/specials.xml"));
	QDomDocument	doc;
	if (!f.open(IO_ReadOnly))
		return false;
	doc.setContent(&f);
	f.close();
	f.setName(locateLocal("data","kdeprint/specials.xml"));

	if (f.open(IO_WriteOnly))
	{
		QTextStream	t(&f);
		QDomElement	root = doc.documentElement();
		root.clear();
		QListIterator<KMPrinter>	it(m_mgr->m_printers);
		for (;it.current();++it)
		{
			if (!it.current()->isSpecial()) continue;
			KMPrinter	*printer = it.current();
			QDomElement	elem = doc.createElement("Printer");
			elem.setAttribute("name",printer->name());
			elem.setAttribute("desc",printer->description());
			elem.setAttribute("command",printer->option("kde-specical-command"));
			elem.setAttribute("file",printer->option("kde-special-file"));
			elem.setAttribute("icon",printer->pixmap());
			root.appendChild(elem);
		}
		t << doc;
		return true;
	}
	else
		return false;
}

bool KMSpecialManager::loadPrinters()
{
	if (m_loaded) return true;

	QFile	f(locate("data","kdeprint/specials.xml"));
	if (f.exists() && f.open(IO_ReadOnly))
	{
		QDomDocument	doc;
		doc.setContent(&f);
		QDomNode	node = doc.documentElement().firstChild();
		while (!node.isNull())
		{
			QDomElement	elem = node.toElement();
			if (!elem.isNull() && elem.tagName() == "Printer")
			{
				KMPrinter	*printer = new KMPrinter;
				printer->setName(elem.attribute("name"));
				printer->setPrinterName(printer->name());
				printer->setDescription(elem.attribute("desc"));
				printer->setOption("kde-special-command",elem.attribute("command"));
				printer->setOption("kde-special-file",elem.attribute("file","0"));
				printer->setPixmap(elem.attribute("icon","unknown"));
				printer->setType(KMPrinter::Special);
				printer->setState(KMPrinter::Idle);
				m_mgr->addPrinter(printer);
			}
			node = node.nextSibling();
		}
		m_loaded = true;
		return true;
	}
	else
		return false;
}

void KMSpecialManager::refresh()
{
	if (!m_loaded)
		loadPrinters();
	else
	{
		QListIterator<KMPrinter>	it(m_mgr->m_printers);
		for (;it.current();++it)
			if (it.current()->isSpecial())
				it.current()->setDiscarded(false);
	}
}
