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

#include "kptagspage.h"

#include <qtable.h>
#include <qheader.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qregexp.h>
#include <klocale.h>

KPTagsPage::KPTagsPage(bool ro, QWidget *parent, const char *name)
: KPrintDialogPage(parent, name)
{
	setTitle(i18n("Additional Tags"));
	setOnlyRealPrinters(true);

	m_tags = new QTable(10, 2, this);
	m_tags->horizontalHeader()->setStretchEnabled(true);
	m_tags->horizontalHeader()->setLabel(0, i18n("Name"));
	m_tags->horizontalHeader()->setLabel(1, i18n("Value"));
	m_tags->setReadOnly(ro);

	QVBoxLayout	*l0 = new QVBoxLayout(this, 0, 5);
	l0->addWidget(m_tags);

	if (ro)
	{
		QLabel	*lab = new QLabel(i18n("Read-Only"), this);
		QFont	f = lab->font();
		f.setBold(true);
		lab->setFont(f);
		lab->setAlignment(AlignVCenter|AlignRight);
		l0->addWidget(lab);
	}
}

KPTagsPage::~KPTagsPage()
{
}

bool KPTagsPage::isValid(QString& msg)
{
	QRegExp	re("\\s");
	for (int r=0; r<m_tags->numCols(); r++)
	{
		QString	tag(m_tags->text(r, 0));
		if (tag.isEmpty())
			continue;
		else if (tag.find(re) != -1 || m_tags->text(r, 1).find(re) != -1)
		{
			msg = i18n("The tag name and value may contain any space: %1.").arg(tag);
			return false;
		}
	}
	return true;
}

void KPTagsPage::setOptions(const QMap<QString,QString>& opts)
{
	int	r(0);
	for (QMap<QString,QString>::ConstIterator it=opts.begin(); it!=opts.end() && r<m_tags->numCols(); ++it)
	{
		if (it.key().startsWith("KDEPrint-"))
		{
			m_tags->setText(r, 0, it.key().mid(9));
			m_tags->setText(r, 1, it.data());
			r++;
		}
	}
	for (; r<m_tags->numCols(); r++)
	{
		m_tags->setText(r, 0, QString::null);
		m_tags->setText(r, 1, QString::null);
	}
}

void KPTagsPage::getOptions(QMap<QString,QString>& opts, bool)
{
	for (int r=0; r<m_tags->numCols(); r++)
	{
		QString	tag(m_tags->text(r, 0)), val(m_tags->text(r, 1));
		if (!tag.isEmpty())
		{
			tag.prepend("KDEPrint-");
			opts[tag] = val;
		}
	}
}

QSize KPTagsPage::sizeHint() const
{
	return QSize(-1, -1);
}

QSize KPTagsPage::minimumSizeHint() const
{
	return QSize(-1, -1);
}
