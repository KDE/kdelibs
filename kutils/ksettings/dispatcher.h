/*  This file is part of the KDE project
    Copyright (C) 2003 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#ifndef KCDDISPATCHER_H
#define KCDDISPATCHER_H

#include <qobject.h>
#include <qmap.h>

class QCString;
class QSignal;
class QStrList;
template<class T> class KStaticDeleter;
class KInstance;

/**
 * @short Dispatch change notifications from the KCMs to the program.
 *
 * Since your program does not have direct control over the KCMs that get loaded
 * into the KConfigureDialog you need a way to get notified. This is what you
 * do:
 * \code
 * KCDDispatcher::self()->registerInstance( instance(), this, SLOT( readSettings() );
 * \endcode
 *
 * @author Matthias Kretz <kretz@kde.org>
 * @since 3.2
 */
class KCDDispatcher : public QObject
{
	friend class KStaticDeleter<KCDDispatcher>;

	Q_OBJECT
	public:
		/**
		 * Get a reference the the KCDDispatcher object.
		 */
		static KCDDispatcher * self();

		/**
		 * Register a slot to be called when the configuration for the instance
		 * associated with @p instanceName has changed. The instanceName is the
		 * name that is passed to KGenericFactory (if it is used). You can query
		 * it with KGenericFactory<YourClassName>::instance()->instanceName().
		 * This is the same name that is put into the .desktop file of the KCMs
		 * for the X-KDE-KCDParents.
		 *
		 * @param instanceName The internal name of the KInstance object and
		 *                     also the value of X-KDE-KCDParents.
		 * @param recv         The object that should receive the signal
		 * @param slot         The slot to be called: SLOT( slotName() )
		 */
		void registerInstance( const QCString & instanceName, QObject * recv, const char * slot );

		/**
		 * Convenience function. See above for what it does.
		 */
		void registerInstance( const KInstance * instance, QObject * recv, const char * slot );

		/**
		 * @return a list of all the instance names that are currently
		 * registered
		 */
		QStrList instanceNames() const;

	public slots:
		/**
		 * Call this slot when the configuration belonging to the associated
		 * instance name has changed. The registered slot will be called.
		 *
		 * @param instanceName The value of X-KDE-KCDParents.
		 */
		void reparseConfiguration( const QCString & instanceName );

	private slots:
		void unregisterInstance( QObject * );

	private:
		KCDDispatcher( QObject * parent = 0, const char * name = 0 );
		~KCDDispatcher();
		static KCDDispatcher * m_self;

		QMap<QCString, QSignal *> m_signals;
		QMap<QCString, int> m_instanceNameCount;
		QMap<QObject *, QCString> m_instanceName;

		class KCDDispatcherPrivate;
		KCDDispatcherPrivate * d;
};

// vim: sw=4 ts=4 noet
#endif // KCDDISPATCHER_H
