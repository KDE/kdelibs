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

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <kutils_export.h>
#include <kcomponentdata.h>

class Q3Signal;
template<class T> class KStaticDeleter;
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
 * Dispatcher::self()->registerComponent( componentData(), this, SLOT( loadSettings() ) );
 * \endcode
 *
 * @author Matthias Kretz <kretz@kde.org>
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
         * Register a slot to be called when the configuration for the componentData
         * has changed. @p componentData is the KComponentData object
         * that is passed to KGenericFactory (if it is used). You can query
         * it with KGenericFactory<YourClassName>::componentData().
         * componentData.componentName() is also the same name that is put into the
         * .desktop file of the KCMs for the X-KDE-ParentComponents.
         *
         * @param componentData     The KComponentData object
         * @param recv         The object that should receive the signal
         * @param slot         The slot to be called: SLOT( slotName() )
         */
        void registerComponent(const KComponentData &componentData, QObject *recv, const char *slot);

        /**
         * @return the KConfig object that belongs to the componentName
         */
        const KSharedConfig::Ptr &configForComponentName(const QByteArray &componentName);

        /**
         * @return a list of all the componentData names that are currently
         * registered
         */
        QList<QByteArray> componentNames() const;

    public Q_SLOTS:
        /**
         * Call this slot when the configuration belonging to the associated
         * componentData name has changed. The registered slot will be called.
         *
         * @param componentName The value of X-KDE-ParentComponents.
         */
        void reparseConfiguration(const QByteArray &componentName);

        /**
         * When this slot is called the KConfig objects of all the registered
         * instances are sync()ed. This is useful when some other KConfig
         * objects will read/write from/to the same config file, so that you
         * can first write out the current state of the KConfig objects.
         */
        void syncConfiguration();

    private Q_SLOTS:
        void unregisterComponent(QObject *);

    private:
        Dispatcher(QObject *parent = 0);
        ~Dispatcher();
        static Dispatcher *m_self;

        class DispatcherPrivate;
        DispatcherPrivate * const d;
};

}

// vim: sw=4 sts=4 et
#endif // KSETTINGS_DISPATCHER_H
