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

#include "kmconfigproxy.h"
#include "kmproxywidget.h"

#include <qlayout.h>
#include <klocale.h>

KMConfigProxy::KMConfigProxy(QWidget *parent)
: KMConfigPage(parent,"Proxy")
{
	setPageName(i18n("Proxy"));
	setPageHeader(i18n("RLPR Proxy Server Settings"));
	setPagePixmap("proxy");

	m_widget = new KMProxyWidget(this);
	QVBoxLayout	*lay0 = new QVBoxLayout(this, 5, 0);
	lay0->addWidget(m_widget);
	lay0->addStretch(1);
}

void KMConfigProxy::loadConfig(KConfig *conf)
{
	m_widget->loadConfig(conf);
}

void KMConfigProxy::saveConfig(KConfig *conf)
{
	m_widget->saveConfig(conf);
}
