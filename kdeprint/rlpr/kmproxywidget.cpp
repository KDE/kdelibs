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

#include "kmproxywidget.h"

#include <qlineedit.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qvalidator.h>
#include <klocale.h>
#include <kconfig.h>
#include <kcursor.h>

KMProxyWidget::KMProxyWidget(QWidget *parent, const char *name)
: QGroupBox(0, Qt::Vertical, i18n("Proxy Settings"), parent, name)
{
	QLabel	*m_hostlabel = new QLabel(i18n("&Host:"), this);
	QLabel	*m_portlabel = new QLabel(i18n("&Port:"), this);
	m_useproxy = new QCheckBox(i18n("&Use proxy server"), this);
	m_useproxy->setCursor(KCursor::handCursor());
	m_proxyhost = new QLineEdit(this);
	m_proxyport = new QLineEdit(this);
	m_proxyport->setValidator(new QIntValidator(m_proxyport));
	m_hostlabel->setBuddy(m_proxyhost);
	m_portlabel->setBuddy(m_proxyport);
	
	connect(m_useproxy,SIGNAL(toggled(bool)),m_proxyhost,SLOT(setEnabled(bool)));
	connect(m_useproxy,SIGNAL(toggled(bool)),m_proxyport,SLOT(setEnabled(bool)));
	m_proxyhost->setEnabled(false);
	m_proxyport->setEnabled(false);

	QGridLayout	*lay0 = new QGridLayout(layout(), 3, 2, 10);
	lay0->setColStretch(1,1);
	lay0->addMultiCellWidget(m_useproxy,0,0,0,1);
	lay0->addWidget(m_hostlabel,1,0);
	lay0->addWidget(m_portlabel,2,0);
	lay0->addWidget(m_proxyhost,1,1);
	lay0->addWidget(m_proxyport,2,1);
}

void KMProxyWidget::loadConfig(KConfig *conf)
{
	conf->setGroup("RLPR");
	m_proxyhost->setText(conf->readEntry("ProxyHost",QString::null));
	m_proxyport->setText(conf->readEntry("ProxyPort",QString::null));
	m_useproxy->setChecked(!m_proxyhost->text().isEmpty());
}

void KMProxyWidget::saveConfig(KConfig *conf)
{
	conf->setGroup("RLPR");
	conf->writeEntry("ProxyHost",(m_useproxy->isChecked() ? m_proxyhost->text() : QString::null));
	conf->writeEntry("ProxyPort",(m_useproxy->isChecked() ? m_proxyport->text() : QString::null));
}
