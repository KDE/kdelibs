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

#include "kmconfigcommand.h"
#include "kxmlcommandselector.h"

#include <qlabel.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <klocale.h>

KMConfigCommand::KMConfigCommand(QWidget *parent, const char *name)
: KMConfigPage(parent, name)
{
	setPageName(i18n("Commands"));
	setPageHeader(i18n("Commands Settings"));
	setPagePixmap("exec");

	QGroupBox	*gb = new QGroupBox(0, Qt::Horizontal, i18n("Edit/Create Commands"), this);
	QLabel	*lab = new QLabel(i18n(
				"<p>Command objects perform a conversion from input to output.<br>"
                "They are used as the basis to build both print filters "
				"and special printers. They are described by a command string, a "
				"set of options, a set of requirements and associated mime types. "
				"Here you can create new command objects and edit existing ones. All "
				"changes will only be effective for you."), gb);
	KXmlCommandSelector	*sel = new KXmlCommandSelector(false, gb);

	QVBoxLayout	*l0 = new QVBoxLayout(this, 5, 10);
	l0->addWidget(gb);
	l0->addStretch(1);
	QVBoxLayout	*l2 = new QVBoxLayout(gb->layout(), 10);
	l2->addWidget(lab);
	l2->addWidget(sel);
}
