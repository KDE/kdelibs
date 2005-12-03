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

#include "kmwrlpr.h"
#include "kmprinter.h"
#include "kmwizard.h"

#include <qdir.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <q3header.h>
#include <klistview.h>
#include <klocale.h>
#include <kiconloader.h>

static Q3ListViewItem* findItem(Q3ListViewItem *c, const QString& txt)
{
	Q3ListViewItem	*item(c);
	while (item)
		if (item->text(0) == txt) return item;
		else item = item->nextSibling();
	return NULL;
}

//*****************************************************************************************************

KMWRlpr::KMWRlpr(QWidget *parent)
    : KMWizardPage(parent)
{
	m_ID = KMWizard::Custom+1;
	m_title = i18n("Remote LPD Queue Settings");
	m_nextpage = KMWizard::Name;

	m_view = new KListView(this);
	m_view->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Sunken);
	m_view->setLineWidth(1);
	m_view->addColumn(QLatin1String(""));
	m_view->header()->hide();
	m_view->setRootIsDecorated(true);
	m_view->setSorting(0);
	m_host = new QLineEdit(this);
	m_queue = new QLineEdit(this);
	QLabel	*m_hostlabel = new QLabel(i18n("Host:"), this);
	QLabel	*m_queuelabel = new QLabel(i18n("Queue:"), this);
	m_hostlabel->setBuddy(m_host);
	m_queuelabel->setBuddy(m_queue);
	connect(m_view,SIGNAL(selectionChanged(Q3ListViewItem*)),SLOT(slotPrinterSelected(Q3ListViewItem*)));

	QHBoxLayout	*lay0 = new QHBoxLayout(this, 0, 10);
	QVBoxLayout	*lay1 = new QVBoxLayout(0, 0, 5);
	lay0->addWidget(m_view,1);
	lay0->addLayout(lay1,1);
	lay1->addWidget(m_hostlabel);
	lay1->addWidget(m_host);
	lay1->addSpacing(20);
	lay1->addWidget(m_queuelabel);
	lay1->addWidget(m_queue);
	lay1->addStretch(1);

	initialize();
}

bool KMWRlpr::isValid(QString& msg)
{
	if (m_host->text().isEmpty())
		msg = i18n("Empty host name.");
	else if (m_queue->text().isEmpty())
		msg = i18n("Empty queue name.");
	else
		return true;
	return false;
}

void KMWRlpr::initPrinter(KMPrinter *p)
{
	m_host->setText(p->option("host"));
	m_queue->setText(p->option("queue"));
	Q3ListViewItem	*item = findItem(m_view->firstChild(),m_host->text());
	if (item)
	{
		item = findItem(item->firstChild(),m_queue->text());
		if (item)
		{
			item->parent()->setOpen(true);
			m_view->setCurrentItem(item);
			m_view->ensureItemVisible(item);
		}
	}
}

void KMWRlpr::updatePrinter(KMPrinter *p)
{
	QString	uri = QString::fromLatin1("lpd://%1/%2").arg(m_host->text()).arg(m_queue->text());
	p->setDevice(uri);
	p->setOption("host",m_host->text());
	p->setOption("queue",m_queue->text());
	p->setOption("kde-backend-description",i18n("Remote LPD queue"));
	// setting default name and description, but only if the "name()" is empty
	// which is the case on first pass when adding a printer. This prevents from
	// bad side-effects when simply modifying an existing printer.
	if (p->name().isEmpty())
	{
		p->setName(m_queue->text());
		p->setPrinterName(p->name());
		p->setDescription(i18n("Remote queue %1 on %2").arg(m_queue->text()).arg(m_host->text()));
	}
}

void KMWRlpr::initialize()
{
	m_view->clear();
	QFile	f(QDir::homePath()+"/.rlprrc");
	if (!f.exists()) f.setName("/etc/rlprrc");
	if (f.exists() && f.open(QIODevice::ReadOnly))
	{
		QTextStream	t(&f);
		QString		line, host;
		int 		p(-1);
		while (!t.atEnd())
		{
			line = t.readLine().trimmed();
			if (line.isEmpty())
				continue;
			if ((p=line.find(':')) != -1)
			{
				host = line.left(p).trimmed();
				Q3ListViewItem	*hitem = new Q3ListViewItem(m_view,host);
				hitem->setPixmap(0,SmallIcon("kdeprint_computer"));
				QStringList	prs = QStringList::split(' ',line.right(line.length()-p-1),false);
				for (QStringList::ConstIterator it=prs.begin(); it!=prs.end(); ++it)
				{
					Q3ListViewItem	*pitem = new Q3ListViewItem(hitem,*it);
					pitem->setPixmap(0,SmallIcon("kdeprint_printer"));
				}
			}
		}
		f.close();
	}

	// parse printcap file for local printers
	f.setName("/etc/printcap");
	if (f.exists() && f.open(QIODevice::ReadOnly))
	{
		QTextStream	t(&f);
		QString		line, buffer;
		Q3ListViewItem	*hitem(m_view->firstChild());
		while (hitem) if (hitem->text(0) == "localhost") break; else hitem = hitem->nextSibling();
		while (!t.atEnd())
		{
			buffer = QString::null;
			while (!t.atEnd())
			{
				line = t.readLine().trimmed();
				if (line.isEmpty() || line[0] == '#')
					continue;
				buffer.append(line);
				if (buffer.right(1) == "\\")
					buffer = buffer.left(buffer.length()-1).trimmed();
				else
					break;
			}
			if (buffer.isEmpty())
				continue;
			int	p = buffer.find(':');
			if (p != -1)
			{
				QString	name = buffer.left(p);
				if (!hitem)
				{
					hitem = new Q3ListViewItem(m_view,"localhost");
					hitem->setPixmap(0,SmallIcon("kdeprint_computer"));
				}
				Q3ListViewItem	*pitem = new Q3ListViewItem(hitem,name);
				pitem->setPixmap(0,SmallIcon("kdeprint_printer"));
			}
		}
	}

	if (m_view->childCount() == 0)
		new Q3ListViewItem(m_view,i18n("No Predefined Printers"));
}

void KMWRlpr::slotPrinterSelected(Q3ListViewItem *item)
{
	if (item && item->depth() == 1)
	{
		m_host->setText(item->parent()->text(0));
		m_queue->setText(item->text(0));
	}
}
#include "kmwrlpr.moc"
