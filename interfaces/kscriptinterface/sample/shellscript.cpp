#include "shellscript.h"
#include <kgenericfactory.h>

typedef KGenericFactory<ShellScript> ShellScriptFactory;
K_EXPORT_COMPONENT_FACTORY( libshellscript, ShellScriptFactory( "shellscript" ) );

ShellScript::ShellScript(QObject *parent, const char *name, const QStringList &args )
{
	m_script =  new KProcess();
}

ShellScript::~ShellScript()
{

}

QString ShellScript::Script() const
{

}

void ShellScript::setParent( QObject *parent)
{

}

void ShellScript::setScript( QString PathToCode )
{
	*m_script << PathToCode;
}

void ShellScript::runScript()
{
	 m_script->start();
}
void ShellScript::stopScript()
{
	if (!m_script->kill())		// Kill the process
		m_script->kill(9);	// Kill it harder
}

void ShellScript::goodExit(KProcess *proc)
{
	emit done(proc->exitStatus());
}
