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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "kmwlocal.h"
#include "kmwizard.h"
#include "kmprinter.h"
#include "kmfactory.h"
#include "kmmanager.h"

#include <klocale.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qheader.h>
#include <klistview.h>
#include <kmessagebox.h>
#include <kiconloader.h>

KMWLocal::KMWLocal(QWidget *parent, const char *name)
: KMWizardPage(parent,name)
{
	m_title = i18n("Local Port Selection");
	m_ID = KMWizard::Local;
	m_nextpage = KMWizard::Driver;
	m_initialized = false;
	m_block = false;

	m_ports = new KListView(this);
	m_ports->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	m_ports->setLineWidth(1);
	m_ports->header()->hide();
	m_ports->addColumn("");
	m_ports->setSorting(-1);
	QListViewItem	*root = new QListViewItem(m_ports, i18n("Local System"));
	root->setPixmap(0, SmallIcon("kdeprint_computer"));
	root->setOpen(true);
	connect(m_ports, SIGNAL(selectionChanged(QListViewItem*)), SLOT(slotPortSelected(QListViewItem*)));
	QLabel	*l1 = new QLabel(i18n("URI:"), this);
	m_localuri = new QLineEdit(this);
	connect( m_localuri, SIGNAL( textChanged( const QString& ) ), SLOT( slotTextChanged( const QString& ) ) );
	m_parents[0] = new QListViewItem(root, i18n("Parallel"));
	m_parents[1] = new QListViewItem(root, m_parents[0], i18n("Serial"));
	m_parents[2] = new QListViewItem(root, m_parents[1], i18n("USB"));
	m_parents[3] = new QListViewItem(root, m_parents[2], i18n("Others"));
	for (int i=0;i<4;i++)
		m_parents[i]->setPixmap(0, SmallIcon("input_devices_settings"));
	QLabel	*l2 = new QLabel(i18n("<p>Select a valid detected port, or enter directly the corresponding URI in the bottom edit field.</p>"), this);

	QVBoxLayout	*lay0 = new QVBoxLayout(this, 0, 10);
	QHBoxLayout	*lay1 = new QHBoxLayout(0, 0, 10);
	lay0->addWidget(l2, 0);
	lay0->addWidget(m_ports, 1);
	lay0->addLayout(lay1, 0);
	lay1->addWidget(l1, 0);
	lay1->addWidget(m_localuri, 1);
}

bool KMWLocal::isValid(QString& msg)
{
	if (m_localuri->text().isEmpty())
	{
		msg = i18n("Empty URI.");
		return false;
	}
	else if (m_uris.findIndex(m_localuri->text()) == -1)
	{
		if (KMessageBox::warningYesNo(this, i18n("The local URI doesn't correspond to a detected port. Continue?")) == KMessageBox::No)
		{
			msg = i18n("Select a valid port.");
			return false;
		}
	}
	return true;
}

void KMWLocal::slotPortSelected(QListViewItem *item)
{
	if ( m_block )
		return;

	QString uri;
	if (!item || item->depth() <= 1 || item->depth() > 3)
		uri = QString::null;
	else if (item->depth() == 3)
		uri = item->parent()->text( 1 );
	else
		uri = item->text( 1 );
	m_block = true;
	m_localuri->setText( uri );
	m_block = false;
}

void KMWLocal::updatePrinter(KMPrinter *printer)
{
	QListViewItem *item = m_ports->selectedItem();
	if ( item && item->depth() == 3 )
		printer->setOption( "kde-autodetect", item->text( 0 ) );
	printer->setDevice(m_localuri->text());
}

void KMWLocal::initPrinter(KMPrinter *printer)
{
	if (!m_initialized)
		initialize();

	if (printer)
	{
		m_localuri->setText(printer->device());
	}
}

QListViewItem* KMWLocal::lookForItem( const QString& uri )
{
	for ( int i=0; i<4; i++ )
	{
		QListViewItem *item = m_parents[ i ]->firstChild();
		while ( item )
			if ( item->text( 1 ) == uri )
				if ( item->firstChild() )
					return item->firstChild();
				else
					return item;
			else
				item = item->nextSibling();
	}
	return 0;
}

void KMWLocal::slotTextChanged( const QString& txt )
{
	if ( m_block )
		return;

	QListViewItem *item = lookForItem( txt );
	if ( item )
	{
		m_block = true;
		m_ports->setSelected( item, true );
		m_block = false;
	}
	else
		m_ports->clearSelection();
}

void KMWLocal::initialize()
{
	QStringList	list = KMFactory::self()->manager()->detectLocalPrinters();
	if (list.isEmpty() || (list.count() % 4) != 0)
	{
		KMessageBox::error(this, i18n("Unable to detect local ports."));
		return;
	}
	QListViewItem	*last[4] = {0, 0, 0, 0};
	for (QStringList::Iterator it=list.begin(); it!=list.end(); ++it)
	{
		QString cl = *it;
		++it;

		QString	uri = *it;
		int p = uri.find( ':' );
		QString	desc = *(++it), prot = ( p != -1 ? uri.left( p ) : QString::null );
		QString	printer = *(++it);
		int	index(-1);
		if (desc.isEmpty())
			desc = uri;
		if (prot == "parallel" || prot == "file")
			index = 0;
		else if (prot == "serial")
			index = 1;
		else if (prot == "usb")
			index = 2;
		else if (cl == "direct")
			index = 3;
		else
			continue;
		last[index] = new QListViewItem(m_parents[index], last[index], desc, uri);
		last[index]->setPixmap(0, SmallIcon("blockdevice"));
		m_parents[index]->setOpen(true);
		m_uris << uri;
		if (!printer.isEmpty())
		{
			QListViewItem	*pItem = new QListViewItem(last[index], printer);
			last[index]->setOpen(true);
			pItem->setPixmap(0, SmallIcon("kdeprint_printer"));
		}
	}
	m_initialized = true;
}

#include "kmwlocal.moc"
