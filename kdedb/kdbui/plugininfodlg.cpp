/*
   This file is part of the KDB libraries
   Copyright (c) 2000 Praduroux Alessandro <pradu@thekompany.com>
 
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
 
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/     
#include "plugininfodlg.h"

#include <qlistview.h>
#include <qvbox.h>

#include <klocale.h>

#include <kdb/plugin.h>

#include "plugininfodlg.moc"

using namespace KDB;

PluginInfoDialog::PluginInfoDialog(Plugin *plugin, QWidget *parent)
    :KDialogBase(parent, "PluginDialog", true, plugin->name(), Help | Close, Close, true)
{
    QVBox *main = new QVBox(this, "MainBox");
    setMainWidget(main);

    m_listview = new QListView(main, "listBox");
    m_listview->addColumn(i18n("Property"));
    m_listview->addColumn(i18n("Value"));
    m_listview->setFocusPolicy(NoFocus);
    m_listview->setSorting(-1);

    setPlugin( plugin );
}

PluginInfoDialog::~PluginInfoDialog()
{
}

void PluginInfoDialog::setPlugin( Plugin *plugin )
{
    Plugin::PluginInfo info = plugin->info();
    
    m_listview->clear();
    new QListViewItem(m_listview, i18n("Name"),plugin->name());
    new QListViewItem(m_listview, i18n("Description"),info.description);
    new QListViewItem(m_listview, i18n("Author"),info.author);
    new QListViewItem(m_listview, i18n("E-Mail"),info.e_mail);
    new QListViewItem(m_listview, i18n("Version"),info.version);
    new QListViewItem(m_listview, i18n("Copyright"),info.copyright);
}

void PluginInfoDialog::showInfo(Plugin *plugin, QWidget *parent)
{
    PluginInfoDialog *dialog = new PluginInfoDialog(plugin, parent);
    
    dialog->exec();    
    delete dialog;
}
