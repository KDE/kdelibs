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
 * KCDDispatcher::self()->registerInstance( instance(), this, SLOT( readSettings() ) );
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
         * has changed. @p instance is the KInstance object
         * that is passed to KGenericFactory (if it is used). You can query
         * it with KGenericFactory<YourClassName>::instance().
         * instance->instanceName() is also the same name that is put into the
         * .desktop file of the KCMs for the X-KDE-ParentComponents.
         *
         * @param instance     The KInstance object
         * @param recv         The object that should receive the signal
         * @param slot         The slot to be called: SLOT( slotName() )
         */
        void registerInstance( KInstance * instance, QObject * recv, const char * slot );

        /**
         * @return a list of all the instance names that are currently
         * registered
         */
        QStrList instanceNames() const;

//X         /**
//X          * @return The KInstance object belonging to the instance name you pass
//X          * (only works for registered instances of course).
//X          */
//X         KInstance * instanceForName( const QCString & instanceName );

    public slots:
        /**
         * Call this slot when the configuration belonging to the associated
         * instance name has changed. The registered slot will be called.
         *
         * @param instanceName The value of X-KDE-ParentComponents.
         */
        void reparseConfiguration( const QCString & instanceName );

        /**
         * When this slot is called the KConfig objects of all the registered
         * instances are sync()ed. This is usefull when some other KConfig
         * objects will read/write from/to the same config file, so that you
         * can first write out the current state of the KConfig objects.
         */
        void syncConfiguration();

    private slots:
        void unregisterInstance( QObject * );

    private:
        KCDDispatcher( QObject * parent = 0, const char * name = 0 );
        ~KCDDispatcher();
        static KCDDispatcher * m_self;

        struct InstanceInfo {
            KInstance * instance;
            QSignal * signal;
            int count;
        };
        QMap<QCString, InstanceInfo> m_instanceInfo;
        QMap<QObject *, QCString> m_instanceName;

        class KCDDispatcherPrivate;
        KCDDispatcherPrivate * d;
};

// vim: sw=4 sts=4 et
#endif // KCDDISPATCHER_H
