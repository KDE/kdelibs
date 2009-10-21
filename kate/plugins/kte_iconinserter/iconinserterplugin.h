/***************************************************************************
*   This file is part of the KDevelop-Icon-Inserter-Plugin                *
*   Copyright 2009 Jonathan Schmidt-Dominé <devel@the-user.org>           *
*                                                                         *
*   This program is free software; you can redistribute it and/or         *
*   modify it under the terms of the GNU General Public License as        *
*   published by the Free Software Foundation; either version 3 of        *
*   the License or (at your option) any later version accepted.           *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
***************************************************************************/

#ifndef KDEVICONINSERTERPLUGIN_H
#define KDEVICONINSERTERPLUGIN_H

#include <KTextEditor/Plugin>

#include <kpluginfactory.h>

#include <QVariantList>

#include <KAction>

using namespace KTextEditor;

class IconInserterPlugin: public Plugin
{
	Q_OBJECT
	public:
		IconInserterPlugin (QObject *parent, const QVariantList & = QVariantList());
		~IconInserterPlugin();
		void addView (View *view);
		virtual void readConfig (KConfig*) {}
		virtual void writeConfig (KConfig*) {}
	private slots:
		void insertIcon();
		void addActionToMenu (KTextEditor::View *view, QMenu *menu);
	private:
		inline KAction* createAction (QObject* parent)
		{
			KAction* action = new KAction (KIcon("kcoloredit"), i18n ("Insert KIcon-Code"), parent); // This icon was selected using this plugin ;) (in an earlier KDevelop-only-version)
			action->setToolTip (i18n ("Insert Code for KIcon-Creation"));
			action->setWhatsThis (i18n ("<b>IconInserter</b><p> Select an icon and use it as a KIcon in your source-code!"));
			
			connect (action, SIGNAL (triggered (bool)), this, SLOT (insertIcon()));
			
			return action;
		}
};

K_PLUGIN_FACTORY_DECLARATION(IconInserterPluginFactory)

#endif
