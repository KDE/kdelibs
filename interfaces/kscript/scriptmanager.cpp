#include "scriptmanager.h"
#include <kparts/part.h>
#include <kparts/componentfactory.h>
#include <kapplication.h>
#include <kdesktopfile.h>
#include <kstandarddirs.h>

//using namespace KScriptInterface;
class ScriptInfo
{
	public:
		QString scriptType;
		QString scriptFile;
		QString scriptMethod;
		ScriptInfo();
		~ScriptInfo(){};
};
ScriptInfo::ScriptInfo()
{
	scriptType = "";
	scriptFile = "";
	scriptMethod = "";
}
KScriptManager::KScriptManager(QObject *parent, const char *name) :
	QObject(parent,name), KScriptClientInterface()
{

}
KScriptManager::~KScriptManager()
{
    m_scripts.setAutoDelete(true);
    m_scriptCache.setAutoDelete(true);

}
bool KScriptManager::addScript( const QString &scriptDesktopFile)
{
	//m_scriptNames.append(scriptName);
	// lets get some information about the script we are going to run...
	bool success = false;
	QString tmpScriptType = "";
	QString tmpScriptFile = "";
	QString tmpScriptMethod = "";
	// Read the desktop file

	if(KDesktopFile::isDesktopFile(scriptDesktopFile))
	{
		KDesktopFile desktop(scriptDesktopFile, true);
		m_scripts.insert(desktop.readName(), new ScriptInfo());
		m_scripts[desktop.readName()]->scriptType = desktop.readType();
		QString localpath = QString(kapp->name()) + "/scripts/" + desktop.readEntry("X-ScriptName", "");
		m_scripts[desktop.readName()]->scriptFile = locate("data", localpath);
#ifdef __GNUC__
#warning FIX ME - we need to decide where we will set the method for the script.
#endif
//		m_scripts[desktop.readName()]->scriptMethod = tmpScriptMethod;
		success = true;
	}
	return success;
}
bool KScriptManager::removeScript( const QString &scriptName )
{
	bool result = m_scriptCache.remove(scriptName);
	result = m_scripts.remove(scriptName);
	return result;
}
QStringList KScriptManager::scripts()
{
	QDictIterator<ScriptInfo> it( m_scripts );
//	return m_scriptNames;
	QStringList scriptList;
	while ( it.current() )
	{
		scriptList.append(it.currentKey());
		++it;
	}
	return scriptList;
}
void KScriptManager::clear()
{
	m_scriptCache.clear();
	m_scripts.clear();
}
void KScriptManager::runScript( const QString &scriptName, QObject *context, const QVariant &arg)
{
	ScriptInfo *newScript = m_scripts[scriptName];
	QString scriptType = "[X-Script-Runner] == '" + newScript->scriptType + "'";
	if (newScript)
	{
		// See if the script is allready cached...
		if ( !m_scriptCache[scriptName] )
		{
			// via some magic we will let the old script engine go away after
			// some minutes...
			// currently i am thinking a QTimer that will throw a signal in 10 minutes
			// to remove m_scriptCache[m_currentScript]
//			m_scriptCache.insert(scriptName, KParts::ComponentFactory::createInstanceFromQuery<KScriptInterface>( "KScriptRunner/KScriptRunner", scriptType, this));
			m_scriptCache.insert(scriptName, KParts::ComponentFactory::createInstanceFromQuery<KScriptInterface>( "KScriptRunner/KScriptRunner", QString::null, this));
		}
		m_currentScript = scriptName;

		if ( m_scriptCache[m_currentScript] )
		{
			m_scriptCache[m_currentScript]->ScriptClientInterface = this;
			if (newScript->scriptMethod != "")
				m_scriptCache[m_currentScript]->setScript( newScript->scriptFile, newScript->scriptMethod );
			else
				m_scriptCache[m_currentScript]->setScript( newScript->scriptFile );
			m_scriptCache[m_currentScript]->run(context, arg);
		}
		else
		{
			// Dialog and say we cant go on...
			// This is also a broken script so we need to remove it
			m_scriptCache.remove(m_currentScript);
		}
	}
}
#include "scriptmanager.moc"
#include "scriptinterface.moc"
