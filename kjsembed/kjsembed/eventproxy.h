/*
*  Copyright (C) 2003, Richard J. Moore <rich@kde.org>
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

#ifndef EVENTPROXY_H
#define EVENTPROXY_H
#include <QObject>
#include <QBitArray>
#include <QEvent>

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
            void callHandler( QEvent *e );

        private:
            QObjectBinding *m_watch;
            KJS::Interpreter *m_interpreter;
            QBitArray m_eventMask;
            uint m_refcount;
    };
}

#endif
