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

	//WhatsThis strings.... (added by pfeifle@kde.org)
	QString whatsThisAdditionalTagsTable = i18n(" <qt><p><b>Additional Tags</b></p>"
						" You may send additional commands to the CUPS server via this editable list. "
						" There are 3 purposes for this:"
						" <ul>"
						" <li>Use any current or future standard CUPS job option not supported by the "
						" KDEPrint GUI. </li>"
						" <li>Control any custom job option you may want to support in custom CUPS filters "
						" and backends plugged into the CUPS filtering chain.</li> "
						" <li>Send short messages to the operators of your production printers in your "
						" <em>Central Repro Department</em>."
						" </ul> "
						" <p><b>Standard CUPS job options:</b> A complete list of standard CUPS job "
						" options is in the <a href=\"http://localhost:631/sum.html\">CUPS User Manual</a>. "
						" Mappings of the kprinter user interface widgets to respective CUPS job option "
						" names are named in the various <em>WhatsThis</em> help items..</p>"
						" <p><b>Custom CUPS job options:</b> CUPS print servers may be customized with additional "
						" print filters and backends which understand custom job options. You can specify such "
						" custom job options here. If in doubt, ask your system administrator..</p>"
						" <p><b> </b></p>"
						" <p><b>Operator Messages:</b> You may send additional messages to the operator(s) of your"
						" production printers (e.g. in your <em>Central Repro Department</p>)"
						" Messages can be read by the operator(s) (or yourself) by viewing"
						" the <em>\"Job IPP Report\"</em> for the job.</p>"
						" <b>Examples:</b><br>"
						" <pre>"
						" A standard CUPS job option:<br> "
						"   <em>(Name) number-up</em>                -- <em>(Value) 9</em>                     <br>"
						" <br>"
						" A job option for custom CUPS filters or backends:<br> "
						"   <em>(Name) DANKA_watermark</em>          -- <em>(Value) Company_Confidential</em>   <br>"
						" <br>"
						" A message to the operator(s):<br> "
						"   <em>(Name) Deliver_after_completion</em> -- <em>(Value) to_Marketing_Departm.</em><br>"
						" </pre>"
						" <p><b>Note:</b> the fields must not include spaces, tabs or quotes. You may need to "
						" double-click on a field to edit it."
						" <p><b>Warning:</b> Do not use such standard CUPS option names which also can be used "
						" through the KDEPrint GUI. Results may be  unpredictable if they conflict, "
						" or if they are sent multiple times. For all options supported by the GUI, please do use "
						" the GUI. (Each GUI element's  'WhatsThis' names the related CUPS option name.) </p> "
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
			msg = i18n("The tag name must not contain any spaces, tabs or quotes: <b>%1</b>.").arg(tag);
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
