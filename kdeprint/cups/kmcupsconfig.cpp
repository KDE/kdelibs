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

#include "kmcupsconfig.h"
#include "kmcupsconfigwidget.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <klocale.h>

KMCupsConfig::KMCupsConfig(QWidget *parent, const char *name)
: KDialog(parent,name,true)
{
	setCaption(i18n("CUPS Settings"));

	// widget creation
	m_widget = new KMCupsConfigWidget(this);
	QPushButton	*m_ok = new QPushButton(i18n("OK"), this);
	m_ok->setDefault(true);
	QPushButton	*m_cancel = new QPushButton(i18n("Cancel"), this);

	// layout creation
	QVBoxLayout	*lay0 = new QVBoxLayout(this, 10, 10);
	QHBoxLayout	*lay1 = new QHBoxLayout(0, 0, 10);
	lay0->addWidget(m_widget,1);
	lay0->addLayout(lay1,0);
	lay1->addStretch(1);
	lay1->addWidget(m_ok);
	lay1->addWidget(m_cancel);

	// connections
	connect(m_ok,SIGNAL(clicked()),SLOT(accept()));
	connect(m_cancel,SIGNAL(clicked()),SLOT(reject()));

	resize(300,100);
}

bool KMCupsConfig::configure(QWidget *parent)
{
	KMCupsConfig	dlg(parent);
	dlg.m_widget->load();
	if (dlg.exec())
	{
		dlg.m_widget->save(true);
		return true;
	}
	return false;
}
