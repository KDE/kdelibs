/***************************************************************************
 * childreninterface.h
 * This file is part of the KDE project
 * copyright (C)2004-2006 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KROSS_CHILDRENINTERFACE_H
#define KROSS_CHILDRENINTERFACE_H


#include <QtCore/QHash>
#include <QtCore/QObject>

#include "krossconfig.h"

namespace Kross {

    /**
     * Interface for managing \a Object collections.
     *
     * The \a Manager as well as the \a Action class inherit this interface
     * to allow to attach QObject to a global or a local context related
     * instances that should be published to the scripting code.
     */
    class KROSSCORE_EXPORT ChildrenInterface
    {
        public:

            /**
            * Additional options that could be defined for a QObject instance.
            */
            enum Options {
                NoOption = 0x00, ///< No additional options. This is the default.
                AutoConnectSignals = 0x01, ///< auto connect signals with scripting functions.

                //TODO probably add more options like;
                //ScriptableSlots = 0x01, ///< Publish slots that have Q_SCRIPTABLE defined.
                //NonScriptableSlots = 0x02, ///< Publish slots that don't have Q_SCRIPTABLE defined.
                //PrivateSlots = 0x04, ///< Publish private slots.
                //ProtectedSlots = 0x08, ///< Publish protected slots.
                //PublicSlots = 0x10, ///< Publish public slots.
                //AllSlots = ScriptableSlots|NonScriptableSlots|PrivateSlots|ProtectedSlots|PublicSlots,
                //ScriptableSignals = 0x100, ///< Publish signals that have Q_SCRIPTABLE defined.
                //NonScriptableSignals = 0x200, ///< Publish signals that don't have Q_SCRIPTABLE defined.
                //PrivateSignals = 0x400, ///< Publish private signals.
                //ProtectedSignals = 0x800, ///< Publish protected signals.
                //PublicSignals = 0x1000, ///< Publish public signals.
                //AllSignals = ScriptableSignals|NonScriptableSignals|PrivateSignals|ProtectedSignals|PublicSignals,
                //ScriptableProperties = 0x10000, ///< Publish properties that have Q_SCRIPTABLE defined.
                //NonScriptableProperties = 0x20000, ///< Publish properties that don't have Q_SCRIPTABLE defined.
                //AllProperties = ScriptableProperties|NonScriptableProperties,
                //GetParentObject = 0x100000, ///< Provide access to the parent QObject the QObject has.
                //SetParentObject = 0x200000, ///< Be able to set the parent QObject the QObject has.
                //ChildObjects = 0x400000, ///< Provide access to the child QObject's the QObject has.
                //AllObjects = GetParentObject|SetParentObject|ChildObjects

                LastOption = 0x1000000
            };

            /**
            * Add a QObject to the list of children.
            * \param object The QObject instance that should be added to the list of children.
            * \param name The name the QObject should be known under. If not defined, the
            * QObject's objectName is used.
            * \param options Additional optional options for the QObject.
            */
            void addObject(QObject* object, const QString& name = QString(), Options options = NoOption) {
                QString n = name.isNull() ? object->objectName() : name;
                m_objects.insert(n, object);
                if( options != NoOption )
                    m_options.insert(n, options);
            }

            /**
            * \return true if there exist a QObject with the \p name else false is returned.
            */
            bool hasObject(const QString& name) const {
                return m_objects.contains(name);
            }

            /**
            * \return the QObject with \p name or NULL if there exist no such object.
            */
            QObject* object(const QString& name) const {
                return m_objects.contains(name) ? m_objects.value(name) : 0;
            }

            /**
            * \return the map of QObject instances.
            */
            QHash< QString, QObject* > objects() const {
                return m_objects;
            }

            /**
            * \return true if the QObject with \p name was added with autoConnect enabled.
            */
            Options objectOption(const QString& name) const {
                return m_options.contains(name) ? m_options.value(name) : NoOption;
            }

            /**
            * \return the map of options.
            */
            QHash< QString, Options > objectOptions() const {
                return m_options;
            }

        private:
            QHash< QString, QObject* > m_objects;
            QHash< QString, Options > m_options;
    };

}

#endif

