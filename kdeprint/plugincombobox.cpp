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

#include "plugincombobox.h"
#include "kmfactory.h"
#include "kmmanager.h"

#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <klocale.h>

PluginComboBox::PluginComboBox(QWidget *parent, const char *name)
:QWidget(parent, name)
{
	m_combo = new QComboBox(this, "PluginCombo");
	QLabel	*m_label = new QLabel(i18n("Print s&ystem currently used:"), this);
	m_label->setAlignment(AlignVCenter|AlignRight);
	m_label->setBuddy(m_combo);
	m_plugininfo = new QLabel("Plugin information", this);
	QGridLayout	*l0 = new QGridLayout(this, 2, 2, 0, 5);
	l0->setColStretch(0, 1);
	l0->addWidget(m_label, 0, 0);
	l0->addWidget(m_combo, 0, 1);
	l0->addWidget(m_plugininfo, 1, 1);

	QValueList<KMFactory::PluginInfo>	list = KMFactory::self()->pluginList();
	QString			currentPlugin = KMFactory::self()->printSystem();
	for (QValueList<KMFactory::PluginInfo>::ConstIterator it=list.begin(); it!=list.end(); ++it)
	{
		m_combo->insertItem((*it).comment);
		if ((*it).name == currentPlugin)
			m_combo->setCurrentItem(m_combo->count()-1);
		m_pluginlist.append((*it).name);
	}

	connect(m_combo, SIGNAL(activated(int)), SLOT(slotActivated(int)));
	configChanged();
}

void PluginComboBox::slotActivated(int index)
{
	QString	plugin = m_pluginlist[index];
	if (!plugin.isEmpty())
	{
		// the factory will notify all registered objects of the change
		KMFactory::self()->reload(plugin, true);
	}
}

void PluginComboBox::reload()
{
	QString	syst = KMFactory::self()->printSystem();
	int	index(-1);
	if ((index=m_pluginlist.findIndex(syst)) != -1)
		m_combo->setCurrentItem(index);
	configChanged();
}

void PluginComboBox::configChanged()
{
	m_plugininfo->setText(KMManager::self()->stateInformation());
}

#include "plugincombobox.moc"
