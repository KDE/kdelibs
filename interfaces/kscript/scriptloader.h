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
#ifndef _script_loader_h
#define _script_loader_h
#include <qptrlist.h>
#include <kmainwindow.h>
#include <kaction.h>
#include <kscript/scriptinterface.h>

/**
 * Script loader
 */
class ScriptLoader : virtual public QObject
{
	Q_OBJECT
	public:
		/** 
		* Default Constructor
		*/
		ScriptLoader(KMainWindow *parent=0);
		~ScriptLoader();
		/** Return the a KSelectAction with all of the scripts
		 * @returns KSelectAction containing access to all of the scripts
		 */
		KSelectAction *getScripts();
	public slots:
		/** Run the current action.
		*/
		void runAction();
		/** Stop the currently running scripts operations
		*/
		void stopAction();
	signals:
		virtual void errors(QString messages);
		virtual void output(QString messages);
		virtual void done(int errorCode);
	private:
		QPtrList<KScriptInterface> m_scripts;
		KSelectAction *m_theAction;
		int m_currentSelection;
		KMainWindow *m_parent;
};
#endif
