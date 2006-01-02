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
#include <q3header.h>
#include <QDragEnterEvent>

#include <kio/netaccess.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <klistview.h>
#include <krun.h>
#include <kmimetype.h>

KFileList::KFileList(QWidget *parent)
    : QWidget(parent)
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
	m_files->setSelectionMode(Q3ListView::Extended);
	m_files->header()->setStretchEnabled(true, 2);
	m_files->setWhatsThis(whatsThisFileSelectionListview);
	connect(m_files, SIGNAL(selectionChanged()), SLOT(slotSelectionChanged()));

	m_add = new QToolButton(this);
	m_add->setIcon(SmallIconSet("fileopen"));
	connect(m_add, SIGNAL(clicked()), SLOT(slotAddFile()));
	m_add->setToolTip(i18n("Add file"));
	m_add->setWhatsThis(whatsThisAddFileButton);

	m_remove = new QToolButton(this);
	m_remove->setIcon(SmallIconSet("remove"));
	connect(m_remove, SIGNAL(clicked()), SLOT(slotRemoveFile()));
	m_remove->setToolTip(i18n("Remove file"));
	m_remove->setWhatsThis(whatsThisRemoveFileButton);
	m_remove->setEnabled(false);

	m_open = new QToolButton(this);
	m_open->setIcon(SmallIconSet("filefind"));
	connect(m_open, SIGNAL(clicked()), SLOT(slotOpenFile()));
	m_open->setToolTip(i18n("Open file"));
	m_open->setWhatsThis(whatsThisOpenFileButton);
	m_open->setEnabled(false);

	m_up = new QToolButton(this);
	m_up->setIcon(SmallIconSet("up"));
	connect(m_up, SIGNAL(clicked()), SLOT(slotUp()));
	m_up->setToolTip(i18n("Move up"));
	m_up->setWhatsThis(whatsThisMoveFileUpButton);
	m_up->setEnabled(false);

	m_down = new QToolButton(this);
	m_down->setIcon(SmallIconSet("down"));
	connect(m_down, SIGNAL(clicked()), SLOT(slotDown()));
	m_down->setToolTip(i18n("Move down"));
	m_down->setWhatsThis(whatsThisMoveFileDownButton);
	m_down->setEnabled(false);

	setAcceptDrops(true);

	m_files->setToolTip(i18n(
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
    e->accept( KURL::List::canDecode( e->mimeData() ) );
}

void KFileList::dropEvent(QDropEvent *e)
{
	KURL::List	files = KURL::List::fromMimeData( e->mimeData() );
	if (!files.isEmpty())
	{
		addFiles(files);
	}
}

void KFileList::addFiles(const KURL::List& files)
{
	if (files.count() > 0)
	{
		// search last item in current list, to add new ones at the end
		Q3ListViewItem	*item = m_files->firstChild();
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
				item = new Q3ListViewItem(m_files, item, url.fileName(), mime->comment(), downloaded);
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
	Q3ListViewItem *item = 0;
	for (QStringList::ConstIterator it=files.begin(); it!=files.end(); ++it)
	{
		KURL	url;
		url.setPath(*it);
		KMimeType::Ptr	mime = KMimeType::findByURL(url, 0, true, false);
		item = new Q3ListViewItem(m_files, item, url.fileName(), mime->comment(), *it);
		item->setPixmap(0, mime->pixmap(url, KIcon::Small));
	}
	slotSelectionChanged();
}

QStringList KFileList::fileList() const
{
	QStringList	l;
	Q3ListViewItem	*item = m_files->firstChild();
	while (item)
	{
		l << item->text(2);
		item = item->nextSibling();
	}
	return l;
}

void KFileList::slotAddFile()
{
	KURL	fname = KFileDialog::getOpenURL(QString(), QString(), this);
	if (!fname.isEmpty())
		addFiles(KURL::List(fname));
}

void KFileList::slotRemoveFile()
{
	Q3PtrList<Q3ListViewItem>	l;
	selection(l);
	l.setAutoDelete(true);
	m_block = true;
	l.clear();
	m_block = false;
	slotSelectionChanged();
}

void KFileList::slotOpenFile()
{
	Q3ListViewItem	*item = m_files->currentItem();
	if (item)
	{
		KURL url( item->text( 2 ) );
		new KRun(url,window());
	}
}

QSize KFileList::sizeHint() const
{
	return QSize(100, 100);
}

void KFileList::selection(Q3PtrList<Q3ListViewItem>& l)
{
	l.setAutoDelete(false);
	Q3ListViewItem	*item = m_files->firstChild();
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

	Q3PtrList<Q3ListViewItem>	l;
	selection(l);
	m_remove->setEnabled(l.count() > 0);
	m_open->setEnabled(l.count() == 1);
	m_up->setEnabled(l.count() == 1 && l.first()->itemAbove());
	m_down->setEnabled(l.count() == 1 && l.first()->itemBelow());
}

void KFileList::slotUp()
{
	Q3PtrList<Q3ListViewItem>	l;
	selection(l);
	if (l.count() == 1 && l.first()->itemAbove())
	{
		Q3ListViewItem	*item(l.first()), *clone;
		clone = new Q3ListViewItem(m_files, item->itemAbove()->itemAbove(), item->text(0), item->text(1), item->text(2));
		clone->setPixmap(0, *(item->pixmap(0)));
		delete item;
		m_files->setCurrentItem(clone);
		m_files->setSelected(clone, true);
	}
}

void KFileList::slotDown()
{
	Q3PtrList<Q3ListViewItem>	l;
	selection(l);
	if (l.count() == 1 && l.first()->itemBelow())
	{
		Q3ListViewItem	*item(l.first()), *clone;
		clone = new Q3ListViewItem(m_files, item->itemBelow(), item->text(0), item->text(1), item->text(2));
		clone->setPixmap(0, *(item->pixmap(0)));
		delete item;
		m_files->setCurrentItem(clone);
		m_files->setSelected(clone, true);
	}
}

#include "kfilelist.moc"
