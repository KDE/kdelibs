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

#include "iconinserterplugin.h"

#include <kicondialog.h>

#include <KPluginFactory>
#include <KAboutData>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <QMenu>


//K_PLUGIN_FACTORY (KdevPlugFactory, registerPlugin<IconInserterPlugin>();)
//K_EXPORT_PLUGIN (KdevPlugFactory (KAboutData ("IconInserter","IconInserter", ki18n ("Select an Icon to use it inside the Code"), "0.1", ki18n ("Insert Code for KIcon-Creation"), KAboutData::License_GPL)))

K_PLUGIN_FACTORY_DEFINITION(IconInserterPluginFactory,
        registerPlugin<IconInserterPlugin>("ktexteditor_iconinserter");
        )
K_EXPORT_PLUGIN(IconInserterPluginFactory(KAboutData ("IconInserter","IconInserter", ki18n ("Select an Icon to use it inside the Code"), "0.1", ki18n ("Insert Code for KIcon-Creation"), KAboutData::License_GPL)))


IconInserterPlugin::IconInserterPlugin (QObject *parent, const QVariantList &)
	: Plugin (parent)
{
}

IconInserterPlugin::~IconInserterPlugin()
{
}

void IconInserterPlugin::insertIcon()
{
	QString iconName = KIconDialog::getIcon ( KIconLoader::Desktop,
							KIconLoader::Application,
							false,
							0,
							false,
							0,
							i18n( "Select the Icon you want to use in your code as KIcon!" )
						);
	if(iconName == "")
		return;
	
	View *view = reinterpret_cast<View*>(sender()->parent());
	Document *doc = view->document();
	
	QString suffix = doc->url().url();
	suffix = suffix.right(suffix.size() - suffix.lastIndexOf('.') - 1);
	QString code;
	if(suffix == "cpp" || suffix == "h" || suffix == "py")
		code = "KIcon (\"" + iconName + "\")";
	else if(suffix == "rb")
		code = "KDE::Icon.new (:\"" + iconName + "\")";
	else if(suffix == "js" || suffix == "qts" || suffix == "cs")
		code = "new KIcon (\"" + iconName + "\")";
	else if(suffix == "java")
		code = "new org.kde.kdeui.KIcon (\"" + iconName + "\")";
	else if(suffix == "fal" || suffix == "ftd")
		code = "KIcon ('" + iconName + "')";
	else if(suffix == "php")
		code = "new KIcon ('" + iconName + "')";
	else if(suffix == "pl")
		code = "KDE::Icon (\"" + iconName + "\")";
	else if(suffix == "pas")
		code = "KIcon_create ('" + iconName + "')";
	else if(suffix == "scm")
		code = "(make KIcon '" + iconName + ")";
	else if(suffix == "hs")
		code = "kIcon \"" + iconName + "\"";
	else if(suffix == "ads" || suffix == "adb")
		code = "KDEui.Icons.Constructos.Create (\"" + iconName + "\")";
	else
		code = iconName;
	doc->insertText(view->cursorPosition(), code);
}

void IconInserterPlugin::addView (KTextEditor::View *view)
{
	if(view->contextMenu() == 0)
		connect(view, SIGNAL(contextMenuAboutToShow(KTextEditor::View*, QMenu*)), this, SLOT(addActionToMenu(KTextEditor::View*, QMenu*)));
	else
		view->contextMenu()->addAction (createAction (view));
}

void IconInserterPlugin::addActionToMenu(KTextEditor::View *view, QMenu *menu)
{
	menu->addAction (createAction (view));
	disconnect(view, SIGNAL(contextMenuAboutToShow(KTextEditor::View*, QMenu*)), this, SLOT(addActionToMenu(KTextEditor::View*, QMenu*)));
}

#include "iconinserterplugin.moc"
