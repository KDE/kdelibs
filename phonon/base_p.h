/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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

#ifndef PHONON_BASE_P_H
#define PHONON_BASE_P_H

#include "base.h"
#include <QList>
#include "basedestructionhandler.h"
#include "factory.h"
#include "phonondefs_p.h"

namespace Phonon
{
    class FactoryPrivate;

class BasePrivate
{
    K_DECLARE_PUBLIC(Base)
    friend class AudioOutputPrivate;
    friend class Phonon::FactoryPrivate;
    protected:
        enum CastId {
            BasePrivateType,
            AbstractAudioOutputPrivateType,
            AudioOutputType
        };
    public:
        /**
         * Returns the backend object. If the object does not exist it tries to
         * create it before returning.
         *
         * \return the Iface object, might return \c 0
         */
        QObject *backendObject()
        {
            if (!m_backendObject) {
                createBackendObject();
            }
            return m_backendObject;
        }

        const CastId castId;

    protected:
        BasePrivate(CastId _castId = BasePrivateType)
            : castId(_castId),
            q_ptr(0),
            m_backendObject(0)
        {
            Factory::registerFrontendObject(this);
        }

        virtual ~BasePrivate()
        {
            Factory::deregisterFrontendObject(this);
            delete m_backendObject;
            m_backendObject = 0;
        }

        /**
         * \internal
         * This method cleanly deletes the Iface object. It is called on
         * destruction and before a backend change.
         */
        void deleteBackendObject()
        {
            if (m_backendObject && aboutToDeleteBackendObject()) {
                delete m_backendObject;
                m_backendObject = 0;
            }
        }

        virtual bool aboutToDeleteBackendObject() = 0;

        /**
         * \internal
         * Creates the Iface object belonging to this class. For most cases the
         * implementation is
         * \code
         * Q_Q(ClassName);
         * m_iface = Factory::createClassName(this);
         * return m_iface;
         * \endcode
         *
         * This function should not be called except from slotCreateIface.
         *
         * \see slotCreateIface
         */
        virtual void createBackendObject() = 0;

        /**
         * \internal
         * This class has its own destroyed signal since some cleanup calls
         * need the pointer to the backend object intact. The
         * QObject::destroyed signals comes after the backend object was
         * deleted.
         *
         * As this class cannot derive from QObject a simple handler
         * interface is used.
         */
        void addDestructionHandler(Base *to, BaseDestructionHandler *handler)
        {
            to->k_ptr->handlers.append(handler);
        }

        /**
         * \internal
         * This class has its own destroyed signal since some cleanup calls
         * need the pointer to the backend object intact. The
         * QObject::destroyed signals comes after the backend object was
         * deleted.
         *
         * As this class cannot derive from QObject a simple handler
         * interface is used.
         */
        void removeDestructionHandler(Base *from, BaseDestructionHandler *handler)
        {
            from->k_ptr->handlers.removeAll(handler);
        }

        Base *q_ptr;
        QObject *m_backendObject;

    private:
        QList<BaseDestructionHandler *> handlers;
        BasePrivate(const BasePrivate &);
        BasePrivate &operator=(const BasePrivate &);
};
} //namespace Phonon

#endif // PHONON_BASE_P_H
// vim: sw=4 ts=4 tw=80
