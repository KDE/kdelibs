/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  $Id$
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

#include "kmwfile.h"
#include "kmwizard.h"
#include "kmprinter.h"

#include <qlabel.h>
#include <qlayout.h>

#include <kurlrequester.h>
#include <klocale.h>
#include <kfiledialog.h>

KMWFile::KMWFile(QWidget *parent, const char *name)
: KMWizardPage(parent,name)
{
	m_ID = KMWizard::File;
	m_title = i18n("File selection");
	m_nextpage = KMWizard::Driver;

	m_url = new KURLRequester(this);
	m_url->setMode((KFile::Mode)(KFile::File|KFile::LocalOnly));
	QLabel	*l1 = new QLabel(this);
	l1->setText(i18n("<p>The printing will be redirected to a file. Enter here the path "
			 "of the file you want to use for redirection. Use an absolute path or "
			 "the browse button for graphical selection.</p>"));
	QLabel	*l2 = new QLabel(i18n("Print to file:"), this);

	QVBoxLayout	*lay1 = new QVBoxLayout(this, 0, 30);
	QVBoxLayout	*lay2 = new QVBoxLayout(0, 0, 5);
	lay1->addWidget(l1);
	lay1->addLayout(lay2);
	lay1->addStretch(1);
	lay2->addWidget(l2);
	lay2->addWidget(m_url);
}

bool KMWFile::isValid(QString& msg)
{
	QFileInfo	fi(m_url->url());
	if (fi.fileName().isEmpty())
	{
		msg = i18n("Empty file name!");
		return false;
	}

	if (!fi.dir().exists())
	{
		msg = i18n("Directory does not exist!");
		return false;
	}

	return true;
}

void KMWFile::updatePrinter(KMPrinter *p)
{
	QString	dev = QString::fromLatin1("file:%1").arg(m_url->url());
	p->setDevice(KURL(dev));
}
