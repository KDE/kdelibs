/* This file is part of the KDE libraries
    Copyright (C) 2003,2004,2005,2006 Ian Reinhart Geiser <geiseri@kde.org>
    Copyright (C) 2003,2004,2005,2006 Matt Broadstone <mbroadst@gmail.com>
    Copyright (C) 2003,2004,2005,2006 Richard J. Moore <rich@kde.org>
    Copyright (C) 2003,2004,2005,2006 Erik L. Bunce <kde@bunce.us>

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


#ifndef EVENTPROXY_H
#define EVENTPROXY_H
#include <QtCore/QObject>
#include <QtCore/QBitRef>
#include <QtCore/QEvent>

#include <kjs/object.h>

namespace KJS
{
    class Interpreter;
}

namespace KJSEmbed
{
    class QObjectBinding;
    /**
    * Filters events for a QObject and forwards them to a JS handler.
    * @author Richard Moore, rich@kde.org
    */
    class EventProxy : public QObject
    {
        public:
            EventProxy( QObjectBinding *watch, KJS::Interpreter *interpreter );
            ~EventProxy();

            /** Returns true iff we forward the event type to JS. */
            bool isFiltered( QEvent::Type t ) const;

            /** Adds an event type to those we forward to JS. */
            void addFilter( QEvent::Type t );

            /**
            * Removes an event type from those we forward to JS. If there are no
            * event types left to forward then we self-destruct.
            */
            void removeFilter( QEvent::Type t );

            /** Reimplemented to forward events to JS. */
            bool eventFilter ( QObject *watched, QEvent *e );

        protected:
            bool callHandler( QEvent *e );

        private:
            QObjectBinding *m_watch;
            KJS::Interpreter *m_interpreter;
            QBitArray m_eventMask;
            uint m_refcount;
    };
}

#endif
//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
