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

#include "kptagspage.h"

#include <qtable.h>
#include <qheader.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qregexp.h>
#include <qwhatsthis.h>

#include <klocale.h>

KPTagsPage::KPTagsPage(bool ro, QWidget *parent, const char *name)
: KPrintDialogPage(parent, name)
{

	QString whatsThisAdditionalTagsTable = i18n(" <qt><p>You may send additional messages to the operator(s) of your"
						" production printers (e.g. in your <em>Central Repro Department</p>)"
						" Your messages can be read by the operator(s) (or yourself) by viewing"
						" the <em>\"Job IPP Report\"</em> for the respective job.</p>"
						" <b>Examples:</b><br><pre>"
						" <em>Call_after_completion</em> (Name) -- <em>Joe User, phone extension 1234</em> (Value)<br>"
						" <em>Mail_after_completion</em> (Name) -- <em>Jill User &lt;juser@somecompany.com&gt;</em> (Value)<br>"
						" <em>Deliver_after_completion</em> (Name) -- <em>to Marketing Department</em> (Value)"
						" </pre>"
						" <p><b>Note:</b> the \"Name\" field must not include spaces or tabs."
					        " </qt>" );
	setTitle(i18n("Additional Tags"));
	setOnlyRealPrinters(true);

	m_tags = new QTable(10, 2, this);
	m_tags->horizontalHeader()->setStretchEnabled(true);
	m_tags->horizontalHeader()->setLabel(0, i18n("Name"));
	m_tags->horizontalHeader()->setLabel(1, i18n("Value"));
	m_tags->setReadOnly(ro);
	QWhatsThis::add(m_tags, whatsThisAdditionalTagsTable);

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
		else if (tag.find(re) != -1)
		{
			msg = i18n("The tag name must not contain any spaces: <b>%1</b>.").arg(tag);
			return false;
		}
	}
	return true;
}

void KPTagsPage::setOptions(const QMap<QString,QString>& opts)
{
	int	r(0);
	QRegExp	re("^\"|\"$");
	for (QMap<QString,QString>::ConstIterator it=opts.begin(); it!=opts.end() && r<m_tags->numRows(); ++it)
	{
		if (it.key().startsWith("KDEPrint-"))
		{
			m_tags->setText(r, 0, it.key().mid(9));
			QString	data = it.data();
			m_tags->setText(r, 1, data.replace(re, ""));
			r++;
		}
	}
	for (; r<m_tags->numRows(); r++)
	{
		m_tags->setText(r, 0, QString::null);
		m_tags->setText(r, 1, QString::null);
	}
}

void KPTagsPage::getOptions(QMap<QString,QString>& opts, bool)
{
	for (int r=0; r<m_tags->numRows(); r++)
	{
		QString	tag(m_tags->text(r, 0)), val(m_tags->text(r, 1));
		if (!tag.isEmpty())
		{
			tag.prepend("KDEPrint-");
			opts[tag] = val.prepend("\"").append("\"");
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
