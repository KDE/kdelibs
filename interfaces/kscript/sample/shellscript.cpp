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

#include "shellscript.h"
#include <kdebug.h>
#include <kapplication.h>
#include <dcopclient.h>

#include <kgenericfactory.h>
#include <scriptclientinterface.h>
//using namespace KScriptInterface;
typedef KGenericFactory<ShellScript> ShellScriptFactory;
K_EXPORT_COMPONENT_FACTORY( libshellscript, ShellScriptFactory( "ShellScript" ) );

ShellScript::ShellScript(QObject *parent, const char *name, const QStringList &args )
{
	m_script =  new KProcess();
	connect ( m_script, SIGNAL(processExited(KProcess *)), SLOT(Exit(KProcess *)));
	connect ( m_script, SIGNAL(receivedStdout(KProcess *, char *, int)), SLOT(stdOut(KProcess *, char *, int )));
	connect ( m_script, SIGNAL(receivedStderr(KProcess *, char *, int)), SLOT(stdErr(KProcess *, char *, int )));
	// Connect feedback signals and slots
	kdDebug() << "Building new script engine" << endl;
}

ShellScript::~ShellScript()
{
	kdDebug() << "Destroying script engine" << endl;
}

QString ShellScript::script() const
{
//	return m_script;
	kdDebug() << "return script path" << endl;
}

void ShellScript::setScript( const QString &scriptFile  )
{
	kdDebug() << "set script " << kapp->dcopClient()->appId() << " " << scriptFile << endl;
	*m_script << "sh" << scriptFile << kapp->dcopClient()->appId();
}

void ShellScript::setScript( const QString &, const QString & )
{
    // ### what is this?
}

void ShellScript::run(QObject *context, const QVariant &arg)
{
	kdDebug() << "running the script" << endl;
	 m_script->start(KProcess::NotifyOnExit,KProcess::All);
}
void ShellScript::kill()
{
	if (!m_script->kill())		// Kill the process
		m_script->kill(9);	// Kill it harder
}

void ShellScript::Exit(KProcess *proc)
{
	kdDebug () << "Done processing..." << endl;
//	ScriptClientInteface->done(proc->exitStatus());
}

void ShellScript::stdErr(KProcess *proc, char *buffer, int buflen)
{
	kdDebug() << "Error" << endl;
	char *data = (char *) malloc(buflen);
	//kdDebug() << data << endl;
	free(data);
}
void ShellScript::stdOut(KProcess *proc, char *buffer, int buflen)
{
	kdDebug() << "Feedback" << endl;
	char *data = (char *) malloc(buflen);
	//kdDebug() << data << endl;
	ScriptClientInterface->output("data");
	free(data);

}

#include "shellscript.moc"
