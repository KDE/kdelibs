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

#ifndef __scriptinterface_h__
#define __scriptinterface_h__
#include <qobject.h>
#include <kparts/plugin.h>

class QString;

class KScriptInterface :  public KParts::Plugin
{
	Q_OBJECT
public:

	/**
		Return the current script code data
		@returns QString containing the currenly runable code
	**/
	virtual QString Script() const = 0;
	/**
		Sets the parent object of the script to the passed in
		QObject.  This is used to access public data members of
		the main application.  This is handy if your script runner
		contains an object twin.  
	**/
	virtual void setParent( QObject *parent) = 0;
	/**
		Sets the path to the actual script that we are going to embed.
	**/
	virtual void setScript( QString PathToCode ) = 0;

public slots:
	/** 
		Run the actual script code 
	**/
	virtual void runScript() = 0;
	/**
		Abort the scripts run
	**/
	virtual void stopScript() =0;
	/**
		Check to see if the script is still running
	**/
	virtual bool status() =0;

signals:
	/**
		Emit a signal that contains the current error message from 
		a script output.
	**/
	virtual void errors(QString messages);
	virtual void output(QString messages);
	virtual void done(int errorcode);
	virtual void progress( int progress);
};

#endif
