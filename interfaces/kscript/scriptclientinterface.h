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
#ifndef __scriptclientinterface_h__
#define __scriptclientinterface_h__
#include <qvariant.h>

class QString;

//namespace KScriptInterface
//{
	/**
	*	This class is used for allowing feedback to the main system.
	*	@author Ian Reinhart Geiser <geiseri@kde.org>
	*
	**/
	class KScriptClientInterface
	{
		enum Result { ResultSuccess, ResultFailure, ResultContinue, ResultBreak };
	public:
		/**
		*	This function will allow the main application of any errors
		*	that have occured during processing of the script.
		*/
		virtual void error( const QString &msg ) =0;
		/**
		*	This function will allow the main application of any warnings
		*	that have occured during the processing of the script.
		*/
		virtual void warning( const QString &msg ) =0;
		/**
		*	This function will allow the main application of any normal
		*	output that has occured during the processing of the script.
		*/
		virtual void output( const QString &msg ) =0;
		/**
		*	This function will allow feedback to any progress bars in the main
		*	application as to how far along the script is.  This is very useful when
		*	a script is processing files or doing some long operation that is of a
		*	known duration.
		*/
		virtual void progress( int percent ) =0;
		/**
		*	This function will allow feedback on completion of the script.
		*	It turns the result as a @ref KScriptInteface::Result, and a return
		*	value as a @ref QVariant
		*/
		virtual void done( KScriptClientInterface::Result result, const QVariant &returned )  =0;
		/**
		*	Check to see if the script is still running
		**/
		//virtual bool isRunning() =0;
	};
//};

#endif
