/***************************************************************************
 * metafunction.h
 * This file is part of the KDE project
 * copyright (C)2005-2006 Ian Reinhart Geiser <geiseri@kde.org>
 * copyright (C)2005-2006 Matt Broadstone     <mbroadst@gmail.com>
 * copyright (C)2005-2006 Richard J. Moore    <rich@kde.org>
 * copyright (C)2005-2006 Erik L. Bunce       <kde@bunce.us>
 * copyright (C)2005-2007 by Sebastian Sauer  <mail@dipe.org>
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

#ifndef KROSS_METAFUNCTION_H
#define KROSS_METAFUNCTION_H

#include <QtCore/QObject>
#include <QtCore/QArgument>
#include <QtCore/QByteRef>
#include <QtCore/QPointer>

namespace Kross {

    /**
     * The MetaFunction class implements a QObject to provide an adaptor
     * between Qt signals+slots and scripting functions.
     *
     * For example the Kross::PythonFunction and the Kross::RubyFunction
     * classes located in kdebindings inherit this class to connect a
     * QObject signal together with a callable python or ruby method.
     */
    class MetaFunction : public QObject
    {
        public:

            /**
            * Constructor.
            *
            * \param sender The QObject instance that sends the signal.
            * \param signal The signature of the signal the QObject emits.
            * \param callable The callable python function that should
            * be executed if the QObject emits the signal.
            */
            MetaFunction(QObject* sender, const QByteArray& signal)
                : QObject(), m_sender(sender), m_signature(QMetaObject::normalizedSignature(signal))
            {
                //krossdebug(QString("MetaFunction sender=\"%1\" signal=\"%2\"").arg(sender->objectName()).arg(m_signature.constData()));
                const uint signatureSize = m_signature.size() + 1;

                // content
                m_data[0] = 1;  // revision
                m_data[1] = 0;  // classname
                m_data[2] = 0;  // classinfo
                m_data[3] = 0;  // classinfo
                m_data[4] = 1;  // methods
                m_data[5] = 15; // methods
                m_data[6] = 0;  // properties
                m_data[7] = 0;  // properties
                m_data[8] = 0;  // enums/sets
                m_data[9] = 0;  // enums/sets

                // slots
                m_data[15] = 15;  // signature start
                m_data[16] = 15 + signatureSize;  // parameters start
                m_data[17] = 15 + signatureSize;  // type start
                m_data[18] = 15 + signatureSize;  // tag start
                m_data[19] = 0x0a; // flags
                m_data[20] = 0;    // eod

                // data
                m_stringData = QByteArray("ScriptFunction\0", 15);
                m_stringData += m_signature;
                m_stringData += QByteArray("\0\0", 2);

                // static metaobject
                staticMetaObject.d.superdata = &QObject::staticMetaObject;
                staticMetaObject.d.stringdata = m_stringData.data();
                staticMetaObject.d.data = m_data;
                staticMetaObject.d.extradata = 0;
            }

            /**
            * Destructor.
            */
            virtual ~MetaFunction() {}

            /**
            * The static \a QMetaObject instance that provides the
            * QMeta-information for this QObject class.
            */
            QMetaObject staticMetaObject;

            /**
            * \return the dynamic build \a QMetaObject instance
            * for this QObject instance.
            */
            const QMetaObject *metaObject() const {
                return &staticMetaObject;
            }

            /**
            * Try to cast this QObject instance into the class with
            * name \p _clname and return the casted pointer or NULL
            * if casting failed.
            */
            void *qt_metacast(const char *_clname) {
                if (! _clname)
                    return 0;
                if (! strcmp(_clname, m_stringData))
                    return static_cast<void*>( const_cast< MetaFunction* >(this) );
                return QObject::qt_metacast(_clname);
            }

            /**
            * This method got called if a method this QObject instance
            * defines should be invoked.
            */
            int qt_metacall(QMetaObject::Call _c, int _id, void **_a) = 0;

        protected:
            /// The sender QObject.
            QPointer<QObject> m_sender;
            /// The signature.
            QByteArray m_signature;
            /// The stringdata.
            QByteArray m_stringData;
            /// The data array.
            uint m_data[21];
    };

}

#endif
