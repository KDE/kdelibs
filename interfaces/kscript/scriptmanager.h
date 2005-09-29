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
#ifndef __scriptmanager_h__
#define __scriptmanager_h__

#include <qvariant.h>
#include <scriptclientinterface.h>
#include <scriptinterface.h>
#include <qdict.h>
#include <qobject.h>

#include <kdelibs_export.h>

class ScriptInfo;
//namespace KScriptInterface
//{

	/**
	*	This class is the base for all script engines.
	*	@author Ian Reinhart Geiser <geiseri@kde.org>
	*
	**/
	class KDE_EXPORT KScriptManager : public QObject, public KScriptClientInterface
	{
	Q_OBJECT
	friend class KScriptInterface;
	public:
		/**
		*	Create a new instance of the script engine.
		*/
		KScriptManager(QObject *parent, const char *name);
		/**
		*	Destroy the current script engine.
		*/
		virtual ~KScriptManager();
		/**
		*	Add a new script instance to the script engine.
		*	This should be the full name and path to the desktop
		*	file.
		*/
		bool addScript( const QString &scriptDesktopFile);
		/**
		*	Remove a script instance from the script engine.
		*	@returns the success of the operation.
		*/
		bool removeScript( const QString &scriptName );
		/**
		*	Access the names of script instances from the script engine.
		*	@returns a QStringList of the current scripts.
		*/
		QStringList scripts();
		/**
		*	Clear all script intstances in memory
		*/
		void clear();
		/**
		*	This function will allow the main application of any errors
		*	that have occurred during processing of the script.
		*/
		void error( const QString &msg ) {emit scriptError(msg);}
		/**
		*	This function will allow the main application of any warnings
		*	that have occurred during the processing of the script.
		*/
		void warning( const QString &msg ) {emit scriptWarning(msg);}
		/**
		*	This function will allow the main application of any normal
		*	output that has occurred during the processing of the script.
		*/
		void output( const QString &msg ) {emit scriptOutput(msg);}
		/**
		*	This function will allow feedback to any progress bars in the main
		*	application as to how far along the script is.  This is very useful when
		*	a script is processing files or doing some long operation that is of a
		*	known duration.
		*/
		void progress( int percent ) {emit scriptProgress(percent);}
		/**
		*	This function will allow feedback on completion of the script.
		*	It turns the result as a KScriptInteface::Result, and a return
		*	value as a QVariant
		*/
		void done( KScriptClientInterface::Result result, const QVariant &returned )  {emit scriptDone(result, returned);}

	public slots:
		/**
		*	Run the selected script
		*/
		void runScript( const QString &scriptName, QObject *context = 0, const QVariant &arg = 0 );
	signals:
		/**
		*	Send out a signal of the error message from the current running
		*	script.
		*/
		void scriptError( const QString &msg );
		/**
		*	Send out a signal of the warning message from the current running
		*	script.
		*/
		void scriptWarning( const QString &msg );
		/**
		*	Send out a signal of the output message from the current running
		*	script.
		*/
		void scriptOutput( const QString &msg );
		/**
		*	Send out a signal of the progress of the current running
		*	script.
		*/
		void scriptProgress( int percent);
		/**
		*	Send out a signal of the exit status of the script
		*
		*/
		void scriptDone( KScriptClientInterface::Result result, const QVariant &returned);
	protected:
		QDict<ScriptInfo> m_scripts;
		QDict<KScriptInterface> m_scriptCache;
		//QStringList m_scriptNames;
		QString m_currentScript;
	};
//};
#endif
