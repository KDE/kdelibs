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

#include "kfilelist.h"

#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qdragobject.h>
#include <qtooltip.h>
#include <qregexp.h>
#include <qheader.h>

#include <kio/netaccess.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <klistview.h>
#include <krun.h>
#include <kmimetype.h>

KFileList::KFileList(QWidget *parent, const char *name)
: QWidget(parent, name)
{
	m_block = false;

	m_files = new KListView(this);
	m_files->addColumn(i18n("Name"));
	m_files->addColumn(i18n("Type"));
	m_files->addColumn(i18n("Path"));
	m_files->setAllColumnsShowFocus(true);
	m_files->setSorting(-1);
	m_files->setAcceptDrops(false);
	m_files->setSelectionMode(QListView::Extended);
	m_files->header()->setStretchEnabled(true, 2);
	connect(m_files, SIGNAL(selectionChanged()), SLOT(slotSelectionChanged()));

	m_add = new QPushButton(this);
	m_add->setPixmap(SmallIcon("fileopen"));
	connect(m_add, SIGNAL(clicked()), SLOT(slotAddFile()));
	QToolTip::add(m_add, i18n("Add File"));

	m_remove = new QPushButton(this);
	m_remove->setPixmap(SmallIcon("remove"));
	connect(m_remove, SIGNAL(clicked()), SLOT(slotRemoveFile()));
	QToolTip::add(m_remove, i18n("Remove File"));
	m_remove->setEnabled(false);

	m_open = new QPushButton(this);
	m_open->setPixmap(SmallIcon("filefind"));
	connect(m_open, SIGNAL(clicked()), SLOT(slotOpenFile()));
	QToolTip::add(m_open, i18n("Open File"));
	m_open->setEnabled(false);

	m_up = new QPushButton(this);
	m_up->setPixmap(SmallIcon("up"));
	connect(m_up, SIGNAL(clicked()), SLOT(slotUp()));
	QToolTip::add(m_up, i18n("Move Up"));
	m_up->setEnabled(false);

	m_down = new QPushButton(this);
	m_down->setPixmap(SmallIcon("down"));
	connect(m_down, SIGNAL(clicked()), SLOT(slotDown()));
	QToolTip::add(m_down, i18n("Move Down"));
	m_down->setEnabled(false);

	setAcceptDrops(true);

	QToolTip::add(m_files, i18n(
		"Drag file(s) here or use the button to open a file dialog. "
		"Leave empty for <b>&lt;STDIN&gt;</b>."));

	QHBoxLayout	*l0 = new QHBoxLayout(this, 0, 5);
	QVBoxLayout	*l1 = new QVBoxLayout(0, 0, 0);
	l0->addWidget(m_files);
	l0->addLayout(l1);
	l1->addWidget(m_add);
	l1->addWidget(m_remove);
	l1->addWidget(m_open);
	l1->addSpacing(10);
	l1->addWidget(m_up);
	l1->addWidget(m_down);
	l1->addStretch(1);
}

KFileList::~KFileList()
{
}

void KFileList::dragEnterEvent(QDragEnterEvent *e)
{
	e->accept(QUriDrag::canDecode(e));
}

void KFileList::dropEvent(QDropEvent *e)
{
	QStringList	files;
	if (QUriDrag::decodeToUnicodeUris(e, files))
	{
		addFiles(files);
	}
}

void KFileList::addFiles(const QStringList& files)
{
	if (files.count() > 0)
	{
		// search last item in current list, to add new ones at the end
		QListViewItem	*item = m_files->firstChild();
		while (item && item->nextSibling())
			item = item->nextSibling();

		// for each file, download it (if necessary) and add it
		QString	downloaded;
		for (QStringList::ConstIterator it=files.begin(); it!=files.end(); ++it)
			if (KIO::NetAccess::download(KURL(*it), downloaded))
			{
				KURL	url(downloaded);
				KMimeType::Ptr	mime = KMimeType::findByURL(url, 0, true, false);
				item = new QListViewItem(m_files, item, url.fileName(), mime->comment(), downloaded);
				item->setPixmap(0, mime->pixmap(url, KIcon::Small));
			}

		slotSelectionChanged();
		/*
		if (m_files->childCount() > 0)
		{
			m_remove->setEnabled(true);
			m_open->setEnabled(true);
			if (m_files->currentItem() == 0)
				m_files->setSelected(m_files->firstChild(), true);
		}
		*/
	}
}

void KFileList::setFileList(const QStringList& files)
{
	m_files->clear();
	addFiles(files);
}

QStringList KFileList::fileList() const
{
	QStringList	l;
	QListViewItem	*item = m_files->firstChild();
	while (item)
	{
		l << item->text(2);
		item = item->nextSibling();
	}
	return l;
}

void KFileList::slotAddFile()
{
	QString	fname = KFileDialog::getOpenFileName(QString::null, QString::null, this);
	if (!fname.isEmpty())
		addFiles(QStringList(fname));
}

void KFileList::slotRemoveFile()
{
	QPtrList<QListViewItem>	l;
	selection(l);
	l.setAutoDelete(true);
	m_block = true;
	l.clear();
	m_block = false;
	slotSelectionChanged();
}

void KFileList::slotOpenFile()
{
	QListViewItem	*item = m_files->currentItem();
	if (item)
	{
		new KRun(KURL(item->text(2)));
	}
}

QSize KFileList::sizeHint() const
{
	return QSize(100, 100);
}

void KFileList::selection(QPtrList<QListViewItem>& l)
{
	l.setAutoDelete(false);
	QListViewItem	*item = m_files->firstChild();
	while (item)
	{
		if (item->isSelected())
			l.append(item);
		item = item->nextSibling();
	}
}

void KFileList::slotSelectionChanged()
{
	if (m_block)
		return;

	QPtrList<QListViewItem>	l;
	selection(l);
	m_remove->setEnabled(l.count() > 0);
	m_open->setEnabled(l.count() == 1);
	m_up->setEnabled(l.count() == 1 && l.first()->itemAbove());
	m_down->setEnabled(l.count() == 1 && l.first()->itemBelow());
}

void KFileList::slotUp()
{
	QPtrList<QListViewItem>	l;
	selection(l);
	if (l.count() == 1 && l.first()->itemAbove())
	{
		QListViewItem	*item(l.first()), *clone(0);
		clone = new QListViewItem(m_files, item->itemAbove()->itemAbove(), item->text(0), item->text(1), item->text(2));
		clone->setPixmap(0, *(item->pixmap(0)));
		delete item;
		m_files->setCurrentItem(clone);
		m_files->setSelected(clone, true);
	}
}

void KFileList::slotDown()
{
	QPtrList<QListViewItem>	l;
	selection(l);
	if (l.count() == 1 && l.first()->itemBelow())
	{
		QListViewItem	*item(l.first()), *clone(0);
		clone = new QListViewItem(m_files, item->itemBelow(), item->text(0), item->text(1), item->text(2));
		clone->setPixmap(0, *(item->pixmap(0)));
		delete item;
		m_files->setCurrentItem(clone);
		m_files->setSelected(clone, true);
	}
}

#include "kfilelist.moc"
