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

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qdragobject.h>
#include <qtooltip.h>
#include <qregexp.h>

#include <kio/netaccess.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kiconloader.h>

KFileList::KFileList(QWidget *parent, const char *name)
: QWidget(parent, name)
{
	m_edit = new QLineEdit(this);
	m_edit->setAcceptDrops(false);
	QLabel	*m_label = new QLabel(i18n("Files:"), this);
	QPushButton	*m_button = new QPushButton(this);
	m_button->setPixmap(SmallIcon("fileopen"));
	connect(m_button, SIGNAL(clicked()), SLOT(slotAddFile()));

	QToolTip::add(m_edit, i18n("<p>Drag file(s) here or use the button to open a file dialog. Leave empty for <b>&lt;STDIN&gt;</b>.</p>"));

	setAcceptDrops(true);

	QHBoxLayout	*l0 = new QHBoxLayout(this, 0, 5);
	l0->addWidget(m_label, 0);
	l0->addWidget(m_edit, 1);
	l0->addWidget(m_button, 0);
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
		// first download files if necessary
		QStringList	localFiles;
		QString		target;
		for (QStringList::ConstIterator it=files.begin(); it!=files.end(); ++it)
			if (KIO::NetAccess::download(KURL(*it), target))
				localFiles.append(target);

		// then add them to the edit line
		QString	txt = m_edit->text().stripWhiteSpace();
		if (!txt.isEmpty())
			txt.append(", ");
		txt.append(localFiles.join(", "));
		m_edit->setText(txt);
	}
}

void KFileList::setFileList(const QStringList& files)
{
	m_edit->clear();
	addFiles(files);
}

QStringList KFileList::fileList() const
{
	QString	txt = m_edit->text();
	QStringList	files = QStringList::split(QRegExp(",\\s*"), txt, false);
	return files;
}

void KFileList::slotAddFile()
{
	QString	fname = KFileDialog::getOpenFileName(QString::null, QString::null, this);
	if (!fname.isEmpty())
		addFiles(QStringList(fname));
}
#include "kfilelist.moc"
