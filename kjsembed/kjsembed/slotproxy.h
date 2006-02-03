/*
*  Copyright (C) 2005, Ian Reinhart Geiser <geiseri@kde.org>
*  Copyright (C) 2006, Matt Broadstone <mbroadst@gmail.com>
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Library General Public
*  License as published by the Free Software Foundation; either
*  version 2 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Library General Public License for more details.
*
*  You should have received a copy of the GNU Library General Public License
*  along with this library; see the file COPYING.LIB.  If not, write to
*  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
*  Boston, MA 02111-1307, USA.
*/

#ifndef SLOTPROXY_H
#define SLOTPROXY_H

#include <QObject>
#include <QByteArray>
#include <kjs/object.h>

namespace KJS
{
    class Interpreter;
    class Value;
    class List;
    class ExecState;
}

namespace KJSEmbed
{
    class SlotProxy : public QObject
    {
        public:
            SlotProxy(KJS::JSObject *obj, KJS::Interpreter *interpreter, QObject *parent, const QByteArray &signature);
            ~SlotProxy();
            //Meta object stuff
            QMetaObject staticMetaObject;
            const QMetaObject *metaObject() const;
            void *qt_metacast(const char *_clname);
            int qt_metacall(QMetaObject::Call _c, int _id, void **_a);

        private:
            KJS::JSValue *callMethod( const QByteArray & methodName, void **_a );
            KJS::List convertArguments(KJS::ExecState *exec, void **_a );
            QByteArray m_signature;
            uint m_data[16];
            QByteArray m_stringData;
            KJS::Interpreter *m_interpreter;
            KJS::JSObject *m_object;
    };
}
#endif
