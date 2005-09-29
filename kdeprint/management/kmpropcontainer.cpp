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

#include "kmpropcontainer.h"
#include "kmpropwidget.h"

#include <kpushbutton.h>
#include <qlayout.h>
#include <klocale.h>
#include <kseparator.h>
#include <kguiitem.h>

KMPropContainer::KMPropContainer(QWidget *parent, const char *name)
: QWidget(parent,name)
{
	KSeparator* sep = new KSeparator( KSeparator::HLine, this);
	sep->setFixedHeight(5);

	m_button = new KPushButton(KGuiItem(i18n("Change..."), "edit"), this);
	m_widget = 0;

	QVBoxLayout	*main_ = new QVBoxLayout(this, 0, 10);
	QHBoxLayout	*btn_ = new QHBoxLayout(0, 0, 0);
	main_->addWidget(sep,0);
	main_->addLayout(btn_,0);
	btn_->addStretch(1);
	btn_->addWidget(m_button);
}

KMPropContainer::~KMPropContainer()
{
}

void KMPropContainer::setWidget(KMPropWidget *w)
{
	if (!m_widget)
	{
		m_widget = w;
		m_widget->reparent(this,QPoint(0,0));
		connect(m_button,SIGNAL(clicked()),m_widget,SLOT(slotChange()));
		connect(m_widget,SIGNAL(enable(bool)),SIGNAL(enable(bool)));
		connect(m_widget,SIGNAL(enableChange(bool)),SLOT(slotEnableChange(bool)));
		QVBoxLayout	*lay = dynamic_cast<QVBoxLayout*>(layout());
		if (lay)
		{
			lay->insertWidget(0,m_widget,1);
		}
	}
}

void KMPropContainer::setPrinter(KMPrinter *p)
{
	if (m_widget)
		m_widget->setPrinterBase(p);
}

void KMPropContainer::slotEnableChange(bool on)
{
	m_button->setEnabled(on && (m_widget ? m_widget->canChange() : true));
}
#include "kmpropcontainer.moc"
