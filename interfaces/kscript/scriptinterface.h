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

/**
*	\section Generic interface for building scripting engines
*
*	This interface will create a generic API for implementing script engines.
*	These engines can then be accessed from any application that supports this interface.
*
*
**/
#ifndef __scriptinterface_h__
#define __scriptinterface_h__
#include <qvariant.h>
#include <qobject.h>
#include <kdemacros.h>

//#include <scripclientinterface.h>
class QString;
class QObject;
class KScriptClientInterface;


//namespace KScriptInterface
//{
	/**
	*	This class is the base for all script engines.
	*	@author Ian Reinhart Geiser <geiseri@kde.org>
	*
	**/
	class KDE_EXPORT KScriptInterface : public QObject
	{
	Q_OBJECT
	public:
		/**
		*	Return the current script code data
		*	@returns QString containing the currenly runable code
		**/
		virtual QString script() const = 0;
		/**
		*	Sets the path to the script library that we are going to embed.
		**/
		virtual void setScript( const QString &scriptFile ) = 0;
		/**
		*	Sets the path to the script library that we are going to embed.
		*	The second argument is the function from the script library that
		*	we wish to call.
		**/
		virtual void setScript( const QString &scriptLibFile, const QString &method ) = 0;
		/**
		*	Run the actual script code
		*	This can both take a context object that will be shared between the
		*	main application and a variant that will contain the arguments.
		**/
		virtual void run(QObject *context = 0, const QVariant &arg = 0) = 0;
		/**
		*	Abort the scripts run
		**/
		virtual void kill() =0;
	public:
		/**
		*	This is the method for sending feedback to applications.
		*	example of how this works:
		*	\code
		*		ScriptClientInterface->error( message_to_send_back_to_the_main_application );
		*	\endcode
		*	Will send the error message back to the main application.
		**/
		KScriptClientInterface	*ScriptClientInterface;
	};
//};
#endif
