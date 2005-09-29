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
	*	To implement KScript in your application you would use this class to interface with the scripting engine.
	*	There are currently a few implementations of script managers around but developers can implement their own custom
	*	interfaces with this class.
	* @code
	*	class MyScript : public QObject, public KScriptClientInterface {
	*	Q_OBJECT
	*	public:
	*
	*	MyScript(QObject *parent)
	*	{
	*		// Create your @ref KScriptInterface here.
	*		m_interface = KParts::ComponentFactory::createInstanceFromQuery<KScriptInterface>( 
	*			"KScriptRunner/KScriptRunner", "([X-KDE-Script-Runner] == 'bash/shell')", this );
	*	}
	*
	*	virtual ~KScriptAction()
	*	{
	*		delete m_interface;
	*	}
	*
	*	signals:
	*	void error ( const QString &msg );
	*	void warning ( const QString &msg );
	*	void output ( const QString &msg );
	*	void progress ( int percent );
	*	void done ( KScriptClientInterface::Result result, const QVariant &returned );
	*
	*	public slots:
	*	void activate(const QVariant &args)
	*	{
	*		m_interface->run(parent(), args);
	*	}
	*
	*	private:
	*
	*	KScriptInterface *m_interface;
	*	};
	* @endcode
	*	Things to note about this example are that it only handles a single script type and instance.  You may wish to
	*	extend this.  
	**/

	class KScriptClientInterface
	{
	public:
		enum Result { ResultSuccess, ResultFailure, ResultContinue, ResultBreak };
		/**
		*	This function will allow the main application of any errors
		*	that have occurred during processing of the script.
		*	For script clients its best to implement this as a signal so feedback
		*	can be sent to the main application.
		*/
		virtual void error( const QString &msg ) =0;
		/**
		*	This function will allow the main application of any warnings
		*	that have occurred during the processing of the script.
		*	For script clients its best to implement this as a signal so feedback
		*	can be sent to the main application.
		*/
		virtual void warning( const QString &msg ) =0;
		/**
		*	This function will allow the main application of any normal
		*	output that has occurred during the processing of the script.
		*	For script clients its best to implement this as a signal so feedback
		*	can be sent to the main application.
		*/
		virtual void output( const QString &msg ) =0;
		/**
		*	This function will allow feedback to any progress bars in the main
		*	application as to how far along the script is.  This is very useful when
		*	a script is processing files or doing some long operation that is of a
		*	known duration.]
		*	For script clients its best to implement this as a signal so feedback
		*	can be sent to the main application.
		*/
		virtual void progress( int percent ) =0;
		/**
		*	This function will allow feedback on completion of the script.
		*	It turns the result as a KScriptInteface::Result, and a return
		*	value as a QVariant
		*	For script clients its best to implement this as a signal so feedback
		*	can be sent to the main application.
		*/
		virtual void done( KScriptClientInterface::Result result, const QVariant &returned )  =0;
		/**
		*	Returned when the script has finished running.
		*	For script clients its best to implement this as a signal so feedback
		*	can be sent to the main application.
		**/
		//virtual bool isRunning() =0;
	};
//};

#endif
