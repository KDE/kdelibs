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
#ifndef __shellscript_h__
#define __shellscript_h__
#include <scriptinterface.h>
#include <kprocess.h>

class ShellScript : public KScriptInterface
{
	Q_OBJECT
public:
	ShellScript(QObject *parent, const char *name, const QStringList &args);
	virtual ~ShellScript();
	/**
		Return the current script code data
		@returns QString containing the currenly runable code
	**/
	QString Script() const;
	/**
		Sets the parent object of the script to the passed in
		QObject.  This is used to access public data members of
		the main application.  This is handy if your script runner
		contains an object twin.
	**/
	void setParent( QObject *parent);
	/**
		Sets the path to the actual script that we are going to embed.
	**/
	void setScript( QString PathToCode );
public slots:
	void runScript();
	void stopScript();
	bool status();
private slots:
	void goodExit(KProcess *proc);
private:
	KProcess *m_script;
};

#endif
