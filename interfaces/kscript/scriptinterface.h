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

/**
*	@libdoc Generic interface for building scripting engines
*
*	This interface will create a generic API for implementing script engines.
*	These engines can then be accessed from any application that supports this interface.
*
*	Example:
**/
#ifndef __scriptinterface_h__
#define __scriptinterface_h__
#include <qobject.h>
#include <kparts/plugin.h>
#include <qvariant.h>

class QString;
/**
*	This class is the base for all script engines.
*	@author Ian Reinhart Geiser <geiseri@kde.org>
*
**/
class KScriptInterface :  public KParts::Plugin
{
	Q_OBJECT
public:

	enum Result { ResultSuccess, ResultFailure, ResultContinue, ResultBreak };

	/**
	*	Return the current script code data
	*	@returns QString containing the currenly runable code
	**/
	virtual QString script() const = 0;
	/**
	*	Sets the parent object of the script to the passed in
	*	QObject.  This is used to access public data members of
	*	the main application.  This is handy if your script runner
	*	contains an object twin.
	**/
	virtual void setScript( const QString &scriptFile ) = 0;
	/**
	*	Sets the path to the script library that we are going to embed.
	*	The second argument is the function from the script library that
	*	we wish to call.
	**/
	virtual void setScript( const QString &scriptLibFile, const QString &method ) = 0;

public slots:
	/**
	*	Run the actual script code
	*	This can both take a context object that will be shared between the
	*	main application and a variant that will contain the arguments.
	**/
	virtual void run(QObject *context = 0, const QVariant &arg = 0) = 0;
	/**
	*	Abort the scripts run
	**/
	virtual void stop() =0;
	/**
	*	Check to see if the script is still running
	**/
	virtual bool isRunning() =0;

signals:
	/*
	*	This signal is emitted to notify the main application of any errors
	*	that have occured during processing of the script.
	*/
	void error( const QString &msg );
	/*
	*	This signal is emitted to notify the main application of any warnings
	*	that have occured during the processing of the script.
	*/
	void warning( const QString &msg );
	/*
	*	This signal is emmitted to notify the main application of any normal
	*	output that has occured during the processing of the script.
	*/
	void output( const QString &msg );
	/*
	*	This signal is emitted to allow feedback to any progress bars in the main
	*	application as to how far along the script is.  This is very useful when
	*	a script is processing files or doing some long operation that is of a
	*	known duration.
	*/
	void progress( int percent );
	/*
	*	This signal is emmited on completion of the script.  It turns the result
	*	as a @ref KScriptInteface::Result, and a return value as a @ref QVariant
	*/
	void done( KScriptInterface::Result result, const QVariant &returned );
};

#endif
