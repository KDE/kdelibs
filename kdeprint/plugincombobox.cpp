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

PluginComboBox::PluginComboBox(QWidget *parent, const char *name)
:QComboBox(parent, name)
{
	QValueList<PluginInfo>	list = KMFactory::self()->pluginList();
	QString			currentPlugin = KMFactory::self()->printSystem();
	for (QValueList<PluginInfo>::ConstIterator it=list.begin(); it!=list.end(); ++it)
	{
		insertItem((*it).comment);
		if ((*it).name == currentPlugin)
			setCurrentItem(count()-1);
		m_pluginlist.append((*it).name);
	}

	connect(this, SIGNAL(activated(int)), SLOT(slotActivated(int)));
}

void PluginComboBox::slotActivated(int index)
{
	QString	plugin = m_pluginlist[index];
	if (!plugin.isEmpty())
	{
		emit aboutToChange();
		// the factory will notify all registered objects of the change
		KMFactory::self()->reload(plugin, true);
	}
}

#include "plugincombobox.moc"
