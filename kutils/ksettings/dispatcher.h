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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef KSETTINGS_DISPATCHER_H
#define KSETTINGS_DISPATCHER_H

#include <qobject.h>
#include <qmap.h>
#include <kdelibs_export.h>

class Q3Signal;
template<class T> class KStaticDeleter;
class KInstance;
class KConfig;

namespace KSettings
{

/**
 * @short Dispatch change notifications from the KCMs to the program.
 *
 * Since your program does not have direct control over the KCMs that get loaded
 * into the KConfigureDialog you need a way to get notified. This is what you
 * do:
 * \code
 * Dispatcher::self()->registerInstance( instance(), this, SLOT( loadSettings() ) );
 * \endcode
 *
 * @author Matthias Kretz <kretz@kde.org>
 * @since 3.2
 */
class KUTILS_EXPORT Dispatcher : public QObject
{
    friend class KStaticDeleter<Dispatcher>;

    Q_OBJECT
    public:
        /**
         * Get a reference the the Dispatcher object.
         */
        static Dispatcher * self();

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
         * @return the KConfig object that belongs to the instanceName
         */
        KConfig * configForInstanceName( const QByteArray & instanceName );

        /**
         * @return a list of all the instance names that are currently
         * registered
         */
        QList<QByteArray> instanceNames() const;

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
        void reparseConfiguration( const QByteArray & instanceName );

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
        Dispatcher( QObject * parent = 0 );
        ~Dispatcher();
        static Dispatcher * m_self;

        struct InstanceInfo {
            KInstance * instance;
            Q3Signal * signal;
            int count;
        };
        QMap<QByteArray, InstanceInfo> m_instanceInfo;
        QMap<QObject *, QByteArray> m_instanceName;

        class DispatcherPrivate;
        DispatcherPrivate * d;
};

}

// vim: sw=4 sts=4 et
#endif // KSETTINGS_DISPATCHER_H
