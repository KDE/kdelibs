/* This file is part of the KDE project
   Copyright (C) 2001 Ian Reinhart Geiser  (geiseri@kde.org)

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
#include "scriptloader.h"

#include <kapplication.h>
#include <kparts/part.h>
#include <kparts/componentfactory.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kdesktopfile.h>
#include <kstandarsdirs.h>
#include <kstdaccel.h>
#include <kdebug.h>

#include <qdir.h>
#include <qfileinfo.h>


ScriptLoader::ScriptLoader(KMainWindow *parent) : QObject (parent)
{
	m_parent = parent;
	m_scripts.clear();
  	m_theAction = new KSelectAction ( i18n("KDE Scripts"),
		0,
		this,
		SLOT(runAction()),
		m_parent->actionCollection(),
		"scripts");
}

ScriptLoader::~ScriptLoader()
{
	// Clean out the list
	m_scripts.clear();
}

KSelectAction * ScriptLoader::getScripts()
{
	// Get the avaliable scripts for this application.
	QStringList pluginList = "";
	// Find plugins
	QString searchPath = kapp->name();
	searchPath += "/scripts/";
	QDir d(locate( "data", searchPath));
	kdDebug() << "loading plugin from " << locate( "data", searchPath) << endl;
	const QFileInfoList *fileList = d.entryInfoList("*.desktop");
	QFileInfoListIterator it ( *fileList );
	QFileInfo *fi;
	// Find all available script desktop files
	while( (fi=it.current()))
	{
	// Query each desktop file
		if(KDesktopFile::isDesktopFile(fi->absFilePath()))
		{
			KDesktopFile desktop((fi->absFilePath()), true);
			kdDebug () << "Trying to load script type: " << desktop.readType() << endl;
			KScriptInterface *tmpIface = KParts::ComponentFactory::createInstanceFromQuery<KScriptInterface>(desktop.readType() );
			if( tmpIface != 0 )
			{
				m_scripts.append(tmpIface);			
				m_scripts.current()->setScript(desktop.readURL());
				//if(m_parent != 0)
					//m_scripts.current()->setParent(m_parent);
				pluginList.append(desktop.readName());
			}
			else
				kdDebug() << desktop.readName() << " could not be loaded!" << endl;
		}
		++it;
	}
	m_theAction->clear();
	m_theAction->setItems(pluginList);
	return m_theAction;
}

void ScriptLoader::runAction()
{
	QString scriptName = m_theAction->currentText();

}

void ScriptLoader::stopAction()
{

}

#include "scriptloader.moc"
