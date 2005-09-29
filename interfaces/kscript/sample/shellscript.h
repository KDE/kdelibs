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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef __shellscript_h__
#define __shellscript_h__

#include <scriptinterface.h>
#include <qvariant.h>
#include <qobject.h>
#include <kprocess.h>
//using namespace KScriptInterface;
class ShellScript :  public KScriptInterface
{
	Q_OBJECT
public:
	ShellScript(KScriptClientInterface *parent, const char *name, const QStringList &args);
	virtual ~ShellScript();
	QString script() const;
	void setScript( const QString &scriptFile );
	void setScript( const QString &scriptLibFile, const QString &method );
	void run(QObject *context = 0, const QVariant &arg = 0);
	void kill();
private slots:
	void Exit(KProcess *proc);
	void stdErr(KProcess *proc, char *buffer, int buflen);
	void stdOut(KProcess *proc, char *buffer, int buflen);
private:
	KProcess *m_script;
	KScriptClientInterface *ScriptClientInterface;
	QString m_scriptName;
};

#endif
