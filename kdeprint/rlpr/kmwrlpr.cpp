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

#include "kmwrlpr.h"
#include "kmprinter.h"
#include "kmwizard.h"

#include <qdir.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qheader.h>
#include <klistview.h>
#include <klocale.h>
#include <kiconloader.h>

static QListViewItem* findChild(QListViewItem *c, const QString& txt)
{
	QListViewItem	*item(c);
	while (item)
		if (item->text(0) == txt) return item;
		else item = item->nextSibling();
	return NULL;
}

//*****************************************************************************************************

KMWRlpr::KMWRlpr(QWidget *parent, const char *name)
: KMWizardPage(parent,name)
{
	m_ID = KMWizard::Custom+1;
	m_title = i18n("Remote LPD Queue Settings");
	m_nextpage = KMWizard::Name;

	m_view = new KListView(this);
	m_view->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	m_view->setLineWidth(1);
	m_view->addColumn(QString::fromLatin1(""));
	m_view->header()->hide();
	m_view->setRootIsDecorated(true);
	m_view->setSorting(0);
	m_host = new QLineEdit(this);
	m_queue = new QLineEdit(this);
	QLabel	*m_hostlabel = new QLabel(i18n("Host:"), this);
	QLabel	*m_queuelabel = new QLabel(i18n("Queue:"), this);
	m_hostlabel->setBuddy(m_host);
	m_queuelabel->setBuddy(m_queue);
	connect(m_view,SIGNAL(selectionChanged(QListViewItem*)),SLOT(slotPrinterSelected(QListViewItem*)));

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
	QListViewItem	*item = findChild(m_view->firstChild(),m_host->text());
	if (item)
	{
		item = findChild(item->firstChild(),m_queue->text());
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
	p->setDevice(KURL(uri));
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
	QFile	f(QDir::homeDirPath()+"/.rlprrc");
	if (!f.exists()) f.setName("/etc/rlprrc");
	if (f.exists() && f.open(IO_ReadOnly))
	{
		QTextStream	t(&f);
		QString		line, host;
		int 		p(-1);
		while (!t.eof())
		{
			line = t.readLine().stripWhiteSpace();
			if (line.isEmpty())
				continue;
			if ((p=line.find(':')) != -1)
			{
				host = line.left(p).stripWhiteSpace();
				QListViewItem	*hitem = new QListViewItem(m_view,host);
				hitem->setPixmap(0,SmallIcon("kdeprint_computer"));
				QStringList	prs = QStringList::split(' ',line.right(line.length()-p-1),false);
				for (QStringList::ConstIterator it=prs.begin(); it!=prs.end(); ++it)
				{
					QListViewItem	*pitem = new QListViewItem(hitem,*it);
					pitem->setPixmap(0,SmallIcon("kdeprint_printer"));
				}
			}
		}
		f.close();
	}

	// parse printcap file for local printers
	f.setName("/etc/printcap");
	if (f.exists() && f.open(IO_ReadOnly))
	{
		QTextStream	t(&f);
		QString		line, buffer;
		QListViewItem	*hitem(m_view->firstChild());
		while (hitem) if (hitem->text(0) == "localhost") break; else hitem = hitem->nextSibling();
		while (!t.eof())
		{
			buffer = QString::null;
			while (!t.eof())
			{
				line = t.readLine().stripWhiteSpace();
				if (line.isEmpty() || line[0] == '#')
					continue;
				buffer.append(line);
				if (buffer.right(1) == "\\")
					buffer = buffer.left(buffer.length()-1).stripWhiteSpace();
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
					hitem = new QListViewItem(m_view,"localhost");
					hitem->setPixmap(0,SmallIcon("kdeprint_computer"));
				}
				QListViewItem	*pitem = new QListViewItem(hitem,name);
				pitem->setPixmap(0,SmallIcon("kdeprint_printer"));
			}
		}
	}

	if (m_view->childCount() == 0)
		new QListViewItem(m_view,i18n("No predefined printers"));
}

void KMWRlpr::slotPrinterSelected(QListViewItem *item)
{
	if (item && item->depth() == 1)
	{
		m_host->setText(item->parent()->text(0));
		m_queue->setText(item->text(0));
	}
}
#include "kmwrlpr.moc"
