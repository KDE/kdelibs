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

#include "kpfileselectpage.h"
#include "kfilelist.h"

#include <qlayout.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <klocale.h>
#include <kiconloader.h>

KPFileSelectPage::KPFileSelectPage(QWidget *parent)
: KPrintDialogPage(parent)
{
	setTitle(i18n("&Files"));
	m_first = true;

	m_files = new KFileList(this);

	QHBoxLayout	*l0 = new QHBoxLayout(this, 0, 10);
	l0->addWidget(m_files);

	resize(100, 100);
}

void KPFileSelectPage::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	// (incldef == false) is a hint telling that it should be the last time
	// and we want to do it only once
	if (!incldef)
	{
		QStringList	l = m_files->fileList();
		opts["kde-filelist"] = l.join("@@");
	}
}

void KPFileSelectPage::setOptions(const QMap<QString,QString>& opts)
{
	// do it only once as files will only be selected there
	if (m_first)
	{
		QStringList	l = QStringList::split("@@", opts["kde-filelist"], false);
		m_files->setFileList(l);

		m_first = false;
	}
}
