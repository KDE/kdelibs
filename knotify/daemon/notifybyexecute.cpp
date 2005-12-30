/*
   Copyright (C) 2005-2006 by Olivier Goffart <ogoffart at kde.org>


   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 */


#include "notifybyexecute.h"
#include "knotifyconfig.h"

#include <kdebug.h>
#include <kprocess.h>
#include <kmacroexpander.h>




NotifyByExecute::NotifyByExecute(QObject *parent) : KNotifyPlugin(parent)
{
}


NotifyByExecute::~NotifyByExecute()
{
}



void NotifyByExecute::notify( int id, KNotifyConfig * config )
{
	QString command=config->readEntry( "execute" );
	
	kdDebug() << k_funcinfo << command << endl;
	
	if (!command.isEmpty()) {
// 	kdDebug() << "executing command '" << command << "'" << endl;
		QHash<QChar,QString> subst;
		subst.insert( 'e', config->eventid );
		subst.insert( 'a', config->appname );
		subst.insert( 's', config->text );
		subst.insert( 'w', QString::number( config->winId ));
		subst.insert( 'i', QString::number( id ));
		QString execLine = KMacroExpander::expandMacrosShellQuote( command, subst );
		if ( execLine.isEmpty() )
			execLine = command; // fallback

		KProcess p;
		p.setUseShell(true);
		p << execLine;
		p.start(KProcess::DontCare);
	}
	
	finish( id );
}

#include "notifybyexecute.moc"
