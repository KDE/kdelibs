#include "scriptloader.h"

#include <kapplication.h>
#include <kparts/part.h>
#include <kparts/componentfactory.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kdesktopfile.h>
#include <kstddirs.h>
#include <kstdaccel.h>

#include <qdir.h>
#include <qfileinfo.h>


ScriptLoader::ScriptLoader(KMainWindow *parent) : QObject (parent)
{
	m_parent = parent;
	m_scripts.clear();
  	m_theAction = new KSelectAction ( i18n("KDE Scripts"),
		0,
		m_parent,
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
	QDir d(locate( "scripts", kapp->name()));
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
			m_scripts.append(KParts::ComponentFactory::createInstanceFromQuery<KScriptInterface>( desktop.readType() ));
			m_scripts.current()->setScript(desktop.readURL());
			if(m_parent != 0)
				m_scripts.current()->setParent(m_parent);
			pluginList.append(desktop.readName());
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
