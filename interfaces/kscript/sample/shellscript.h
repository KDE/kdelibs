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

#include <kscript/scriptinterface.h>
#include <qvariant.h>
#include <kprocess.h>

class ShellScript : public KScriptInterface
{
	Q_OBJECT
public:
	ShellScript(QObject *parent, const char *name, const QStringList &args);
	virtual ~ShellScript();
	/**
	*	Return the current script code data
	*	@returns QString containing the currenly runable code
	**/
	QString script() const;
	/**
	*	Sets the parent object of the script to the passed in
	*	QObject.  This is used to access public data members of
	*	the main application.  This is handy if your script runner
	*	contains an object twin.
	**/
	void setScript( const QString &scriptFile );
	/**
	*	Sets the path to the script library that we are going to embed.
	*	The second argument is the function from the script library that
	*	we wish to call.
	**/
	void setScript( const QString &scriptLibFile, const QString &method );

public slots:
	/**
	*	Run the actual script code
	*	This can both take a context object that will be shared between the
	*	main application and a variant that will contain the arguments.
	**/
	void run(QObject *context = 0, const QVariant &arg = 0);
	/**
	*	Abort the scripts run
	**/
	void stop();
	/**
	*	Check to see if the script is still running
	**/
	bool isRunning();
private slots:
	void goodExit(KProcess *proc);
private:
	KProcess *m_script;
};

#endif
