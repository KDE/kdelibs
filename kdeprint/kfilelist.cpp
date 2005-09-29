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

#include "kfilelist.h"

#include <qtoolbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qheader.h>
#include <qwhatsthis.h>

#include <kio/netaccess.h>
#include <kurldrag.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <klistview.h>
#include <krun.h>
#include <kmimetype.h>

KFileList::KFileList(QWidget *parent, const char *name)
: QWidget(parent, name)
{
	//WhatsThis strings.... (added by pfeifle@kde.org)
	QString whatsThisAddFileButton = i18n(  " <qt> <b>Add File button</b>"
						" <p>This button calls the <em>'File Open'</em> dialog to let you"
						" select a file for printing. Note, that "
						" <ul><li>you can select ASCII or International Text, PDF,"
						" PostScript, JPEG, TIFF, PNG, GIF and many other graphic"
						" formats."
						" <li>you can select various files from different paths"
						" and send them as one \"multi-file job\" to the printing"
						" system."
						" </ul>"
					        " </qt>" );

	QString whatsThisRemoveFileButton = i18n(" <qt> <b>Remove File button</b>"
                                                " <p>This button removes the highlighted file from the"
						" list of to-be-printed files."
					        " </qt>" );

	QString whatsThisMoveFileUpButton = i18n(" <qt> <b>Move File Up button</b>"
                                                " <p>This button moves the highlighted file up in the list"
						" of files to be printed.</p>"
						" <p>In effect, this changes the order"
						" of the files' printout.</p>"
					        " </qt>" );

	QString whatsThisMoveFileDownButton = i18n(" <qt> <b>Move File Down button</b>"
                                                " <p>This button moves the highlighted file down in the list"
						" of files to be printed.</p>"
						" <p>In effect, this changes the order"
						" of the files' printout.</p>"
					        " </qt>" );

	QString whatsThisOpenFileButton = i18n( " <qt> <b>File Open button</b>"
                                                " <p>This button tries to open the highlighted file, so"
						" you can view or edit it before you send it to the printing"
						" system.</p>"
						" <p>If you open"
						" files, KDEPrint will use the application matching the MIME type of"
						" the file.</p>"
					        " </qt>" );

	QString whatsThisFileSelectionListview = i18n( " <qt> <b>File List view</b>"
                                                " <p>This list displays all the files you selected for printing."
						" You can see the file name(s), file path(s) and the file"
						" (MIME) type(s) as determined by KDEPrint. You may re-arrange the "
						" initial order of the list "
						" with the help of the arrow buttons on the right.</p>"
						" <p>The files will be printed as a single job,"
						" in the same order as displayed in the list.</p>"
						" <p><b>Note:</b> You can select multiple files. The files may be in multiple"
						" locations. The files may be of multiple MIME types. The buttons on the right"
						" side let you add more files, remove already selected files from the list, "
						" re-order the list (by moving files up or down), and open files. If you open"
						" files, KDEPrint will use the application matching the MIME type of"
						" the file.</p>"
					        " </qt>" );

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
	QWhatsThis::add(m_files, whatsThisFileSelectionListview);
	connect(m_files, SIGNAL(selectionChanged()), SLOT(slotSelectionChanged()));

	m_add = new QToolButton(this);
	m_add->setIconSet(SmallIconSet("fileopen"));
	connect(m_add, SIGNAL(clicked()), SLOT(slotAddFile()));
	QToolTip::add(m_add, i18n("Add file"));
	QWhatsThis::add(m_add, whatsThisAddFileButton);

	m_remove = new QToolButton(this);
	m_remove->setIconSet(SmallIconSet("remove"));
	connect(m_remove, SIGNAL(clicked()), SLOT(slotRemoveFile()));
	QToolTip::add(m_remove, i18n("Remove file"));
	QWhatsThis::add(m_remove, whatsThisRemoveFileButton);
	m_remove->setEnabled(false);

	m_open = new QToolButton(this);
	m_open->setIconSet(SmallIconSet("filefind"));
	connect(m_open, SIGNAL(clicked()), SLOT(slotOpenFile()));
	QToolTip::add(m_open, i18n("Open file"));
	QWhatsThis::add(m_open, whatsThisOpenFileButton);
	m_open->setEnabled(false);

	m_up = new QToolButton(this);
	m_up->setIconSet(SmallIconSet("up"));
	connect(m_up, SIGNAL(clicked()), SLOT(slotUp()));
	QToolTip::add(m_up, i18n("Move up"));
	QWhatsThis::add(m_up, whatsThisMoveFileUpButton);
	m_up->setEnabled(false);

	m_down = new QToolButton(this);
	m_down->setIconSet(SmallIconSet("down"));
	connect(m_down, SIGNAL(clicked()), SLOT(slotDown()));
	QToolTip::add(m_down, i18n("Move down"));
	QWhatsThis::add(m_down, whatsThisMoveFileDownButton);
	m_down->setEnabled(false);

	setAcceptDrops(true);

	QToolTip::add(m_files, i18n(
		"Drag file(s) here or use the button to open a file dialog. "
		"Leave empty for <b>&lt;STDIN&gt;</b>."));

	QHBoxLayout	*l0 = new QHBoxLayout(this, 0, KDialog::spacingHint());
	QVBoxLayout	*l1 = new QVBoxLayout(0, 0, 1);
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
	e->accept(KURLDrag::canDecode(e));
}

void KFileList::dropEvent(QDropEvent *e)
{
	KURL::List	files;
	if (KURLDrag::decode(e, files))
	{
		addFiles(files);
	}
}

void KFileList::addFiles(const KURL::List& files)
{
	if (files.count() > 0)
	{
		// search last item in current list, to add new ones at the end
		QListViewItem	*item = m_files->firstChild();
		while (item && item->nextSibling())
			item = item->nextSibling();

		// for each file, download it (if necessary) and add it
		QString	downloaded;
		for (KURL::List::ConstIterator it=files.begin(); it!=files.end(); ++it)
			if (KIO::NetAccess::download(*it, downloaded, this))
			{
				KURL	url;
				url.setPath(downloaded);
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
	QListViewItem *item = 0;
	for (QStringList::ConstIterator it=files.begin(); it!=files.end(); ++it)
	{
		KURL	url;
		url.setPath(*it);
		KMimeType::Ptr	mime = KMimeType::findByURL(url, 0, true, false);
		item = new QListViewItem(m_files, item, url.fileName(), mime->comment(), *it);
		item->setPixmap(0, mime->pixmap(url, KIcon::Small));
	}
	slotSelectionChanged();
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
	KURL	fname = KFileDialog::getOpenURL(QString::null, QString::null, this);
	if (!fname.isEmpty())
		addFiles(KURL::List(fname));
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
		KURL url( item->text( 2 ) );
		new KRun(url);
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
		QListViewItem	*item(l.first()), *clone;
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
		QListViewItem	*item(l.first()), *clone;
		clone = new QListViewItem(m_files, item->itemBelow(), item->text(0), item->text(1), item->text(2));
		clone->setPixmap(0, *(item->pixmap(0)));
		delete item;
		m_files->setCurrentItem(clone);
		m_files->setSelected(clone, true);
	}
}

#include "kfilelist.moc"
