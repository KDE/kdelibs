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

#include <kgenericfactory.h>
#include <kscript/scriptclientinterface.h>
typedef KGenericFactory<ShellScript> ShellScriptFactory;
K_EXPORT_COMPONENT_FACTORY( libshellscript, ShellScriptFactory( "ShellScript" ) );

ShellScript::ShellScript(QObject *parent, const char *name, const QStringList &args )
{
	m_script =  new KProcess();
}

ShellScript::~ShellScript()
{

}

QString ShellScript::script() const
{
//	return m_script;
}

void ShellScript::setScript( const QString &scriptFile  )
{
	*m_script << scriptFile;
}

void ShellScript::setScript( const QString &, const QString & )
{
    // ### what is this?
}

void ShellScript::run(QObject *context, const QVariant &arg)
{
	 m_script->start();
}
void ShellScript::kill()
{
	if (!m_script->kill())		// Kill the process
		m_script->kill(9);	// Kill it harder
}

void ShellScript::goodExit(KProcess *proc)
{
//	emit done(proc->exitStatus());
}


void ShellScript::badExit(KProcess *proc)
{

}
void ShellScript::stdErr(KProcess *proc)
{

}
void ShellScript::stdOut(KProcess *proc)
{
	ScriptClientInterface->output("message");
}

#include "shellscript.moc"
